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
 * Python API to the pdp processor functions
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#include "pyppc_compat.h"
#include "Python.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pyravedata2d.h"

#define PYPDPPROCESSOR_MODULE   /**< to get correct part in pypdpprocessor */
#include "pypdpprocessor.h"
#include "pyppcradaroptions.h"
#include "pypolarscan.h"
#include "pyrave_debug.h"
#include "rave_alloc.h"

/**
 * Debug this module
 */
PYRAVE_DEBUG_MODULE("_pdpprocessor");

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
/// PdpProcessor
/// --------------------------------------------------------------------
/*@{ PdpProcessor */
/**
 * Returns the native PdpProcessor_t instance.
 * @param[in] processor - the python pdp processor instance
 * @returns the native PdpProcessor_t instance.
 */
static PdpProcessor_t*
PyPdpProcessor_GetNative(PyPdpProcessor* processor)
{
  RAVE_ASSERT((processor != NULL), "processor == NULL");
  return RAVE_OBJECT_COPY(processor->processor);
}

/**
 * Creates a python pdp processor from a native pdp processor or will create an
 * initial native pdp processor if p is NULL.
 * @param[in] p - the native pdp processor (or NULL)
 * @returns the python fmi image.
 */
static PyPdpProcessor*
PyPdpProcessor_New(PdpProcessor_t* p)
{
  PyPdpProcessor* result = NULL;
  PdpProcessor_t* cp = NULL;

  if (p == NULL) {
    cp = RAVE_OBJECT_NEW(&PdpProcessor_TYPE);
    if (cp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for pdp processor.");
      raiseException_returnNULL(PyExc_MemoryError, "Failed to allocate memory for pdp processor.");
    }
  } else {
    cp = RAVE_OBJECT_COPY(p);
    result = RAVE_OBJECT_GETBINDING(p); // If p already have a binding, then this should only be increfed.
    if (result != NULL) {
      Py_INCREF(result);
    }
  }

  if (result == NULL) {
    result = PyObject_NEW(PyPdpProcessor, &PyPdpProcessor_Type);
    if (result != NULL) {
      PYRAVE_DEBUG_OBJECT_CREATED;
      result->processor = RAVE_OBJECT_COPY(cp);
      RAVE_OBJECT_BIND(result->processor, result);
    } else {
      RAVE_CRITICAL0("Failed to create PyPdpProcessor instance");
      raiseException_gotoTag(done, PyExc_MemoryError, "Failed to allocate memory for pdp processor.");
    }
  }

done:
  RAVE_OBJECT_RELEASE(cp);
  return result;
}

/**
 * Deallocates the pdp processor
 * @param[in] obj the object to deallocate.
 */
static void _pypdpprocessor_dealloc(PyPdpProcessor* obj)
{
  /*Nothing yet*/
  if (obj == NULL) {
    return;
  }
  PYRAVE_DEBUG_OBJECT_DESTROYED;
  RAVE_OBJECT_UNBIND(obj->processor, obj);
  RAVE_OBJECT_RELEASE(obj->processor);
  PyObject_Del(obj);
}

/**
 * Creates a new instance of the pdp processor
 * @param[in] self this instance.
 * @param[in] args arguments for creation
 * @return the object on success, otherwise NULL
 */
static PyObject* _pypdpprocessor_new(PyObject* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }
  return (PyObject*)PyPdpProcessor_New(NULL);
}

/**
 * See \ref PdpProcessor_texture
 * @param[in] self - self
 * @param[in] args -
 * @return self on success otherwise NULL
 */
static PyObject* _pypdpprocessor_texture(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyin = NULL;
  PyObject* result = NULL;
  RaveData2D_t* tresult = NULL;

  if (!PyArg_ParseTuple(args, "O", &pyin))
    return NULL;
  if (!PyRaveData2D_Check(pyin))
    raiseException_returnNULL(PyExc_AttributeError, "Inparameter must be of type RaveData2DCore");
  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyin)->field)) {

    raiseException_returnNULL(PyExc_AttributeError, "To create texture, nodata must be set");
  }

  tresult = PdpProcessor_texture(self->processor, ((PyRaveData2D*)pyin)->field);
  if (tresult == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to generate a texture");
  }

  result = (PyObject*)PyRaveData2D_New(tresult);
  RAVE_OBJECT_RELEASE(tresult);
  return result;
}

