//
// cas.cc
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mhash.h>
#include <stdint.h>

#include <smache/chunk.hh>
#include <smache/cas.hh>

#define PRIME_BASE  257
#define PRIME_MOD   1000000007
#define WINDOW_SIZE 32

#define BLOCK_MIN (sizeof(hash_t))
#define BLOCK_MAX (MaximumChunkSize)

Chunker::Chunker(uint32_t blocksize) : block(blocksize), input(blocksize)
{
}

uint32_t Chunker::inputsize() 
{
    return this->input;
}

FixedChunker::FixedChunker(uint32_t blocksize) : Chunker(blocksize)
{
    input = this->block < BLOCK_MAX ? this->block : BLOCK_MAX;
    input = this->block > BLOCK_MIN ? this->block : BLOCK_MIN;
}

RabinChunker::RabinChunker(uint32_t blocksize) : Chunker(blocksize), power(1)
{
    // Precompute the power.
    int i = 0;
    for (i = 0; i <= WINDOW_SIZE; i++)
    {
        power = power*PRIME_BASE;
        power %= PRIME_MOD;
    }
    // Save the maximum input.
    input = MaximumChunkSize;
}

Chunk* FixedChunker::next(void* data, uint64_t length)
{
    Chunk* chunk = new DataChunk();
    chunk->flags.length = this->block < length ? this->block : length;
    chunk->data = (unsigned char*)data;
    return chunk;
}

Chunk* RabinChunker::next(void* data, uint64_t length)
{
    long long hash = 0;
    uint32_t i = 0;
    for (; i < length; i++)
    {
        // Add the next character to the window.
        hash = hash*PRIME_BASE + ((char*)data)[i];
        hash %= PRIME_MOD;

        // Remove the first character if necessary.
        if ( i > WINDOW_SIZE )
        {
            hash = (hash - (power * ((char*)data)[i-WINDOW_SIZE-1])) % PRIME_MOD;
        }

        // Check for cut point.
        if ( ((hash % this->block == 0) && (i >= BLOCK_MIN)) ||
             (i >= BLOCK_MAX) )
        {
            break;
        }
    }

    Chunk* chunk = new DataChunk();
    chunk->flags.length = (i+1);
    chunk->data = (unsigned char*)data;
    return chunk;
}

//
// The below will be integrated as necessary.
//

#if 0
/*
 * smache.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <smache/smache.h>

/*
 * Constructor and modifiers for a smache instance.
 */

smache*
smache_create(void)
{
    /*
     * Allocate the instance.
     */
    smache* instance = malloc(sizeof(smache));
    if( instance == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        return NULL;
    }
    memset(instance, 0, sizeof(*instance));
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Created smache instance (%p).\n", instance);

    return instance;
}

void
smache_destroy(smache* instance)
{
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Destroying instance...\n");
    free(instance);
}

int
smache_read(
smache* instance,
smache_chunk* chunk,
void* data,
uint64_t offset,
uint64_t length)
{
    if( chunk->metahash ) {
        fprintf(stderr, "smache_read: chunk is a metahash\n");
        return SMACHE_ERROR;
    } else if( offset + length > chunk->length ) {
        fprintf(stderr, "smache_read: read too long\n");
        return SMACHE_ERROR;
    }

    /*
     * Uncompress the chunk.
     */
    void* uncompressed;
    size_t uncompressed_length;
    if( smache_uncompress(chunk, &uncompressed, &uncompressed_length) != SMACHE_SUCCESS )
    {
        fprintf(stderr, "smache_uncompress: %s\n", strerror(errno));
        return SMACHE_ERROR;
    }
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Uncompressed size is %ld.\n", (long)uncompressed_length);

    /*
     * Copy the data out.
     */
    memcpy(data, ((char*)uncompressed) + offset, length);

    smache_release(chunk, uncompressed);
    return SMACHE_SUCCESS;
} 

