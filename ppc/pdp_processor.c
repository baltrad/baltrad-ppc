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

/**
 * Represents one transformator
 */
struct _PdpProcessor_t {
  RAVE_OBJECT_HEAD /** Always on top */

  double parUZ[5]; /**< parameters for TH */
  double parVel[5]; /**< parameters for VRADH */
  double parTextPHIDP[5]; /**< parameters for the PHIDP texture */
  double parRHV[5]; /**< parameters for RHOHV */
  double parTextUZ[5]; /**< parameters for the TH texture */
  double parClutterMap[5]; /**< parameters for the clutter map */
  double nodata; /**< nodata to used in most products */
  double minDBZ; /**< min DBZ threshold in the clutter correction */
  double qualityThreshold; /**< quality threshold in the clutter correction */
  double residualMinZClutterThreshold; /**< min z clutter threshold during residual clutter filtering */
  double residualThresholdZ; /**< min Z threshold in the residual clutter filtering */
  double residualThresholdTexture; /**< texture threshold in the residual clutter filtering */
  double residualClutterNodata; /**< the nodata value to be used when creating the residual clutter image used for creating the mask */
  double residualClutterMaskNodata; /** Nodata value for the residual clutter mask */
  double residualClutterTextureFilteringMaxZ; /**< Max Z value when creating the residual clutter mask, anything higher will be set to min value */
  long residualFilterBinSize; /**< number of bins used in the window when creating the residual mask */
  long residualFilterRaySize; /**< number of rays used in the window when creating the residual mask */

  double minZMedfilterThreshold; /**< min z threshold used in the median filter that is used by the residual clutter filter */
  double processingTextureThreshold; /**< threshold for the texture created in the pdp processing*/

  long minWindow; /**< min window size */
  double pdpRWin1; /**< pdp ray window 1 */
  double pdpRWin2; /**< pdp ray window 1 */
  long pdpNrIterations; /**< number of iterations in pdp processing */

  double kdpUp; /**< Maximum allowed value of Kdp */
  double kdpDown; /** Minimum allowed value of kdp */
  double kdpStdThreshold; /**< Kdp STD threshold */
  double BB; /**< BB value used in the zphi part of the pdp processing */
  double thresholdPhidp; /**< threshold for PHIDP in the pdp processing */
  double minAttenuationMaskRHOHV; /**< min RHOHV value for marking value as 1 in the attenuation mask */
  double minAttenuationMaskKDP; /**< min KDP value for marking value as 1 in the attenuation mask */
  double minAttenuationMaskTH;/**< min TH value for marking value as 1 in the attenuation mask */
  double attenuationGammaH; /**< gamma h value used in the attenuation */
  double attenuationAlpha;  /**< alpha value used in the attenuation */
  double attenuationPIAminZ; /**< min PIA Z value in attenuation process */

  int requestedFieldMask; /**< the fields that should be added to the result */
};

//                                          Weight | X2   |  X3  | Delta1  | Delta2
// X1=X2-Delta1, X3=X4-Delta2
static double DEFAULT_PAR_UZ[5] =          {0.00,  30.00,  90.00,  62.00,  20.00 };
static double DEFAULT_PAR_VEL[5] =         {0.30,  -0.90,   0.90,   0.15,   0.15 };
static double DEFAULT_PAR_TEXT_PHIDP[5] =  {0.80,  15.00,  40.00,   5.00,  40.00 };
static double DEFAULT_PAR_RHV[5] =         {0.20,   0.00,   0.60,   0.00,   0.10 };
static double DEFAULT_PAR_TEXT_UZ[5] =     {0.30,  20.00,  60.00,   5.00,  10.00 };
static double DEFAULT_PAR_CLUTTER_MAP[5] = {0.90,   5.00,  70.00,  20.00,  60.00 };

/*@{ Private functions */
/**
 * Constructor
 */
static int PdpProcessor_constructor(RaveCoreObject* obj)
{
	PdpProcessor_t* pdp = (PdpProcessor_t*)obj;
  memcpy(pdp->parUZ,  DEFAULT_PAR_UZ, sizeof(pdp->parUZ));
  memcpy(pdp->parVel, DEFAULT_PAR_VEL, sizeof(pdp->parVel));
  memcpy(pdp->parTextPHIDP, DEFAULT_PAR_TEXT_PHIDP, sizeof(pdp->parTextPHIDP));
  memcpy(pdp->parRHV, DEFAULT_PAR_RHV, sizeof(pdp->parRHV));
  memcpy(pdp->parTextUZ, DEFAULT_PAR_TEXT_UZ, sizeof(pdp->parTextUZ));
  memcpy(pdp->parClutterMap, DEFAULT_PAR_CLUTTER_MAP, sizeof(pdp->parClutterMap));
  pdp->minWindow = 11;
  pdp->nodata = -999.0;
  pdp->minDBZ = -32.0;
  pdp->qualityThreshold = 0.75;
  pdp->residualMinZClutterThreshold = -31.5;
  pdp->residualClutterNodata = -999.0;
  pdp->residualClutterMaskNodata = -1.0;
  pdp->residualThresholdZ = -20.0;
  pdp->residualThresholdTexture = 20.0;
  pdp->residualFilterBinSize = 1;
  pdp->residualFilterRaySize = 1;

  pdp->minZMedfilterThreshold = -30.0;
  pdp->processingTextureThreshold = 10.0;
  pdp->residualClutterTextureFilteringMaxZ = 70.0;
  pdp->pdpRWin1 = 3.5;
  pdp->pdpRWin2 = 1.5;
  pdp->pdpNrIterations = 2;

  pdp->kdpUp = 20.0; /**< c band */
  pdp->kdpDown = -2.0; /**< c band */
  pdp->kdpStdThreshold = 5.0; /**< c band */
  pdp->BB = 0.7987; /* C-band */
  pdp->thresholdPhidp = 40.0;

  pdp->minAttenuationMaskRHOHV = 0.8;
  pdp->minAttenuationMaskKDP = 0.001;
  pdp->minAttenuationMaskTH = -20.0;
  pdp->attenuationGammaH = 0.08;
  pdp->attenuationAlpha = 0.2;
  pdp->attenuationPIAminZ = -30;

  pdp->requestedFieldMask = PdpProcessor_CORR_ATT_TH | PdpProcessor_CORR_PHIDP | PdpProcessor_QUALITY_RESIDUAL_CLUTTER_MASK;

  return 1;
}

