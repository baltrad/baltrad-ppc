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
#ifndef PPC_RADAR_OPTIONS_H
#define PPC_RADAR_OPTIONS_H

#include "polarvolume.h"
#include "rave_data2d.h"
/**
 * Defines a transformer
 */
typedef struct _PpcRadarOptions_t PpcRadarOptions_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PpcRadarOptions_TYPE;


#define PpcRadarOptions_TH_CORR        (1)       /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected TH to result */
#define PpcRadarOptions_ATT_TH_CORR    (1 << 1)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected and attenuated TH to result */
#define PpcRadarOptions_DBZH_CORR      (1 << 2)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected DBZH to result */
#define PpcRadarOptions_ATT_DBZH_CORR  (1 << 3)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected and attenuated DBZH to result */
#define PpcRadarOptions_KDP_CORR       (1 << 4)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected KDP to result */
#define PpcRadarOptions_RHOHV_CORR     (1 << 5)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected RHOHV to result */
#define PpcRadarOptions_PHIDP_CORR     (1 << 6)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected PHIDP to result */
#define PpcRadarOptions_ZDR_CORR       (1 << 7)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected ZDR to result */
#define PpcRadarOptions_ZPHI_CORR      (1 << 8)  /**< Used by @ref #PpcRadarOptions_setRequestedFields Add Corrected ZPHI to result */
#define PpcRadarOptions_QUALITY_RESIDUAL_CLUTTER_MASK (1 << 9) /**< Used by @ref #PpcRadarOptions_setRequestedFields Add quality flag for residual clutter flag */
#define PpcRadarOptions_QUALITY_ATTENUATION_MASK (1 << 10) /**< Used by @ref #PpcRadarOptions_setRequestedFields Add quality flag for attenuation mask */
#define PpcRadarOptions_QUALITY_ATTENUATION (1 << 11) /**< Used by @ref #PpcRadarOptions_setRequestedFields Add quality flag for actual attenuation */

/**
 * Set the name of the owner of these options
 * @param[in] self - self
 * @param[in] name - the name of the owner
 * @return 1 on success otherwise 0
 */
int PpcRadarOptions_setName(PpcRadarOptions_t* self, const char* name);

/**
 * @returns the name of the owner of these options
 * @param[in] self - self
 */
const char* PpcRadarOptions_getName(PpcRadarOptions_t* self);

/**
 * Sets the name of the default setting if there is one.
 * @param[in] self - self
 * @param[in] name - the name of the default setting
 * @return 1 on success otherwise 0
 */
int PpcRadarOptions_setDefaultName(PpcRadarOptions_t* self, const char* name);

/**
 * @returns the name of the default setting if there is any
 * @param[in] self - self
 */
const char* PpcRadarOptions_getDefaultName(PpcRadarOptions_t* self);

/**
 * Sets the field mask which should be bitwised ored from the PdpProcessor field defines
 * For example: PpcRadarOptions_CORR_TH | PpcRadarOptions_CORR_KDP
 * @param[in] self - self
 * @param[in] fieldmask - the bit mask
 */
void PpcRadarOptions_setRequestedFields(PpcRadarOptions_t* self, int fieldmask);

/**
 * @param[in] self - self
 * @returns the field mask
 */
int PpcRadarOptions_getRequestedFields(PpcRadarOptions_t* self);

/**
 * Helper function that sets kdpUp, kdpDown and kdpStdThreshold to predfined values.
 * band = 's' => kdpUp = 14, kdpDown=-2, kdpStdThreshold=5
 * band = 'c' => kdpUp = 20, kdpDown=-2, kdpStdThreshold=5
 * band = 'x' => kdpUp = 40, kdpDown=-2, kdpStdThreshold=5
 * @param[in] self - self
 * @param[in] band - the band, see above
 * @returns 1 if band is either 's', 'c' or 'x' else 0 and kdp-values will not be changed.
 */
int PpcRadarOptions_setBand(PpcRadarOptions_t* self, char band);

/**
 * Sets the upper threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the upper threshold
 */
void PpcRadarOptions_setKdpUp(PpcRadarOptions_t* self, double v);

/**
 * @returns the kdp upper threshold
 * @param[in] self - self
 */
double PpcRadarOptions_getKdpUp(PpcRadarOptions_t* self);

/**
 * Sets the lower threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the lower threshold
 */
void PpcRadarOptions_setKdpDown(PpcRadarOptions_t* self, double v);

/**
 * @returns the kdp lower threshold
 * @param[in] self - self
 */
double PpcRadarOptions_getKdpDown(PpcRadarOptions_t* self);

/**
 * Sets the kdp standard deviation threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the threshold
 */
void PpcRadarOptions_setKdpStdThreshold(PpcRadarOptions_t* self, double v);

/**
 * @returns the kdp standard deviation threshold
 * @param[in] self - self
 */
double PpcRadarOptions_getKdpStdThreshold(PpcRadarOptions_t* self);

/**
 * Sets the BB value used in the zphi part of the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setBB(PpcRadarOptions_t* self, double v);

/**
 * @returns the BB value used in the zphi part of the pdp processing
 * @param[in] self - selfPpcRadarOptions_TH_CORR_ATT
 */
