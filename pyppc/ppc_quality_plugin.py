'''
Copyright (C) 2019- Swedish Meteorological and Hydrological Institute (SMHI)

This file is part of the baltrad-ppc extension to RAVE.

baltrad-ppc is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

baltrad-ppc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with baltrad-ppc.  If not, see <http://www.gnu.org/licenses/>.
'''
##
# A quality plugin for enabling the baltrad-ppc support 

## 
# @file
# @author Anders Henja, SMHI
# @date 2019-05-03

from rave_quality_plugin import rave_quality_plugin
from rave_quality_plugin import QUALITY_CONTROL_MODE_ANALYZE_AND_APPLY
from rave_quality_plugin import QUALITY_CONTROL_MODE_ANALYZE

import rave_pgf_logger

import _polarscan
import _polarvolume
import _pdpprocessor
import _ppcoptions
import _ppcradaroptions
import odim_source
import os

logger = rave_pgf_logger.create_logger()

CONFIG_FILE = os.path.join(os.path.join(os.path.split(os.path.split(_pdpprocessor.__file__)[0])[0],
                                        'config'), 'ppc_options.xml')

PPC_OPTIONS=None
try:
  logger.info("Loading options for %s"%CONFIG_FILE)
  PPC_OPTIONS=_ppcoptions.load(CONFIG_FILE)
except:
  logger.exception("Failed to load options")

nodomdb=False
try:
  import rave_dom_db
except:
  nodomdb=True


# The baltrad-ppc quality plugin
#
class ppc_quality_plugin(rave_quality_plugin):
  ##
  # Default constructor
  def __init__(self):
    super(ppc_quality_plugin, self).__init__()
    self._options = PPC_OPTIONS 
  
  def get_options(self, polarobj):
    odim_source.CheckSource(polarobj)
    S = odim_source.ODIM_Source(polarobj.source)
    nod = None
    if S is not None and S.nod is not None:
      nod = S.nod

    if self._options:
      if self._options.exists(S.nod):
        #logger.info("Using %s ppc radar options"%S.nod)
        return nod, self._options.getRadarOptions(S.nod)
      elif self._options.exists("default"):
        #logger.info("Using default ppc radar options")
        return nod, self._options.getRadarOptions("default")
    if S is not None and S.nod is not None:
      logger.info("Check configuration! Using backup default radar option for %s"%S.nod)
    return nod, _ppcradaroptions.new()
  
  ##
  # @return a list containing the string se.baltrad.ppc.residual_clutter_mask
  def getQualityFields(self):
    return ["se.baltrad.ppc.residual_clutter_mask"]
  
  ##
  # @param obj: A rave object that should be processed.
  # @param reprocess_quality_flag: If the quality fields should be reprocessed or not.
  # @param arguments: Not used
  # @return: The modified object if this quality plugin has performed changes 
  # to the object.
  def process(self, obj, reprocess_quality_flag=True, quality_control_mode=QUALITY_CONTROL_MODE_ANALYZE_AND_APPLY, arguments=None):
    if obj != None:
      try:
        if _polarscan.isPolarScan(obj):
          if reprocess_quality_flag == False and obj.findQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask") != None:
            return obj
          processor = _pdpprocessor.new()
          nod, processor.options = self.get_options(obj)
          if not nodomdb and nod is not None:
            try:
              db = rave_dom_db.create_db_from_conf()
              latest=db.get_latest_melting_layer(nod, processor.options.meltingLayerHourThreshold)
              if latest is not None and latest.bottom is not None:
                processor.meltingLayerBottomHeight = latest.bottom
            except Exception as e:
              logger.error("Failed to determine melting layer bottom height: "%e.__str__())
          processor.options.requestedFields = processor.options.requestedFields | _ppcradaroptions.P_DBZH_CORR | _ppcradaroptions.P_ATT_DBZH_CORR | _ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK
          result = processor.process(obj)
          obj.addOrReplaceQualityField(result.getQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask"))
          if quality_control_mode != QUALITY_CONTROL_MODE_ANALYZE:
            f = result.getParameter("ATT_DBZH_CORR")
            f.quantity = "DBZH"
            obj.addParameter(f)
          
        elif _polarvolume.isPolarVolume(obj):
          nod, options = self.get_options(obj)
          meltingLayer = None
          if not nodomdb and nod is not None:
            try:
              db = rave_dom_db.create_db_from_conf()
              latest=db.get_latest_melting_layer(nod, options.meltingLayerHourThreshold)
              if latest is not None and latest.bottom is not None:
                meltingLayer = latest.bottom
            except Exception as e:
              logger.error("Failed to determine melting layer bottom height: "%e.__str__())
          
          for i in range(obj.getNumberOfScans()):
            scan = obj.getScan(i)
            if reprocess_quality_flag == False and scan.findQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask") != None:
              continue
            processor = _pdpprocessor.new()
            processor.options = options
            if meltingLayer is not None:
              processor.meltingLayerBottomHeight = meltingLayer
            processor.options.requestedFields = processor.options.requestedFields | _ppcradaroptions.P_DBZH_CORR | _ppcradaroptions.P_ATT_DBZH_CORR | _ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK
            result = processor.process(scan)
            scan.addOrReplaceQualityField(result.getQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask"))
            if quality_control_mode != QUALITY_CONTROL_MODE_ANALYZE:
              f = result.getParameter("ATT_DBZH_CORR")
              f.quantity = "DBZH"
              scan.addParameter(f)
      except:
        logger.exception("Failed to generate baltrad-ppc field")

    return obj
