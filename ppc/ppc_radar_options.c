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
 * Keeps one radar options setup
 * This object does support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-05-13
 */
#include "ppc_radar_options.h"

#include <string.h>
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_utilities.h"
#include <time.h>
#include <sys/time.h>
#include "pdp_processor.h"

/**
 * The radar options class
 */
struct _PpcRadarOptions_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char* name; /**< name of owner for these options */
  char* defaultName; /**< name of default setting for these options */
  double parUZ[5]; /**< parameters for TH */
  double parVel[5]; /**< parameters for VRADH */
  double parTextPHIDP[5]; /**< parameters for the PHIDP texture */
  double parRHV[5]; /**< parameters for RHOHV */
  double parTextUZ[5]; /**< parameters for the TH texture */
  double parClutterMap[5]; /**< parameters for the clutter map */
  double nodata; /**< nodata to used in most products */
  double minDBZ; /**< min DBZ threshold in the clutter correction */
  double qualityThreshold; /**< quality threshold in the clutter correction */
  double preprocessZThreshold; /**< preprocessing Z threshold before starting actual processing */
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

  double meltingLayerBottomHeight; /**< the default melting layer bottom height */
  long meltingLayerHourThreshold; /**< number of hours before default height is used */
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
static int PpcRadarOptions_constructor(RaveCoreObject* obj)
{
  PpcRadarOptions_t* options = (PpcRadarOptions_t*)obj;
  options->name = NULL;
  options->defaultName = NULL;
  memcpy(options->parUZ,  DEFAULT_PAR_UZ, sizeof(options->parUZ));
  memcpy(options->parVel, DEFAULT_PAR_VEL, sizeof(options->parVel));
  memcpy(options->parTextPHIDP, DEFAULT_PAR_TEXT_PHIDP, sizeof(options->parTextPHIDP));
  memcpy(options->parRHV, DEFAULT_PAR_RHV, sizeof(options->parRHV));
  memcpy(options->parTextUZ, DEFAULT_PAR_TEXT_UZ, sizeof(options->parTextUZ));
  memcpy(options->parClutterMap, DEFAULT_PAR_CLUTTER_MAP, sizeof(options->parClutterMap));
  options->minWindow = 11;
  options->nodata = -999.0;
  options->minDBZ = -32.0;
  options->qualityThreshold = 0.75;
  options->preprocessZThreshold = -20.0;
  options->residualMinZClutterThreshold = -31.5;
  options->residualClutterNodata = -999.0;
  options->residualClutterMaskNodata = -1.0;
  options->residualThresholdZ = -20.0;
  options->residualThresholdTexture = 20.0;
  options->residualFilterBinSize = 1;
  options->residualFilterRaySize = 1;
  options->residualClutterTextureFilteringMaxZ = 70.0;

  options->minZMedfilterThreshold = -30.0;
  options->processingTextureThreshold = 10.0;
  options->pdpRWin1 = 3.5;
  options->pdpRWin2 = 1.5;
  options->pdpNrIterations = 2;

  options->kdpUp = 20.0; /**< c band */
  options->kdpDown = -2.0; /**< c band */
  options->kdpStdThreshold = 5.0; /**< c band */
  options->BB = 0.7987; /* C-band */
  options->thresholdPhidp = 40.0;

  options->minAttenuationMaskRHOHV = 0.8;
  options->minAttenuationMaskKDP = 0.001;
  options->minAttenuationMaskTH = -20.0;
  options->attenuationGammaH = 0.08;
  options->attenuationAlpha = 0.2;
  options->attenuationPIAminZ = -30;
  options->meltingLayerBottomHeight = 2.463;
  options->meltingLayerHourThreshold = 6;

  options->requestedFieldMask = PpcRadarOptions_DBZH_CORR|PpcRadarOptions_ATT_DBZH_CORR|PpcRadarOptions_PHIDP_CORR|PpcRadarOptions_QUALITY_RESIDUAL_CLUTTER_MASK;

  return 1;
}

/**
 * Destructor
 */
static void PpcRadarOptions_destructor(RaveCoreObject* obj)
{
  PpcRadarOptions_t* options = (PpcRadarOptions_t*)obj;
  RAVE_FREE(options->name);
  RAVE_FREE(options->defaultName);
}

/**
 * Copy constructor
 */
static int PpcRadarOptions_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PpcRadarOptions_t* this = (PpcRadarOptions_t*)obj;
  PpcRadarOptions_t* src = (PpcRadarOptions_t*)srcobj;
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
  this->preprocessZThreshold = src->preprocessZThreshold;
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

  this->meltingLayerBottomHeight = src->meltingLayerBottomHeight;
  this->meltingLayerHourThreshold = src->meltingLayerHourThreshold;

  this->requestedFieldMask = src->requestedFieldMask;

  this->name = NULL;
  this->defaultName = NULL;
  if (src->name != NULL) {
    this->name = RAVE_STRDUP(src->name);
    if (this->name == NULL) {
      RAVE_ERROR0("Failed to duplicate option name");
      goto done;
    }
  }
  if (src->defaultName != NULL) {
    this->defaultName = RAVE_STRDUP(src->defaultName);
    if (this->defaultName == NULL) {
      RAVE_ERROR0("Failed to duplicate default name");
      goto done;
    }
  }
  return 1;
