//
// local.cc
//

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <iostream>

#include <smache/log.hh>
#include <smache/store.hh>

#define PAGESIZE      4096
#define ROUNDUP(x,y) (y*((x + (y-1))/y))
#define INDEXGROWTH   1024

AutoFile::AutoFile(const char* directory, const char* file) :
    filename(NULL),
    fds(),
    use(0),
    size(0),
    map(NULL)
{
    filename = (char*)malloc(sizeof(char) * strlen(directory) + 1 + strlen(file) + 1);
    sprintf(filename, "%s/%s", directory, file);
    int fd = getfd();
    this->size = lseek(fd, 0L, SEEK_END);
    relfd(fd);
    remap();
}

void AutoFile::remap()
{
    this->acquire();
    if( this->size > 0 ) {
        int fd = getfd();
        this->map = mmap(NULL, ROUNDUP(this->size, PAGESIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        relfd(fd);
    }
    this->release();
}

void AutoFile::setsize(uint64_t size)
{
    if( this->map != NULL ) {
        munmap(this->map, this->size);
        this->map = NULL;
    }
    this->acquire();
    this->closefds();
    int fd = this->getfd();
    ftruncate(fd, ROUNDUP(size, PAGESIZE));
    this->size = size;
    remap();
    relfd(fd);
    this->release();
}

int AutoFile::getfd() 
{
    int fd = -1;
    this->acquire();
    while( this->fds.size() == 0 ) {
        // Try to open a new one.
        fd = open(this->filename, O_CREAT | O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR);
        if( fd == -1 ) {
            this->wait();
        } else {
            this->fds.push_back(fd);
        }
    }
    fd = this->fds.back();
    this->fds.pop_back();
    this->use++;
    this->release();
    return fd;
}

void AutoFile::relfd(int fd)
{
    this->acquire();
    this->use--;
    this->fds.push_back(fd);
    this->broadcast();
    this->release();
}

AutoFile::~AutoFile()
{
    this->closefds();
}

void AutoFile::closefds()
{
    this->acquire();
    while( this->use > 0 ) {
        this->wait();
    }
    if( this->map != NULL ) {
        munmap(this->map, this->size);
        this->map = NULL;
    }
    while( this->fds.size() > 0 ) {
        close(this->fds.back());
        this->fds.pop_back();
    }
    this->release();
}

void AutoFile::dump(std::ostream& out)
{
    out << "[filename=" << this->filename
        << ",fds=" << this->fds.size()
        << ",size=" << this->size
        << ",map=" << this->map << "]";
}

IndexFile::IndexFile(const char* datadir) :
    AutoFile(datadir, "index")
{
    if( this->size < sizeof(header_t) ) {
        this->setsize(sizeof(header_t));
        header_t* header = (header_t*)this->map;
        header->magic[0] = '3';
        header->magic[1] = '7';
        header->magic[2] = '0';
        header->magic[3] = '4';
        header->magic[4] = '7';
        header->magic[5] = '7';
        header->magic[6] = '3';
        header->magic[7] = '4';
        header->version  = 1;
    }

    // Check the header.
    remap();
    header_t* header = (header_t*)this->map;
    if( strcmp((const char*)header->magic, "37047734") ||
        header->version != 1 ) {
        // Version mismatch.  Raise exception.
    }

    // Load existing records.
    uint64_t max = this->records();
    for( uint64_t i = 0; i < max; i++ ) {
        progressShow(LEVEL_INFO, "Loading index", i, max);
        record_t* rec = this->get(i, false);
        if( rec == NULL ) {
            this->free.push_back(i);
        } else {
            this->done(i);
        }
    }
    progressDone(LEVEL_INFO, "Loading index");
}

IndexFile::~IndexFile()
{
}

record_t* IndexFile::get(uint64_t num, bool inactive)
{
    record_t* res = NULL;
    this->acquire();
    res = (record_t*)(((unsigned char*)this->map) + sizeof(header_t) + (sizeof(record_t) * num));
    if( res->active || inactive ) {
        this->use++;
    } else {
        res = NULL;
    }
    this->release();
    return res;
}

void IndexFile::done(uint64_t recnum)
{
    this->acquire();
    this->use--;
    this->broadcast();
    this->release();
}

uint64_t IndexFile::create()
{
    uint64_t recnum = 0;
    this->acquire();
    if( this->free.size() > 0 ) {
        recnum = this->free.back();
        this->free.pop_back();
    } else {
        recnum = this->records();
        this->setsize(this->size + INDEXGROWTH * sizeof(record_t));
        for( uint64_t freenum = recnum + 1; freenum < this->records(); freenum++ ) {
            this->free.push_back(freenum);
        }
    }
    record_t* rec = this->get(recnum, true);
    rec->active = 1;
    this->done(recnum);
    this->release();
    return recnum;
}

void IndexFile::drop(uint64_t num)
{
    record_t* record = this->get(num, false);
    if( record == NULL ) {
        return;
    }
    record->active = 0;
    this->acquire();
    this->free.push_back(num);
    this->release();
    this->done(num);
}

uint64_t IndexFile::records() const
{
    return (this->size - sizeof(header_t)) / sizeof(record_t);
}

void IndexFile::dump(std::ostream& out)
{
    out << " index[records=" << this->records() << ",free=" << this->free.size() << "]" << std::endl;
    out << "  auto"; AutoFile::dump(out); out << std::endl;
    for( uint64_t i = 0; i < this->records(); i++ ) {
        record_t* record = this->get(i, true);
        if( record == NULL ) {
            continue;
        }
        Hash h(record->hash);
        out << "    record[active=" << record->active
                      << ",hash=" << h
                      << ",offset=" << record->offset
                      << ",length=" << record->flags.length 
                      << "]" << std::endl;
        this->done(i);
    }
}

DataFile::DataFile(const char* datadir) : 
    AutoFile(datadir, "data")
{
    this->free.push_back(free_t(0, this->size));
}

DataFile::~DataFile()
{
}

void DataFile::mark(uint64_t offset, uint32_t length)
{
    for( freelist_t::iterator it = this->free.begin();
         it != this->free.end();
         it++ ) {
        // Whole thing within the section?
        if( offset <= it->first && (it->first + it->second) <= (offset + length) ) {
            it = this->free.erase(it);
            if( it != this->free.begin() ) it--;
        // The first half cut off?
        } else if( offset <= it->first && it->first < (offset + length) ) {
            it->second = (it->first + it->second) - (offset + length);
            it->first  = (offset + length);
        // The second half cut off?
        } else if( offset < (it->first + it->second) && (it->first + it->second) <= (offset + length) ) {
            it->second = offset - it->first;
        // Something in the middle?
        } else if( it->first < offset && (offset + length) < (it->first + it->second) ) {
            uint64_t orig_end = (it->first + it->second); 
            this->free.insert(it, free_t(it->first, offset - it->first));
            it->first  = (offset + length);
            it->second = orig_end - (offset + length);
        // Nothing.
        } else {
        }
    }
}

uint64_t DataFile::find(uint32_t length)
{
    for( freelist_t::iterator it = this->free.begin();
         it != this->free.end();
         it++ ) {
        if( it->second >= length ) {
            uint64_t offset = it->first;
            it->second -= length;
            if( it->second == 0 ) {
                this->free.erase(it);
            }
            return offset;
        }
    }
    // Nothing found.  Grow.
    uint64_t offset = this->size;
    this->setsize(this->size + length);
    return offset;
}

void DataFile::release(uint64_t offset, uint32_t length)
{
    if( length == 0 ) {
        return;
    }

    freelist_t::iterator it = this->free.begin();
    while( it != this->free.end() ) {
        if( it->first + it->second == offset ) {
            it->second += length;
            uint64_t current = it->first;
            uint64_t length  = it->second;
            it++;
            if( it != this->free.end() ) {
                if( it->first == (current + length) ) {
                    it->first   = current;
                    it->second += length;
                }
            }
            return;
        } else if( it->first == (offset + length) ) {
            it->first = offset;
            it->second += length;
            return;
        } else if( it->first > offset ) {
            break;
        }
        it++;
    }

    // Add to the free list, no coalescing to be done.
    this->free.insert(it, free_t(offset, length));
}

void DataFile::load(unsigned char* sink, uint64_t off, uint32_t length)
{
    int fd = this->getfd();
    lseek(fd, off, SEEK_SET);
    while( length > 0 ) {
        int b = read(fd, sink, length);
        length -= b;
        sink += b;
    }
    this->relfd(fd);
}

void DataFile::save(unsigned char* source, uint64_t off, uint32_t length)
{
    int fd = this->getfd();
    lseek(fd, off, SEEK_SET);
    while( length > 0 ) {
        int b = write(fd, source, length);
        length -= b;
        source += b;
    }
    this->relfd(fd);
}

void DataFile::dump(std::ostream& out)
{
    out << " data[free=" << this->free.size() << "]" << std::endl;
    out << "  auto"; AutoFile::dump(out); out << std::endl;
    for( freelist_t::iterator it = this->free.begin();
         it != this->free.end();
         it++ ) {
        out << "    free[" << it->first << "," << (it->first + it->second) << "]" << std::endl;
    }
    out << std::endl;
}

Store::Store(const char* directory) :
    index(directory),
    data(directory),
    records(),
    n_data(0),
    n_meta(0),
    n_index(0)
{
    uint64_t r_count = this->index.records();
    for( uint64_t i = 0; i < r_count; i++ ) {
        record_t* record = this->index.get(i, false);
        if( record != NULL ) {
            count(record);
            this->data.mark(record->offset, record->flags.length);
            this->records[record->hash] = i;
            this->index.done(i);
        }
    }
}

void Store::count(record_t* rec)
{
    switch(rec->flags.chunktype) {
        case DATA:
            this->n_data++;
            break;
        case META:
            this->n_meta++;
            break;
        case INDEX:
            this->n_index++;
            break;
    }
}

void Store::uncount(record_t* rec)
{
    switch(rec->flags.chunktype) {
        case DATA:
            this->n_data--;
            break;
        case META:
            this->n_meta--;
            break;
        case INDEX:
            this->n_index--;
            break;
    }
}

Chunk* Store::get(const Hash& hash)
{
    records_t::iterator it = this->records.find(hash);
    if( it == this->records.end() ) {
        return NULL;
    }
    uint64_t i = it->second;
    record_t* record = this->index.get(i, false);
    if( record == NULL ) {
        return NULL;
    }
    Chunk* chunk = NULL;
    switch( record->flags.chunktype ) {
        case DATA:
            chunk = new DataChunk();
            break;
        case META:
            chunk = new MetaChunk();
            break;
        case INDEX:
            chunk = new IndexChunk();
            break;
        default:
            return NULL;
    }
    chunk->prealloc(record->flags.length);
    chunk->flags = record->flags;
    uint64_t offset = record->offset;
    this->index.done(i);
    this->data.load(chunk->data, offset, chunk->flags.length);
    return chunk;
}

Chunk* Store::head(const Hash& hash)
{
    records_t::iterator it = this->records.find(hash);
    if( it == this->records.end() ) {
        return NULL;
    }
    uint64_t i = it->second;
    record_t* record = this->index.get(i, false);
    if( record == NULL ) {
        return NULL;
    }
    Chunk* chunk = NULL;
    switch( record->flags.chunktype ) {
        case DATA:
            chunk = new DataChunk();
            break;
        case META:
            chunk = new MetaChunk();
            break;
        case INDEX:
            chunk = new IndexChunk();
            break;
        default:
            return NULL;
    }
    chunk->flags = record->flags;
    this->index.done(i);
    return chunk;
}

bool Store::add(Chunk* chunk)
{
    records_t::iterator it = this->records.find(chunk->hash());
    if( it != this->records.end() ) {
        return false;
    }
    uint64_t offset = this->data.find(chunk->flags.length);
    this->data.save(chunk->data, offset, chunk->flags.length);

    uint64_t i = this->index.create();
    record_t* record = this->index.get(i, false);
    if( record == NULL ) {
        return false;
    }
    record->hash   = chunk->hash().value();
    record->offset = offset;
    record->flags  = chunk->flags;
    count(record);
    this->records[chunk->hash()] = i;
    this->index.done(i);
    return true;
}

bool Store::remove(const Hash& key)
{
    records_t::iterator it = this->records.find(key);
    if( it == this->records.end() ) {
        return false;
    }
    uint64_t i = it->second;
    record_t* record = this->index.get(i, false);
    if( record == NULL ) {
        return false;
    }
    uint64_t offset = record->offset;
    uint32_t length = record->flags.length;
    uncount(record);
    this->index.done(i);
    this->index.drop(i);
    data.release(offset, length);
    this->records.erase(it);
    return true;
}

bool Store::adjrefs(const Hash& key, int delta)
{
    records_t::iterator it = this->records.find(key);
    if( it == this->records.end() ) {
        return false;
    }
    uint64_t i = it->second;
    record_t* record = this->index.get(i, false);
    if( record == NULL ) {
        return false;
    }
    if( delta + record->flags.references < 0 ) {
        return false;
    } else {
        record->flags.references += delta;
    }
    this->index.done(i);
    return true;
}

HashList* Store::fetchAll(uint64_t start, uint32_t count)
{
    HashList* rval = new HashList();
    records_t::const_iterator it = this->records.begin();
    while( start > 0 && it != this->records.end() ) {
        start--;
        it++;
    }
    while( count > 0 && it != this->records.end() ) {
        count--;
        rval->push_back(it->first);
        it++;
    }
    return rval;
}

HashList* Store::fetchData(uint64_t start, uint32_t count)
{
    HashList* rval = new HashList();
    records_t::const_iterator it = this->records.begin();
    while( start > 0 && it != this->records.end() ) {
        start--;
        it++;
    }
    while( count > 0 && it != this->records.end() ) {
        record_t* rec = this->index.get(it->second, true);
        if( rec->flags.chunktype == DATA ) {
            count--;
            rval->push_back(it->first);
        }
        this->index.done(it->second);
        it++;
    }
    return rval;
}

HashList* Store::fetchMeta(uint64_t start, uint32_t count)
{
    HashList* rval = new HashList();
    records_t::const_iterator it = this->records.begin();
    while( start > 0 && it != this->records.end() ) {
        start--;
        it++;
    }
    while( count > 0 && it != this->records.end() ) {
        record_t* rec = this->index.get(it->second, true);
        if( rec->flags.chunktype == META ) {
            count--;
            rval->push_back(it->first);
        }
        this->index.done(it->second);
        it++;
    }
    return rval;
}

HashList* Store::fetchIndex(uint64_t start, uint32_t count)
{
    HashList* rval = new HashList();
    records_t::const_iterator it = this->records.begin();
    while( start > 0 && it != this->records.end() ) {
        start--;
        it++;
    }
    while( count > 0 && it != this->records.end() ) {
        record_t* rec = this->index.get(it->second, true);
        if( rec->flags.chunktype == INDEX ) {
            count--;
            rval->push_back(it->first);
        }
        this->index.done(it->second);
        it++;
    }
    return rval;
}

uint64_t Store::countAll()
{
    return this->records.size();
}

uint64_t Store::countData()
{
    return this->n_data;
}

uint64_t Store::countMeta()
{
    return this->n_meta;
}

uint64_t Store::countIndex()
{
    return this->n_index;
}

void Store::dump(std::ostream& out)
{
    out << "store[records=" << this->records.size() << "]" << std::endl;
    for( records_t::iterator it = this->records.begin();
         it != this->records.end();
         it++ ) {
        Hash hash = it->first;
        out << "  map[" << hash << "->" << it->second << "]" << std::endl;
    }
    out << this->index;
    out << this->data;
}

std::ostream& operator<<(std::ostream& out, AutoFile& af)
{
    af.dump(out);
    return out;
}

std::ostream& operator<<(std::ostream& out, IndexFile& i)
{
    i.dump(out);
    return out;
}

std::ostream& operator<<(std::ostream& out, DataFile& df)
{
    df.dump(out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Store& s)
{
    s.dump(out);
    return out;
}
