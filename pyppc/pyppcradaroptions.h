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
 * Python version of the ppc radar options class
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#ifndef PYPPCRADAROPTIONS_H
#define PYPPCRADAROPTIONS_H
#include "ppc_radar_options.h"

/**
 * The ppc radar objects class
 */
typedef struct {
   PyObject_HEAD /*Always have to be on top*/
   PpcRadarOptions_t* options;  /**< the options */
} PyPpcRadarOptions;

#define PyPpcRadarOptions_Type_NUM 0                              /**< index of type */

#define PyPpcRadarOptions_GetNative_NUM 1                         /**< index of GetNative*/
#define PyPpcRadarOptions_GetNative_RETURN PpcRadarOptions_t*        /**< return type for GetNative */
#define PyPpcRadarOptions_GetNative_PROTO (PyPpcRadarOptions*)       /**< arguments for GetNative */

#define PyPpcRadarOptions_New_NUM 2                               /**< index of New */
#define PyPpcRadarOptions_New_RETURN PyPpcRadarOptions*              /**< return type for New */
#define PyPpcRadarOptions_New_PROTO (PpcRadarOptions_t*)             /**< arguments for New */

#define PyPpcRadarOptions_API_pointers 3                         /**< number of type and function pointers */

#define PyPpcRadarOptions_CAPSULE_NAME "_ppcradaroptions._C_API"

#ifdef PYPPCRADAROPTIONS_MODULE
/** Forward declaration of type */
extern PyTypeObject PyPpcRadarOptions_Type;

/** Checks if the object is a PyPdpProcessor or not */
#define PyPpcRadarOptions_Check(op) ((op)->ob_type == &PyPpcRadarOptions_Type)

/** Forward declaration of PyPdpProcessor_GetNative */
static PyPpcRadarOptions_GetNative_RETURN PyPpcRadarOptions_GetNative PyPpcRadarOptions_GetNative_PROTO;

/** Forward declaration of PyPdpProcessor_New */
static PyPpcRadarOptions_New_RETURN PyPpcRadarOptions_New PyPpcRadarOptions_New_PROTO;

#else
/** Pointers to types and functions */
static void **PyPpcRadarOptions_API;

/**
 * Returns a pointer to the internal ppc radar options, remember to release the reference
 * when done with the object. (RAVE_OBJECT_RELEASE).
 */
#define PyPpcRadarOptions_GetNative \
  (*(PyPpcRadarOptions_GetNative_RETURN (*)PyPpcRadarOptions_GetNative_PROTO) PyPpcRadarOptions_API[PyPpcRadarOptions_GetNative_NUM])

/**
 * Creates a new ppc radar options instance. Release this object with Py_DECREF. If a PpcRadarOptions_t instance is
 * provided and this instance already is bound to a python instance, this instance will be increfed and
 * returned.
 * @param[in] processor - the PpcRadarOptions_t instance.
 * @returns the PyPpcRadarOptions instance.
 */
#define PyPpcRadarOptions_New \
  (*(PyPpcRadarOptions_New_RETURN (*)PyPpcRadarOptions_New_PROTO) PyPpcRadarOptions_API[PyPpcRadarOptions_New_NUM])

/**
 * Checks if the object is a python ppc radar options instance.
 */
#define PyPpcRadarOptions_Check(op) \
  (Py_TYPE(op) == &PyPpcRadarOptions_Type)

#define PyPpcRadarOptions_Type (*(PyTypeObject*)PyPpcRadarOptions_API[PyPpcRadarOptions_Type_NUM])

/**
 * Imports the PyPpcRadarOptions module (like import _ppcradaroptions in python).
 */
#define import_ppcradaroptions() \
    PyPpcRadarOptions_API = (void **)PyCapsule_Import(PyPpcRadarOptions_CAPSULE_NAME, 1);

#endif

#endif /* PYPPCRADAROPTIONS_H */
