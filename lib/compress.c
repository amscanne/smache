/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lzo/lzo1f.h>
#include <smache/smache.h>

static void* lzo_working_mem = NULL;
static void* lzo_data_chunk       = NULL;

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
    if( chunk->compression_type == SMACHE_NONE )
    {
        /*
         * Just copy out a pointer to the data.
         */
        *data   = chunk->data;
        *length = chunk->length; 
    }
    else if( chunk->compression_type == SMACHE_LZO )
    {
        /*
         * Decompress to the scratch data chunk.
         * NOTE: This will set the length automatically.
         */
        fprintf(stderr, "Uncompressing chunk of length %d from %p to %p.\n", chunk->length, chunk->data, lzo_data_chunk);
        *length = SMACHE_MAXIMUM_CHUNKSIZE*2;
        lzo1f_decompress( chunk->data, chunk->length, lzo_data_chunk, length, lzo_working_mem);
        *data = lzo_data_chunk;
    }
    else
    {
        fprintf(stderr, "compression: Unsupported type.\n");
        return SMACHE_ERROR;
    }

    return SMACHE_SUCCESS;
}

int
smache_compress(smache_chunk* chunk, void* data, size_t length, smache_compression_type compression)
{
    if( compression == SMACHE_LZO )
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
    }
    else
    {
        fprintf(stderr, "warning: Unsupported compression type.\n");

        /*
         * Simply copy in the data.
         */
        memcpy(chunk->data, data, length);
        chunk->length = length;
        chunk->compression_type = SMACHE_NONE;
    }

    return SMACHE_SUCCESS;
}

int
smache_release(smache_chunk* chunk, void* data)
{
    return SMACHE_SUCCESS;
}
