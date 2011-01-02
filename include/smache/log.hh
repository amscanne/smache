//
// log.hh
//

#ifndef _LOG_HH_
#define _LOG_HH_

#include <stdint.h>
#include <stdio.h>

extern int logLevel;

#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define LEVEL_DEBUG 3
#define LEVEL_INFO  2
#define LEVEL_WARN  1
#define LEVEL_ERROR 0
#define SMACHE_DEBUG(l,s,a...) do { \
        if( logLevel >= l) { \
            fprintf(stderr, __FILE__ "." STRINGIFY(__LINE__) ": " s,  ## a); \
            fflush(stderr); \
        } \
    } while(0)

#define DEBUG(s,a...) SMACHE_DEBUG(LEVEL_DEBUG, s, ## a)
#define INFO(s,a...) SMACHE_DEBUG(LEVEL_INFO, s, ## a)
#define WARN(s,a...) SMACHE_DEBUG(LEVEL_WARN, s, ## a)
#define ERROR(s,a...) SMACHE_DEBUG(LEVEL_ERROR, s, ## a)

void setLogLevel(int level);
int getLogLevel();
void progressShow(int level, const char* what, uint64_t current, uint64_t maximum);
void progressDone(int level, const char* what);

#endif // _LOG_HH_
