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

typedef enum {
    SMACHE_SUCCESS = 0,
    SMACHE_NOEXIST = 1,    /* Key does not exist. */
    SMACHE_NOSUPP  = 2,    /* Parameter not supported. */
    SMACHE_ERROR   = 255   /* Unknown/other error. (Check errno) */
} smache_error;

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

typedef struct {
    uint32_t    offset;
    smache_hash hash;
} smache_metahash;

#define SMACHE_MAXIMUM_CHUNKSIZE  (0xffff)
#define SMACHE_METAHASH_CHUNKSIZE (SMACHE_MAXIMUM_CHUNKSIZE)
#define SMACHE_METAHASH_COUNT     (SMACHE_METAHASH_CHUNKSIZE >> 3)

/*
 * Definitions for the backend, remote and smache instance.
 */

typedef struct struct_smache_backend {
    uint8_t  push     :1;
    uint32_t reserved :31;

    smache_error (*get)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    smache_error (*put)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    smache_error (*delete)(struct struct_smache_backend*, smache_hash*);
    smache_error (*close)(struct struct_smache_backend*);

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
 * Constructor for a smache instance.
 */

smache* smache_create();
smache_error smache_add_backend(smache*, smache_backend*);
void smache_destroy(smache*);

/*
 * Operations for sending/receiving data.
 */
smache_error smache_info(smache*, smache_hash*, size_t* length);
smache_error smache_get(smache*, smache_hash*, size_t offset, void* data, size_t length);
smache_error smache_put(smache*, smache_hash* rval, void* data, size_t length, smache_block_algorithm, smache_compression_type compression);
smache_error smache_delete(smache*, smache_hash*);

/*
 * Option to start serving.
 */
smache_error smache_server(unsigned short port);

#endif /* _SMACHE_H_ */
