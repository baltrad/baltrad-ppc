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
 * Main routine for the pdp processing based on the algorithm from Vulpiani et al. (2012)
 * This object does support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-02-17
 */
#include "pdp_processor.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_utilities.h"
#include <string.h>
#include <math.h>
#include <rave_data2d.h>
#include <polarvolume.h>
#include <time.h>
#include <sys/time.h>
#include "ppc_radar_options.h"

/**
 * Represents one transformator
 */
struct _PdpProcessor_t {
  RAVE_OBJECT_HEAD /** Always on top */
  double meltingLayerBottomHeight;
  PpcRadarOptions_t* options; /**< the processing options */
};

/*@{ Private functions */
/**
 * Constructor
 */
static int PdpProcessor_constructor(RaveCoreObject* obj)
{
	PdpProcessor_t* pdp = (PdpProcessor_t*)obj;
	pdp->meltingLayerBottomHeight = -1.0;
	pdp->options = RAVE_OBJECT_NEW(&PpcRadarOptions_TYPE);
	if (pdp->options == NULL) {
	  return 0;
	}
  return 1;
}

/**
 * Destructor
 */
static void PdpProcessor_destructor(RaveCoreObject* obj)
{
  RAVE_OBJECT_RELEASE(((PdpProcessor_t*)obj)->options);
}

/**
 * Copy constructor
 */
static int PdpProcessor_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PdpProcessor_t* this = (PdpProcessor_t*)obj;
  PdpProcessor_t* src = (PdpProcessor_t*)srcobj;
  int result = 0;
  this->meltingLayerBottomHeight = src->meltingLayerBottomHeight;
  this->options = RAVE_OBJECT_CLONE(src->options);
  if (this->options == NULL) {
    goto fail;
  }
  result = 1;
fail:
  RAVE_OBJECT_RELEASE(this->options);
  return result;
}

/**
 * Creates a polar scan param from a data 2d field.
 * @param[in] data2d - the 2d data field
 * @param[in] quantity - the quantity for this field
 * @param[in] touchar - if 1, then data will be scaled to 0-255
 * @param[in] nodata - the nodata value that the param should get
 * @param[in] undetect - the undetect value that the param should get
 * @returns the polar scan parameter
 */
static PolarScanParam_t* PdpProcessorInternal_createPolarScanParamFromData2D(RaveData2D_t* data2d, const char* quantity, int touchar, double nodata, double undetect)
{
  PolarScanParam_t* param = NULL;
  PolarScanParam_t* result = NULL;

  if (data2d == NULL || quantity == NULL) {
    RAVE_ERROR0("data2d or quantity is NULL");
    goto done;
  }

  param = RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  if (param == NULL) {
    goto done;
  }

  if (touchar) {
    double minv, maxv, spread, gain, offset, v;
    long bi, ri, nbins, nrays;
    int usingNodata = 0;
    double fieldNodata;
    nbins = RaveData2D_getXsize(data2d);
    nrays = RaveData2D_getYsize(data2d);
    if (!PolarScanParam_createData(param, nbins, nrays, RaveDataType_UCHAR)) {
      goto done;
    }
    minv = RaveData2D_min(data2d);
    maxv = RaveData2D_max(data2d);

    spread = (maxv - minv) / 254;
    offset = minv;
    gain = spread;
    if (gain == 0.0) {
      RAVE_ERROR0("Gain = 0.0");
      goto done;
    }
    PolarScanParam_setNodata(param, nodata);
    PolarScanParam_setUndetect(param, 0.0);
    PolarScanParam_setOffset(param, offset);
    PolarScanParam_setGain(param, gain);
    fieldNodata = RaveData2D_getNodata(data2d);
    usingNodata = RaveData2D_usingNodata(data2d);

    for (bi = 0; bi < nbins; bi++) {
      for (ri = 0; ri < nrays; ri++) {
        RaveData2D_getValueUnchecked(data2d, bi, ri, &v);
        if (!usingNodata || fieldNodata != v) {
          PolarScanParam_setValue(param, bi, ri, (v - offset)/gain);
        } else {
          PolarScanParam_setValue(param, bi, ri, nodata);
        }
      }
    }
  } else {
    if (!PolarScanParam_setData2D(param, data2d)) {
      goto done;
    }
    PolarScanParam_setNodata(param, nodata);
    PolarScanParam_setUndetect(param, undetect);
    PolarScanParam_setOffset(param, 0.0);
    PolarScanParam_setGain(param, 1.0);
  }

  if (!PolarScanParam_setQuantity(param, quantity)) {
    goto done;
  }

  result = RAVE_OBJECT_COPY(param);
done:
  RAVE_OBJECT_RELEASE(param);
  return result;
}

/**
 * Adds a data 2d field as a quality field to provided scan
 * @param[in] scan - the scan that should get the quality field associated
 * @param[in] data2d - the 2d data field
 * @param[in] qualityName - the how/task name
 * @returns 1 on success otherwise 0
 */
static int PdpProcessorInternal_addRaveQualityFieldToScanFromData2D(PolarScan_t* scan, RaveData2D_t* data2d, const char* qualityName)
{
  int result = 0;
  RaveField_t* field = NULL;
  RaveAttribute_t *attr = NULL, *gainAttr = NULL, *offsetAttr = NULL;
  long nrays, nbins, bi, ri;
  double minv, maxv;
  double spread = 0.0, gain = 0.0, offset = 0.0;
  int usingNodata = 0;
  double nodata;

  if (scan == NULL || data2d == NULL) {
    RAVE_ERROR0("scan or data2d is NULL");
    goto done;
  }
  field = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (field == NULL) {
    goto done;
  }
  if (!RaveField_createData(field, RaveData2D_getXsize(data2d), RaveData2D_getYsize(data2d), RaveDataType_UCHAR)) {
    goto done;
  }

  minv = RaveData2D_min(data2d);
  maxv = RaveData2D_max(data2d);

  spread = (maxv - minv) / 254;
  offset = minv;
  gain = spread;

  if (gain == 0.0) {
    RAVE_ERROR0("gain = 0.0");
    goto done;
  }
  attr = RaveAttributeHelp_createString("how/task", qualityName);
  gainAttr = RaveAttributeHelp_createDouble("what/gain", gain);
  offsetAttr = RaveAttributeHelp_createDouble("what/offset", offset);

  if (attr == NULL || gainAttr == NULL || offsetAttr == NULL) {
    goto done;
  }
  if (!RaveField_addAttribute(field, attr) || !RaveField_addAttribute(field, gainAttr) || !RaveField_addAttribute(field, offsetAttr)) {
    goto done;
  }
  nrays = PolarScan_getNrays(scan);
  nbins = PolarScan_getNbins(scan);
  nodata = RaveData2D_getNodata(data2d);
  usingNodata = RaveData2D_usingNodata(data2d);

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(data2d, bi, ri, &v);
      if (!usingNodata || nodata != v) {
        RaveField_setValue(field, bi, ri, (v - offset)/gain);
      } else {
        RaveField_setValue(field, bi, ri, 255);
      }
    }
  }
  if (!PolarScan_addQualityField(scan, field)) {
    goto done;
  }
  result = 1;
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(gainAttr);
  RAVE_OBJECT_RELEASE(offsetAttr);

  return result;
}

/**
 * Returns the parameter data field as a converted data 2d field
 * @param[in] param - the scan param
 * @param[in] nodata - the nodata value that should be used for the returned data 2d field
 * @returns the data 2d field on success otherwise NULL
 */
RaveData2D_t* PdpProcessorInternal_getData2DFromParam(PolarScanParam_t* param, double nodata)
{
  RaveData2D_t *result = NULL, *data2d = NULL;
  if (param != NULL) {
    long nrays = PolarScanParam_getNrays(param);
    long nbins = PolarScanParam_getNbins(param);

    data2d = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
    if (data2d != NULL) {
      long ri, bi;
      RaveData2D_setNodata(data2d, nodata);
      RaveData2D_useNodata(data2d, 1);
      for (ri = 0; ri < nrays; ri++) {
        for (bi = 0; bi < nbins; bi++) {
          double v = 0.0;
          RaveValueType t = PolarScanParam_getConvertedValue(param, bi, ri, &v);
          if (t == RaveValueType_DATA) {
            RaveData2D_setValueUnchecked(data2d, bi, ri, v);
          } else if (t == RaveValueType_UNDETECT) {
            RaveData2D_setValueUnchecked(data2d, bi, ri, PolarScanParam_getUndetect(param)*PolarScanParam_getGain(param) + PolarScanParam_getOffset(param));
          } else {
            RaveData2D_setValueUnchecked(data2d, bi, ri, nodata);
          }
        }
      }
      result = RAVE_OBJECT_COPY(data2d);
    }
  }
  RAVE_OBJECT_RELEASE(data2d);
  return result;
}

/**
 * @returns the current time in milliseconds since epoch
 */
