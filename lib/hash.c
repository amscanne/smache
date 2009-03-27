/*
 * smache.c
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mhash.h>
#include <smache/smache.h>

int
smache_computehash(smache_hash* hash, const void* data, size_t length)
{
    /*
     * Initialize the hash context.
     */
    MHASH td;
    td = mhash_init(MHASH_MD5);
    if( td == MHASH_FAILED || mhash_get_block_size(MHASH_MD5) != 16 )
    {
        fprintf(stderr, "mhash: Failed to initialize.\n");
        return SMACHE_ERROR;
    }

    /*
     * Perform the hash.
     */
    mhash(td, (unsigned char*)data, length);
   
    /*
     * Save the hash result.
     */ 
    unsigned char* val = mhash_end(td);
    memcpy(hash, val, sizeof(*hash));

    return SMACHE_SUCCESS;
}

static int32_t
onechartoint(char a)
{
    if( a >= '0' && a <= '9' )      return a - '0';
    else if( a >= 'a' && a <= 'f' ) return a - 'a' + 10;
    else if( a >= 'A' && a <= 'F' ) return a - 'A' + 10;
    else                            return -1;
}

static int32_t
twochartoint(char a, char b)
{
    return (onechartoint(a) << 4) + onechartoint(b); 
}

static char
inttochar(uint8_t a)
{
    if( a < 10 )      return '0' + a;
    else if( a < 16 ) return 'A' + (a - 10);
    else              return 'X';
}

int
smache_parsehash(smache_hash* hash, const char* str)
{
    int err = -1;
    int i   = 0;
    /*
     * Parse the hex string.
     */
    for( i = 0; i < 16; i++ )
    {
        char s1 = str[2*i];
        char s2 = str[2*i + 1];

        int32_t val = twochartoint(s1, s2);
        if( val < 0 )
        {
            err = i * 2;
            break;
        }

        hash->val[i] = val & 0xff;
    }

    /*
     * Check for the proper length.
     */
    if( err >= 0 || str[32] != '\0' )
    {
        fprintf(stderr, "error: Only able to parse hex hash strings (error in position %d of %s).\n", err, str);
        return SMACHE_ERROR;
    }

    return SMACHE_SUCCESS;
}

char*
smache_create_hashstr(smache_hash* hash)
{
    char* rval = malloc(sizeof(char) * 33);
    smache_write_hashstr(hash, rval);
    return rval;
}

int smache_write_hashstr(smache_hash* hash, char* rval)
{
    int i = 0;
    for( i = 0; i < 16; i++ )
    {
        rval[2*i]     = inttochar((hash->val[i] >> 4) & 0xf);
        rval[2*i + 1] = inttochar( hash->val[i]       & 0xf);
    }
    rval[32] = '\0';
    return 33;
}

char tmphashstr[33];
char*
smache_temp_hashstr(smache_hash* hash)
{
    smache_write_hashstr(hash, tmphashstr);
    return tmphashstr;
}

void
smache_delete_hashstr(char* hash)
{
    free(hash);
}
