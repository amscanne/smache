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
bdb_exists(smache_backend* backend, smache_hash* hash)
{
    DB* dbp = *((DB**)(backend->internals));
    DBT key;
    DBT val;

    memset(&key, 0, sizeof(key));

    key.data = hash;
    key.size = sizeof(*hash);
    key.ulen = sizeof(*hash);

    val.data = NULL;
    val.size = 0;
    val.ulen = 0;
    val.flags = DB_DBT_USERMEM;

    int rval = DBGET(dbp, key, val);
    if( rval == DB_BUFFER_SMALL )
    {
        /*
         * Found, but not returnable.
         */
        return SMACHE_SUCCESS;
    }
    else
    {
        return SMACHE_ERROR;
    }
}

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
    key.ulen = sizeof(*hash);

    val.data = data;
    val.size = SMACHE_MAXIMUM_CHUNKSIZE;
    val.ulen = SMACHE_MAXIMUM_CHUNKSIZE;
    val.flags = DB_DBT_USERMEM;

    int rval = DBGET(dbp, key, val);
    if( rval ) 
    {
        fprintf(stderr, "bdb_get: %s (%d)\n", smache_temp_hashstr(hash), rval);
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

    val.data = data;
    val.size = sizeof(*data) + data->length;
    val.ulen = SMACHE_MAXIMUM_CHUNKSIZE;
    val.flags = 0;

    int rval = DBPUT(dbp, key, val);
    if( rval )
    {
        fprintf(stderr, "bdb_put: %s (%d)\n", smache_temp_hashstr(hash), rval);
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

    int rval = DBDEL(dbp, key);
    if( rval )
    {
        fprintf(stderr, "bdb_delete: %s (%d)\n", smache_temp_hashstr(hash), rval);
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
        fprintf(stderr, "bdb_close: (%d)\n", rval);
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
    res->internals = malloc(sizeof(DB**));
    *((DB**)(res->internals)) = dbp;

    res->exists    = &bdb_exists;
    res->get       = &bdb_get;
    res->put       = &bdb_put;
    res->delete    = &bdb_delete;
    res->close     = &bdb_close;
    res->push      = 1;

    return res;
}