done:
  RAVE_FREE(this->name);
  RAVE_FREE(this->defaultName);
  return 0;
}

/*@} End of Private functions */

/*@{ Interface functions */
int PpcRadarOptions_setName(PpcRadarOptions_t* self, const char* name)
{
  int result = 0;
  char* tmp = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (name != NULL) {
    tmp = RAVE_STRDUP(name);
    if (tmp == NULL) {
      goto done;
    }
  }
  RAVE_FREE(self->name);
  self->name = tmp;
done:
  return result;
}

const char* PpcRadarOptions_getName(PpcRadarOptions_t* self)
{
  return (const char*)self->name;
}

int PpcRadarOptions_setDefaultName(PpcRadarOptions_t* self, const char* name)
{
  int result = 0;
  char* tmp = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (name != NULL) {
    tmp = RAVE_STRDUP(name);
    if (tmp == NULL) {
      goto done;
    }
  }
  RAVE_FREE(self->defaultName);
  self->defaultName = tmp;
  result = 1;
done:
  return result;
}

const char* PpcRadarOptions_getDefaultName(PpcRadarOptions_t* self)
{
  return (const char*)self->defaultName;
}


void PpcRadarOptions_setRequestedFields(PpcRadarOptions_t* self, int fieldmask)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->requestedFieldMask = fieldmask;
}

int PpcRadarOptions_getRequestedFields(PpcRadarOptions_t* self)
{
  return self->requestedFieldMask;
}

int PpcRadarOptions_setBand(PpcRadarOptions_t* self, char band)
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

void PpcRadarOptions_setKdpUp(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpUp = v;
}

double PpcRadarOptions_getKdpUp(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpUp;
}

void PpcRadarOptions_setKdpDown(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpDown = v;
}

double PpcRadarOptions_getKdpDown(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpDown;
}

void PpcRadarOptions_setKdpStdThreshold(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->kdpStdThreshold = v;
}

double PpcRadarOptions_getKdpStdThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->kdpStdThreshold;
}

void PpcRadarOptions_setBB(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->BB = v;
}

double PpcRadarOptions_getBB(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->BB;
}

void PpcRadarOptions_setThresholdPhidp(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->thresholdPhidp = v;
}

double PpcRadarOptions_getThresholdPhidp(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  return self->thresholdPhidp;
}

void PpcRadarOptions_setMinWindow(PpcRadarOptions_t* self, long window)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (window <= 0) {
    RAVE_ERROR0("Window size must be > 0");
    return;
  }
  self->minWindow = window;
}

long PpcRadarOptions_getMinWindow(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minWindow;
}

void PpcRadarOptions_setPdpRWin1(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpRWin1 = v;
}

double PpcRadarOptions_getPdpRWin1(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpRWin1;
}

void PpcRadarOptions_setPdpRWin2(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpRWin2 = v;
}

double PpcRadarOptions_getPdpRWin2(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpRWin2;
}

void PpcRadarOptions_setPdpNrIterations(PpcRadarOptions_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->pdpNrIterations = v;
}

long PpcRadarOptions_getPdpNrIterations(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->pdpNrIterations;
}

/**
 * Helper to simplify work with the parameter constants
 */
static void PpcRadarOptionsInternal_getParameters(double* pars, double* weight, double* X2, double* X3, double* delta1, double* delta2)
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
static void PpcRadarOptionsInternal_setParameters(double* pars, double weight, double X2, double X3, double delta1, double delta2)
{
  pars[0] = weight;
  pars[1] = X2;
  pars[2] = X3;
  pars[3] = delta1;
  pars[4] = delta2;
}

