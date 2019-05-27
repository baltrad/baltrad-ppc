'''
Created on Mar 30, 2019

@author: anders
'''
import unittest

import _raveio
import os, string
import numpy
import _rave
import _ppcoptions
import _ppcradaroptions

class PyPpcOptionsTest(unittest.TestCase):
  FIXTURE_1 = "fixtures/ppc_options_fixture_1.xml"
  def setUp(self):
    pass

  def tearDown(self):
    pass

  def testNew(self):
    a = _ppcoptions.new()
    self.assertNotEqual(-1, str(type(a)).find("PpcOptions"))
    
  def testLoad(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("default")
    self.assertEqual("default", a.name)
    self.assertAlmostEqual(0.1, a.parametersUZ[0], 3)
    self.assertAlmostEqual(0.2, a.parametersUZ[1], 3)
    self.assertAlmostEqual(0.3, a.parametersUZ[2], 3)
    self.assertAlmostEqual(0.4, a.parametersUZ[3], 3)
    self.assertAlmostEqual(0.5, a.parametersUZ[4], 3)

    self.assertAlmostEqual(1.0, a.parametersVEL[0], 3)
    self.assertAlmostEqual(2.0, a.parametersVEL[1], 3)
    self.assertAlmostEqual(3.0, a.parametersVEL[2], 3)
    self.assertAlmostEqual(4.0, a.parametersVEL[3], 3)
    self.assertAlmostEqual(5.0, a.parametersVEL[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersTEXT_PHIDP[0], 3)
    self.assertAlmostEqual(-0.2, a.parametersTEXT_PHIDP[1], 3)
    self.assertAlmostEqual(-0.3, a.parametersTEXT_PHIDP[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_PHIDP[3], 3)
    self.assertAlmostEqual(-0.5, a.parametersTEXT_PHIDP[4], 3)

    self.assertAlmostEqual(-1.0, a.parametersRHV[0], 3)
    self.assertAlmostEqual(-2.0, a.parametersRHV[1], 3)
    self.assertAlmostEqual(-3.0, a.parametersRHV[2], 3)
    self.assertAlmostEqual(-4.0, a.parametersRHV[3], 3)
    self.assertAlmostEqual(-5.0, a.parametersRHV[4], 3)

    self.assertAlmostEqual(0.1, a.parametersTEXT_UZ[0], 3)
    self.assertAlmostEqual(0.4, a.parametersTEXT_UZ[1], 3)
    self.assertAlmostEqual(-0.1, a.parametersTEXT_UZ[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_UZ[3], 3)
    self.assertAlmostEqual(2.0, a.parametersTEXT_UZ[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersCLUTTER_MAP[0], 3)
    self.assertAlmostEqual(-0.4, a.parametersCLUTTER_MAP[1], 3)
    self.assertAlmostEqual(0.1, a.parametersCLUTTER_MAP[2], 3)
    self.assertAlmostEqual(0.4, a.parametersCLUTTER_MAP[3], 3)
    self.assertAlmostEqual(-2.0, a.parametersCLUTTER_MAP[4], 3)

    self.assertEqual(2, a.minWindow)
    self.assertAlmostEqual(-77.7, a.nodata, 3)
    self.assertAlmostEqual(99.7, a.minDBZ, 3)    
    self.assertAlmostEqual(0.1, a.qualityThreshold, 3)    
    self.assertAlmostEqual(-2.0, a.preprocessZThreshold, 3)    

    self.assertAlmostEqual(111.1, a.residualMinZClutterThreshold, 3)    
    self.assertAlmostEqual(111.2, a.residualThresholdZ, 3)    
    self.assertAlmostEqual(111.3, a.residualThresholdTexture, 3)    
    self.assertAlmostEqual(111.4, a.residualClutterNodata, 3)    
    self.assertAlmostEqual(111.5, a.residualClutterMaskNodata, 3)    
    self.assertAlmostEqual(111.6, a.residualClutterTextureFilteringMaxZ, 3)    
    self.assertEqual(7, a.residualFilterBinSize)
    self.assertEqual(8, a.residualFilterRaySize)

    self.assertAlmostEqual(222.1, a.minZMedfilterThreshold, 3)    
    self.assertAlmostEqual(222.2, a.processingTextureThreshold, 3)    
    self.assertAlmostEqual(222.3, a.pdpRWin1, 3)    
    self.assertAlmostEqual(222.4, a.pdpRWin2, 3)    
    self.assertEqual(5, a.pdpNrIterations)    
    self.assertAlmostEqual(222.6, a.kdpUp, 3)    
    self.assertAlmostEqual(222.7, a.kdpDown, 3)    
    self.assertAlmostEqual(222.8, a.kdpStdThreshold, 3)    
    self.assertAlmostEqual(222.9, a.BB, 3)    
    self.assertAlmostEqual(222.11, a.thresholdPhidp, 3)    

    self.assertAlmostEqual(333.1, a.minAttenuationMaskRHOHV, 3)    
    self.assertAlmostEqual(333.2, a.minAttenuationMaskKDP, 3)    
    self.assertAlmostEqual(333.3, a.minAttenuationMaskTH, 3)    
    self.assertAlmostEqual(333.4, a.attenuationGammaH, 3)    
    self.assertAlmostEqual(333.5, a.attenuationAlpha, 3)    
    self.assertAlmostEqual(333.6, a.attenuationPIAminZ, 3)    

    self.assertAlmostEqual(444.1, a.meltingLayerBottomHeight, 3)    
    self.assertEqual(5, a.meltingLayerHourThreshold)
    
    self.assertEqual(_ppcradaroptions.P_TH_CORR|_ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.Q_ATTENUATION_MASK, a.requestedFields)
    
  def testLoad_allRequestedFields(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("all_requested_fields")
    self.assertEqual("all_requested_fields", a.name)

    expected = _ppcradaroptions.P_TH_CORR|_ppcradaroptions.P_ATT_TH_CORR|_ppcradaroptions.P_DBZH_CORR
    expected = _ppcradaroptions.P_ATT_DBZH_CORR|_ppcradaroptions.P_KDP_CORR|_ppcradaroptions.P_RHOHV_CORR | expected 
    expected = _ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.P_ZDR_CORR|_ppcradaroptions.P_ZPHI_CORR | expected 
    expected = _ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK|_ppcradaroptions.Q_ATTENUATION_MASK|_ppcradaroptions.Q_ATTENUATION | expected 

    self.assertEqual(expected, a.requestedFields)
    
  def testLoad_uses_standard(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("uses_standard")
    self.assertEqual("uses_standard", a.name)
    self.assertAlmostEqual(0.0, a.parametersUZ[0], 3)
    self.assertAlmostEqual(30.0, a.parametersUZ[1], 3)
    self.assertAlmostEqual(90.0, a.parametersUZ[2], 3)
    self.assertAlmostEqual(62.0, a.parametersUZ[3], 3)
    self.assertAlmostEqual(20.0, a.parametersUZ[4], 3)

    self.assertAlmostEqual(0.3, a.parametersVEL[0], 3)
    self.assertAlmostEqual(-0.9, a.parametersVEL[1], 3)
    self.assertAlmostEqual(0.9, a.parametersVEL[2], 3)
    self.assertAlmostEqual(0.15, a.parametersVEL[3], 3)
    self.assertAlmostEqual(0.15, a.parametersVEL[4], 3)

    self.assertAlmostEqual(0.8, a.parametersTEXT_PHIDP[0], 3)
    self.assertAlmostEqual(15.0, a.parametersTEXT_PHIDP[1], 3)
    self.assertAlmostEqual(40.0, a.parametersTEXT_PHIDP[2], 3)
    self.assertAlmostEqual(5.0, a.parametersTEXT_PHIDP[3], 3)
    self.assertAlmostEqual(40.0, a.parametersTEXT_PHIDP[4], 3)

    self.assertAlmostEqual(0.2, a.parametersRHV[0], 3)
    self.assertAlmostEqual(0.0, a.parametersRHV[1], 3)
    self.assertAlmostEqual(0.6, a.parametersRHV[2], 3)
    self.assertAlmostEqual(0.0, a.parametersRHV[3], 3)
    self.assertAlmostEqual(0.1, a.parametersRHV[4], 3)

    self.assertAlmostEqual(0.3, a.parametersTEXT_UZ[0], 3)
    self.assertAlmostEqual(20.0, a.parametersTEXT_UZ[1], 3)
    self.assertAlmostEqual(60.0, a.parametersTEXT_UZ[2], 3)
    self.assertAlmostEqual(5.0, a.parametersTEXT_UZ[3], 3)
    self.assertAlmostEqual(10.0, a.parametersTEXT_UZ[4], 3)

    self.assertAlmostEqual(0.9, a.parametersCLUTTER_MAP[0], 3)
    self.assertAlmostEqual(5.0, a.parametersCLUTTER_MAP[1], 3)
    self.assertAlmostEqual(70.0, a.parametersCLUTTER_MAP[2], 3)
    self.assertAlmostEqual(20.0, a.parametersCLUTTER_MAP[3], 3)
    self.assertAlmostEqual(60.0, a.parametersCLUTTER_MAP[4], 3)

    self.assertEqual(11, a.minWindow)
    self.assertAlmostEqual(-999.0, a.nodata, 3)
    self.assertAlmostEqual(-32.0, a.minDBZ, 3)    
    self.assertAlmostEqual(0.75, a.qualityThreshold, 3)    
    self.assertAlmostEqual(-20.0, a.preprocessZThreshold, 3)    

    self.assertAlmostEqual(-31.5, a.residualMinZClutterThreshold, 3)    
    self.assertAlmostEqual(-20.0, a.residualThresholdZ, 3)    
    self.assertAlmostEqual(20.0, a.residualThresholdTexture, 3)    
    self.assertAlmostEqual(-999.0, a.residualClutterNodata, 3)    
    self.assertAlmostEqual(-1.0, a.residualClutterMaskNodata, 3)    
    self.assertAlmostEqual(70.0, a.residualClutterTextureFilteringMaxZ, 3)    
    self.assertEqual(1, a.residualFilterBinSize)
    self.assertEqual(1, a.residualFilterRaySize)

    self.assertAlmostEqual(-30.0, a.minZMedfilterThreshold, 3)    
    self.assertAlmostEqual(10.0, a.processingTextureThreshold, 3)    
    self.assertAlmostEqual(3.5, a.pdpRWin1, 3)    
    self.assertAlmostEqual(1.5, a.pdpRWin2, 3)    
    self.assertEqual(2, a.pdpNrIterations)    
    self.assertAlmostEqual(20.0, a.kdpUp, 3)    
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)    
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)    
    self.assertAlmostEqual(0.7987, a.BB, 3)    
    self.assertAlmostEqual(40.0, a.thresholdPhidp, 3)    

    self.assertAlmostEqual(0.8, a.minAttenuationMaskRHOHV, 3)    
    self.assertAlmostEqual(0.001, a.minAttenuationMaskKDP, 3)    
    self.assertAlmostEqual(-20.0, a.minAttenuationMaskTH, 3)    
    self.assertAlmostEqual(0.08, a.attenuationGammaH, 3)    
    self.assertAlmostEqual(0.2, a.attenuationAlpha, 3)    
    self.assertAlmostEqual(-30.0, a.attenuationPIAminZ, 3) 

    self.assertEqual(_ppcradaroptions.P_DBZH_CORR|_ppcradaroptions.P_ATT_DBZH_CORR|_ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK, a.requestedFields)

  def testLoad_uses_default(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("uses_default")
    self.assertEqual("uses_default", a.name)
    self.assertAlmostEqual(0.1, a.parametersUZ[0], 3)
    self.assertAlmostEqual(0.2, a.parametersUZ[1], 3)
    self.assertAlmostEqual(0.3, a.parametersUZ[2], 3)
    self.assertAlmostEqual(0.4, a.parametersUZ[3], 3)
    self.assertAlmostEqual(0.5, a.parametersUZ[4], 3)

    self.assertAlmostEqual(1.0, a.parametersVEL[0], 3)
    self.assertAlmostEqual(2.0, a.parametersVEL[1], 3)
    self.assertAlmostEqual(3.0, a.parametersVEL[2], 3)
    self.assertAlmostEqual(4.0, a.parametersVEL[3], 3)
    self.assertAlmostEqual(5.0, a.parametersVEL[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersTEXT_PHIDP[0], 3)
    self.assertAlmostEqual(-0.2, a.parametersTEXT_PHIDP[1], 3)
    self.assertAlmostEqual(-0.3, a.parametersTEXT_PHIDP[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_PHIDP[3], 3)
    self.assertAlmostEqual(-0.5, a.parametersTEXT_PHIDP[4], 3)

    self.assertAlmostEqual(-1.0, a.parametersRHV[0], 3)
    self.assertAlmostEqual(-2.0, a.parametersRHV[1], 3)
    self.assertAlmostEqual(-3.0, a.parametersRHV[2], 3)
    self.assertAlmostEqual(-4.0, a.parametersRHV[3], 3)
    self.assertAlmostEqual(-5.0, a.parametersRHV[4], 3)

    self.assertAlmostEqual(0.1, a.parametersTEXT_UZ[0], 3)
    self.assertAlmostEqual(0.4, a.parametersTEXT_UZ[1], 3)
    self.assertAlmostEqual(-0.1, a.parametersTEXT_UZ[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_UZ[3], 3)
    self.assertAlmostEqual(2.0, a.parametersTEXT_UZ[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersCLUTTER_MAP[0], 3)
    self.assertAlmostEqual(-0.4, a.parametersCLUTTER_MAP[1], 3)
    self.assertAlmostEqual(0.1, a.parametersCLUTTER_MAP[2], 3)
    self.assertAlmostEqual(0.4, a.parametersCLUTTER_MAP[3], 3)
    self.assertAlmostEqual(-2.0, a.parametersCLUTTER_MAP[4], 3)

    self.assertEqual(2, a.minWindow)
    self.assertAlmostEqual(-77.7, a.nodata, 3)
    self.assertAlmostEqual(99.7, a.minDBZ, 3)    
    self.assertAlmostEqual(0.1, a.qualityThreshold, 3)    
    self.assertAlmostEqual(-2.0, a.preprocessZThreshold, 3)    

    self.assertAlmostEqual(111.1, a.residualMinZClutterThreshold, 3)    
    self.assertAlmostEqual(111.2, a.residualThresholdZ, 3)    
    self.assertAlmostEqual(111.3, a.residualThresholdTexture, 3)    
    self.assertAlmostEqual(111.4, a.residualClutterNodata, 3)    
    self.assertAlmostEqual(111.5, a.residualClutterMaskNodata, 3)    
    self.assertAlmostEqual(111.6, a.residualClutterTextureFilteringMaxZ, 3)    
    self.assertEqual(7, a.residualFilterBinSize)
    self.assertEqual(8, a.residualFilterRaySize)

    self.assertAlmostEqual(222.1, a.minZMedfilterThreshold, 3)    
    self.assertAlmostEqual(222.2, a.processingTextureThreshold, 3)    
    self.assertAlmostEqual(222.3, a.pdpRWin1, 3)    
    self.assertAlmostEqual(222.4, a.pdpRWin2, 3)    
    self.assertEqual(5, a.pdpNrIterations)    
    self.assertAlmostEqual(222.6, a.kdpUp, 3)    
    self.assertAlmostEqual(222.7, a.kdpDown, 3)    
    self.assertAlmostEqual(222.8, a.kdpStdThreshold, 3)    
    self.assertAlmostEqual(222.9, a.BB, 3)    
    self.assertAlmostEqual(222.11, a.thresholdPhidp, 3)    

    self.assertAlmostEqual(333.1, a.minAttenuationMaskRHOHV, 3)    
    self.assertAlmostEqual(333.2, a.minAttenuationMaskKDP, 3)    
    self.assertAlmostEqual(333.3, a.minAttenuationMaskTH, 3)    
    self.assertAlmostEqual(333.4, a.attenuationGammaH, 3)    
    self.assertAlmostEqual(333.5, a.attenuationAlpha, 3)    
    self.assertAlmostEqual(333.6, a.attenuationPIAminZ, 3)    

    self.assertEqual(_ppcradaroptions.P_TH_CORR|_ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.Q_ATTENUATION_MASK, a.requestedFields)

  def testLoad_modified_default(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("modified_default")
    self.assertEqual("modified_default", a.name)
    self.assertAlmostEqual(0.1, a.parametersUZ[0], 3)
    self.assertAlmostEqual(0.2, a.parametersUZ[1], 3)
    self.assertAlmostEqual(0.3, a.parametersUZ[2], 3)
    self.assertAlmostEqual(0.4, a.parametersUZ[3], 3)
    self.assertAlmostEqual(0.5, a.parametersUZ[4], 3)

    self.assertAlmostEqual(1.0, a.parametersVEL[0], 3)
    self.assertAlmostEqual(2.0, a.parametersVEL[1], 3)
    self.assertAlmostEqual(3.0, a.parametersVEL[2], 3)
    self.assertAlmostEqual(4.0, a.parametersVEL[3], 3)
    self.assertAlmostEqual(5.0, a.parametersVEL[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersTEXT_PHIDP[0], 3)
    self.assertAlmostEqual(-0.2, a.parametersTEXT_PHIDP[1], 3)
    self.assertAlmostEqual(-0.3, a.parametersTEXT_PHIDP[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_PHIDP[3], 3)
    self.assertAlmostEqual(-0.5, a.parametersTEXT_PHIDP[4], 3)

    self.assertAlmostEqual(-1.0, a.parametersRHV[0], 3)
    self.assertAlmostEqual(-2.0, a.parametersRHV[1], 3)
    self.assertAlmostEqual(-3.0, a.parametersRHV[2], 3)
    self.assertAlmostEqual(-4.0, a.parametersRHV[3], 3)
    self.assertAlmostEqual(-5.0, a.parametersRHV[4], 3)

    self.assertAlmostEqual(0.1, a.parametersTEXT_UZ[0], 3)
    self.assertAlmostEqual(0.4, a.parametersTEXT_UZ[1], 3)
    self.assertAlmostEqual(-0.1, a.parametersTEXT_UZ[2], 3)
    self.assertAlmostEqual(-0.4, a.parametersTEXT_UZ[3], 3)
    self.assertAlmostEqual(2.0, a.parametersTEXT_UZ[4], 3)

    self.assertAlmostEqual(-0.1, a.parametersCLUTTER_MAP[0], 3)
    self.assertAlmostEqual(-0.4, a.parametersCLUTTER_MAP[1], 3)
    self.assertAlmostEqual(0.1, a.parametersCLUTTER_MAP[2], 3)
    self.assertAlmostEqual(0.4, a.parametersCLUTTER_MAP[3], 3)
    self.assertAlmostEqual(-2.0, a.parametersCLUTTER_MAP[4], 3)

    self.assertEqual(2, a.minWindow)
    self.assertAlmostEqual(-77.7, a.nodata, 3)
    self.assertAlmostEqual(99.7, a.minDBZ, 3)    
    self.assertAlmostEqual(0.1, a.qualityThreshold, 3)    
    self.assertAlmostEqual(-2.0, a.preprocessZThreshold, 3)    

    self.assertAlmostEqual(111.1, a.residualMinZClutterThreshold, 3)    
    self.assertAlmostEqual(111.2, a.residualThresholdZ, 3)    
    self.assertAlmostEqual(111.3, a.residualThresholdTexture, 3)    
    self.assertAlmostEqual(111.4, a.residualClutterNodata, 3)    
    self.assertAlmostEqual(111.5, a.residualClutterMaskNodata, 3)    
    self.assertAlmostEqual(111.6, a.residualClutterTextureFilteringMaxZ, 3)    
    self.assertEqual(7, a.residualFilterBinSize)
    self.assertEqual(8, a.residualFilterRaySize)

    self.assertAlmostEqual(222.1, a.minZMedfilterThreshold, 3)    
    self.assertAlmostEqual(222.2, a.processingTextureThreshold, 3)    
    self.assertAlmostEqual(222.3, a.pdpRWin1, 3)    
    self.assertAlmostEqual(222.4, a.pdpRWin2, 3)    
    self.assertEqual(5, a.pdpNrIterations)    
    self.assertAlmostEqual(444.1, a.kdpUp, 3)    
    self.assertAlmostEqual(444.2, a.kdpDown, 3)    
    self.assertAlmostEqual(444.3, a.kdpStdThreshold, 3)    
    self.assertAlmostEqual(222.9, a.BB, 3)    
    self.assertAlmostEqual(222.11, a.thresholdPhidp, 3)    

    self.assertAlmostEqual(333.1, a.minAttenuationMaskRHOHV, 3)    
    self.assertAlmostEqual(333.2, a.minAttenuationMaskKDP, 3)    
    self.assertAlmostEqual(333.3, a.minAttenuationMaskTH, 3)    
    self.assertAlmostEqual(333.4, a.attenuationGammaH, 3)    
    self.assertAlmostEqual(333.5, a.attenuationAlpha, 3)    
    self.assertAlmostEqual(333.6, a.attenuationPIAminZ, 3)    

    self.assertEqual(_ppcradaroptions.P_TH_CORR|_ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.Q_ATTENUATION_MASK, a.requestedFields)

  def testLoad_overrided_default(self):
    a = _ppcoptions.load(self.FIXTURE_1).getRadarOptions("overrided_default")
    self.assertEqual("overrided_default", a.name)

    self.assertAlmostEqual(555.1, a.kdpUp, 3)    
    self.assertAlmostEqual(444.2, a.kdpDown, 3)    
    self.assertAlmostEqual(444.3, a.kdpStdThreshold, 3)    

    self.assertEqual(_ppcradaroptions.P_TH_CORR|_ppcradaroptions.P_PHIDP_CORR|_ppcradaroptions.Q_ATTENUATION_MASK, a.requestedFields)


  def testExists(self):
    a = _ppcoptions.load(self.FIXTURE_1)
    self.assertEqual(True, a.exists("default"))
    self.assertEqual(True, a.exists("uses_standard"))
    self.assertEqual(True, a.exists("uses_default"))
    self.assertEqual(True, a.exists("modified_default"))
    self.assertEqual(True, a.exists("overrided_default"))
    self.assertEqual(True, a.exists("all_requested_fields"))
    self.assertEqual(False, a.exists("nisse"))
    self.assertEqual(False, a.exists("nosuch_default"))
    
    

if __name__ == "__main__":
  #import sys;sys.argv = ['', 'Test.testName']
  unittest.main()