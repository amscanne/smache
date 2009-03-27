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

    SMACHE_DEBUG(instance, "Putting chunk of length %d with key %s.\n", chunk->length, smache_temp_hashstr(hash));
    if( chunk->metahash ) { SMACHE_DEBUG(instance, "* Chunk is a metahash.\n"); } 

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

static int
_smache_get(smache* instance, smache_hash* hash, size_t offset, void* data, size_t* length, size_t* totallength)
{
    int rval = SMACHE_SUCCESS;
    smache_chunk* chunk = smache_create_chunk();
    struct backend_list* backends = &(instance->internals->backends);

    SMACHE_DEBUG(instance, "Fetching piece of hash %s.\n", smache_temp_hashstr(hash));

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
            SMACHE_DEBUG(instance, "Uncompressed size is %ld.\n", uncompressed_length);

            /*
             * Now, we see if it's a metahash and adjust the offset.
             */
            if( chunk->metahash )
            {
                smache_metahash* metahash = uncompressed;
                int count = uncompressed_length / sizeof(smache_metahash);
                SMACHE_DEBUG(instance, "Chunk is a metahash with %d entries.\n", count);

                int index = 0;
                for( index = 0; index < count; index++ )
                {
                    /*
                     * Metahashes record the ending offset.
                     */
                    if( metahash[index].offset > offset )
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

                size_t lastend = 0;
                if( index > 0 ) lastend = metahash[index-1].offset;

                smache_hash newhash = metahash[index].hash;
                size_t newoffset    = (offset - lastend);

                if( data )
                {
                    SMACHE_DEBUG(instance, "Fetching from hash index %d.\n", index-1);
                    rval = _smache_get(instance, &newhash, newoffset, data, length, NULL);
                }
            }
            else
            {
                size_t availlength = uncompressed_length - offset;
                size_t minlength = *length < availlength ? *length : availlength;

                SMACHE_DEBUG(instance, "Chunk is a normal hash.\n");
                SMACHE_DEBUG(instance, "Returning %lu of %lu bytes.\n", minlength, availlength);

                if( data )
                {
                    memcpy(data, uncompressed, minlength);
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
            SMACHE_DEBUG(instance, "Releasing compressed version.\n");
            smache_release(chunk, uncompressed);
        }

        /*
         * Check the next backend.
         */
        backends = backends->next;
    }

    smache_delete_chunk(chunk);
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
        SMACHE_DEBUG(instance, "Getting offset %lu, length %lu of key %s.\n", offset, length, smache_temp_hashstr(hash));
        size_t thislength = length;
        rval = _smache_get(instance, hash, offset, data, &thislength, NULL);
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
    int rval = _smache_get(instance, hash, 0, NULL, &thislength, &totallength);
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

        if( length - current_offset < 512 )
        {
            rval->length = length - current_offset;
        }
        else
        {
            rval->length = 512;
        }
        current_offset += rval->length;
    }

    return first.next;
}

static int
smache_put_flags(smache* instance, smache_hash* hash, void* data, size_t length,
                 smache_block_algorithm block, smache_compression_type compression, int meta)
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
    SMACHE_DEBUG(instance, "Creating chunklist for data of length %lu.\n", length);
    struct chunklist* chunks = NULL;
    size_t count = 0;
    switch( block )
    {
        case SMACHE_FIXED:
        SMACHE_DEBUG(instance, "Using fixed algorithm.\n");
        chunks = smache_chunk_fixed(data, length, &count);
        break;

        default:
        fprintf(stderr, "put: Unknown block algorithm %d.\n", block);
        return SMACHE_ERROR;
    }
    SMACHE_DEBUG(instance, "Created %ld chunks.\n", count);


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
            fprintf(stderr, "%3.2f%% finished.\r", newperc);
        }

        /*
         * Compute the data hash.
         */
        SMACHE_DEBUG(instance, "Computing the hash for data at offset %ld.\n", ((char*)chunks->data - (char*)data));
        smache_computehash(hash, chunks->data, chunks->length);

        /*
         * Create the compressed block. (And mark it as a metahash).
         */
        SMACHE_DEBUG(instance, "Original data length is %lu.\n", chunks->length);
        smache_compress(chunk, chunks->data, chunks->length, compression);
        chunk->metahash = meta;
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
        if( meta )
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

    fprintf(stderr, "100.00%% finished.\n");

    if( count > 1 )
    {
        /*
         * Recursively compute the hash of the metahash.
         */
        rval |= smache_put_flags(instance, hash, metahashes, count * sizeof(smache_metahash), SMACHE_FIXED, compression, 1);
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
