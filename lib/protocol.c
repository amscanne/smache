/*
 * protocol.c
 */

#include <smache/smache.h>
#include <stdlib.h>

smache_remote* smache_socket_remote(int socket)
{
    smache_remote* remote = malloc(sizeof(smache_remote));
    if( remote == NULL )
    {
        return NULL;
    }

    remote->socket = socket;
    return remote;
}