void PpcRadarOptions_getParametersUZ(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parUZ, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_setParametersUZ(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parUZ, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_getParametersVEL(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parVel, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_setParametersVEL(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parVel, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_getParametersTEXT_PHIDP(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parTextPHIDP, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_setParametersTEXT_PHIDP(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parTextPHIDP, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_getParametersRHV(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parRHV, weight, X2, X3, delta1, delta2);

}

void PpcRadarOptions_setParametersRHV(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parRHV, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_getParametersTEXT_UZ(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parTextUZ, weight, X2, X3, delta1, delta2);

}

void PpcRadarOptions_setParametersTEXT_UZ(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parTextUZ, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_getParametersCLUTTER_MAP(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_getParameters(self->parClutterMap, weight, X2, X3, delta1, delta2);

}

void PpcRadarOptions_setParametersCLUTTER_MAP(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  PpcRadarOptionsInternal_setParameters(self->parClutterMap, weight, X2, X3, delta1, delta2);
}

void PpcRadarOptions_setMeltingLayerBottomHeight(PpcRadarOptions_t* self, double height)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->meltingLayerBottomHeight = height;
}

double PpcRadarOptions_getMeltingLayerBottomHeight(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->meltingLayerBottomHeight;
}

void PpcRadarOptions_setMeltingLayerHourThreshold(PpcRadarOptions_t* self, long hours)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->meltingLayerHourThreshold = hours;
}

long PpcRadarOptions_getMeltingLayerHourThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->meltingLayerHourThreshold;
}


void PpcRadarOptions_setNodata(PpcRadarOptions_t* self, double nodata)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  self->nodata = nodata;
}

double PpcRadarOptions_getNodata(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodata;
}

void PpcRadarOptions_setMinDBZ(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  self->minDBZ = minv;
}

double PpcRadarOptions_getMinDBZ(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minDBZ;
}

void PpcRadarOptions_setQualityThreshold(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  self->qualityThreshold = minv;
}

double PpcRadarOptions_getQualityThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->qualityThreshold;
}

void PpcRadarOptions_setPreprocessZThreshold(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  self->preprocessZThreshold = minv;
}

double PpcRadarOptions_getPreprocessZThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->preprocessZThreshold;
}

void PpcRadarOptions_setResidualMinZClutterThreshold(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");

  self->residualMinZClutterThreshold = minv;
}

double PpcRadarOptions_getResidualMinZClutterThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualMinZClutterThreshold;
}

void PpcRadarOptions_setResidualThresholdZ(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualThresholdZ = minv;
}

double PpcRadarOptions_getResidualThresholdZ(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualThresholdZ;
}

void PpcRadarOptions_setResidualThresholdTexture(PpcRadarOptions_t* self, double minv)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualThresholdTexture = minv;
}

double PpcRadarOptions_getResidualThresholdTexture(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualThresholdTexture;
}

void PpcRadarOptions_setResidualClutterNodata(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterNodata = v;
}

double PpcRadarOptions_getResidualClutterNodata(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterNodata;
}

void PpcRadarOptions_setResidualClutterMaskNodata(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterMaskNodata = v;
}

double PpcRadarOptions_getResidualClutterMaskNodata(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterMaskNodata;
}

void PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualClutterTextureFilteringMaxZ = v;
}

double PpcRadarOptions_getResidualClutterTextureFilteringMaxZ(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualClutterTextureFilteringMaxZ;
}

void PpcRadarOptions_setResidualFilterBinSize(PpcRadarOptions_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualFilterBinSize = v;
}

long PpcRadarOptions_getResidualFilterBinSize(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualFilterBinSize;
}

void PpcRadarOptions_setResidualFilterRaySize(PpcRadarOptions_t* self, long v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->residualFilterRaySize = v;
}

long PpcRadarOptions_getResidualFilterRaySize(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->residualFilterRaySize;
}

void PpcRadarOptions_setMinZMedfilterThreshold(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minZMedfilterThreshold = v;
}

double PpcRadarOptions_getMinZMedfilterThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minZMedfilterThreshold;
}

void PpcRadarOptions_setProcessingTextureThreshold(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->processingTextureThreshold = v;
}

double PpcRadarOptions_getProcessingTextureThreshold(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->processingTextureThreshold;
}

void PpcRadarOptions_setMinAttenuationMaskRHOHV(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskRHOHV = v;
}

double PpcRadarOptions_getMinAttenuationMaskRHOHV(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskRHOHV;
}

void PpcRadarOptions_setMinAttenuationMaskKDP(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskKDP = v;
}

double PpcRadarOptions_getMinAttenuationMaskKDP(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskKDP;
}

void PpcRadarOptions_setMinAttenuationMaskTH(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->minAttenuationMaskTH = v;
}

double PpcRadarOptions_getMinAttenuationMaskTH(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->minAttenuationMaskTH;
}

void PpcRadarOptions_setAttenuationGammaH(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationGammaH = v;
}

double PpcRadarOptions_getAttenuationGammaH(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationGammaH;
}

void PpcRadarOptions_setAttenuationAlpha(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationAlpha = v;
}

double PpcRadarOptions_getAttenuationAlpha(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationAlpha;
}

void PpcRadarOptions_setAttenuationPIAminZ(PpcRadarOptions_t* self, double v)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->attenuationPIAminZ = v;
}

double PpcRadarOptions_getAttenuationPIAminZ(PpcRadarOptions_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->attenuationPIAminZ;
}


/*@} End of Interface functions */

RaveCoreObjectType PpcRadarOptions_TYPE = {
    "PpcRadarOptions",
    sizeof(PpcRadarOptions_t),
    PpcRadarOptions_constructor,
    PpcRadarOptions_destructor,
    PpcRadarOptions_copyconstructor
};

