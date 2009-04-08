/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <smache/smache.h>

struct backend_list {
    smache_backend* current;
    struct backend_list* next;
};

struct socket_list {
    int socket;
    struct socket_list* next;
};

struct smache_priv {
    struct backend_list backends;
};


/*
 * Constructor and modifiers for a smache instance.
 */

smache*
smache_create()
{
    smache* instance = malloc(sizeof(smache));
    if( instance == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        return NULL;
    }
    memset(instance, 0, sizeof(*instance));

    instance->internals = malloc(sizeof(struct smache_priv));
    if( instance->internals == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        free(instance);
        return NULL;
    }
    memset(instance->internals, 0, sizeof(*(instance->internals)));

    SMACHE_DEBUG(instance, "Created smache instance.\n");

    return instance;
}

void
smache_destroy(smache* instance)
{
    struct backend_list* backends = &(instance->internals->backends);

    SMACHE_DEBUG(instance, "Destroying instance...\n");

    /*
     * Call close on all the backends.
     */
    while( backends->current != NULL )
    {
        SMACHE_DEBUG(instance, "Destroying backend...\n");
        backends->current->close(backends->current);
        struct backend_list* last_backend = backends;
        backends = backends->next;
        if( last_backend != &(instance->internals->backends) )
        {
            free(last_backend);
        }
    }

    SMACHE_DEBUG(instance, "Freeing internals...\n");
    free(instance->internals);

    SMACHE_DEBUG(instance, "Freeing instance...\n");
    free(instance);
}

int
smache_add_backend(smache* instance, smache_backend* backend)
{
    struct backend_list* backends = &(instance->internals->backends);

    SMACHE_DEBUG(instance, "Adding backend...\n");

    while( backends->current != NULL ) backends = backends->next;
    backends->current = backend;

    backends->next = malloc(sizeof(struct backend_list));
    if( backends->next == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        backends->current = NULL;
        return SMACHE_ERROR;
    }

    backends->next->current = NULL;
    backends->next->next    = NULL;

    return SMACHE_SUCCESS;
}

static int
_smache_put(smache* instance, smache_hash* hash, smache_chunk* chunk)
{
    struct backend_list* backends = &(instance->internals->backends);

    SMACHE_DEBUG(instance, "Putting chunk%s of length %d with key %s.\n", chunk->metahash ? "*" : "", chunk->length, smache_temp_hashstr(hash));

    while( backends->current != NULL )
    {
        if( backends->current->push && backends->current->exists(backends->current, hash) )
        {
            SMACHE_DEBUG(instance, "Found backend without key.\n");
            backends->current->put(backends->current, hash, chunk);
        }
        backends = backends->next;
    }

    return SMACHE_SUCCESS;
}

#define ARROWS() { int ai = 0; for(ai = 0; ai < depth; ai++) SMACHE_DEBUG_NL(instance, "-"); SMACHE_DEBUG_NL(instance, "-> "); }

