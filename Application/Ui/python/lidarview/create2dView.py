import paraview.simple as smp

renderingWindowLayout = None
pointcloud = None
pointcloudView = None
imageView = None
trailingFrame = None

def initAsensingRendering():
    global renderingWindowLayout
    global pointcloud
    global pointcloudView
    global imageView
    global trailingFrame
    # Create a new horizontal layout
    if renderingWindowLayout is None:
        renderingWindowLayout = smp.GetLayout()
        renderingWindowLayout.SplitVertical(0, 0.70)
    # create a new render view
    if imageView is None:
        imageView = smp.CreateView("RenderView")
        imageView.AxesGrid = 'GridAxes3DActor'
        imageView.StereoType = 0
        imageView.Background = [0.0, 0.0, 0.0]
        renderingWindowLayout.AssignView(2, imageView)
        imageView.InteractionMode = '2D'
        imageView.OrientationAxesVisibility = 0 # Hide the tri-axe
        # BLock the rotation arount the normal of the image plane
        imageView.Camera2DManipulators = ['Pan', 'None', 'Zoom', 'Zoom', 'Zoom', 'ZoomToMouse', 'Roll', 'Pan', 'Rotate']
    # Create a lidar as image
    pointcloud = smp.FindSource('LidarReader1')
    LidarAsImage = smp.PointCloudLinearProjector(pointcloud)
    # show the image
    smp.SetActiveView(imageView)
    smp.SetActiveSource(LidarAsImage)
    lidarAsImageDisplay = smp.Show(LidarAsImage, imageView)
    # Hacky to update automatically lidar as image
    dataDisplay = smp.Show(pointcloud, imageView)
    smp.Hide(pointcloud, imageView)
    # Set color scale
    imageScalarsLUT = smp.GetColorTransferFunction('ImageScalars')
    #  imageScalarsLUT = smp.GetColorTransferFunction('LidarAsImage')
    imageScalarsLUT.RescaleTransferFunction(0.0, 255.0)
    imageScalarsLUT.ApplyPreset('Viridis (matplotlib)', True)
    # render
    smp.Render()

    # Set default pointcloud colorization to intensity
    smp.SetActiveView(pointcloudView)
    smp.SetActiveSource(trailingFrame)

    if trailingFrame is not None:
        pointcloudViewDisplay = smp.GetDisplayProperties(trailingFrame, view=pointcloudView)
        smp.ColorBy(pointcloudViewDisplay, ('POINTS', 'Intensity'))
        pointcloudViewDisplay.RescaleTransferFunctionToDataRange(True, False)
        pointcloudViewDisplay.SetScalarBarVisibility(pointcloudView, True)
        # render
        smp.Show(trailingFrame, pointcloudView)
        smp.Render()

def setAsensing3DDisplayPropertied(source):
    rep = smp.GetDisplayProperties(source)
    rep.RescaleTransferFunctionToDataRange(False, True)
    rep.ColorArrayName = 'Intensity'
    rep.LookupTable = smp.GetLookupTableForArray('Intensity', 1) # not needed ?
    intensityLUT = smp.GetColorTransferFunction('Intensity')
    # Ouster documentation do not indicate which range can be expected
    # so this range could be improved
    intensityLUT.RescaleTransferFunction(0.0, 1600.0)
    intensityLUT.ApplyPreset('Viridis (matplotlib)', True)

    intensityLUT = smp.GetColorTransferFunction('Intensity')
    # Ouster documentation do not indicate which range can be expected
    # so this range could be improved
    intensityLUT.RescaleTransferFunction(0.0, 1600.0)
    intensityLUT.ApplyPreset('Viridis (matplotlib)', True)

    reflectivityLUT = smp.GetColorTransferFunction('Reflectivity')
    # again, no indication of expected range in documentation
    reflectivityLUT.RescaleTransferFunction(0.0, 20000.0)
    reflectivityLUT.ApplyPreset('Viridis (matplotlib)', True)

    rangeLUT = smp.GetColorTransferFunction('Range')
    # tresholding distance to 50 meters
    rangeLUT.RescaleTransferFunction(0.0, 50000.0) # range is in millimeters
    rangeLUT.ApplyPreset('Viridis (matplotlib)', True)

def hasArrayName(sourceProxy, arrayName):
    '''
    Returns True if the data has non-zero points and has a point data
    attribute with the given arrayName.
    '''
    if not sourceProxy:
        return False

    info = sourceProxy.GetDataInformation().DataInformation

    if info.GetNumberOfPoints() == 0:
        return False

    info = info.GetAttributeInformation(0)
    for i in xrange(info.GetNumberOfArrays()):
        if info.GetArrayInformation(i).GetName() == arrayName:
            return True
    return False

def findPresetByName(name):
    presets = servermanager.vtkSMTransferFunctionPresets()

    numberOfPresets = presets.GetNumberOfPresets()

    for i in range(0,numberOfPresets):
        currentName = presets.GetPresetName(i)
        if currentName == name:
            return i

    return -1


