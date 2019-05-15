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
 * Main routine for the ppc options loader
 * This object does support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2019-05-13
 */
#include <string.h>
#include "ppc_options.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_utilities.h"
#include <time.h>
#include <sys/time.h>
#include "rave_simplexml.h"


/**
 * The ppc class
 */
struct _PpcOptions_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char* filename; /**< name of the loaded options */
  RaveObjectHashTable_t* radaroptions;
  RaveObjectHashTable_t* radarTagNames;
};

/*@{ Private functions */
/**
 * Constructor
 */
static int PpcOptions_constructor(RaveCoreObject* obj)
{
  PpcOptions_t* options = (PpcOptions_t*)obj;
  options->filename = NULL;
  options->radaroptions = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  options->radarTagNames = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (options->radaroptions == NULL || options->radarTagNames == NULL) {
    RAVE_OBJECT_RELEASE(options->radaroptions);
    RAVE_OBJECT_RELEASE(options->radarTagNames);
    return 0;
  }
  return 1;
}

/**
 * Destructor
 */
static void PpcOptions_destructor(RaveCoreObject* obj)
{
  PpcOptions_t* options = (PpcOptions_t*)obj;
  RAVE_FREE(options->filename);
  RAVE_OBJECT_RELEASE(options->radaroptions);
  RAVE_OBJECT_RELEASE(options->radarTagNames);
}

/**
 * Copy constructor
 */
static int PpcOptions_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PpcOptions_t* this = (PpcOptions_t*)obj;
  PpcOptions_t* src = (PpcOptions_t*)srcobj;
  this->filename = NULL;
  this->radaroptions = NULL;
  this->radarTagNames = NULL;

  if (src->filename != NULL) {
    this->filename = RAVE_STRDUP(src->filename);
    if (this->filename == NULL) {
      goto fail;
    }
  }

  this->radaroptions = RAVE_OBJECT_CLONE(src->radaroptions);
  if (this->radaroptions == NULL) {
    RAVE_ERROR0("Failed to clone options");
    goto fail;
  }

  this->radarTagNames = RAVE_OBJECT_CLONE(src->radarTagNames);
  if (this->radarTagNames == NULL) {
    RAVE_ERROR0("Failed to clone tag names");
    goto fail;
  }

  return 1;
fail:
  RAVE_FREE(this->filename);
  RAVE_OBJECT_RELEASE(this->radaroptions);
  RAVE_OBJECT_RELEASE(this->radarTagNames);
  return 0;
}

