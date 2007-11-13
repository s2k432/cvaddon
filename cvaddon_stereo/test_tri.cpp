// Testing Stereo Triangulation

#include "cvaddon_stereo_tri.h"

#include "cv.h"

#include <iostream>
using std::cerr;
using std::endl;


#include "cvaddon_geometry3D.h"
#include "cvaddon_stereo_sym.h"
#include "cvaddon_stereo_calib.h"

#include "../cvaddon_util/cvaddon_display.h"
#include "../cvaddon_util/cvaddon_draw.h"


int main()
{
	string path = "./_MATLAB_/";	
	string stereoDataFilename = "Calib_Results_stereo.mat";
	string leftCameraDataFilename = "Calib_Results_left.mat";
	string rightCameraDataFilename = "Calib_Results_right.mat";


	// Loading Stereo Calibration data (intrinsics and extrinsics) obtained using MATLAB Calib. Toolbox
	// into C++ structures
	CvAddonStereoParameters stereoParams;
	int stat;
	bool LOAD_INDIVIDUAL_CAMERAS = false;
	stat = fillStereoDataFromFile( (path+stereoDataFilename).c_str(), stereoParams, LOAD_INDIVIDUAL_CAMERAS);

	// Loading individual Cameras
	if(LOAD_INDIVIDUAL_CAMERAS) {
		stat = fillCameraDataFromFile((path+leftCameraDataFilename).c_str(), stereoParams.left);
		stat = fillCameraDataFromFile((path+rightCameraDataFilename).c_str(), stereoParams.right);
	}
	printStereoData(stereoParams);

	cvAddonStereoTriangulator tri;
	
	CvPoint2D32f xL, xR;
	CvPoint3D32f XL, XR;

	xL.x = 105;
	xL.y = 100;
	xR.x = 55;
	xR.y = 90;

	tri.tri(xL, xR, stereoParams, XL, XR);

	cerr << XL.x << ", ";
	cerr << XL.y << ", ";
	cerr << XL.z << endl;

	cerr << XR.x << ", ";
	cerr << XR.y << ", ";
	cerr << XR.z << endl;

	return 0;
}