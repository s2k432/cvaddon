//#define MY_HSV

#include <iostream>
using std::cerr;
using std::endl;
#include "cvaddon_display.h"

#include "cv.h"
#include "highgui.h"

#include "cvaddon_hsv_filter.h"

#include "windows_fast_timer.h"

#ifdef _DEBUG
	static const int TRIALS = 1;	
#else
	static const int TRIALS = 1000;
#endif

int main()
{
	IplImage *in = cvLoadImage("test1.bmp");
	CvSize imgSize = cvGetSize(in);

	cvAddonShowImageOnce(in);

	IplImage *h = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *s = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *v = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *bp = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);

	IplImage *histImg = cvCreateImage(cvSize(64, 360), IPL_DEPTH_8U, 3);

	CvAddonHSVFilter hsvFilter;

	

	int i;
	FastTimer t0;

	t0.getLoopTime();
	for(i = 0; i < TRIALS; ++i)
		hsvFilter.buildHist(in, h, s, v, cvScalar(-1,-1,-1), cvScalar(256,256,256), NULL);

	cerr << "buildHist: " << t0.getLoopTime() / (float)TRIALS << endl;

	hsvFilter.drawHist(histImg);
	cvAddonShowImageOnce(histImg);

	exit(1);

	t0.getLoopTime();
	for(i = 0; i < TRIALS; ++i)
		hsvFilter.backProject(in, h, s, v, bp, cvScalar(-1,-1,-1), cvScalar(256,256,256), NULL);
	cerr << "bp: " << t0.getLoopTime() / (float)TRIALS << endl;

	cvAddonShowImageOnce(bp);

	const double alpha = 0.5;
	
	t0.getLoopTime();
	for(i = 0; i < TRIALS; ++i)
		hsvFilter.blendHist(in, h, s, v, cvScalar(-1,-1,-1), cvScalar(256,256,256), NULL, alpha);
	cerr << "blend: " << t0.getLoopTime() / (float)TRIALS << endl;

	hsvFilter.drawHist(histImg);
	cvAddonShowImageOnce(histImg);


	hsvFilter.backProject(in, h, s, v, bp, cvScalar(-1,-1,-1), cvScalar(256,256,256), NULL);

	cvAddonShowImageOnce(bp);

//	cvEqualizeHist(bp, v);
//
//	cvAddonShowImageOnce(v);

	return 0;
}










#ifdef MY_HSV

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
	IplImage *in = cvLoadImage("test0.bmp");
	CvSize imgSize = cvGetSize(in);

	cvAddonShowImageOnce(in);

	IplImage *hsv = cvCloneImage(in);
	IplImage *f_hue = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *f_sat = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	IplImage *f_val = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);

	// TESTING: Generic 2D filter
	int i;
	const int TRIALS_O_O = 250;
	FastTimer t0;

	cvAddonInitHsvLut();
	cvAddonBGR2HSV_LUT(in, f_hue, f_sat, f_val);

	cvAddonShowImageOnce(f_hue);
	cvAddonShowImageOnce(f_sat);
	cvAddonShowImageOnce(f_val);

	CvAddonFilter2D<uchar> filt(45, 8, 179, 255);

	t0.getLoopTime();
	for(i = 0; i < TRIALS_O_O; ++i)
		filt.buildHist(f_hue, f_sat, 0, 179, 0, 255, true, false, f_val);

	cerr << "Hist Build: " << t0.getLoopTime() / (float)TRIALS_O_O << endl;
//	cerr << filt << endl;

	IplImage *bp = cvCloneImage(f_val);
	cvZero(bp);

	t0.getLoopTime();
	for(i = 0; i < TRIALS_O_O; ++i)
		filt.backProject(f_hue, f_sat, bp, 0, 179, 0, 255, f_val);

	cerr << "Hist BP: " << t0.getLoopTime() / (float)TRIALS_O_O << endl;

	cvAddonShowImageOnce(bp);

//	cvAddonSaveFilter2D(&filt, "test.txt");
//	CvAddonFilter2D<uchar> *filtOld = cvAddonLoadFilter2D<uchar>("test.txt");
//	
//	if(filtOld)
//		cerr << *filtOld << endl;

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

#endif

//#ifdef CV_HSV
//
//static const int TRIALS = 250;
//
//#include <cv.h>
//#include <highgui.h>
//
//#include "windows_fast_timer.h"
//
//#include <iostream>
//using std::cerr;
//using std::endl;
//
//int main( int argc, char** argv )
//{
//    IplImage* src;
////    if( argc == 2 && (src=cvLoadImage(argv[1], 1))!= 0)
////    {
//		src = cvLoadImage("filter2d_test3.bmp");
//
//        IplImage* h_plane = cvCreateImage( cvGetSize(src), 8, 1 );
//        IplImage* s_plane = cvCreateImage( cvGetSize(src), 8, 1 );
//        IplImage* v_plane = cvCreateImage( cvGetSize(src), 8, 1 );
//        IplImage* planes[] = { h_plane, s_plane };
//        IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3 );
////        int h_bins = 30, s_bins = 32;
//        int h_bins = 45, s_bins = 8;
//        int hist_size[] = {h_bins, s_bins};
//        float h_ranges[] = { 0, 180 }; /* hue varies from 0 (~0°red) to 180 (~360°red again) */
//        float s_ranges[] = { 0, 255 }; /* saturation varies from 0 (black-gray-white) to 255 (pure spectrum color) */
//        float* ranges[] = { h_ranges, s_ranges };
//        int scale = 10;
//        IplImage* hist_img = cvCreateImage( cvSize(h_bins*scale,s_bins*scale), 8, 3 );
//        CvHistogram* hist;
//        float max_value = 0;
//        int h, s;
//
//        cvCvtColor( src, hsv, CV_BGR2HSV );
//        cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0 );
//        hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY, ranges, 1 );
//
//		int i;
//		FastTimer t0;
//
//		t0.getLoopTime();
//		for(i = 0; i < TRIALS; ++i)
//			cvCalcHist( planes, hist, 0, 0 );
//		
//		cerr << t0.getLoopTime() / (float)TRIALS << endl;
//
//		cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
//        cvZero( hist_img );
//
//        for( h = 0; h < h_bins; h++ )
//        {
//            for( s = 0; s < s_bins; s++ )
//            {
//                float bin_val = cvQueryHistValue_2D( hist, h, s );
//                int intensity = cvRound(bin_val*255/max_value);
//                cvRectangle( hist_img, cvPoint( h*scale, s*scale ),
//                             cvPoint( (h+1)*scale - 1, (s+1)*scale - 1),
//                             CV_RGB(intensity,intensity,intensity), /* graw a grayscale histogram.
//                                                                       if you have idea how to do it
//                                                                       nicer let us know */
//                             CV_FILLED );
//            }
//        }
//
//        cvNamedWindow( "Source", 1 );
//        cvShowImage( "Source", src );
//
//        cvNamedWindow( "H-S Histogram", 1 );
//        cvShowImage( "H-S Histogram", hist_img );
//
//        cvWaitKey(0);
////    }
//}
//
//#endif