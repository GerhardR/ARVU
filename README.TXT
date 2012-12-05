ARVU 2012 Kinect SDK 1.1
===========================

This is a short README for the ARVU Kinect SDK. 

The ARVU SDK consists of two parts that work with different drivers
for the Kinect device.

I) KinectViewer.exe to capture raw images. This uses the libfreenect
   drivers. 
II) Kinect3D.exe to display an interactive 3D view. This uses the MS Kinect 
   SDK drivers. 

Please make sure to always have the right drivers installed. We recommend 
that you start with the drivers for I) and then switch to the drivers for
II). 
   
Installation for I) KinectViewer.exe
------------------------------------

1) Extract the archive (you have done that, if you are reading this :)

2) Connect the Kinect device

3) Install the 3 devices that appear 
  - The drivers are in libfreenect/platform/windows/inf
  - Either point the device notification dialog to this directory
  - Or install the drivers afterwards through the device manager

4) Run the KinectViewer.exe program at KinectViewer/KinectViewer.exe

KinectViewer
------------

KinectViewer is a simple program to view and record the different image 
streams captured by the Kinect. Upon startup it shows the RGB image and a 
color coded depth image. It can switch to another view that shows the intensity
image from the infrared camera plus the depth image.

The following keys can be used to interact with the program

A       switch to infrared camera + depth
S       switch to RGB camera + depth
Space   save a snapshot of RGB or infrared + depth
I       print information on resolution
Esc     exit program

RGB images are stored as 640x480 png images

Depth images are stored in two formats:
  as 640x488 16bit gray png images with depth encoded as an 
    11bit disparity measurement per pixel
  as 640x480 8bit gray png where disparity is scaled to 8 bit

Infrared images are stored as 640x488 gray scale png images

Installation for II) Kinect3D.exe
---------------------------------

1) First remove all libfreenect drivers. To do so, plug in the 
Kinect and go to the Device Manager. There click on the Kinect devices, 
open up the drivers section and remove the driver.

2) Install the MS Kinect SDK

3) Connect the Kinect device

4) Run the 'Sample Skeletal Viewer' from the MS Kinect SDK. If this
   works you are set. Otherwise, make sure to follow step 1)

5) Run the Kinect3D.exe program at Kinect3D/Kinect3D.exe

Kinect3D
--------

This program is a small demo program for using the 3D information to
create a rendered and animated 3D scene. It serves as the basis for integrating
visual effects and interaction.

The following keys can be used to control the program

Space	switch between an image view, AR view and 3D scene view
S		switch between different contents, currently there are two
Esc		exit the program

