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
 * @date 2019-02-16
 */
#ifndef PDP_PROCESSOR_H
#define PDP_PROCESSOR_H
#include "polarvolume.h"
#include "rave_data2d.h"
/**
 * Defines a transformer
 */
typedef struct _PdpProcessor_t PdpProcessor_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PdpProcessor_TYPE;

/**
 * Undef value used in the trap function.
 */
#define TRAP_UNDEF_VALUE -99999999999.0

/**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected TH to result */
#define PdpProcessor_CORR_TH        (1)       /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected TH to result */
#define PdpProcessor_CORR_ATT_TH    (1 << 1)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected and attenuated TH to result */
#define PdpProcessor_CORR_KDP       (1 << 2)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected KDP to result */
#define PdpProcessor_CORR_RHOHV     (1 << 3)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected RHOHV to result */
#define PdpProcessor_CORR_PHIDP     (1 << 4)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected PHIDP to result */
#define PdpProcessor_CORR_ZDR       (1 << 5)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected ZDR to result */
#define PdpProcessor_CORR_ZPHI      (1 << 6)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected ZPHI to result */
#define PdpProcessor_QUALITY_RESIDUAL_CLUTTER_MASK (1 << 7) /**< Used by @ref #PdpProcessor_setRequestedFields Add quality flag for residual clutter flag */
#define PdpProcessor_QUALITY_ATTENUATION_MASK (1 << 8) /**< Used by @ref #PdpProcessor_setRequestedFields Add quality flag for attenuation mask */


/**
 * Combines all functions implemented in this class into one process that performs the actual polar data processing chain
 * according to the matlab prototype developed by Gianfranco Vulpiani.
 * @param[in] self - self
 * @param[in] scan - the polar scan
 * @returns new scan on success otherwise NULL
 */
PolarScan_t* PdpProcessor_process(PdpProcessor_t* self, PolarScan_t* scan);

/**
 * Sets the field mask which should be bitwised ored from the PdpProcessor field defines
 * For example: PdpProcessor_CORR_TH | PdpProcessor_CORR_KDP
 * @param[in] self - self
 * @param[in] fieldmask - the bit mask
 */
void PdpProcessor_setRequestedFields(PdpProcessor_t* self, int fieldmask);

/**
 * @param[in] self - self
 * @returns the field mask
 */
int PdpProcessor_getRequestedFields(PdpProcessor_t* self);

/**
 * Sets the upper threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the upper threshold
 */
void PdpProcessor_setKdpUp(PdpProcessor_t* self, double v);

/**
 * @returns the kdp upper threshold
 * @param[in] self - self
 */
double PdpProcessor_getKdpUp(PdpProcessor_t* self);

/**
 * Sets the lower threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the lower threshold
 */
void PdpProcessor_setKdpDown(PdpProcessor_t* self, double v);

/**
 * @returns the kdp lower threshold
 * @param[in] self - self
 */
double PdpProcessor_getKdpDown(PdpProcessor_t* self);

/**
 * Sets the kdp standard deviation threshold for the generated kdp field
 * @param[in] self - self
 * @param[in] v - the threshold
 */
void PdpProcessor_setKdpStdThreshold(PdpProcessor_t* self, double v);

/**
 * @returns the kdp standard deviation threshold
 * @param[in] self - self
 */
double PdpProcessor_getKdpStdThreshold(PdpProcessor_t* self);

/**
 * Sets the BB value used in the zphi part of the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setBB(PdpProcessor_t* self, double v);

/**
 * @returns the BB value used in the zphi part of the pdp processing
 * @param[in] self - self
 */
double PdpProcessor_getBB(PdpProcessor_t* self);

/**
 * Sets the threshold for PHIDP in the pdp processing
 * @param[in] self - self
 * @param[in] v - the threshold
 */
void PdpProcessor_setThresholdPhidp(PdpProcessor_t* self, double v);

/**
 * @returns the threshold for PHIDP in the pdp processing
 * @param[in] self - self
 */
double PdpProcessor_getThresholdPhidp(PdpProcessor_t* self);

/**
 * Sets the min window size during pdp processing
 * @param[in] self - self
 * @param[in] window - the window
 */
void PdpProcessor_setMinWindow(PdpProcessor_t* self, long window);

/**
 * @returns the min window size for the pdp processing
 * @param[in] self - self
 */
long PdpProcessor_getMinWindow(PdpProcessor_t* self);