static PyObject* _pypdpprocessor_trap(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyin = NULL;
  RaveData2D_t* trapResult = NULL;
  PyObject* result = NULL;
  double a = 0.0, b = 0.0, s = 0.0, t = 0.0;

  if (!PyArg_ParseTuple(args, "Odddd", &pyin, &a, &b, &s, &t))
    return NULL;

  if (!PyRaveData2D_Check(pyin)) {
    raiseException_returnNULL(PyExc_AttributeError, "First argument must be a RaveData2DCore object");
  }

  trapResult = PdpProcessor_trap(self->processor, ((PyRaveData2D*)pyin)->field, a, b, s, t);
  if (trapResult == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run trap on x");
  }
  result = (PyObject*)PyRaveData2D_New(trapResult);
  RAVE_OBJECT_RELEASE(trapResult);
  return result;
}

static PyObject* _pypdpprocessor_clutterID(PyPdpProcessor* self, PyObject* args)
{
  PyObject *pyinZ = NULL, *pyinVRADH = NULL, *pyinTexturePHIDP = NULL;
  PyObject *pyinRHOHV = NULL, *pyinTextureZ = NULL, *pyinClutterMap = NULL;
  RaveData2D_t* clutterIDResult = NULL;
  PyObject* result = NULL;

  double nodataZ = 0.0, nodataVRADH = 0.0;

  if (!PyArg_ParseTuple(args, "OOOOOOdd", &pyinZ, &pyinVRADH, &pyinTexturePHIDP, &pyinRHOHV, &pyinTextureZ, &pyinClutterMap, &nodataZ, &nodataVRADH))
    return NULL;

  clutterIDResult = PdpProcessor_clutterID(self->processor, ((PyRaveData2D*)pyinZ)->field, ((PyRaveData2D*)pyinVRADH)->field,
      ((PyRaveData2D*)pyinTexturePHIDP)->field, ((PyRaveData2D*)pyinRHOHV)->field, ((PyRaveData2D*)pyinTextureZ)->field,
      ((PyRaveData2D*)pyinClutterMap)->field, nodataZ, nodataVRADH);

  if (clutterIDResult == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run clutter ID");
  }

  result = (PyObject*)PyRaveData2D_New(clutterIDResult);
  RAVE_OBJECT_RELEASE(clutterIDResult);
  return result;
}

static PyObject* _pypdpprocessor_clutterCorrection(PyPdpProcessor* self, PyObject* args)
{
  PyObject *pyinZ = NULL, *pyinVRADH = NULL, *pyinTexturePHIDP = NULL;
  PyObject *pyinRHOHV = NULL, *pyinTextureZ = NULL, *pyinClutterMap = NULL;
  RaveData2D_t *outZ = NULL, *outQuality = NULL, *outClutterMask = NULL;
  PyObject *pyoutZ = NULL, *pyoutQuality = NULL, *pyoutClutterMask = NULL;
  PyObject* result = NULL;
  double nodataZ = 0.0, nodataVRADH = 0.0, qualityThreshold = 0.0;

  if (!PyArg_ParseTuple(args, "OOOOOOddd", &pyinZ, &pyinVRADH, &pyinTexturePHIDP, &pyinRHOHV, &pyinTextureZ, &pyinClutterMap, &nodataZ, &nodataVRADH, &qualityThreshold))
    return NULL;

  if (!PdpProcessor_clutterCorrection(self->processor, ((PyRaveData2D*)pyinZ)->field, ((PyRaveData2D*)pyinVRADH)->field,
      ((PyRaveData2D*)pyinTexturePHIDP)->field, ((PyRaveData2D*)pyinRHOHV)->field, ((PyRaveData2D*)pyinTextureZ)->field,
      ((PyRaveData2D*)pyinClutterMap)->field, nodataZ, nodataVRADH, qualityThreshold, &outZ, &outQuality, &outClutterMask)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to generate clutter correction");
  }
  pyoutZ = (PyObject*)PyRaveData2D_New(outZ);
  pyoutQuality = (PyObject*)PyRaveData2D_New(outQuality);
  pyoutClutterMask = (PyObject*)PyRaveData2D_New(outClutterMask);
  if (pyoutZ == NULL || pyoutQuality == NULL || pyoutClutterMask == NULL) {
    Py_XDECREF(pyoutZ);
    Py_XDECREF(pyoutQuality);
    Py_XDECREF(pyoutClutterMask);
    raiseException_returnNULL(PyExc_RuntimeError, "Could not create python result");
  }

  result = Py_BuildValue("(OOO)", pyoutZ, pyoutQuality, pyoutClutterMask);
  RAVE_OBJECT_RELEASE(outZ);
  RAVE_OBJECT_RELEASE(outQuality);
  RAVE_OBJECT_RELEASE(outClutterMask);
  Py_XDECREF(pyoutZ);
  Py_XDECREF(pyoutQuality);
  Py_XDECREF(pyoutClutterMask);
  return result;
}

