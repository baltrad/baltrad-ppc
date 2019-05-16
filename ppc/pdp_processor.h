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
#include "ppc_radar_options.h"
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

#define PdpProcessor_TH_CORR        (1)       /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected TH to result */
#define PdpProcessor_ATT_TH_CORR    (1 << 1)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected and attenuated TH to result */
#define PdpProcessor_DBZH_CORR      (1 << 2)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected DBZH to result */
#define PdpProcessor_ATT_DBZH_CORR  (1 << 3)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected and attenuated DBZH to result */
#define PdpProcessor_KDP_CORR       (1 << 4)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected KDP to result */
#define PdpProcessor_RHOHV_CORR     (1 << 5)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected RHOHV to result */
#define PdpProcessor_PHIDP_CORR     (1 << 6)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected PHIDP to result */
#define PdpProcessor_ZDR_CORR       (1 << 7)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected ZDR to result */
#define PdpProcessor_ZPHI_CORR      (1 << 8)  /**< Used by @ref #PdpProcessor_setRequestedFields Add Corrected ZPHI to result */
#define PdpProcessor_QUALITY_RESIDUAL_CLUTTER_MASK (1 << 9) /**< Used by @ref #PdpProcessor_setRequestedFields Add quality flag for residual clutter flag */
#define PdpProcessor_QUALITY_ATTENUATION_MASK (1 << 10) /**< Used by @ref #PdpProcessor_setRequestedFields Add quality flag for attenuation mask */
#define PdpProcessor_QUALITY_ATTENUATION (1 << 11) /**< Used by @ref #PdpProcessor_setRequestedFields Add quality flag for actual attenuation */


/**
 * Sets the option instance to be used in the processing. Note, the options will be
 * stored as a reference which means that you can directly modify the options instance
 * to modify behaviour.
 * @param[in] self - self
 * @param[in] options - options (must not be NULL)
 * @returns 1 on success or null if options is NULL
 */
int PdpProcessor_setRadarOptions(PdpProcessor_t* self, PpcRadarOptions_t* options);

/**
 * @param[in] self - self
 * @returns a reference to the internal options class
 */
PpcRadarOptions_t* PdpProcessor_getRadarOptions(PdpProcessor_t* self);

/**
 * Combines all functions implemented in this class into one process that performs the actual polar data processing chain
 * according to the matlab prototype developed by Gianfranco Vulpiani.
 * @param[in] self - self
 * @param[in] scan - the polar scan
 * @returns new scan on success otherwise NULL
 */
PolarScan_t* PdpProcessor_process(PdpProcessor_t* self, PolarScan_t* scan);

/**
 * Returns the bottom melting layer height for the specified scan
 */
double PdpProcessor_getMeltingLayerBottomHeight(PolarScan_t* scan);

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

int PdpProcessor_attenuation(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* zdr, RaveData2D_t* dbzh, RaveData2D_t* pdp,
    RaveData2D_t* mask, double gamma_h, double alpha, RaveData2D_t** outz, RaveData2D_t** outzdr, RaveData2D_t** outPIA, RaveData2D_t** outDBZH);

int PdpProcessor_zphi(PdpProcessor_t* self, RaveData2D_t* Z, RaveData2D_t* pdp, RaveData2D_t* mask,
    double dr, double BB, double gamma_h, RaveData2D_t** outzphi, RaveData2D_t** outAH);
#endif
