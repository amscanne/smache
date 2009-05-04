/*
 * smachezip.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <smache/smache.h>

/*
 * Putfile is a simple helper function that takes an instance and filename,
 * and inserts the contents of the file into the cache.
 */
int smache_putfile(smache* sm, smache_hash* hash, const char* filename, size_t block_length)
{
    int filedes = open(filename, O_CREAT|O_RDONLY);
    if( filedes < 0 )
    {
        fprintf(stderr, "open: %s (%d)\n", strerror(errno), errno);
        return SMACHE_ERROR;
    }

    SMACHE_DEBUG(sm, "Opened file %s.\n", filename);

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

    SMACHE_DEBUG(sm, "Found length %ld.\n", (long)statbuf.st_size);

    /*
     * Map the file.
     */
    size_t length = statbuf.st_size;
    void* map_region = mmap(NULL, length, PROT_READ, MAP_SHARED, filedes, 0);
    if( map_region == NULL || map_region == (void*)-1 )
    {
        fprintf(stderr, "mmap: %s (%d)\n", strerror(errno), errno);
        close(filedes);
        return SMACHE_ERROR;
    }
    SMACHE_DEBUG(sm, "Mapped file.\n");

    SMACHE_DEBUG(sm, "Using block length of %ld.\n", (long)block_length);
    if( smache_put(sm, hash, map_region, length, block_length) != SMACHE_SUCCESS )
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

/*
 * Getfile is a simple helper function that takes an instance and a hash,
 * and outputs the contents of the hash to the filename.
 */
int smache_getfile(smache* sm, smache_hash* hash, const char* filename)
{
    /*
     * Given the correct hash, stat the file.
     */
    uint64_t length;
    uint64_t totallength;
    size_t references;
    if( smache_info(sm, hash, &length, &totallength, &references) != SMACHE_SUCCESS )
    {
        fprintf(stderr, "error: unable to stat %s.\n", filename);
        return SMACHE_ERROR;
    }

    /*
     * Open the file to be written.
     */
    int filedes = open(filename, O_CREAT|O_RDWR|O_TRUNC);
    if( filedes < 0 )
    {
        fprintf(stderr, "open: %s (%d)\n", strerror(errno), errno);
        return SMACHE_ERROR;
    }

    /*
     * Make the file the correct length.
     */
    lseek(filedes, totallength-1, SEEK_SET);
    char zero = 0;
    write(filedes, &zero, 1);

    /*
     * Open in an mmap region.
     */
    void* map_region = mmap(NULL, totallength, PROT_WRITE, MAP_SHARED, filedes, 0);
    if( map_region == NULL || map_region == (void*)-1 )
    {
        fprintf(stderr, "mmap: %s (%d)\n", strerror(errno), errno);
        close(filedes);
        return SMACHE_ERROR;
    }
    if( smache_get(sm, hash, 0, map_region, totallength) != SMACHE_SUCCESS )
    {
        munmap(map_region, totallength);
        close(filedes);
        return SMACHE_ERROR;
    }

    /*
     * Unmap the region and close the file.
     */
    munmap(map_region, totallength);
    close(filedes);

    return SMACHE_SUCCESS;
}
