//
// store.hh
//

#ifndef _STORE_HH_
#define _STORE_HH_

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include <iostream>
#include <vector>
#include <list>
#include <map>

class AutoFile;
class IndexFile;
class DataFile;
class Store;

#include <smache/locks.hh>
#include <smache/record.hh>
#include <smache/backend.hh>

class AutoFile : protected Safe {
private:
    char* filename;
    std::vector<int> fds;

protected:
    uint32_t use;
    uint64_t size;
    void* map;

protected:
    AutoFile(const char* directory, const char* file);
    int getfd();
    void relfd(int fd);
    void remap();
    void setsize(uint64_t size);
    ~AutoFile();

private:
    void closefds();

public:
    void dump(std::ostream& out);
};

#ifndef SWIG
std::ostream& operator<<(std::ostream& out, AutoFile& af);
#endif

class IndexFile : protected AutoFile {
protected:
    std::vector<uint64_t> free;

public:
    IndexFile(const char* directory);
    ~IndexFile();

    record_t* get(uint64_t recnum, bool inactive);
    uint64_t create();
    uint64_t records() const;
    void done(uint64_t recnum);
    void drop(uint64_t recnum);

    void dump(std::ostream& out);
};

#ifndef SWIG
std::ostream& operator<<(std::ostream& out, IndexFile& i);
#endif

class DataFile : protected AutoFile {
private:
    typedef std::pair<uint64_t, uint32_t> free_t;
    typedef std::list<free_t> freelist_t;
    freelist_t free;

public:
    DataFile(const char* directory);
    ~DataFile();

public:
    void mark(uint64_t offset, uint32_t length);
    uint64_t find(uint32_t length);
    void release(uint64_t offset, uint32_t length);

    void load(unsigned char* sink, uint64_t offset, uint32_t length);
    void save(unsigned char* source, uint64_t offset, uint32_t length);

    void dump(std::ostream& out);
};

#ifndef SWIG
std::ostream& operator<<(std::ostream& out, DataFile& df);
#endif

class Store : public Backend {
private:
    IndexFile index;
    DataFile  data;

    typedef std::map<Hash, uint64_t, Hashcmp> records_t;
    records_t records;

    uint32_t n_data;
    uint32_t n_meta;
    uint32_t n_index;

    void count(record_t* rec);
    void uncount(record_t* rec);

public:
    Store(const char* directory);

    virtual Chunk* get(const Hash& key);
    virtual Chunk* head(const Hash& key);
    virtual bool add(Chunk* chunk);
    virtual bool remove(const Hash& key);
    virtual bool adjrefs(const Hash& key, int delta);

    virtual HashList* fetchAll(uint64_t start, uint32_t count);
    virtual HashList* fetchData(uint64_t start, uint32_t count);
    virtual HashList* fetchMeta(uint64_t start, uint32_t count);
    virtual HashList* fetchIndex(uint64_t start, uint32_t count);
    virtual uint64_t countAll();
    virtual uint64_t countData();
    virtual uint64_t countMeta();
    virtual uint64_t countIndex();

    void dump(std::ostream& out);
};

#ifndef SWIG
std::ostream& operator<<(std::ostream& out, Store& s);
#endif

#endif // _STORE_HH_
