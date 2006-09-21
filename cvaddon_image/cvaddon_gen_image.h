#ifndef _CVADDON_GEN_IMAGE_H
#define _CVADDON_GEN_IMAGE_H

////////////////////////////////////////////////////////////
//        CvAddon Synthetic Image Generation Functions
////////////////////////////////////////////////////////////
// Written by Wai Ho Li
// Last Modified: 28 Aug 2006
////////////////////////////////////////////////////////////
// Code to generate various pseudo-random and fixed-pattern
// images, for use with OpenCV
////////////////////////////////////////////////////////////
// Usage:
// ---
// See function comments
////////////////////////////////////////////////////////////

#include <cv.h>

// <src> is replaced with a dot grid pattern, as 
// defined by the other input parameters. src 
// should be allocated before being passed to 
// this function
inline void cvAddonGenDotGrid(CvArr *src
	, const CvScalar &color = CV_RGB(255, 255, 255)
	, const int& xSpacing = 2, const int& ySpacing = 2
	, const int& xOrigin = 0, const int& yOrigin = 0 )
{
	int i,j;
	CvSize imgSize;

	CV_FUNCNAME("cvAddonGenDotGrid");

	__BEGIN__;

	if(src == NULL) 
		CV_ERROR(CV_StsBadArg, "NULL src image");

	imgSize = cvGetSize(src);

	if(imgSize.width <= 1 || imgSize.height <= 1)
		CV_ERROR(CV_StsBadArg, "Bad src image dimensions");
	if(xSpacing <= 0 || ySpacing <= 0) 
		CV_ERROR(CV_StsBadArg, "Spacing too small (should be >= 1)");
	if(xOrigin < 0 || xOrigin >= imgSize.width) 
		CV_ERROR(CV_StsBadArg, "xOrigin out of bounds");
	if(yOrigin < 0 || yOrigin >= imgSize.height) 
		CV_ERROR(CV_StsBadArg, "yOrigin out of bounds");


	cvZero(src);
	for(i = yOrigin; i < imgSize.height; i += ySpacing)
	{
		for(j = xOrigin; j < imgSize.width; j += xSpacing)
		{
			CvPoint pt = cvPoint(j, i);
			cvLine(src, pt, pt, color);
		}
	}
	__END__;
}

#endif