static PyObject* _pypdpprocessor_medfilt(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyinZ = NULL;
  double threshZ = -20.0;
  double nodataZ = -999;
  long filtXsize = 3, filtYsize = 3;
  RaveData2D_t* mask = NULL;
  PyObject* pyresult = NULL;
  if (!PyArg_ParseTuple(args, "O|dd(ll)", &pyinZ, &threshZ, &nodataZ, &filtXsize, &filtYsize))
    return NULL;
  if (!PyRaveData2D_Check(pyinZ)) {
    raiseException_returnNULL(PyExc_AttributeError, "First argument must be of type RaveData2DCore");
  }
  mask = PdpProcessor_medfilt(self->processor, (RaveData2D_t*)((PyRaveData2D*)pyinZ)->field, threshZ, nodataZ, filtXsize, filtYsize);
  if (mask == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to create filtered Z field");
  }
  pyresult = (PyObject*)PyRaveData2D_New(mask);
  RAVE_OBJECT_RELEASE(mask);
  return pyresult;
}

static PyObject* _pypdpprocessor_residualClutterFilter(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyinZ = NULL;
  double threshZ = -20.0, threshTexture = 20;
  long filtXsize = 3, filtYsize = 3;
  RaveData2D_t* mask = NULL;
  PyObject* pyresult = NULL;

  if (!PyArg_ParseTuple(args, "O|dd(ll)", &pyinZ, &threshZ, &threshTexture, &filtXsize, &filtYsize))
    return NULL;

  if (!PyRaveData2D_Check(pyinZ)) {
    raiseException_returnNULL(PyExc_AttributeError, "First argument must be of type RaveData2DCore");
  }
  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyinZ)->field)) {
    raiseException_returnNULL(PyExc_AttributeError, "Z must be defined to use nodata");
  }
  mask = PdpProcessor_residualClutterFilter(self->processor, (RaveData2D_t*)((PyRaveData2D*)pyinZ)->field, threshZ, threshTexture, filtXsize, filtYsize);
  if (mask == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to create residual clutter mask");
  }
  pyresult = (PyObject*)PyRaveData2D_New(mask);
  RAVE_OBJECT_RELEASE(mask);
  return pyresult;
}

/**
 * See \ref PdpProcessor_process
 * @param[in] self - self
 * @param[in] args -
 * @return self on success otherwise NULL
 */
static PyObject* _pypdpprocessor_process(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyin = NULL;
  PolarScan_t* resultScan = NULL;
  PyObject* pyresult = NULL;

  if (!PyArg_ParseTuple(args, "O", &pyin))
    return NULL;

  if (!PyPolarScan_Check(pyin)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Indata must be polar scan");
  }

  resultScan = PdpProcessor_process(self->processor, ((PyPolarScan*)pyin)->scan);
  if (resultScan == NULL) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to process scan");
  }
  pyresult = (PyObject*)PyPolarScan_New(resultScan);
  if (pyresult == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to create python polar scan");
  }
  RAVE_OBJECT_RELEASE(resultScan);
  return pyresult;
}

static PyObject* _pypdpprocessor_pdpProcessing(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyinPdp = NULL;
  PyObject *pypdpres = NULL, *pykdpres = NULL;
  RaveData2D_t* pdpres = NULL;
  RaveData2D_t* kdpres = NULL;
  PyObject* result = NULL;
  double dr;
  long window;
  long nrIter;

  if (!PyArg_ParseTuple(args, "Odll", &pyinPdp, &dr, &window, &nrIter))
    return NULL;

  if (!PyRaveData2D_Check(pyinPdp)) {
    raiseException_returnNULL(PyExc_AttributeError, "First argument must be of type RaveData2DCore");
  }

  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyinPdp)->field)) {
    raiseException_returnNULL(PyExc_AttributeError, "Pdp field must be defined to use nodata");
  }
  if (!PdpProcessor_pdpProcessing(self->processor, ((PyRaveData2D*)pyinPdp)->field, dr, window, nrIter, &pdpres, &kdpres)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run pdp processing");
  }
  pypdpres = (PyObject*)PyRaveData2D_New(pdpres);
  pykdpres = (PyObject*)PyRaveData2D_New(kdpres);

  result = Py_BuildValue("(OO)", pypdpres, pykdpres);

  RAVE_OBJECT_RELEASE(pdpres);
  RAVE_OBJECT_RELEASE(kdpres);
  Py_XDECREF(pypdpres);
  Py_XDECREF(pykdpres);
  return result;
}

