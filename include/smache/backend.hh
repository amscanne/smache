//
// backend.hh
//

#ifndef _BACKEND_HH_
#define _BACKEND_HH_

#include <list>
#include <map>

class Backend;
class BackendPool;

#include <smache/chunk.hh>

class Backend {
public:
    virtual Chunk* get(const Hash& key) = 0;
    virtual Chunk* head(const Hash& key) = 0;
    virtual bool add(Chunk* chunk) = 0;
    virtual bool remove(const Hash& key) = 0;
    virtual bool adjrefs(const Hash& key, int delta) = 0;

    virtual HashList* fetchAll(uint64_t start, uint32_t count) = 0;
    virtual HashList* fetchData(uint64_t start, uint32_t count) = 0;
    virtual HashList* fetchMeta(uint64_t start, uint32_t count) = 0;
    virtual HashList* fetchIndex(uint64_t start, uint32_t count) = 0;
    virtual uint64_t countAll() = 0;
    virtual uint64_t countData() = 0;
    virtual uint64_t countMeta() = 0;
    virtual uint64_t countIndex() = 0;
};
typedef std::list<Backend*> BackendList;

class BackendPool {
protected:
    typedef std::map<Hash, Backend*, Hashcmp> buckets_t;
    buckets_t buckets;

public:
    void add(Hash bucket, Backend* backend)
    {
        this->buckets[bucket] = backend;
    }

    BackendList* closest(Hash key, unsigned int n)
    {
        BackendList* rval = new BackendList();
        buckets_t::const_iterator it = this->buckets.lower_bound(key);

        // Wrap around if necessary.
        if( it == this->buckets.end() ) {
            it = this->buckets.begin();
        }

        // Make sure there is at least one key.
        if( it == this->buckets.end() ) {
            return NULL;
        }

        // Sweep around.
        while( n > 0 ) {
            it++;
            if( it == this->buckets.end() ) {
                it = this->buckets.begin();
            }
            rval->push_back(it->second);
            n--;
        }

        return rval;
    }

    void remove(Hash bucket)
    {
        buckets_t::iterator it = this->buckets.find(bucket);
        if( it != this->buckets.end() ) {
            this->buckets.erase(it); 
        }
    }
};

#endif // _BACKEND_HH_
