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

#define TRAP_UNDEF_VALUE -99999999999.0

#define PdpProcessor_CORR_TH        (1)
#define PdpProcessor_CORR_ATT_TH    (1 << 1)
#define PdpProcessor_CORR_KDP       (1 << 2)
#define PdpProcessor_CORR_RHOHV     (1 << 3)
#define PdpProcessor_CORR_PHIDP     (1 << 4)
#define PdpProcessor_CORR_ZDR       (1 << 5)
#define PdpProcessor_CORR_ZPHI      (1 << 6)
#define PdpProcessor_QUALITY_RESIDUAL_CLUTTER_MASK (1 << 7)
#define PdpProcessor_QUALITY_ATTENUATION_MASK (1 << 8)

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
 * Combines all functions implemented in this class into one process that performs the actual polar data processing chain
 * according to the matlab prototype developed by Gianfranco Vulpiani.
 * @param[in] self - self
 * @param[in] scan - the polar scan
 * @returns new scan on success otherwise NULL
 */
PolarScan_t* PdpProcessor_process(PdpProcessor_t* self, PolarScan_t* scan);

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
 * Calculates the texture from the data 2d field. Note, X must have nodata and useNodata set.
 * @param[in] self - self
 * @param[in] X - data 2D field
 * @returns a new data 2D field
 */
RaveData2D_t* PdpProcessor_texture(PdpProcessor_t* self, RaveData2D_t* X);

RaveData2D_t* PdpProcessor_trap(PdpProcessor_t* self, RaveData2D_t* x, double a, double b, double c, double d);

RaveData2D_t* PdpProcessor_clutterID(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap, double nodataZ, double nodataVRADH);

int PdpProcessor_clutterCorrection(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* VRADH,
    RaveData2D_t* texturePHIDP, RaveData2D_t* RHOHV, RaveData2D_t* textureZ, RaveData2D_t* clutterMap,
    double nodataZ, double nodataVRADH, double qualityThreshold,
    RaveData2D_t** outZ, RaveData2D_t** outQuality, RaveData2D_t** outClutterMask);

/**
 *
 * @param[in] self - self
 * @param[in] Z - img to filter
 * @param[in] thresh - min Z threshold to know if median filtering should be performed
 * @param[in] filtXsize - window x size
 * @param[in] filtYsize - window y size
 * @returns the filtered img
 */
RaveData2D_t* PdpProcessor_medfilt(PdpProcessor_t* self, RaveData2D_t* Z, double thresh, double nodataZ, long filtXsize, long filtYsize);

RaveData2D_t* PdpProcessor_residualClutterFilter(PdpProcessor_t* self, RaveData2D_t* Z,
    double thresholdZ, double thresholdTexture, long filtXsize, long filtYsize);

int PdpProcessor_pdpProcessing(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, long window, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp);

int PdpProcessor_pdpScript(PdpProcessor_t* self, RaveData2D_t* pdp, double dr, double rWin1, double rWin2, long nrIter, RaveData2D_t** pdpf, RaveData2D_t** kdp);

int PdpProcessor_attenuation(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* zdr, RaveData2D_t* pdp,
    RaveData2D_t* mask, double gamma_h, double alpha, RaveData2D_t** outz, RaveData2D_t** outzdr, RaveData2D_t** outPIA);

int PdpProcessor_zphi(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* pdp, RaveData2D_t* mask,
    double dr, double BB, double gamma_h, RaveData2D_t** outzphi, RaveData2D_t** outAH);
#endif