/**
 * Sets the pdp ray window 1 used in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setPdpRWin1(PdpProcessor_t* self, double v);

/**
 * @returns the pdp ray window 1 used in the pdp processing
 * @param[in] self - self
 */
double PdpProcessor_getPdpRWin1(PdpProcessor_t* self);

/**
 * Sets the pdp ray window 2 used in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setPdpRWin2(PdpProcessor_t* self, double v);

/**
 * @returns the pdp ray window 2 used in the pdp processing
 * @param[in] self - self
 */
double PdpProcessor_getPdpRWin2(PdpProcessor_t* self);

/**
 * Sets the number of iterations in pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setPdpNrIterations(PdpProcessor_t* self, long v);

/**
 * @returns the number of iterations in pdp processing
 * @param[in] self - self
 */
long PdpProcessor_getPdpNrIterations(PdpProcessor_t* self);

/**
 * Helper function that sets kdpUp, kdpDown and kdpStdThreshold to predfined values.
 * band = 's' => kdpUp = 14, kdpDown=-2, kdpStdThreshold=5
 * band = 'c' => kdpUp = 20, kdpDown=-2, kdpStdThreshold=5
 * band = 'x' => kdpUp = 40, kdpDown=-2, kdpStdThreshold=5
 * @param[in] self - self
 * @param[in] band - the band, see above
 * @returns 1 if band is either 's', 'c' or 'x' else 0 and kdp-values will not be changed.
 */
int PdpProcessor_setBand(PdpProcessor_t* self, char band);

/**
 * Gets the parameters used for the UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersUZ(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersUZ(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the VEL parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersVEL(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the VEL parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersVEL(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the TEXT_PHIDP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersTEXT_PHIDP(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the TEXT_PHIDP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersTEXT_PHIDP(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the RHV parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersRHV(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the RHV parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersRHV(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the TEXT_UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersTEXT_UZ(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the TEXT_UZ parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersTEXT_UZ(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Gets the parameters used for the CLUTTER_MAP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[out] weight - the weight
 * @param[out] X2 - the X2 value
 * @param[out] X3 - the X3 value
 * @param[out] delta1 - delta 1
 * @param[out] delta2 - delta 2
 */
void PdpProcessor_getParametersCLUTTER_MAP(PdpProcessor_t* self, double* weight, double* X2, double* X3, double* delta1, double* delta2);

/**
 * Sets the parameters used for the CLUTTER_MAP parameter when performing the clutter ID.
 * @param[in] self - self
 * @param[in] weight - the weight
 * @param[in] X2 - the X2 value
 * @param[in] X3 - the X3 value
 * @param[in] delta1 - delta 1
 * @param[in] delta2 - delta 2
 */
void PdpProcessor_setParametersCLUTTER_MAP(PdpProcessor_t* self, double weight, double X2, double X3, double delta1, double delta2);

/**
 * Returns the bottom melting layer height for the specified scan
 */
double PdpProcessor_getMeltingLayerBottomHeight(PolarScan_t* scan);

/**
 * Sets the nodata value to be used in most sub-products
 * @param[in] self - self
 * @param[in] nodata - the nodata value
 */
void PdpProcessor_setNodata(PdpProcessor_t* self, double nodata);

/**
 * @returns the nodata value used in most products
 * @param[in] self - self
 */
double PdpProcessor_getNodata(PdpProcessor_t* self);

/**
 * Sets the min DBZ threshold to be used in the clutter correction.
 * @param[in] self - self
 * @param[in] minv - the min dbz threshold
 */
void PdpProcessor_setMinDBZ(PdpProcessor_t* self, double minv);

/**
 * @returns the min DBZ threshold that is used in the clutter correction
 * @param[in] self - self
 */
double PdpProcessor_getMinDBZ(PdpProcessor_t* self);

/**
 * The quality threshold that should be used in the clutter correction
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PdpProcessor_setQualityThreshold(PdpProcessor_t* self, double minv);

/**
 * @returns the quality threshold that is used in the clutter correction.
 * @param[in] self - self
 */
double PdpProcessor_getQualityThreshold(PdpProcessor_t* self);

/**
 * The residual min z clutter threshold that should be used in the residual clutter filter process
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PdpProcessor_setResidualMinZClutterThreshold(PdpProcessor_t* self, double minv);

/**
 * @returns the residual min z clutter threshold that should be used in the residual clutter filter process.
 * @param[in] self - self
 */
