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
 * Python API to the ppc radar options
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#include "pyppc_compat.h"
#include "Python.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PYPPCRADAROPTIONS_MODULE   /**< to get correct part in pyppcradaroptions */
#include "pyppcradaroptions.h"
#include "pyrave_debug.h"
#include "rave_alloc.h"

/**
 * Debug this module
 */
PYRAVE_DEBUG_MODULE("_ppcradaroptions");

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
/// PpcRadarOptions
/// --------------------------------------------------------------------
/*@{ PpcRadarOptions */
/**
 * Returns the native PpcRadarOptions_t instance.
 * @param[in] options - the python ppc radar options instance
 * @returns the native PpcRadarOptions_t instance.
 */
static PpcRadarOptions_t*
PyPpcRadarOptions_GetNative(PyPpcRadarOptions* options)
{
  RAVE_ASSERT((options != NULL), "options == NULL");
  return RAVE_OBJECT_COPY(options->options);
}

/**
 * Creates a python ppc radar options from a native ppc radar options or will create an
 * initial native ppc radar options processor if p is NULL.
 * @param[in] p - the native pdp processor (or NULL)
 * @returns the python fmi image.
 */
static PyPpcRadarOptions*
PyPpcRadarOptions_New(PpcRadarOptions_t* p)
{
  PyPpcRadarOptions* result = NULL;
  PpcRadarOptions_t* cp = NULL;

  if (p == NULL) {
    cp = RAVE_OBJECT_NEW(&PpcRadarOptions_TYPE);
    if (cp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for PpcRadarOptions.");
      raiseException_returnNULL(PyExc_MemoryError, "Failed to allocate memory for PpcRadarOptions.");
    }
  } else {
    cp = RAVE_OBJECT_COPY(p);
    result = RAVE_OBJECT_GETBINDING(p); // If p already have a binding, then this should only be increfed.
    if (result != NULL) {
      Py_INCREF(result);
    }
  }

  if (result == NULL) {
    result = PyObject_NEW(PyPpcRadarOptions, &PyPpcRadarOptions_Type);
    if (result != NULL) {
      PYRAVE_DEBUG_OBJECT_CREATED;
      result->options = RAVE_OBJECT_COPY(cp);
      RAVE_OBJECT_BIND(result->options, result);
    } else {
      RAVE_CRITICAL0("Failed to create PyPpcRadarOptions instance");
      raiseException_gotoTag(done, PyExc_MemoryError, "Failed to allocate memory for PpcRadarOptions.");
    }
  }

done:
  RAVE_OBJECT_RELEASE(cp);
  return result;
}

/**
 * Deallocates the ppc radar options
 * @param[in] obj the object to deallocate.
 */
static void _pyppcradaroptions_dealloc(PyPpcRadarOptions* obj)
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
 * Creates a new instance of the ppc radar options
 * @param[in] self this instance.
 * @param[in] args arguments for creation
 * @return the object on success, otherwise NULL
 */
static PyObject* _pyppcradaroptions_new(PyObject* self, PyObject* args)
{
  //PyObject* inptr = NULL;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }
  return (PyObject*)PyPpcRadarOptions_New(NULL);
}

/**
 * All methods a ppc radar options can have
 */
static struct PyMethodDef _pyppcradaroptions_methods[] =
{
  {"name", NULL},
  {"defaultName", NULL},
  {"minWindow", NULL},
  {"parametersUZ", NULL},
  {"parametersVEL", NULL},
  {"parametersTEXT_PHIDP", NULL},
  {"parametersRHV", NULL},
  {"parametersTEXT_UZ", NULL},
  {"parametersCLUTTER_MAP", NULL},
  {"nodata", NULL},
  {"minDBZ", NULL},
  {"qualityThreshold", NULL},
  {"preprocessZThreshold", NULL},
  {"kdpUp", NULL},
  {"kdpDown", NULL},
  {"kdpStdThreshold", NULL},
  {"BB", NULL},
  {"thresholdPhidp", NULL},
  {"requestedFields", NULL},
  {"residualMinZClutterThreshold", NULL},
  {"residualThresholdZ", NULL},
  {"residualThresholdTexture", NULL},
  {"residualClutterNodata", NULL},
  {"residualClutterMaskNodata", NULL},
  {"residualClutterTextureFilteringMaxZ", NULL},
  {"residualFilterBinSize", NULL},
  {"residualFilterRaySize", NULL},
  {"minAttenuationMaskRHOHV", NULL},
  {"minAttenuationMaskKDP", NULL},
  {"minAttenuationMaskTH", NULL},
  {"attenuationGammaH", NULL},
  {"attenuationAlpha", NULL},
  {"attenuationPIAminZ", NULL},
  {"pdpRWin1", NULL},
  {"pdpRWin2", NULL},
  {"pdpNrIterations", NULL},
  {"minZMedfilterThreshold", NULL},
  {"processingTextureThreshold", NULL},
  {NULL, NULL} /* sentinel */
};

