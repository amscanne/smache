//
// chunk.cc
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mhash.h>

#include <smache/chunk.hh>

Chunk::Chunk() : data(NULL)
{
    this->flags.reserved    = 0;
    this->flags.compression = NONE;
    this->flags.references  = 1;
}

uint32_t Chunk::refs()
{
    return this->flags.references;
}

void Chunk::adjrefs(int delta)
{
    this->flags.references += delta;
}

uint32_t Chunk::length()
{
    return this->flags.length;
}

void Chunk::set(unsigned char* indata, uint32_t length)
{
    this->flags.length = length;
    this->prealloc(length);
    memcpy(this->data, indata, length);
}
    
Chunk::~Chunk()
{
    if( this->data != NULL ) {
        delete this->data;
    }
}

DataChunk::DataChunk()
{
    flags.chunktype = DATA;
}

DataChunk::~DataChunk()
{
}

MetaChunk::MetaChunk()
{
    flags.chunktype = META;
}

MetaChunk::~MetaChunk()
{
}

IndexChunk::IndexChunk()
{
    flags.chunktype = INDEX;
}

IndexChunk::~IndexChunk()
{
}

void Chunk::prealloc(uint32_t length) 
{
    this->data = (unsigned char*)realloc(this->data, length); 
}

Hash Chunk::hash()
{
    hash_t result;

    // Initialize the hash context.
    MHASH td;
    td = mhash_init(MHASH_MD5);
    if( td == MHASH_FAILED || mhash_get_block_size(MHASH_MD5) != 16 )
    {
        fprintf(stderr, "mhash: Failed to initialize.\n");
        return result;
    }

    // Perform the hash.
    mhash(td, (unsigned char*)this->data, this->flags.length);

    // Save the hash result.
    unsigned char* val = (unsigned char*)mhash_end(td);
    memcpy(&result, val, sizeof(result));

    return Hash(result);
}

void Chunk::compress()
{
}

void Chunk::uncompress()
{
}

std::ostream& operator<<(std::ostream& out, Hash& h1)
{
    out << h1.tostr();
    return out;
}

bool load(Chunk* chunk, int fd)
{
    // Make sure we have enough space.
    chunk->prealloc(chunk->flags.length);
    if( read(fd, chunk->data, chunk->flags.length) < 0 ) {
        return false;
    }
    return true;
}

bool save(Chunk* chunk, int fd)
{
    // Write out just the data.
    if( write(fd, chunk->data, chunk->flags.length) < 0 ) {
        return false;
    }
    return true;
}

const unsigned int MaximumChunkSize = (0xffff);
const unsigned int MaximumChunkList = (MaximumChunkSize / sizeof(Piece));

//
// The following code will be worked in to the above
// compress routines (and executed automatically).
//

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lzo/lzo1f.h>
#include <smache/smache.h>

static void* lzo_working_mem = NULL;
static void* lzo_data_chunk  = NULL;

void
__attribute__((constructor))
smache_compression_init()
{
    if (lzo_init() != LZO_E_OK)
    {
        return;
    }

    lzo_working_mem = (unsigned char*)malloc(LZO1F_999_MEM_COMPRESS);
    lzo_data_chunk  = (void*)malloc(SMACHE_MAXIMUM_CHUNKSIZE*2);
}

void
__attribute__((destructor))
smache_compression_fini()
{
    if( lzo_working_mem )
    {
        free(lzo_working_mem);
    }
}

int
smache_uncompress(smache_chunk* chunk, void** data, size_t* length)
{
    switch( chunk->compression_type )
    {
        case SMACHE_NONE:
        {
            /*
             * Just copy out a pointer to the data.
             */
            *data   = chunk->data;
            *length = chunk->length; 
            break;
        }

        case SMACHE_LZO:
        {
            /*
             * Decompress to the scratch data chunk.
             * NOTE: This will set the length automatically.
             */
            *length = SMACHE_MAXIMUM_CHUNKSIZE*2;
            lzo1f_decompress( chunk->data, chunk->length, lzo_data_chunk, length, lzo_working_mem);
            *data = lzo_data_chunk;
            break;
        }

        default:
        {
            fprintf(stderr, "compression: Unsupported compression type (%d).\n", chunk->compression_type);
            return SMACHE_ERROR;
        }
    }

    return SMACHE_SUCCESS;
}

int
smache_compress(smache_chunk* chunk, void* data, size_t length, smache_compression_type compression)
{
    switch( compression )
    {
        case SMACHE_LZO:
        {
            size_t compressed_length = SMACHE_MAXIMUM_CHUNKSIZE * 2;
            lzo1f_999_compress( data, length, lzo_data_chunk, &compressed_length, (lzo_voidp)lzo_working_mem);
            if( compressed_length > length )
            {
                return smache_compress(chunk, data, length, SMACHE_NONE);
            }

            /*
             * Copy the compressed data in.
             */
            memcpy(chunk->data, lzo_data_chunk, compressed_length);
            chunk->length = compressed_length;
            chunk->compression_type = SMACHE_LZO;
            break;
        }

        default:
        {
            if( compression != SMACHE_NONE )
            {
                fprintf(stderr, "compression: Unsupported compression type (%d).\n", compression);
            }

            /*
             * Simply copy in the data.
             */
            memcpy(chunk->data, data, length);
            chunk->length = length;
            chunk->compression_type = SMACHE_NONE;
            break;
        }
    }

    return SMACHE_SUCCESS;
}

int
smache_release(smache_chunk* chunk, void* data)
{
    return SMACHE_SUCCESS;
}

#endif