double PdpProcessor_getResidualMinZClutterThreshold(PdpProcessor_t* self);

/**
 * The residual min Z threshold in the residual clutter filtering
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PdpProcessor_setResidualThresholdZ(PdpProcessor_t* self, double minv);

/**
 * @returns the min Z threshold in the residual clutter filtering
 * @param[in] self - self
 */
double PdpProcessor_getResidualThresholdZ(PdpProcessor_t* self);

/**
 * The texture threshold in the residual clutter filtering
 * @param[in] self - self
 * @param[in] minv - the threshold
 */
void PdpProcessor_setResidualThresholdTexture(PdpProcessor_t* self, double minv);

/**
 * @returns the texture threshold in the residual clutter filtering
 * @param[in] self - self
 */
double PdpProcessor_getResidualThresholdTexture(PdpProcessor_t* self);

/**
 * The nodata value to be used when creating the residual clutter image used for creating the mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setResidualClutterNodata(PdpProcessor_t* self, double v);

/**
 * @returns the nodata value to be used when creating the residual clutter image used for creating the mask
 * @param[in] self - self
 */
double PdpProcessor_getResidualClutterNodata(PdpProcessor_t* self);

/**
 * The nodata value for the residual clutter mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setResidualClutterMaskNodata(PdpProcessor_t* self, double v);

/**
 * @returns the nodata value for the residual clutter mask
 * @param[in] self - self
 */
double PdpProcessor_getResidualClutterMaskNodata(PdpProcessor_t* self);

/**
 * The max Z value when creating the residual clutter mask, anything higher will be set to min value
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setResidualClutterTextureFilteringMaxZ(PdpProcessor_t* self, double v);

/**
 * @returns the max Z value when creating the residual clutter mask, anything higher will be set to min value
 * @param[in] self - self
 */
double PdpProcessor_getResidualClutterTextureFilteringMaxZ(PdpProcessor_t* self);

/**
 * The number of bins used in the window when creating the residual mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setResidualFilterBinSize(PdpProcessor_t* self, long v);

/**
 * @returns the number of bins used in the window when creating the residual mask
 * @param[in] self - self
 */
long PdpProcessor_getResidualFilterBinSize(PdpProcessor_t* self);

/**
 * The number of rays used in the window when creating the residual mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setResidualFilterRaySize(PdpProcessor_t* self, long v);

/**
 * @returns the number of rays used in the window when creating the residual mask
 * @param[in] self - self
 */
long PdpProcessor_getResidualFilterRaySize(PdpProcessor_t* self);

/**
 * The min z threshold used in the median filter that is used by the residual clutter filter
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setMinZMedfilterThreshold(PdpProcessor_t* self, double v);

/**
 * @returns the min z threshold used in the median filter that is used by the residual clutter filter
 * @param[in] self - self
 */
double PdpProcessor_getMinZMedfilterThreshold(PdpProcessor_t* self);

/**
 * The threshold for the texture created in the pdp processing
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setProcessingTextureThreshold(PdpProcessor_t* self, double v);

/**
 * @returns the threshold for the texture created in the pdp processing
 * @param[in] self - self
 */
double PdpProcessor_getProcessingTextureThreshold(PdpProcessor_t* self);

/**
 * The min RHOHV value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setMinAttenuationMaskRHOHV(PdpProcessor_t* self, double v);

/**
 * @returns the min RHOHV value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PdpProcessor_getMinAttenuationMaskRHOHV(PdpProcessor_t* self);
/**
 * The min KDP value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setMinAttenuationMaskKDP(PdpProcessor_t* self, double v);

/**
 * @returns the min KDP value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PdpProcessor_getMinAttenuationMaskKDP(PdpProcessor_t* self);

/**
 * The min TH value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setMinAttenuationMaskTH(PdpProcessor_t* self, double v);

/**
 * @returns the min TH value for marking value as 1 in the attenuation mask
 * @param[in] self - self
 */
double PdpProcessor_getMinAttenuationMaskTH(PdpProcessor_t* self);

/**
 * The gamma h value used in the attenuation
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setAttenuationGammaH(PdpProcessor_t* self, double v);

/**
 * @returns the gamma h value used in the attenuation
 * @param[in] self - self
 */
double PdpProcessor_getAttenuationGammaH(PdpProcessor_t* self);

/**
 * The alpha value used in the attenuation
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setAttenuationAlpha(PdpProcessor_t* self, double v);

/**
 * @returns the alpha value used in the attenuation
 * @param[in] self - self
 */
