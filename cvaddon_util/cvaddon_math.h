#ifndef _CVADDON_MATH_H
#define _CVADDON_MATH_H


////////////////////////////////////////////////////////////
//                CvAddon Math Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Math functions including:
// - Matrix operations
// - CvRect resize
// - MAX, MIN etc
////////////////////////////////////////////////////////////
// TODO
// ---
// Error checks
//
// DOC issues:
// Check and reject bad hessians? (abs(shift) > 1 etc)
////////////////////////////////////////////////////////////
#include <cxcore.h>
#include <cv.h>

#include <iostream>
using std::endl;
using std::cerr;

#define CV_MAT_VAL(mat, type, row, col) \
	( ((type*)( (mat)->data.ptr + (mat)->step* (row) ))[ (col) ] )


// Finds centroid of grayscale single channel image
inline CvPoint2D32f findCentroid(const IplImage *src)
{
	CvMoments moments;
	cvMoments(src, &moments, 0);
	return cvPoint2D32f(moments.m10 / moments.m00, moments.m01 / moments.m00);
}

// Orders two scalars such that their elements are placed into a 
// lower and upper resulting scalar
// eg. (1,2,3); (4,0,6) ==> lower:(1,0,3), upper:(4,2,6)
inline void lowerUpper(const CvScalar& a, const CvScalar& b, CvScalar& lower, CvScalar& upper)
{
	int i;
	for(i = 0; i < 4; ++i) {
		if(a.val[i] > b.val[i]) {
			lower.val[i] = b.val[i];
			upper.val[i] = a.val[i];
		}
		else {
			lower.val[i] = a.val[i];
			upper.val[i] = b.val[i];
		}
	}
}

// Output:
// <xrPt> - point where r interects polar line
// <p0>, <p1> - End points of polar line
inline void cvAddonFindPolarLineEndPoints(const CvSize &size, const float &r, const float &theta
										  , CvPoint2D32f &xrPt, CvPoint2D32f &p0, CvPoint2D32f &p1)
{
	int i;
	float c_x, c_y;	// Image center
	
	float r_x, r_y;	// r components in x and y
	float x_r, y_r;	// point where r interects polar line
	float sin_th, cos_th;

	float d[4];		// dist to closest image boundary
	float min_d;	// MIN(d)
		

	c_x = (float)(size.width - 1) / 2.0f;
	c_y = (float)(size.height - 1) / 2.0f;

	cos_th = cosf(theta);
	sin_th = sinf(theta);

	r_x = r*cos_th;
	r_y = r*sin_th;

	x_r = c_x + r_x;
	y_r = c_y + r_y;
	
	// Finding closest border, so we can find line's end points
	if(sin_th != 0) {
		d[0] = x_r / sin_th;
		d[1] = (x_r - (float)size.width + 1) / sin_th;
	}
	if(cos_th != 0) {
		d[2] = -y_r / cos_th; 
		d[3] = ((float)size.height - 1 - y_r) / cos_th;
	}

	min_d = d[0];
	for(i = 1; i < 4; ++i)
	{
		float dist = d[i];
		if(dist < min_d && dist > 0 || min_d < 0) min_d = dist;
	}
	min_d -= 2;

	p0 = cvPoint2D32f( -min_d * sin_th + x_r, min_d * cos_th + y_r );


	if(sin_th != 0) {
		d[0] = -x_r / sin_th;
		d[1] = ((float)size.width - x_r - 1) / sin_th;
	}
	if(cos_th != 0) {
		d[2] = y_r / cos_th;
		d[3] = (1 + y_r - (float)size.height) / cos_th;
	}

	min_d = d[0];
	for(i = 1; i < 4; ++i)
	{
		float dist = d[i];
		if(dist < min_d && dist > 0 || min_d < 0) min_d = dist;
	}
	min_d -= 2;

	p1 = cvPoint2D32f( min_d * sin_th + x_r, -min_d * cos_th + y_r );
	
	xrPt = cvPoint2D32f( x_r, y_r );
}


inline void cvAddonFindPolarLineFromEndPoints(const CvPoint2D32f &origin
	, const CvPoint2D32f &p0, const CvPoint2D32f &p1, float &r, float &theta)
{
	float dx, dy;
	CvPoint2D32f a,b, ro, d, du;
	float b_dot_du, d_norm;

	a = cvPoint2D32f(p0.x - origin.x, p0.y - origin.y);
	b = cvPoint2D32f(p1.x - origin.x, p1.y - origin.y);

	d.x = b.x - a.x;
	d.y = b.y - a.y;

	d_norm = sqrtf(d.x*d.x + d.y*d.y);
	du.x = d.x / d_norm;
	du.y = d.y / d_norm;
	
	// Dot product: p1 . d
	b_dot_du = (du.x*b.x + du.y*b.y);

	// 
	ro.x = b.x - b_dot_du * du.x;
	ro.y = b.y - b_dot_du * du.y;
	
	r = sqrtf( ro.x*ro.x + ro.y*ro.y );

	theta = atan2f(ro.y, ro.x);	

	// Wrapping theta to +- pi/2
	if(theta > CV_PI / 2) {
		theta -= CV_PI;
		r *= -1;
	}
	if(theta < -CV_PI / 2) {
		theta += CV_PI;
		r *= -1;
	}
}


// The "pivot" is the point that when connected to the origin will form
// a line perpendicular to (T-junction) the line described by {r, theta}.
inline void cvAddonFindPolarLineFromPivot(const CvPoint2D32f &origin
	, const CvPoint2D32f &pivot, float &r, float &theta)
{
	CvPoint2D32f ro;

	ro.x = pivot.x - origin.x;
	ro.y = pivot.y - origin.y;
	
	r = sqrtf( ro.x*ro.x + ro.y*ro.y );

	theta = atan2f(ro.y, ro.x);	

	// Wrapping theta to +- pi/2
	if(theta > CV_PI / 2) {
		theta -= CV_PI;
		r *= -1;
	}
	if(theta < -CV_PI / 2) {
		theta += CV_PI;
		r *= -1;
	}
}

// Resizes CvRect based on two ratios
inline void cvAddonResizeRect(CvRect &rect, const float& widthRatio, const float &heightRatio)
{
	rect.x += (1.0f - widthRatio)/2 * rect.width;
	rect.width *= widthRatio;
	rect.y += (1.0f - heightRatio)/2 * rect.height;
	rect.height *= heightRatio;
}

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