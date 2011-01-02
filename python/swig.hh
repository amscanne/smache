#ifndef SWIG
#define SWIG
#endif

#include "../include/smache/backend.hh"

class Remote : protected Backend {
protected:
    PyObject* obj;

public:
    Remote(PyObject *o) : obj(o)
    {
        Py_XINCREF(this->obj);
    }
    ~Remote()
    {
        Py_XDECREF(this->obj);
    }

    PyObject* eval(const char* method, PyObject* arglist)
    {
        PyObject *s = Py_BuildValue("s", method);
        PyObject *meth = PyObject_GetAttr(this->obj, s);
        Py_XDECREF(s);

        // Sanity check the method.
        if (!PyCallable_Check(meth)) {
            PyErr_BadInternalCall();
            return NULL;
        }

        PyObject *result = PyEval_CallObject(meth, arglist);

        return result;
    }

    virtual Chunk* head(const Hash& key)
    {
        PyObject *param = SWIG_NewPointerObj(SWIG_as_voidptr(new Hash(key)), SWIGTYPE_p_Hash, SWIG_POINTER_NEW |  0 );
        PyObject *arglist = Py_BuildValue("(O)", param);
        PyObject *result = eval("_head", arglist);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the resulting chunk.
        Chunk *chunk = NULL;
        if ((SWIG_ConvertPtr(result, (void **)&chunk, SWIGTYPE_p_Chunk,1)) == -1) {
            return NULL;
        }
        Py_XDECREF(result); 

        return chunk;
    }

    virtual Chunk* get(const Hash& key)
    {
        PyObject *param = SWIG_NewPointerObj(SWIG_as_voidptr(new Hash(key)), SWIGTYPE_p_Hash, SWIG_POINTER_NEW |  0 );
        PyObject *arglist = Py_BuildValue("(O)", param);
        PyObject *result = eval("_get", arglist);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the resulting chunk.
        Chunk *chunk = NULL;
        if ((SWIG_ConvertPtr(result, (void **)&chunk, SWIGTYPE_p_Chunk,1)) == -1) {
            return NULL;
        }
        Py_XDECREF(result); 

        return chunk;
    }


    virtual bool add(Chunk* chunk)
    {
        PyObject *param = SWIG_NewPointerObj(SWIG_as_voidptr(chunk), SWIGTYPE_p_Hash, SWIG_POINTER_NEW |  0 );
        PyObject *arglist = Py_BuildValue("(O)", param);
        PyObject *result = eval("_add", arglist);
        SwigPyObject_disown(param);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the result.
        bool val = (PyBool_Check(result) != 0);
        Py_XDECREF(result);

        return val;
    }
 
    virtual bool remove(const Hash& key)
    {
        PyObject *param = SWIG_NewPointerObj(SWIG_as_voidptr(new Hash(key)), SWIGTYPE_p_Hash, SWIG_POINTER_NEW |  0 );
        PyObject *arglist = Py_BuildValue("(O)", param);
        PyObject *result = eval("_remove", arglist);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the result.
        bool val = (PyBool_Check(result) != 0);
        Py_XDECREF(result);

        return val;
    }

    virtual bool adjrefs(const Hash& key, int delta)
    {
        PyObject *param = SWIG_NewPointerObj(SWIG_as_voidptr(new Hash(key)), SWIGTYPE_p_Hash, SWIG_POINTER_NEW |  0 );
        PyObject *arglist = Py_BuildValue("(Oi)", param, delta);
        PyObject *result = eval("_adjrefs", arglist);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the result.
        bool val = (PyBool_Check(result) != 0);
        Py_XDECREF(result); 

        return val;
    }

    HashList* list(const char* method, uint64_t start, uint32_t count)
    {
        PyObject *s = PyLong_FromLong(start);
        PyObject *c = PyLong_FromLong(count);
        PyObject *arglist = Py_BuildValue("(OO)", s, c);
        PyObject *result = eval(method, arglist);
        Py_DECREF(arglist);
        Py_DECREF(s);
        Py_DECREF(c);
        if( result == NULL ) return NULL;

        return new HashList();

        // Grab the result.
        HashList* rval = new HashList();
        int size = PyList_Size(result);
        for (int i = 0; i < size; i++) {
            PyObject *o = PyList_GetItem(result,i);
            Hash* hash = NULL;
            if ((SWIG_ConvertPtr(o, (void **)&hash, SWIGTYPE_p_Hash,1)) == -1) {
                // Unable to convert.
            } else {
                SwigPyObject_disown(o);
                rval->push_back(*hash);
            }
        }
        Py_XDECREF(result);

        return rval;
    }

    uint64_t count(const char* method)
    {
        PyObject *arglist = Py_BuildValue("()");
        PyObject *result = eval(method, arglist);
        Py_DECREF(arglist);
        if( result == NULL ) return NULL;

        // Grab the result.
        uint64_t res = (uint64_t)(PyInt_AsLong(result));
        Py_XDECREF(result); 

        return res;
    }

    virtual HashList* fetchAll(uint64_t start, uint32_t count)
    {
        return list("_fetchAll", start, count);
    }

    virtual HashList* fetchData(uint64_t start, uint32_t count)
    {
        return list("_fetchData", start, count);
    }

    virtual HashList* fetchMeta(uint64_t start, uint32_t count)
    {
        return list("_fetchMeta", start, count);
    }

    virtual HashList* fetchIndex(uint64_t start, uint32_t count)
    {
        return list("_fetchIndex", start, count);
    }

    virtual uint64_t countAll()
    {
        return count("_countAll");
    }

    virtual uint64_t countData()
    {
        return count("_countData");
    }

    virtual uint64_t countMeta()
    {
        return count("_countMeta");
    }

    virtual uint64_t countIndex()
    {
        return count("_countIndex");
    }
};
