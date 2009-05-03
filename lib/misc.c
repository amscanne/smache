/*
 * misc.c
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <smache/smache.h>

smache_hash*
smache_create_hash(void)
{
    smache_hash* hash = (smache_hash*)malloc(sizeof(smache_hash));
    if( hash == NULL )
    {
        fprintf(stderr, "error: Out of memory!\n");
        exit(1);
    }
    memset(hash, 0, sizeof(smache_hash));
    return hash;
}

void
smache_delete_hash(smache_hash* hash)
{
    free(hash);
}

smache_chunk*
smache_create_chunk(void)
{
    smache_chunk* chunk = (smache_chunk*)malloc(sizeof(smache_chunk) + SMACHE_MAXIMUM_CHUNKSIZE + 1);
    if( chunk == NULL )
    {
        fprintf(stderr, "error: Out of memory!\n");
        exit(1);
    }
    memset(chunk, 0, sizeof(smache_chunk));
    return chunk;
}

void
smache_delete_chunk(smache_chunk* chunk)
{
    free(chunk);
}

void smache_null_check(smache* instance)
{
    fprintf(stderr, "Check -> Pointer is %p.\n", instance);
}

void smache_hash_null_check(smache_hash* instance)
{
    fprintf(stderr, "Check -> Pointer is %p.\n", instance);
}
