#include "cv.h"
#include "highgui.h"

#include "cvaddon_display.h"

#include "windows_fast_timer.h"

#include "cvaddon_fast_bgr2hsv.h"

#include "cvaddon_filter2d.h"

#include "cvaddon_print.h"

//#define _USE_IPP

#ifdef _USE_IPP
	#include "ipp.h"
	#pragma comment(lib, "ippcore")
	#pragma comment(lib, "ipps")	
	#pragma comment(lib, "ippi")
	#pragma comment(lib, "ippcv")
	#pragma comment(lib, "ippcc")
#endif


int main()
{
	IplImage *in = cvLoadImage("filter2d_test1.bmp");
	CvSize imgSize = cvGetSize(in);

	cvAddonShowImageOnce(in);

	IplImage *hsv = cvCloneImage(in);
	IplImage *f_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *f_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *f_val = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);

	// TESTING: Generic 2D filter
	cvAddonInitHsvLut();
	cvAddonBGR2HSV_LUT(in, f_hue, f_sat, f_val);

	cvAddonShowImageOnce(f_hue);
	cvAddonShowImageOnce(f_sat);
	cvAddonShowImageOnce(f_val);

	CvAddonFilter2D<uchar> filt(4, 4, 179, 255);

	filt.buildHist(f_hue, f_sat, 0, 179, 0, 255, true, false, f_val);

	cerr << filt << endl;

// Seems to work
//	filt.blendHist(f_sat, f_hue, 0.5f, 0, 179, 0, 179, true, false, f_val);
//
//	cerr << filt << endl;

//	int i,j;
//	for(i = 0; i < 6; ++i) {
//		for(j = 0; j < 6; ++j) {
//			cerr << filt.hist[i][j] << ",";
//		}
//		cerr << endl;
//	}

//	CvRect orgROI = cvGetImageROI(f_sat);
////	cerr << orgROI << endl;
//
//	CvRect newROI = cvRect(-2,-2,1318,1318);
//	cvSetImageROI(f_sat, newROI);
//
//	cerr << newROI << endl;
//
////	cerr << cvGetSize(f_sat) << endl;
//
//	newROI = cvGetImageROI(f_sat);
//
//	cerr << newROI << endl;
//
//	IplImage *tmp = cvCreateImage(cvGetSize(f_sat), IPL_DEPTH_8U, 1);
//
//	//cvCopy(f_sat, tmp);
//	uchar *srcRow, *tmpRow;
//	int i,j;
//	for(i = newROI.y; i < newROI.y + newROI.height; ++i)
//	{
//		srcRow = (uchar*)(f_sat->imageData + f_sat->widthStep*i);
//		tmpRow = (uchar*)(tmp->imageData + tmp->widthStep*(i-newROI.y));
//		for(j = newROI.x; j < newROI.x + newROI.width; ++j)
//		{
//			tmpRow[j-newROI.x] = srcRow[j];
//		}
//	}
//
//	cvAddonShowImageOnce(tmp);
	

//	CvAddonFilter2D<uchar> filt(32, 32);
//	filt.buildHist(f_hue, f_sat, 0, 255, 0, 255, NULL);

//	IplImage *cv_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//	IplImage *cv_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//	IplImage *cv_val = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//
//
////	TESTING: Time Trials
//	IplImage *img[6];
//	img[0] = cvLoadImage("test0.bmp");
//	img[1] = cvLoadImage("test1.bmp");
//	img[2] = cvLoadImage("test2.bmp");
//	img[3] = cvLoadImage("test3.bmp");
//	img[4] = cvLoadImage("test4.bmp");
//	img[5] = cvLoadImage("test5.bmp");
//
//	const int TRIALS = 2000;
//	int i;
//	float timeForTrials;
//	FastTimer t0;
//
//	cvAddonInitHsvLut();
//	
//	t0.getLoopTime();
//	for(i = 0; i < TRIALS; ++i) {
//		cvAddonBGR2HSV_LUT(img[i%6], f_hue, f_sat, f_val);
//	}
//	timeForTrials = t0.getLoopTime();
//	cerr << "LUT: " << timeForTrials / (float)TRIALS << endl;
//
//	t0.getLoopTime();
//	for(i = 0; i < TRIALS; ++i) {
//		cvCvtColor(img[i%6], hsv, CV_BGR2HSV);
//	}
//	timeForTrials = t0.getLoopTime();
//	cerr << "CV: " << timeForTrials / (float)TRIALS << endl;
//
//
//
//#ifdef _USE_IPP
//	IplImage *inIPP = cvCloneImage(in);
//	IplImage *hsvIPP = cvCloneImage(in);
//	IplImage *ipp_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//	IplImage *ipp_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//	IplImage *ipp_val = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
//
//	IppiSize ippSize;
//	ippSize.width = imgSize.width;
//	ippSize.height = imgSize.height;
//
//	t0.getLoopTime();
//	for(i = 0; i < TRIALS; ++i) {
//
//		cvCvtColor(in, inIPP, CV_BGR2RGB);
//		IppStatus ippStat = ippiRGBToHSV_8u_C3R((unsigned char*)inIPP->imageData, inIPP->widthStep,
//		(unsigned char*)hsvIPP->imageData, hsvIPP->widthStep, ippSize);
//
//	}
//	timeForTrials = t0.getLoopTime();
//	cerr << "IPP: " << timeForTrials / (float)trials << endl;
//
//	cvSplit(hsvIPP, ipp_hue, ipp_sat, ipp_val, NULL);
//
//#endif


//
////  TESTING: Visual Inspection of colour conversion
//	cvCvtColor(img[(i-1)%6], hsv, CV_BGR2HSV);
//	
//	cvSplit(hsv, cv_hue, cv_sat, cv_val, NULL);
//	
//
//
//	cvAddonShowImageOnce(f_hue);
//	cvAddonShowImageOnce(cv_hue);
////	cvAddonShowImageOnce(ipp_hue);
//
//	cvAddonShowImageOnce(f_sat);
//	cvAddonShowImageOnce(cv_sat);
////	cvAddonShowImageOnce(ipp_sat);
//
//	cvAddonShowImageOnce(f_val);
//	cvAddonShowImageOnce(cv_val);
////	cvAddonShowImageOnce(ipp_val);
//

	return 0;
}