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

#ifdef __APPLE__
#define DBGET(x, key, val) x->get(x, &key, &val, 0)
#define DBPUT(x, key, val) x->put(x, &key, &val, 0)
#define DBDEL(x, key)      x->del(x, &key, 0)
#define DBCLOSE(x)         x->close(x)
#else
#define DBGET(x, key, val) x->get(x, NULL, &key, &val, 0)
#define DBPUT(x, key, val) x->put(x, NULL, &key, &val, 0)
#define DBDEL(x, key)      x->del(x, NULL, &key, 0)
#define DBCLOSE(x)         x->close(x, 0)
#endif

static int
bdb_get(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;
    DBT val;

    key.data = hash;
    key.size = sizeof(*hash);

    val.data = data;
    val.size = sizeof(*data) + data->length;

    if( DBGET(dbp, key, val) )
    {
        return SMACHE_ERROR;
    }
    return SMACHE_SUCCESS;
}

static int
bdb_put(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;
    DBT val;

    key.data = hash;
    key.size = sizeof(*hash);

    val.data = data;
    val.size = sizeof(*data) + data->length;
   
    if( DBPUT(dbp, key, val) )
    {
        return SMACHE_ERROR;
    }
    return SMACHE_SUCCESS;
}

static int
bdb_delete(smache_backend* backend, smache_hash* hash)
{
    DB* dbp = (DB*)(backend->internals);
    DBT key;

    key.data = hash;
    key.size = sizeof(*hash);

    if( DBDEL(dbp, key) )
    {
        return SMACHE_ERROR;
    }
    return SMACHE_SUCCESS;
}
 
static int
bdb_close(smache_backend* backend)
{
    DB* dbp = (DB*)(backend->internals);

    if( DBCLOSE(dbp) != 0 )
    {
        free(backend);
        return SMACHE_ERROR;
    }
    free(backend);
    return SMACHE_SUCCESS;
}

smache_backend* smache_berkeleydb_backend(const char* filename)
{
    smache_backend* res = malloc(sizeof(smache_backend));
    if( res == NULL )
    {
        return NULL;
    }

    DB* dbp = NULL;
    if( db_create(&dbp, NULL, 0) != 0 )
    {
        fprintf(stderr, "dbopen: %s\n", strerror(errno));
        free(res);
        return NULL;
    }
    
    if( dbp->open(dbp, NULL, filename, "smache", DB_BTREE, DB_CREATE, 0644) )
    {
        fprintf(stderr, "dbopen: %s\n", strerror(errno));
        DBCLOSE(dbp);
        free(res);
        return NULL;
    }

    res->internals = (void*)dbp;
    res->get       = &bdb_get;
    res->put       = &bdb_put;
    res->delete    = &bdb_delete;
    res->close     = &bdb_close;

    return res;
}