static long long PdpProcessorInternal_timestamp(void) {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

/*@} End of Private functions */

/*@{ Interface functions */
//void PdpProcessor_setRequestedFields(PdpProcessor_t* self, int fieldmask)
//{
//  RAVE_ASSERT((self != NULL), "self == NULL");
//  PpcRadarOptions_setRequestedFields(self->options, fieldmask);
//  //self->requestedFieldMask = fieldmask;
//
//int PdpProcessor_getRequestedFields(PdpProcessor_t* self)
//{
//  RAVE_ASSERT((self != NULL), "self == NULL");
//  return PpcRadarOptions_getRequestedFields(self->options);
//}

int PdpProcessor_setRadarOptions(PdpProcessor_t* self, PpcRadarOptions_t* options)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (options == NULL) {
    return 0;
  }
  RAVE_OBJECT_RELEASE(self->options);
  self->options = RAVE_OBJECT_COPY(options);
  return 1;
}

PpcRadarOptions_t* PdpProcessor_getRadarOptions(PdpProcessor_t* self)
{
  return RAVE_OBJECT_COPY(self->options);
}

static void disp_int(RaveData2D_t* field, int bmin_limit, int rmin_limit, int bmax_limit, int rmax_limit)
{
  int bi, ri, nbins, nrays;
  nbins = RaveData2D_getXsize(field);
  nrays = RaveData2D_getYsize(field);
  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(field, bi, ri, &v);
      if (bi>=bmin_limit && bi <bmax_limit && ri>=rmin_limit&&ri<rmax_limit) {
        fprintf(stderr, "%f   ", v);
      }
    }
    if (bi>=bmin_limit && bi <bmax_limit) { fprintf(stderr, "\n"); }
  }
  fprintf(stderr, "\n");
}

static void disp_sint(const char* msg, RaveData2D_t* field, int bmin_limit, int rmin_limit, int bmax_limit, int rmax_limit)
{
  fprintf(stderr, "%s\n", msg);
  disp_int(field, bmin_limit, rmin_limit, bmax_limit, rmax_limit);
}