static int
_smache_get_metahash(
smache* instance,
smache_chunk* chunk,
uint64_t offset,
uint64_t length,
smache_hashlist* hashlist)
{
    /*
     * Uncompress the chunk.
     */
    void* uncompressed;
    size_t uncompressed_length;
    if( smache_uncompress(chunk, &uncompressed, &uncompressed_length) != SMACHE_SUCCESS )
    {
        fprintf(stderr, "smache_uncompress: %s\n", strerror(errno));
        return SMACHE_ERROR;
    }
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Uncompressed size is %ld.\n", (long)uncompressed_length);

    smache_metahash* metahash = uncompressed;
    int count = uncompressed_length / sizeof(smache_metahash);
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Chunk is a metahash with %d entries.\n", count);

    /*
     * Grab the appropriate underlying chunks.
     */
    uint64_t end = offset + length;
    int index = 0;
    for( index = 0; index < count; index++ )
    {
        /*
         * Metahashes record the ending offset.
         */
        uint64_t mh_start = 0;
        uint64_t mh_end   = metahash[index].offset;
        if( index > 0 ) { mh_start = metahash[index-1].offset; }

        SMACHE_DEBUG(instance, LEVEL_DEBUG, "Metahash %d spans [%ld,%ld] looking for [%ld,%ld].\n",
                                            index, mh_start, mh_end, offset, end);

        /* Check for completely after this metahash. */
        if( mh_end < offset ) {
            break;
        } else if( mh_start < (offset + length) ) {
            continue;
        }

        uint64_t local_offset = 0;
        if( mh_start < offset ) { local_offset = (offset - mh_start); }
        uint64_t local_length = (mh_end - mh_start) - local_offset;
        if( mh_end > end ) { local_length -= (mh_end - end); }

        /* Append this piece to the list. */
        smache_append_hashlist(hashlist, smache_copy_hash(&(metahash[index].hash)), local_offset, local_length);
    }

    smache_release(chunk, uncompressed);
    return SMACHE_SUCCESS;
}

smache_hashlist*
smache_get_metahash(
smache* instance,
smache_chunk* chunk,
uint64_t offset,
uint64_t length)
{
    if( !chunk->metahash ) {
        fprintf(stderr, "smache_get_metahash: chunk is not a metahash\n");
        return NULL;
    }

    smache_hashlist* hashlist = smache_create_hashlist();
    _smache_get_metahash(instance, chunk, offset, length, hashlist);
    return hashlist;
}

int
smache_totallength(
smache_chunk* chunk,
uint64_t* totallength)
{
    if( !chunk->metahash ) {
        *totallength = chunk->length;
    } else {
        /*
         * Uncompress the chunk.
         */
        void* uncompressed;
        size_t uncompressed_length;
        if( smache_uncompress(chunk, &uncompressed, &uncompressed_length) != SMACHE_SUCCESS )
        {
            fprintf(stderr, "smache_uncompress: %s\n", strerror(errno));
            return SMACHE_ERROR;
        }

        smache_metahash* metahash = uncompressed;
        int count = uncompressed_length / sizeof(smache_metahash);
        *totallength = metahash[count-1].offset;
        smache_release(chunk, uncompressed);
    }

    return SMACHE_SUCCESS;
}

int
smache_info(
smache_chunk* chunk,
unsigned int* metahash,
smache_compression_type* compression,
uint64_t* length,
uint64_t* totallength,
size_t* references)
{
    if( metahash )    *metahash    = chunk->metahash;
    if( compression ) *compression = chunk->compression_type;
    if( length )      *length      = chunk->length;
    if( totallength ) smache_totallength(chunk, totallength);
    if( references )  *references  = chunk->references;
    return SMACHE_SUCCESS;
}