static int
smache_get_flags(smache* instance, smache_hash* hash,
    size_t meta_offset, size_t first_meta_offset, size_t local_offset,
    void* data, size_t* length, size_t* totallength, int depth)
{
    int rval = SMACHE_SUCCESS;
    smache_chunk* chunk = smache_create_chunk();
    struct backend_list* backends = &(instance->internals->backends);
    size_t offset = meta_offset + local_offset;

    ARROWS();
    SMACHE_DEBUG(instance, "Fetching piece of hash %s (offset %ld).\n", smache_temp_hashstr(hash), (long)meta_offset);

    /*
     * Loop through all the backends.
     */
    while( backends->current != NULL )
    {
        int rval = SMACHE_SUCCESS;

        /*
         * We found the thing in a backend.
         */
        if( backends->current->get(backends->current, hash, chunk) == SMACHE_SUCCESS )
        {
            ARROWS();
            SMACHE_DEBUG(instance, "Found backend with key. Uncompressing...\n");

            /*
             * We got a valid key.  Now uncompress it.
             */
            void* uncompressed;
            size_t uncompressed_length;
            if( smache_uncompress(chunk, &uncompressed, &uncompressed_length) != SMACHE_SUCCESS )
            {
                fprintf(stderr, "smache_uncompress: %s\n", strerror(errno));
                smache_delete_chunk(chunk);
                return SMACHE_ERROR;
            }
            ARROWS();
            SMACHE_DEBUG(instance, "Uncompressed size is %ld.\n", (long)uncompressed_length);

            /*
             * Now, we see if it's a metahash and adjust the offset.
             */
            if( chunk->metahash )
            {
                smache_metahash* metahash = uncompressed;
                int count = uncompressed_length / sizeof(smache_metahash);
                ARROWS();
                SMACHE_DEBUG(instance, "Chunk is a metahash with %d entries.\n", count);

                int index = 0;
                for( index = 0; index < count; index++ )
                {
                    /*
                     * Metahashes record the ending offset.
                     */
                    ARROWS();
                    SMACHE_DEBUG(instance, "Metahash %d ends at %ld (need %ld).\n", index, (long)metahash[index].offset, (long)meta_offset);
                    if( metahash[index].offset > meta_offset && (index == 0 || metahash[index-1].offset <= meta_offset) )
                    {
                        break;
                    }
                }

                /*
                 * Save the length of the whole thing.
                 */
                if( totallength )
                {
                    *totallength = metahash[count-1].offset;
                }

                size_t lastend      = (index > 0 ? metahash[index-1].offset : first_meta_offset);
                smache_hash newhash = metahash[index].hash;
                size_t newoffset    = (meta_offset - lastend);

                if( data )
                {
                    /* NOTE: We don't actually *use* the newoffset though. */
                    ARROWS();
                    SMACHE_DEBUG(instance, "Fetching from hash index %d (offset %ld).\n", index, (long)offset);
                    rval = smache_get_flags(instance, &newhash, meta_offset, lastend, newoffset, data, length, NULL, depth+1);
                }
            }
            else
            {
                size_t availlength = uncompressed_length - local_offset;
                size_t minlength = *length < availlength ? *length : availlength;

                ARROWS();
                SMACHE_DEBUG(instance, "Chunk is a normal hash.\n");
                ARROWS();
                SMACHE_DEBUG(instance, "Returning %ld of %ld bytes.\n", (long)minlength, (long)availlength);

                if( data )
                {
                    memcpy(data, uncompressed + local_offset, minlength);
                }
                *length = minlength;
                if( totallength )
                {
                    *totallength = uncompressed_length;
                }
            }

            /*
             * Put the data back into our local cache.
             */
            if( rval == SMACHE_SUCCESS )
            {
                ARROWS();
                SMACHE_DEBUG(instance, "Writing through to local cache...\n");

                rval = _smache_put(instance, hash, chunk);
                if( rval != SMACHE_SUCCESS )
                {
                    fprintf(stderr, "put: %s\n", strerror(errno));
                }
            }

            /*
             * Release the compressed version.
             */
            ARROWS();
            SMACHE_DEBUG(instance, "Releasing compressed version.\n");
            smache_release(chunk, uncompressed);
        }

        /*
         * Check the next backend.
         */
        backends = backends->next;
    }

    smache_delete_chunk(chunk);
    ARROWS();
    SMACHE_DEBUG(instance, "Fetch complete.\n");
    return rval;
}

int
smache_get(smache* instance, smache_hash* hash, size_t offset, void* data, size_t length)
{
    int rval = SMACHE_SUCCESS;

    /*
     * Loop through and call get repeatedly.
     */
    while( length > 0 && rval == SMACHE_SUCCESS )
    {
        SMACHE_DEBUG(instance, "Getting offset %ld, length %ld of key %s.\n", (long)offset, (long)length, smache_temp_hashstr(hash));
        size_t thislength = length;
        rval = smache_get_flags(instance, hash, offset, 0, 0, data, &thislength, NULL, 0);
        SMACHE_DEBUG(instance, "Received data of length %ld.\n", (long)thislength);
        data = (void*)(((unsigned char*)data) + thislength);
        length -= thislength;
        offset += thislength;
    }

    return rval;
}

int
smache_info(smache* instance, smache_hash* hash, size_t* length)
{
    size_t thislength = 0;
    size_t totallength = 0;
    int rval = smache_get_flags(instance, hash, 0, 0, 0, NULL, &thislength, &totallength, 0);
    *length = totallength;
    return rval;
}

struct chunklist
{
    void*  data;
    size_t length;
    struct chunklist* next;
};

static struct chunklist*
smache_chunk_fixed(void* data, size_t length, size_t* count)
{
    struct chunklist first;
    struct chunklist* rval = &first;
    size_t current_offset = 0;

    rval->next = NULL;

    while( current_offset < length )
    {
        (*count)++;
        rval->next = malloc(sizeof(struct chunklist));
        rval = rval->next;
        rval->data = (void*)(((unsigned char*)data) + current_offset);
        rval->next = NULL;

        if( length - current_offset < 1024 )
        {
            rval->length = length - current_offset;
        }
        else
        {
            rval->length = 1024;
        }
        current_offset += rval->length;
    }

    return first.next;
}