PolarScan_t* PdpProcessor_process(PdpProcessor_t* self, PolarScan_t* scan, RaveData2D_t* sclutterMap)
{
  PolarScan_t *result = NULL, *tmpresult = NULL;
  double elangle = 0.0;
  double range = 0.0, rangeKm = 0.0;
  long nbins = 0, nrays = 0;
  long bi = 0, ri = 0;
  double nodataPHIDP = 0.0, nodataTH = 0.0, nodataZDR = 0.0, nodataDBZH = 0.0, nodataRHOHV = 0.0;
  double flag = -999.9;
  double undetectTH = 0.0;
  RaveData2D_t *dataTH = NULL, *dataZDR = NULL, *dataDV = NULL, *texturePHIDP = NULL, *dataDBZH = NULL;
  RaveData2D_t *dataRHOHV = NULL, *textureZ = NULL, *dataPHIDP = NULL, *dataPDP = NULL;
  RaveData2D_t *clutterMap = NULL, *residualClutterMask = NULL;
  RaveData2D_t *outZ = NULL, *outQuality = NULL, *outClutterMask = NULL;
  RaveData2D_t *outPDP = NULL, *outKDP = NULL, *attenuationMask = NULL;
  RaveData2D_t *outAttenuationZ = NULL, *outAttenuationZDR = NULL, *outAttenuationPIA = NULL, *outAttenuationDBZH = NULL;
  RaveData2D_t *outZPHI = NULL, *outAH = NULL;
  RaveData2D_t *thThresholdIndex = NULL;
  RaveField_t* pdpQualityField = NULL;
  PolarScanParam_t *correctedZ = NULL, *correctedZDR = NULL, *attCorrectedZDR = NULL, *correctedZPHI = NULL, *attenuatedZ = NULL, *correctedDBZH = NULL, *attenuatedDBZH = NULL;
  PolarScanParam_t *paramKDP = NULL, *paramRHOHV = NULL, *correctedPDP = NULL;
  PolarNavigator_t* navigator = NULL;
  PolarScanParam_t *TH = NULL, *ZDR = NULL, *DV = NULL, *PHIDP = NULL, *RHOHV = NULL, *DBZH = NULL;
  double nodata, qualityThreshold, residualClutterMaskNodata, minAttenuationMaskRHOHV, minAttenuationMaskKDP, minAttenuationMaskTH;

  long starttime = PdpProcessorInternal_timestamp();

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (scan == NULL) {
    RAVE_ERROR0("No scan provided");
    goto done;
  }

  nodata = PpcRadarOptions_getNodata(self->options);
  navigator = PolarScan_getNavigator(scan);

  elangle = PolarScan_getElangle(scan);

  range = PolarScan_getRscale(scan);
  rangeKm =  range / 1000.0;
  nbins = PolarScan_getNbins(scan);
  nrays = PolarScan_getNrays(scan);

  TH = PolarScan_getParameter(scan, "TH");
  ZDR = PolarScan_getParameter(scan, "ZDR");
  DV = PolarScan_getParameter(scan, "VRADH");
  PHIDP = PolarScan_getParameter(scan, "PHIDP");
  RHOHV = PolarScan_getParameter(scan, "RHOHV");
  DBZH = PolarScan_getParameter(scan, "DBZH");
  if (TH == NULL || ZDR == NULL || DV == NULL || PHIDP == NULL || RHOHV == NULL || DBZH == NULL) {
    RAVE_ERROR0("Can not generate PPC product since one or more of TH, ZDR, DV, PHIDP, RHOHV and DBZH is missing");
    goto done;
  }

  dataTH = PdpProcessorInternal_getData2DFromParam(TH, nodata);
  dataZDR = PdpProcessorInternal_getData2DFromParam(ZDR, nodata);
  dataDV = PdpProcessorInternal_getData2DFromParam(DV, nodata);
  dataPHIDP = PdpProcessorInternal_getData2DFromParam(PHIDP, nodata);
  dataRHOHV = PdpProcessorInternal_getData2DFromParam(RHOHV, nodata);
  dataDBZH = PdpProcessorInternal_getData2DFromParam(DBZH, PolarScanParam_getNodata(DBZH));
  if (dataTH == NULL || dataZDR == NULL || dataDV == NULL || dataPHIDP == NULL || dataRHOHV == NULL || dataDBZH == NULL) {
    RAVE_ERROR0("Can not generate PPC product since one or more of data fields for TH, ZDR, DV, PHIDP, RHOHV and DBZH not could be retrieved");
    goto done;
  }

  if (sclutterMap != NULL) {
    if (RaveData2D_getXsize(sclutterMap) != nbins || RaveData2D_getYsize(sclutterMap) != nrays) {
      RAVE_ERROR0("Clutter Map dimension doesn't match number of rays/bins rays == ysize, bins = xsize");
      goto done;
    }
    clutterMap = RAVE_OBJECT_COPY(sclutterMap);
  } else {
    clutterMap = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
    if (clutterMap == NULL) {
      RAVE_ERROR0("Could not create clutter map");
      goto done;
    }
    RaveData2D_useNodata(clutterMap, 1);
    RaveData2D_setNodata(clutterMap, 0.0);
  }

  if (PpcRadarOptions_getInvertPHIDP(self->options) == 1) {
    dataPDP = RaveData2D_mulNumber(dataPHIDP, -1.0); /** RSP produces inverted data */
  } else {
    dataPDP = RaveData2D_mulNumber(dataPHIDP, 1.0); /* Really don't do anything.*/
  }
  if (dataPDP == NULL) {
    RAVE_ERROR0("Failed to multiplicate PHIDP");
    goto done;
  }
  nodataPHIDP = nodata;
  nodataTH = nodata;
  nodataDBZH = PolarScanParam_getNodata(DBZH);
  nodataZDR = nodata;
  nodataRHOHV = nodata;
  undetectTH = PolarScanParam_getUndetect(TH)*PolarScanParam_getGain(TH) + PolarScanParam_getOffset(TH);

  thThresholdIndex = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
  if (thThresholdIndex == NULL) {
    goto done;
  }
  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v;
      RaveData2D_getValueUnchecked(dataTH, bi, ri, &v);
      if (v < PpcRadarOptions_getPreprocessZThreshold(self->options)) {
        RaveData2D_setValueUnchecked(thThresholdIndex, bi, ri, 1.0);
        RaveData2D_setValueUnchecked(dataTH, bi, ri, nodataTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, nodataZDR);
        RaveData2D_setValueUnchecked(dataPDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataPHIDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, nodataRHOHV);
      }
    }
  }

  texturePHIDP = PdpProcessor_texture(self, dataPDP);

  textureZ = PdpProcessor_texture(self, dataTH);

  /**************************************************************
   * Clutter removal by using a Fuzzy Logic Approach
   **************************************************************/
  qualityThreshold = PpcRadarOptions_getQualityThreshold(self->options);

  RAVE_DEBUG1("clutterMap: %d", RaveData2D_usingNodata(clutterMap));
  if (!RaveData2D_usingNodata(clutterMap)) {
    RAVE_ERROR0("Static clutter map doesn't specify nodata!");
  }
  if (!PdpProcessor_clutterCorrection(self, dataTH, dataDV, texturePHIDP, dataRHOHV, textureZ, clutterMap,
        PolarScanParam_getNodata(TH), PolarScanParam_getNodata(DV), qualityThreshold,
        &outZ, &outQuality, &outClutterMask)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(outZ); /* Not used in matlab */

  //disp_sint("QualityMap:", outQuality, 14, 153, 18, 158);
  //disp_sint("QualityMap:", outQuality, 153, 14, 158, 18);

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(outQuality, bi, ri, &v);
      if (v < qualityThreshold) {
        RaveData2D_setValueUnchecked(dataTH, bi, ri, undetectTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, nodataZDR);
        RaveData2D_setValueUnchecked(dataPHIDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataPDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, nodataRHOHV);
        RaveData2D_setValueUnchecked(dataDBZH, bi, ri, nodataDBZH);
      }
    }
  }

  /**************************************************************
   * MEDIAN FILTERING TO REMOVE RESIDUAL ISOLATED PIXELS AFFECTED BY CLUTTER
   **************************************************************/
  residualClutterMask = PdpProcessor_residualClutterFilter(self, dataTH,
      PpcRadarOptions_getResidualThresholdZ(self->options),
      PpcRadarOptions_getResidualThresholdTexture(self->options),
      PpcRadarOptions_getResidualFilterBinSize(self->options),
      PpcRadarOptions_getResidualFilterRaySize(self->options));
  if (residualClutterMask == NULL) {
    goto done;
  }

  /**************************************************************
   * PHIDP Filtering and Kdp retrieval
   **************************************************************/

  if (!PdpProcessor_pdpScript(self, dataPDP, rangeKm,
      PpcRadarOptions_getPdpRWin1(self->options),
      PpcRadarOptions_getPdpRWin2(self->options),
      PpcRadarOptions_getPdpNrIterations(self->options), &outPDP, &outKDP)) {
    goto done;
  }
  residualClutterMaskNodata = PpcRadarOptions_getResidualClutterMaskNodata(self->options);

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0, tv = 0.0;
      RaveData2D_getValueUnchecked(residualClutterMask, bi, ri, &v);
      if (v == 0.0 || v == residualClutterMaskNodata) {
        RaveData2D_setValueUnchecked(dataTH, bi, ri, undetectTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, flag);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, flag);
        RaveData2D_setValueUnchecked(dataDV, bi, ri, flag);
      }

      RaveData2D_getValueUnchecked(thThresholdIndex, bi, ri, &tv);
      RaveData2D_getValueUnchecked(dataTH, bi, ri, &v);
      if (tv == 1.0 || v < -900.0) {
        RaveData2D_setValueUnchecked(outPDP, bi, ri, undetectTH);
      }
    }
  }
  //disp_sint("PDP after filter:", outPDP, 350, 20, 370, 40);

  /**************************************************************
   * Attenuation correction using a linear approach (Bringi et al., 1990)
   **************************************************************/
  attenuationMask = RaveData2D_zeros(bi, ri, RaveDataType_DOUBLE);
  if (attenuationMask == NULL) {
    RAVE_ERROR0("Failed to create attenuation mask");
    goto done;
  }
  minAttenuationMaskRHOHV = PpcRadarOptions_getMinAttenuationMaskRHOHV(self->options);
  minAttenuationMaskKDP = PpcRadarOptions_getMinAttenuationMaskKDP(self->options);
  minAttenuationMaskTH = PpcRadarOptions_getMinAttenuationMaskTH(self->options);


  for (bi = 0; bi < nbins; bi++) {
    double d = 0.0, h = 0.0;
    double vRHOHV = 0, vKDP = 0, vTH = 0;
    PolarNavigator_reToDh(navigator, range * ((double)bi+0.5), elangle, &d, &h);
    h = h / 1000.0;
    if (h < PdpProcessor_getMeltingLayerBottomHeight(self)) {
      for (ri = 0; ri < nrays; ri++) {
        RaveData2D_getValueUnchecked(dataRHOHV, bi, ri, &vRHOHV);
        RaveData2D_getValueUnchecked(outKDP, bi, ri, &vKDP);
        RaveData2D_getValueUnchecked(dataTH, bi, ri, &vTH);
        if (vRHOHV > minAttenuationMaskRHOHV && vKDP > minAttenuationMaskKDP && vTH > minAttenuationMaskTH) {
          RaveData2D_setValueUnchecked(attenuationMask, bi, ri, 1.0);
        }
      }
    }
  }
  if (!PdpProcessor_attenuation(self, dataTH, dataZDR, dataDBZH, outPDP, attenuationMask,
      PpcRadarOptions_getAttenuationGammaH(self->options),
      PpcRadarOptions_getAttenuationAlpha(self->options),
	  PolarScanParam_getUndetect(TH)*PolarScanParam_getGain(TH) + PolarScanParam_getOffset(TH),
	  PolarScanParam_getUndetect(DBZH)*PolarScanParam_getGain(DBZH) + PolarScanParam_getOffset(DBZH),
	  &outAttenuationZ, &outAttenuationZDR, &outAttenuationPIA, &outAttenuationDBZH)) {
    goto done;
  }

  /**************************************************************
   * Application of the ZPHI methodology (Testud et al, 2000) for
   * attenuation correction
   **************************************************************/
  if (!PdpProcessor_zphi(self, dataTH, outPDP, attenuationMask, rangeKm,
      PpcRadarOptions_getBB(self->options), PpcRadarOptions_getAttenuationGammaH(self->options), &outZPHI, &outAH)) {
    goto done;
  }

  RaveData2D_useNodata(dataTH, 1);
  RaveData2D_setNodata(dataTH, -999.9);

  RaveData2D_replace(residualClutterMask, RaveData2D_getNodata(residualClutterMask), 0.0);

  tmpresult = RAVE_OBJECT_CLONE(scan);
  if (tmpresult == NULL) {
    goto done;
  }

  if (PpcRadarOptions_TH_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    correctedZ = PdpProcessorInternal_createPolarScanParamFromData2D(dataTH, "TH_CORR", 1, 255.0, 0.0);
    if (correctedZ == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZ)) {
      RAVE_ERROR0("Failed to add corrected TH field");
      goto done;
    }
  }

  if (PpcRadarOptions_ATT_TH_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    attenuatedZ = PdpProcessorInternal_createPolarScanParamFromData2D(outAttenuationZ, "ATT_TH_CORR", 1, 255.0, 0.0);
    if (attenuatedZ == NULL ||
        !PolarScan_addParameter(tmpresult, attenuatedZ)) {
      RAVE_ERROR0("Failed to add corrected and attenuated TH field");
      goto done;
    }
  }

  if (PpcRadarOptions_DBZH_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    correctedDBZH = PdpProcessorInternal_createPolarScanParamFromData2D(dataDBZH, "DBZH_CORR", 1, 255.0, 0.0);
    if (correctedDBZH == NULL ||
        !PolarScan_addParameter(tmpresult, correctedDBZH)) {
      RAVE_ERROR0("Failed to add corrected DBZH field");
      goto done;
    }
  }

  if (PpcRadarOptions_ATT_DBZH_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    attenuatedDBZH = PdpProcessorInternal_createPolarScanParamFromData2D(outAttenuationDBZH, "ATT_DBZH_CORR", 1, 255.0, 0.0);
    if (attenuatedDBZH == NULL ||
        !PolarScan_addParameter(tmpresult, attenuatedDBZH)) {
      RAVE_ERROR0("Failed to add corrected and attenuated DBZH field");
      goto done;
    }
  }


  if (PpcRadarOptions_KDP_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    paramKDP = PdpProcessorInternal_createPolarScanParamFromData2D(outKDP, "KDP_CORR", 1, 255.0, 0.0);
    if (paramKDP == NULL ||
        !PolarScan_addParameter(tmpresult, paramKDP)) {
      RAVE_ERROR0("Failed to add corrected KDP field");
      goto done;
    }
  }

  if (PpcRadarOptions_RHOHV_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    paramRHOHV = PdpProcessorInternal_createPolarScanParamFromData2D(dataRHOHV, "RHOHV_CORR", 1, 255.0, 0.0);
    if (paramRHOHV == NULL ||
        !PolarScan_addParameter(tmpresult, paramRHOHV)) {
      RAVE_ERROR0("Failed to add corrected RHOHV field");
      goto done;
    }
  }

  if (PpcRadarOptions_PHIDP_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    correctedPDP = PdpProcessorInternal_createPolarScanParamFromData2D(outPDP, "PHIDP_CORR", 1, 255.0, 0.0);
    if (correctedPDP == NULL ||
        !PolarScan_addParameter(tmpresult, correctedPDP)) {
      RAVE_ERROR0("Failed to add corrected PDP field");
      goto done;
    }
  }

  if (PpcRadarOptions_ZDR_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    correctedZDR = PdpProcessorInternal_createPolarScanParamFromData2D(dataZDR, "ZDR_CORR", 1, 255.0, 0.0);
    if (correctedZDR == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZDR)) {
      RAVE_ERROR0("Failed to add corrected ZDR field");
      goto done;
    }
  }

  if (PpcRadarOptions_ATT_ZDR_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    attCorrectedZDR = PdpProcessorInternal_createPolarScanParamFromData2D(outAttenuationZDR, "ATT_ZDR_CORR", 1, 255.0, 0.0);
    if (attCorrectedZDR == NULL ||
        !PolarScan_addParameter(tmpresult, attCorrectedZDR)) {
      RAVE_ERROR0("Failed to add corrected ZDR field");
      goto done;
    }
  }

  if (PpcRadarOptions_ZPHI_CORR & PpcRadarOptions_getRequestedFields(self->options)) {
    correctedZPHI = PdpProcessorInternal_createPolarScanParamFromData2D(outZPHI, "ZPHI_CORR", 1, 255.0, 0.0);
    if (correctedZPHI == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZPHI)) {
      RAVE_ERROR0("Failed to add corrected >ZPHI field");
      goto done;
    }
  }

  if (PpcRadarOptions_QUALITY_RESIDUAL_CLUTTER_MASK & PpcRadarOptions_getRequestedFields(self->options)) {
    if (!PdpProcessorInternal_addRaveQualityFieldToScanFromData2D(tmpresult, residualClutterMask, "se.baltrad.ppc.residual_clutter_mask")) {
      goto done;
    }
  }

  if (PpcRadarOptions_QUALITY_ATTENUATION_MASK & PpcRadarOptions_getRequestedFields(self->options)) {
    if (!PdpProcessorInternal_addRaveQualityFieldToScanFromData2D(tmpresult, attenuationMask, "se.baltrad.ppc.attenuation_mask")) {
      goto done;
    }
  }

  fprintf(stderr, "PdpProcessor_process: Total execution time for scan: %lld ms\n", PdpProcessorInternal_timestamp() - starttime);

  result = RAVE_OBJECT_COPY(tmpresult);
