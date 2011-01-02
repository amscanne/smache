//
// remote.hh
//

#ifndef _REMOTE_HH_
#define _REMOTE_HH_

#include <smache/backend.hh>

class Remote {
public:
    virtual Chunk* get(hash_t key);
    virtual bool add(Chunk* chunk);
    virtual bool remove(Chunk* chunk);
    virtual bool adjrefs(Chunk* chunk, int delta);
};

#endif // _REMOTE_HH_
