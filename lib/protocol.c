/*
 * protocol.c
 */

#include <stdlib.h>
#include <smache/smache.h>

static smache_error
remote_get(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    return SMACHE_SUCCESS;
}

static smache_error
remote_put(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    return SMACHE_SUCCESS;
}

static smache_error
remote_delete(smache_backend* backend, smache_hash* hash)
{
    return SMACHE_SUCCESS;
}

static smache_error
remote_close(smache_backend* backend)
{
    close((int)backend->internals);
    free(backend);
    return SMACHE_SUCCESS;
}

smache_backend*
smache_remote_backend(int socket)
{
    smache_backend* remote = malloc(sizeof(smache_backend));
    if( remote == NULL )
    {
        return NULL;
    }

    remote->internals = (void*)socket;
    remote->get       = &remote_get;
    remote->put       = &remote_put;
    remote->delete    = &remote_delete;
    remote->close     = &remote_close;

    return remote;
}
