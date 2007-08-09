// Data structures and functions dealing with 
// stereo camera calibration
// REQUIRES: MATLAB io libraries

#include <cv.h>		// OpenCV
#include <mat.h>	// MATLAB

#include <iostream>
using std::cerr;
using std::endl;

#include "cvaddon_stereo_calib.h"

// Make sure to include MATLAB .mat io libraries
// Libraries located at: C:\Program Files\MATLAB71\extern\include
//#pragma comment(lib, "libmat.lib")
//#pragma comment(lib, "libmx.lib")

// Utility function to copy MATLAB data to a variable, including error checks
inline bool copyMatlabData(MATFile *mfile, const char* varName, double *dst, const int& size)
{
	mxArray *tmp = matGetVariable(mfile, varName);
	if(tmp == NULL)  return false;

	double *data = mxGetPr(tmp);
	if(data == NULL) return false;

	memcpy(dst, data, sizeof(double)*size);
	mxDestroyArray(tmp);	
	return true;
}


// Reads MATLAB .MAT file from stereo calibration and 
// fills calibration data structure
int fillStereoDataFromFile(const char *filename, CvAddonStereoParameters &param
	, const bool& loadIndividualCameras)
{
	const char *FILE_MODE = "r";
	MATFile *mfile = matOpen(filename, FILE_MODE);
	if(mfile == NULL) return -1;
	
	// Stereo Pair Data
	if(!copyMatlabData(mfile, "om", param.om, 3)) return -1;
	if(!copyMatlabData(mfile, "T", param.T, 3)) return -1;
	if(!copyMatlabData(mfile, "R", param.R, 9)) return -1;
	
	double nx, ny;
	if(!copyMatlabData(mfile, "nx", &nx, 1)) return -1;
	if(!copyMatlabData(mfile, "ny", &ny, 1)) return -1;
	param.imgSize = cvSize(nx, ny);

	// Individual Camera Data
	if(!loadIndividualCameras) {
		// -- Left Camera --
		if(!copyMatlabData(mfile, "fc_left", param.left.fc, 2)) return -1;
		if(!copyMatlabData(mfile, "cc_left", param.left.cc, 2)) return -1;
		if(!copyMatlabData(mfile, "kc_left", param.left.kc, 5)) return -1;

		// -- Right Camera --
		if(!copyMatlabData(mfile, "fc_right", param.right.fc, 2)) return -1;
		if(!copyMatlabData(mfile, "cc_right", param.right.cc, 2)) return -1;
		if(!copyMatlabData(mfile, "kc_right", param.right.kc, 5)) return -1;
	}
	return 0;
}

int fillCameraDataFromFile(const char *filename, CvAddonCameraIntrinsics &param)
{
	const char *FILE_MODE = "r";
	MATFile *mfile = matOpen(filename, FILE_MODE);
	if(mfile == NULL) return -1;

	if(!copyMatlabData(mfile, "fc", param.fc, 2)) return -1;
	if(!copyMatlabData(mfile, "cc", param.cc, 2)) return -1;
	if(!copyMatlabData(mfile, "kc", param.kc, 5)) return -1;

	return 0;
}

void printCameraData(const CvAddonCameraIntrinsics& param)
{
	cerr << "Focal Length (in pixels): [" << param.fc[0] << ", " << param.fc[1] << "]" << endl;
	cerr << "Focal Center (in pixels): [" << param.cc[0] << ", " << param.cc[1] << "]" << endl;
	cerr << "Distortion Params (kc):\n[" << param.kc[0] << "," 
		<< param.kc[1] << "," << param.kc[2] << "," << param.kc[3] << "," << param.kc[4] << "]" << endl;
}

void printStereoData(const CvAddonStereoParameters &param)
{
	cerr << " --- Stereo Calibration Data ---" << endl;
	cerr << "Image Pixel Width: " << param.imgSize.width << endl;
	cerr << "Image Pixel Height: " << param.imgSize.height << endl;
	cerr << "-----------------------------------" << endl;
	cerr << "Inter-Camera Translation (T): [" << param.T[0] << ", " << param.T[1] << ", " << param.T[2] << "]" << endl;
	cerr << "Inter-Camera Rotation (om):   [" << param.om[0] << ", " << param.om[1] << ", " << param.om[2] << "]" << endl;
	cerr << "3x3 Rotation Matrix (R):      |" << param.R[0] << ", " << param.R[1] << ", " << param.R[2] << "|" << endl;
	cerr << "                              |" << param.R[3] << ", " << param.R[4] << ", " << param.R[5] << "|" << endl;
	cerr << "                              |" << param.R[6] << ", " << param.R[7] << ", " << param.R[8] << "|" << endl;
	cerr << "-----------------------------------" << endl;
	cerr << "--- Left Camera ---" << endl;
	printCameraData(param.left);
	cerr << "-----------------------------------" << endl;
	cerr << "--- Right Camera ---" << endl;
	printCameraData(param.right);
	cerr << "-----------------------------------" << endl;

}