static PyObject* _pypdpprocessor_pdpScript(PyPdpProcessor* self, PyObject* args)
{
  PyObject* pyinPdp = NULL;
  PyObject *pypdpres = NULL, *pykdpres = NULL;
  RaveData2D_t* pdpres = NULL;
  RaveData2D_t* kdpres = NULL;
  PyObject* result = NULL;
  double dr;
  double rWin1,rWin2;
  long nrIter;

  if (!PyArg_ParseTuple(args, "Odddl", &pyinPdp, &dr, &rWin1, &rWin2, &nrIter))
    return NULL;

  if (!PyRaveData2D_Check(pyinPdp)) {
    raiseException_returnNULL(PyExc_AttributeError, "First argument must be of type RaveData2DCore");
  }

  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyinPdp)->field)) {
    raiseException_returnNULL(PyExc_AttributeError, "Pdp field must be defined to use nodata");
  }

  if (!PdpProcessor_pdpScript(self->processor, ((PyRaveData2D*)pyinPdp)->field, dr, rWin1, rWin2, nrIter, &pdpres, &kdpres)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run pdp processing");
  }
  pypdpres = (PyObject*)PyRaveData2D_New(pdpres);
  pykdpres = (PyObject*)PyRaveData2D_New(kdpres);

  result = Py_BuildValue("(OO)", pypdpres, pykdpres);

  RAVE_OBJECT_RELEASE(pdpres);
  RAVE_OBJECT_RELEASE(kdpres);
  Py_XDECREF(pypdpres);
  Py_XDECREF(pykdpres);
  return result;
}

static PyObject* _pypdpprocessor_attenuation(PyPdpProcessor* self, PyObject* args)
{
  PyObject *pyinz = NULL, *pyinzdr = NULL, *pyindbzh = NULL, *pyinpdp = NULL, *pyinmask = NULL;
  RaveData2D_t *outz = NULL, *outzdr = NULL, *outpia = NULL, *outdbzh = NULL;
  PyObject *pyoutz = NULL, *pyoutzdr = NULL, *pyoutpia = NULL, *pyoutdbzh = NULL;;
  PyObject* result = NULL;
  double gamma_h = 0.0, alpha = 0.0;
  double zundetect = -32.0, dbzhundetect = -32.0;

  if (!PyArg_ParseTuple(args, "OOOOOdddd", &pyinz, &pyinzdr, &pyindbzh, &pyinpdp, &pyinmask, &gamma_h, &alpha, &zundetect, &dbzhundetect))
    return NULL;

  if (!PyRaveData2D_Check(pyinz) || !PyRaveData2D_Check(pyinzdr) || !PyRaveData2D_Check(pyindbzh) || !PyRaveData2D_Check(pyinpdp) || !PyRaveData2D_Check(pyinmask)) {
    raiseException_returnNULL(PyExc_AttributeError, "1st,2nd,3rd, 4th and 5th argument must be of type RaveData2DCore");
  }

  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyinpdp)->field) || !RaveData2D_usingNodata(((PyRaveData2D*)pyindbzh)->field)) {
    raiseException_returnNULL(PyExc_AttributeError, "pdp and dbzh field must be defined to use nodata");
  }

  if (!PdpProcessor_attenuation(self->processor, ((PyRaveData2D*)pyinz)->field, ((PyRaveData2D*)pyinzdr)->field,
      ((PyRaveData2D*)pyindbzh)->field, ((PyRaveData2D*)pyinpdp)->field, ((PyRaveData2D*)pyinmask)->field, gamma_h, alpha, zundetect, dbzhundetect,
      &outz, &outzdr, &outpia, &outdbzh)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run pdp processing");
  }
  pyoutz = (PyObject*)PyRaveData2D_New(outz);
  pyoutzdr = (PyObject*)PyRaveData2D_New(outzdr);
  pyoutpia = (PyObject*)PyRaveData2D_New(outpia);
  pyoutdbzh = (PyObject*)PyRaveData2D_New(outdbzh);
  if (pyoutz == NULL || pyoutzdr == NULL || pyoutpia == NULL || pyoutdbzh == NULL)  {
    raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to create rave data 2d python objects");
  }

  result = Py_BuildValue("(OOOO)", pyoutz, pyoutzdr, pyoutpia, pyoutdbzh);