double PpcRadarOptions_getBB(PpcRadarOptions_t* self);

/**
 * Sets the threshold for PHIDP in the pdp processing
 * @param[in] self - self
 * @param[in] v - the threshold
 */
void PpcRadarOptions_setThresholdPhidp(PpcRadarOptions_t* self, double v);

/**
 * @returns the threshold for PHIDP in the pdp processing
 * @param[in] self - self
 */
double PpcRadarOptions_getThresholdPhidp(PpcRadarOptions_t* self);

/**
 * Sets the min window size during pdp processing
 * @param[in] self - self
 * @param[in] window - the window
 */
void PpcRadarOptions_setMinWindow(PpcRadarOptions_t* self, long window);

/**
 * @returns the min window size for the pdp processing
 * @param[in] self - self
 */
long PpcRadarOptions_getMinWindow(PpcRadarOptions_t* self);

/**
 * Sets the pdp ray window 1 used in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setPdpRWin1(PpcRadarOptions_t* self, double v);

/**
 * @returns the pdp ray window 1 used in the pdp processing
 * @param[in] self - self
 */
double PpcRadarOptions_getPdpRWin1(PpcRadarOptions_t* self);

/**
 * Sets the pdp ray window 2 used in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setPdpRWin2(PpcRadarOptions_t* self, double v);

/**
 * @returns the pdp ray window 2 used in the pdp processing
 * @param[in] self - self
 */
double PpcRadarOptions_getPdpRWin2(PpcRadarOptions_t* self);

/**
 * Sets the number of iterations in pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setPdpNrIterations(PpcRadarOptions_t* self, long v);

/**
 * @returns the number of iterations in pdp processing
 * @param[in] self - self
 */
long PpcRadarOptions_getPdpNrIterations(PpcRadarOptions_t* self);

/**
 * Helper function that sets kdpUp, kdpDown and kdpStdThreshold to predfined values.
 * band = 's' => kdpUp = 14, kdpDown=-2, kdpStdThreshold=5
 * band = 'c' => kdpUp = 20, kdpDown=-2, kdpStdThreshold=5
 * band = 'x' => kdpUp = 40, kdpDown=-2, kdpStdThreshold=5
 * @param[in] self - self
 * @param[in] band - the band, see above
 * @returns 1 if band is either 's', 'c' or 'x' else 0 and kdp-values will not be changed.
 */
int PpcRadarOptions_setBand(PpcRadarOptions_t* self, char band);

/**
 * Gets the parameters used for the UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersUZ(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersUZ(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the VEL parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersVEL(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the VEL parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersVEL(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the TEXT_PHIDP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersTEXT_PHIDP(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the TEXT_PHIDP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersTEXT_PHIDP(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the RHV parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersRHV(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the RHV parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersRHV(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the TEXT_UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersTEXT_UZ(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the TEXT_UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersTEXT_UZ(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the CLUTTER_MAP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PpcRadarOptions_getParametersCLUTTER_MAP(PpcRadarOptions_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the CLUTTER_MAP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PpcRadarOptions_setParametersCLUTTER_MAP(PpcRadarOptions_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Sets the melting layer bottom height.
 * @param[in] self - self
 * @param[in] height - the height in km
 */
void PpcRadarOptions_setMeltingLayerBottomHeight(PpcRadarOptions_t* self, double height);

/**
 * @returns the bottom melting layer height for the specified scan
 * @param[in] self - self
 */
double PpcRadarOptions_getMeltingLayerBottomHeight(PpcRadarOptions_t* self);

/**
 * Sets the nodata value to be used in most sub-products
 * @param[in] self - self
 * @param[in] nodata - the nodata value
 */
void PpcRadarOptions_setNodata(PpcRadarOptions_t* self, double nodata);

/**
 * @returns the nodata value used in most products
 * @param[in] self - self
 */
double PpcRadarOptions_getNodata(PpcRadarOptions_t* self);

/**
 * Sets the min DBZ threshold to be used in the clutter correction.
 * @param[in] self - self
 * @param[in] minv - the min dbz threshold
 */
void PpcRadarOptions_setMinDBZ(PpcRadarOptions_t* self, double minv);

/**
 * @returns the min DBZ threshold that is used in the clutter correction
 * @param[in] self - self
 */
double PpcRadarOptions_getMinDBZ(PpcRadarOptions_t* self);

/**
 * The quality threshold that should be used in the clutter correction
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PpcRadarOptions_setQualityThreshold(PpcRadarOptions_t* self, double minv);

/**
 * @returns the quality threshold that is used in the clutter correction.
 * @param[in] self - self
 */
double PpcRadarOptions_getQualityThreshold(PpcRadarOptions_t* self);

/**
 * A preprocessing min threshold before the actual processing begins. Defaults at -20.0
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PpcRadarOptions_setPreprocessZThreshold(PpcRadarOptions_t* self, double minv);

/**
 * @returns the preprocessing min threshold before the actual processing begins. Defaults at -20.0
 * @param[in] self - self
 */
double PpcRadarOptions_getPreprocessZThreshold(PpcRadarOptions_t* self);

