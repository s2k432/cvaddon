#ifndef _CVADDON_EDGE_H
#define _CVADDON_EDGE_H

////////////////////////////////////////////////////////////
//          CvAddon Edge Image Processing Functions
////////////////////////////////////////////////////////////
// Written by Wai Ho Li
////////////////////////////////////////////////////////////
// Functions dealing with single-channel (IPL_DEPTH_8U) 
// images, such as edge pixel location listing
////////////////////////////////////////////////////////////
// Usage:
// ---
// See function comments
////////////////////////////////////////////////////////////

#include <cv.h>

// Extracts locations of all non-zero pixels
// RETURNS number of edge pixels found, -1 if maxPoints is reached
// (ie, maxPoints pixels found)
template <typename T, typename PointType>
inline int cvAddonFindNonZeroPixels(const IplImage* src, PointType *pixels
	, const unsigned int &maxPoints, const T &zeroVal = 0)
{
	int i, j, c;
	T *row;
	CvRect rect;
	unsigned int widthStep;

	CV_FUNCNAME("cvAddonFindNonZeroPixels");

	__BEGIN__;

	if(src == NULL) 
		CV_ERROR( CV_StsBadArg, "Null Pointer Input" );
	if(src->nChannels > 1)
		CV_ERROR( CV_StsBadArg, "Multi-channel Input Image" );

 	rect = cvGetImageROI(src);
	widthStep = src->widthStep;

	for(i = 0, c = 0; i < rect.height; ++i)
	{
		row = (T*)(src->imageData + widthStep*(i + rect.y)) + rect.x;
		for(j = 0; j < rect.width; ++j)
		{
			if( row[j] != zeroVal) {
				pixels[c].y = i;
				pixels[c].x = j;
				++c;

				if(c >= maxPoints) return -1;
			}
		}
	}

	__END__;

	return c;
}


#endif