done:
  RAVE_OBJECT_RELEASE(dataTH);
  RAVE_OBJECT_RELEASE(thThresholdIndex);
  RAVE_OBJECT_RELEASE(dataZDR);
  RAVE_OBJECT_RELEASE(dataDV);
  RAVE_OBJECT_RELEASE(texturePHIDP);
  RAVE_OBJECT_RELEASE(dataRHOHV);
  RAVE_OBJECT_RELEASE(textureZ);
  RAVE_OBJECT_RELEASE(dataPHIDP);
  RAVE_OBJECT_RELEASE(dataPDP);
  RAVE_OBJECT_RELEASE(dataDBZH);
  RAVE_OBJECT_RELEASE(clutterMap);
  RAVE_OBJECT_RELEASE(residualClutterMask);
  RAVE_OBJECT_RELEASE(outZ);
  RAVE_OBJECT_RELEASE(outQuality);
  RAVE_OBJECT_RELEASE(outClutterMask);
  RAVE_OBJECT_RELEASE(outPDP);
  RAVE_OBJECT_RELEASE(outKDP);
  RAVE_OBJECT_RELEASE(attenuationMask);
  RAVE_OBJECT_RELEASE(outAttenuationZ);
  RAVE_OBJECT_RELEASE(outAttenuationZDR);
  RAVE_OBJECT_RELEASE(outAttenuationPIA);
  RAVE_OBJECT_RELEASE(outAttenuationDBZH);
  RAVE_OBJECT_RELEASE(outZPHI);
  RAVE_OBJECT_RELEASE(outAH);
  RAVE_OBJECT_RELEASE(navigator);
  RAVE_OBJECT_RELEASE(TH);
  RAVE_OBJECT_RELEASE(ZDR);
  RAVE_OBJECT_RELEASE(DV);
  RAVE_OBJECT_RELEASE(PHIDP);
  RAVE_OBJECT_RELEASE(RHOHV);
  RAVE_OBJECT_RELEASE(DBZH);
  RAVE_OBJECT_RELEASE(pdpQualityField);
  RAVE_OBJECT_RELEASE(correctedZ);
  RAVE_OBJECT_RELEASE(correctedZDR);
  RAVE_OBJECT_RELEASE(attCorrectedZDR);
  RAVE_OBJECT_RELEASE(correctedZPHI);
  RAVE_OBJECT_RELEASE(correctedPDP);
  RAVE_OBJECT_RELEASE(correctedDBZH);
  RAVE_OBJECT_RELEASE(attenuatedZ);
  RAVE_OBJECT_RELEASE(attenuatedDBZH);
  RAVE_OBJECT_RELEASE(paramKDP);
  RAVE_OBJECT_RELEASE(paramRHOHV);
  RAVE_OBJECT_RELEASE(tmpresult);

  return result;
}

void PdpProcessor_setMeltingLayerBottomHeight(PdpProcessor_t* self, double height)
{
  self->meltingLayerBottomHeight = height;
}

double PdpProcessor_getMeltingLayerBottomHeight(PdpProcessor_t* self)
{
  /* @TODO: implement proper support for this */
  if (self->meltingLayerBottomHeight <= -1.0) {
    return PpcRadarOptions_getMeltingLayerBottomHeight(self->options);
  }
  return self->meltingLayerBottomHeight;
}

RaveData2D_t* PdpProcessor_texture(PdpProcessor_t* self, RaveData2D_t* X)
{
  RaveData2D_t* result = NULL;
  RaveData2D_t* texture = NULL;
  RaveData2D_t *weight = NULL;
  long xsize = 0, ysize = 0;
  long x, y;
  long i, j;
  double nodata = 0.0;
  RAVE_ASSERT((self != NULL), "pdp processor == NULL");
  if (X == NULL) {
    RAVE_ERROR0("Field to create texture from must be provided");
    return NULL;
  }

  if (!RaveData2D_usingNodata(X)) {
    RAVE_ERROR0("Nodata must be set to create texture");
    return NULL;
  }
  nodata = RaveData2D_getNodata(X);
  xsize = RaveData2D_getXsize(X);
  ysize = RaveData2D_getYsize(X);

  texture = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  weight = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (texture == NULL || weight == NULL) {
    RAVE_ERROR0("Allocation error when creating texture");
    goto done;
  }

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      if (RaveData2D_getValue(X, x, y, &v) && v != nodata) { /* Everything should count, X>-900 | isnan(X)==0 is always true unless NaN */
        RaveData2D_setValue(weight, x, y, 1.0);
      } else {
        RaveData2D_setValue(weight, x, y, 0.0);
      }
    }
  }

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double valueTexture = 0.0;
      double valueSumWeight = 0.0;
      double valueX = 0.0;
      double valueWeight = 0.0;

      RaveData2D_getValueUnchecked(weight, x, y, &valueWeight);
      RaveData2D_getValueUnchecked(X, x, y, &valueX);

      for (j = 1; j >= -1; j--) {
        for (i = 1; i >= -1; i--) {
          long xi = (x + i)%xsize;
          long yj = (y + j)%ysize;
          double valueCircshiftWeight = 0.0;
          double valueCircshiftX = 0.0;

          if (i==0 && j==0) continue;

          while (xi < 0) {
            xi += xsize;
          }
          while (yj < 0) {
            yj += ysize;
          }

          RAVE_ASSERT((xi >= 0 && xi <xsize && yj >= 0 && yj < ysize), "BAD PROGRAMMING");

          RaveData2D_getValueUnchecked(weight, xi, yj, &valueCircshiftWeight);
          RaveData2D_getValueUnchecked(X, xi, yj, &valueCircshiftX);

          valueTexture = valueTexture + valueWeight * valueCircshiftWeight * (valueCircshiftX - valueX)*(valueCircshiftX - valueX);
          valueSumWeight = valueSumWeight + valueWeight * valueCircshiftWeight;
        }
      }
      if (valueSumWeight >= 3.0) {
        if (valueTexture >= 0) {
          RaveData2D_setValueUnchecked(texture, x, y, sqrt(valueTexture) / valueSumWeight);
        } else {
          RaveData2D_setValueUnchecked(texture, x, y, nodata);
        }
      } else {
        RaveData2D_setValueUnchecked(texture, x, y, nodata);
      }
    }
  }

  result = RAVE_OBJECT_COPY(texture);
done:
  RAVE_OBJECT_RELEASE(texture);
  RAVE_OBJECT_RELEASE(weight);
  RaveData2D_useNodata(X, 1);
  return result;
}

RaveData2D_t* PdpProcessor_trap(PdpProcessor_t* self, RaveData2D_t* xarr, double a, double b, double s, double t)
{
  long xi, yi, xsize, ysize;
  int usingNodata = 0;
  double nodataV = 0.0;

  RaveData2D_t* field = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (xarr == NULL) {
    RAVE_ERROR0("Passing xarr as NULL");
    return NULL;
  }
  xsize = RaveData2D_getXsize(xarr);
  ysize = RaveData2D_getYsize(xarr);
  field = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (field == NULL) {
    return NULL;
  }
  usingNodata = RaveData2D_usingNodata(xarr);
  nodataV = RaveData2D_getNodata(xarr);

  for (xi = 0; xi < xsize; xi++) {
    for (yi = 0; yi < ysize; yi++) {
      double x = 0.0, out = 0.0;
      RaveData2D_getValueUnchecked(xarr, xi, yi, &x);
      if (usingNodata && x == nodataV)  {
        continue;
      }
      if ((x <= a - s) || (x > b + t)) {
        out = 0;
      }
      if ((x >= a) && (x <= b)) {
        out = 1;
      }
      if ((x > a - s) && (x < a)) {
        if (s != 0.0) // Just to avoid NaN
          out = (x - a + s) / s;
        else
          out = TRAP_UNDEF_VALUE;
      }
      if ((x >= b) && (x < b + t)) {
        if (t != 0.0)
          out = (b + t - x) / t;
        else
          out = TRAP_UNDEF_VALUE;
      }
      RaveData2D_setValueUnchecked(field, xi, yi, out);
    }
  }

  return field;
}