done:
  RAVE_OBJECT_RELEASE(outz);
  RAVE_OBJECT_RELEASE(outzdr);
  RAVE_OBJECT_RELEASE(outpia);
  RAVE_OBJECT_RELEASE(outdbzh);
  Py_XDECREF(pyoutz);
  Py_XDECREF(pyoutzdr);
  Py_XDECREF(pyoutpia);
  Py_XDECREF(pyoutdbzh);

  return result;
}

static PyObject* _pypdpprocessor_zphi(PyPdpProcessor* self, PyObject* args)
{
  PyObject *pyinz = NULL, *pyinpdp = NULL, **pyinmask = NULL;
  RaveData2D_t *outzphi = NULL, *outah = NULL;
  PyObject *pyoutzphi = NULL, *pyoutah = NULL;
  PyObject* result = NULL;
  double dr = 0.0, BB = 0.0, gamma_h = 0.0;

  if (!PyArg_ParseTuple(args, "OOOddd", &pyinz, &pyinpdp, &pyinmask, &dr, &BB, &gamma_h))
    return NULL;

  if (!PyRaveData2D_Check(pyinz) || !PyRaveData2D_Check(pyinpdp) || !PyRaveData2D_Check(pyinmask)) {
    raiseException_returnNULL(PyExc_AttributeError, "1st,2nd and 3rd argument must be of type RaveData2DCore");
  }

  if (!RaveData2D_usingNodata(((PyRaveData2D*)pyinpdp)->field)) {
    raiseException_returnNULL(PyExc_AttributeError, "pdp field must be defined to use nodata");
  }

  if (!PdpProcessor_zphi(self->processor, ((PyRaveData2D*)pyinz)->field, ((PyRaveData2D*)pyinpdp)->field,
      ((PyRaveData2D*)pyinmask)->field, dr, BB, gamma_h, &outzphi, &outah)) {
    raiseException_returnNULL(PyExc_RuntimeError, "Failed to run zphi");
  }
  pyoutzphi = (PyObject*)PyRaveData2D_New(outzphi);
  pyoutah = (PyObject*)PyRaveData2D_New(outah);
  if (pyoutzphi == NULL || pyoutah == NULL)  {
    raiseException_gotoTag(done, PyExc_RuntimeError, "Failed to create rave data 2d python objects");
  }

  result = Py_BuildValue("(OO)", pyoutzphi, pyoutah);

done:
  RAVE_OBJECT_RELEASE(outzphi);
  RAVE_OBJECT_RELEASE(outah);
  Py_XDECREF(pyoutzphi);
  Py_XDECREF(pyoutah);

  return result;
}

/**
 * All methods a ropo generator can have
 */
static struct PyMethodDef _pypdpprocessor_methods[] =
{
  {"options", NULL},
  {"meltingLayerBottomHeight", NULL},
  {"texture", (PyCFunction)_pypdpprocessor_texture, 1},
  {"trap", (PyCFunction)_pypdpprocessor_trap, 1},
  {"clutterID", (PyCFunction)_pypdpprocessor_clutterID, 1},
  {"clutterCorrection", (PyCFunction)_pypdpprocessor_clutterCorrection, 1},
  {"medfilt", (PyCFunction)_pypdpprocessor_medfilt, 1},
  {"residualClutterFilter", (PyCFunction)_pypdpprocessor_residualClutterFilter, 1},
  {"process", (PyCFunction)_pypdpprocessor_process, 1},
  {"pdpProcessing", (PyCFunction)_pypdpprocessor_pdpProcessing, 1},
  {"pdpScript", (PyCFunction)_pypdpprocessor_pdpScript, 1},
  {"attenuation", (PyCFunction)_pypdpprocessor_attenuation, 1},
  {"zphi", (PyCFunction)_pypdpprocessor_zphi, 1},
  {NULL, NULL} /* sentinel */
};

