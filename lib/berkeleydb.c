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
#define DBPUT(x, key, val) x->put(x, &key, &val, R_SETCURSOR)
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
    DB* dbp = *((DB**)(backend->internals));
    DBT key;
    DBT val;

    memset(&key, 0, sizeof(key));
    memset(&val, 0, sizeof(val));

    key.data = hash;
    key.size = sizeof(*hash);
#ifndef __APPLE__
    key.ulen = sizeof(*hash);
    key.flags = 0;
#endif

    val.data = data;
    val.size = SMACHE_MAXIMUM_CHUNKSIZE;
#ifndef __APPLE__
    val.ulen = SMACHE_MAXIMUM_CHUNKSIZE;
    val.flags = DB_DBT_USERMEM;
#endif

    int rval = DBGET(dbp, key, val);
    if( rval ) 
    {
#ifdef __APPLE__
        fprintf(stderr, "bdb_get: %s (%d)\n", strerror(errno), errno);
#else
        fprintf(stderr, "bdb_get: %s (%d)\n", smache_temp_hashstr(hash), rval);
#endif
        return SMACHE_ERROR;
    }

    return SMACHE_SUCCESS;
}

static int
bdb_put(smache_backend* backend, smache_hash* hash, smache_chunk* data)
{
    DB* dbp = *((DB**)(backend->internals));
    DBT key;
    DBT val;

    memset(&key, 0, sizeof(key));
    memset(&val, 0, sizeof(val));

    key.data = hash;
    key.size = sizeof(*hash);
#ifndef __APPLE__
    key.ulen = sizeof(*hash);
    key.flags = 0;
#endif

    val.data = data;
    val.size = sizeof(*data) + data->length;
#ifndef __APPLE__
    val.ulen = SMACHE_MAXIMUM_CHUNKSIZE;
    val.flags = 0;
#endif

    int rval = DBPUT(dbp, key, val);
    if( rval )
    {
#ifdef __APPLE__
        fprintf(stderr, "bdb_put: %s (%d)\n", strerror(errno), errno);
#else
        fprintf(stderr, "bdb_put: %s (%d)\n", smache_temp_hashstr(hash), rval);
#endif
        return SMACHE_ERROR;
    }

    return SMACHE_SUCCESS;
}

static int
bdb_delete(smache_backend* backend, smache_hash* hash)
{
    DB* dbp = *((DB**)(backend->internals));
    DBT key;

    memset(&key, 0, sizeof(key));

    key.data = hash;
    key.size = sizeof(*hash);
#ifndef __APPLE__
    key.ulen = sizeof(*hash);
    key.flags = 0;
#endif

    int rval = DBDEL(dbp, key);
    if( rval )
    {
#ifdef __APPLE__
        fprintf(stderr, "bdb_delete: %s (%d)\n", strerror(errno), errno);
#else
        fprintf(stderr, "bdb_delete: %s (%d)\n", smache_temp_hashstr(hash), rval);
#endif
        return SMACHE_ERROR;
    }
    return SMACHE_SUCCESS;
}
 
static int
bdb_close(smache_backend* backend)
{
    DB* dbp = *((DB**)(backend->internals));

    int rval = DBCLOSE(dbp);
    if( rval )
    {
#ifdef __APPLE__
        fprintf(stderr, "bdb_close: %s (%d)\n", strerror(errno), errno);
#else
        fprintf(stderr, "bdb_close: (%d)\n", rval);
#endif
        free(backend->internals);
        free(backend);
        return SMACHE_ERROR;
    }

    free(backend->internals);
    free(backend);
    return SMACHE_SUCCESS;
}

smache_backend* smache_berkeleydb_backend(const char* filename)
{
    smache_backend* res = malloc(sizeof(smache_backend));
    if( res == NULL )
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        return NULL;
    }

#ifdef __APPLE__
    DB* dbp = dbopen(filename, O_CREAT, 0644, DB_BTREE, NULL);
    if( dbp == NULL )
    {
        fprintf(stderr, "dbopen: unknown error.\n");
        free(res);
        return NULL;
    }
#else
    DB* dbp = NULL;
    int rval = db_create(&dbp, NULL, 0);
    if( rval )
    {
        fprintf(stderr, "dbopen: (%d)\n", rval);
        free(res);
        return NULL;
    }
    
    rval = dbp->open(dbp, NULL, filename, NULL, DB_BTREE, DB_CREATE, 0644);
    if( rval ) 
    {
        fprintf(stderr, "dbopen: (%d)\n", rval);
        DBCLOSE(dbp);
        free(res);
        return NULL;
    }

    dbp->set_errfile(dbp, stderr);
#endif

    res->internals = malloc(sizeof(DB**));
    *((DB**)(res->internals)) = dbp;

    res->get       = &bdb_get;
    res->put       = &bdb_put;
    res->delete    = &bdb_delete;
    res->close     = &bdb_close;
    res->push      = 1;

    return res;
}