RaveData2D_t* PdpProcessor_clutterID(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap, double nodataZ, double nodataVRADH)
{
  long xsize = 0, ysize = 0;
  long x, y;

  double uzWeight, uzX2, uzX3, uzDelta1, uzDelta2;
  double velWeight, velX2, velX3, velDelta1, velDelta2;
  double textPhidpWeight, textPhidpX2, textPhidpX3, textPhidpDelta1, textPhidpDelta2;
  double rhvWeight, rhvX2, rhvX3, rhvDelta1, rhvDelta2;
  double textUzWeight, textUzX2, textUzX3, textUzDelta1, textUzDelta2;
  double clutterMapWeight, clutterMapX2, clutterMapX3, clutterMapDelta1, clutterMapDelta2;


  //double sumWeight = self->parUZ[0] + self->parVel[0] + self->parTextPHIDP[0] + self->parRHV[0] + self->parTextUZ[0] + self->parClutterMap[0];
  double sumWeight;

  RaveData2D_t* degree = NULL;
  RaveData2D_t* result = NULL;
  RaveData2D_t *mbf_Z = NULL, *mbf_VRADH = NULL, *mbf_texturePHIDP = NULL;
  RaveData2D_t *mbf_RHOHV = NULL, *mbf_textureZ = NULL, *mbf_clutterMap = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");

  PpcRadarOptions_getParametersUZ(self->options, &uzWeight, &uzX2, &uzX3, &uzDelta1, &uzDelta2);
  PpcRadarOptions_getParametersVEL(self->options, &velWeight, &velX2, &velX3, &velDelta1, &velDelta2);
  PpcRadarOptions_getParametersTEXT_PHIDP(self->options, &textPhidpWeight, &textPhidpX2, &textPhidpX3, &textPhidpDelta1, &textPhidpDelta2);
  PpcRadarOptions_getParametersRHV(self->options, &rhvWeight, &rhvX2, &rhvX3, &rhvDelta1, &rhvDelta2);
  PpcRadarOptions_getParametersTEXT_UZ(self->options, &textUzWeight, &textUzX2, &textUzX3, &textUzDelta1, &textUzDelta2);
  PpcRadarOptions_getParametersCLUTTER_MAP(self->options, &clutterMapWeight, &clutterMapX2, &clutterMapX3, &clutterMapDelta1, &clutterMapDelta2);

  sumWeight = uzWeight + velWeight + textPhidpWeight + rhvWeight + textUzWeight + clutterMapWeight;

  if (sumWeight == 0.0) {
    RAVE_ERROR0("Sum of parameter weights == 0.0");
    return NULL;
  }

  if (Z == NULL || VRADH == NULL || texturePHIDP == NULL || RHOHV == NULL || textureZ == NULL || clutterMap == NULL) {
    RAVE_ERROR0("Z, VRADH, texturePHIDP, RHOHV, textureZ and clutterMap must be NON NULL");
    return NULL;
  }
  xsize = RaveData2D_getXsize(Z);
  ysize = RaveData2D_getYsize(Z);
  degree = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (degree == NULL) {
    return NULL;
  }
  mbf_Z = PdpProcessor_trap(self, Z, uzX2, uzX3, uzDelta1, uzDelta2);
  mbf_VRADH = PdpProcessor_trap(self, VRADH, velX2, velX3, velDelta1, velDelta2);
  mbf_texturePHIDP = PdpProcessor_trap(self, texturePHIDP, textPhidpX2, textPhidpX3, textPhidpDelta1, textPhidpDelta2);
  mbf_RHOHV = PdpProcessor_trap(self, RHOHV, rhvX2, rhvX3, rhvDelta1, rhvDelta2);
  mbf_textureZ = PdpProcessor_trap(self, textureZ, textUzX2, textUzX3, textUzDelta1, textUzDelta2);
  mbf_clutterMap = PdpProcessor_trap(self, clutterMap, clutterMapX2, clutterMapX3, clutterMapDelta1, clutterMapDelta2);
  if (mbf_Z == NULL || mbf_VRADH == NULL || mbf_texturePHIDP == NULL ||
      mbf_RHOHV == NULL || mbf_textureZ == NULL || mbf_clutterMap == NULL) {
    RAVE_ERROR0("Failed to create trap matrices");
    goto done;
  }

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double vDegree = 0.0;
      double vZ=0.0, vVRADH=0.0, vTexturePHIDP=0.0, vRHOHV=0.0, vTextureZ=0.0, vClutterMap=0.0;
      double inZ = 0.0, inVRADH = 0.0;
      RaveData2D_getValueUnchecked(Z, x, y, &inZ);
      RaveData2D_getValueUnchecked(VRADH, x, y, &inVRADH);

      RaveData2D_getValueUnchecked(mbf_Z, x, y, &vZ);
      RaveData2D_getValueUnchecked(mbf_VRADH, x, y, &vVRADH);
      RaveData2D_getValueUnchecked(mbf_texturePHIDP, x, y, &vTexturePHIDP);
      RaveData2D_getValueUnchecked(mbf_RHOHV, x, y, &vRHOHV);
      RaveData2D_getValueUnchecked(mbf_textureZ, x, y, &vTextureZ);
      RaveData2D_getValueUnchecked(mbf_clutterMap, x, y, &vClutterMap);

      if (inVRADH != nodataVRADH && inZ != nodataZ) {
        vDegree = uzWeight * vZ + velWeight * vVRADH + textPhidpWeight * vTexturePHIDP +
            rhvWeight * vRHOHV + textUzWeight * vTextureZ + clutterMapWeight * vClutterMap;
        vDegree /= sumWeight;
      }
      if (inVRADH == nodataVRADH && inZ != nodataZ) {
        vDegree = uzWeight * vZ + velWeight * vVRADH + textPhidpWeight * vTexturePHIDP +
            rhvWeight * vRHOHV + textUzWeight * vTextureZ + clutterMapWeight * vClutterMap;
        vDegree /= sumWeight;
      }
      RaveData2D_setValueUnchecked(degree, x, y, vDegree);
    }
  }

  result = RAVE_OBJECT_COPY(degree);
done:
  RAVE_OBJECT_RELEASE(mbf_Z);
  RAVE_OBJECT_RELEASE(mbf_VRADH);
  RAVE_OBJECT_RELEASE(mbf_texturePHIDP);
  RAVE_OBJECT_RELEASE(mbf_RHOHV);
  RAVE_OBJECT_RELEASE(mbf_textureZ);
  RAVE_OBJECT_RELEASE(mbf_clutterMap);
  RAVE_OBJECT_RELEASE(degree);
  return result;
}

int PdpProcessor_clutterCorrection(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap,
    double nodataZ, double nodataVRADH, double qualityThreshold,
    RaveData2D_t** outZ, RaveData2D_t** outQuality, RaveData2D_t** outClutterMask)
{
  long xsize = 0, ysize = 0;
  long x, y;
  int result = 0;
  RaveData2D_t* degree = NULL;
  RaveData2D_t* tmp = NULL;
  RaveData2D_t *Z2 = NULL, *quality = NULL, *clutterMask = NULL;
  double minDBZ;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (Z == NULL || VRADH==NULL || texturePHIDP == NULL || RHOHV == NULL ||
      textureZ == NULL || clutterMap == NULL || outZ == NULL || outQuality == NULL || outClutterMask == NULL) {
    RAVE_ERROR0("All ravedata2d fields, both in and out must be != NULL");
    goto done;
  }
  xsize = RaveData2D_getXsize(Z);
  ysize = RaveData2D_getYsize(Z);

  degree = PdpProcessor_clutterID(self, Z, VRADH, texturePHIDP, RHOHV, textureZ, clutterMap, nodataZ, nodataVRADH);
  if (degree == NULL) {
    RAVE_ERROR0("Failed to process clutterID");
    goto done;
  }

  Z2 = RAVE_OBJECT_CLONE(Z);
  clutterMask = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  tmp = RaveData2D_ones(xsize, ysize, RaveDataType_DOUBLE);
  if (Z2 == NULL || clutterMask == NULL || tmp == NULL) {
    goto done;
  }
  quality = RaveData2D_sub(tmp, degree);
  if (quality == NULL) {
    goto done;
  }
  minDBZ = PpcRadarOptions_getMinDBZ(self->options);
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double vZ = 0.0, vQ = 0.0;
      RaveData2D_getValueUnchecked(Z, x, y, &vZ);
      RaveData2D_getValueUnchecked(quality, x, y, &vQ);
      if (vZ >= minDBZ && vZ != nodataZ && vQ < qualityThreshold) {
        RaveData2D_setValueUnchecked(Z2, x, y, nodataZ);
        RaveData2D_setValueUnchecked(clutterMask, x, y, 1.0);
      } else {
        //
      }
    }
  }

  *outZ = RAVE_OBJECT_COPY(Z2);
  *outQuality = RAVE_OBJECT_COPY(quality);
  *outClutterMask = RAVE_OBJECT_COPY(clutterMask);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(degree);
  RAVE_OBJECT_RELEASE(tmp);
  RAVE_OBJECT_RELEASE(Z2);
  RAVE_OBJECT_RELEASE(quality);
  RAVE_OBJECT_RELEASE(clutterMask);
  return result;
}

