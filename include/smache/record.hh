//
// record.hh
//

#ifndef _RECORD_HH_
#define _RECORD_HH_

#include <smache/chunk.hh>

typedef struct {
    unsigned char magic[8];
    uint32_t      version;
} header_t;

typedef struct {
    uint32_t active   :1;
    uint32_t reserved :31;

    hash_t   hash;
    flags_t  flags;
    uint64_t offset;
#ifndef SWIG
} __attribute__((packed)) record_t;
#else
} record_t;
#endif

#endif // _RECORD_HH_
