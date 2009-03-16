/*
 * smache.h
 */

#ifndef _SMACHE_H_
#define _SMACHE_H_

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

/*
 * Error and success codes.
 * (Used for all functions below).
 */

#define SMACHE_SUCCESS (0)
#define SMACHE_ERROR   (1)

/*
 * Different compression algorithms.
 * By default, we use no compression.
 */

typedef enum {
    SMACHE_FIXED = 0
} smache_block_algorithm;

typedef enum {
    SMACHE_NONE = 0
} smache_compression_type;

/*
 * Data definitions.
 */

typedef struct {
    /* NOTE: This is always 16 bytes == 128 bits. */
    unsigned char val[16];
} smache_hash;

smache_hash* smache_create_hash(void);
void smache_delete_hash(smache_hash*);

typedef struct {
    /*
     * If this is a metahash, then the data is a list
     * of hash objects and 32-bit offsets.
     * Each hash has a constant, fixed size.
     */
    uint8_t  metahash         :1;
    uint8_t  compression_type :3;
    uint16_t references       :12;
    uint16_t length           :16;
    unsigned char data[0];
} __attribute__((packed)) smache_chunk;

smache_chunk* smache_create_chunk(void);
void smache_delete_chunk(smache_chunk*);

typedef struct {
    uint32_t    offset;
    smache_hash hash;
} smache_metahash;

#define SMACHE_MAXIMUM_CHUNKSIZE  (0xffff)
#define SMACHE_METAHASH_COUNT(l)  (l >> 3)

/*
 * Definitions for the backend, remote and smache instance.
 */

typedef struct struct_smache_backend {
    uint8_t  push     :1;
    uint32_t reserved :31;

    int (*get)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    int (*put)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    int (*delete)(struct struct_smache_backend*, smache_hash*);
    int (*close)(struct struct_smache_backend*);

    void* internals;

} __attribute__((packed)) smache_backend;

typedef struct {
    struct smache_priv* internals;
} smache;

/*
 * Constructors for backends and remotes.
 */

smache_backend* smache_berkeleydb_backend(const char* filename);
smache_backend* smache_memcached_backend(const char* spec);
smache_backend* smache_remote_backend(int socket);

/*
 * Hash helpers.
 */
int smache_computehash(smache_hash*, const void* data, size_t length);
int smache_parsehash(smache_hash*, const char*);
char* smache_create_hashstr(smache_hash*);
void smache_delete_hashstr(char* hash);

/*
 * Compression helpers.
 */

int smache_uncompress(smache_chunk*, void** data, size_t* length);
int smache_compress(smache_chunk*, void** data, size_t* length);
int smache_release(smache_chunk*, void* data);

/*
 * Constructor for a smache instance.
 */

smache* smache_create(void);
int smache_add_backend(smache*, smache_backend*);
void smache_destroy(smache*);

/*
 * Operations for sending/receiving data.
 */
int smache_info(smache*, smache_hash*, size_t* length);
int smache_get(smache*, smache_hash*, size_t offset, void* data, size_t length);
int smache_put(smache*, smache_hash* rval, size_t offset, void* data, size_t length, smache_block_algorithm, smache_compression_type compression);
int smache_delete(smache*, smache_hash*);

/*
 * Useful functions for sending/receiving files.
 */
int smache_getfile(smache*, smache_hash*, const char* filename);
int smache_putfile(smache*, smache_hash*, const char* filename, smache_block_algorithm block, smache_compression_type compression);

/*
 * Option to start serving.
 */
int smache_server(unsigned short port);

#endif /* _SMACHE_H_ */