RaveData2D_t* PdpProcessor_medfilt(PdpProcessor_t* self, RaveData2D_t* Z, double thresh, double nodataZ, long filtXsize, long filtYsize)
{
  double minVal = 0.0;
  double v = 0.0;
  long xsize = 0, ysize = 0, x = 0, y = 0;
  RaveData2D_t* result = NULL;
  RaveData2D_t* mask = NULL;
  RaveData2D_t* filtmask = NULL;
  RaveData2D_t *zout = NULL, *ztmp = NULL;
  int usingNodata = 0;
  double minZMedfilterThreshold;

  int threshctr = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (Z == NULL) {
    RAVE_ERROR0("Z == NULL");
    return NULL;
  }

  xsize = RaveData2D_getXsize(Z);
  ysize = RaveData2D_getYsize(Z);
  mask = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  zout = RAVE_OBJECT_CLONE(Z);
  if (mask == NULL || zout == NULL) {
    return NULL;
  }

  usingNodata = RaveData2D_usingNodata(Z);

  RaveData2D_useNodata(Z, 0);
  minVal = RaveData2D_min(Z);
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      RaveData2D_getValueUnchecked(Z, x, y, &v);
      if (v > thresh) {
        RaveData2D_setValueUnchecked(mask, x, y, 1.0);
        threshctr++;
      }
    }
  }
  RaveData2D_useNodata(Z, usingNodata);

  if (threshctr > 0) {
    filtmask = RaveData2D_medfilt2(mask, filtXsize, filtYsize);
    if (filtmask == NULL) {
      goto done;
    }
  }

  RaveData2D_useNodata(zout, 0);
  ztmp = RaveData2D_emul(zout, filtmask);
  if (ztmp == NULL) {
    goto done;
  }
  RaveData2D_useNodata(zout, 1);
  RAVE_OBJECT_RELEASE(zout);
  zout = RAVE_OBJECT_COPY(ztmp);
  RAVE_OBJECT_RELEASE(ztmp);
  minZMedfilterThreshold = PpcRadarOptions_getMinZMedfilterThreshold(self->options);

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      RaveData2D_getValueUnchecked(filtmask, x, y, &v);
      if (v == 0.0)
        RaveData2D_setValueUnchecked(zout, x, y, minVal);
      RaveData2D_getValueUnchecked(Z, x, y, &v);
      if (v >= minVal && v < minZMedfilterThreshold) {
        RaveData2D_setValueUnchecked(zout, x, y, minVal);
      }
    }
  }

  result = RAVE_OBJECT_COPY(zout);
done:
  RAVE_OBJECT_RELEASE(mask);
  RAVE_OBJECT_RELEASE(filtmask);
  RAVE_OBJECT_RELEASE(zout);
  RAVE_OBJECT_RELEASE(ztmp);
  return result;
}

RaveData2D_t* PdpProcessor_residualClutterFilter(PdpProcessor_t* self, RaveData2D_t* Z,
    double thresholdZ, double thresholdTexture, long filtXsize, long filtYsize)
{
  RaveData2D_t* result = NULL;
  RaveData2D_t* img = NULL;
  RaveData2D_t* mask = NULL;
  RaveData2D_t* textureZ = NULL;
  RaveData2D_t* Zout = NULL;
  RaveData2D_t* medZ = NULL;
  RaveData2D_t* textureZout = NULL;
  double nodata = 0.0;
  double residualClutterNodata, residualMinZClutterThreshold, residualClutterTextureFilteringMaxZ, residualClutterMaskNodata;
  // long long starttime = PdpProcessorInternal_timestamp();

  long nhctr = 0;
  double nh = 0.0, EN = 0.0;

  long xsize = 0, ysize = 0, x = 0, y = 0;
  double minZ = 0.0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (Z == NULL) {
    RAVE_ERROR0("Z is NULL");
    return NULL;
  }
  if (!RaveData2D_usingNodata(Z)) {
    RAVE_ERROR0("Z must define nodata usage");
    return NULL;
  }

  nodata = RaveData2D_getNodata(Z);
  xsize = RaveData2D_getXsize(Z);
  ysize = RaveData2D_getYsize(Z);
  minZ = RaveData2D_min(Z);

  residualClutterNodata = PpcRadarOptions_getResidualClutterNodata(self->options);
  residualMinZClutterThreshold = PpcRadarOptions_getResidualMinZClutterThreshold(self->options);
  residualClutterTextureFilteringMaxZ = PpcRadarOptions_getResidualClutterTextureFilteringMaxZ(self->options);
  residualClutterMaskNodata = PpcRadarOptions_getResidualClutterMaskNodata(self->options);

  img = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  RaveData2D_setNodata(img, residualClutterNodata);
  RaveData2D_useNodata(img, 1);

  mask = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (img == NULL || mask == NULL) {
    goto done;
  }
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(Z, x, y, &v);
      if (v < residualMinZClutterThreshold || v == nodata) {
        RaveData2D_setValueUnchecked(img, x, y, residualClutterNodata); /* TODO: Specify as residualClutterNodata? */
      } else {
        RaveData2D_setValueUnchecked(img, x, y, v);
        if (v > thresholdZ) {
          RaveData2D_setValueUnchecked(mask, x, y, 1.0);
        }
      }
    }
  }

  textureZ = PdpProcessor_texture(self, img);

  if (textureZ == NULL) {
    goto done;
  }

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(textureZ, x, y, &v);
      if (v > thresholdZ)
        nhctr++;
    }
  }

  nh = ((double)nhctr) / (double)(xsize*ysize*100.0);
  if (!RaveData2D_entropy(mask, 2, &EN)) {
    RAVE_ERROR0("Failed to calculate entropy");
    goto done;
  }

  if (nh <= 70.0 && EN > 5e-4) {
    Zout = PdpProcessor_medfilt(self, img, thresholdZ, nodata, filtXsize, filtYsize);
    if (Zout == NULL) {
      goto done;
    }

    RaveData2D_setNodata(Zout, residualClutterNodata);
    RaveData2D_useNodata(Zout, 1);
    textureZout = PdpProcessor_texture(self, Zout);

    for (x = 0; x < xsize; x++) {
      for (y = 0; y < ysize; y++) {
        double v = 0.0;
        RaveData2D_getValueUnchecked(textureZout, x, y, &v);
        if (v >= thresholdTexture) {
          RaveData2D_setValueUnchecked(Zout, x, y, minZ);
        }
        RaveData2D_getValueUnchecked(Zout, x, y, &v);
        if (v >= residualClutterTextureFilteringMaxZ) {
          RaveData2D_setValueUnchecked(Zout, x, y, minZ);
        }
      }
    }
    medZ = PdpProcessor_medfilt(self, Zout, thresholdZ, nodata, filtXsize, filtYsize);
    if (medZ == NULL) {
      goto done;
    }
    for (x = 0; x < xsize; x++) {
      for (y = 0; y < ysize; y++) {
        double v = 0.0;
        RaveData2D_getValueUnchecked(medZ, x, y, &v);
        if (v <= thresholdZ) {
          RaveData2D_setValueUnchecked(medZ, x, y, minZ);
          v = minZ;
        }
        if (v <= residualMinZClutterThreshold) {
          RaveData2D_setValueUnchecked(mask, x, y, residualClutterMaskNodata);
        }
      }
    }
  }

  RaveData2D_setNodata(mask, residualClutterMaskNodata);
  RaveData2D_useNodata(mask, 1);

  result = RAVE_OBJECT_COPY(mask);
done:
  RAVE_OBJECT_RELEASE(img);
  RAVE_OBJECT_RELEASE(mask);
  RAVE_OBJECT_RELEASE(textureZ);
  RAVE_OBJECT_RELEASE(Zout);
  RAVE_OBJECT_RELEASE(medZ);
  RAVE_OBJECT_RELEASE(textureZout);
  return result;
}

