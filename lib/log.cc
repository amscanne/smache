//
// log.hh
//

#include <stdio.h>

#include <smache/log.hh>

int logLevel = LEVEL_ERROR;

void setLogLevel(int level)
{
    logLevel = level;
}

int getLogLevel()
{
    return logLevel;
}

void progressShow(int level, const char* what, uint64_t current, uint64_t maximum)
{
    static char indicator[] = { '-', '\\', '|', '/', '.' };
    if( current % 10000 == 0 ) {
        int perc = (maximum == 0) ? 100 : (100*current) / maximum;
        int index = (maximum == 0) ? 5 : (current / 10000) % 4;
        SMACHE_DEBUG(logLevel, "%s...  % 3d%% %c\r", what, perc, indicator[index]);
    }
}

void progressDone(int level, const char* what)
{
    SMACHE_DEBUG(logLevel, "%s... done.      \r\n", what);
}
