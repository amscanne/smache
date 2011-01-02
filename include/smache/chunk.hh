//
// chunk.hh
//

#ifndef _CHUNK_HH_
#define _CHUNK_HH_

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <list>
#include <iostream>

class Chunk;
class Piece;
class DataChunk;
class MetaChunk;
class IndexChunk;

typedef struct {
    /* NOTE: This is always 16 bytes == 128 bits.  */
    /* We only support one kind of hash currently. */
    unsigned char val[16];
#ifndef SWIG
} __attribute__((packed)) hash_t;
#else
} hash_t;
#endif

struct Hashcmp;
class Hash
{
protected:
    hash_t h;
    char*  s;

public:
    Hash() : s(NULL)
    {
        for( int i = 0; i < 16; i++ ) {
            h.val[i] = random() % 256;
        }
    }
    Hash(const char* c) : s(NULL)
    {
        fromstr(c);
    }
    Hash(const Hash& h2) : h(h2.h), s(NULL)
    {
    }
    Hash(const hash_t& val) : h(val), s(NULL)
    {
    }
    ~Hash()
    {
        if( this->s != NULL ) {
            free(this->s);
        }
    }
    hash_t value() const
    {
        return h;
    }
    void fromstr(const char* c)
    {
        for( int i = 0; i < 16 && c[2*i] && c[2*i+1]; i++ ) {
            #define intfromchar(x) ((x) >= 'A' && (x) <= 'F' ? ((x) - 'A' + 10) : (x) - '0')
            h.val[i] = 0;
            h.val[i] |= (intfromchar(c[2*i]) << 4) & 0xf0;
            h.val[i] |= (intfromchar(c[2*i + 1])) & 0x0f;
        }
        if( this->s != NULL ) {
            free(this->s);
        }
    }
    const char* tostr()
    {
        if( this->s == NULL ) {
            this->s = (char*)malloc(33);
            memset(this->s, 0, 33);
            for( int i = 0; i < 16; i++ ) {
                #define inttochar(x) ((x) >= 10 ? 'A' + ((x)-10) : '0' + (x))
                this->s[2*i]   = (inttochar(((this->h.val[i]) >> 4) & 0xf));
                this->s[2*i+1] = (inttochar(this->h.val[i] & 0xf));
            }
        }
        return this->s;
    }
    friend struct Hashcmp;
};

typedef std::list<Hash> HashList;

struct Hashcmp
{
    bool operator()(const Hash& h1, const Hash& h2) const
    {
        return memcmp((const char*)h1.h.val, (const char*)h2.h.val, 16) < 0;
    }
};

#ifndef SWIG
std::ostream& operator<<(std::ostream& out, Hash& h1);
#endif

typedef enum {
    DATA = 0,
    META = 1,
    INDEX = 2    
} chunktype_t;

typedef enum {
    NONE = 0,
    LZO = 1
} compression_t;

typedef struct {
    chunktype_t   chunktype   :3;
    compression_t compression :3;
    uint32_t      reserved    :10;
    uint32_t      references  :16;
    uint32_t      length;
#ifndef SWIG
} __attribute__((packed)) flags_t;
#else
} flags_t;
#endif

class Chunk {
public:
    flags_t flags;
    unsigned char* data;

public:
    Chunk();
    virtual ~Chunk();

    virtual Hash hash();
    void prealloc(uint32_t length);

    uint32_t refs();
    void adjrefs(int delta);

    uint32_t length();
    void set(unsigned char* data, uint32_t length);

    void compress();
    void uncompress();
};

class DataChunk : public Chunk {
public:
    DataChunk();
    virtual ~DataChunk();
};

class Piece {
public:
    Hash hash;
    uint64_t end;
};

class MetaChunk : public Chunk {
public:
    MetaChunk();
    virtual ~MetaChunk();
};

class IndexChunk : public Chunk {
protected:
    unsigned char* key;

public:
    IndexChunk();
    virtual ~IndexChunk();
};

// Simple serialization.
bool load(Chunk* chunk, int fd);
bool save(Chunk* chunk, int fd);

extern const unsigned int MaximumChunkSize;
extern const unsigned int MaximumChunkList;

#endif // _TYPES_HH_
