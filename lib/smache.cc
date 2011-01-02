//
// smache.cc
//

#include <smache/smache.hh>

Smache::Smache()
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
