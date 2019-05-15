/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of bRopo.

bRopo is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bRopo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with bRopo.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Python version of the ppc  options class
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#ifndef PYPPCOPTIONS_H
#define PYPPCOPTIONS_H
#include "ppc_options.h"

/**
 * The ppc objects class
 */
typedef struct {
   PyObject_HEAD /*Always have to be on top*/
   PpcOptions_t* options;  /**< the options */
} PyPpcOptions;

#define PyPpcOptions_Type_NUM 0                              /**< index of type */

#define PyPpcOptions_GetNative_NUM 1                         /**< index of GetNative*/
#define PyPpcOptions_GetNative_RETURN PpcOptions_t*        /**< return type for GetNative */
#define PyPpcOptions_GetNative_PROTO (PyPpcOptions*)       /**< arguments for GetNative */

#define PyPpcOptions_New_NUM 2                               /**< index of New */
#define PyPpcOptions_New_RETURN PyPpcOptions*              /**< return type for New */
#define PyPpcOptions_New_PROTO (PpcOptions_t*)             /**< arguments for New */

#define PyPpcOptions_API_pointers 3                         /**< number of type and function pointers */

#define PyPpcOptions_CAPSULE_NAME "_ppcoptions._C_API"

#ifdef PYPPCOPTIONS_MODULE
/** Forward declaration of type */
extern PyTypeObject PyPpcOptions_Type;

/** Checks if the object is a PyPdpProcessor or not */
#define PyPpcOptions_Check(op) ((op)->ob_type == &PyPpcOptions_Type)

/** Forward declaration of PyPdpProcessor_GetNative */
static PyPpcOptions_GetNative_RETURN PyPpcOptions_GetNative PyPpcOptions_GetNative_PROTO;

/** Forward declaration of PyPdpProcessor_New */
static PyPpcOptions_New_RETURN PyPpcOptions_New PyPpcOptions_New_PROTO;

#else
/** Pointers to types and functions */
static void **PyPpcOptions_API;

/**
 * Returns a pointer to the internal ppc  options, remember to release the reference
 * when done with the object. (RAVE_OBJECT_RELEASE).
 */
#define PyPpcOptions_GetNative \
  (*(PyPpcOptions_GetNative_RETURN (*)PyPpcOptions_GetNative_PROTO) PyPpcOptions_API[PyPpcOptions_GetNative_NUM])

/**
 * Creates a new ppc  options instance. Release this object with Py_DECREF. If a PpcOptions_t instance is
 * provided and this instance already is bound to a python instance, this instance will be increfed and
 * returned.
 * @param[in] processor - the PpcOptions_t instance.
 * @returns the PyPpcOptions instance.
 */
#define PyPpcOptions_New \
  (*(PyPpcOptions_New_RETURN (*)PyPpcOptions_New_PROTO) PyPpcOptions_API[PyPpcOptions_New_NUM])

/**
 * Checks if the object is a python ppc  options instance.
 */
#define PyPpcOptions_Check(op) \
  (Py_TYPE(op) == &PyPpcOptions_Type)

#define PyPpcOptions_Type (*(PyTypeObject*)PyPpcOptions_API[PyPpcOptions_Type_NUM])

/**
 * Imports the PyPpcOptions module (like import _ppcoptions in python).
 */
#define import_ppcoptions() \
    PyPpcOptions_API = (void **)PyCapsule_Import(PyPpcOptions_CAPSULE_NAME, 1);

#endif

#endif /* PYPPCOPTIONS_H */
