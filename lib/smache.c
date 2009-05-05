/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <smache/smache.h>

/*
 * These are the private data structures that define a SMACHE instance.
 */

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
    /*
     * Allocate the instance.
     */
    smache* instance = malloc(sizeof(smache));
    if( instance == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        return NULL;
    }
    memset(instance, 0, sizeof(*instance));

    /*
     * Allocate the internals.
     */
    instance->internals = malloc(sizeof(struct smache_priv));
    if( instance->internals == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        free(instance);
        return NULL;
    }
    memset(instance->internals, 0, sizeof(*(instance->internals)));

    SMACHE_DEBUG(instance, "Created smache instance (%p).\n", instance);

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

    SMACHE_DEBUG(instance, "Added.\n");

    return SMACHE_SUCCESS;
}

static char scratch_data[sizeof(smache_chunk) + SMACHE_MAXIMUM_CHUNKSIZE];

static int
_smache_put(smache* instance, smache_hash* hash, smache_chunk* chunk, int adjrefs)
{
    struct backend_list* backends = &(instance->internals->backends);
    smache_chunk* sc_chunk = (smache_chunk*)(&scratch_data);

    SMACHE_DEBUG(instance, "Putting chunk%s of length %d with key %s.\n",
                           chunk->metahash ? "*" : "", chunk->length, smache_temp_hashstr(hash));

    while( backends->current != NULL )
    {
        if( backends->current->push )
        {
            if( backends->current->get(backends->current, hash, sc_chunk) == SMACHE_SUCCESS )
            {
                if( sc_chunk->length != chunk->length ||
                    !memcpy(sc_chunk->data, chunk->data, chunk->length) )
                {
                    fprintf(stderr, "Found mismatched chunks for key %s.  Aborting...\n", smache_temp_hashstr(hash));
                    exit(1);
                }
                sc_chunk->references += adjrefs;
            }
            else
            {
                chunk->references += 1;
                SMACHE_DEBUG(instance, "Found backend without key.\n");
                backends->current->put(backends->current, hash, chunk);
            }
        }
        backends = backends->next;
    }

    return SMACHE_SUCCESS;
}

#define ARROWS() { \
    int ai = 0; \
    for(ai = 0; ai < depth; ai++) \
        SMACHE_DEBUG_NL(instance, "-"); \
    SMACHE_DEBUG_NL(instance, "-> "); \
    }

