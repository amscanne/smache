//
// smache.cc
//

#include <smache/smache.hh>

Smache::Smache(uint32_t _r, uint32_t _w, uint32_t _n, Chunker* _chunker) : r(_r), w(_w), n(_n), chunker(_chunker)
{
}

Smache::~Smache()
{
}

Hash Smache::dcreate(int fd, uint64_t offset, uint64_t length)
{
    return Hash();
}

Hash Smache::dappend(int fd, uint64_t length)
{
    return Hash();
}

bool Smache::dread(Hash key, int fd, uint64_t offset, uint64_t length)
{
    return false;
}

Hash Smache::dwrite(Hash key, int fd, uint64_t offset, uint64_t length)
{
    return Hash();
}

Hash Smache::dremove(Hash key, uint64_t offset, uint64_t length)
{
    return Hash();
}

Hash Smache::dtruncate(Hash key, uint64_t length)
{
    return Hash();
}

bool Smache::imap(const char* name, Hash key)
{
    return false;
}

bool Smache::iremap(const char* name, Hash key)
{
    return false;
}

bool Smache::iunmap(const char* name)
{
    return false;
}
