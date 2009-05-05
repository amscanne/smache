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
    SMACHE_FIXED = 0,
    SMACHE_RABIN = 1
} smache_block_algorithm;

typedef enum {
    SMACHE_NONE = 0,
    SMACHE_LZO  = 1
} smache_compression_type;

/*
 * Data definitions.
 */

typedef struct {
    /* NOTE: This is always 16 bytes == 128 bits.  */
    /* We only support one kind of hash currently. */
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
    uint16_t references       :12;
    uint16_t length           :16;
    unsigned char data[0];
} __attribute__((packed)) smache_chunk;

smache_chunk* smache_create_chunk(void);
void smache_delete_chunk(smache_chunk*);
/* The largest possible chunk (data size) */
#define SMACHE_MAXIMUM_CHUNKSIZE     (0xffff)
#define SMACHE_METAHASH_BLOCK_LENGTH (512)

typedef struct {
    uint64_t offset;
    uint64_t reserved;
    smache_hash hash;
} smache_metahash;

/*
 * Definitions for the backend, remote and smache instance.
 */

typedef struct struct_smache_backend {
    uint8_t  push     :1;
    uint8_t  debug    :1;
    uint32_t reserved :30;

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
    uint8_t  reserved :6;

    smache_block_algorithm  block_algorithm  :8;
    smache_compression_type compression_type :8;

    uint8_t padding :8;

    struct smache_priv* internals;
} __attribute__((packed)) smache;

static void inline smache_setdebug(smache* instance, int dbg)
{ instance->debug = dbg; }
static void inline smache_setprogress(smache* instance, int onoff)
{ instance->progress = onoff; }
static void inline smache_setblockalgorithm(smache* instance, smache_block_algorithm algorithm)
{ instance->block_algorithm = algorithm; }
static void inline smache_setcompressiontype(smache* instance, smache_compression_type compression)
{ instance->compression_type = compression; }

void smache_null_check(smache* instance);
void smache_hash_null_check(smache_hash* instance);

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

struct chunklist
{
    void*  data;
    size_t length;
    struct chunklist* next;
};

struct chunklist* smache_chunk_fixed(void* data, size_t length, size_t* count, size_t block_length);
struct chunklist* smache_chunk_rabin(void* data, size_t length, size_t* count, size_t block_length);

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
int smache_info(smache*, smache_hash*,
                unsigned int* metahash, smache_compression_type* compression,
                uint64_t* length, uint64_t* totallength,
                size_t* references);

int smache_adjref(smache* instance, smache_hash* hash, int references);
int smache_get(smache*, smache_hash*, uint64_t offset, void* data, uint64_t* length);
int smache_put(smache*, smache_hash* rval, void* data, uint64_t length, size_t block_size);
int smache_delete(smache*, smache_hash*);

/*
 * Convenience functions, do not use special types (for python bindings...).
 */
unsigned int smache_info_length(smache*, smache_hash*);
unsigned int smache_info_totallength(smache*, smache_hash*);
unsigned int smache_info_references(smache*, smache_hash*);
unsigned int smache_info_metahash(smache*, smache_hash*);
unsigned int smache_info_compression(smache*, smache_hash*);

/*
 * Useful functions for sending/receiving files.
 */
int smache_getfile(smache*, smache_hash*, const char* filename);
int smache_putfile(smache*, smache_hash*, const char* filename, size_t block_length);

/*
 * Option to start serving.
 */
int smache_server(unsigned short port);

#endif /* _SMACHE_H_ */