static int PpcOptionsInternal_addTagName(RaveObjectHashTable_t* tagNames, const char* name)
{
  int result = 0;
  RaveAttribute_t* attr = RaveAttributeHelp_createLong(name, 1);
  if (attr != NULL) {
    result = RaveObjectHashTable_put(tagNames, name, (RaveCoreObject*)attr);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

static int PpcOptionsInternal_setParametersFun(SimpleXmlNode_t* child, PpcRadarOptions_t* options, RaveObjectHashTable_t* tagNames, const char* name, void (*paramfun)(PpcRadarOptions_t*, double, double, double, double, double))
{
  int result = 0;
  const char* value = SimpleXmlNode_getAttribute(child, "value");
  if (value != NULL) {
    RaveList_t* tokens = RaveUtilities_getTrimmedTokens(value, ',');
    if (tokens != NULL && RaveList_size(tokens) == 5) {
      double weight = 0.0, X2 = 0.0, X3 = 0.0, delta1 = 0.0, delta2 = 0.0;
      if (sscanf((const char*)RaveList_get(tokens, 0), "%lf", &weight) != 1 ||
          sscanf((const char*)RaveList_get(tokens, 1), "%lf", &X2) != 1 ||
          sscanf((const char*)RaveList_get(tokens, 2), "%lf", &X3) != 1 ||
          sscanf((const char*)RaveList_get(tokens, 3), "%lf", &delta1) != 1 ||
          sscanf((const char*)RaveList_get(tokens, 4), "%lf", &delta2) != 1) {
        RAVE_ERROR0("Failed to parse parameters");
        goto done;
      }
      paramfun(options, weight, X2, X3, delta1, delta2);
      PpcOptionsInternal_addTagName(tagNames, name);
      result = 1;
    }
    RaveList_freeAndDestroy(&tokens);
  }
done:
  return result;
}

static int PpcOptionsInternal_setLongFun(SimpleXmlNode_t* child, PpcRadarOptions_t* options, RaveObjectHashTable_t* tagNames, const char* name, void (*longfun)(PpcRadarOptions_t*, long))
{
  int result = 0;
  const char* value = SimpleXmlNode_getAttribute(child, "value");
  if (value != NULL) {
    long v = 0;
    if (sscanf(value, "%ld", &v) != 1) {
      RAVE_ERROR0("Failed to parse long value");
      goto done;
    }
    longfun(options, v);
    PpcOptionsInternal_addTagName(tagNames, name);
    result = 1;
  }
done:
  return result;
}

static int PpcOptionsInternal_setRequestedFields(SimpleXmlNode_t* child, PpcRadarOptions_t* options, RaveObjectHashTable_t* tagNames, const char* name)
{
  int result = 0;
  int fieldMask = 0;
  const char* value = SimpleXmlNode_getAttribute(child, "value");
  if (value != NULL) {
    RaveList_t* tokens = RaveUtilities_getTrimmedTokens(value, '|');
    if (tokens != NULL) {
      int i = 0;
      int nrTokens = RaveList_size(tokens);
      for (i = 0; i < nrTokens; i++) {
        const char* t = (const char*)RaveList_get(tokens, i);
        if (strcmp("P_TH_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_TH_CORR;
        } else if (strcmp("P_ATT_TH_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_ATT_TH_CORR;
        } else if (strcmp("P_DBZH_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_DBZH_CORR;
        } else if (strcmp("P_ATT_DBZH_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_ATT_DBZH_CORR;
        } else if (strcmp("P_KDP_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_KDP_CORR;
        } else if (strcmp("P_RHOHV_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_RHOHV_CORR;
        } else if (strcmp("P_PHIDP_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_PHIDP_CORR;
        } else if (strcmp("P_ZDR_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_ZDR_CORR;
        } else if (strcmp("P_ZPHI_CORR", t) == 0) {
          fieldMask |= PpcRadarOptions_ZPHI_CORR;
        } else if (strcmp("Q_RESIDUAL_CLUTTER_MASK", t) == 0) {
          fieldMask |= PpcRadarOptions_QUALITY_RESIDUAL_CLUTTER_MASK;
        } else if (strcmp("Q_ATTENUATION_MASK", t) == 0) {
          fieldMask |= PpcRadarOptions_QUALITY_ATTENUATION_MASK;
        } else if (strcmp("Q_ATTENUATION", t) == 0) {
          fieldMask |= PpcRadarOptions_QUALITY_ATTENUATION;
        } else {
          RAVE_INFO1("Unknown field name: %s, ignoring", t);
        }
      }
      RaveList_freeAndDestroy(&tokens);
    }
    PpcRadarOptions_setRequestedFields(options, fieldMask);
    PpcOptionsInternal_addTagName(tagNames, name);
    result = 1;
  }

  return result;
}

static int PpcOptionsInternal_setDoubleFun(SimpleXmlNode_t* child, PpcRadarOptions_t* options, RaveObjectHashTable_t* tagNames, const char* name, void (*doublefun)(PpcRadarOptions_t*, double))
{
  int result = 0;
  const char* value = SimpleXmlNode_getAttribute(child, "value");
  if (value != NULL) {
    double v = 0;
    if (sscanf(value, "%lf", &v) != 1) {
      RAVE_ERROR0("Failed to parse double value");
      goto done;
    }
    doublefun(options, v);
    PpcOptionsInternal_addTagName(tagNames, name);
    result = 1;
  }
done:
  return result;
}

//static int PpcOptionsInternal_setStringFun(SimpleXmlNode_t* child, PpcRadarOptions_t* options, RaveObjectHashTable_t* tagNames, const char* name, int (*stringfun)(PpcRadarOptions_t*, const char*))
//{
//  int result = 0;
//  const char* value = SimpleXmlNode_getAttribute(child, "value");
//  if (value != NULL) {
//    result = stringfun(options, value);
//    PpcOptionsInternal_addTagName(tagNames, name);
//  }
//done:
//  return result;
//}

static PpcRadarOptions_t* PpcOptionsInternal_createRadarOptionsFromNode(PpcOptions_t* self, SimpleXmlNode_t* node, const char* name)
{
  PpcRadarOptions_t *options = NULL;
  RaveObjectHashTable_t* tagNames = NULL;
  int nrchildren = 0, i = 0;
  options = RAVE_OBJECT_NEW(&PpcRadarOptions_TYPE);
  tagNames = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);

  if (options == NULL || tagNames == NULL) {
    RAVE_ERROR0("Failed to allocate ppc radar options or tag names");
    RAVE_OBJECT_RELEASE(options);
    RAVE_OBJECT_RELEASE(tagNames);
    return NULL;
  }
  PpcRadarOptions_setName(options, name);

  if (SimpleXmlNode_getAttribute(node, "default") != NULL) {
    if (!PpcRadarOptions_setDefaultName(options, SimpleXmlNode_getAttribute(node, "default"))) {
      RAVE_ERROR0("Failed to set default name");
    }
  }

  nrchildren = SimpleXmlNode_getNumberOfChildren(node);
  for (i = 0; i < nrchildren; i++) {
    SimpleXmlNode_t* child = SimpleXmlNode_getChild(node, i);
    if (child != NULL) {
      const char* nodeName = SimpleXmlNode_getName(child);
      if (nodeName == NULL)
        continue;
      if (strcasecmp("parametersUZ", nodeName) == 0 &&
          !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersUZ)) {
          RAVE_ERROR0("Failed to set parametersUZ in radar options");
      } else if (strcasecmp("parametersVEL", nodeName) == 0 &&
                 !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersVEL)) {
          RAVE_ERROR0("Failed to set parametersVEL in radar options");
      } else if (strcasecmp("parametersTextPHIDP", nodeName) == 0 &&
                 !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersTEXT_PHIDP)) {
          RAVE_ERROR0("Failed to set parametersTextPHIDP in radar options");
      } else if (strcasecmp("parametersRHV", nodeName) == 0 &&
                 !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersRHV)) {
          RAVE_ERROR0("Failed to set parametersRHV in radar options");
      } else if (strcasecmp("parametersTextUZ", nodeName) == 0 &&
                 !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersTEXT_UZ)) {
          RAVE_ERROR0("Failed to set parametersRHV in radar options");
      } else if (strcasecmp("parametersClutterMap", nodeName) == 0 &&
                 !PpcOptionsInternal_setParametersFun(child, options, tagNames, nodeName, PpcRadarOptions_setParametersCLUTTER_MAP)) {
          RAVE_ERROR0("Failed to set parametersClutterMap in radar options");
      } else if (strcasecmp("minWindow", nodeName) == 0 &&
                 !PpcOptionsInternal_setLongFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinWindow)) {
          RAVE_ERROR0("Failed to set minWindow in radar options");
      } else if (strcasecmp("nodata", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setNodata)) {
          RAVE_ERROR0("Failed to set nodata in radar options");
      } else if (strcasecmp("minDBZ", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinDBZ)) {
          RAVE_ERROR0("Failed to set minDBZ in radar options");
      } else if (strcasecmp("qualityThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setQualityThreshold)) {
          RAVE_ERROR0("Failed to set qualityThreshold in radar options");
      } else if (strcasecmp("preprocessZThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setPreprocessZThreshold)) {
          RAVE_ERROR0("Failed to set preprocessZThreshold in radar options");
      } else if (strcasecmp("residualMinZClutterThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualMinZClutterThreshold)) {
          RAVE_ERROR0("Failed to set residualMinZClutterThreshold in radar options");
      } else if (strcasecmp("residualThresholdZ", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualThresholdZ)) {
          RAVE_ERROR0("Failed to set residualThresholdZ in radar options");
      } else if (strcasecmp("residualThresholdTexture", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualThresholdTexture)) {
          RAVE_ERROR0("Failed to set residualThresholdTexture in radar options");
      } else if (strcasecmp("residualClutterNodata", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualClutterNodata)) {
          RAVE_ERROR0("Failed to set residualClutterNodata in radar options");
      } else if (strcasecmp("residualClutterMaskNodata", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualClutterMaskNodata)) {
          RAVE_ERROR0("Failed to set residualClutterMaskNodata in radar options");
      } else if (strcasecmp("residualClutterTextureFilteringMaxZ", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualClutterTextureFilteringMaxZ)) {
          RAVE_ERROR0("Failed to set residualClutterTextureFilteringMaxZ in radar options");
      } else if (strcasecmp("residualFilterBinSize", nodeName) == 0 &&
                 !PpcOptionsInternal_setLongFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualFilterBinSize)) {
          RAVE_ERROR0("Failed to set residualFilterBinSize in radar options");
      } else if (strcasecmp("residualFilterRaySize", nodeName) == 0 &&
                 !PpcOptionsInternal_setLongFun(child, options, tagNames, nodeName, PpcRadarOptions_setResidualFilterRaySize)) {
          RAVE_ERROR0("Failed to set residualFilterRaySize in radar options");
      } else if (strcasecmp("minZMedfilterThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinZMedfilterThreshold)) {
          RAVE_ERROR0("Failed to set minZMedfilterThreshold in radar options");
      } else if (strcasecmp("processingTextureThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setProcessingTextureThreshold)) {
          RAVE_ERROR0("Failed to set processingTextureThreshold in radar options");
      } else if (strcasecmp("pdpRWin1", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setPdpRWin1)) {
          RAVE_ERROR0("Failed to set pdpRWin1 in radar options");
      } else if (strcasecmp("pdpRWin2", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setPdpRWin2)) {
          RAVE_ERROR0("Failed to set pdpRWin2 in radar options");
      } else if (strcasecmp("pdpNrIterations", nodeName) == 0 &&
                 !PpcOptionsInternal_setLongFun(child, options, tagNames, nodeName, PpcRadarOptions_setPdpNrIterations)) {
          RAVE_ERROR0("Failed to set pdpNrIterations in radar options");
      } else if (strcasecmp("kdpUp", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setKdpUp)) {
          RAVE_ERROR0("Failed to set kdpUp in radar options");
      } else if (strcasecmp("kdpDown", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setKdpDown)) {
          RAVE_ERROR0("Failed to set kdpDown in radar options");
      } else if (strcasecmp("kdpStdThreshold", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setKdpStdThreshold)) {
          RAVE_ERROR0("Failed to set kdpStdThreshold in radar options");
      } else if (strcasecmp("BB", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setBB)) {
          RAVE_ERROR0("Failed to set BB in radar options");
      } else if (strcasecmp("thresholdPhidp", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setThresholdPhidp)) {
          RAVE_ERROR0("Failed to set thresholdPhidp in radar options");
      } else if (strcasecmp("minAttenuationMaskRHOHV", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinAttenuationMaskRHOHV)) {
          RAVE_ERROR0("Failed to set minAttenuationMaskRHOHV in radar options");
      } else if (strcasecmp("minAttenuationMaskKDP", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinAttenuationMaskKDP)) {
          RAVE_ERROR0("Failed to set minAttenuationMaskKDP in radar options");
      } else if (strcasecmp("minAttenuationMaskTH", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setMinAttenuationMaskTH)) {
          RAVE_ERROR0("Failed to set minAttenuationMaskTH in radar options");
      } else if (strcasecmp("attenuationGammaH", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setAttenuationGammaH)) {
          RAVE_ERROR0("Failed to set attenuationGammaH in radar options");
      } else if (strcasecmp("attenuationAlpha", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setAttenuationAlpha)) {
          RAVE_ERROR0("Failed to set attenuationAlpha in radar options");
      } else if (strcasecmp("attenuationPIAminZ", nodeName) == 0 &&
                 !PpcOptionsInternal_setDoubleFun(child, options, tagNames, nodeName, PpcRadarOptions_setAttenuationPIAminZ)) {
          RAVE_ERROR0("Failed to set attenuationPIAminZ in radar options");
      } else if (strcasecmp("requestedFields", nodeName) == 0 &&
                 !PpcOptionsInternal_setRequestedFields(child, options, tagNames, nodeName)) {
          RAVE_ERROR0("Failed to set requestedFields in radar options");
      }
    }

    RAVE_OBJECT_RELEASE(child);
  }

  RaveObjectHashTable_put(self->radarTagNames, name, (RaveCoreObject*)tagNames);

  RAVE_OBJECT_RELEASE(tagNames);
  return options;
}

void PpcOptionsInternal_moveParameterValues(PpcRadarOptions_t* options, PpcRadarOptions_t* other,
    void (*setterfun)(PpcRadarOptions_t*, double, double, double, double, double),
    void (*getterfun)(PpcRadarOptions_t*, double*, double*, double*, double*, double*))
{
  double weight, X2, X3, delta1, delta2;
  getterfun(other, &weight, &X2, &X3, &delta1, &delta2);
  setterfun(options, weight, X2, X3, delta1, delta2);
}

int PpcOptionsInternal_merge(PpcOptions_t* self, PpcRadarOptions_t* options, PpcRadarOptions_t* other)
{
  int result = 0;
  RaveObjectHashTable_t* optionTagNames = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((options != NULL), "options == NULL");
  RAVE_ASSERT((other != NULL), "other == NULL");
  optionTagNames = (RaveObjectHashTable_t*)RaveObjectHashTable_get(self->radarTagNames, PpcRadarOptions_getName(options));
  if (optionTagNames != NULL) {
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersUZ")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersUZ, PpcRadarOptions_getParametersUZ);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersVEL")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersVEL, PpcRadarOptions_getParametersVEL);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersTextPHIDP")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersTEXT_PHIDP, PpcRadarOptions_getParametersTEXT_PHIDP);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersRHV")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersRHV, PpcRadarOptions_getParametersRHV);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersTextUZ")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersTEXT_UZ, PpcRadarOptions_getParametersTEXT_UZ);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "parametersClutterMap")) {
      PpcOptionsInternal_moveParameterValues(options, other, PpcRadarOptions_setParametersCLUTTER_MAP, PpcRadarOptions_getParametersCLUTTER_MAP);
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minWindow")) {
      PpcRadarOptions_setMinWindow(options, PpcRadarOptions_getMinWindow(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "nodata")) {
      PpcRadarOptions_setNodata(options, PpcRadarOptions_getNodata(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minDBZ")) {
      PpcRadarOptions_setMinDBZ(options, PpcRadarOptions_getMinDBZ(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "qualityThreshold")) {
      PpcRadarOptions_setQualityThreshold(options, PpcRadarOptions_getQualityThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "preprocessZThreshold")) {
      PpcRadarOptions_setPreprocessZThreshold(options, PpcRadarOptions_getPreprocessZThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualMinZClutterThreshold")) {
      PpcRadarOptions_setResidualMinZClutterThreshold(options, PpcRadarOptions_getResidualMinZClutterThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualThresholdZ")) {
      PpcRadarOptions_setResidualThresholdZ(options, PpcRadarOptions_getResidualThresholdZ(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualThresholdTexture")) {
      PpcRadarOptions_setResidualThresholdTexture(options, PpcRadarOptions_getResidualThresholdTexture(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualClutterNodata")) {
      PpcRadarOptions_setResidualClutterNodata(options, PpcRadarOptions_getResidualClutterNodata(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualClutterMaskNodata")) {
      PpcRadarOptions_setResidualClutterMaskNodata(options, PpcRadarOptions_getResidualClutterMaskNodata(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualClutterTextureFilteringMaxZ")) {
      PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(options, PpcRadarOptions_getResidualClutterTextureFilteringMaxZ(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualFilterBinSize")) {
      PpcRadarOptions_setResidualFilterBinSize(options, PpcRadarOptions_getResidualFilterBinSize(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "residualFilterRaySize")) {
      PpcRadarOptions_setResidualFilterRaySize(options, PpcRadarOptions_getResidualFilterRaySize(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minZMedfilterThreshold")) {
      PpcRadarOptions_setMinZMedfilterThreshold(options, PpcRadarOptions_getMinZMedfilterThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "processingTextureThreshold")) {
      PpcRadarOptions_setProcessingTextureThreshold(options, PpcRadarOptions_getProcessingTextureThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "pdpRWin1")) {
      PpcRadarOptions_setPdpRWin1(options, PpcRadarOptions_getPdpRWin1(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "pdpRWin2")) {
      PpcRadarOptions_setPdpRWin2(options, PpcRadarOptions_getPdpRWin2(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "pdpNrIterations")) {
      PpcRadarOptions_setPdpNrIterations(options, PpcRadarOptions_getPdpNrIterations(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "kdpUp")) {
      PpcRadarOptions_setKdpUp(options, PpcRadarOptions_getKdpUp(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "kdpDown")) {
      PpcRadarOptions_setKdpDown(options, PpcRadarOptions_getKdpDown(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "kdpStdThreshold")) {
      PpcRadarOptions_setKdpStdThreshold(options, PpcRadarOptions_getKdpStdThreshold(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "BB")) {
      PpcRadarOptions_setBB(options, PpcRadarOptions_getBB(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "thresholdPhidp")) {
      PpcRadarOptions_setThresholdPhidp(options, PpcRadarOptions_getThresholdPhidp(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minAttenuationMaskRHOHV")) {
      PpcRadarOptions_setMinAttenuationMaskRHOHV(options, PpcRadarOptions_getMinAttenuationMaskRHOHV(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minAttenuationMaskKDP")) {
      PpcRadarOptions_setMinAttenuationMaskKDP(options, PpcRadarOptions_getMinAttenuationMaskKDP(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "minAttenuationMaskTH")) {
      PpcRadarOptions_setMinAttenuationMaskTH(options, PpcRadarOptions_getMinAttenuationMaskTH(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "attenuationGammaH")) {
      PpcRadarOptions_setAttenuationGammaH(options, PpcRadarOptions_getAttenuationGammaH(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "attenuationAlpha")) {
      PpcRadarOptions_setAttenuationAlpha(options, PpcRadarOptions_getAttenuationAlpha(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "attenuationPIAminZ")) {
      PpcRadarOptions_setAttenuationPIAminZ(options, PpcRadarOptions_getAttenuationPIAminZ(other));
    }
    if (!RaveObjectHashTable_exists(optionTagNames, "requestedFields")) {
      PpcRadarOptions_setRequestedFields(options, PpcRadarOptions_getRequestedFields(other));
    }
  }
  RAVE_OBJECT_RELEASE(optionTagNames);
  return result;
}


/*@} End of Private functions */

/*@{ Interface functions */
PpcOptions_t* PpcOptions_load(const char* filename)
{
  SimpleXmlNode_t* node = NULL;
  PpcOptions_t *result=NULL, *options=NULL;

  int nrchildren = 0;
  int i = 0;
  RAVE_ASSERT((filename != NULL), "filename == NULL");

  node = SimpleXmlNode_parseFile(filename);
  if (node == NULL) {
    goto done;
  }

  options = RAVE_OBJECT_NEW(&PpcOptions_TYPE);
  if (options == NULL) {
    RAVE_ERROR0("Failed to create options");
    goto done;
  }

  nrchildren = SimpleXmlNode_getNumberOfChildren(node);
  for (i = 0; i < nrchildren; i++) {
    SimpleXmlNode_t* child = SimpleXmlNode_getChild(node, i);
    const char* nodeName = SimpleXmlNode_getName(child);
    if (nodeName != NULL && strcasecmp("radaroptions", nodeName)==0) {
      const char* attr = SimpleXmlNode_getAttribute(child, "name");
      if (attr != NULL) {
        PpcRadarOptions_t* radaroptions = PpcOptionsInternal_createRadarOptionsFromNode(options, child, attr);
        if (radaroptions != NULL && PpcRadarOptions_getDefaultName(radaroptions) != NULL) {
          PpcRadarOptions_t* o2 = PpcOptions_getRadarOptions(options, PpcRadarOptions_getDefaultName(radaroptions));
          if (o2 == NULL) {
            RAVE_ERROR3("Referring to section '%s' from '%s' but '%s' does not exist", PpcRadarOptions_getDefaultName(radaroptions), attr, PpcRadarOptions_getDefaultName(radaroptions));
          } else {
            PpcOptionsInternal_merge(options, radaroptions, o2);
          }
          RAVE_OBJECT_RELEASE(o2);
        }

        if (radaroptions == NULL || !PpcOptions_addRadarOptions(options, radaroptions)) {
          RAVE_ERROR0("Failed to create radar options");
          RAVE_OBJECT_RELEASE(radaroptions);
          goto done;
        }

        RAVE_OBJECT_RELEASE(radaroptions);
      } else {
        RAVE_ERROR0("No name defined for radaroption");
        goto done;
      }
    }
    RAVE_OBJECT_RELEASE(child);
  }

  result = RAVE_OBJECT_COPY(options);

done:
  RAVE_OBJECT_RELEASE(options);
  RAVE_OBJECT_RELEASE(node);
  return result;
}

PpcRadarOptions_t* PpcOptions_getRadarOptions(PpcOptions_t* self, const char* name)
{
  PpcRadarOptions_t* result = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (RaveObjectHashTable_exists(self->radaroptions, name)) {
    result = (PpcRadarOptions_t*)RaveObjectHashTable_get(self->radaroptions, name);
  }
  return result;
}

int PpcOptions_addRadarOptions(PpcOptions_t* self, PpcRadarOptions_t* options)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (options != NULL) {
    if (PpcRadarOptions_getName(options) != NULL) {
      result = RaveObjectHashTable_put(self->radaroptions, PpcRadarOptions_getName(options), (RaveCoreObject*)options);
    }
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType PpcOptions_TYPE = {
    "PpcOptions",
    sizeof(PpcOptions_t),
    PpcOptions_constructor,
    PpcOptions_destructor,
    PpcOptions_copyconstructor
};

