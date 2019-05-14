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

/**
 * The ppc class
 */
struct _PpcOptions_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char* filename; /**< name of the loaded options */
};

/*@{ Private functions */
/**
 * Constructor
 */
static int PpcOptions_constructor(RaveCoreObject* obj)
{
  PpcOptions_t* options = (PpcOptions_t*)obj;
  options->filename = NULL;
  return 1;
}

/**
 * Destructor
 */
static void PpcOptions_destructor(RaveCoreObject* obj)
{
  PpcOptions_t* options = (PpcOptions_t*)obj;
  RAVE_FREE(options->filename);
}

/**
 * Copy constructor
 */
static int PpcOptions_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PpcOptions_t* this = (PpcOptions_t*)obj;
  PpcOptions_t* src = (PpcOptions_t*)srcobj;
  this->filename = NULL;
  if (src->filename != NULL) {
    this->filename = RAVE_STRDUP(src->filename);
    if (this->filename == NULL) {
      goto fail;
    }
  }

  return 1;
fail:
  return 0;
}

/*@} End of Private functions */

/*@{ Interface functions */
PpcOptions_t* PpcOptions_load(const char* filename)
{

}

const char* PpcOptions_getValue(PpcOptions_t* self, const char* name)
{

}


/*@} End of Interface functions */

RaveCoreObjectType PpcOptions_TYPE = {
    "PpcOptions",
    sizeof(PpcOptions_t),
    PpcOptions_constructor,
    PpcOptions_destructor,
    PpcOptions_copyconstructor
};

