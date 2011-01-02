//
// smache.hh
//

#ifndef _SMACHE_HH_
#define _SMACHE_HH_

class Smache;

#include <smache/chunk.hh>
#include <smache/backend.hh>
#include <smache/record.hh>
#include <smache/store.hh>
#include <smache/worker.hh>
#include <smache/cas.hh>

class Smache {
protected:
    uint32_t r;
    uint32_t w;
    uint32_t n;

    BackendPool backends;
    WorkPool work;
    Chunker* chunker;

public:
    Smache(uint32_t r, uint32_t w, uint32_t n, Chunker* chunker);
    ~Smache();

    // Data manipulation.
    Hash dcreate(int fd, uint64_t offset, uint64_t length);
    Hash dappend(int fd, uint64_t length);
    bool dread(Hash key, int fd, uint64_t offset, uint64_t length);
    Hash dwrite(Hash key, int fd, uint64_t offset, uint64_t length);
    Hash dremove(Hash key, uint64_t offset, uint64_t length);
    Hash dtruncate(Hash key, uint64_t length);

    // Index manipulation.
    bool imap(const char* name, Hash key);
    bool iremap(const char* name, Hash key);
    bool iunmap(const char* name);
};

#endif // _SMACHE_HH_
