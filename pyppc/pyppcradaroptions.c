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
 * Sets python exception and returns specified value
 */
#define raiseException_returnValue(val, type, msg) \
{PyErr_SetString(type, msg); return val;}

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


static PyObject* _pyppcradaroptions_setBand(PyPpcRadarOptions* self, PyObject* args)
{
  char* c;
  if (!PyArg_ParseTuple(args, "s", &c))
    return NULL;

  if (strlen(c) != 1 ||
      (c[0] != 's' && c[0] != 'c' && c[0] != 'x')) {
    raiseException_returnNULL(PyExc_ValueError, "Band must be either s, c or x");
  }

  if (!PpcRadarOptions_setBand(self->options, c[0])) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failure to set band");
  }

  Py_RETURN_NONE;
}

PyDoc_STRVAR(_pyppcro_setBand_doc,
    "Sets kdpUp, kdpDown and kdpStdThreshold based on what band.\n"
    "  band = 's' => kdpUp = 14, kdpDown = -2, kdpStdThreshold = 5\n"
    "  band = 'c' => kdpUp = 20, kdpDown = -2, kdpStdThreshold = 5\n"
    "  band = 'x' => kdpUp = 40, kdpDown = -2, kdpStdThreshold = 5\n"
    );

/**
 * All methods a ppc radar options can have
 */
static struct PyMethodDef _pyppcradaroptions_methods[] =
{
  {"name", NULL, METH_VARARGS, NULL},
  {"defaultName", NULL, METH_VARARGS, NULL},
  {"minWindow", NULL, METH_VARARGS, NULL},
  {"parametersUZ", NULL, METH_VARARGS, NULL},
  {"parametersVEL", NULL, METH_VARARGS, NULL},
  {"parametersTEXT_PHIDP", NULL, METH_VARARGS, NULL},
  {"parametersRHV", NULL, METH_VARARGS, NULL},
  {"parametersTEXT_UZ", NULL, METH_VARARGS, NULL},
  {"parametersCLUTTER_MAP", NULL, METH_VARARGS, NULL},
  {"nodata", NULL, METH_VARARGS, NULL},
  {"minDBZ", NULL, METH_VARARGS, NULL},
  {"qualityThreshold", NULL, METH_VARARGS, NULL},
  {"preprocessZThreshold", NULL, METH_VARARGS, NULL},
  {"kdpUp", NULL, METH_VARARGS, NULL},
  {"kdpDown", NULL, METH_VARARGS, NULL},
  {"kdpStdThreshold", NULL, METH_VARARGS, NULL},
  {"BB", NULL, METH_VARARGS, NULL},
  {"thresholdPhidp", NULL, METH_VARARGS, NULL},
  {"requestedFields", NULL, METH_VARARGS, NULL},
  {"residualMinZClutterThreshold", NULL, METH_VARARGS, NULL},
  {"residualThresholdZ", NULL, METH_VARARGS, NULL},
  {"residualThresholdTexture", NULL, METH_VARARGS, NULL},
  {"residualClutterNodata", NULL, METH_VARARGS, NULL},
  {"residualClutterMaskNodata", NULL, METH_VARARGS, NULL},
  {"residualClutterTextureFilteringMaxZ", NULL, METH_VARARGS, NULL},
  {"residualFilterBinSize", NULL, METH_VARARGS, NULL},
  {"residualFilterRaySize", NULL, METH_VARARGS, NULL},
  {"minAttenuationMaskRHOHV", NULL, METH_VARARGS, NULL},
  {"minAttenuationMaskKDP", NULL, METH_VARARGS, NULL},
  {"minAttenuationMaskTH", NULL, METH_VARARGS, NULL},
  {"attenuationGammaH", NULL, METH_VARARGS, NULL},
  {"attenuationAlpha", NULL, METH_VARARGS, NULL},
  {"attenuationPIAminZ", NULL, METH_VARARGS, NULL},
  {"pdpRWin1", NULL, METH_VARARGS, NULL},
  {"pdpRWin2", NULL, METH_VARARGS, NULL},
  {"pdpNrIterations", NULL, METH_VARARGS, NULL},
  {"minZMedfilterThreshold", NULL, METH_VARARGS, NULL},
  {"processingTextureThreshold", NULL, METH_VARARGS, NULL},
  {"meltingLayerBottomHeight", NULL, METH_VARARGS, NULL},
  {"meltingLayerHourThreshold", NULL, METH_VARARGS, NULL},
  {"invertPHIDP", NULL, METH_VARARGS, NULL},
  {"setBand", (PyCFunction)_pyppcradaroptions_setBand, METH_VARARGS, _pyppcro_setBand_doc},
  {NULL, NULL, 0, NULL} /* sentinel */
};

