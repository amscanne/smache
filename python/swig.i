/* swig.i */
%module native
%{
#include "swig.hh"
#include "smache/log.hh"
#include "smache/chunk.hh"
#include "smache/cas.hh"
#include "smache/backend.hh"
#include "smache/store.hh"
#include "smache/smache.hh"
%}

typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

%typemap(out) HashList* {
    $result = PyList_New($1->size());
    int i = 0;
    for( HashList::const_iterator it = $1->begin(); it != $1->end(); it++ ) {
        PyList_SetItem($result, i++, SWIG_NewPointerObj(SWIG_as_voidptr(new Hash(*it)), SWIGTYPE_p_Hash, SWIG_POINTER_NEW | 0 ));
    }
}

%typemap(out) BackendList* {
    $result = PyList_New($1->size());
    int i = 0;
    for( BackendList::const_iterator it = $1->begin(); it != $1->end(); it++ ) {
        PyObject* obj = SWIG_NewPointerObj(SWIG_as_voidptr(*it), SWIGTYPE_p_Backend, SWIG_POINTER_NEW | 0 );
        SwigPyObject_disown(obj);
        PyList_SetItem($result, i++, obj);
    }
}

%typemap(in) unsigned char* {
    $1 = (unsigned char*)PyString_AsString($input);
}

%include "swig.hh"
%include "smache/log.hh"
%include "smache/chunk.hh"
%include "smache/cas.hh"
#include "smache/backend.hh"
%include "smache/store.hh"
%include "smache/smache.hh"
