/* --------------------------------------------------------------------
Copyright (C) 2019 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of baltrad-ppc.

baltrad-ppc is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

baltrad-ppc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with baltrad-ppc.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Python API to the ppc  options
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#include "pyppc_compat.h"
#include "Python.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PYPPCOPTIONS_MODULE   /**< to get correct part in pyppcoptions */
#include "pyppcoptions.h"
#include "pyppcradaroptions.h"
#include "pyrave_debug.h"
#include "rave_alloc.h"

/**
 * Debug this module
 */
PYRAVE_DEBUG_MODULE("_ppcoptions");

/**
 * Sets a python exception and goto tag
 */
#define raiseException_gotoTag(tag, type, msg) \
{PyErr_SetString(type, msg); goto tag;}

/**
 * Sets python exception and returns NULL
 */
#define raiseException_returnNULL(type, msg) \
{PyErr_SetString(type, msg); return NULL;}

/**
 * Error object for reporting errors to the python interpreeter
 */
static PyObject *ErrorObject;

/// --------------------------------------------------------------------
/// PpcOptions
/// --------------------------------------------------------------------
/*@{ PpcOptions */
/**
 * Returns the native PpcOptions_t instance.
 * @param[in] options - the python ppc  options instance
 * @returns the native PpcOptions_t instance.
 */
static PpcOptions_t*
PyPpcOptions_GetNative(PyPpcOptions* options)
{
  RAVE_ASSERT((options != NULL), "options == NULL");
  return RAVE_OBJECT_COPY(options->options);
}

/**
 * Creates a python ppc  options from a native ppc  options or will create an
 * initial native ppc  options processor if p is NULL.
 * @param[in] p - the native pdp processor (or NULL)
 * @returns the python fmi image.
 */
static PyPpcOptions*
PyPpcOptions_New(PpcOptions_t* p)
{
  PyPpcOptions* result = NULL;
  PpcOptions_t* cp = NULL;

  if (p == NULL) {
    cp = RAVE_OBJECT_NEW(&PpcOptions_TYPE);
    if (cp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for PpcOptions.");
      raiseException_returnNULL(PyExc_MemoryError, "Failed to allocate memory for PpcOptions.");
    }
  } else {
    cp = RAVE_OBJECT_COPY(p);
    result = RAVE_OBJECT_GETBINDING(p); // If p already have a binding, then this should only be increfed.
    if (result != NULL) {
      Py_INCREF(result);
    }
  }

  if (result == NULL) {
    result = PyObject_NEW(PyPpcOptions, &PyPpcOptions_Type);
    if (result != NULL) {
      PYRAVE_DEBUG_OBJECT_CREATED;
      result->options = RAVE_OBJECT_COPY(cp);
      RAVE_OBJECT_BIND(result->options, result);
    } else {
      RAVE_CRITICAL0("Failed to create PyPpcOptions instance");
      raiseException_gotoTag(done, PyExc_MemoryError, "Failed to allocate memory for PpcOptions.");
    }
  }

done:
  RAVE_OBJECT_RELEASE(cp);
  return result;
}

/**
 * Deallocates the ppc  options
 * @param[in] obj the object to deallocate.
 */
static void _pyppcoptions_dealloc(PyPpcOptions* obj)
{
  /*Nothing yet*/
  if (obj == NULL) {
    return;
  }
  PYRAVE_DEBUG_OBJECT_DESTROYED;
  RAVE_OBJECT_UNBIND(obj->options, obj);
  RAVE_OBJECT_RELEASE(obj->options);
  PyObject_Del(obj);
}

/**
 * Creates a new instance of the ppc  options
 * @param[in] self this instance.
 * @param[in] args arguments for creation
 * @return the object on success, otherwise NULL
 */
static PyObject* _pyppcoptions_new(PyObject* self, PyObject* args)
{
  //PyObject* inptr = NULL;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }
  return (PyObject*)PyPpcOptions_New(NULL);
}

