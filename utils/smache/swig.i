/* swig.i */
%module native
%{
#include "../../include/smache/smache.h"
%}

/*
 * NOTE: This section is maintained automatically.
 * I find that it is better this way, since we can
 * limit weirdness demonstrated by Swig.
 * Generally this is caused by packed structures, etc.
 */

void smache_backend_setdebug(smache_backend* backend, int dbg);
void smache_backend_setpush(smache_backend* backend, int push);
void smache_setdebug(smache* instance, int dbg);
void smache_setprogress(smache* instance, int progress);
void smache_setblockalgorithm(smache* instance, smache_block_algorithm algorithm);
void smache_setcompressiontype(smache* instance, smache_compression_type compression);

void smache_null_check(smache* instance);
void smache_hash_null_check(smache_hash* instance);

typedef enum {
    SMACHE_FIXED = 0,
    SMACHE_RABIN = 1,
} smache_block_algorithm;

typedef enum {
    SMACHE_NONE = 0,
    SMACHE_LZO  = 1,
} smache_compression_type;

smache_backend* smache_berkeleydb_backend(const char* filename);
smache_backend* smache_memcached_backend(const char* spec);
smache_backend* smache_remote_backend(int socket);

int smache_computehash(smache_hash*, const void* data, size_t length);
int smache_parsehash(smache_hash*, const char*);
char* smache_create_hashstr(smache_hash*);
void smache_delete_hashstr(char*);

int smache_getfile(smache*, smache_hash*, const char* filename);
int smache_putfile(smache*, smache_hash*, const char* filename, size_t block_size);

smache_chunk* smache_create_chunk(void);
void smache_delete_chunk(smache_chunk*);
smache_hash* smache_create_hash();
void smache_delete_hash(smache_hash*);

smache* smache_create();
int smache_add_backend(smache*, smache_backend*);
void smache_destroy(smache*);

int smache_info(smache*, smache_hash*, size_t* length, size_t* references);
int smache_get(smache*, smache_hash*, size_t offset, void* data, size_t length);
int smache_put(smache*, smache_hash* rval, void* data, size_t length, size_t block_size);
int smache_delete(smache*, smache_hash*);
