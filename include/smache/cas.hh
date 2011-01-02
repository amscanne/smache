//
// cas.hh
//

#ifndef _CAS_HH_
#define _CAS_HH_

#include <smache/cas.hh>

class Chunker {
protected:
    uint32_t block;
    uint32_t input;

protected:
    Chunker(uint32_t blocksize);

public:
    virtual Chunk* next(void* data, uint64_t length) = 0;
    uint32_t inputsize();
};

class FixedChunker : public Chunker {
public:
    FixedChunker(uint32_t blocksize);
    virtual Chunk* next(void* data, uint64_t length);
};

class RabinChunker : public Chunker {
protected:
    long long power;
public:
    RabinChunker(uint32_t blocksize);
    virtual Chunk* next(void* data, uint64_t length);
};

#endif // _CAS_HH_ 
