#pragma once

#include "cvaddon_display.h"
#include "cvaddon_draw.h"
#include "cvaddon_math.h"

// Affine Transformatio of OpenCV images, such as image rotation

class CvAddonImageRotator
{
public:
	CvAddonImageRotator() { rotMat = cvCreateMat(2, 3, CV_64FC1); }
	~CvAddonImageRotator() { cvReleaseMat(&rotMat); }

	// Theta in radians
	void rot(const IplImage* src, const float& theta, IplImage* dst);

private:
	CvMat *rotMat;
};

inline void CvAddonImageRotator::rot(const IplImage* src, const float& theta, IplImage* dst)
{
	
	CvSize imgSize = cvGetSize(src);
	
	CvPoint2D32f centre = cvPoint2D32f( ((float)src->width-1) / 2.0f, ((float)src->height-1) / 2.0f );

	cv2DRotationMatrix( centre, theta / CV_PI * 180.0f, 1.0, rotMat);

	cvWarpAffine(src, dst, rotMat, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0) );

	

//    //CvMat* N = cvCreateMat(2, 3, CV_32F);
//    //cv2DRotationMatrix(Center, 10, 1 , N);
//    CvMat N;
//    double trans[] = {
//        0,    1,    0,            // 0 = (1-scale*cos(90))*center.x -
//scale*sin(90)*center.y + (height-width)/2
//        -1,    0,    width     // width = scale*sin(90)*center.x +
//(1-scale*cos(90))*center.y + (width-height)/2
//    };
//    cvInitMatHeader(&N, 2, 3, CV_64F, trans);
//    cvWarpAffine(img, dest, &N, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,
//cvScalarAll(200)); 

}