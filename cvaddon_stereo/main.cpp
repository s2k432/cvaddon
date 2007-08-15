// Test main file to exercise Stereo functions

#include <iostream>
using std::cerr;
using std::endl;


#include "cv.h"

#include "cvaddon_geometry3D.h"
#include "cvaddon_stereo_sym.h"
#include "cvaddon_stereo_calib.h"

#include "../cvaddon_util/cvaddon_display.h"
#include "../cvaddon_util/cvaddon_draw.h"


void cleanEdgePixelsFromBorder(IplImage *srcEdge)
{
	int width = 8;
	cvLine(srcEdge, cvPoint(0, 0), cvPoint(0, srcEdge->height-1), CV_RGB(0,0,0), width);
	cvLine(srcEdge, cvPoint(0, 0), cvPoint(srcEdge->width-1, 0), CV_RGB(0,0,0), width);
	cvLine(srcEdge, cvPoint(srcEdge->width-1, srcEdge->height-1), cvPoint(0, srcEdge->height-1), CV_RGB(0,0,0), width);
	cvLine(srcEdge, cvPoint(srcEdge->width-1, srcEdge->height-1), cvPoint(srcEdge->width-1, 0), CV_RGB(0,0,0), width);
}

int main()
{
	string path = "./_MATLAB_/";	
	string stereoDataFilename = "Calib_Results_stereo.mat";
	string leftCameraDataFilename = "Calib_Results_left.mat";
	string rightCameraDataFilename = "Calib_Results_right.mat";

//	string path = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/";
//	string stereoDataFilename = "Calib_Results_stereo.mat";
//	string leftCameraDataFilename = "left_Calib_Results.mat";
//	string rightCameraDataFilename = "right_Calib_Results.mat";


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

	// NEW data from arm-camera experiment setup
	const char* leftImgName = "./_MATLAB_/test_left.bmp";
	const char* rightImgName = "./_MATLAB_/test_right.bmp";


	// Cup 03 not very accuratate, and has 4 matching 3D lines due to horizontal lines
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/cup_left_01.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/cup_right_01.bmp";

//	// Cup 02 not detected due to edge noise and horizontal lines. Too many lines in 3D matching in general
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/multi_left_02.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/multi_right_02.bmp";

	// Bot 03 not detected accurately, so no 3D symline found. Else, works fine
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/bot_left_01.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/bot_right_01.bmp";

	// Too many 3D symlines, else its alright
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/can_left_04.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/can_right_04.bmp";

	// Ok, still too many 3D symlines, and slower detection due to large number of edge pixels
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/trans_left_03.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/trans_right_03.bmp";

	// Good, still detecting a couple of noisy 3D symlines
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/tex_left_04.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/tex_right_04.bmp";
	


	// A lot of edge noise, but, seems promising. Needs more testing
//	const char* leftImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/box_left_01.bmp";
//	const char* rightImgName = "F:/_WORK/_PhD/code_and_data/symmetry/_matlab/stereo/stereo_data_images/box_right_01.bmp";

	
//+============= TEST CASES =============
//	const char* leftImgName = "cup_left_03.bmp";
//	const char* rightImgName = "cup_right_03.bmp";


	IplImage *leftImg = cvLoadImage(leftImgName, 0);
	IplImage *rightImg = cvLoadImage(rightImgName, 0);

	IplImage *leftImgBGR = cvLoadImage(leftImgName, 1);
	IplImage *rightImgBGR = cvLoadImage(rightImgName, 1);


	// Canny
	const double CANNY_TH1 = 30;
	const double CANNY_TH2 = 90;
	
	// Fast Sym Detectionn
	const int MAX_DIST = 200;
	const int MIN_DIST = 20;
	//const float SAMPLE_RATIO = 1.0f;
	const int MIN_ANGLE = -15;
	const int MAX_ANGLE = 15;


	IplImage *leftEdge = cvCloneImage(leftImg);	
	IplImage *rightEdge = cvCloneImage(rightImg);


	CvSize imgSize = cvGetSize(leftEdge);
	float rDivs = sqrtf(leftImg->width*leftImg->width + leftImg->height*leftImg->height) + 1;
	int thDivs = 180;
	CvAddonFastSymDetector fastSym(imgSize, cvSize(rDivs, thDivs));

	// Fast Sym Peakfind
	const int NUM_PEAKS_FOUND = 5;
	const float PEAK_FIND_THRESH =  0.25f;
	const int R_SUPP_SIZE = 10;
	const int TH_SUPP_SIZE = 10;

	// Calibration (table) plane wrt Right camera (from fitplane MATLAB function in stereo_2D_to_3D.m)
	PlaneHessian3D<float> calibPlane = {0.00006052735995f, 0.00193667471661f, 0.00145470661978f, -0.99999706472376f};
	
	// Old plane
	//{0.00021343422080f, 0.00251180346276f, 0.00162225558579f, -0.99999550677791f};

    
	// TODO vector pre-allocation needs tidying up (can have seg fault if not allocated with enough mem)
	vector< Line3D<float> > symLines3D(30);

	// Edge Detection
	cvCanny(leftImg, leftEdge, CANNY_TH1, CANNY_TH2);	
	cvCanny(rightImg, rightEdge, CANNY_TH1, CANNY_TH2);
	cleanEdgePixelsFromBorder(leftEdge);
	cleanEdgePixelsFromBorder(rightEdge);

	// DEBUG
	cvAddonShowImageOnce(leftEdge);
	cvAddonShowImageOnce(rightEdge);

	CvAddonFastSymResults leftSymResults(NUM_PEAKS_FOUND);
	fastSym.vote(leftEdge, MIN_DIST, MAX_DIST, true, MIN_ANGLE, MAX_ANGLE);				
	fastSym.getResult(NUM_PEAKS_FOUND, leftSymResults, R_SUPP_SIZE, TH_SUPP_SIZE, false, true, MIN_ANGLE, MAX_ANGLE);

	CvAddonFastSymResults rightSymResults(NUM_PEAKS_FOUND);
	fastSym.vote(rightEdge, MIN_DIST, MAX_DIST, true, MIN_ANGLE, MAX_ANGLE);	
	fastSym.getResult(NUM_PEAKS_FOUND, rightSymResults, R_SUPP_SIZE, TH_SUPP_SIZE, false, true, MIN_ANGLE, MAX_ANGLE);

	int m;
	for(m = 0; m < leftSymResults.numSym; ++m)
	{
		cvAddonDrawPolarLine(leftImgBGR, leftSymResults.symLines[m].r, leftSymResults.symLines[m].theta
			, CV_RGB(255,0,0) );
	}
	cvAddonShowImageOnce(leftImgBGR);

	for(m = 0; m < rightSymResults.numSym; ++m)
	{
		cvAddonDrawPolarLine(rightImgBGR, rightSymResults.symLines[m].r, rightSymResults.symLines[m].theta
			, CV_RGB(255,0,0) );
	}	
	cvAddonShowImageOnce(rightImgBGR);

	int numTriangulated = cvAddonTriangluateSymLines(leftSymResults, rightSymResults
	, calibPlane, imgSize, stereoParams, symLines3D);

	int i;
	for(i = 0; i < symLines3D.size(); ++i) {
		cerr << "SymLine3D " << i << endl;
		cerr << symLines3D[i].x0.x << ",";
		cerr << symLines3D[i].x0.y << ",";
		cerr << symLines3D[i].x0.z << endl;
		cerr << symLines3D[i].x1.x << ",";
		cerr << symLines3D[i].x1.y << ",";
		cerr << symLines3D[i].x1.z << endl;

		// Finding intersect with table plane
		Point3D<float> intersectPoint;
		int inter = linePlaneIntersect(symLines3D[i], calibPlane, intersectPoint);

		cerr << "SymLine-Table intersect: ";
		printPoint3D(intersectPoint);
		cerr << endl;
	}


	cvNamedWindow("LOL LETS WAIT", 0);
	cvWaitKey(0);
	
	return 0;
}