#include "Python.h"
#include "structmember.h"

/* set_combinations
   by Keyhan Vakil <kvakil@berkeley.edu>
   modified from itertools module written
   by Raymond D. Hettinger <python@rcn.com>
*/

/* combinations object ************************************************************/

typedef struct {
    PyObject_HEAD
    PyObject *pool;                     /* input converted to a tuple */
    Py_ssize_t *indices;            /* one index per result element */
    PyObject *result;               /* most recently returned result tuple */
    Py_ssize_t r;                       /* size of result tuple */
    int stopped;                        /* set to 1 when the combinations iterator is exhausted */
} combinationsobject;

static PyTypeObject combinations_type;

static PyObject *
combinations_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    combinationsobject *co;
    Py_ssize_t n;
    Py_ssize_t r;
    PyObject *pool = NULL;
    PyObject *iterable = NULL;
    Py_ssize_t *indices = NULL;
    Py_ssize_t i;
    static char *kwargs[] = {"iterable", "r", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "On:combinations", kwargs,
                                     &iterable, &r))
        return NULL;

    pool = PySequence_Tuple(iterable);
    if (pool == NULL)
        goto error;
    n = PyTuple_GET_SIZE(pool);
    if (r < 0) {
        PyErr_SetString(PyExc_ValueError, "r must be non-negative");
        goto error;
    }

    indices = PyMem_Malloc(r * sizeof(Py_ssize_t));
    if (indices == NULL) {
        PyErr_NoMemory();
        goto error;
    }

    for (i = 0; i < r; i++)
        indices[i] = i;

    /* create combinationsobject structure */
    co = (combinationsobject *)type->tp_alloc(type, 0);
    if (co == NULL)
        goto error;

    co->pool = pool;
    co->indices = indices;
    co->result = NULL;
    co->r = r;
    co->stopped = r > n ? 1 : 0;

    return (PyObject *)co;

error:
    if (indices != NULL)
        PyMem_Free(indices);
    Py_XDECREF(pool);
    return NULL;
}

static void
combinations_dealloc(combinationsobject *co)
{
    PyObject_GC_UnTrack(co);
    Py_XDECREF(co->pool);
    Py_XDECREF(co->result);
    if (co->indices != NULL)
        PyMem_Free(co->indices);
    Py_TYPE(co)->tp_free(co);
}

static int
combinations_traverse(combinationsobject *co, visitproc visit, void *arg)
{
    Py_VISIT(co->pool);
    Py_VISIT(co->result);
    return 0;
}

static PyObject *
combinations_next(combinationsobject *co)
{
    PyObject *elem;
    PyObject *pool = co->pool;
    Py_ssize_t *indices = co->indices;
    PyObject *result = co->result;
    Py_ssize_t n = PyTuple_GET_SIZE(pool);
    Py_ssize_t r = co->r;
    Py_ssize_t i, j, index;

    if (co->stopped)
        return NULL;

    if (result == NULL) {
        /* On the first pass, initialize result tuple using the indices */
        result = PySet_New(NULL);
        if (result == NULL)
            goto empty;
        co->result = result;
        for (i = 0; i < r; i++) {
            index = indices[i];
            elem = PyTuple_GET_ITEM(pool, index);
            PySet_Add(result, elem);
        }
    } else {
        /* Copy the previous result tuple or re-use it if available */
        if (Py_REFCNT(result) > 1) {
            PyObject *old_result = result;
            result = PySet_New(NULL);
            if (result == NULL)
                goto empty;
            co->result = result;
            Py_DECREF(old_result);
        }

        /* Now, we've got the only copy so we can update it in-place
         * CPython's empty tuple is a singleton and cached in
         * PyTuple's freelist.
         */
        assert(r == 0 || Py_REFCNT(result) == 1);

        /* Scan indices right-to-left until finding one that is not
           at its maximum (i + n - r). */
        for (i = r - 1; i >= 0 && indices[i] == i + n - r; i--)
            ;

        /* If i is negative, then the indices are all at
           their maximum value and we're done. */
        if (i < 0)
            goto empty;

        /* Increment the current index which we know is not at its
           maximum.  Then move back to the right setting each index
           to its lowest possible value (one higher than the index
           to its left -- this maintains the sort order invariant). */
        indices[i]++;
        for (j = i + 1; j < r; j++)
            indices[j] = indices[j-1] + 1;

        /* Add elements to the result set */
        for (i = 0; i < r; i++) {
            index = indices[i];
            elem = PyTuple_GET_ITEM(pool, index);
            PySet_Add(result, elem);
        }
    }

    Py_INCREF(result);
    return result;

empty:
    co->stopped = 1;
    return NULL;
}

PyDoc_STRVAR(combinations_doc,
"set_combinations(iterable, r) --> combinations object\n\
\n\
Return successive r-length combinations of elements in the iterable as sets.\n\n\
set_combinations(range(4), 3) --> {0,1,2}, {0,1,3}, {0,2,3}, {1,2,3}");

static PyTypeObject combinations_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "set_combinations.set_combinations",        /* tp_name */
    sizeof(combinationsobject),         /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)combinations_dealloc,           /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    PyObject_GenericGetAttr,            /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
        Py_TPFLAGS_BASETYPE,            /* tp_flags */
    combinations_doc,                           /* tp_doc */
    (traverseproc)combinations_traverse,        /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    PyObject_SelfIter,                  /* tp_iter */
    (iternextfunc)combinations_next,            /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    combinations_new,                           /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

/* module level code ********************************************************/

PyDoc_STRVAR(module_doc,
"itertools.combinations which returns sets.\n\
set_combinations(p, r)\n\
");


static PyMethodDef module_methods[] = {
    {NULL,              NULL}           /* sentinel */
};

PyMODINIT_FUNC
initset_combinations(void)
{
    int i;
    PyObject *m;
    char *name;
    PyTypeObject *typelist[] = {
        &combinations_type,
        NULL
    };

    m = Py_InitModule3("set_combinations", module_methods, module_doc);
    if (m == NULL)
        return;

    for (i=0 ; typelist[i] != NULL ; i++) {
        if (PyType_Ready(typelist[i]) < 0)
            return;
        name = strchr(typelist[i]->tp_name, '.');
        assert (name != NULL);
        Py_INCREF(typelist[i]);
        PyModule_AddObject(m, name+1, (PyObject *)typelist[i]);
    }
}
