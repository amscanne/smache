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
            // ...
        }
    }
    else
    {
        if( argc > 3 )
        {
            for( int fileno = 3; fileno < argc; fileno++ )
            {
                smache_hash hash;
                if( smache_lookup(argv[fileno], &hash) != SMACHE_SUCCESS )
                {
                    fprintf(stderr, "error: unable to find %s.\n", argv[fileno]);
                    continue;
                }
                // ...
            }
        }
        else
        {
            // ...
        }
    }

    return 0;
}
