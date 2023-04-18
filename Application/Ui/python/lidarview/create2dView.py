import paraview.simple as smp
renderingWindowLayout = smp.GetLayout()
renderingWindowLayout.SplitVertical(0, 0.70)
imageView = smp.CreateView("RenderView")
imageView.AxesGrid = 'GridAxes3DActor'
imageView.StereoType = 0
imageView.Background = [0.0, 0.0, 0.0]
renderingWindowLayout.AssignView(2, imageView)
imageView.InteractionMode = '2D'
imageView.OrientationAxesVisibility = 0
imageView.Camera2DManipulators = ['Pan', 'None', 'Zoom', 'Zoom', 'Zoom', 'ZoomToMouse', 'Roll', 'Pan', 'Rotate']
# Create a lidar as image
pointcloud = smp.FindSource('LidarReader1')
lidarAsImage = smp.PointCloudLinearProjector(pointcloud)
# show the image
smp.SetActiveView(imageView)
smp.SetActiveSource(lidarAsImage)
lidarAsImageDisplay = smp.Show(lidarAsImage, imageView)
# Hacky to update automatically lidar as image
dataDisplay = smp.Show(pointcloud, imageView)
smp.Hide(pointcloud, imageView)
# Set color scale
#imageScalarsLUT = smp.GetColorTransferFunction('ImageScalars')
#  imageScalarsLUT = smp.GetColorTransferFunction('LidarAsImage')
#imageScalarsLUT.RescaleTransferFunction(0.0, 255.0)
#imageScalarsLUT.ApplyPreset('Viridis (matplotlib)', True)
# render
smp.Render()

pointcloudView = smp.FindViewOrCreate('RenderView1', viewtype='RenderView')

# Set default pointcloud colorization to intensity
smp.SetActiveView(pointcloudView)
trailingFrame = smp.FindSource("TrailingFrame1")
trailingFrame = smp.SetActiveSource(trailingFrame)

if trailingFrame is not None:
    pointcloudViewDisplay = smp.GetDisplayProperties(trailingFrame, view=pointcloudView)
    smp.ColorBy(pointcloudViewDisplay, ('POINTS', 'Intensity'))
    pointcloudViewDisplay.RescaleTransferFunctionToDataRange(True, False)
    pointcloudViewDisplay.SetScalarBarVisibility(pointcloudView, True)
# render
smp.Show(trailingFrame, pointcloudView)
smp.Render()