static int
_smache_compute(
smache* instance,
smache_hash* hash,
void* data,
uint64_t length,
size_t block_length,
int depth,
smache_chunklist* chunklist)
{
    int rval = SMACHE_SUCCESS;
    float perc = 0;

    /*
     * Create a re-use a single chunk for all operations.
     */
    char scratch_data[sizeof(smache_chunk) + SMACHE_MAXIMUM_CHUNKSIZE];
    smache_chunk* chunk = (smache_chunk*)(&scratch_data);

    /*
     * Get the chunklist.
     */
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Creating chunklist for data of length %ld.\n", (long)length);

    struct chunklist* first_chunk = NULL;
    struct chunklist* chunks      = NULL;
    size_t count = 0;
    if( depth == 0 )
    {
        switch( instance->block_algorithm )
        {
            case SMACHE_FIXED:
            SMACHE_DEBUG(instance, LEVEL_DEBUG, "Using fixed algorithm.\n");
            first_chunk = smache_chunk_fixed(data, length, &count, block_length);
            break;

            case SMACHE_RABIN:
            SMACHE_DEBUG(instance, LEVEL_DEBUG, "Using Rabin-Karp rolling hashes.\n");
            first_chunk = smache_chunk_rabin(data, length, &count, block_length);
            break;

            default:
            fprintf(stderr, "put: Unknown block algorithm %d.\n", instance->block_algorithm);
            return SMACHE_ERROR;
        }
    }
    else
    {
        SMACHE_DEBUG(instance, LEVEL_DEBUG, "Using fixed algorithm (metahashes).\n");
        first_chunk = smache_chunk_fixed(data, length, &count, SMACHE_METAHASH_BLOCK_LENGTH);
    }
    SMACHE_DEBUG(instance, LEVEL_DEBUG, "Created %ld chunks.\n", (long)count);
    chunks = first_chunk;

    /*
     * Allocate space for the metahashes.
     * NOTE: We do everything here, then recursively shrink the metahashes.
     */
    smache_metahash* metahashes = malloc(count * sizeof(smache_metahash));
    int hashno      = 0;
    uint64_t offset = 0;
    for( hashno = 0; hashno < count; hashno++ )
    {
        /*
         * Compute the new percentage done.
         */
        float newperc = (float)((char*)chunks->data - (char*)data) * 100.0 / length;
        if( (int)newperc > (int)perc || hashno % 10000 == 0 )
        {
            perc = newperc;
            if( instance->progress ) fprintf(stderr, "%3.2f%% finished.\r", newperc);
        }

        /*
         * Figure out the offset (for the metahashes), and correct all underlying metahashes.
         */
        if( depth > 0 )
        {
            smache_metahash* underlyingmetahash = &((smache_metahash*)chunks->data)[(chunks->length / sizeof(smache_metahash)) - 1];
            offset = underlyingmetahash->offset;
        }
        if( depth == 0 )
        {
            offset += chunks->length;
        }

        metahashes[hashno].offset = offset;

        if( depth > 0 && hashno > 0 )
        {
            uint64_t lasthashoffset = metahashes[hashno-1].offset;
            int i = 0;
            for( i = 0; i < (chunks->length / sizeof(smache_metahash)); i++ )
            {
                ((smache_metahash*)chunks->data)[i].offset -= lasthashoffset;
            }
        }
        if( depth > 0 )
        {
            SMACHE_DEBUG(instance, LEVEL_DEBUG, "Underlying data offets:\n");
            int i = 0;
            for( i = 0; i < (chunks->length / sizeof(smache_metahash)); i++ )
            {
                SMACHE_DEBUG(instance, LEVEL_DEBUG, " %d: %ld\n", i, (long)((smache_metahash*)chunks->data)[i].offset);
            }
        }

        /*
         * Compute the data hash.
         */
        SMACHE_DEBUG(instance, LEVEL_DEBUG, "Computing the hash for data at offset %ld.\n", (long)((char*)chunks->data - (char*)data));
        smache_computehash(hash, chunks->data, chunks->length);
        metahashes[hashno].hash = *hash;

        /*
         * Create the compressed block. (And mark it as a metahash).
         */
        SMACHE_DEBUG(instance, LEVEL_DEBUG, "Original data length is %ld.\n", (long)chunks->length);
        smache_compress(chunk, chunks->data, chunks->length, instance->compression_type);
        chunk->references = 0;
        chunk->metahash   = (depth > 0);
        SMACHE_DEBUG(instance, LEVEL_DEBUG, "Compressed data length is %ld.\n", (long)chunk->length);

        /*
         * Add the block to our chunklist.
         */
        smache_append_chunklist(chunklist, chunk);

        /*
         * Grab the next chunk.
         */
        struct chunklist* prev = chunks;
        chunks = chunks->next;
        free(prev);
    }

    if( instance->progress )
    {
        fprintf(stderr, "100.00%% finished.\n");
    }

    if( count > 1 )
    {
        /* Recursively compute the hash of the metahash. */
        rval |= _smache_compute(instance, hash, metahashes, count * sizeof(smache_metahash), block_length, depth + 1, chunklist);
    }
    else
    {
        /* The one and only hash computed was appropriate, we just don't touch the hash variable.  */
    }

    /*
     * Free the allocated metahashes.
     */
    free(metahashes);
    free(chunk);

    return rval;
}

smache_chunklist*
smache_compute(smache* instance, smache_hash* hash, void* data, uint64_t length, size_t block_length)
{
    smache_chunklist* chunklist = smache_create_chunklist();
    _smache_compute(instance, hash, data, length, block_length, 0, chunklist);
    return chunklist;
}

#endif
