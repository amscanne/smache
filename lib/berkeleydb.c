/*
 * berkeleydb.c
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <db.h>
#include <smache/smache.h>

static smache_error
bdb_get(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;
    DBT val;

    key.data = hash;
    key.size = sizeof(*hash);

    val.data = data;
    val.size = sizeof(*data) + data->length;

    if( dbp->get(dbp, &key, &val, 0) )
    {
        return SMACHE_ERROR;
    }
    else
    {
        return SMACHE_SUCCESS;
    }
}

static smache_error
bdb_put(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;
    DBT val;

    key.data = hash;
    key.size = sizeof(*hash);

    val.data = data;
    val.size = sizeof(*data) + data->length;
   
    if( dbp->put(dbp, &key, &val, 0) )
    {
        return SMACHE_ERROR;
    }
    else
    {
        return SMACHE_SUCCESS;
    }
}

static smache_error
bdb_delete(smache_backend* backend, smache_hash* hash)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;

    key.data = hash;
    key.size = sizeof(*hash);

    if( dbp->del(dbp, &key, 0) )
    {
        return SMACHE_ERROR;
    }

    return SMACHE_SUCCESS;
}
 
static smache_error
bdb_close(smache_backend* backend)
{
    DB* dbp = (DB*)(backend->internals);

    if( dbp->close(dbp) == 0 )
    {
        free(backend);
        return SMACHE_SUCCESS;
    }
    else
    {
        free(backend);
        return SMACHE_ERROR;
    }
}

smache_backend* smache_berkeleydb_backend(const char* filename)
{
    smache_backend* res = malloc(sizeof(smache_backend));
    if( res == NULL )
    {
        return NULL;
    }

    res->internals = (void*)dbopen(filename, O_CREAT, 0644, DB_BTREE, NULL);
    if( res->internals == NULL )
    {
        fprintf(stderr, "db_create: %s\n", strerror(errno));
        free(res);    
        return NULL;
    }

    res->get       = &bdb_get;
    res->put       = &bdb_put;
    res->delete    = &bdb_delete;
    res->close     = &bdb_close;

    return res;
}
