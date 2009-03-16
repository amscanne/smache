/*
 * memcached.c
 */

#include <stdio.h>
#include <smache/smache.h>

smache_backend* smache_memcached_backend(const char* spec)
{
    fprintf(stderr, "warning: memcached backend not implemented.\n");
    return NULL;
}
