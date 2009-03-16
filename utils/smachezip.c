/*
 * smachezip.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
usage(int argc, char** argv)
{
    printf("usage: %s <x|a> <archive> [files ...]\n", argv[0]);
    printf(" x - Extract the given archive.\n");
    printf(" a - Add to the given archive.\n");
    printf(" You must specify one of x or c.\n");
    return 0;
}

int
main (int argc, char** argv)
{
    /*
     * Parse the options.
     */
    if( argc < 3 || argv[1][1] != '\0' || !(argv[1][0] == 'x' || argv[1][0] == 'a') )
    {
        return usage(argc, argv);
    }
    create = (argv[1][0] == 'c');

    /*
     * Create the backend.
     */
    smache* sm = smache_create();
    smache_backend* bdb = smache_berkeleydb_backend(argv[2]);
    bdb.push = 1; /* Mark this backend as the 'push' backend. */
    if( sm == NULL ||  bdb == NULL || (smache_add_backend(sm, bdb) != SMACHE_SUCCESS) )
    {
        return 1;
    }

    /*
     * Add or remove files.
     */
    if( create )
    {
        for( int fileno = 3; fileno < argc; fileno++ )
        {
            smache_
            // ...
        }
    }
    else
    {
        if( argc > 3 )
        {
            for( int fileno = 3; fileno < argc; fileno++ )
            {
                /*
                 * Check the format of this hash (it must be a hash!)
                 */
                smache_hash hash;

                /*
                 * Given the correct hash, stat the file.
                 */
                size_t length;
                if( smache_info(sm, &hash, &length) != SMACHE_SUCCESS )
                {
                    fprintf(stderr, "error: unable to stat %s.\n", argv[fileno]);
                    continue;
                }

                /*
                 * Actually write the file out.
                 */
                int filedes = open(argv[fileno], O_CREAT|O_WRONLY);
                void* map_region = mmap(NULL, length, PROT_WRITE, MAP_SHARED, filedes, 0);
                if( smache_get(sm, &hash, 0, map_region, length) != SMACHE_SUCCESS )
                {
                }
            }
        }
        else
        {
            fprintf("Sorry, indexes are not currently supported.\n");
        }
    }

    return 0;
}