int PdpProcessor_pdpProcessing(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, long window, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp)
{
  int result = 0;
  long xsize = 0, ysize = 0;
  long x = 0, y = 0;
  long ki = 0;
  RaveData2D_t *pdpres = NULL, *kdpres = NULL;
  RaveData2D_t *stdK = NULL;
  RaveData2D_t *tmp = NULL, *tmp2 = NULL;
  double kdpUp, kdpDown, kdpStdThreshold;

  // long long starttime = PdpProcessorInternal_timestamp();

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (pdp == NULL) {
    RAVE_ERROR0("pdp is NULL");
    goto done;
  }
  if (dr == 0.0) {
    RAVE_ERROR0("dr == 0.0");
    goto done;
  }
  RAVE_ASSERT((pdpf != NULL), "pdpf == NULL");
  RAVE_ASSERT((kdp != NULL), "kdp == NULL");

  RAVE_OBJECT_RELEASE(*pdpf);
  RAVE_OBJECT_RELEASE(*kdp);

  xsize = RaveData2D_getXsize(pdp); /* Bin */
  ysize = RaveData2D_getYsize(pdp); /* Ray */
  pdpres = RAVE_OBJECT_CLONE(pdp);
  kdpres = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  RaveData2D_setNodata(kdpres, -999.0);
  RaveData2D_useNodata(kdpres, 1);

  if (pdpres == NULL || kdpres == NULL) {
    goto done;
  }
  kdpUp = PpcRadarOptions_getKdpUp(self->options);
  kdpDown = PpcRadarOptions_getKdpDown(self->options);
  kdpStdThreshold = PpcRadarOptions_getKdpStdThreshold(self->options);

  // starttime = PdpProcessorInternal_timestamp();
  // fprintf(stderr, "pdpProcessing: Initialization %ld\n", PdpProcessorInternal_timestamp() - starttime);
  //Kdp = (Bx - Ax) / 2*(2*dr*window) == 0.5*(Bx-Ax)/(2*dr*window);
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double Ax, Bx, Kdpv;
      long bxi = (x + window)%xsize;
      long axi = (x - window)%xsize;
      while (bxi < 0) {
        bxi += xsize;
      }
      while (axi < 0) {
        axi += xsize;
      }
      RaveData2D_getValueUnchecked(pdpres, axi, y, &Ax);
      RaveData2D_getValueUnchecked(pdpres, bxi, y, &Bx);
      Kdpv = 0.5 * (Bx - Ax) / (2 * dr * window);
      if (Kdpv < kdpDown || Kdpv > kdpUp) {
        Kdpv = 0.0;
      }
      if (x < window || x >= xsize - window ) { /* Side effects compensation */
        Kdpv = 0.0;
      }
      RaveData2D_setValueUnchecked(kdpres, x, y, Kdpv);
    }
  }

  stdK = RaveData2D_movingstd(kdpres, window, 0); /* In matlab they use 0, window as inparam, but they are used as window, 0 in array...... */

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(stdK,  x, y, &v);
      if (v > kdpStdThreshold) {
        RaveData2D_setValueUnchecked(kdpres, x, y, 0.0);
      }
    }
  }

  for (ki = 0; ki < nrIter; ki++) {
    RAVE_OBJECT_RELEASE(tmp);
    tmp = RAVE_OBJECT_CLONE(kdpres);

    if (tmp == NULL) {
      goto done;
    }

    for (x = 0; x < xsize; x++) {
      for (y = 0; y < ysize; y++) {
        double v = 0.0;
        RaveData2D_getValueUnchecked(tmp, x, y, &v);
        if (v < kdpDown) {
          RaveData2D_setValueUnchecked(tmp, x, y, 0.0);
        }
      }
    }
    tmp2 = RaveData2D_mulNumber(tmp, 2.0 * dr);
    if (tmp2 == NULL) {
      goto done;
    }

    RAVE_OBJECT_RELEASE(pdpres);
    pdpres = RaveData2D_cumsum(tmp2, 0);  // Matlab - cumsum(2*tmp*dr,2); Basically says that it's in y-direction since x is vertically and y horizontally
    if (pdpres == NULL) {
      goto done;
    }

    for (x = 0; x < xsize; x++) {
      for (y = 0; y < ysize; y++) {
        double Ax, Bx, Kdpv;

        long bxi = (x + window)%xsize;
        long axi = (x - window)%xsize;
        while (bxi < 0) {
          bxi += xsize;
        }
        while (axi < 0) {
          axi += xsize;
        }

        RaveData2D_getValueUnchecked(pdpres, axi, y, &Ax);
        RaveData2D_getValueUnchecked(pdpres, bxi, y, &Bx);
        Kdpv = 0.5 * (Bx - Ax) / (2 * dr * window);
        if (Kdpv < kdpDown || Kdpv > kdpUp) {
          Kdpv = 0.0;
        }
        if (x < window || x >= xsize - window ) { /* Side effects compensation */
          Kdpv = 0.0;
        }
        RaveData2D_setValueUnchecked(kdpres, x, y, Kdpv);
      }
    }

    RAVE_OBJECT_RELEASE(tmp);
    RAVE_OBJECT_RELEASE(tmp2);
  }

  tmp = RaveData2D_mulNumber(kdpres, 2.0 * dr);
  if (tmp == NULL) {
    goto done;
  }

  RAVE_OBJECT_RELEASE(pdpres);
  pdpres = RaveData2D_cumsum(tmp, 0);
  if (pdpres == NULL) {
    goto done;
  }

  *pdpf = RAVE_OBJECT_COPY(pdpres);
  *kdp = RAVE_OBJECT_COPY(kdpres);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(pdpres);
  RAVE_OBJECT_RELEASE(kdpres);
  RAVE_OBJECT_RELEASE(stdK);
  RAVE_OBJECT_RELEASE(tmp);
  RAVE_OBJECT_RELEASE(tmp2);

  return result;
}

int PdpProcessor_pdpScript(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, double rWin1, double rWin2, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp)
{
  int result = 0;
  long x, y, xsize = 0, ysize = 0;
  long window = 0;
  int isempty = 1;
  RaveData2D_t* texture = NULL;
  RaveData2D_t* pdpwork = NULL;
  RaveData2D_t *pdpres = NULL, *kdpres = NULL;
  double processingTextureThreshold, nodata, thresholdPhidp;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (pdp == NULL) {
    RAVE_ERROR0("pdp == NULL");
    goto done;
  }
  pdpwork = RAVE_OBJECT_CLONE(pdp);
  if (pdpwork == NULL) {
    goto done;
  }
  if (dr == 0.0) {
    RAVE_ERROR0("dr must be > 0");
    goto done;
  }
  nodata = PpcRadarOptions_getNodata(self->options);
  processingTextureThreshold = PpcRadarOptions_getProcessingTextureThreshold(self->options);
  thresholdPhidp = PpcRadarOptions_getThresholdPhidp(self->options);

  window = round(rWin1/ dr);
  if (window < PpcRadarOptions_getMinWindow(self->options)) {
    window = PpcRadarOptions_getMinWindow(self->options);
  }

  xsize = RaveData2D_getXsize(pdp);
  ysize = RaveData2D_getYsize(pdp);

  texture = PdpProcessor_texture(self,  pdpwork);
  if (texture == NULL) {
    goto done;
  }

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(texture, x, y, &v);
      if (v > processingTextureThreshold) {
        RaveData2D_setValueUnchecked(pdpwork, x, y, nodata);
      }
    }
  }


  if (!PdpProcessor_pdpProcessing(self, pdpwork, dr, (long)window, nrIter, &pdpres, &kdpres)) {
    goto done;
  }

  for (x = 0; isempty == 1 && x < xsize; x++) {
    for (y = 0; isempty == 1 && y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(pdpres, x, y, &v);
      if (v > thresholdPhidp) {
        isempty = 0;
      }
    }
  }

  if (!isempty && rWin2 < rWin1) {
    window = round(rWin2/dr);
    if (window < PpcRadarOptions_getMinWindow(self->options)) {
      window = PpcRadarOptions_getMinWindow(self->options);
    }
    RAVE_OBJECT_RELEASE(pdpres);
    RAVE_OBJECT_RELEASE(kdpres);
    if (!PdpProcessor_pdpProcessing(self, pdpwork, dr, window, nrIter, &pdpres, &kdpres)) {
      goto done;
    }
  }

  *pdpf = RAVE_OBJECT_COPY(pdpres);
  *kdp = RAVE_OBJECT_COPY(kdpres);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(texture);
  RAVE_OBJECT_RELEASE(pdpwork);
  RAVE_OBJECT_RELEASE(pdpres);
  RAVE_OBJECT_RELEASE(kdpres);

  return result;
}