/**
 * The residual min z clutter threshold that should be used in the residual clutter filter process
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PpcRadarOptions_setResidualMinZClutterThreshold(PpcRadarOptions_t* self, double minv);

/**
 * @returns the residual min z clutter threshold that should be used in the residual clutter filter process.
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualMinZClutterThreshold(PpcRadarOptions_t* self);

/**
 * The residual min Z threshold in the residual clutter filtering
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PpcRadarOptions_setResidualThresholdZ(PpcRadarOptions_t* self, double minv);

/**
 * @returns the min Z threshold in the residual clutter filtering
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualThresholdZ(PpcRadarOptions_t* self);

/**
 * The texture threshold in the residual clutter filtering
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PpcRadarOptions_setResidualThresholdTexture(PpcRadarOptions_t* self, double minv);

/**
 * @returns the texture threshold in the residual clutter filtering
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualThresholdTexture(PpcRadarOptions_t* self);

/**
 * The nodata value to be used when creating the residual clutter image used for creating the mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setResidualClutterNodata(PpcRadarOptions_t* self, double v);

/**
 * @returns the nodata value to be used when creating the residual clutter image used for creating the mask
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualClutterNodata(PpcRadarOptions_t* self);

/**
 * The nodata value for the residual clutter mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setResidualClutterMaskNodata(PpcRadarOptions_t* self, double v);

/**
 * @returns the nodata value for the residual clutter mask
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualClutterMaskNodata(PpcRadarOptions_t* self);

/**
 * The max Z value when creating the residual clutter mask, anything higher will be set to min value
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setResidualClutterTextureFilteringMaxZ(PpcRadarOptions_t* self, double v);

/**
 * @returns the max Z value when creating the residual clutter mask, anything higher will be set to min value
 * @param[in] self - self
 */
double PpcRadarOptions_getResidualClutterTextureFilteringMaxZ(PpcRadarOptions_t* self);

/**
 * The number of bins used in the window when creating the residual mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setResidualFilterBinSize(PpcRadarOptions_t* self, long v);

/**
 * @returns the number of bins used in the window when creating the residual mask
 * @param[in] self - self
 */
long PpcRadarOptions_getResidualFilterBinSize(PpcRadarOptions_t* self);

/**
 * The number of rays used in the window when creating the residual mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setResidualFilterRaySize(PpcRadarOptions_t* self, long v);

/**
 * @returns the number of rays used in the window when creating the residual mask
 * @param[in] self - self
 */
long PpcRadarOptions_getResidualFilterRaySize(PpcRadarOptions_t* self);

/**
 * The min z threshold used in the median filter that is used by the residual clutter filter
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setMinZMedfilterThreshold(PpcRadarOptions_t* self, double v);

/**
 * @returns the min z threshold used in the median filter that is used by the residual clutter filter
 * @param[in] self - self
 */
double PpcRadarOptions_getMinZMedfilterThreshold(PpcRadarOptions_t* self);

/**
 * The threshold for the texture created in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setProcessingTextureThreshold(PpcRadarOptions_t* self, double v);

/**
 * @returns the threshold for the texture created in the pdp processing
 * @param[in] self - self
 */
double PpcRadarOptions_getProcessingTextureThreshold(PpcRadarOptions_t* self);

/**
 * The min RHOHV value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setMinAttenuationMaskRHOHV(PpcRadarOptions_t* self, double v);

/**
 * @returns the min RHOHV value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PpcRadarOptions_getMinAttenuationMaskRHOHV(PpcRadarOptions_t* self);
/**
 * The min KDP value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setMinAttenuationMaskKDP(PpcRadarOptions_t* self, double v);

/**
 * @returns the min KDP value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PpcRadarOptions_getMinAttenuationMaskKDP(PpcRadarOptions_t* self);

/**
 * The min TH value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setMinAttenuationMaskTH(PpcRadarOptions_t* self, double v);

/**
 * @returns the min TH value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PpcRadarOptions_getMinAttenuationMaskTH(PpcRadarOptions_t* self);

/**
 * The gamma h value used in the attenuation
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setAttenuationGammaH(PpcRadarOptions_t* self, double v);

/**
 * @returns the gamma h value used in the attenuation
 * @param[in] self - self
 */
double PpcRadarOptions_getAttenuationGammaH(PpcRadarOptions_t* self);

/**
 * The alpha value used in the attenuation
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setAttenuationAlpha(PpcRadarOptions_t* self, double v);

/**
 * @returns the alpha value used in the attenuation
 * @param[in] self - self
 */
double PpcRadarOptions_getAttenuationAlpha(PpcRadarOptions_t* self);

/**
 * The min PIA Z value in attenuation process
 * @param[in] self - self
 * @param[in] v - the value
 */
void PpcRadarOptions_setAttenuationPIAminZ(PpcRadarOptions_t* self, double v);

/**
 * @returns the min PIA Z value in attenuation process
 * @param[in] self - self
 */
double PpcRadarOptions_getAttenuationPIAminZ(PpcRadarOptions_t* self);

#endif /* PPC_RADAR_OPTIONS_H_ */