double PdpProcessor_getAttenuationAlpha(PdpProcessor_t* self);

/**
 * The min PIA Z value in attenuation process
 * @param[in] self - self
 * @param[in] v - the value
 */
void PdpProcessor_setAttenuationPIAminZ(PdpProcessor_t* self, double v);

/**
 * @returns the min PIA Z value in attenuation process
 * @param[in] self - self
 */
double PdpProcessor_getAttenuationPIAminZ(PdpProcessor_t* self);

/**
 * Calculates the texture from the data 2d field. Note, X must have nodata and useNodata set.
 * @param[in] self - self
 * @param[in] X - data 2D field
 * @returns a new data 2D field
 */
RaveData2D_t* PdpProcessor_texture(PdpProcessor_t* self, RaveData2D_t* X);

/**
 * Trapezoidal function.
 * @param[in] self - self
 * @param[in] x - matrix
 * @param[in] a - a
 * @param[in] b - b
 * @param[in] s - s
 * @param[in] t - t
 */
RaveData2D_t* PdpProcessor_trap(PdpProcessor_t* self, RaveData2D_t* x, double a, double b, double s, double t);

/**
 * Clutter identificaton function
 * @param[in] self - self
 * @param[in] Z - the Z field
 * @param[in] VRADH - the VRADH field
 * @param[in] texturePHIDP - the PHIDP texture
 * @param[in] RHOHV - the RHOHV field
 * @param[in] textureZ - the Z texture
 * @param[in] clutterMap - the clutter map
 * @param[in] nodataZ - Z nodata value
 * @param[in] nodataVRADH - the VRADH nodata value
 * @returs the identified clutter field
 */
RaveData2D_t* PdpProcessor_clutterID(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap, double nodataZ, double nodataVRADH);

/**
 * Performs the clutter correction
 * @param[in] self - self
 * @param[in] Z - the Z field
 * @param[in] VRADH - the VRADH field
 * @param[in] texturePHIDP - the PHIDP texture
 * @param[in] RHOHV - the RHOHV field
 * @param[in] textureZ - the Z texture
 * @param[in] clutterMap - the clutter map
 * @param[in] nodataZ - Z nodata value
 * @param[in] qualityThreshold - quality threshold value
 * @param[out] outZ - the resulting Z field
 * @param[out] outQuality - the resulting quality field
 * @param[out] outClutterMask - the resulting clutter mask
 * @returns 1 on success or 0 on failure
 */
int PdpProcessor_clutterCorrection(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap,
    double nodataZ, double nodataVRADH, double qualityThreshold,
    RaveData2D_t** outZ, RaveData2D_t** outQuality, RaveData2D_t** outClutterMask);

/**
 * Creates a median filtered field
 * @param[in] self - self
 * @param[in] Z - img to filter
 * @param[in] thresh - min Z threshold to know if median filtering should be performed
 * @param[in] filtXsize - window x size
 * @param[in] filtYsize - window y size
 * @returns the filtered img
 */
RaveData2D_t* PdpProcessor_medfilt(PdpProcessor_t* self, RaveData2D_t* Z, double thresh, double nodataZ, long filtXsize, long filtYsize);

/**
 * Runs the residual clutter filter on the image
 * @param[in] self - self
 * @param[in] Z - the Z field
 * @param[in] thresholdZ - the Z threshold
 * @param[in] thresholdTexture - the threshold value on the created texture
 * @param[in] filtXsize - the window size bin-wise
 * @param[in] filtYsize - the window size ray-wise
 * @returns the clutter mask
 */
RaveData2D_t* PdpProcessor_residualClutterFilter(PdpProcessor_t* self, RaveData2D_t* Z,
    double thresholdZ, double thresholdTexture, long filtXsize, long filtYsize);

int PdpProcessor_pdpProcessing(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, long window, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp);

int PdpProcessor_pdpScript(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, double rWin1, double rWin2, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp);

int PdpProcessor_attenuation(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* zdr, RaveData2D_t* pdp,
    RaveData2D_t* mask, double gamma_h, double alpha, RaveData2D_t** outz, RaveData2D_t** outzdr, RaveData2D_t** outPIA);

int PdpProcessor_zphi(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* pdp, RaveData2D_t* mask,
    double dr, double BB, double gamma_h, RaveData2D_t** outzphi, RaveData2D_t** outAH);
#endif
