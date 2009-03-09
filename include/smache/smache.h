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
 * Data definitions.
 */

typedef void* smache_hash;

typedef struct {
    uint16_t flags;
    uint16_t length;
} __attribute__((packed)) smache_metadata;

typedef void* smache_data;

/*
 * Different hash and block function types.
 */

typedef enum {
    SMACHE_SHA1 = 0
} smache_hash_type;

typedef enum {
    SMACHE_FIXED = 0
} smache_block_type;

/*
 * Definitions for the backend, remote and smache instance.
 */

typedef struct {
    smache_error (*get)(smache_hash*, smache_metadata*, smache_data*);
    smache_error (*put)(smache_hash*, smache_metadata*, smache_data*);
} smache_backend;

typedef struct {
    int socket;
} smache_remote;

typedef struct {
    smache_hash_type  hash_algorithm;
    smache_block_type block_algorithm;

    struct smache_priv* internals;
} smache;

/*
 * Constructors for backends and remotes.
 */

smache_backend* smache_berkeleydb_backend(const char* filename);
smache_backend* smache_memcached_backend(const char* spec);
smache_remote*  smache_socket_remote(int socket);

/*
 * Constructor for a smache instance.
 */

smache* smache_create();
smache_error smache_add_backend(smache*, smache_backend*);
smache_error smache_add_remote(smache*, smache_remote*);

#endif /* _SMACHE_H_ */
