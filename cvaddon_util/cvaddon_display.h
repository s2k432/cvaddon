#ifndef _CVADDON_DISPLAY_H
#define _CVADDON_DISPLAY_H

////////////////////////////////////////////////////////////
//                 CvAddon Display Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Various scaling and display functions that extend
// and make use of OpenCV. Mainly for visualization purposes
////////////////////////////////////////////////////////////
// Usage:
// ---
// See individual functions for detailed usage instructions.
// In general, the function argument list is (input, output)
// for the scaling functions, and (input image, window_name)
// for the display functions
////////////////////////////////////////////////////////////

#include <cv.h>
#include <highgui.h>

#include <iostream>
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <algorithm>
using std::replace;

///////////////////////////////////////////////////////////////////
// Scales input single-channel image for display
///////////////////////////////////////////////////////////////////
// Returns input image scaled to a range of 0-255.
// The returned image should be released using cvReleaseImage
// once it is no longer needed. 
//
// For floating point input, the image values are scaled and shifted
// to the 0-255 range. 8-bit (display-ready) images are left unchanged
// Only attempts to convert single channel images
///////////////////////////////////////////////////////////////////
inline IplImage* cvAddonScaleImage(const CvArr* in)
{
	// Doesn't support multi-channel images
	if(in == NULL)
		return NULL;

	IplImage* img;
	IplImage* tmp = new IplImage;
	img = cvGetImage(in, tmp);
	IplImage* res;

	// No need for conversion
	// Color images are not altered
	if(img->depth == IPL_DEPTH_8U || img->nChannels > 1) {
		res = cvCloneImage(img);
	}
	else {
		res = cvCreateImage(cvGetSize(img),  IPL_DEPTH_8U, 1);
		double MagMin, MagMax;
		cvMinMaxLoc(img, &MagMin, &MagMax);
		cvConvertScale(img, res, 255.0/(MagMax - MagMin), (255.0 * MagMin / (MagMin - MagMax) ) );
	}
	delete tmp;
	return res;
}

// Fast scaling function. No error checking. The cvConvertScale will complain
// if something is wrong
// {in} should be a floating point or 16-bit image, and res should be a 8-bit unsigned image.
// Use this function to convert images to show with cvShowImage
inline void cvAddonQuickScaleImage(const CvArr* in, CvArr* res) {
	double MagMin, MagMax;
	cvMinMaxLoc(in, &MagMin, &MagMax);
	cvConvertScale(in, res, 255.0/(MagMax - MagMin), (255.0 * MagMin / (MagMin - MagMax) ) );
}

// Scaling with no shift
inline void cvAddonQuickScaleImageNoShift(const CvArr* in, CvArr* res) {
	double MagMax;
	cvMinMaxLoc(in, NULL, &MagMax);
	cvConvertScale(in, res, 255.0/MagMax, 0);
}
#define cvAddonQuickScale cvAddonQuickScaleImage 
#define cvAddonQuickScaleNoShift cvAddonQuickScaleImageNoShift

///////////////////////////////////////////////////////////////////
// Displays input image in a window until user press a key
///////////////////////////////////////////////////////////////////
// Scales input image using cvAddonScaleImage() and then displays
// image in a window named {windowName}. The window can be dismissed
// by pressing any key. 
//
// Special Features:
// ---
// Press 's' to save the image in png format, with name {windowName}
// Note that the scaled (0-255 range) image is saved, not the original
///////////////////////////////////////////////////////////////////
inline bool cvAddonShowImageOnce(const CvArr* img, const char* windowName = "Temp Display")
{	
	IplImage *tmpImg = cvAddonScaleImage(img);

	if(tmpImg != NULL) {
		cvNamedWindow(windowName, CV_WINDOW_AUTOSIZE);		
		cvShowImage(windowName, tmpImg);

		char key = cvWaitKey(0);
		if(key == 's') {
			string filename(windowName);

			//Replacing spaces with underscores in name
			std::replace(filename.begin(), filename.end(), ' ', '_');

			filename += ".png";
			cerr << "Saving Image: " << filename << endl;
			cvSaveImage(filename.c_str(), tmpImg);
		}
		cvDestroyWindow(windowName);
		cvReleaseImage(&tmpImg);
		return true;
	}
	return false;
}

#endif