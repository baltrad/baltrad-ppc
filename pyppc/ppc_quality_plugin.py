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

logger = rave_pgf_logger.create_logger()

# The baltrad-ppc quality plugin
#
class ppc_quality_plugin(rave_quality_plugin):
  ##
  # Default constructor
  def __init__(self):
    super(ppc_quality_plugin, self).__init__()
  
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
          processor.requestedFields = _pdpprocessor.P_CORR_ATT_TH | _pdpprocessor.Q_QUALITY_RESIDUAL_CLUTTER_MASK
          result = processor.process(obj)
          obj.addOrReplaceQualityField(result.getQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask"))
          if quality_control_mode != QUALITY_CONTROL_MODE_ANALYZE:
            f = obj.getParameter("CORR_ATT_TH")
            f.quantity = "TH"
            obj.addParameter(f)
          
        elif _polarvolume.isPolarVolume(obj):
          for i in range(obj.getNumberOfScans()):
            scan = obj.getScan(i)
            if reprocess_quality_flag == False and scan.findQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask") != None:
              continue
            processor = _pdpprocessor.new()
            processor.requestedFields = _pdpprocessor.P_CORR_ATT_TH | _pdpprocessor.Q_QUALITY_RESIDUAL_CLUTTER_MASK
            result = processor.process(scan)
            scan.addOrReplaceQualityField(result.getQualityFieldByHowTask("se.baltrad.ppc.residual_clutter_mask"))
            if quality_control_mode != QUALITY_CONTROL_MODE_ANALYZE:
              f = scan.getParameter("CORR_ATT_TH")
              f.quantity = "TH"
              scan.addParameter(f)
      except:
        logger.exception("Failed to generate baltrad-ppc field")

    return obj
