'''
Created on Mar 30, 2019

@author: anders
'''
import unittest

import _raveio
import os, string
import numpy
import _rave
import _ravefield
import _ravedata2d
import _pdpprocessor

_rave.setDebugLevel(_rave.Debug_RAVE_SPEWDEBUG)

class PyPdpProcessorTest(unittest.TestCase):
  PVOL_TESTFILE="fixtures/sevax_qcvol_pn129_20170816T000000Z_0x73fc7b.h5"
  
  TEMPORARY_FILE="ppctest_file1.h5"
  TEMPORARY_FILE2="ropotest_file2.h5"
  
  def setUp(self):
    if os.path.isfile(self.TEMPORARY_FILE):
      os.unlink(self.TEMPORARY_FILE)
    if os.path.isfile(self.TEMPORARY_FILE2):
      os.unlink(self.TEMPORARY_FILE2)

  def tearDown(self):
    if os.path.isfile(self.TEMPORARY_FILE):
      os.unlink(self.TEMPORARY_FILE)
    if os.path.isfile(self.TEMPORARY_FILE2):
     os.unlink(self.TEMPORARY_FILE2)  

  def testNew(self):
    a = _pdpprocessor.new()
    self.assertNotEqual(-1, str(type(a)).find("PdpProcessor"))

  def testMinWindow(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minWindow" in dir(a))
    
    self.assertEqual(11, a.minWindow)
    
    a.minWindow = 1
    
    self.assertEqual(1, a.minWindow)
  
  def testPdpRWin1(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("pdpRWin1" in dir(a))
    
    self.assertAlmostEqual(3.5, a.pdpRWin1, 3)
    a.pdpRWin1 = 1
    self.assertAlmostEqual(1.0, a.pdpRWin1, 3);

  def testpdpRWin2(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("pdpRWin2" in dir(a))
    
    self.assertAlmostEqual(1.5, a.pdpRWin2, 3)
    a.pdpRWin2 = 1
    self.assertAlmostEqual(1.0, a.pdpRWin2, 3);

  def testPdpNrIterations(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("pdpNrIterations" in dir(a))
    
    self.assertEqual(2, a.pdpNrIterations)
    a.pdpNrIterations = 1
    self.assertEqual(1.0, a.pdpNrIterations);

  def testMinZMedfilterThreshold(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minZMedfilterThreshold" in dir(a))
    
    self.assertAlmostEqual(-30.0, a.minZMedfilterThreshold, 3)
    a.minZMedfilterThreshold = 1
    self.assertAlmostEqual(1.0, a.minZMedfilterThreshold, 3);

  def testProcessingTextureThreshold(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("processingTextureThreshold" in dir(a))
    
    self.assertAlmostEqual(10.0, a.processingTextureThreshold, 3)
    a.processingTextureThreshold = 1
    self.assertAlmostEqual(1.0, a.processingTextureThreshold, 3);

  def testParametersUZ(self):
    a = _pdpprocessor.new()
    
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
    a = _pdpprocessor.new()

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
    a = _pdpprocessor.new()
    
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
    a = _pdpprocessor.new()

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
    a = _pdpprocessor.new()
    
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
    a = _pdpprocessor.new()
    
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
    a = _pdpprocessor.new()
    
    self.assertTrue("nodata" in dir(a))
    
    self.assertAlmostEqual(-999.0, a.nodata, 3)
    a.nodata = 1
    self.assertAlmostEqual(1.0, a.nodata, 3);

  def testMinDBZ(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minDBZ" in dir(a))
    
    self.assertAlmostEqual(-32.0, a.minDBZ, 3)
    a.minDBZ = 1
    self.assertAlmostEqual(1.0, a.minDBZ, 3);

  def testQualityThreshold(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("qualityThreshold" in dir(a))
    
    self.assertAlmostEqual(0.75, a.qualityThreshold, 3)
    a.qualityThreshold = 1
    self.assertAlmostEqual(1.0, a.qualityThreshold, 3);
    
  def testKdpUp(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("kdpUp" in dir(a))
    
    self.assertAlmostEqual(20.0, a.kdpUp, 3)
    a.kdpUp = 1
    self.assertAlmostEqual(1.0, a.kdpUp, 3);
    
  def testKdpDown(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("kdpDown" in dir(a))
    
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)
    a.kdpDown = 1
    self.assertAlmostEqual(1.0, a.kdpDown, 3);

  def testKdpStdThreshold(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("kdpStdThreshold" in dir(a))
    
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)
    a.kdpStdThreshold = 1
    self.assertAlmostEqual(1.0, a.kdpStdThreshold, 3);

  def testBB(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("BB" in dir(a))
    
    self.assertAlmostEqual(0.7987, a.BB, 3)
    a.BB = 1
    self.assertAlmostEqual(1.0, a.BB, 3);

  def testThresholdPhidp(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("thresholdPhidp" in dir(a))
    
    self.assertAlmostEqual(40.0, a.thresholdPhidp, 3)
    a.thresholdPhidp = 1
    self.assertAlmostEqual(1.0, a.thresholdPhidp, 3);

  def testSetBand(self):
    a = _pdpprocessor.new()
    a.setBand('s')
    self.assertAlmostEqual(14.0, a.kdpUp, 3)
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)
    
    a.setBand('c')
    self.assertAlmostEqual(20.0, a.kdpUp, 3)
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)
        
    a.setBand('x')
    self.assertAlmostEqual(40.0, a.kdpUp, 3)
    self.assertAlmostEqual(-2.0, a.kdpDown, 3)
    self.assertAlmostEqual(5.0, a.kdpStdThreshold, 3)

  def testSetBand_invalid(self):
    a = _pdpprocessor.new()
    try:
      a.setBand('y')
      self.fail("Expected ValueError")
    except ValueError:
      pass
      
  def testRequestedFields(self):
    a = _pdpprocessor.new()
        
    self.assertTrue("requestedFields" in dir(a))

    self.assertEqual(_pdpprocessor.P_CORR_ATT_TH | _pdpprocessor.P_CORR_PHIDP | _pdpprocessor.Q_QUALITY_RESIDUAL_CLUTTER_MASK, a.requestedFields);
    
    a.requestedFields = _pdpprocessor.P_CORR_KDP | _pdpprocessor.P_CORR_PHIDP | _pdpprocessor.P_CORR_ZDR

    self.assertEqual(_pdpprocessor.P_CORR_KDP | _pdpprocessor.P_CORR_PHIDP | _pdpprocessor.P_CORR_ZDR, a.requestedFields);

    self.assertNotEqual(_pdpprocessor.P_CORR_KDP | _pdpprocessor.P_CORR_PHIDP | _pdpprocessor.P_CORR_ZDR | _pdpprocessor.P_CORR_ZPHI, a.requestedFields)
        
    a.requestedFields = a.requestedFields | _pdpprocessor.P_CORR_ZPHI

    self.assertEqual(_pdpprocessor.P_CORR_KDP | _pdpprocessor.P_CORR_PHIDP | _pdpprocessor.P_CORR_ZDR | _pdpprocessor.P_CORR_ZPHI, a.requestedFields);

  def testResidualMinZClutterThreshold(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualMinZClutterThreshold" in dir(a))
    
    self.assertAlmostEqual(-31.5, a.residualMinZClutterThreshold, 3)
    a.residualMinZClutterThreshold = 1
    self.assertAlmostEqual(1.0, a.residualMinZClutterThreshold, 3);

  def testResidualThresholdZ(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualThresholdZ" in dir(a))
    
    self.assertAlmostEqual(-20.0, a.residualThresholdZ, 3)
    a.residualThresholdZ = 1
    self.assertAlmostEqual(1.0, a.residualThresholdZ, 3);

  def testResidualThresholdTexture(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualThresholdTexture" in dir(a))
    
    self.assertAlmostEqual(20.0, a.residualThresholdTexture, 3)
    a.residualThresholdTexture = 1
    self.assertAlmostEqual(1.0, a.residualThresholdTexture, 3);

  def testResidualClutterNodata(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualClutterNodata" in dir(a))
    
    self.assertAlmostEqual(-999.0, a.residualClutterNodata, 3)
    a.residualClutterNodata = 1
    self.assertAlmostEqual(1.0, a.residualClutterNodata, 3);

  def testResidualClutterMaskNodata(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualClutterMaskNodata" in dir(a))
    
    self.assertAlmostEqual(-1.0, a.residualClutterMaskNodata, 3)
    a.residualClutterMaskNodata = 1
    self.assertAlmostEqual(1.0, a.residualClutterMaskNodata, 3);

  def testResidualClutterTextureFilteringMaxZ(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualClutterTextureFilteringMaxZ" in dir(a))
    
    self.assertAlmostEqual(70.0, a.residualClutterTextureFilteringMaxZ, 3)
    a.residualClutterTextureFilteringMaxZ = 1
    self.assertAlmostEqual(1.0, a.residualClutterTextureFilteringMaxZ, 3);

  def testResidualFilterBinSize(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualFilterBinSize" in dir(a))
    
    self.assertEqual(1, a.residualFilterBinSize)
    a.residualFilterBinSize = 2
    self.assertEqual(2, a.residualFilterBinSize);

  def testResidualFilterRaySize(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("residualFilterRaySize" in dir(a))
    
    self.assertEqual(1, a.residualFilterRaySize)
    a.residualFilterRaySize = 2
    self.assertEqual(2, a.residualFilterRaySize);

  def testMinAttenuationMaskRHOHV(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minAttenuationMaskRHOHV" in dir(a))
    
    self.assertAlmostEqual(0.8, a.minAttenuationMaskRHOHV, 3)
    a.minAttenuationMaskRHOHV = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskRHOHV, 3);

  def testMinAttenuationMaskKDP(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minAttenuationMaskKDP" in dir(a))
    
    self.assertAlmostEqual(0.001, a.minAttenuationMaskKDP, 3)
    a.minAttenuationMaskKDP = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskKDP, 3);

  def testMinAttenuationMaskTH(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("minAttenuationMaskTH" in dir(a))
    
    self.assertAlmostEqual(-20.0, a.minAttenuationMaskTH, 3)
    a.minAttenuationMaskTH = 1
    self.assertAlmostEqual(1.0, a.minAttenuationMaskTH, 3);

  def testAttenuationGammaH(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("attenuationGammaH" in dir(a))
    
    self.assertAlmostEqual(0.08, a.attenuationGammaH, 3)
    a.attenuationGammaH = 1
    self.assertAlmostEqual(1.0, a.attenuationGammaH, 3);

  def testAttenuationAlpha(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("attenuationAlpha" in dir(a))
    
    self.assertAlmostEqual(0.2, a.attenuationAlpha, 3)
    a.attenuationAlpha = 1
    self.assertAlmostEqual(1.0, a.attenuationAlpha, 3);

  def testAttenuationPIAminZ(self):
    a = _pdpprocessor.new()
    
    self.assertTrue("attenuationPIAminZ" in dir(a))
    
    self.assertAlmostEqual(-30.0, a.attenuationPIAminZ, 3)
    a.attenuationPIAminZ = 1
    self.assertAlmostEqual(1.0, a.attenuationPIAminZ, 3);

  def test_texture(self):
    processor = _pdpprocessor.new()
    data2d = _ravedata2d.new()
    data2d.setData(numpy.array([[1.0, 2.0, 3.0, 4.0],
                             [5.0, 6.0, 7.0, 8.0],
                             [8.0, 7.0, 6.0, 5.0],
                             [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    data2d.useNodata = True
    data2d.nodata = -999.0

    result = processor.texture(data2d)
    # Expected result retrieved from test run of the matlab code
    expected = numpy.array([
      [1.32877,   0.94373,   0.94373,  0.87500],
      [0.87500,   0.94373,   0.94373,  1.32877],
      [1.32877,   0.94373,   0.94373,  0.87500],
      [0.87500,   0.94373,   0.94373,  1.32877]], numpy.float64)
    
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def test_texture_2(self):
    processor = _pdpprocessor.new()
    data2d = _ravedata2d.new()
    numpy.set_printoptions(precision=3)
    numpy.set_printoptions(suppress=True)  
      
    data2d.setData(numpy.array([[-999.0, 2.0, 3.0, -999.0],
                             [5.0, 6.0, 7.0, 8.0],
                             [8.0, 7.0, 6.0, 5.0],
                             [-999.0, 3.0, 2.0, -999.0]], numpy.float64))

    data2d.useNodata=True
    data2d.nodata=-999;
    result = processor.texture(data2d)
    # Expected result retrieved from test run of the matlab code
    expected = numpy.array([
      [-999.00000,     1.20185,     1.20185,  -999.00000],
      [0.94281,     0.80812,     0.98974,     1.15470],
      [1.15470,     0.98974,     0.80812,     0.94281],
      [-999.00000,     1.20185,     1.20185,  -999.00000]], numpy.float64)
    
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)
        
  def test_texture_3(self):
    processor = _pdpprocessor.new()
    data2d = _ravedata2d.new()
    #numpy.set_printoptions(precision=3)
    #numpy.set_printoptions(suppress=True)  
    data2d.setData(numpy.array([[-999, 2.0, 3.0, 4.0, -999],
                             [5.0, 6.0, 6.0, 7.0, 8.0],
                             [8.0, 7.0, 7.0, 6.0, 5.0],
                             [-999, 3.0, 1.0, 2.0, -999]], numpy.float64))
    data2d.useNodata=True
    data2d.nodata=-999;
    result = processor.texture(data2d)
    
    
    expected = numpy.array([
      [-999, 1.106, 0.8, 1.093, -999],
      [0.943, 0.808, 0.707, 0.808, 1.041],
      [1.155, 1.097, 1.118, 0.990, 0.943],
      [-999, 1.312, 1.346, 1.247, -999]], numpy.float64)

    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)        

  def test_texture_vol_PHIDP(self):
    a=_raveio.open(self.PVOL_TESTFILE)
    processor = _pdpprocessor.new()
    par = a.object.getScan(0).getParameter("PHIDP")
    data = par.getData2D().emul(par.gain).add(par.offset).emul(-1.0)
    result = processor.texture(data)
    expected = numpy.array([14.2319, 3.0310, 0.6113, 0.6835, 0.8277, 0.8824,
                            0.9338, 0.7487, 0.7276, 0.6603, 0.4323, 0.8645,
                            0.9170, 0.2496, 0.5853, 0.3057, 0.3529, 0.3057,
                            0.3057, 0.4991], numpy.float64);
    for i in range(expected.shape[0]):
      self.assertAlmostEqual(result.getData()[20,i], expected[i], 3)  

  def test_texture_vol_TH(self):
    a=_raveio.open(self.PVOL_TESTFILE)
    processor = _pdpprocessor.new()
    par = a.object.getScan(0).getParameter("TH")
    data = par.getData2D().emul(par.gain).add(par.offset)
    result = processor.texture(data)
    #print(result.getData()[20,0:20])
    expected = numpy.array([18.3395, 6.0493, 0.5762, 0.7262, 1.4470,
                            1.5424, 1.0861, 1.3905, 0.7552, 0.9763,
                            0.9763, 1.5130, 1.0232, 1.0251, 1.0662,
                            1.5194, 1.9425, 1.7689, 1.5811, 0.8339], numpy.float64)
    for i in range(expected.shape[0]):
      self.assertAlmostEqual(result.getData()[20,i], expected[i], 3)  

  def test_trap(self):
    processor = _pdpprocessor.new()
    data2d = _ravedata2d.new()
    data2d.setData(numpy.array([[1.0, 2.0, 3.0, 4.0],
                             [5.0, 6.0, 7.0, 8.0],
                             [8.0, 7.0, 6.0, 5.0],
                             [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    result = processor.trap(data2d, 1.0, 2.0, 3.0, 4.0)
    expected = numpy.array([
      [1.00000,   1.00000,   0.75000,   0.50000],
      [0.25000,   0.00000,   0.00000,   0.00000],
      [0.00000,   0.00000,   0.00000,   0.25000],
      [0.50000,   0.75000,   1.00000,   1.00000]], numpy.float64)

    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def test_trap_2(self):
    processor = _pdpprocessor.new()
    data2d = _ravedata2d.new()
    data2d.setData(numpy.array([[1.0, 2.0, 3.0, 4.0],
                             [5.0, 6.0, 7.0, 8.0],
                             [8.0, 7.0, 6.0, 5.0],
                             [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    result = processor.trap(data2d, 1.0, 2.0, 3.0, 0.0)
    expected = numpy.array([
      [1.00000,   1.00000,   0.00000,   0.00000],
      [0.00000,   0.00000,   0.00000,   0.00000],
      [0.00000,   0.00000,   0.00000,   0.00000],
      [0.00000,   0.00000,   1.00000,   1.00000]], numpy.float64)

    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def test_clutterID_1(self):
    processor = _pdpprocessor.new()
    
    Z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                      [5.0, 6.0, 7.0, 8.0],
                                      [8.0, 7.0, 6.0, 5.0],
                                      [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    VRADH = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                         [5.0, 6.0, 7.0, 8.0],
                                         [8.0, 7.0, 6.0, 5.0],
                                         [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    texturePHIDP = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                                [5.0, 6.0, 7.0, 8.0],
                                                [8.0, 7.0, 6.0, 5.0],
                                                [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    RHOHV = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                         [5.0, 6.0, 7.0, 8.0],
                                         [8.0, 7.0, 6.0, 5.0],
                                         [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    textureZ = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                            [5.0, 6.0, 7.0, 8.0],
                                            [8.0, 7.0, 6.0, 5.0],
                                            [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    clutterMap = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                              [5.0, 6.0, 7.0, 8.0],
                                              [8.0, 7.0, 6.0, 5.0],
                                              [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    result = processor.clutterID(Z, VRADH, texturePHIDP, RHOHV, textureZ, clutterMap, -9999.0, -9999.0)
    expected = numpy.array([ # Expected result is same as produced by matlab code
      [0.32800,   0.30600,   0.32400,   0.34200],
      [0.36000,   0.36000,   0.36000,   0.36000], 
      [0.36000,   0.36000,   0.36000,   0.36000],
      [0.34200,   0.32400,   0.30600,   0.32800]], numpy.float64)
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def test_clutterCorrection_1(self):
    processor = _pdpprocessor.new()
    
    Z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, -33.0],
                                      [5.0, 6.0, 7.0, 8.0],
                                      [8.0, 7.0, 6.0, 5.0],
                                      [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    VRADH = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                         [5.0, 6.0, 7.0, 8.0],
                                         [8.0, 7.0, 6.0, 5.0],
                                         [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    texturePHIDP = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                                [5.0, 6.0, 7.0, 8.0],
                                                [8.0, 7.0, 6.0, 5.0],
                                                [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    RHOHV = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                         [5.0, 6.0, 7.0, 8.0],
                                         [8.0, 7.0, 6.0, 5.0],
                                         [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    textureZ = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                            [5.0, 6.0, 7.0, 8.0],
                                            [8.0, 7.0, 6.0, 5.0],
                                            [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    
    clutterMap = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                              [5.0, 6.0, 7.0, 8.0],
                                              [8.0, 7.0, 6.0, 5.0],
                                              [4.0, 3.0, 2.0, 1.0]], numpy.float64))

    newZ, qualityField, clutterMask = processor.clutterCorrection(Z, VRADH, texturePHIDP, RHOHV, textureZ, clutterMap, -9999.0, -9999.0, 0.65)
    
    expectedZ = numpy.array([
      [    1.0,     2.0,      3.0,     -33.0],
      [-9999.0, -9999.0,  -9999.0,   -9999.0],
      [-9999.0, -9999.0,  -9999.0,   -9999.0],
      [    4.0,     3.0,      2.0,       1.0]])
    
    expectedQ = numpy.array([
      [0.67200,   0.69400,   0.67600,   0.65800],
      [0.64000,   0.64000,   0.64000,   0.64000],
      [0.64000,   0.64000,   0.64000,   0.64000],
      [0.65800,   0.67600,   0.69400,   0.67200]])
    
    expectedMask = numpy.array([
      [0,   0,   0,   0],
      [1,   1,   1,   1],
      [1,   1,   1,   1],
      [0,   0,   0,   0]])

    
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(newZ.getData()[i,j], expectedZ[i,j], 3)
        self.assertAlmostEqual(qualityField.getData()[i,j], expectedQ[i,j], 3)
        self.assertAlmostEqual(clutterMask.getData()[i,j], expectedMask[i,j], 3)


  def test_medfilt_1(self):
    processor = _pdpprocessor.new()
    Z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, -33.0],
                                      [5.0, 6.0, 7.0, 8.0],
                                      [8.0, 7.0, 6.0, 5.0],
                                      [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    result = processor.medfilt(Z, -20.0, -999, (3, 3))
    
    expected = numpy.array([
      [-33,    2,    3,  -33],
      [5,    6,    7,    8],
      [8,    7,    6,    5],
      [-33,    3,    2,  -33]], numpy.float64)
    
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def test_residualClutterFilter_1(self):
    processor = _pdpprocessor.new()
    Z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, -33.0],
                                      [5.0, 6.0, 7.0, 8.0],
                                      [8.0, 7.0, 6.0, 5.0],
                                      [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    Z.nodata = -999
    Z.useNodata = True
    result = processor.residualClutterFilter(Z, -20.0, 20.0, (3, 3))
    #print(str(result.getData()))
    
    expected = numpy.array([
      [-1,    1,    1,  -1], 
      [ 1,    1,    1,   1],
      [ 1,    1,    1,   1],
      [-1,    1,    1,  -1]], numpy.float64)
    
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(result.getData()[i,j], expected[i,j], 3)

  def testPdpProcessing_1(self):
    processor = _pdpprocessor.new()
    pdp = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                       [5.0, 6.0, 7.0, 8.0],
                                       [8.0, 7.0, 6.0, 5.0],
                                       [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    pdp.nodata = -999
    pdp.useNodata = True
    
    pdpf, kdpf = processor.pdpProcessing(pdp, 1.0, 1, 2)

    expected_pdpf = numpy.array([
      [0.0,  0.7500,  1.0000,  1.0000],
      [0.0,  0.7500,  1.0000,  1.0000],
      [0.0, -0.7500, -1.0000, -1.0000],
      [0.0, -0.7500, -1.0000, -1.0000]], numpy.float64)

    expected_kdpf = numpy.array([
      [0.0, 0.3750, 0.1250, 0.0],
      [0.0, 0.3750, 0.1250, 0.0],
      [0.0,-0.3750,-0.1250, 0.0],
      [0.0,-0.3750,-0.1250, 0.0]], numpy.float64)
   
    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(pdpf.getData()[i,j], expected_pdpf[i,j], 3)
        self.assertAlmostEqual(kdpf.getData()[i,j], expected_kdpf[i,j], 3)

  def testPdpScript_1(self):
    processor = _pdpprocessor.new()
    pdp = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                       [5.0, 6.0, 7.0, 8.0],
                                       [8.0, 7.0, 6.0, 5.0],
                                       [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    pdp.nodata = -999
    pdp.useNodata = True
    processor.minWindow = 1
    
    pdpf, kdpf = processor.pdpScript(pdp, 1.0, 1.4, 1.0, 2)

    expected_pdpf = numpy.array([
      [0.0,  0.7500,  1.0000,  1.0000],
      [0.0,  0.7500,  1.0000,  1.0000],
      [0.0, -0.7500, -1.0000, -1.0000],
      [0.0, -0.7500, -1.0000, -1.0000]], numpy.float64)

    expected_kdpf = numpy.array([
      [0.0, 0.3750, 0.1250, 0.0],
      [0.0, 0.3750, 0.1250, 0.0],
      [0.0,-0.3750,-0.1250, 0.0],
      [0.0,-0.3750,-0.1250, 0.0]], numpy.float64)
    
    for i in range(4): 
      for j in range(4):
        self.assertAlmostEqual(pdpf.getData()[i,j], expected_pdpf[i,j], 3)
        self.assertAlmostEqual(kdpf.getData()[i,j], expected_kdpf[i,j], 3)  

  def testAttenuation(self):
    processor = _pdpprocessor.new()
    z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                     [5.0, 6.0, 7.0, 8.0],
                                     [8.0, 7.0, 6.0, 5.0],
                                     [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    zdr = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                       [5.0, 6.0, 7.0, 8.0],
                                       [8.0, 7.0, 6.0, 5.0],
                                         [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    pdp = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                       [5.0, 6.0, 7.0, 8.0],
                                       [8.0, 7.0, 6.0, 5.0],
                                       [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    mask = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 0.0],
                                       [5.0, 6.0, 7.0, 0.0],
                                       [8.0, 7.0, 6.0, 0.0],
                                       [4.0, 3.0, 2.0, 0.0]], numpy.float64))
    pdp.nodata = -999
    pdp.useNodata = True
    
    zres, zdrres, piares = processor.attenuation(z, zdr, pdp, mask, 1.0, 2.0)
    #print(str(zres.getData()))
    #print(str(zdrres.getData()))
    #print(str(piares.getData()))
    
    expected_z = numpy.array([
      [1,    3,    5,    6],
      [5,    7,    9,   10],
      [8,    7,    6,    5],
      [4,    3,    2,    1]], numpy.float64)
    expected_zdr = numpy.array([
      [1,    4,    7,    8],
      [5,    8,   11,   12],
      [8,    7,    6,    5],
      [4,    3,    2,    1]], numpy.float64)
    expected_pia = numpy.array([
      [0,   1,   2,   2],
      [0,   1,   2,   2],
      [0,  -1,  -2,  -2],
      [0,  -1,  -2,  -2]], numpy.float64)

    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(zres.getData()[i,j], expected_z[i,j], 3)
        self.assertAlmostEqual(zdrres.getData()[i,j], expected_zdr[i,j], 3)  
        self.assertAlmostEqual(piares.getData()[i,j], expected_pia[i,j], 3)  
     
  def testZphi(self):
    processor = _pdpprocessor.new()
    z = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                     [5.0, 6.0, 7.0, 8.0],
                                     [8.0, 7.0, 6.0, 5.0],
                                     [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    pdp = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
                                       [5.0, 6.0, 7.0, 8.0],
                                       [8.0, 7.0, 6.0, 5.0],
                                       [4.0, 3.0, 2.0, 1.0]], numpy.float64))
    mask = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 0.0],
                                       [5.0, 6.0, 7.0, 0.0],
                                       [8.0, 7.0, 6.0, 0.0],
                                       [4.0, 3.0, 2.0, 0.0]], numpy.float64))
    pdp.nodata = -999
    pdp.useNodata = True
    z.nodata=255
    z.useNodata = True
    zphi, ah = processor.zphi(z, pdp, mask, 1.0, 2.0, 3.0)
    #print(str(zphi.getData()))
    #print(str(ah.getData()))
    #print(str(piares.getData()))
    expected_zphi = numpy.array([
      [1.48962,    3.69637,   20.60532,   21.60532],
      [5.48962,    7.69637,   24.60532,   25.60532],
      [6.08743,    4.31144,    2.91183,    1.91183],
      [2.08743,    0.31144,   -1.08817,   -2.08817]], numpy.float64)
    expected_ah = numpy.array([
      [0.24481,   0.60337,   7.95448,   0.00000],
      [0.24481,   0.60337,   7.95448,   0.00000],
      [-0.95628,  -0.38800,  -0.19981,   0.00000],
      [-0.95628,  -0.38800,  -0.19981,   0.00000]], numpy.float64)

    for i in range(4):
      for j in range(4):
        self.assertAlmostEqual(zphi.getData()[i,j], expected_zphi[i,j], 3)
        self.assertAlmostEqual(ah.getData()[i,j], expected_ah[i,j], 3)  

  #def test_process_CORR_KDP_CORR_ZPHI(self):
  #  a=_raveio.open(self.PVOL_TESTFILE)
  #  processor = _pdpprocessor.new()
  #  result = processor.process(a.object.getScan(0))
  #  b = _raveio.new()
  #  b.object = result
  #  b.save("slask.h5")

  def test_process_CORR_KDP_CORR_ZPHI(self):
    a=_raveio.open(self.PVOL_TESTFILE)
    processor = _pdpprocessor.new()
    processor.requestedFields = _pdpprocessor.P_CORR_KDP | _pdpprocessor.P_CORR_ZPHI  
    result = processor.process(a.object.getScan(0))
    self.assertFalse(result.hasParameter("CORR_TH"))
    self.assertTrue(result.hasParameter("CORR_KDP"))
    self.assertTrue(result.hasParameter("CORR_ZPHI"))

  def test_process_CORR_TH_CORR_ZPHI(self):
    a=_raveio.open(self.PVOL_TESTFILE)
    processor = _pdpprocessor.new()
    processor.requestedFields = _pdpprocessor.P_CORR_TH | _pdpprocessor.P_CORR_ZPHI  
    result = processor.process(a.object.getScan(0))
    self.assertTrue(result.hasParameter("CORR_TH"))
    self.assertFalse(result.hasParameter("CORR_KDP"))
    self.assertTrue(result.hasParameter("CORR_ZPHI"))
    
    
if __name__ == "__main__":
  #import sys;sys.argv = ['', 'Test.testName']
  unittest.main()