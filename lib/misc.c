/*
 * misc.c
 */

#include <stdlib.h>
#include <string.h>
#include <smache/smache.h>

/* 
 * SMACHE_MAXIMUM_CHUNKSIZE
 * SMACHE_METAHASH_COUNT
 */

smache_hash*
smache_create_hash(void)
{
    smache_hash* hash = (smache_hash*)malloc(sizeof(smache_hash));
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
    smache_chunk* chunk = (smache_chunk*)malloc(sizeof(smache_chunk) + SMACHE_MAXIMUM_CHUNKSIZE);
    memset(chunk, 0, sizeof(smache_chunk));
    return chunk;
}

void
smache_delete_chunk(smache_chunk* chunk)
{
    free(chunk);
}
