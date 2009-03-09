/*
 * smache.c
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <smache/smache.h>

struct backend_list {
    smache_backend* current;
    struct backend_list* next;
};

struct remote_list {
    smache_remote* current;
    struct remote_list* next;
};

struct socket_list {
    int socket;
    struct socket_list* next;
};

struct smache_priv {
    struct backend_list  backends;
    struct remote_list   remotes;
    struct socket_list   sockets;
};

/*
 * Constructor and modifiers for a smache instance.
 */

smache* smache_create()
{
    smache* instance = malloc(sizeof(smache));
    if( instance == NULL )
    {
        return NULL;
    }
    memset(instance, 0, sizeof(*instance));

    instance->internals = malloc(sizeof(struct smache_priv));
    if( instance->internals == NULL )
    {
        free(instance);
        return NULL;
    }
    memset(instance->internals, 0, sizeof(*(instance->internals)));

    return instance;
}

smache_error smache_add_backend(smache* instance, smache_backend* backend)
{
    struct backend_list* backends = &(instance->internals->backends);

    while( backends->current != NULL ) backends = backends->next;
    backends->current = backend;

    backends->next = malloc(sizeof(struct backend_list));
    if( backends->next == NULL )
    {
        backends->current = NULL;
        return SMACHE_ERROR;
    }

    backends->next->current = NULL;
    backends->next->next    = NULL;

    return SMACHE_SUCCESS;
}

smache_error smache_add_remote(smache* instance, smache_remote* remote)
{
    struct remote_list* remotes = &(instance->internals->remotes);

    while( remotes->current != NULL ) remotes = remotes->next;
    remotes->current = remote;

    remotes->next = malloc(sizeof(struct remote_list));
    if( remotes->next == NULL )
    {
        remotes->current = NULL;
        return SMACHE_ERROR;
    }

    remotes->next->current = NULL;
    remotes->next->next    = NULL;

    return SMACHE_SUCCESS;
}