static int
smache_put_flags(smache* instance, smache_hash* hash, void* data, size_t length,
                 smache_block_algorithm block, smache_compression_type compression, int depth)
{
    int rval = SMACHE_SUCCESS;
    float perc = 0;

    /*
     * Create a re-use a single chunk for all operations.
     */
    smache_chunk* chunk = smache_create_chunk();

    /*
     * Get the chunklist.
     */
    ARROWS();
    SMACHE_DEBUG(instance, "Creating chunklist for data of length %ld.\n", (long)length);
    struct chunklist* chunks = NULL;
    size_t count = 0;
    if( depth == 0 )
    {
        switch( block )
        {
            case SMACHE_FIXED:
            ARROWS();
            SMACHE_DEBUG(instance, "Using fixed algorithm.\n");
            chunks = smache_chunk_fixed(data, length, &count);
            break;

            default:
            fprintf(stderr, "put: Unknown block algorithm %d.\n", block);
            return SMACHE_ERROR;
        }
    }
    else
    {
        ARROWS();
        SMACHE_DEBUG(instance, "Using fixed algorithm.\n");
        chunks = smache_chunk_fixed(data, length, &count);
    }
    ARROWS();
    SMACHE_DEBUG(instance, "Created %ld chunks.\n", (long)count);

    /*
     * Allocate space for the metahashes.
     * NOTE: We do everything here, then recursively shrink the metahashes.
     */
    smache_metahash* metahashes = malloc(count * sizeof(smache_metahash));
    int hashno = 0;
    size_t offset = 0;
    for( hashno = 0; hashno < count; hashno++ )
    {
        /*
         * Compute the new percentage done.
         */
        float newperc = (float)((char*)chunks->data - (char*)data) * 100.0 / length;
        if( (int)newperc > (int)perc || hashno % 10000 == 0 )
        {
            perc = newperc;
            if( instance->progress ) fprintf(stderr, "%3.2f%% finished.\r", newperc);
        }

        /*
         * Compute the data hash.
         */
        ARROWS();
        SMACHE_DEBUG(instance, "Computing the hash for data at offset %ld.\n", (long)((char*)chunks->data - (char*)data));
        smache_computehash(hash, chunks->data, chunks->length);

        /*
         * Create the compressed block. (And mark it as a metahash).
         */
        ARROWS();
        SMACHE_DEBUG(instance, "Original data length is %ld.\n", (long)chunks->length);
        smache_compress(chunk, chunks->data, chunks->length, compression);
        chunk->metahash = (depth > 0);
        ARROWS();
        SMACHE_DEBUG(instance, "Compressed data length is %d.\n", chunk->length);

        /*
         * Put the chunk into the database.
         */
        rval = _smache_put(instance, hash, chunk);
        if( rval != SMACHE_SUCCESS )
        {
            fprintf(stderr, "put: %s\n", strerror(errno));
        }

        /*
         * Cycle to the next one.
         */
        if( depth > 0 )
        {
            smache_metahash* underlyingmetahash = (smache_metahash*)(((unsigned char*)chunks->data) + chunks->length - sizeof(smache_metahash));
            offset = underlyingmetahash->offset;
        }
        else
        {
            offset += chunks->length;
        }

        metahashes[hashno].offset = offset;
        metahashes[hashno].hash   = *hash;

        struct chunklist* prev = chunks;
        chunks = chunks->next;
        free(prev);
    }

    if( instance->progress ) fprintf(stderr, "100.00%% finished.\n");

    if( count > 1 )
    {
        /*
         * Recursively compute the hash of the metahash.
         */
        rval |= smache_put_flags(instance, hash, metahashes, count * sizeof(smache_metahash), SMACHE_FIXED, compression, depth + 1);
    }
    else
    {
        /*
         * The one and only hash computed was appropriate, we just don't touch the hash variable.
         */
    }
    
    /*
     * Free the allocated metahashes.
     */
    free(metahashes);

    return rval;
}

int
smache_put(smache* instance, smache_hash* rval, void* data, size_t length, smache_block_algorithm block, smache_compression_type compression)
{
    return smache_put_flags(instance, rval, data, length, block, compression, 0);
}

int
smache_delete(smache* instance, smache_hash* hash)
{
    struct backend_list* backends = &(instance->internals->backends);

    while( backends->current != NULL )
    {
        backends->current->delete(backends->current, hash);
        backends = backends->next;
    }

    return SMACHE_SUCCESS;
}