def createDSRColorsPreset():

    dsrColorIndex = findPresetByName("DSR Colors")

    if dsrColorIndex == -1:
        rcolor = [0,        0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,
                  0,         0,         0,         0,         0,         0,         0,         0,         0,    0.0625,    0.1250,    0.1875,    0.2500,    0.3125,    0.3750,
                  0.4375,    0.5000,    0.5625,    0.6250,    0.6875,    0.7500,    0.8125,    0.8750,    0.9375,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,
                  1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000 ,   1.0000,    1.0000,    0.9375,    0.8750,    0.8125,    0.7500,
                  0.6875,    0.6250,    0.5625,    0.5000]

        gcolor = [0,         0,         0,         0,         0,         0 ,        0,         0,    0.0625,    0.1250,    0.1875,    0.2500,    0.3125,    0.3750,    0.4375,
                  0.5000,    0.5625,    0.6250,    0.6875,    0.7500,    0.8125,    0.8750,    0.9375,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,
                  1.0000,    1.0000,    1.0000,    1.0000,   1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    0.9375,    0.8750,    0.8125,    0.7500,    0.6875,
                  0.6250,    0.5625,    0.5000,    0.4375,    0.3750,    0.3125,    0.2500,    0.1875,    0.1250,    0.0625,         0,         0,         0,         0,         0,
                  0,         0,         0,         0]

        bcolor = [0.5625,    0.6250,    0.6875,    0.7500,    0.8125,    0.8750,    0.9375,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,
                  1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    1.0000,    0.9375,    0.8750,    0.8125,    0.7500,    0.6875,    0.6250,
                  0.5625,    0.5000,    0.4375,    0.3750,    0.3125,    0.2500,   0.1875,    0.1250,    0.0625,         0,         0,         0,         0,         0,         0,
                  0,         0 ,        0,         0,         0,         0,         0,         0,         0 ,        0 ,        0 ,        0,         0 ,        0 ,        0,
                  0,         0,         0,         0]

        intensityColor = [0] * 256

        for i in range(0,63):
            index = i/63.0*255.0

            intensityColor[i*4] = index
            intensityColor[i*4+1] = rcolor[i]
            intensityColor[i*4+2] = gcolor[i]
            intensityColor[i*4+3] = bcolor[i]
            i = i + 1

        presets = servermanager.vtkSMTransferFunctionPresets()

        intensityString = ',\n'.join(map(str, intensityColor))

        intensityJSON = "{\n\"ColorSpace\" : \"RGB\",\n\"Name\" : \"DSR\",\n\"NanColor\" : [ 1, 1, 0 ],\n\"RGBPoints\" : [\n"+ intensityString + "\n]\n}"

        presets.AddPreset("DSR Colors",intensityJSON)

def setDefaultLookupTables(sourceProxy):
    createDSRColorsPreset()

    presets = servermanager.vtkSMTransferFunctionPresets()

    dsrIndex = findPresetByName("DSR Colors")
    presetDSR = presets.GetPresetAsString(dsrIndex)

    # LUT for 'intensity'
    smp.GetLookupTableForArray(
      'Intensity', 1,
      ScalarRangeInitialized=1.0,
      ColorSpace='HSV',
      RGBPoints=[0.0, 0.0, 0.0, 1.0,
               100.0, 1.0, 1.0, 0.0,
               256.0, 1.0, 0.0, 0.0])

    # LUT for 'dual_distance'
    smp.GetLookupTableForArray(
      'dual_distance', 1,
      InterpretValuesAsCategories=True, NumberOfTableValues=3,
      RGBPoints=[-1.0, 0.1, 0.5, 0.7,
                  0.0, 0.9, 0.9, 0.9,
                 +1.0, 0.8, 0.2, 0.3],
      Annotations=['-1', 'near', '0', 'dual', '1', 'far'])

    # LUT for 'dual_intensity'
    smp.GetLookupTableForArray(
      'dual_intensity', 1,
      InterpretValuesAsCategories=True, NumberOfTableValues=3,
      RGBPoints=[-1.0, 0.5, 0.2, 0.8,
                  0.0, 0.6, 0.6, 0.6,
                 +1.0, 1.0, 0.9, 0.4],
      Annotations=['-1', 'low', '0', 'dual', '1', 'high'])

    # LUT for 'laser_id'. This LUT is extracted from the XML calibration file
    # which doesn't exist in live stream mode
    if False and getReader() is not None:
        rgbRaw = [0] * 256
        sourceProxy.GetClientSideObject().GetXMLColorTable(rgbRaw)

        smp.GetLookupTableForArray(
          'laser_id', 1,
          ScalarRangeInitialized=1.0,
          ColorSpace='RGB',
          RGBPoints=rgbRaw)

def colorByIntensity(sourceProxy):
    if not hasArrayName(sourceProxy, 'Intensity'):
        print(sourceProxy, "no intensity array, returning")
        return False
    setDefaultLookupTables(sourceProxy)
    rep = smp.GetDisplayProperties(sourceProxy)
    rep.ColorArrayName = 'Intensity'
    rep.LookupTable = smp.GetLookupTableForArray('Intensity', 1)
    return True

def getSpreadSheetViewProxy():
    return smp.servermanager.ProxyManager().GetProxy("views", "main spreadsheet view")

def showSourceInSpreadSheet(source):

    spreadSheetView = getSpreadSheetViewProxy()
    smp.Show(source, spreadSheetView)

    # Work around a bug where the 'Showing' combobox doesn't update.
    # Calling hide and show again will trigger the refresh.
    smp.Hide(source, spreadSheetView)
    smp.Show(source, spreadSheetView)

########## Asensing #############
# Get tailing frame
if trailingFrame is None:
    trailingFrame = smp.FindSource("TrailingFrame1")
if pointcloudView is None:
    pointcloudView = smp.FindViewOrCreate('RenderView1', viewtype='RenderView')
smp.SetActiveView(pointcloudView)
smp.SetActiveSource(trailingFrame)
smp.Show(trailingFrame, pointcloudView)
colorByIntensity(trailingFrame)
showSourceInSpreadSheet(pointcloud)

#smp.SetActiveView(app.mainView)
initAsensingRendering()
smp.SetActiveSource(trailingFrame)
setAsensing3DDisplayPropertied(pointcloud)
smp.Render()
