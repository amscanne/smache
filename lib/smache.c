/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
        return NULL;
    }
    memset(instance, 0, sizeof(*instance));

    instance->internals = malloc(sizeof(struct smache_priv));
    if( instance->internals == NULL )
    {
        free(instance);
        return NULL;
    }
    memset(instance->internals, 0, sizeof(*(instance->internals)));

    return instance;
}

void
smache_destroy(smache* instance)
{
    struct backend_list* backends = &(instance->internals->backends);

    /*
     * Call close on all the backends.
     */
    while( backends->current != NULL )
    {
        backends->current->close(backends->current);
        struct backend_list* last_backend = backends;
        backends = backends->next;
        if( last_backend != &(instance->internals->backends) )
        {
            free(last_backend);
        }
    }

    free(instance->internals);
    free(instance);
}

int
smache_add_backend(smache* instance, smache_backend* backend)
{
    struct backend_list* backends = &(instance->internals->backends);

    while( backends->current != NULL ) backends = backends->next;
    backends->current = backend;

    backends->next = malloc(sizeof(struct backend_list));
    if( backends->next == NULL )
    {
        backends->current = NULL;
        return SMACHE_ERROR;
    }

    backends->next->current = NULL;
    backends->next->next    = NULL;

    return SMACHE_SUCCESS;
}

int
smache_info(smache* instance, smache_hash* hash, size_t* length)
{
    return SMACHE_SUCCESS;
}

static int
_smache_put(smache* instance, smache_hash* hash, smache_chunk* chunk)
{
    struct backend_list* backends = &(instance->internals->backends);

    while( backends->current != NULL )
    {
        if( backends->current->push )
        {
            backends->current->put(backends->current, hash, chunk);
        }
        backends = backends->next;
    }

    return SMACHE_SUCCESS;
}

static int
_smache_get(smache* instance, smache_hash* hash, size_t offset, void* data, size_t* length)
{
    smache_chunk* chunk = smache_create_chunk();
    struct backend_list* backends = &(instance->internals->backends);

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
            /*
             * We got a valid key.  Now uncompress it.
             */
            void* uncompressed;
            size_t uncompressed_length;
            if( smache_uncompress(chunk, &uncompressed, &uncompressed_length) != SMACHE_SUCCESS )
            {
                smache_delete_chunk(chunk);
                return SMACHE_ERROR;
            }

            /*
             * Now, we see if it's a metahash and adjust the offset.
             */
            if( chunk->metahash )
            {
                smache_metahash* metahash = uncompressed;
                int count = uncompressed_length / sizeof(smache_metahash);
                int index = 0;
                for( index = 1; index < count; index++ )
                {
                    if( metahash[index].offset > offset )
                    {
                        break;
                    }
                }

                smache_hash newhash = metahash[index].hash;
                size_t newoffset    = (offset - metahash[index].offset);
                rval                = _smache_get(instance, &newhash, newoffset, data, length);
            }
            else
            {
                size_t minlength = *length < (uncompressed_length - offset) ? *length : (uncompressed_length - offset);
                memcpy(data, uncompressed, minlength);
                *length = minlength;
            }

            /*
             * Put the data back into our local cache.
             */
            if( rval == SMACHE_SUCCESS )
            {
                _smache_put(instance, hash, chunk);
            }

            /*
             * Release the compressed version.
             */
            smache_release(chunk, uncompressed);
        }

        /*
         * Check the next backend.
         */
        backends = backends->next;
    }

    smache_delete_chunk(chunk);
    return SMACHE_SUCCESS;
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
        size_t thislength = length;
        rval = _smache_get(instance, hash, offset, data, &thislength);
        data = (void*)(((unsigned char*)data) + thislength);
        length -= thislength;
        offset += thislength;
    }

    return rval;
}

static int
smache_put_fixed(smache* instance, smache_hash* rval, size_t offset, void* data, size_t length, smache_compression_type compression)
{
    if( length > SMACHE_MAXIMUM_CHUNKSIZE )
    {
        /*
         * We are making at least one metahash.
         * Divide up into chunks of a fixed size.
         */
    }
    else
    {
    }

    int i = 0;
    for( i = 0; i < 16; i++ )
    {
        rval->val[i] = i;
    }

    return SMACHE_ERROR;
}

int
smache_put(smache* instance, smache_hash* rval, size_t offset, void* data, size_t length, smache_block_algorithm block, smache_compression_type compression)
{
    switch( block )
    {
        case SMACHE_FIXED:
        return smache_put_fixed(instance, rval, offset, data, length, compression);
        default:
        fprintf(stderr, "put: Unknown block algorithm %d.\n", block);
        return SMACHE_ERROR;
    }
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
