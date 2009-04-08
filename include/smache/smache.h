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
     * of hash objects and 64-bit offsets.
     * Each hash has a constant, fixed size.
     */
    uint8_t  metahash         :1;
    uint8_t  compression_type :3;
    uint16_t reserved         :12;
    uint16_t length           :16;
    unsigned char data[0];
} __attribute__((packed)) smache_chunk;

smache_chunk* smache_create_chunk(void);
void smache_delete_chunk(smache_chunk*);

typedef struct {
    uint32_t    offset;
    uint32_t    padding1;
    uint32_t    padding2;
    uint32_t    padding3;
    smache_hash hash;
} smache_metahash;

#define SMACHE_MAXIMUM_CHUNKSIZE  (0xffff)
#define SMACHE_METAHASH_COUNT(l)  (l >> 3)
#define SMACHE_METAHASH_DEFAULT   0x80

/*
 * Definitions for the backend, remote and smache instance.
 */

typedef struct struct_smache_backend {
    uint8_t  push     :1;
    uint8_t  debug    :1;
    uint32_t reserved :30;

    int (*exists)(struct struct_smache_backend*, smache_hash*);
    int (*get)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    int (*put)(struct struct_smache_backend*, smache_hash*, smache_chunk*);
    int (*delete)(struct struct_smache_backend*, smache_hash*);
    int (*close)(struct struct_smache_backend*);

    void* internals;

} __attribute__((packed)) smache_backend;

static void inline smache_backend_setpush(smache_backend* backend, int push)
{ backend->push  = push; }
static void inline smache_backend_setdebug(smache_backend* backend, int dbg)
{ backend->debug = dbg; }

typedef struct {
    uint8_t  debug    :1;
    uint8_t  progress :1;
    uint32_t reserved :30;

    struct smache_priv* internals;
} __attribute__((packed)) smache;

static void inline smache_setdebug(smache* instance, int dbg)
{ instance->debug = dbg; }
static void inline smache_setprogress(smache* instance, int onoff)
{ instance->progress = onoff; }

#define stringify2(x) #x
#define stringify(x) stringify2(x)
#define SMACHE_DEBUG(x,s,a...)    if( x->debug ) { fprintf(stderr, __FILE__ "." stringify(__LINE__) ": " s,  ## a); }
#define SMACHE_DEBUG_NL(x,s,a...) if( x->debug ) { fprintf(stderr, s,  ## a); }

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
char* smache_temp_hashstr(smache_hash*);
int smache_write_hashstr(smache_hash*, char*);
void smache_delete_hashstr(char* hash);

/*
 * Compression helpers.
 */

int smache_uncompress(smache_chunk*, void** data, size_t* length);
int smache_compress(smache_chunk*, void* data, size_t length, smache_compression_type compression);
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
int smache_put(smache*, smache_hash* rval, void* data, size_t length, smache_block_algorithm, smache_compression_type compression);
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
