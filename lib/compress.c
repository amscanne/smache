/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <smache/smache.h>

int
smache_uncompress(smache_chunk* chunk, void** data, size_t* length)
{
    if( chunk->compression_type != SMACHE_NONE )
    {
        fprintf(stderr, "compression: Unsupported type.\n");
        return SMACHE_ERROR;
    }

    /*
     * Just copy out a pointer to the data.
     */
    *data   = chunk->data;
    *length = chunk->length; 
    return SMACHE_SUCCESS;
}

int
smache_compress(smache_chunk* chunk, void* data, size_t length, smache_compression_type compression)
{
    if( chunk->compression_type != SMACHE_NONE )
    {
        fprintf(stderr, "compression: Unsupported type.\n");
        return SMACHE_ERROR;
    }

    /*
     * Simply copy in the data.
     */
    memcpy(chunk->data, data, length);
    chunk->length = length;
    chunk->compression_type = compression;

    return SMACHE_SUCCESS;
}

int
smache_release(smache_chunk* chunk, void* data)
{
    return SMACHE_SUCCESS;
}