static int
smache_get_flags(smache* instance, smache_hash* hash,
    uint64_t offset, int adjrefs,
    void* data, uint64_t* length, uint64_t* totallength,
    unsigned int* metahash_flag, smache_compression_type* compression_flag, 
    size_t* references, int depth)
{
    int rval = SMACHE_SUCCESS;
    smache_chunk* chunk = smache_create_chunk();
    struct backend_list* backends = &(instance->internals->backends);

    ARROWS();
    SMACHE_DEBUG(instance, "Fetching piece of hash %s (offset %ld).\n", smache_temp_hashstr(hash), (long)offset);

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
             * Set the appropriate flags on return.
             */
            if( metahash_flag )
            {
                *metahash_flag = chunk->metahash;
            }
            if( compression_flag )
            {
                *compression_flag = chunk->compression_type;
            }

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
                    SMACHE_DEBUG(instance, "Metahash %d ends at %ld (need %ld).\n", index, (long)metahash[index].offset, (long)offset);
                    if( metahash[index].offset > offset && (index == 0 || metahash[index-1].offset <= offset) )
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
                if( length )
                {
                    *length = uncompressed_length;
                }

                size_t lastend      = (index > 0 ? metahash[index-1].offset : 0);
                smache_hash newhash = metahash[index].hash;
                size_t newoffset    = (offset - lastend);

                if( data || adjrefs )
                {
                    /* NOTE: We don't actually *use* the newoffset though. */
                    ARROWS();
                    SMACHE_DEBUG(instance, "Fetching from hash index %d (offset %ld).\n", index, (long)newoffset);
                    rval = smache_get_flags(instance, &newhash, newoffset, adjrefs, data, length, NULL, NULL, NULL, NULL, depth+1);
                }
            }
            else
            {
                size_t availlength = uncompressed_length - offset;

                ARROWS();
                SMACHE_DEBUG(instance, "Chunk is a normal hash.\n");

                if( length )
                {
                    size_t minlength = *length < availlength ? *length : availlength;
                    if( data )
                    {
                        ARROWS();
                        SMACHE_DEBUG(instance, "Returning %ld of %ld bytes.\n", (long)minlength, (long)availlength);
                        memcpy(data, uncompressed + offset, minlength);
                    }
                    *length = minlength;
                }
                if( totallength )
                {
                    *totallength = uncompressed_length;
                }
            }

            /*
             * Save the references.
             */
            if( references )
            {
                *references = chunk->references;
            }

            /*
             * Put the data back into our local cache.
             */
            if( rval == SMACHE_SUCCESS )
            {
                ARROWS();
                SMACHE_DEBUG(instance, "Writing through to local cache...\n");

                rval = _smache_put(instance, hash, chunk, adjrefs);
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
smache_get(smache* instance, smache_hash* hash, uint64_t offset, void* data, uint64_t* length)
{
    int rval = SMACHE_SUCCESS;

    /*
     * Save the original offset and get the total length.
     */
    uint64_t orig_offset = offset;
    uint64_t totallength = 0;
    rval = smache_info(instance, hash, NULL, NULL, NULL, &totallength, NULL);

    /*
     * Loop through and call get repeatedly.
     */
    while( length > 0 && offset < totallength && rval == SMACHE_SUCCESS )
    {
        SMACHE_DEBUG(instance, "Getting offset %ld, length %ld of key %s (%ld left).\n", (long)offset, (long)length, smache_temp_hashstr(hash), (long)totallength-offset);
        uint64_t thislength = *length;
        rval = smache_get_flags(instance, hash, offset, 0, (char*)data, &thislength, NULL, NULL, NULL, NULL, 0);
        SMACHE_DEBUG(instance, "Received data of length %ld.\n", (long)thislength);
        data = (void*)(((unsigned char*)data) + thislength);
        *length -= thislength;
        offset  += thislength;
    }

    /*
     * Save the amount copied.
     */
    *length = (offset - orig_offset);

    return rval;
}

int
smache_adjref(smache* instance, smache_hash* hash, int refs)
{
    int rval = SMACHE_SUCCESS;

    /*
     * Get the length.
     */
    uint64_t length = 0;
    uint64_t offset = 0;
    rval = smache_get_flags(instance, hash, 0, 0, NULL, NULL, &length, NULL, NULL, NULL, 0);

    /*
     * Loop through and call get repeatedly.
     */
    while( length > 0 && rval == SMACHE_SUCCESS )
    {
        uint64_t thislength;
        rval = smache_get_flags(instance, hash, offset, refs, NULL, &thislength, NULL, NULL, NULL, NULL, 0);
        length -= thislength;
        offset += thislength;
    }

    return rval;
}

int
smache_info(smache* instance, smache_hash* hash, unsigned int* metahash, smache_compression_type* compression, uint64_t* length, uint64_t* totallength, size_t* references)
{
    if( length ) 
    {
        *length = SMACHE_MAXIMUM_CHUNKSIZE;
    }
    int rval = smache_get_flags(instance, hash, 0, 0, NULL, length, totallength, metahash, compression, references, 0);
    return rval;
}

static int
smache_put_flags(smache* instance, smache_hash* hash, void* data, uint64_t length, size_t block_length, int depth)
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

    struct chunklist* first_chunk = NULL;
    struct chunklist* chunks      = NULL;
    size_t count = 0;
    if( depth == 0 )
    {
        switch( instance->block_algorithm )
        {
            case SMACHE_FIXED:
            ARROWS();
            SMACHE_DEBUG(instance, "Using fixed algorithm.\n");
            first_chunk = smache_chunk_fixed(data, length, &count, block_length);
            break;

            case SMACHE_RABIN:
            ARROWS();
            SMACHE_DEBUG(instance, "Using Rabin-Karp rolling hashes.\n");
            first_chunk = smache_chunk_rabin(data, length, &count, block_length);
            break;

            default:
            fprintf(stderr, "put: Unknown block algorithm %d.\n", instance->block_algorithm);
            free(chunk);
            return SMACHE_ERROR;
        }
    }
    else
    {
        ARROWS();
        SMACHE_DEBUG(instance, "Using fixed algorithm (metahashes).\n");
        first_chunk = smache_chunk_fixed(data, length, &count, SMACHE_METAHASH_BLOCK_LENGTH);
    }
    ARROWS();
    SMACHE_DEBUG(instance, "Created %ld chunks.\n", (long)count);
    chunks = first_chunk;

    /*
     * Allocate space for the metahashes.
     * NOTE: We do everything here, then recursively shrink the metahashes.
     */
    smache_metahash* metahashes = malloc(count * sizeof(smache_metahash));
    int hashno      = 0;
    uint64_t offset = 0;
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
         * Figure out the offset (for the metahashes), and correct all underlying metahashes.
         */
        if( depth > 0 )
        {
            smache_metahash* underlyingmetahash = &((smache_metahash*)chunks->data)[(chunks->length / sizeof(smache_metahash)) - 1];
            offset = underlyingmetahash->offset;
        }
        if( depth == 0 )
        {
            offset += chunks->length;
        }

        metahashes[hashno].offset = offset;

        if( depth > 0 && hashno > 0 )
        {
            uint64_t lasthashoffset = metahashes[hashno-1].offset;
            int i = 0;
            for( i = 0; i < (chunks->length / sizeof(smache_metahash)); i++ )
            {
                ((smache_metahash*)chunks->data)[i].offset -= lasthashoffset;
            }
        }
        if( depth > 0 )
        {
            SMACHE_DEBUG(instance, "Underlying data offets:\n");
            int i = 0;
            for( i = 0; i < (chunks->length / sizeof(smache_metahash)); i++ )
            {
                SMACHE_DEBUG(instance, " %d: %ld\n", i, (long)((smache_metahash*)chunks->data)[i].offset);
            }
        }

        /*
         * Compute the data hash.
         */
        ARROWS();
        SMACHE_DEBUG(instance, "Computing the hash for data at offset %ld.\n", (long)((char*)chunks->data - (char*)data));
        smache_computehash(hash, chunks->data, chunks->length);

        metahashes[hashno].hash = *hash;

        /*
         * Create the compressed block. (And mark it as a metahash).
         */
        ARROWS();
        SMACHE_DEBUG(instance, "Original data length is %ld.\n", (long)chunks->length);
        smache_compress(chunk, chunks->data, chunks->length, instance->compression_type);
        chunk->metahash = (depth > 0);
        ARROWS();
        SMACHE_DEBUG(instance, "Compressed data length is %ld.\n", (long)chunk->length);

        /*
         * Actually put the block in the database.
         */
        rval = _smache_put(instance, hash, chunk, 1);
        if( rval != SMACHE_SUCCESS )
        {
            fprintf(stderr, "put: %s\n", strerror(errno));
        }

        struct chunklist* prev = chunks;
        chunks = chunks->next;
        free(prev);
    }

    if( instance->progress )
    {
        fprintf(stderr, "100.00%% finished.\n");
    }

    if( count > 1 )
    {
        /*
         * Recursively compute the hash of the metahash.
         */
        rval |= smache_put_flags(instance, hash, metahashes,
                    count * sizeof(smache_metahash), block_length, depth + 1);
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
    free(chunk);

    return rval;
}

int
smache_put(smache* instance, smache_hash* rval, void* data, uint64_t length, size_t block_length)
{
    return smache_put_flags(instance, rval, data, length, block_length, 0);
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
