# test-spinnaker-opencv
Code to test an FLIR camera.
Creates a binary named studyCam that reads the camera settings from a test.ini file in the same directory.
Uses opencv to display the camera output in a named window 'show'. 
In a separate 'status' window, frames per second (fps), Average Pixel value and Max. Pixel values are displayed.
Exposure time can be changed using + / - keys.
Esc or X key quits the program.
