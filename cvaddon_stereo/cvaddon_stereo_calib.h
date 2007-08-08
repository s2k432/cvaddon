#pragma once

// Data structures and functions dealing with 
// stereo camera calibration
// REQUIRES: MATLAB io libraries

#include <cv.h>		// OpenCV
#include <mat.h>	// MATLAB

#include <iostream>
using std::cerr;
using std::endl;

// Make sure to include MATLAB .mat io libraries
//#pragma comment(lib, "libmat.lib")
//#pragma comment(lib, "libmx.lib")

// Parameters of single cameras in stereo pair
struct CvAddonCameraIntrinsics
{	
	double fc[2];	// Focal length
	double cc[2];	// Focal Center
	double kc[5];	// Distortion parameters
};

// Relationship between stereo camera pair
struct CvAddonStereoParameters
{
	double R[9];	// 3x3
	double T[3];
	double om[3];
	CvSize imgSize;	// In pixels

	CvAddonCameraIntrinsics left;
	CvAddonCameraIntrinsics right;
};


// Reads MATLAB .MAT file from stereo calibration and 
// fills calibration data structure
int fillStereoDataFromFile(const char *filename, CvAddonStereoParameters &param
	, const bool& loadIndividualCameras = false);

int fillCameraDataFromFile(const char *filename, CvAddonCameraIntrinsics &param);

void printCameraData(const CvAddonCameraIntrinsics& param);

void printStereoData(const CvAddonStereoParameters &param);
