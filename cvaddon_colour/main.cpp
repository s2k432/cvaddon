#include "cvaddon_hsv_filter.h"
#include "cv.h"
#include "highgui.h"

#include "cvaddon_display.h"

#include "windows_fast_timer.h"


int main()
{
	IplImage *in = cvLoadImage("test1.bmp");
	CvSize imgSize = cvGetSize(in);

	cvAddonShowImageOnce(in);

	IplImage *hsv = cvCloneImage(in);
	IplImage *f_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *f_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	
	IplImage *cv_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *cv_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *cv_val = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);


//	TESTING: Time Trials
//	const int TRIALS = 1000;
//	int i;
//	float timeForTrials;
//	FastTimer t0;
//
//	cvAddonInitHsvLut();
//	t0.getLoopTime();
//	for(i = 0; i < TRIALS; ++i) {
//		cvAddonBGR2HueSat(in, f_hue, f_sat);
//	}
//	timeForTrials = t0.getLoopTime();
//	cerr << timeForTrials << endl;
//
//	t0.getLoopTime();
//	for(i = 0; i < TRIALS; ++i) {
//		cvCvtColor(in, hsv, CV_BGR2HSV);
//	}
//	timeForTrials = t0.getLoopTime();
//	cerr << "CV: " << timeForTrials << endl;


//  TESTING: Visual Inspection of colour conversion
//	cvSplit(hsv, cv_hue, cv_sat, cv_val, NULL);
//
//	cvAddonShowImageOnce(f_hue);
//	cvAddonShowImageOnce(cv_hue);
//
//	cvAddonShowImageOnce(f_sat);
//	cvAddonShowImageOnce(cv_sat);



	return 0;
}