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
import _ppcradaroptions

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

  def test_options(self):
    a = _pdpprocessor.new()
    self.assertTrue("options" in dir(a))
    self.assertNotEqual(-1, str(type(a.options)).find("PpcRadarOptions"))
    
    a.options.minWindow = 99
    self.assertEqual(99, a.options.minWindow)

    a.options = _ppcradaroptions.new()
    self.assertEqual(11, a.options.minWindow)

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
    processor.options.minWindow = 1
    
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
    dbzh = _ravedata2d.new(numpy.array([[1.0, 2.0, 3.0, 4.0],
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
    dbzh.nodata = -999
    dbzh.useNodata = True
    
    zres, zdrres, piares, dbzhres = processor.attenuation(z, zdr, dbzh, pdp, mask, 1.0, 2.0)
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
    expected_dbzh = numpy.array([
      [1,    3,    5,    6],
      [5,    7,    9,   10],
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
        self.assertAlmostEqual(dbzhres.getData()[i,j], expected_dbzh[i,j], 3)  
     
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
    processor.options.requestedFields = _ppcradaroptions.P_KDP_CORR | _ppcradaroptions.P_ZPHI_CORR  
    result = processor.process(a.object.getScan(0))
    self.assertFalse(result.hasParameter("TH_CORR"))
    self.assertTrue(result.hasParameter("KDP_CORR"))
    self.assertTrue(result.hasParameter("ZPHI_CORR"))

  def test_process_CORR_TH_CORR_ZPHI(self):
    a=_raveio.open(self.PVOL_TESTFILE)
    processor = _pdpprocessor.new()
    processor.options.requestedFields = _ppcradaroptions.P_TH_CORR | _ppcradaroptions.P_ZPHI_CORR  
    result = processor.process(a.object.getScan(0))
    self.assertTrue(result.hasParameter("TH_CORR"))
    self.assertFalse(result.hasParameter("KDP_CORR"))
    self.assertTrue(result.hasParameter("ZPHI_CORR"))
    
    
if __name__ == "__main__":
  #import sys;sys.argv = ['', 'Test.testName']
  unittest.main()