int PdpProcessor_attenuation(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* zdr, RaveData2D_t* dbzh, RaveData2D_t* pdp,
    RaveData2D_t* mask, double gamma_h, double alpha, double zundetect, double dbzhundetect, RaveData2D_t** outz, RaveData2D_t** outzdr, RaveData2D_t** outPIA, RaveData2D_t** outDBZH)
{
  long nrays = 0;
  long nbins = 0;
  int result = 0;
  long ri = 0, bi = 0;
  double vpianodata = 0.0;
  double znodata = 0.0;
  double dbzhnodata = 0.0;
  RaveData2D_t *PIA = NULL, *PIDA = NULL;
  RaveData2D_t *zdrres = NULL, *zres = NULL, *dbzhres = NULL;
  double attenuationPIAminZ;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (Z == NULL || zdr == NULL || pdp == NULL || mask == NULL) {
    RAVE_ERROR0("Z, zdr, pdp or mask is NULL");
    goto done;
  }
  if (outz == NULL || outzdr == NULL || outPIA == NULL) {
    RAVE_ERROR0("Out Z / zdr or PIA is NULL");
    goto done;
  }
  if (!RaveData2D_usingNodata(pdp) || !RaveData2D_usingNodata(dbzh)) {
    RAVE_ERROR0("pdp or dbzh is not using nodata");
    goto done;
  }
  nrays = RaveData2D_getYsize(Z);
  nbins = RaveData2D_getXsize(Z);

  if (nrays != RaveData2D_getYsize(zdr) || nrays != RaveData2D_getYsize(pdp) || nrays != RaveData2D_getYsize(mask)) {
    RAVE_ERROR0("zdr, pdp or mask hasn't got same nrays as Z");
    goto done;
  }

  if (nbins != RaveData2D_getXsize(zdr) || nbins != RaveData2D_getXsize(pdp) || nbins != RaveData2D_getXsize(mask)) {
    RAVE_ERROR0("zdr, pdp or mask hasn't got same nbins as Z");
    goto done;
  }

  PIA = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
  if (PIA == NULL) {
    goto done;
  }
  vpianodata = RaveData2D_getNodata(pdp);
  znodata = RaveData2D_getNodata(Z);
  RaveData2D_setNodata(PIA, vpianodata);
  RaveData2D_useNodata(PIA, 1);

  dbzhnodata = RaveData2D_getNodata(dbzh);

  for (ri = 0; ri < nrays; ri++) {
    long startbi = -1, endbi = -1;
    for (bi = 0; bi < nbins; bi++) {
      double v = 0;
      RaveData2D_getValueUnchecked(mask, bi, ri, &v);
      if (v > 0 && startbi == -1) {
        startbi = bi;
      } else if (v > 0) {
        endbi = bi;
      }
    }
    if (startbi != -1 && endbi != -1 &&  endbi < nbins-1) { /* don't want end bin to activate attenuation for some reason */
      double pdpFirst = 0.0;
      double lastValue = 0.0;
      RaveData2D_getValueUnchecked(pdp, startbi, ri, &pdpFirst);
      for (bi = startbi; bi <= endbi; bi++) {
        double v = 0.0;
        RaveData2D_getValueUnchecked(pdp, bi, ri, &v);
        lastValue = gamma_h * (v - pdpFirst);
        RaveData2D_setValueUnchecked(PIA, bi, ri, lastValue);
      }
      for (bi = endbi + 1; bi < nbins; bi++) {
        RaveData2D_setValueUnchecked(PIA, bi, ri, lastValue);
      }
    }
  }

  PIDA = RaveData2D_mulNumber(PIA, alpha);
  if (PIDA == NULL) {
    goto done;
  }
  zdrres = RAVE_OBJECT_CLONE(zdr);
  zres = RAVE_OBJECT_CLONE(Z);
  dbzhres = RAVE_OBJECT_CLONE(dbzh);

  if (zdrres == NULL || zres == NULL || dbzhres == NULL) {
    RAVE_ERROR0("Failed to clone resulting fields");
    goto done;
  }

  attenuationPIAminZ = PpcRadarOptions_getAttenuationPIAminZ(self->options);

  for (ri = 0; ri < nrays; ri++) {
    for (bi = 0; bi < nbins; bi++) {
      double vz = 0, vpia = 0, vzdr = 0, vpida = 0, vdbzh = 0;
      RaveData2D_getValueUnchecked(Z, bi, ri, &vz);
      RaveData2D_getValueUnchecked(dbzh, bi, ri, &vdbzh);
      RaveData2D_getValueUnchecked(PIA, bi, ri, &vpia);
      RaveData2D_getValueUnchecked(zdr, bi, ri, &vzdr);
      RaveData2D_getValueUnchecked(PIDA, bi, ri, &vpida);
      if (vpia != vpianodata && vpia >= 0.0 && znodata != vz && vz != zundetect) {
        RaveData2D_setValueUnchecked(zres, bi, ri, vz + vpia);
        RaveData2D_setValueUnchecked(zdrres, bi, ri, vzdr + vpida);
      }

      if (vpia != vpianodata && vpia >= 0.0 && dbzhnodata != vdbzh && vdbzh != dbzhundetect) { /* Adding attenuation to DBZH */
        RaveData2D_setValueUnchecked(dbzhres, bi, ri, vdbzh + vpia);
      }

      RaveData2D_getValueUnchecked(zres, bi, ri, &vz);
      if (vz < attenuationPIAminZ) {
        RaveData2D_setValueUnchecked(PIA, bi, ri, vpianodata);
      }
    }
  }

  *outz = RAVE_OBJECT_COPY(zres);
  *outzdr = RAVE_OBJECT_COPY(zdrres);
  *outPIA = RAVE_OBJECT_COPY(PIA);
  *outDBZH = RAVE_OBJECT_COPY(dbzhres);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(PIA);
  RAVE_OBJECT_RELEASE(PIDA);
  RAVE_OBJECT_RELEASE(zdrres);
  RAVE_OBJECT_RELEASE(zres);
  RAVE_OBJECT_RELEASE(dbzhres);
  return result;
}

/* BB=0.7987; % at C-band */
int PdpProcessor_zphi(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* pdp, RaveData2D_t* mask,
    double dr, double BB, double gamma_h, RaveData2D_t** outzphi, RaveData2D_t** outAH)
{
  long nrays = 0;
  long nbins = 0;
  int result = 0;
  long ri = 0, bi = 0;
  double vpdpnodata = 0.0;
  double Znodata = 0.0;
  RaveData2D_t *ah = NULL, *zphi = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (Z == NULL || pdp == NULL || mask == NULL) {
    RAVE_ERROR0("Z, pdp or mask is NULL");
    goto done;
  }
  if (outzphi == NULL || outAH == NULL) {
    RAVE_ERROR0("Out zphi AH is NULL");
    goto done;
  }
  if (!RaveData2D_usingNodata(pdp)) {
    RAVE_ERROR0("pdp is not using nodata");
    goto done;
  }
  if (!RaveData2D_usingNodata(Z)) {
    RAVE_ERROR0("Z is not using nodata");
    goto done;
  }
  vpdpnodata = RaveData2D_getNodata(pdp);
  Znodata = RaveData2D_getNodata(Z);
  nrays = RaveData2D_getYsize(Z);
  nbins = RaveData2D_getXsize(Z);

  ah = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
  zphi = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);
  if (ah == NULL || zphi == NULL) {
    goto done;
  }
  RaveData2D_useNodata(ah, 1);
  RaveData2D_useNodata(zphi, 1);
  RaveData2D_setNodata(ah, RaveData2D_getNodata(Z));
  RaveData2D_setNodata(zphi, RaveData2D_getNodata(Z));

  for (ri = 0; ri < nrays; ri++) {
    long startbi = -1, endbi = -1;
    for (bi = 0; bi < nbins; bi++) {
      double v = 0;
      RaveData2D_getValueUnchecked(mask, bi, ri, &v);
      if (v > 0 && startbi == -1) {
        startbi = bi;
        endbi = bi;
      } else if (v > 0) {
        endbi = bi;
      }
    }
    if (startbi != -1) {
      double DPDP = 0.0;
      double vpdp = 0.0;
      double factor = 0.0;
      double Ir1rn = 0.0;
      double cumsum = 0.0;
      double cumsum_zphi = 0.0;
      RaveData2D_getValueUnchecked(pdp, endbi, ri, &vpdp);
      if (vpdp > 0 && vpdp != vpdpnodata) {
        double vpdp1 = 0.0;
        RaveData2D_getValueUnchecked(pdp, startbi, ri, &vpdp1);
        DPDP = vpdp - vpdp1;
      }
      if (vpdp < 0 || vpdp == vpdpnodata) {
        DPDP = 0.0;
      }
      factor = pow(10, 0.1 * BB * gamma_h * DPDP) - 1;
      for (bi = startbi; bi <= endbi; bi++) {
        double zv = 0.0;
        RaveData2D_getValueUnchecked(Z, bi, ri, &zv);
        /* Linearization.. */
        if (zv != Znodata) {
          double linval = pow(10.0, 0.1*zv); /* 10.^(0.1*xx); */
          Ir1rn += pow(linval, BB);
        }
      }

      for (bi = startbi; bi <= endbi; bi++) {
        double zv = 0.0;
        RaveData2D_getValueUnchecked(Z, bi, ri, &zv);
        /* Linearization.. */
        if (zv != Znodata) {
          double linval = pow(10.0, 0.1*zv); // 10.^(0.1*xx);
          double nv = 0.0;
          /* Original matlab code
           * factor=10^(0.1*BB*gamma*DPDP)-1;
           * Ir1rn=0.46*BB*sum(Z(r1:rn,kkk).^BB*res,1,'omitnan');
           * Irrn=Ir1rn-0.46*BB*cumsum(Z(r1:rn,kkk).^BB*res,1,'omitnan');
           * AH(r1:rn,kkk)=factor*(Z(r1:rn,kkk).^BB)./(Ir1rn+factor*Irrn);
           *
           * Below is an atempted simplifcation of above calculations
           */
          double simplified_denominator = 0.0;
          cumsum += pow(linval,BB);
          simplified_denominator = 0.46*BB*dr*(Ir1rn + factor*Ir1rn - factor * cumsum);
          if (simplified_denominator != 0.0) {
            nv = factor * (pow(linval, BB) / simplified_denominator);
            RaveData2D_setValueUnchecked(ah, bi, ri, nv);
            cumsum_zphi += 2*dr*nv;
            RaveData2D_setValueUnchecked(zphi, bi, ri, zv+cumsum_zphi);
          }
        }
      }
      /* To get same behaviour as matlab code, we pad values until end of ray with last cumsum */
      for (bi = endbi; bi < nbins; bi++) {
        double zv = 0.0;
        RaveData2D_getValueUnchecked(Z, bi, ri, &zv);
        RaveData2D_setValueUnchecked(zphi, bi, ri, zv+cumsum_zphi);
      }
    }
  }

  *outzphi = RAVE_OBJECT_COPY(zphi);
  *outAH = RAVE_OBJECT_COPY(ah);
  result = 1;
done:
  RAVE_OBJECT_RELEASE(ah);
  RAVE_OBJECT_RELEASE(zphi);
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType PdpProcessor_TYPE = {
    "PdpProcessor",
    sizeof(PdpProcessor_t),
    PdpProcessor_constructor,
    PdpProcessor_destructor,
    PdpProcessor_copyconstructor
};
