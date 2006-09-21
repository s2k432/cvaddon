#ifndef _CVADDON_MATH_H
#define _CVADDON_MATH_H


////////////////////////////////////////////////////////////
//                CvAddon Math Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Math functions 
// The functions here make use of OpenCV's matrix 
// and angle math functions
////////////////////////////////////////////////////////////
// TODO
// ---
// Check and reject bad hessians? (abs(shift) > 1 etc)
////////////////////////////////////////////////////////////

#include <cv.h>
#include <iostream>
using std::endl;
using std::cerr;

#define CV_MAT_VAL(mat, type, row, col) \
	( ((type*)( (mat)->data.ptr + (mat)->step* (row) ))[ (col) ] )


// Finds local extrema in 3x3 array of values relative
// to center element's location
// <dif> should be a 2x1 vector of floats
// <mat> should be a 2x2 matrix of floats
// <shift> should be a 2x1 vector of floats.
// ** Variables are ordered x,y
// Returns pointer to 2x1 results array
inline float *cvAddonFindExtrema3x3_2D(const float arr[9], CvMat *vec, CvMat *mat, CvMat *shift)
{
	float *diff = (vec->data.fl);
	float *hessian = (mat->data.fl);

#define val(x, y) (arr[3*(y+1) + (x+1)])

	diff[0] = -0.5f * (val(1, 0) - val(-1, 0));
	diff[1] = -0.5f * (val(0, 1) - val(0, -1));
	
	// Hessian
	hessian[0] = val(1,0) + val(-1,0) - 2.0f * val(0,0);
	hessian[1] = hessian[2] = 0.25f * ( val(1,1) + val(-1,-1) - val(-1,1) - val(+1,-1) );
	hessian[3] = val(0,1) + val(0,-1) - 2.0f * val(0,0);

#undef val
	
	cvSolve(mat, vec, shift, CV_LU);

	return shift->data.fl;
}

#endif