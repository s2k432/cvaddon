// Test main for various affine operations

#include "highgui.h"
#include "cv.h"

#include "cvaddon_display.h"
#include "cvaddon_draw.h"
#include "cvaddon_math.h"

#include "cvaddon_affine.h"

//#include <iostream>
//using std::cerr;
//using std::endl;

const float R = 9.97502f;
const float TH = -0.0863508f;

int mirrorMask(const IplImage *mask, const float& r, IplImage* mirroredMask
	, const float& maxDist = 250, const bool& fillGaps = true);

int main()
{
	IplImage* gray = cvLoadImage("gray.png", CV_LOAD_IMAGE_GRAYSCALE);
	CvSize imgSize = cvGetSize(gray);

	IplImage* mask = cvLoadImage("mask.png", CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* mask_rot = cvCreateImage(imgSize, gray->depth, 1);
	IplImage* mirroredMask = cvCreateImage(imgSize, gray->depth, 1);

	IplImage* BGR = cvCreateImage(imgSize, gray->depth, 3);
	IplImage* BGR_rot = cvCreateImage(imgSize, gray->depth, 3);

	cvAddonShowImageOnce(mask, "MOTION MASK");

//	cvCvtColor(mask, BGR, CV_GRAY2BGR);
//	cvAddonDrawPolarLine(BGR, R, TH, CV_RGB(0,255,0));
//	cvAddonShowImageOnce(BGR);

	CvAddonImageRotator rotator;

	rotator.rot(mask, TH, mask_rot);

	cvAddonShowImageOnce(mask_rot);

	int topRowOfObj = mirrorMask(mask_rot, R, mirroredMask);

	CvPoint2D32f centre = cvPoint2D32f( ((float)mirroredMask->width-1) / 2.0f, ((float)mirroredMask->height-1) / 2.0f );
	cvCircle( mirroredMask, cvPoint(R + centre.x, topRowOfObj), 3, CV_RGB(70,70,70), CV_FILLED, CV_AA);

	cvAddonShowImageOnce(mirroredMask);

//	cvCvtColor(gray, BGR, CV_GRAY2BGR);
//	cvAddonDrawPolarLine(BGR, R, TH, CV_RGB(0,255,0));
//	cvAddonShowImageOnce(BGR, "RESULTS");

	cvAddonShowImageOnce(gray, "INPUT IMAGE");

	rotator.rot(mirroredMask, -TH, mask);
	cvAnd(mask, gray, gray);

	
	CvMat* tmp = cvCreateMat(3,1,CV_64FC1);
	CvMat* topOfObj = cvCreateMat(2,1,CV_64FC1);
	CV_MAT_ELEM(*tmp, double, 0, 0) = R + centre.x;
	CV_MAT_ELEM(*tmp, double, 1, 0) = (double)topRowOfObj;
	CV_MAT_ELEM(*tmp, double, 2, 0) = 1.0;

	
	CvMat *rotMat = cvCreateMat(2, 3, CV_64FC1);
	
	cv2DRotationMatrix( centre, -TH / CV_PI * 180.0f, 1.0, rotMat);
	cvMatMul(rotMat, tmp, topOfObj);

	CvPoint2D32f topOfObjPt = cvPoint2D32f( CV_MAT_ELEM(*topOfObj, double, 0, 0), CV_MAT_ELEM(*topOfObj, double, 1, 0) );

	cerr << "TOP OF OBJECT: " <<  topOfObjPt.x << ", " << topOfObjPt.y << endl;
	
	cvCvtColor(gray, BGR, CV_GRAY2BGR);
	cvAddonDrawPolarLine(BGR, R, TH, CV_RGB(0,255,0), 3, CV_AA);
	cvCircle( BGR, cvPointFrom32f(topOfObjPt), 3, CV_RGB(0,0,255), CV_FILLED, CV_AA);
	cvAddonShowImageOnce(BGR, "RESULTS");



//	rotator.rot(BGR, TH, BGR_rot);
	

//	cerr << centre.x << ", " << centre.y << endl;
//	cerr << pt1.x << ", " << pt1.y << endl;
//	cerr << pt2.x << ", " << pt2.y << endl;

//	cvLine( BGR_rot, pt1, pt2, CV_RGB(255,0,0) );

//	cvAddonShowImageOnce(BGR_rot);

	return 0;
}

int mirrorMask(const IplImage *mask, const float& r, IplImage* mirroredMask
	, const float& maxDist, const bool& fillGaps)
{
	const uchar MASK_MAX_VAL = 255;
	const uchar FILL_GAPS_VAL = 255;

	CvSize imgSize = cvGetSize(mask);
	CvPoint2D32f centre = cvPoint2D32f( ((float)mask->width-1) / 2.0f, ((float)mask->height-1) / 2.0f );

//	CvPoint pt1 = cvPoint(centre.x + R, centre.y);
//	CvPoint pt2 = cvPoint(centre.x + R - 150, centre.y);

	float mid = centre.x + r;
	float LHS = mid - maxDist/2;
	float RHS = mid + maxDist/2;

	// TODO more - fix maxDist if boudary, etc...
	// error Checks
	if(LHS < 0) LHS = 0;
	if(RHS >= imgSize.width) RHS = imgSize.width - 1;


	cvZero(mirroredMask);

	int i, j;
	for(i = 0; i < imgSize.height; ++i)
	{
		uchar *ptr = (uchar*)(mask->imageData + i*mask->widthStep);
		uchar *m_ptr = (uchar*)(mirroredMask->imageData + i*mirroredMask->widthStep);
		for(j = LHS; j < mid; ++j)
		{
			if(ptr[j]) {
				m_ptr[j] = MASK_MAX_VAL;	//ptr[j];
				m_ptr[ cvRound(RHS + (LHS - j) )] = MASK_MAX_VAL;	//ptr[j];
			}
		}
	}

	// Finding top of object
	int firstNonZeroRow = -1;
	for(i = 0; i < imgSize.height; ++i)
	{
		uchar *m_ptr = (uchar*)(mirroredMask->imageData + i*mirroredMask->widthStep);
		for(j = LHS; j < mid; ++j)
		{
			if(m_ptr[j]) {
				firstNonZeroRow = i;
				break;
			}
		}

		if(firstNonZeroRow != -1) break;
	}

	// 2nd Pass to fill gaps
	if(fillGaps) {
		for(i = 0; i < imgSize.height; ++i)
		{
			uchar *ptr = (uchar*)(mask->imageData + i*mask->widthStep);
			uchar *m_ptr = (uchar*)(mirroredMask->imageData + i*mirroredMask->widthStep);
			for(j = LHS; j < mid; ++j)
			{
				if(ptr[j]) {
					int k;
					for(k = j; k <= cvRound(RHS + (LHS - j) ); ++k)
					{
						m_ptr[k] = FILL_GAPS_VAL;
					}
					break;
				}
			}
		}
	}

	return firstNonZeroRow;
}