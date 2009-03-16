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

    *data   = chunk->data;
    *length = chunk->length; 
    return SMACHE_SUCCESS;
}

int
smache_compress(smache_chunk* chunk, void** data, size_t* length)
{
    return SMACHE_SUCCESS;
}

int
smache_release(smache_chunk* chunk, void* data)
{
    return SMACHE_SUCCESS;
}