/**
 * Destructor
 */
static void PdpProcessor_destructor(RaveCoreObject* obj)
{
}

/**
 * Copy constructor
 */
static int PdpProcessor_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PdpProcessor_t* this = (PdpProcessor_t*)obj;
  PdpProcessor_t* src = (PdpProcessor_t*)srcobj;

  memcpy(this->parUZ,  src->parUZ, sizeof(src->parUZ));
  memcpy(this->parVel, src->parVel, sizeof(src->parVel));
  memcpy(this->parTextPHIDP, src->parTextPHIDP, sizeof(src->parTextPHIDP));
  memcpy(this->parRHV, src->parRHV, sizeof(src->parRHV));
  memcpy(this->parTextUZ, src->parTextUZ, sizeof(src->parTextUZ));
  memcpy(this->parClutterMap, src->parClutterMap, sizeof(src->parClutterMap));
  this->minWindow = src->minWindow;
  this->nodata = src->nodata;
  this->minDBZ = src->minDBZ;
  this->qualityThreshold = src->qualityThreshold;
  this->residualMinZClutterThreshold = src->residualMinZClutterThreshold;
  this->residualClutterNodata = src->residualClutterNodata;
  this->residualClutterMaskNodata = src->residualClutterMaskNodata;
  this->residualThresholdZ = src->residualThresholdZ;
  this->residualThresholdTexture = src->residualThresholdTexture;
  this->residualFilterBinSize = src->residualFilterBinSize;
  this->residualFilterRaySize = src->residualFilterRaySize;

  this->minZMedfilterThreshold = src->minZMedfilterThreshold;
  this->processingTextureThreshold = src->processingTextureThreshold;
  this->residualClutterTextureFilteringMaxZ = src->residualClutterTextureFilteringMaxZ;
  this->pdpRWin1 = src->pdpRWin1;
  this->pdpRWin2 = src->pdpRWin2;
  this->pdpNrIterations = src->pdpNrIterations;

  this->kdpUp = src->kdpUp;
  this->kdpDown = src->kdpDown;
  this->kdpStdThreshold = src->kdpStdThreshold;
  this->BB = src->BB;
  this->thresholdPhidp = src->thresholdPhidp;

  this->minAttenuationMaskRHOHV = src->minAttenuationMaskRHOHV;
  this->minAttenuationMaskKDP = src->minAttenuationMaskKDP;
  this->minAttenuationMaskTH = src->minAttenuationMaskTH;
  this->attenuationGammaH = src->attenuationGammaH;
  this->attenuationAlpha = src->attenuationAlpha;
  this->attenuationPIAminZ = src->attenuationPIAminZ;

  this->requestedFieldMask = src->requestedFieldMask;

  return 1;
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
  //fprintf(stderr, "minv = %f, maxv = %f, spread = %f\n", minv, maxv, spread);
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
static long long PdpProcessorInternal_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

/*@} End of Private functions */

/*@{ Interface functions */
void PdpProcessor_setRequestedFields(PdpProcessor_t* self, int fieldmask)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->requestedFieldMask = fieldmask;
}

int PdpProcessor_getRequestedFields(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->requestedFieldMask;
}

