/* Study the camera characteristics
	
a) Study the effect of changing the exposure time on the pixel output from the camera sensor

keys to be used

* + (or =) key increases exposure time by 0.1 ms
* - (or _) key decreases exposure time by 0.1 ms

* ESC, x or X key quits

*/

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <sys/time.h> // gettimeofday()

using namespace cv;
using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;

// Function declarations
void setCamera(CameraPtr pCam);

int main()
{
	int result = 0;
	system("clear");
	
	// Print application build information
	cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

	SystemPtr system = System::GetInstance();
    
	// Retrieve list of cameras from the system
    CameraList camList = system->GetCameras();
    
	unsigned int numCameras = camList.GetSize();
    cout << "Number of cameras detected: " << numCameras << endl << endl;
	if (numCameras == 0)
    {
        // Clear camera list before releasing system
        camList.Clear();
        // Release system
        system->ReleaseInstance();
        cout << "Camera not detected. " << endl;
        cout << "Done! Press Enter to exit..." << endl;
        getchar();
        return -1;
    }

	CameraPtr pCam = nullptr;
	pCam = camList.GetByIndex(0);

	namedWindow("show", 0); // 0 = WINDOW_NORMAL
	moveWindow("show", 200, 0);

	namedWindow("Status", 0); // 0 = WINDOW_NORMAL
	moveWindow("Status", 0, 600);

	ifstream infile("test.ini");
	string tempstring;

	try 
	{
		// Initialize camera
		pCam->Init();
		setCamera(pCam); // set for trigger mode

		unsigned int w, h, camtime;
	
		w = pCam->Width.GetValue();
		h = pCam->Height.GetValue();
		camtime = pCam->ExposureTime.GetValue();

		cout << "Acquiring images " << endl;
		pCam->BeginAcquisition();
		ImagePtr pResultImage;
		ImagePtr convertedImage;
		int key;
		bool doneflag = false;
		int ret;
		Mat m;
		int fps, dt;
		Mat mvector;
		double minVal, maxVal, avgVal;
	
		unsigned long time_start, time_end;	
		struct timeval tv;
		gettimeofday(&tv,NULL);	
		time_start = 1000000 * tv.tv_sec + tv.tv_usec;
	
		Mat statusimg = Mat::zeros(cv::Size(600, 300), CV_64F);
		Mat firstrowofstatusimg = statusimg(Rect(0, 0, 600, 50)); // x,y,width,height
		Mat secrowofstatusimg = statusimg(Rect(0, 50, 600, 50));
		Mat thirdrowofstatusimg = statusimg(Rect(0, 100, 600, 50));
		char textbuffer[80];
		Scalar tempscalar;

		bool expchanged;
		
		while (1)	// camera frames acquisition loop, which is inside the try
		{
			ret = 0;
			// save one image to Mat m
			while(ret == 0)
			{
				pResultImage = pCam->GetNextImage();
				if (pResultImage->IsIncomplete())
				{
					ret = 0;
				}
				else
				{
					ret = 1;
					convertedImage = pResultImage;
					m = Mat(h, w, CV_16UC1, convertedImage->GetData(), convertedImage->GetStride());
					fps++; 
				}
			}

			imshow("show", m);

			gettimeofday(&tv,NULL);	
			time_end = 1000000 * tv.tv_sec + tv.tv_usec;	
			// update the image windows
			dt = time_end - time_start;

			if(dt > 1000000) // 1 second in microseconds 
			{
				m.copyTo(mvector);
				mvector.reshape(0, 1);	//make it into a row array
				minMaxLoc(mvector, &minVal, &maxVal);
				tempscalar = mean(mvector);
				avgVal = tempscalar.val[0];
				sprintf(textbuffer, "fps: %d Avg: %d Max: %d", fps, int(floor(avgVal)), int(floor(maxVal)));
				firstrowofstatusimg = Mat::zeros(cv::Size(600, 50), CV_64F);
				putText(statusimg, textbuffer, Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 3, 1);
			
				
				resizeWindow("Status", 600, 300);
				imshow("Status", statusimg);
			
				fps = 0;
				gettimeofday(&tv,NULL);	
				time_start = 1000000 * tv.tv_sec + tv.tv_usec;	
			}		


			key = waitKey(3); // wait for keypress
			switch (key)
			{

				case 27: //ESC key
				case 'x':
				case 'X':
					doneflag = true;
					break;

				case '=':
					camtime = camtime + 100;
					expchanged = true;
					break;

				case '+':
					camtime = camtime + 10;
					expchanged = true;
					break;

				case '-':
					if (camtime < 8)	// spinnaker has a min of 8 microsec
					{
						camtime = 8;
						break;
					}
					camtime = camtime - 100;
					expchanged = true;
					break;

				case '_':
					if (camtime < 8)	// spinnaker has a min of 8 microsec
					{
						camtime = 8;
						break;
					}
					camtime = camtime - 10;
					expchanged = true;
					break;

				default:
					break;

			} // end of switch

			if (doneflag == 1)
			{
				break;
			}

			if (expchanged == true)
			{
				//Set exp with QuickSpin
				ret = 0;
				if (IsReadable(pCam->ExposureTime) && IsWritable(pCam->ExposureTime))
				{
					pCam->ExposureTime.SetValue(camtime);
					ret = 1;
				}
				if (ret == 1)
				{
					sprintf(textbuffer, "Exp time = %d ", camtime);
					secrowofstatusimg = Mat::zeros(cv::Size(600, 50), CV_64F);
					putText(statusimg, textbuffer, Point(0, 80), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 3, 1);
					imshow("Status", statusimg);

				}
				else
				{
					sprintf(textbuffer, "CONTROL_EXPOSURE failed");
					secrowofstatusimg = Mat::zeros(cv::Size(600, 50), CV_64F);
					putText(statusimg, textbuffer, Point(0, 80), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 3, 1);
					imshow("Status", statusimg);
				}

			} // end of if expchanged


		} // end of while

		pCam->EndAcquisition();
		pCam->DeInit();
		pCam = nullptr;

		// Clear camera list before releasing system
    	camList.Clear();
    	
		// Release system
    	system->ReleaseInstance();

	} // end of try
	catch (Spinnaker::Exception &e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}
  
// Function definitions
void setCamera(CameraPtr pCam)
{
	int result = 0;    
	unsigned int w, h, camspeed, burstframecount,triggerdelay, camtime, camgain = 1, bpp;
	unsigned int offsetx = 0, offsety = 0;
	unsigned int cambinx, cambiny;
	
	ifstream infile("test.ini");
	string tempstring;
	
	// inputs from ini file
	if (infile.is_open())
	{
		infile >> tempstring;
		infile >> tempstring;
		infile >> tempstring;
		// first three lines of ini file are comments
		infile >> camgain;
		infile >> tempstring;
		infile >> camtime;
		infile >> tempstring;
		infile >> bpp;
		infile >> tempstring;
		infile >> w;
		infile >> tempstring;
		infile >> h;
		infile >> tempstring;
		infile >> offsetx;
		infile >> tempstring;
		infile >> offsety;
		infile >> tempstring;
		infile >> camspeed;
		infile >> tempstring;
		infile >> burstframecount;
		infile >> tempstring;
		infile >> triggerdelay;
		infile >> tempstring;
		infile >> cambinx;
		infile >> tempstring;
		infile >> cambiny;

		infile.close();
	}

	cout << "Initialising Camera settings ..." << endl;
	
	pCam->TLStream.StreamBufferHandlingMode.SetValue(StreamBufferHandlingMode_NewestOnly);
	pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
		
	// gain
	pCam->GainAuto.SetValue(GainAuto_Off);	
	pCam->Gain.SetValue(camgain);
	cout << "Gain set to " << pCam->Gain.GetValue() << " dB ..." << endl;

	// exposure time
	pCam->ExposureAuto.SetValue(ExposureAuto_Off);
	pCam->ExposureMode.SetValue(ExposureMode_Timed);
	pCam->ExposureTime.SetValue(camtime);
	cout << "Exp set to " << pCam->ExposureTime.GetValue() << " microsec ..." << endl;

	// bpp or cambitdepth 
	if (bpp == 16)
	{
		pCam->PixelFormat.SetValue(PixelFormat_Mono16);
		cout << "Pixel format set to " << pCam->PixelFormat.GetCurrentEntry()->GetSymbolic() << "..." << endl;
	}
	
	// cambinx
	pCam->BinningHorizontal.SetValue(cambinx);
	cout << "BinningHorizontal set to " << pCam->BinningHorizontal.GetValue() << "..." << endl;

	// cambiny
	pCam->BinningVertical.SetValue(cambiny);
	cout << "BinningVertical set to " << pCam->BinningVertical.GetValue() << "..." << endl;
	
	// width 
	if (IsReadable(pCam->Width) && IsWritable(pCam->Width))
	{
		pCam->Width.SetValue(w);
	}
	else
	{
		cout << "Width not available..." << endl;
	}
	
	// height 
	if (IsReadable(pCam->Height) && IsWritable(pCam->Height))
	{
		pCam->Height.SetValue(h);
	}
	else
	{
		cout << "Height not available..." << endl;
	}

	// offsetx
	if (IsReadable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
	{
		pCam->OffsetX.SetValue(offsetx);
	}
	else
	{
		cout << "Offset X not available..." << endl;
	}
	
	// offsety
	if (IsReadable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
	{
		pCam->OffsetY.SetValue(offsety);
	}
	else
	{
		cout << "Offset Y not available..." << endl;
	}

	// frame rate
	pCam->AcquisitionFrameRateEnable.SetValue(1);
	pCam->AcquisitionFrameRate.SetValue(camspeed);
	cout << "Frame rate set to " << camspeed << endl;

	// reset the hardware trigger	     
	pCam->TriggerMode.SetValue(TriggerMode_Off);
	cout << "Camera set to trigger mode OFF" << endl;

}
