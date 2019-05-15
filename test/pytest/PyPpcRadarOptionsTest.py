'''
Created on Mar 30, 2019

@author: anders
'''
import unittest

import _raveio
import os, string
import numpy
import _rave
import _ppcradaroptions

class PyPpcRadarOptionsTest(unittest.TestCase):
  def setUp(self):
    pass

  def tearDown(self):
    pass

  def testNew(self):
    a = _ppcradaroptions.new()
    self.assertNotEqual(-1, str(type(a)).find("PpcRadarOptions"))

  def testMinWindow(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minWindow" in dir(a))
    
    self.assertEqual(11, a.minWindow)
    
    a.minWindow = 1
    
    self.assertEqual(1, a.minWindow)
  
  def testPdpRWin1(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("pdpRWin1" in dir(a))
    
    self.assertAlmostEqual(3.5, a.pdpRWin1, 3)
    a.pdpRWin1 = 1
    self.assertAlmostEqual(1.0, a.pdpRWin1, 3);

  def testpdpRWin2(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("pdpRWin2" in dir(a))
    
    self.assertAlmostEqual(1.5, a.pdpRWin2, 3)
    a.pdpRWin2 = 1
    self.assertAlmostEqual(1.0, a.pdpRWin2, 3);

  def testPdpNrIterations(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("pdpNrIterations" in dir(a))
    
    self.assertEqual(2, a.pdpNrIterations)
    a.pdpNrIterations = 1
    self.assertEqual(1.0, a.pdpNrIterations);

  def testMinZMedfilterThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minZMedfilterThreshold" in dir(a))
    
    self.assertAlmostEqual(-30.0, a.minZMedfilterThreshold, 3)
    a.minZMedfilterThreshold = 1
    self.assertAlmostEqual(1.0, a.minZMedfilterThreshold, 3);

  def testProcessingTextureThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("processingTextureThreshold" in dir(a))
    
    self.assertAlmostEqual(10.0, a.processingTextureThreshold, 3)
    a.processingTextureThreshold = 1
    self.assertAlmostEqual(1.0, a.processingTextureThreshold, 3);

  def testParametersUZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("parametersUZ" in dir(a))
    
    self.assertAlmostEqual(0.0, a.parametersUZ[0], 3);
    self.assertAlmostEqual(30.0, a.parametersUZ[1], 3);
    self.assertAlmostEqual(90.0, a.parametersUZ[2], 3);
    self.assertAlmostEqual(62.0, a.parametersUZ[3], 3);
    self.assertAlmostEqual(20.0, a.parametersUZ[4], 3);
    
    a.parametersUZ = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersUZ[0], 3);
    self.assertAlmostEqual(2.0, a.parametersUZ[1], 3);
    self.assertAlmostEqual(3.0, a.parametersUZ[2], 3);
    self.assertAlmostEqual(4.0, a.parametersUZ[3], 3);
    self.assertAlmostEqual(5.0, a.parametersUZ[4], 3);

  def testParametersVEL(self):
    a = _ppcradaroptions.new()

    self.assertTrue("parametersVEL" in dir(a))

    self.assertAlmostEqual(0.3, a.parametersVEL[0], 3);
    self.assertAlmostEqual(-0.9, a.parametersVEL[1], 3);
    self.assertAlmostEqual(0.9, a.parametersVEL[2], 3);
    self.assertAlmostEqual(0.15, a.parametersVEL[3], 3);
    self.assertAlmostEqual(0.15, a.parametersVEL[4], 3);
    
    a.parametersVEL = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersVEL[0], 3);
    self.assertAlmostEqual(2.0, a.parametersVEL[1], 3);
    self.assertAlmostEqual(3.0, a.parametersVEL[2], 3);
    self.assertAlmostEqual(4.0, a.parametersVEL[3], 3);
    self.assertAlmostEqual(5.0, a.parametersVEL[4], 3);

  def testParametersTEXT_PHIDP(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("parametersTEXT_PHIDP" in dir(a))

    self.assertAlmostEqual(0.8, a.parametersTEXT_PHIDP[0], 3);
    self.assertAlmostEqual(15.0, a.parametersTEXT_PHIDP[1], 3);
    self.assertAlmostEqual(40.0, a.parametersTEXT_PHIDP[2], 3);
    self.assertAlmostEqual(5.0, a.parametersTEXT_PHIDP[3], 3);
    self.assertAlmostEqual(40.0, a.parametersTEXT_PHIDP[4], 3);
    
    a.parametersTEXT_PHIDP = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersTEXT_PHIDP[0], 3);
    self.assertAlmostEqual(2.0, a.parametersTEXT_PHIDP[1], 3);
    self.assertAlmostEqual(3.0, a.parametersTEXT_PHIDP[2], 3);
    self.assertAlmostEqual(4.0, a.parametersTEXT_PHIDP[3], 3);
    self.assertAlmostEqual(5.0, a.parametersTEXT_PHIDP[4], 3);

  def testParametersRHV(self):
    a = _ppcradaroptions.new()

    self.assertTrue("parametersRHV" in dir(a))

    self.assertAlmostEqual(0.2, a.parametersRHV[0], 3);
    self.assertAlmostEqual(0.0, a.parametersRHV[1], 3);
    self.assertAlmostEqual(0.6, a.parametersRHV[2], 3);
    self.assertAlmostEqual(0.0, a.parametersRHV[3], 3);
    self.assertAlmostEqual(0.1, a.parametersRHV[4], 3);
    
    a.parametersRHV = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersRHV[0], 3);
    self.assertAlmostEqual(2.0, a.parametersRHV[1], 3);
    self.assertAlmostEqual(3.0, a.parametersRHV[2], 3);
    self.assertAlmostEqual(4.0, a.parametersRHV[3], 3);
    self.assertAlmostEqual(5.0, a.parametersRHV[4], 3);

  def testParametersTEXT_UZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("parametersTEXT_UZ" in dir(a))
    
    # DEFAULT_PAR_TEXT_UZ[5] =     {0.30,  20.00,  60.00,   5.00,  10.00 };
    self.assertAlmostEqual(0.3, a.parametersTEXT_UZ[0], 3);
    self.assertAlmostEqual(20.0, a.parametersTEXT_UZ[1], 3);
    self.assertAlmostEqual(60.0, a.parametersTEXT_UZ[2], 3);
    self.assertAlmostEqual(5.0, a.parametersTEXT_UZ[3], 3);
    self.assertAlmostEqual(10.0, a.parametersTEXT_UZ[4], 3);
    
    a.parametersTEXT_UZ = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersTEXT_UZ[0], 3);
    self.assertAlmostEqual(2.0, a.parametersTEXT_UZ[1], 3);
    self.assertAlmostEqual(3.0, a.parametersTEXT_UZ[2], 3);
    self.assertAlmostEqual(4.0, a.parametersTEXT_UZ[3], 3);
    self.assertAlmostEqual(5.0, a.parametersTEXT_UZ[4], 3);

  def testParametersCLUTTER_MAP(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("parametersCLUTTER_MAP" in dir(a))
    
    self.assertAlmostEqual(0.9, a.parametersCLUTTER_MAP[0], 3);
    self.assertAlmostEqual(5.0, a.parametersCLUTTER_MAP[1], 3);
    self.assertAlmostEqual(70.0, a.parametersCLUTTER_MAP[2], 3);
    self.assertAlmostEqual(20.0, a.parametersCLUTTER_MAP[3], 3);
    self.assertAlmostEqual(60.0, a.parametersCLUTTER_MAP[4], 3);
    
    a.parametersCLUTTER_MAP = (1.0, 2.0, 3.0, 4.0, 5.0)
    self.assertAlmostEqual(1.0, a.parametersCLUTTER_MAP[0], 3);
    self.assertAlmostEqual(2.0, a.parametersCLUTTER_MAP[1], 3);
    self.assertAlmostEqual(3.0, a.parametersCLUTTER_MAP[2], 3);
    self.assertAlmostEqual(4.0, a.parametersCLUTTER_MAP[3], 3);
    self.assertAlmostEqual(5.0, a.parametersCLUTTER_MAP[4], 3);

  def testNodata(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("nodata" in dir(a))
    
    self.assertAlmostEqual(-999.0, a.nodata, 3)
    a.nodata = 1
    self.assertAlmostEqual(1.0, a.nodata, 3);

  def testMinDBZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minDBZ" in dir(a))
    
    self.assertAlmostEqual(-32.0, a.minDBZ, 3)
    a.minDBZ = 1
    self.assertAlmostEqual(1.0, a.minDBZ, 3);

  def testQualityThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("qualityThreshold" in dir(a))
    
    self.assertAlmostEqual(0.75, a.qualityThreshold, 3)
    a.qualityThreshold = 1
    self.assertAlmostEqual(1.0, a.qualityThreshold, 3);

  def testPreprocessZThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("preprocessZThreshold" in dir(a))
    
    self.assertAlmostEqual(-20.0, a.preprocessZThreshold, 3)
    a.preprocessZThreshold = 1
    self.assertAlmostEqual(1.0, a.preprocessZThreshold, 3);
    
    
  def testKdpUp(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("kdpUp" in dir(a))
    
    self.assertAlmostEqual(20.0, a.kdpUp, 3)
    a.kdpUp = 1
    self.assertAlmostEqual(1.0, a.kdpUp, 3);
    
  def testKdpDown(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("kdpDown" in dir(a))
    
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)
    a.kdpDown = 1
    self.assertAlmostEqual(1.0, a.kdpDown, 3);

  def testKdpStdThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("kdpStdThreshold" in dir(a))
    
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)
    a.kdpStdThreshold = 1
    self.assertAlmostEqual(1.0, a.kdpStdThreshold, 3);

  def testBB(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("BB" in dir(a))
    
    self.assertAlmostEqual(0.7987, a.BB, 3)
    a.BB = 1
    self.assertAlmostEqual(1.0, a.BB, 3);

  def testThresholdPhidp(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("thresholdPhidp" in dir(a))
    
    self.assertAlmostEqual(40.0, a.thresholdPhidp, 3)
    a.thresholdPhidp = 1
    self.assertAlmostEqual(1.0, a.thresholdPhidp, 3);
      
  def testRequestedFields(self):
    a = _ppcradaroptions.new()
        
    self.assertTrue("requestedFields" in dir(a))

    self.assertEqual(_ppcradaroptions.P_DBZH_CORR |_ppcradaroptions.P_ATT_DBZH_CORR | _ppcradaroptions.P_PHIDP_CORR | _ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK, a.requestedFields);
    
    a.requestedFields = _ppcradaroptions.P_KDP_CORR | _ppcradaroptions.P_PHIDP_CORR | _ppcradaroptions.P_ZDR_CORR

    self.assertEqual(_ppcradaroptions.P_KDP_CORR | _ppcradaroptions.P_PHIDP_CORR | _ppcradaroptions.P_ZDR_CORR, a.requestedFields);

    self.assertNotEqual(_ppcradaroptions.P_KDP_CORR | _ppcradaroptions.P_PHIDP_CORR | _ppcradaroptions.P_ZDR_CORR | _ppcradaroptions.P_ZPHI_CORR, a.requestedFields)
        
    a.requestedFields = a.requestedFields | _ppcradaroptions.P_ZPHI_CORR

    self.assertEqual(_ppcradaroptions.P_KDP_CORR | _ppcradaroptions.P_PHIDP_CORR | _ppcradaroptions.P_ZDR_CORR | _ppcradaroptions.P_ZPHI_CORR, a.requestedFields);

  def testAvailableResultSelectors(self):
    self.assertEqual(1, _ppcradaroptions.P_TH_CORR)
    self.assertEqual(1<<1, _ppcradaroptions.P_ATT_TH_CORR)
    self.assertEqual(1<<2, _ppcradaroptions.P_DBZH_CORR)
    self.assertEqual(1<<3, _ppcradaroptions.P_ATT_DBZH_CORR)
    self.assertEqual(1<<4, _ppcradaroptions.P_KDP_CORR)
    self.assertEqual(1<<5, _ppcradaroptions.P_RHOHV_CORR)
    self.assertEqual(1<<6, _ppcradaroptions.P_PHIDP_CORR)
    self.assertEqual(1<<7, _ppcradaroptions.P_ZDR_CORR)
    self.assertEqual(1<<8, _ppcradaroptions.P_ZPHI_CORR)
    self.assertEqual(1<<9, _ppcradaroptions.Q_RESIDUAL_CLUTTER_MASK)
    self.assertEqual(1<<10, _ppcradaroptions.Q_ATTENUATION_MASK)
    self.assertEqual(1<<11, _ppcradaroptions.Q_ATTENUATION)

  def testResidualMinZClutterThreshold(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualMinZClutterThreshold" in dir(a))
    
    self.assertAlmostEqual(-31.5, a.residualMinZClutterThreshold, 3)
    a.residualMinZClutterThreshold = 1
    self.assertAlmostEqual(1.0, a.residualMinZClutterThreshold, 3);

  def testResidualThresholdZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualThresholdZ" in dir(a))
    
    self.assertAlmostEqual(-20.0, a.residualThresholdZ, 3)
    a.residualThresholdZ = 1
    self.assertAlmostEqual(1.0, a.residualThresholdZ, 3);

  def testResidualThresholdTexture(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualThresholdTexture" in dir(a))
    
    self.assertAlmostEqual(20.0, a.residualThresholdTexture, 3)
    a.residualThresholdTexture = 1
    self.assertAlmostEqual(1.0, a.residualThresholdTexture, 3);

  def testResidualClutterNodata(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualClutterNodata" in dir(a))
    
    self.assertAlmostEqual(-999.0, a.residualClutterNodata, 3)
    a.residualClutterNodata = 1
    self.assertAlmostEqual(1.0, a.residualClutterNodata, 3);

  def testResidualClutterMaskNodata(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualClutterMaskNodata" in dir(a))
    
    self.assertAlmostEqual(-1.0, a.residualClutterMaskNodata, 3)
    a.residualClutterMaskNodata = 1
    self.assertAlmostEqual(1.0, a.residualClutterMaskNodata, 3);

  def testResidualClutterTextureFilteringMaxZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualClutterTextureFilteringMaxZ" in dir(a))
    
    self.assertAlmostEqual(70.0, a.residualClutterTextureFilteringMaxZ, 3)
    a.residualClutterTextureFilteringMaxZ = 1
    self.assertAlmostEqual(1.0, a.residualClutterTextureFilteringMaxZ, 3);

  def testResidualFilterBinSize(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualFilterBinSize" in dir(a))
    
    self.assertEqual(1, a.residualFilterBinSize)
    a.residualFilterBinSize = 2
    self.assertEqual(2, a.residualFilterBinSize);

  def testResidualFilterRaySize(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("residualFilterRaySize" in dir(a))
    
    self.assertEqual(1, a.residualFilterRaySize)
    a.residualFilterRaySize = 2
    self.assertEqual(2, a.residualFilterRaySize);

  def testMinAttenuationMaskRHOHV(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minAttenuationMaskRHOHV" in dir(a))
    
    self.assertAlmostEqual(0.8, a.minAttenuationMaskRHOHV, 3)
    a.minAttenuationMaskRHOHV = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskRHOHV, 3);

  def testMinAttenuationMaskKDP(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minAttenuationMaskKDP" in dir(a))
    
    self.assertAlmostEqual(0.001, a.minAttenuationMaskKDP, 3)
    a.minAttenuationMaskKDP = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskKDP, 3);

  def testMinAttenuationMaskTH(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("minAttenuationMaskTH" in dir(a))
    
    self.assertAlmostEqual(-20.0, a.minAttenuationMaskTH, 3)
    a.minAttenuationMaskTH = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskTH, 3);

  def testAttenuationGammaH(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("attenuationGammaH" in dir(a))
    
    self.assertAlmostEqual(0.08, a.attenuationGammaH, 3)
    a.attenuationGammaH = 1
    self.assertAlmostEqual(1.0, a.attenuationGammaH, 3);

  def testAttenuationAlpha(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("attenuationAlpha" in dir(a))
    
    self.assertAlmostEqual(0.2, a.attenuationAlpha, 3)
    a.attenuationAlpha = 1
    self.assertAlmostEqual(1.0, a.attenuationAlpha, 3);

  def testAttenuationPIAminZ(self):
    a = _ppcradaroptions.new()
    
    self.assertTrue("attenuationPIAminZ" in dir(a))
    
    self.assertAlmostEqual(-30.0, a.attenuationPIAminZ, 3)
    a.attenuationPIAminZ = 1
    self.assertAlmostEqual(1.0, a.attenuationPIAminZ, 3);
    
if __name__ == "__main__":
  #import sys;sys.argv = ['', 'Test.testName']
  unittest.main()