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
 * Python version of the baltrad ppc generator
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#ifndef PYPDPPROCESSING_H
#define PYPDPPROCESSING_H
#include "pdp_processor.h"

/**
 * The fmi image object
 */
typedef struct {
   PyObject_HEAD /*Always have to be on top*/
   PdpProcessor_t* processor;  /**< the pdp processor */
} PyPdpProcessor;

#define PyPdpProcessor_Type_NUM 0                              /**< index of type */

#define PyPdpProcessor_GetNative_NUM 1                         /**< index of GetNative*/
#define PyPdpProcessor_GetNative_RETURN PdpProcessor_t*        /**< return type for GetNative */
#define PyPdpProcessor_GetNative_PROTO (PyPdpProcessor*)       /**< arguments for GetNative */

#define PyPdpProcessor_New_NUM 2                               /**< index of New */
#define PyPdpProcessor_New_RETURN PyPdpProcessor*              /**< return type for New */
#define PyPdpProcessor_New_PROTO (PdpProcessor_t*)             /**< arguments for New */

#define PyPdpProcessor_API_pointers 3                         /**< number of type and function pointers */

#define PyPdpProcessor_CAPSULE_NAME "_pdpprocessor._C_API"

#ifdef PYPDPPROCESSOR_MODULE
/** Forward declaration of type */
extern PyTypeObject PyPdpProcessor_Type;

/** Checks if the object is a PyPdpProcessor or not */
#define PyPdpProcessor_Check(op) ((op)->ob_type == &PyPdpProcessor_Type)

/** Forward declaration of PyPdpProcessor_GetNative */
static PyPdpProcessor_GetNative_RETURN PyPdpProcessor_GetNative PyPdpProcessor_GetNative_PROTO;

/** Forward declaration of PyPdpProcessor_New */
static PyPdpProcessor_New_RETURN PyPdpProcessor_New PyPdpProcessor_New_PROTO;

#else
/** Pointers to types and functions */
static void **PyPdpProcessor_API;

/**
 * Returns a pointer to the internal pdp processor, remember to release the reference
 * when done with the object. (RAVE_OBJECT_RELEASE).
 */
#define PyPdpProcessor_GetNative \
  (*(PyPdpProcessor_GetNative_RETURN (*)PyPdpProcessor_GetNative_PROTO) PyPdpProcessor_API[PyPdpProcessor_GetNative_NUM])

/**
 * Creates a new pdp processor instance. Release this object with Py_DECREF. If a PdpProcessor_t instance is
 * provided and this instance already is bound to a python instance, this instance will be increfed and
 * returned.
 * @param[in] processor - the PyPdpProcessor_t instance.
 * @returns the PyPdpProcessor instance.
 */
#define PyPdpProcessor_New \
  (*(PyPdpProcessor_New_RETURN (*)PyPdpProcessor_New_PROTO) PyPdpProcessor_API[PyPdpProcessor_New_NUM])

/**
 * Checks if the object is a python pdp processor.
 */
#define PyPdpProcessor_Check(op) \
  (Py_TYPE(op) == &PyPdpProcessor_Type)

#define PyPdpProcessor_Type (*(PyTypeObject*)PyPdpProcessor_API[PyPdpProcessor_Type_NUM])

/**
 * Imports the PyPdpProcessor module (like import _pdpprocessor in python).
 */
#define import_pdpprocessor() \
    PyPdpProcessor_API = (void **)PyCapsule_Import(PyPdpProcessor_CAPSULE_NAME, 1);

#endif

#endif /* PYPDPPROCESSOR_H */
