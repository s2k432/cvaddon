#pragma once

// Stereo Triangulation Code
// Based on MATLAB Calibration Toolbox function stereo_triangulation


#include <cv.h>
#include "cvaddon_stereo_calib.h"

class cvAddonStereoTriangulator
{
public:

	cvAddonStereoTriangulator();
	~cvAddonStereoTriangulator();

	// Stereo Triangulation of 2 calibrated views of the same 3D point (known coorespondence)
	// _xL and _xR are the left and right camera pixel coordinates (not normalized) of the target point
	// stereoParams is the calibration parameters of the stereo camera pair (intrinsic and extrinsic)
	// The return values are given in XL and XR, with the 3D location relative to the Left and Right 
	// cameras respectively
	void tri(const CvPoint2D32f _xL, const CvPoint2D32f _xR
		, const CvAddonStereoParameters &stereoParams
		, CvPoint3D32f& XL, CvPoint3D32f& XR);

private:
	CvMat *R, *R_inv, *T;
	CvMat *u, *xt, *xtt;
	CvMat *NN1, *NN2, *Zt, *Ztt, *X1, *X2;
	CvMat *XL, *XR;
	CvMat *Zt_mat, *Ztt_mat;

};
