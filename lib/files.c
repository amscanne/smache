/*
 * smachezip.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <smache/smache.h>

int smache_putfile(smache* sm, smache_hash* hash, const char* filename, smache_block_algorithm block, smache_compression_type compression)
{
    int filedes = open(filename, O_CREAT|O_RDONLY);

    /*
     * Stat the file.
     */
    struct stat statbuf;
    if( fstat(filedes, &statbuf) < 0 )
    {
        fprintf(stderr, "error: unable to stat %s.\n", filename);
        close(filedes);
        return SMACHE_ERROR;
    }

    /*
     * Map the file.
     */
    size_t length = statbuf.st_size;
    void* map_region = mmap(NULL, length, PROT_READ, MAP_SHARED, filedes, 0);
    if( map_region == NULL )
    {
        fprintf(stderr, "error: mmap failed.\n");
        close(filedes);
        return SMACHE_ERROR;
    }
    if( smache_put(sm, hash, 0, map_region, length, block, compression) != SMACHE_SUCCESS )
    {
        munmap(map_region, length);
        close(filedes);
        return SMACHE_ERROR;
    }

    /*
     * Unmap the region and close the file.
     */
    munmap(map_region, length);
    close(filedes);

    return SMACHE_SUCCESS;
}


int smache_getfile(smache* sm, smache_hash* hash, const char* filename)
{
    /*
     * Given the correct hash, stat the file.
     */
    size_t length;
    if( smache_info(sm, hash, &length) != SMACHE_SUCCESS )
    {
        fprintf(stderr, "error: unable to stat %s.\n", filename);
        return SMACHE_ERROR;
    }

    /*
     * Actually write the file out.
     */
    int filedes = open(filename, O_CREAT|O_WRONLY);
    void* map_region = mmap(NULL, length, PROT_WRITE, MAP_SHARED, filedes, 0);
    if( map_region == NULL )
    {
        fprintf(stderr, "error: mmap failed.\n");
        close(filedes);
        return SMACHE_ERROR;
    }
    if( smache_get(sm, hash, 0, map_region, length) != SMACHE_SUCCESS )
    {
        munmap(map_region, length);
        close(filedes);
        return SMACHE_ERROR;
    }

    /*
     * Unmap the region and close the file.
     */
    munmap(map_region, length);
    close(filedes);

    return SMACHE_SUCCESS;
}