/**
 * Returns the specified attribute in the ppc radar options
 */
static PyObject* _pyppcradaroptions_getattro(PyPpcRadarOptions* self, PyObject* name)
{
  if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "name") == 0) {
    if (PpcRadarOptions_getName(self->options) == NULL) {
      Py_RETURN_NONE;
    }
    return PyString_FromString(PpcRadarOptions_getName(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "defaultName") == 0) {
    if (PpcRadarOptions_getDefaultName(self->options) == NULL) {
      Py_RETURN_NONE;
    }
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
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerBottomHeight") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMeltingLayerBottomHeight(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerHourThreshold") == 0) {
    return PyFloat_FromDouble(PpcRadarOptions_getMeltingLayerHourThreshold(self->options));
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "invertPHIDP") == 0) {
    return PyBool_FromLong(PpcRadarOptions_getInvertPHIDP(self->options));
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
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerBottomHeight") == 0) {
    if (PyFloat_Check(val)) {
      PpcRadarOptions_setMeltingLayerBottomHeight(self->options, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PpcRadarOptions_setMeltingLayerBottomHeight(self->options, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMeltingLayerBottomHeight(self->options, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "meltingLayerBottomHeight must be of type float or long");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerHourThreshold") == 0) {
    if (PyLong_Check(val)) {
      PpcRadarOptions_setMeltingLayerHourThreshold(self->options, (int)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PpcRadarOptions_setMeltingLayerHourThreshold(self->options, (int)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "meltingLayerHourThreshold must be of integer");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "invertPHIDP") == 0) {
    if (PyBool_Check(val)) {
      if (PyObject_IsTrue(val)) {
        PpcRadarOptions_setInvertPHIDP(self->options, 1);
      } else {
        PpcRadarOptions_setInvertPHIDP(self->options, 0);
      }
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "invertPHIDP must be of bool");
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

PyDoc_STRVAR(_pyppcro_object_definition_doc,
    "Keeps track of options used for the different radar sources when it comes to the polarimetric process chain\n"
    "processing. In this documentation section all the available options are listed for the different radars and a description of the values they assume.\n"
    "\n"
    "The following settings are available:\n"
    "parametersUZ                 - parameters for TH, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "parametersVEL                - parameters for VRADH, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "parametersTextPHIDP          - parameters for the PHIDP texture, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "parametersRHV                - parameters for RHOHV, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "parametersTextUZ             - parameters for the TH texture, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "parametersClutterMap         - parameters for the clutter map, 5 values separated by ',' Weight,X2,X3,Delta1,Delta2\n"
    "                               and the derived values will be X1=X2-Delta1, X3=X4-Delta2\n"
    "nodata                       - nodata to be used in most products\n"
    "minDBZ                       - min DBZ threshold in the clutter correction\n"
    "qualityThreshold             - quality threshold in the clutter correction\n"
    "preprocessZThreshold         - preprocessing Z threshold before starting actual processing\n"
    "residualMinZClutterThreshold - min z clutter threshold during residual clutter filtering\n"
    "residualThresholdZ           - min Z threshold in the residual clutter filtering\n"
    "residualThresholdTexture     - texture threshold in the residual clutter filtering\n"
    "residualClutterNodata        - the nodata value to be used when creating the residual clutter image used for creating the mask\n"
    "residualClutterMaskNodata    - Nodata value for the residual clutter mask\n"
    "residualClutterTextureFilteringMaxZ - Max Z value when creating the residual clutter mask, anything higher will be set to min value\n"
    "residualFilterBinSize        - number of bins used in the window when creating the residual mask\n"
    "residualFilterRaySize        - number of rays used in the window when creating the residual mask\n"
    "minZMedfilterThreshold       - min z threshold used in the median filter that is used by the residual clutter filter\n"
    "processingTextureThreshold   - threshold for the texture created in the pdp processing\n"
    "minWindow                    - min window size\n"
    "pdpRWin1                     - pdp ray window 1\n"
    "pdpRWin2                     - pdp ray window 2\n"
    "pdpNrIterations              - number of iterations in pdp processing\n"
    "kdpUp                        - Maximum allowed value of Kdp\n"
    "kdpDown                      - Minimum allowed value of kdp\n"
    "kdpStdThreshold              - Kdp STD threshold\n"
    "BB                           - BB value used in the zphi part of the pdp processing\n"
    "thresholdPhidp               - threshold for PHIDP in the pdp processing\n"
    "minAttenuationMaskRHOHV      - min RHOHV value for marking value as 1 in the attenuation mask\n"
    "minAttenuationMaskKDP        - min KDP value for marking value as 1 in the attenuation mask\n"
    "minAttenuationMaskTH         - min TH value for marking value as 1 in the attenuation mask\n"
    "attenuationGammaH            - gamma h value used in the attenuation\n"
    "attenuationAlpha             - alpha value used in the attenuation\n"
    "attenuationPIAminZ           - min PIA Z value in attenuation process\n"
    "meltingLayerBottomHeight     - The melting layer bottom height\n"
    "meltingLayerHourThreshold    - The number of hours before default height should be used.\n"
    "invertPHIDP                  - if the PHIDP should be inverted (multiplied with -1) or not. Typically this can be needed if the RSP produces inverted values.\n"
    "requestedFields              - '|' separated list of flags that defines what products should be added to the finished result.\n"
    "                               If the flag begins with a P, it means that the result is added as a parameter and the name of\n"
    "                               the parameter will be without the P_. If on the other hand the flag begins with a Q_ it means\n"
    "                               that the result is added as a quality field and in those cases the how/task name will be\n"
    "                               se.baltrad.ppc.<mask name without Q_ in lowercase>\n"
    "                               Available flags are:\n"
    "                                + P_TH_CORR\n"
    "                                + P_ATT_TH_CORR\n"
    "                                + P_DBZH_CORR\n"
    "                                + P_ATT_DBZH_CORR\n"
    "                                + P_KDP_CORR\n"
    "                                + P_RHOHV_CORR\n"
    "                                + P_PHIDP_CORR\n"
    "                                + P_ZDR_CORR\n"
    "                                + P_ZPHI_CORR\n"
    "                                + Q_RESIDUAL_CLUTTER_MASK\n"
    "                                + Q_ATTENUATION_MASK\n"
    "                                + Q_ATTENUATION\n"
    );


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
  _pyppcro_object_definition_doc, /*tp_doc*/
  (traverseproc)0,              /*tp_traverse*/
  (inquiry)0,                   /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  _pyppcradaroptions_methods,   /*tp_methods*/
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
  {"new", (PyCFunction)_pyppcradaroptions_new, METH_VARARGS, NULL},
  {NULL,NULL,0,NULL} /*Sentinel*/
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

  MOD_INIT_DEF(module, "_ppcradaroptions", _pyppcro_object_definition_doc/*doc*/, functions);
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
  add_long_constant(dictionary, "P_ATT_ZDR_CORR", PpcRadarOptions_ATT_ZDR_CORR);

  PYRAVE_DEBUG_INITIALIZE;
  return MOD_INIT_SUCCESS(module);
}

/*@} End of Module setup */