/**
 * Returns the specified attribute in the pdp processor
 */
static PyObject* _pypdpprocessor_getattro(PyPdpProcessor* self, PyObject* name)
{
  if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "options") == 0) {
    PpcRadarOptions_t* options = PdpProcessor_getRadarOptions(self->processor);
    PyObject* result = (PyObject*)PyPpcRadarOptions_New(options);
    RAVE_OBJECT_RELEASE(options);
    return result;
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerBottomHeight") == 0) {
    return PyFloat_FromDouble(PdpProcessor_getMeltingLayerBottomHeight(self->processor));
  }

  return PyObject_GenericGetAttr((PyObject*)self, name);
}

/**
 * Sets the attribute
 */
static int _pypdpprocessor_setattro(PyPdpProcessor* self, PyObject* name, PyObject* val)
{
  int result = -1;
  if (name == NULL) {
    goto done;
  }
  if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "options") == 0) {
    if (PyPpcRadarOptions_Check(val)) {
      PpcRadarOptions_t* options = PyPpcRadarOptions_GetNative((PyPpcRadarOptions*)val);
      PdpProcessor_setRadarOptions(self->processor, options);
      RAVE_OBJECT_RELEASE(options);
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "options must be of type PpcRadarOptions");
    }
  } else if (PY_COMPARE_ATTRO_NAME_WITH_STRING(name, "meltingLayerBottomHeight") == 0) {
    if (PyFloat_Check(val)) {
      PdpProcessor_setMeltingLayerBottomHeight(self->processor, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      PdpProcessor_setMeltingLayerBottomHeight(self->processor, (double)PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      PdpProcessor_setMeltingLayerBottomHeight(self->processor, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_ValueError, "meltingLayerBottomHeight must be of type float or long");
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
PyTypeObject PyPdpProcessor_Type =
{
  PyVarObject_HEAD_INIT(NULL, 0) /*ob_size*/
  "PdpProcessor", /*tp_name*/
  sizeof(PyPdpProcessor), /*tp_size*/
  0, /*tp_itemsize*/
  /* methods */
  (destructor)_pypdpprocessor_dealloc, /*tp_dealloc*/
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
  (getattrofunc)_pypdpprocessor_getattro, /*tp_getattro*/
  (setattrofunc)_pypdpprocessor_setattro, /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT, /*tp_flags*/
  0,                            /*tp_doc*/
  (traverseproc)0,              /*tp_traverse*/
  (inquiry)0,                   /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  _pypdpprocessor_methods,              /*tp_methods*/
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
  {"new", (PyCFunction)_pypdpprocessor_new, 1},
  {NULL,NULL} /*Sentinel*/
};

MOD_INIT(_pdpprocessor)
{
  PyObject *module=NULL,*dictionary=NULL;
  static void *PyPdpProcessor_API[PyPdpProcessor_API_pointers];
  PyObject *c_api_object = NULL;

  MOD_INIT_SETUP_TYPE(PyPdpProcessor_Type, &PyType_Type);

  MOD_INIT_VERIFY_TYPE_READY(&PyPdpProcessor_Type);

  MOD_INIT_DEF(module, "_pdpprocessor", NULL/*doc*/, functions);
  if (module == NULL) {
    return MOD_INIT_ERROR;
  }

  PyPdpProcessor_API[PyPdpProcessor_Type_NUM] = (void*)&PyPdpProcessor_Type;
  PyPdpProcessor_API[PyPdpProcessor_GetNative_NUM] = (void *)PyPdpProcessor_GetNative;
  PyPdpProcessor_API[PyPdpProcessor_New_NUM] = (void*)PyPdpProcessor_New;


  c_api_object = PyCapsule_New(PyPdpProcessor_API, PyPdpProcessor_CAPSULE_NAME, NULL);
  dictionary = PyModule_GetDict(module);
  PyDict_SetItemString(dictionary, "_C_API", c_api_object);

  ErrorObject = PyErr_NewException("_pdpprocessor.error", NULL, NULL);
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _pdpprocessor.error");
    return MOD_INIT_ERROR;
  }

  import_pypolarscan();
  import_ppcradaroptions();
  import_ravedata2d();
  PYRAVE_DEBUG_INITIALIZE;
  return MOD_INIT_SUCCESS(module);
}

/*@} End of Module setup */
