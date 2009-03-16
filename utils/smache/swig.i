/* swig.i */
%module native
%{
#include "../../include/smache/smache.h"
%}

typedef enum {
    SMACHE_FIXED = 0
} smache_block_algorithm;

typedef enum {
    SMACHE_NONE = 0
} smache_compression_type;

smache_backend* smache_berkeleydb_backend(const char* filename);
smache_backend* smache_memcached_backend(const char* spec);
smache_backend* smache_remote_backend(int socket);

int smache_computehash(smache_hash*, const void* data, size_t length);
int smache_parsehash(smache_hash*, const char*);
char* smache_create_hashstr(smache_hash*);
void smache_delete_hashstr(char*);

int smache_getfile(smache*, smache_hash*, const char* filename);
int smache_putfile(smache*, smache_hash*, const char* filename, smache_block_algorithm block, smache_compression_type compression);

smache_chunk* smache_create_chunk(void);
void smache_delete_chunk(smache_chunk*);
smache_hash* smache_create_hash(void);
void smache_delete_hash(smache_hash*);

int smache_uncompress(smache_chunk*, void** data, size_t* length);
int smache_compress(smache_chunk*, void** data, size_t* length);
int smache_release(smache_chunk*, void* data);

smache* smache_create();
int smache_add_backend(smache*, smache_backend*);
void smache_destroy(smache*);

int smache_info(smache*, smache_hash*, size_t* length);
int smache_get(smache*, smache_hash*, size_t offset, void* data, size_t length);
int smache_put(smache*, smache_hash* rval, size_t offset, void* data, size_t length, smache_block_algorithm, smache_compression_type compression);
int smache_delete(smache*, smache_hash*);

int smache_server(unsigned short port);
