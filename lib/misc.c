/*
 * misc.c
 */

#include <stdlib.h>
#include <smache/smache.h>

/* 
 * SMACHE_MAXIMUM_CHUNKSIZE
 * SMACHE_METAHASH_COUNT
 */

smache_hash*
smache_create_hash(void)
{
    return (smache_hash*)malloc(sizeof(smache_hash));
}

void
smache_delete_hash(smache_hash* hash)
{
    free(hash);
}

smache_chunk*
smache_create_chunk(void)
{
    return (smache_chunk*)malloc(sizeof(smache_chunk) + SMACHE_MAXIMUM_CHUNKSIZE);
}

void
smache_delete_chunk(smache_chunk* chunk)
{
    free(chunk);
}