static PyObject* _pyppcoptions_load(PyObject* self, PyObject* args)
{
  char* filename = NULL;
  PpcOptions_t* options = NULL;
  PyObject* result = NULL;
  if(!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  if (filename == NULL) {
    raiseException_returnNULL(PyExc_ValueError, "argument must be a filename");
  }
  options = PpcOptions_load(filename);
  if (options == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Could not load file");
  }
  result = (PyObject*)PyPpcOptions_New(options);

  RAVE_OBJECT_RELEASE(options);

  return result;
}

static PyObject* _pyppcoptions_getRadarOptions(PyPpcOptions* self, PyObject* args)
{
  char* radarname = NULL;
  PpcRadarOptions_t* options = NULL;
  PyObject* result = NULL;
  if (!PyArg_ParseTuple(args, "s", &radarname))
    return NULL;
  options = PpcOptions_getRadarOptions(self->options, radarname);
  if (options != NULL) {
    result = (PyObject*)PyPpcRadarOptions_New(options);
  } else {
    raiseException_returnNULL(PyExc_RuntimeError, "Could not find radarname in options");
  }
  RAVE_OBJECT_RELEASE(options);
  return result;
}

static PyObject* _pyppcoptions_exists(PyPpcOptions* self, PyObject* args)
{
  char* radarname = NULL;
  if (!PyArg_ParseTuple(args, "s", &radarname))
    return NULL;
  return PyBool_FromLong(PpcOptions_exists(self->options, radarname));
}


/**
 * All methods a ppc  options can have
 */
static struct PyMethodDef _pyppcoptions_methods[] =
{
  {"getRadarOptions", (PyCFunction)_pyppcoptions_getRadarOptions, 1},
  {"exists", (PyCFunction)_pyppcoptions_exists, 1},
  {NULL, NULL} /* sentinel */
};

/**
 * Returns the specified attribute in the ppc  options
 */
static PyObject* _pyppcoptions_getattro(PyPpcOptions* self, PyObject* name)
{
  return PyObject_GenericGetAttr((PyObject*)self, name);
}

/**
 * Sets the attribute
 */
static int _pyppcoptions_setattro(PyPpcOptions* self, PyObject* name, PyObject* val)
{
  int result = -1;
  if (name == NULL) {
    goto done;
  }

  result = 0;
done:
  return result;
}

/*@} End of Fmi Image */

/// --------------------------------------------------------------------
/// Type definitions
/// --------------------------------------------------------------------
/*@{ Type definitions */
PyTypeObject PyPpcOptions_Type =
{
  PyVarObject_HEAD_INIT(NULL, 0) /*ob_size*/
  "PpcOptions", /*tp_name*/
  sizeof(PyPpcOptions), /*tp_size*/
  0, /*tp_itemsize*/
  /* methods */
  (destructor)_pyppcoptions_dealloc, /*tp_dealloc*/
  0, /*tp_print*/
  (getattrfunc)0,               /*tp_getattr*/
  (setattrfunc)0,               /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number */
  0,
  0,                            /*tp_as_mapping */
  0,                            /*tp_hash*/
  (ternaryfunc)0,               /*tp_call*/
  (reprfunc)0,                  /*tp_str*/
  (getattrofunc)_pyppcoptions_getattro, /*tp_getattro*/
  (setattrofunc)_pyppcoptions_setattro, /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT, /*tp_flags*/
  0,                            /*tp_doc*/
  (traverseproc)0,              /*tp_traverse*/
  (inquiry)0,                   /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  _pyppcoptions_methods,              /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  0,                            /*tp_init*/
  0,                            /*tp_alloc*/
  0,                            /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};
/*@} End of Type definitions */

/*@{ Module setup */
static PyMethodDef functions[] = {
  {"new", (PyCFunction)_pyppcoptions_new, 1},
  {"load", (PyCFunction)_pyppcoptions_load, 1},
  {NULL,NULL} /*Sentinel*/
};

/**
 * Adds constants to the dictionary (probably the modules dictionary).
 * @param[in] dictionary - the dictionary the long should be added to
 * @param[in] name - the name of the constant
 * @param[in] value - the value
 */
static void add_string_constant(PyObject* dictionary, const char* name, const char* value)
{
  PyObject* tmp = NULL;
  tmp = PyString_FromString(value);
  if (tmp != NULL) {
    PyDict_SetItemString(dictionary, name, tmp);
  }
  Py_XDECREF(tmp);
}

MOD_INIT(_ppcoptions)
{
  PyObject *module=NULL,*dictionary=NULL;
  static void *PyPpcOptions_API[PyPpcOptions_API_pointers];
  PyObject *c_api_object = NULL;

  MOD_INIT_SETUP_TYPE(PyPpcOptions_Type, &PyType_Type);

  MOD_INIT_VERIFY_TYPE_READY(&PyPpcOptions_Type);

  MOD_INIT_DEF(module, "_ppcoptions", NULL/*doc*/, functions);
  if (module == NULL) {
    return MOD_INIT_ERROR;
  }

  PyPpcOptions_API[PyPpcOptions_Type_NUM] = (void*)&PyPpcOptions_Type;
  PyPpcOptions_API[PyPpcOptions_GetNative_NUM] = (void *)PyPpcOptions_GetNative;
  PyPpcOptions_API[PyPpcOptions_New_NUM] = (void*)PyPpcOptions_New;


  c_api_object = PyCapsule_New(PyPpcOptions_API, PyPpcOptions_CAPSULE_NAME, NULL);
  dictionary = PyModule_GetDict(module);
  PyDict_SetItemString(dictionary, "_C_API", c_api_object);

  ErrorObject = PyErr_NewException("_ppcoptions.error", NULL, NULL);
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _ppcoptions.error");
    return MOD_INIT_ERROR;
  }

  add_string_constant(dictionary, "P_TH_CORR", "P_TH_CORR");
  add_string_constant(dictionary, "P_ATT_TH_CORR", "P_ATT_TH_CORR");
  add_string_constant(dictionary, "P_DBZH_CORR", "P_DBZH_CORR");
  add_string_constant(dictionary, "P_ATT_DBZH_CORR", "P_ATT_DBZH_CORR");
  add_string_constant(dictionary, "P_KDP_CORR", "P_KDP_CORR");
  add_string_constant(dictionary, "P_RHOHV_CORR", "P_RHOHV_CORR");
  add_string_constant(dictionary, "P_PHIDP_CORR", "P_PHIDP_CORR");
  add_string_constant(dictionary, "P_ZDR_CORR", "P_ZDR_CORR");
  add_string_constant(dictionary, "P_ZPHI_CORR", "P_ZPHI_CORR");
  add_string_constant(dictionary, "Q_RESIDUAL_CLUTTER_MASK", "Q_RESIDUAL_CLUTTER_MASK");
  add_string_constant(dictionary, "Q_ATTENUATION_MASK", "Q_ATTENUATION_MASK");
  add_string_constant(dictionary, "Q_ATTENUATION", "Q_ATTENUATION");

  import_ppcradaroptions();
  PYRAVE_DEBUG_INITIALIZE;
  return MOD_INIT_SUCCESS(module);
}

/*@} End of Module setup */