/**
 * Returns the specified attribute in the ppc radar options
 */
static PyObject* _pyppcradaroptions_getattro(PyPpcRadarOptions* self, PyObject* name)
{
  if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "name") == 0) {
    return PyString_FromString(PpcRadarOptions_getName(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "defaultName") == 0) {
      return PyString_FromString(PpcRadarOptions_getDefaultName(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minWindow") == 0) {
    return PyInt_FromLong(PpcRadarOptions_getMinWindow(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersUZ") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersUZ(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersVEL") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersVEL(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersTEXT_PHIDP") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersTEXT_PHIDP(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersRHV") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersRHV(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersTEXT_UZ") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersTEXT_UZ(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersCLUTTER_MAP") == 0) {
    double weight, X2, X3, delta1, delta2;
    PpcRadarOptions_getParametersCLUTTER_MAP(self->options, &weight, &X2, &X3, &delta1, &delta2);
    return Py_BuildValue("(ddddd)", weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "nodata") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getNodata(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minDBZ") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMinDBZ(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "qualityThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getQualityThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpUp") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getKdpUp(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpDown") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getKdpDown(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpStdThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getKdpStdThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "requestedFields") == 0) {
    return PyLong_FromLong(PpcRadarOptions_getRequestedFields(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualMinZClutterThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualMinZClutterThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualThresholdZ") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualThresholdZ(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualThresholdTexture") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualThresholdTexture(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterNodata") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualClutterNodata(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterMaskNodata") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualClutterMaskNodata(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterTextureFilteringMaxZ") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getResidualClutterTextureFilteringMaxZ(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualFilterBinSize") == 0) {
    return PyLong_FromLong(PpcRadarOptions_getResidualFilterBinSize(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualFilterRaySize") == 0) {
    return PyLong_FromLong(PpcRadarOptions_getResidualFilterRaySize(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskRHOHV") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMinAttenuationMaskRHOHV(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskKDP") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMinAttenuationMaskKDP(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskTH") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMinAttenuationMaskTH(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationGammaH") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getAttenuationGammaH(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationAlpha") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getAttenuationAlpha(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationPIAminZ") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getAttenuationPIAminZ(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpRWin1") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getPdpRWin1(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpRWin2") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getPdpRWin2(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpNrIterations") == 0) {
    return PyLong_FromLong(PpcRadarOptions_getPdpNrIterations(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minZMedfilterThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMinZMedfilterThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "processingTextureThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getProcessingTextureThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "BB") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getBB(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "thresholdPhidp") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getThresholdPhidp(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "preprocessZThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getPreprocessZThreshold(self->options));
  }

  return PyObject_GenericGetAttr((PyObject*)self, name);
}

/**
 * Sets the attribute
 */
static int _pyppcradaroptions_setattro(PyPpcRadarOptions* self, PyObject* name, PyObject* val)
{
  int result = -1;
  if (name == NULL) {
    goto done;
  }
  if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "name") == 0) {
    if (PyString_Check(val)) {
      if (!PpcRadarOptions_setName(self->options, PyString_AsString(val))) {
        raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to set name");
      }
    } else if (val == Py_None) {
      if (!PpcRadarOptions_setName(self->options, NULL)) {
        raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to clear name");
      }
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "name must be of type string");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "defaultName") == 0) {
    if (PyString_Check(val)) {
      if (!PpcRadarOptions_setDefaultName(self->options, PyString_AsString(val))) {
        raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to set default name");
      }
    } else if (val == Py_None) {
      if (!PpcRadarOptions_setDefaultName(self->options, NULL)) {
        raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to clear default name");
      }
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "name must be of type string");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minWindow") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setMinWindow(self->options, PyLong_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "nodata must be of type int");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersUZ") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersUZ(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersVEL") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersVEL(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersTEXT_PHIDP") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersTEXT_PHIDP(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersRHV") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersRHV(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersTEXT_UZ") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersTEXT_UZ(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "parametersCLUTTER_MAP") == 0) {
    double weight, X2, X3, delta1, delta2;
    if (!PyArg_ParseTuple(val, "ddddd", &weight, &X2, &X3, &delta1, &delta2))
      goto done;
    PpcRadarOptions_setParametersCLUTTER_MAP(self->options, weight, X2, X3, delta1, delta2);
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "nodata") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setNodata(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setNodata(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setNodata(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "nodata must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minDBZ") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMinDBZ(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMinDBZ(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMinDBZ(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "minDBZ must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "qualityThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setQualityThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setQualityThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setQualityThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "qualityThreshold must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpUp") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setKdpUp(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setKdpUp(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setKdpUp(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "kdpUp must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpDown") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setKdpDown(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setKdpDown(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setKdpDown(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "kdpDown must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "kdpStdThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setKdpStdThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setKdpStdThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setKdpStdThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "kdpStdThreshold must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "requestedFields") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setRequestedFields(self->options, (int)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setRequestedFields(self->options, (int)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "requestedFields must be of integer");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualMinZClutterThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualMinZClutterThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualMinZClutterThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualMinZClutterThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualMinZClutterThreshold must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualThresholdZ") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualThresholdZ(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualThresholdZ(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualThresholdZ(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualThresholdZ must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualThresholdTexture") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualThresholdTexture(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualThresholdTexture(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualThresholdTexture(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualThresholdTexture must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterNodata") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualClutterNodata(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualClutterNodata(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualClutterNodata(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualClutterNodata must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterMaskNodata") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualClutterMaskNodata(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualClutterMaskNodata(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualClutterMaskNodata(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualClutterMaskNodata must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualClutterTextureFilteringMaxZ") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualClutterTextureFilteringMaxZ must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualFilterBinSize") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualFilterBinSize(self->options, (int)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualFilterBinSize(self->options, (int)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualFilterBinSize must be of integer");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "residualFilterRaySize") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setResidualFilterRaySize(self->options, (int)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setResidualFilterRaySize(self->options, (int)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "residualFilterRaySize must be of integer");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskRHOHV") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskRHOHV(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskRHOHV(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskRHOHV(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "minAttenuationMaskRHOHV must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskKDP") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskKDP(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskKDP(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskKDP(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "minAttenuationMaskKDP must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minAttenuationMaskTH") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskTH(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskTH(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMinAttenuationMaskTH(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "minAttenuationMaskTH must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationGammaH") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setAttenuationGammaH(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setAttenuationGammaH(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setAttenuationGammaH(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "attenuationGammaH must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationAlpha") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setAttenuationAlpha(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setAttenuationAlpha(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setAttenuationAlpha(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "attenuationAlpha must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "attenuationPIAminZ") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setAttenuationPIAminZ(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setAttenuationPIAminZ(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setAttenuationPIAminZ(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "attenuationPIAminZ must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpRWin1") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setPdpRWin1(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setPdpRWin1(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setPdpRWin1(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "pdpRWin1 must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpRWin2") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setPdpRWin2(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setPdpRWin2(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setPdpRWin2(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "pdpRWin2 must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "pdpNrIterations") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setPdpNrIterations(self->options, (int)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setPdpNrIterations(self->options, (int)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "pdpNrIterations must be of integer");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "minZMedfilterThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMinZMedfilterThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMinZMedfilterThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMinZMedfilterThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "minZMedfilterThreshold must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "processingTextureThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setProcessingTextureThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setProcessingTextureThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setProcessingTextureThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "processingTextureThreshold must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "BB") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setBB(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setBB(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setBB(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "BB must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "thresholdPhidp") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setThresholdPhidp(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setThresholdPhidp(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setThresholdPhidp(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "thresholdPhidp must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "preprocessZThreshold") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setPreprocessZThreshold(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setPreprocessZThreshold(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setPreprocessZThreshold(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "preprocessZThreshold must be of type float or long");
    }
  } else {
    raiseException_gotoTag(done, PyExc_AttributeError, PY_RAVE_ATTRO_NAME_TO_STRING(name));
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
PyTypeObject PyPpcRadarOptions_Type =
{
  PyVarObject_HEAD_INIT(NULL, 0) /*ob_size*/
  "PpcRadarOptions", /*tp_name*/
  sizeof(PyPpcRadarOptions), /*tp_size*/
  0, /*tp_itemsize*/
  /* methods */
  (destructor)_pyppcradaroptions_dealloc, /*tp_dealloc*/
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
  (getattrofunc)_pyppcradaroptions_getattro, /*tp_getattro*/
  (setattrofunc)_pyppcradaroptions_setattro, /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT, /*tp_flags*/
  0,                            /*tp_doc*/
  (traverseproc)0,              /*tp_traverse*/
  (inquiry)0,                   /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  _pyppcradaroptions_methods,              /*tp_methods*/
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
  {"new", (PyCFunction)_pyppcradaroptions_new, 1},
  {NULL,NULL} /*Sentinel*/
};

/**
 * Adds constants to the dictionary (probably the modules dictionary).
 * @param[in] dictionary - the dictionary the long should be added to
 * @param[in] name - the name of the constant
 * @param[in] value - the value
 */
static void add_long_constant(PyObject* dictionary, const char* name, long value)
{
  PyObject* tmp = NULL;
  tmp = PyInt_FromLong(value);
  if (tmp != NULL) {
    PyDict_SetItemString(dictionary, name, tmp);
  }
  Py_XDECREF(tmp);
}

MOD_INIT(_ppcradaroptions)
{
  PyObject *module=NULL,*dictionary=NULL;
  static void *PyPpcRadarOptions_API[PyPpcRadarOptions_API_pointers];
  PyObject *c_api_object = NULL;

  MOD_INIT_SETUP_TYPE(PyPpcRadarOptions_Type, &PyType_Type);

  MOD_INIT_VERIFY_TYPE_READY(&PyPpcRadarOptions_Type);

  MOD_INIT_DEF(module, "_ppcradaroptions", NULL/*doc*/, functions);
  if (module == NULL) {
    return MOD_INIT_ERROR;
  }

  PyPpcRadarOptions_API[PyPpcRadarOptions_Type_NUM] = (void*)&PyPpcRadarOptions_Type;
  PyPpcRadarOptions_API[PyPpcRadarOptions_GetNative_NUM] = (void *)PyPpcRadarOptions_GetNative;
  PyPpcRadarOptions_API[PyPpcRadarOptions_New_NUM] = (void*)PyPpcRadarOptions_New;


  c_api_object = PyCapsule_New(PyPpcRadarOptions_API, PyPpcRadarOptions_CAPSULE_NAME, NULL);
  dictionary = PyModule_GetDict(module);
  PyDict_SetItemString(dictionary, "_C_API", c_api_object);

  ErrorObject = PyErr_NewException("_ppcradaroptions.error", NULL, NULL);
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _ppcradaroptions.error");
    return MOD_INIT_ERROR;
  }

  add_long_constant(dictionary, "P_TH_CORR", PpcRadarOptions_TH_CORR);
  add_long_constant(dictionary, "P_ATT_TH_CORR", PpcRadarOptions_ATT_TH_CORR);
  add_long_constant(dictionary, "P_DBZH_CORR", PpcRadarOptions_DBZH_CORR);
  add_long_constant(dictionary, "P_ATT_DBZH_CORR", PpcRadarOptions_ATT_DBZH_CORR);
  add_long_constant(dictionary, "P_KDP_CORR", PpcRadarOptions_KDP_CORR);
  add_long_constant(dictionary, "P_RHOHV_CORR", PpcRadarOptions_RHOHV_CORR);
  add_long_constant(dictionary, "P_PHIDP_CORR", PpcRadarOptions_PHIDP_CORR);
  add_long_constant(dictionary, "P_ZDR_CORR", PpcRadarOptions_ZDR_CORR);
  add_long_constant(dictionary, "P_ZPHI_CORR", PpcRadarOptions_ZPHI_CORR);
  add_long_constant(dictionary, "Q_RESIDUAL_CLUTTER_MASK", PpcRadarOptions_QUALITY_RESIDUAL_CLUTTER_MASK);
  add_long_constant(dictionary, "Q_ATTENUATION_MASK", PpcRadarOptions_QUALITY_ATTENUATION_MASK);
  add_long_constant(dictionary, "Q_ATTENUATION", PpcRadarOptions_QUALITY_ATTENUATION);

  PYRAVE_DEBUG_INITIALIZE;
  return MOD_INIT_SUCCESS(module);
}

/*@} End of Module setup */
