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
#ifndef PPC_OPTIONS_H
#define PPC_OPTIONS_H
#include "ppc_radar_options.h"
#include <raveobject_hashtable.h>

/**
 * Defines a transformer
 */
typedef struct _PpcOptions_t PpcOptions_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PpcOptions_TYPE;

PpcOptions_t* PpcOptions_load(const char* filename);

/**
 * @returns if there are options for the specified (node) name.
 * @param[in] self - self
 * @param[in] name - the node name
 */
int PpcOptions_exists(PpcOptions_t* self, const char* name);

/**
 * @returns the options for the specified (node) name.
 * @param[in] self - self
 * @param[in] name - the node name
 */
PpcRadarOptions_t* PpcOptions_getRadarOptions(PpcOptions_t* self, const char* name);

/**
 * Adds one radar option to the option table. The radar option name must be set.
 * @param[in] self - self
 * @param[in] options - options
 * @return 1 on success otherwise 0
 */
int PpcOptions_addRadarOptions(PpcOptions_t* self, PpcRadarOptions_t* options);

/**
 * Returns a list of options in the repository
 * @param[in] self - self
 * @returns a hashtable with the options. Note, this is cloned and not internal.
 */
RaveObjectHashTable_t* PpcOptions_options(PpcOptions_t* self);

#endif /* PPC_OPTIONS_H_ */
