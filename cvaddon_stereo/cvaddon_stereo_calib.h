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


// Normalizes 2D locations of pixels given camera instrinsics
// Based on normalize.m and comp_distortion_oulu.m from 
// the MATLAB calibration toolbox
inline void normalizePixel(const CvPoint2D32f &pt, const CvAddonCameraIntrinsics &params, CvPoint2D32f &result)
{
	static const int UNDISTORT_ITERATIONS = 20;

	result.x = (pt.x - params.cc[0]) / params.fc[0];
	result.y = (pt.y - params.cc[1]) / params.fc[1];

	// Lens Distortion
    float k1 = params.kc[0];
    float k2 = params.kc[1];
    float k3 = params.kc[4];
    float p1 = params.kc[2];
    float p2 = params.kc[3];
    
	// Iterative undistort
	CvPoint2D32f tmp = result;	// Initial guess
	CvPoint2D32f delta;
	int i;
	float r_2, k_radial;
    for(i = 0; i < UNDISTORT_ITERATIONS; ++i) 
	{
		// r_2 = sum(x.^2);
		r_2 = tmp.x*tmp.x + tmp.y*tmp.y;

		// k_radial =  1 + k1 * r_2 + k2 * r_2.^2 + k3 * r_2.^3;
		k_radial = 1 + k1*r_2 + k2*r_2*r_2 + k3*r_2*r_2*r_2;

        //delta_x = [2*p1*x(1,:).*x(2,:) + p2*(r_2 + 2*x(1,:).^2);
		//		p1 * (r_2 + 2*x(2,:).^2)+2*p2*x(1,:).*x(2,:)];
		delta.x = 2*p1*tmp.x*tmp.y + p2*(r_2 + 2*tmp.x*tmp.x);
		delta.y = p1*(r_2 + 2*tmp.y*tmp.y) + 2*p2*tmp.x*tmp.y;

        //x = (xd - delta_x)./(ones(2,1)*k_radial);
		tmp.x = (result.x - delta.x) / k_radial;
		tmp.y = (result.y - delta.y) / k_radial;
    }
	result = tmp;
}