PolarScan_t* PdpProcessor_process(PdpProcessor_t* self, PolarScan_t* scan)
{
  PolarScan_t *result = NULL, *tmpresult = NULL;
  double elangle = 0.0;
  double range = 0.0, rangeKm = 0.0;
  long nbins = 0, nrays = 0;
  long bi = 0, ri = 0;
  double nodataPHIDP = 0.0, nodataTH = 0.0, nodataZDR = 0.0, /*nodataDV = 0.0, */nodataRHOHV = 0.0;
  double flag = -999.9;
  double undetectTH = 0.0;
  RaveData2D_t *dataTH = NULL, *dataZDR = NULL, *dataDV = NULL, *texturePHIDP = NULL;
  RaveData2D_t *dataRHOHV = NULL, *textureZ = NULL, *dataPHIDP = NULL, *dataPDP = NULL;
  RaveData2D_t *clutterMap = NULL, *residualClutterMask = NULL;
  RaveData2D_t *outZ = NULL, *outQuality = NULL, *outClutterMask = NULL;
  RaveData2D_t *outPDP = NULL, *outKDP = NULL, *attenuationMask = NULL;
  RaveData2D_t *outAttenuationZ = NULL, *outAttenuationZDR = NULL, *outAttenuationPIA = NULL;
  RaveData2D_t *outZPHI = NULL, *outAH = NULL;
  RaveField_t* pdpQualityField = NULL;
  PolarScanParam_t *correctedZ = NULL, *correctedZDR = NULL, *correctedZPHI = NULL, *attenuatedZ = NULL, *paramKDP = NULL, *paramRHOHV = NULL, *correctedPDP = NULL;
  PolarNavigator_t* navigator = NULL;
  PolarScanParam_t *TH = NULL, *ZDR = NULL, *DV = NULL, *PHIDP = NULL, *RHOHV = NULL;

  long starttime = PdpProcessorInternal_timestamp();

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (scan == NULL) {
    RAVE_ERROR0("No scan provided");
    goto done;
  }

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

  dataTH = PdpProcessorInternal_getData2DFromParam(TH, self->nodata);
  dataZDR = PdpProcessorInternal_getData2DFromParam(ZDR, self->nodata);
  dataDV = PdpProcessorInternal_getData2DFromParam(DV, self->nodata);
  dataPHIDP = PdpProcessorInternal_getData2DFromParam(PHIDP, self->nodata);
  dataRHOHV = PdpProcessorInternal_getData2DFromParam(RHOHV, self->nodata);
  clutterMap = RaveData2D_zeros(nbins, nrays, RaveDataType_DOUBLE);

  // fprintf(stderr, "Starting actual calculations after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);
  // IMPORTANT NOTE: The following command has to be applied until the
  //                 moment comuptation is corrected at RSP level
  dataPDP = RaveData2D_mulNumber(dataPHIDP, -1.0);

  nodataPHIDP = self->nodata;
  nodataTH = self->nodata;
  // nodataDV = self->nodata;
  nodataZDR = self->nodata;
  nodataRHOHV = self->nodata;
  undetectTH = PolarScanParam_getUndetect(TH)*PolarScanParam_getGain(TH) + PolarScanParam_getOffset(TH);

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v;
      RaveData2D_getValueUnchecked(dataTH, bi, ri, &v);
      if (v < -20) {
        RaveData2D_setValueUnchecked(dataTH, bi, ri, nodataTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, nodataZDR);
        RaveData2D_setValueUnchecked(dataPDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataPHIDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, nodataRHOHV);
      }
    }
  }

  // fprintf(stderr, "Starting texture creations after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  texturePHIDP = PdpProcessor_texture(self, dataPDP);

  // fprintf(stderr, "First texture created after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  textureZ = PdpProcessor_texture(self, dataTH);

  // fprintf(stderr, "Second texture created after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  /**************************************************************
   * Clutter removal by using a Fuzzy Logic Approach
   **************************************************************/
  if (!PdpProcessor_clutterCorrection(self, dataTH, dataDV, texturePHIDP, dataRHOHV, textureZ, dataPDP,
        PolarScanParam_getNodata(TH), PolarScanParam_getNodata(DV), self->qualityThreshold,
        &outZ, &outQuality, &outClutterMask)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(outZ); /* Not used in matlab */

  // fprintf(stderr, "Clutter correction performed after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);


  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(outQuality, bi, ri, &v);

      if (v < 0.75 /*self->qualityThreshold*/) {
        RaveData2D_setValueUnchecked(dataTH, bi, ri, undetectTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, nodataZDR);
        RaveData2D_setValueUnchecked(dataPHIDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataPDP, bi, ri, nodataPHIDP);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, nodataRHOHV);
      }
    }
  }

  /**************************************************************
   * MEDIAN FILTERING TO REMOVE RESIDUAL ISOLATED PIXELS AFFECTED BY CLUTTER
   **************************************************************/
  residualClutterMask = PdpProcessor_residualClutterFilter(self, dataTH, self->residualThresholdZ, self->residualThresholdTexture, self->residualFilterBinSize, self->residualFilterRaySize);
  if (residualClutterMask == NULL) {
    goto done;
  }
  // fprintf(stderr, "Residual clutter filter performed after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  /**************************************************************
   * PHIDP Filtering and Kdp retrieval
   **************************************************************/

  if (!PdpProcessor_pdpScript(self, dataPDP, rangeKm, self->pdpRWin1, self->pdpRWin2, self->pdpNrIterations, &outPDP, &outKDP)) {
    goto done;
  }

  for (bi = 0; bi < nbins; bi++) {
    for (ri = 0; ri < nrays; ri++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(residualClutterMask, bi, ri, &v);
      if (v == 0.0 || v == self->residualClutterMaskNodata) {
        RaveData2D_setValueUnchecked(dataTH, bi, ri, undetectTH);
        RaveData2D_setValueUnchecked(dataZDR, bi, ri, flag);
        RaveData2D_setValueUnchecked(dataRHOHV, bi, ri, flag);
        RaveData2D_setValueUnchecked(dataDV, bi, ri, flag);
      }

      RaveData2D_getValueUnchecked(outQuality, bi, ri, &v);
      RaveData2D_getValueUnchecked(dataTH, bi, ri, &v);
      if (v < self->qualityThreshold || v < -900) {
        RaveData2D_setValueUnchecked(outPDP, bi, ri, nodataPHIDP);
      }
    }
  }

  /**************************************************************
   * Attenuation correction using a linear approach (Bringi et al., 1990)
   **************************************************************/
  attenuationMask = RaveData2D_zeros(bi, ri, RaveDataType_DOUBLE);

  for (bi = 0; bi < nbins; bi++) {
    double d = 0.0, h = 0.0;
    double vRHOHV = 0, vKDP = 0, vTH = 0;
    PolarNavigator_reToDh(navigator, range * ((double)bi+0.5), elangle, &d, &h);
    h = h / 1000.0;
    if (h < PdpProcessor_getMeltingLayerBottomHeight(scan)) {
      for (ri = 0; ri < nrays; ri++) {
        RaveData2D_getValueUnchecked(dataRHOHV, bi, ri, &vRHOHV);
        RaveData2D_getValueUnchecked(outKDP, bi, ri, &vKDP);
        RaveData2D_getValueUnchecked(dataTH, bi, ri, &vTH);
        if (vRHOHV > self->minAttenuationMaskRHOHV && vKDP > self->minAttenuationMaskKDP && vTH > self->minAttenuationMaskTH) {
          RaveData2D_setValueUnchecked(attenuationMask, bi, ri, 1.0);
        }
      }
    }
  }
  if (!PdpProcessor_attenuation(self, dataTH, dataZDR, outPDP, attenuationMask, self->attenuationGammaH,
        self->attenuationAlpha, &outAttenuationZ, &outAttenuationZDR, &outAttenuationPIA)) {
    goto done;
  }
  // fprintf(stderr, "attenuation ran after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  /**************************************************************
   * Application of the ZPHI methodology (Testud et al, 2000) for
   * attenuation correction
   **************************************************************/

  if (!PdpProcessor_zphi(self, dataTH, outPDP, attenuationMask, rangeKm,
      self->BB, self->attenuationGammaH, &outZPHI, &outAH)) {
    goto done;
  }
  // fprintf(stderr, "zphi ran after %ld ms\n", PdpProcessorInternal_timestamp() - starttime);

  RaveData2D_useNodata(dataTH, 1);
  RaveData2D_setNodata(dataTH, -999.9);

  RaveData2D_replace(residualClutterMask, RaveData2D_getNodata(residualClutterMask), 0.0);

  tmpresult = RAVE_OBJECT_CLONE(scan);
  if (tmpresult == NULL) {
    goto done;
  }

  if (PdpProcessor_CORR_TH & self->requestedFieldMask) {
    correctedZ = PdpProcessorInternal_createPolarScanParamFromData2D(dataTH, "CORR_TH", 1, 255.0, 0.0);
    if (correctedZ == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZ)) {
      RAVE_ERROR0("Failed to add corrected TH field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_ATT_TH & self->requestedFieldMask) {
    attenuatedZ = PdpProcessorInternal_createPolarScanParamFromData2D(outAttenuationZ, "CORR_ATT_TH", 1, 255.0, 0.0);
    if (attenuatedZ == NULL ||
        !PolarScan_addParameter(tmpresult, attenuatedZ)) {
      RAVE_ERROR0("Failed to add corrected and attenuated TH field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_KDP & self->requestedFieldMask) {
    paramKDP = PdpProcessorInternal_createPolarScanParamFromData2D(outKDP, "CORR_KDP", 1, 255.0, 0.0);
    if (paramKDP == NULL ||
        !PolarScan_addParameter(tmpresult, paramKDP)) {
      RAVE_ERROR0("Failed to add corrected KDP field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_RHOHV & self->requestedFieldMask) {
    paramRHOHV = PdpProcessorInternal_createPolarScanParamFromData2D(outKDP, "CORR_RHOHV", 1, 255.0, 0.0);
    if (paramRHOHV == NULL ||
        !PolarScan_addParameter(tmpresult, paramRHOHV)) {
      RAVE_ERROR0("Failed to add corrected RHOHV field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_PHIDP & self->requestedFieldMask) {
    correctedPDP = PdpProcessorInternal_createPolarScanParamFromData2D(outPDP, "CORR_PHIDP", 1, 255.0, 0.0);
    if (correctedPDP == NULL ||
        !PolarScan_addParameter(tmpresult, correctedPDP)) {
      RAVE_ERROR0("Failed to add corrected PDP field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_ZDR & self->requestedFieldMask) {
    correctedZDR = PdpProcessorInternal_createPolarScanParamFromData2D(outPDP, "CORR_ZDR", 1, 255.0, 0.0);
    if (correctedZDR == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZDR)) {
      RAVE_ERROR0("Failed to add corrected ZDR field");
      goto done;
    }
  }

  if (PdpProcessor_CORR_ZPHI & self->requestedFieldMask) {
    correctedZPHI = PdpProcessorInternal_createPolarScanParamFromData2D(outPDP, "CORR_ZPHI", 1, 255.0, 0.0);
    if (correctedZPHI == NULL ||
        !PolarScan_addParameter(tmpresult, correctedZPHI)) {
      RAVE_ERROR0("Failed to add corrected >ZPHI field");
      goto done;
    }
  }

  if (PdpProcessor_QUALITY_RESIDUAL_CLUTTER_MASK & self->requestedFieldMask) {
    if (!PdpProcessorInternal_addRaveQualityFieldToScanFromData2D(result, residualClutterMask, "se.baltrad.ppc.residual_clutter_mask")) {
      goto done;
    }
  }

  if (PdpProcessor_QUALITY_ATTENUATION_MASK & self->requestedFieldMask) {
    if (!PdpProcessorInternal_addRaveQualityFieldToScanFromData2D(result, attenuationMask, "se.baltrad.ppc.attenuation_mask")) {
      goto done;
    }
  }

  fprintf(stderr, "PdpProcessor_process: Total execution time for scan: %lld ms\n", PdpProcessorInternal_timestamp() - starttime);

  result = RAVE_OBJECT_COPY(tmpresult);
done:
  RAVE_OBJECT_RELEASE(dataTH);
  RAVE_OBJECT_RELEASE(dataZDR);
  RAVE_OBJECT_RELEASE(dataDV);
  RAVE_OBJECT_RELEASE(texturePHIDP);
  RAVE_OBJECT_RELEASE(dataRHOHV);
  RAVE_OBJECT_RELEASE(textureZ);
  RAVE_OBJECT_RELEASE(dataPHIDP);
  RAVE_OBJECT_RELEASE(dataPDP);
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
  RAVE_OBJECT_RELEASE(outZPHI);
  RAVE_OBJECT_RELEASE(outAH);
  RAVE_OBJECT_RELEASE(navigator);
  RAVE_OBJECT_RELEASE(TH);
  RAVE_OBJECT_RELEASE(ZDR);
  RAVE_OBJECT_RELEASE(DV);
  RAVE_OBJECT_RELEASE(PHIDP);
  RAVE_OBJECT_RELEASE(RHOHV);
  RAVE_OBJECT_RELEASE(pdpQualityField);
  RAVE_OBJECT_RELEASE(correctedZ);
  RAVE_OBJECT_RELEASE(correctedZDR);
  RAVE_OBJECT_RELEASE(correctedZPHI);
  RAVE_OBJECT_RELEASE(correctedPDP);
  RAVE_OBJECT_RELEASE(attenuatedZ);
  RAVE_OBJECT_RELEASE(paramKDP);
  RAVE_OBJECT_RELEASE(paramRHOHV);
  RAVE_OBJECT_RELEASE(tmpresult);

  return result;
}

void PdpProcessor_setKdpUp(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpUp = v;
}

double PdpProcessor_getKdpUp(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpUp;
}

void PdpProcessor_setKdpDown(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpDown = v;
}

double PdpProcessor_getKdpDown(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpDown;
}

void PdpProcessor_setKdpStdThreshold(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpStdThreshold = v;
}

double PdpProcessor_getKdpStdThreshold(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpStdThreshold;
}

void PdpProcessor_setBB(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->BB = v;
}

double PdpProcessor_getBB(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->BB;
}

void PdpProcessor_setThresholdPhidp(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->thresholdPhidp = v;
}

double PdpProcessor_getThresholdPhidp(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->thresholdPhidp;
}

void PdpProcessor_setMinWindow(PdpProcessor_t* self, long window)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (window <= 0) {
    RAVE_ERROR0("Window size must be > 0");
    return;
  }
  self->minWindow = window;
}

long PdpProcessor_getMinWindow(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minWindow;
}


int PdpProcessor_setBand(PdpProcessor_t* self, char band)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (band == 's') {
    self->kdpUp = 14;
    self->kdpDown = -2;
    self->kdpStdThreshold = 5;
  } else if (band == 'c') {
    self->kdpUp = 20;
    self->kdpDown = -2;
    self->kdpStdThreshold = 5;
  } else if (band == 'x') {
    self->kdpUp = 40;
    self->kdpDown = -2;
    self->kdpStdThreshold = 5;
  } else {
    return 0; // No such band
  }
  return 1;
}

void PdpProcessor_setPdpRWin1(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpRWin1 = v;
}

double PdpProcessor_getPdpRWin1(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpRWin1;
}

void PdpProcessor_setPdpRWin2(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpRWin2 = v;
}

double PdpProcessor_getPdpRWin2(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpRWin2;
}

void PdpProcessor_setPdpNrIterations(PdpProcessor_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpNrIterations = v;
}

long PdpProcessor_getPdpNrIterations(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpNrIterations;
}

/**
 * Helper to simplify work with the parameter constants
 */
static void PdpProcessorInternal_getParameters(double* pars, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  *weight = pars[0];
  *X2 = pars[1];
  *X3 = pars[2];
  *delta1 = pars[3];
  *delta2 = pars[4];
}

/**
 * Helper to simplify work with the parameter constants
 */
static void PdpProcessorInternal_setParameters(double* pars, double weight, double X2, double X3, double delta1, double delta2)
{
  pars[0] = weight;
  pars[1] = X2;
  pars[2] = X3;
  pars[3] = delta1;
  pars[4] = delta2;
}

void PdpProcessor_getParametersUZ(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parUZ, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_setParametersUZ(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parUZ, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_getParametersVEL(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parVel, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_setParametersVEL(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parVel, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_getParametersTEXT_PHIDP(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parTextPHIDP, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_setParametersTEXT_PHIDP(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parTextPHIDP, weight, X2, X3, delta1, delta2);
}

void PdpProcessor_getParametersRHV(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parRHV, weight, X2, X3, delta1, delta2);

}

void PdpProcessor_setParametersRHV(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parRHV, weight, X2, X3, delta1, delta2);

}

void PdpProcessor_getParametersTEXT_UZ(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parTextUZ, weight, X2, X3, delta1, delta2);

}

void PdpProcessor_setParametersTEXT_UZ(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parTextUZ, weight, X2, X3, delta1, delta2);

}

void PdpProcessor_getParametersCLUTTER_MAP(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_getParameters(self->parClutterMap, weight, X2, X3, delta1, delta2);

}

void PdpProcessor_setParametersCLUTTER_MAP(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PdpProcessorInternal_setParameters(self->parClutterMap, weight, X2, X3, delta1, delta2);

}


double PdpProcessor_getMeltingLayerBottomHeight(PolarScan_t* scan)
{
  /* @TODO: implement proper support for this */
  return 2.463;
}

void PdpProcessor_setNodata(PdpProcessor_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->nodata = nodata;
}

double PdpProcessor_getNodata(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}

void PdpProcessor_setMinDBZ(PdpProcessor_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minDBZ = minv;
}

double PdpProcessor_getMinDBZ(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minDBZ;
}

void PdpProcessor_setQualityThreshold(PdpProcessor_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->qualityThreshold = minv;
}

double PdpProcessor_getQualityThreshold(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->qualityThreshold;
}

void PdpProcessor_setResidualMinZClutterThreshold(PdpProcessor_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualMinZClutterThreshold = minv;
}

double PdpProcessor_getResidualMinZClutterThreshold(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualMinZClutterThreshold;
}

void PdpProcessor_setResidualThresholdZ(PdpProcessor_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualThresholdZ = minv;
}

double PdpProcessor_getResidualThresholdZ(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualThresholdZ;
}

void PdpProcessor_setResidualThresholdTexture(PdpProcessor_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualThresholdTexture = minv;
}

double PdpProcessor_getResidualThresholdTexture(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualThresholdTexture;
}

void PdpProcessor_setResidualClutterNodata(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterNodata = v;
}

double PdpProcessor_getResidualClutterNodata(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterNodata;
}

void PdpProcessor_setResidualClutterMaskNodata(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterMaskNodata = v;
}

double PdpProcessor_getResidualClutterMaskNodata(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterMaskNodata;
}

void PdpProcessor_setResidualClutterTextureFilteringMaxZ(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterTextureFilteringMaxZ = v;
}

double PdpProcessor_getResidualClutterTextureFilteringMaxZ(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterTextureFilteringMaxZ;
}

void PdpProcessor_setResidualFilterBinSize(PdpProcessor_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualFilterBinSize = v;
}

long PdpProcessor_getResidualFilterBinSize(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualFilterBinSize;
}

void PdpProcessor_setResidualFilterRaySize(PdpProcessor_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualFilterRaySize = v;
}

long PdpProcessor_getResidualFilterRaySize(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualFilterRaySize;
}

void PdpProcessor_setMinZMedfilterThreshold(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minZMedfilterThreshold = v;
}

double PdpProcessor_getMinZMedfilterThreshold(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minZMedfilterThreshold;
}

void PdpProcessor_setProcessingTextureThreshold(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->processingTextureThreshold = v;
}

double PdpProcessor_getProcessingTextureThreshold(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->processingTextureThreshold;
}

void PdpProcessor_setMinAttenuationMaskRHOHV(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskRHOHV = v;
}

double PdpProcessor_getMinAttenuationMaskRHOHV(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskRHOHV;
}

void PdpProcessor_setMinAttenuationMaskKDP(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskKDP = v;
}

double PdpProcessor_getMinAttenuationMaskKDP(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskKDP;
}

void PdpProcessor_setMinAttenuationMaskTH(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskTH = v;
}

double PdpProcessor_getMinAttenuationMaskTH(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskTH;
}

void PdpProcessor_setAttenuationGammaH(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationGammaH = v;
}

double PdpProcessor_getAttenuationGammaH(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationGammaH;
}

void PdpProcessor_setAttenuationAlpha(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationAlpha = v;
}

double PdpProcessor_getAttenuationAlpha(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationAlpha;
}

void PdpProcessor_setAttenuationPIAminZ(PdpProcessor_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationPIAminZ = v;
}

double PdpProcessor_getAttenuationPIAminZ(PdpProcessor_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationPIAminZ;
}

//
//static double internal_get(RaveData2D_t* f, long x, long y) {
//  double v;
//  RaveData2D_getValueUnchecked(f,  x,  y,  &v);
//  return v;
//}

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
  RaveData2D_t* field = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (xarr == NULL) {
    RAVE_ERROR0("Passing xarr as NULL");
    return NULL;
  }
  xsize = RaveData2D_getXsize(xarr);
  ysize = RaveData2D_getYsize(xarr);
  field = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (field == NULL)
    return NULL;

  for (xi = 0; xi < xsize; xi++) {
    for (yi = 0; yi < ysize; yi++) {
      double x = 0.0, out = 0.0;
      RaveData2D_getValueUnchecked(xarr, xi, yi, &x);
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
  double sumWeight = self->parUZ[0] + self->parVel[0] + self->parTextPHIDP[0] + self->parRHV[0] + self->parTextUZ[0] + self->parClutterMap[0];

  RaveData2D_t* degree = NULL;
  RaveData2D_t* result = NULL;
  RaveData2D_t *mbf_Z = NULL, *mbf_VRADH = NULL, *mbf_texturePHIDP = NULL;
  RaveData2D_t *mbf_RHOHV = NULL, *mbf_textureZ = NULL, *mbf_clutterMap = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");

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
  mbf_Z = PdpProcessor_trap(self, Z, self->parUZ[1], self->parUZ[2], self->parUZ[3], self->parUZ[4]);
  mbf_VRADH = PdpProcessor_trap(self, VRADH, self->parVel[1], self->parVel[2], self->parVel[3], self->parVel[4]);
  mbf_texturePHIDP = PdpProcessor_trap(self, texturePHIDP, self->parTextPHIDP[1], self->parTextPHIDP[2], self->parTextPHIDP[3], self->parTextPHIDP[4]);
  mbf_RHOHV = PdpProcessor_trap(self, RHOHV, self->parRHV[1], self->parRHV[2], self->parRHV[3], self->parRHV[4]);
  mbf_textureZ = PdpProcessor_trap(self, textureZ, self->parTextUZ[1], self->parTextUZ[2], self->parTextUZ[3], self->parTextUZ[4]);
  mbf_clutterMap = PdpProcessor_trap(self, clutterMap, self->parClutterMap[1], self->parClutterMap[2], self->parClutterMap[3], self->parClutterMap[4]);
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
        vDegree = self->parUZ[0] * vZ + self->parVel[0] * vVRADH + self->parTextPHIDP[0] * vTexturePHIDP +
            self->parRHV[0] * vRHOHV + self->parTextUZ[0] * vTextureZ + self->parClutterMap[0] * vClutterMap;
        vDegree /= sumWeight;
        //fprintf(stderr, "x=%ld, y=%ld => vDegree1 = %f\n", x, y, vDegree);
      }
      if (inVRADH == nodataVRADH && inZ != nodataZ) {
        vDegree = self->parUZ[0] * vZ + self->parVel[0] * vVRADH + self->parTextPHIDP[0] * vTexturePHIDP +
            self->parRHV[0] * vRHOHV + self->parTextUZ[0] * vTextureZ + self->parClutterMap[0] * vClutterMap;
        vDegree /= sumWeight;
        //fprintf(stderr, "x=%ld, y=%ld => vDegree2 = %f\n", x, y, vDegree);
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
  //fprintf(stderr, "quality=%s\n", RaveData2D_str(quality));
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double vZ = 0.0, vQ = 0.0;
      RaveData2D_getValueUnchecked(Z, x, y, &vZ);
      RaveData2D_getValueUnchecked(quality, x, y, &vQ);
      if (vZ >= self->minDBZ && vZ != nodataZ && vQ < qualityThreshold) {
//        if (vZ != -32.0)
//          fprintf(stderr, "(%ld, %ld) = %f\n", x, y, vZ);
        RaveData2D_setValueUnchecked(Z2, x, y, nodataZ);
        RaveData2D_setValueUnchecked(clutterMask, x, y, 1.0);
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

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      RaveData2D_getValueUnchecked(filtmask, x, y, &v);
      if (v == 0.0)
        RaveData2D_setValueUnchecked(zout, x, y, minVal);
      RaveData2D_getValueUnchecked(Z, x, y, &v);
      if (v >= minVal && v < self->minZMedfilterThreshold) {
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

  img = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  RaveData2D_setNodata(img, self->residualClutterNodata);
  RaveData2D_useNodata(img, 1);

  mask = RaveData2D_zeros(xsize, ysize, RaveDataType_DOUBLE);
  if (img == NULL || mask == NULL) {
    goto done;
  }
  // fprintf(stderr, "residualClutterFilter: Beginning at: %ld\n", PdpProcessorInternal_timestamp() - starttime);
  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(Z, x, y, &v);
      if (v < self->residualMinZClutterThreshold || v == nodata) {
        RaveData2D_setValueUnchecked(img, x, y, self->residualClutterNodata); /* TODO: Specify as residualClutterNodata? */
      } else {
        RaveData2D_setValueUnchecked(img, x, y, v);
        if (v > thresholdZ) {
          RaveData2D_setValueUnchecked(mask, x, y, 1.0);
        }
      }
    }
  }

  // fprintf(stderr, "residualClutterFilter: First loop done: %ld\n", PdpProcessorInternal_timestamp() - starttime);

  textureZ = PdpProcessor_texture(self, img);

  // fprintf(stderr, "residualClutterFilter: Texture created at: %ld\n", PdpProcessorInternal_timestamp() - starttime);

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

  // fprintf(stderr, "residualClutterFilter: Second loop: %ld\n", PdpProcessorInternal_timestamp() - starttime);

  nh = ((double)nhctr) / (double)(xsize*ysize*100.0);
  if (!RaveData2D_entropy(mask, 2, &EN)) {
    RAVE_ERROR0("Failed to calculate entropy");
    goto done;
  }
  // fprintf(stderr, "residualClutterFilter: Entropy at: %ld\n", PdpProcessorInternal_timestamp() - starttime);

  if (nh <= 70.0 && EN > 5e-4) {
    Zout = PdpProcessor_medfilt(self, img, thresholdZ, nodata, filtXsize, filtYsize);
    if (Zout == NULL) {
      goto done;
    }
    // fprintf(stderr, "residualClutterFilter: First medfilt at: %ld\n", PdpProcessorInternal_timestamp() - starttime);

    RaveData2D_setNodata(Zout, self->residualClutterNodata);
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
        if (v >= self->residualClutterTextureFilteringMaxZ) {
          RaveData2D_setValueUnchecked(Zout, x, y, minZ);
        }
      }
    }
    medZ = PdpProcessor_medfilt(self, Zout, thresholdZ, nodata, filtXsize, filtYsize);
    // fprintf(stderr, "residualClutterFilter: Second medfilt at: %ld\n", PdpProcessorInternal_timestamp() - starttime);
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
        if (v <= self->residualMinZClutterThreshold) {
          RaveData2D_setValueUnchecked(mask, x, y, self->residualClutterMaskNodata);
        }
      }
    }
  }
  // fprintf(stderr, "residualClutterFilter: Done: %ld\n", PdpProcessorInternal_timestamp() - starttime);

  RaveData2D_setNodata(mask, self->residualClutterMaskNodata);
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
      Kdpv = 0.5 * (Bx - Ax) / 2 * dr * window;
      if (Kdpv < self->kdpDown || Kdpv > self->kdpUp) {
        Kdpv = 0.0;
      }
      if (x < window || x >= xsize - window ) { /* Side effects compensation */
        Kdpv = 0.0;
      }
      if (Kdpv > self->kdpStdThreshold) {
        Kdpv = 0.0;
      }
      RaveData2D_setValueUnchecked(kdpres, x, y, Kdpv);
    }
  }
  // fprintf(stderr, "pdpProcessing: First loop %ld\n", PdpProcessorInternal_timestamp() - starttime);

  stdK = RaveData2D_movingstd(kdpres, window, 0); /* In matlab they use 0, window as inparam, but they are used as window, 0 in array...... */

  // fprintf(stderr, "pdpProcessing: Moving std %ld\n", PdpProcessorInternal_timestamp() - starttime);

  for (x = 0; x < xsize; x++) {
    for (y = 0; y < ysize; y++) {
      double v = 0.0;
      RaveData2D_getValueUnchecked(stdK,  x, y, &v);
      if (v > self->kdpStdThreshold) {
        RaveData2D_setValueUnchecked(kdpres, x, y, 0.0);
      }
    }
  }
  // fprintf(stderr, "pdpProcessing: Second loop %ld\n", PdpProcessorInternal_timestamp() - starttime);

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
        if (v < self->kdpDown) {
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
        Kdpv = 0.5 * (Bx - Ax) / 2 * dr * window;
        if (Kdpv < self->kdpDown || Kdpv > self->kdpUp) {
          Kdpv = 0.0;
        }
        if (x < window || x >= xsize - window ) { /* Side effects compensation */
          Kdpv = 0.0;
        }
        if (Kdpv > self->kdpStdThreshold) {
          Kdpv = 0.0;
        }
        RaveData2D_setValueUnchecked(kdpres, x, y, Kdpv);
      }
    }

    RAVE_OBJECT_RELEASE(tmp);
    RAVE_OBJECT_RELEASE(tmp2);
  }
  // fprintf(stderr, "pdpProcessing: Third loop %ld\n", PdpProcessorInternal_timestamp() - starttime);

  tmp = RaveData2D_mulNumber(kdpres, 2.0 * dr);
  if (tmp == NULL) {
    goto done;
  }

  // fprintf(stderr, "pdpProcessing: mulNumber %ld\n", PdpProcessorInternal_timestamp() - starttime);

  RAVE_OBJECT_RELEASE(pdpres);
  pdpres = RaveData2D_cumsum(tmp, 0);
  if (pdpres == NULL) {
    goto done;
  }

  // fprintf(stderr, "pdpProcessing: cumsum %ld\n", PdpProcessorInternal_timestamp() - starttime);

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

  window = round(rWin1/ dr);
  if (window < self->minWindow) {
    window = self->minWindow;
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
      if (v > self->processingTextureThreshold) {
        RaveData2D_setValueUnchecked(pdpwork, x, y, self->nodata);
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
      if (v > self->thresholdPhidp) {
        isempty = 0;
      }
    }
  }

  if (!isempty && rWin2 < rWin1) {
    window = round(rWin2/dr);
    if (window < self->minWindow) {
      window = self->minWindow;
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

int PdpProcessor_attenuation(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* zdr, RaveData2D_t* pdp,
    RaveData2D_t* mask, double gamma_h, double alpha, RaveData2D_t** outz, RaveData2D_t** outzdr, RaveData2D_t** outPIA)
{
  long nrays = 0;
  long nbins = 0;
  int result = 0;
  long ri = 0, bi = 0;
  double vpianodata = 0.0;
  RaveData2D_t *PIA = NULL, *PIDA = NULL;
  RaveData2D_t *zdrres = NULL, *zres = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (Z == NULL || zdr == NULL || pdp == NULL || mask == NULL) {
    RAVE_ERROR0("Z, zdr, pdp or mask is NULL");
    goto done;
  }
  if (outz == NULL || outzdr == NULL || outPIA == NULL) {
    RAVE_ERROR0("Out Z / zdr or PIA is NULL");
    goto done;
  }
  if (!RaveData2D_usingNodata(pdp)) {
    RAVE_ERROR0("pdp is not using nodata");
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
  RaveData2D_setNodata(PIA, vpianodata);
  RaveData2D_useNodata(PIA, 1);

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
  if (zdrres == NULL || zres == NULL) {
    goto done;
  }

  for (ri = 0; ri < nrays; ri++) {
    for (bi = 0; bi < nbins; bi++) {
      double vz = 0, vpia = 0, vzdr = 0, vpida = 0;
      RaveData2D_getValueUnchecked(Z, bi, ri, &vz);
      RaveData2D_getValueUnchecked(PIA, bi, ri, &vpia);
      RaveData2D_getValueUnchecked(zdr, bi, ri, &vzdr);
      RaveData2D_getValueUnchecked(PIDA, bi, ri, &vpida);
      if (vpia != vpianodata && vpia >= 0.0) {
        RaveData2D_setValueUnchecked(zres, bi, ri, vz + vpia);
        RaveData2D_setValueUnchecked(zdrres, bi, ri, vzdr + vpida);
      }
      RaveData2D_getValueUnchecked(zres, bi, ri, &vz);
      if (vz < self->attenuationPIAminZ) {
        RaveData2D_setValueUnchecked(PIA, bi, ri, vpianodata);
      }
    }
  }

  *outz = RAVE_OBJECT_COPY(zres);
  *outzdr = RAVE_OBJECT_COPY(zdrres);
  *outPIA = RAVE_OBJECT_COPY(PIA);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(PIA);
  RAVE_OBJECT_RELEASE(PIDA);
  RAVE_OBJECT_RELEASE(zdrres);
  RAVE_OBJECT_RELEASE(zres);
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
