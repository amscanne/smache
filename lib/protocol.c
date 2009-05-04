/*
 * protocol.c
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <smache/smache.h>

struct remote_internals
{
    int socket;
};

static int
remote_get(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    // int socket = ((struct remote_internals*)backend->internals)->socket;
    return SMACHE_SUCCESS;
}

static int
remote_put(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    // int socket = ((struct remote_internals*)backend->internals)->socket;
    return SMACHE_SUCCESS;
}

static int
remote_delete(smache_backend* backend, smache_hash* hash)
{
    // int socket = ((struct remote_internals*)backend->internals)->socket;
    return SMACHE_SUCCESS;
}

static int
remote_close(smache_backend* backend)
{
    int socket = ((struct remote_internals*)backend->internals)->socket;
    close(socket);
    free(backend->internals);
    free(backend);
    return SMACHE_SUCCESS;
}

smache_backend*
smache_remote_backend(int socket)
{
    smache_backend* remote = malloc(sizeof(smache_backend));
    if( remote == NULL )
    {
        fprintf(stderr, "error: Out of memory.\n");
        return NULL;
    }

    remote->internals = malloc(sizeof(struct remote_internals));
    if( remote->internals == NULL )
    {
        fprintf(stderr, "error: Out of memory.\n");
        free(remote);
        return NULL;
    }

    ((struct remote_internals*)remote->internals)->socket = socket;

    remote->get       = &remote_get;
    remote->put       = &remote_put;
    remote->delete    = &remote_delete;
    remote->close     = &remote_close;

    return remote;
}

int
smache_server(unsigned short port)
{
    struct sockaddr_in addr;
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( sock < 0 )
    {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return SMACHE_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0 )
    {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return SMACHE_ERROR;
    }

    if( listen(sock, 10) < 0 )
    {
        fprintf(stderr, "error: %s\n", strerror(errno));
        return SMACHE_ERROR;
    }

    while( 1 )
    {
        int clientfd = accept(sock, NULL, NULL);
        if( clientfd < 0 )
        {
            fprintf(stderr, "warning: %s\n", strerror(errno));
            continue;
        }
        close(clientfd);
    }

    return SMACHE_SUCCESS;
}
