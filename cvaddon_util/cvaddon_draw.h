#ifndef _CVADDON_DRAW_H
#define _CVADDON_DRAW_H

////////////////////////////////////////////////////////////
//                 CvAddon Draw Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Functions that draw onto IplImages. 
////////////////////////////////////////////////////////////
// Usage:
// ---
// See individual functions for detailed usage instructions.
////////////////////////////////////////////////////////////
// TODO:
// ---
// - Make all functions handle ROI
////////////////////////////////////////////////////////////


#include <cv.h>

// OpenCv doesnt have a function to draw Rectangles. Go figure...
inline void cvAddonDrawRectangle(CvArr *img, const CvRect& rect
	, const CvScalar& colour, const int& thickness=1
	, const int& linetype=8)
{
	cvRectangle(img
		, cvPoint(rect.x, rect.y)
		, cvPoint(rect.x + rect.width - 1, rect.y + rect.height - 1)
		, colour
		, thickness
		, linetype);
}

// Draws pixels on an image using <color>. 
template <typename T>
inline void cvAddonDrawPixels(IplImage *dst, CvPoint *pixels
	, const int& numPixels, const CvScalar &color)
{
	int i, j;
	int x,y;
	int x0, x1, y0, y1;
	CvRect rect;
	unsigned int widthStep;
	unsigned int channels;

	CV_FUNCNAME("cvAddonDrawPixels");

	__BEGIN__;

	if(dst == NULL) CV_ERROR( CV_StsBadArg, "Null Pointer Destination <dst>" );
	if(pixels == NULL) CV_ERROR( CV_StsBadArg, "Null Pointer Pixels Data <pixels>" );

 	rect = cvGetImageROI(dst);
	widthStep = dst->widthStep;
	channels = dst->nChannels;

	x0 = rect.x;
	x1 = rect.x + rect.width;
	y0 = rect.y;
	y1 = rect.y + rect.height;
	for(i = 0; i < numPixels; ++i)
	{
		x = pixels[i].x;
		y = pixels[i].y;

		if(x >= x0 && x < x1 && y >= y0 && y < y1) {
			T *val = (T*)(dst->imageData + widthStep*y) + x*channels;
			for(j = 0; j < channels; ++j) {
				val[j] = (T)color.val[j]; 
			}
		}
	}

	__END__;
}


// Draws a straight line defined in polar form relative to the 
// center of the image <dst>
inline void cvAddonFindPolarLineEndPoints(CvSize size, const float &r, const float &theta
										  , CvPoint &xrPt, CvPoint &p0, CvPoint &p1)
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

	min_d = fabsf(d[0]);
	for(i = 1; i < 4; ++i)
	{
		float dist = d[i];
		if(dist < min_d && dist > 0 || min_d < 0) min_d = dist;
	}
	min_d -= 2;

	p0 = cvPoint(-min_d * sin_th + x_r, min_d * cos_th + y_r);


	if(sin_th != 0) {
		d[0] = -x_r / sin_th;
		d[1] = ((float)size.width - x_r - 1) / sin_th;
	}
	if(cos_th != 0) {
		d[2] = y_r / cos_th;
		d[3] = (1 + y_r - (float)size.height) / cos_th;
	}

	min_d = fabsf(d[0]);
	for(i = 1; i < 4; ++i)
	{
		float dist = d[i];
		if(dist < min_d && dist > 0 || min_d < 0) min_d = dist;
	}
	min_d -= 2;

	p1 = cvPoint(min_d * sin_th + x_r, -min_d * cos_th + y_r);
	
	xrPt = cvPoint( x_r, y_r);


}


inline void cvAddonDrawPolarLine(IplImage *dst, const float &r, const float &theta
	, const CvScalar &color, const int &thickness = 1)
{
	CvPoint xrPt, p0, p1;

	CV_FUNCNAME("cvAddonDrawPolarLine");

	__BEGIN__;

	if(dst == NULL) CV_ERROR( CV_StsBadArg, "Null Pointer Destination <dst>" );

	cvAddonFindPolarLineEndPoints(cvGetSize(dst), r, theta, xrPt, p0, p1);

	cvLine(dst, xrPt, p0, color, thickness);
	cvLine(dst, xrPt, p1, color, thickness);

	cvLine(dst, p0, p0, color, thickness*3);
	cvLine(dst, p1, p1, color, thickness*3);

	cvLine(dst, xrPt, xrPt, color, thickness*3, CV_AA);

	__END__;
}


#endif
