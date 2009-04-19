/*
 * cut.c
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mhash.h>
#include <smache/smache.h>

#define PRIME_BASE  257
#define PRIME_MOD   1000000007
#define WINDOW_SIZE 32

/*
 * The large and small bounds for hashes and implicit,
 * we don't really have any choice in the matter.
 */
#define PATH_SMALL (sizeof(smache_hash))
#define PATH_LARGE (SMACHE_MAXIMUM_CHUNKSIZE)


struct chunklist*
smache_chunk_fixed(void* data, size_t length, size_t* count, size_t block_length)
{
    struct chunklist first;
    struct chunklist* rval = &first;
    size_t current_offset = 0;

    rval->next = NULL;

    while( current_offset < length )
    {
        (*count)++;
        rval->next = malloc(sizeof(struct chunklist));
        rval = rval->next;
        rval->data = (void*)(((unsigned char*)data) + current_offset);
        rval->next = NULL;

        if( length - current_offset < block_length )
        {
            rval->length = length - current_offset;
        }
        else
        {
            rval->length = length;
        }
        current_offset += rval->length;
    }

    return first.next;
}

struct chunklist*
smache_chunk_rabin(void* data, size_t length, size_t* count, size_t block_length)
{
    struct chunklist* first = malloc(sizeof(struct chunklist));
    struct chunklist* rval  = first;
    rval->data   = data;
    rval->next   = NULL;
    (*count)++;

    /*
     * NOTE: This should be broken into a separate "init"
     * function.  But it's not too computational expense,
     * since we have a fixed, short window size.
     */
    long long power = 1;
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        power = (power * PRIME_BASE) % PRIME_MOD;
    }

    int lasti = 0;
    long long hash;
    for (int i = 0; i < length; i++)
    {
        /*
         * Add the next character to the window.
         */
        hash = (hash*PRIME_BASE + ((char*)data)[i]) % PRIME_MOD;

        /*
         * Remove the first character if necessary.
         */
        if ( i > length )
        {
            hash -= (power * haystack[i-length] % PRIME_MOD);
            if( hash < 0 ) hash += PRIME_MOD;
        }

        /*
         * Check for cut point.
         */
        if (hash % block_length == 0)
        {
            (*count)++;
            rval->next   = malloc(sizeof(struct chunklist));
            rval->length = (i - lasti);
            rval = rval->next;
            rval->data   = (void*)(((unsigned char*)data) + i);
            rval->next = NULL;
            lasti = i;
        }
    }

    /*
     * Set last length.
     */
    rval->length = (length - lasti);

    return first;
}
