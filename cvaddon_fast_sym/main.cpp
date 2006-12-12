// Testing Fast Symmetry Code (version 2)

#include <cv.h>
#include <highgui.h>

#include "fast_sym.h"
#include "cvaddon_fast_sym_detect.h"

#include "cvaddon_draw.h"
#include "cvaddon_display.h"

#include "windows_fast_timer.h"

const int NUM_DRAW_PEAKS = 3;
const double CANNY_TH[2] = {30.0, 90.0};
const int TRIALS = 1;

#include <iostream>
using std::cerr;
using std::endl;

int main()
{
	FastTimer t0;

	IplImage *in = cvLoadImage("test_sym_cw30deg.png");
	IplImage *inGray = cvCreateImage(cvGetSize(in), IPL_DEPTH_8U, 1);
	IplImage *inEdge = cvCreateImage(cvGetSize(in), IPL_DEPTH_8U, 1);

	float rDivs = sqrtf(in->width*in->width + in->height*in->height);
	int thDivs = 180;	
	CvAddonFastSymDetector symDetector(cvGetSize(in), cvSize(rDivs, thDivs));

	cvCvtColor(in, inGray, CV_BGR2GRAY);
	cvCanny(inGray, inEdge, CANNY_TH[0], CANNY_TH[1]);

	cvAddonShowImageOnce(inEdge);

	t0.getLoopTime();
	for(int i = 0 ; i < TRIALS; ++i)
		symDetector.vote(inEdge, 25, 250);
	cerr << t0.getLoopTime() / (float)TRIALS << endl;

	cvAddonShowImageOnce(symDetector.H);

//	symDetector.vote(inEdge, 0, 10000);
//	cvAddonShowImageOnce(symDetector.H);	


	CvAddonFastSymResults symResults(10);
	symDetector.getResult(1, symResults);

	for(int p = 0; p < symResults.numSym; ++p) {
		cerr << "IDX: " << symResults.symLines[p].rIndexRaw 
			<< "," << symResults.symLines[p].thetaIndexRaw << endl;
		float r = symDetector.getPixelFromIndex(symResults.symLines[p].rIndexRaw);
		float theta = symDetector.getRadiansFromIndex(symResults.symLines[p].thetaIndexRaw);
		cerr << r << "," << theta * 180.0f / CV_PI << endl;
	}


	return 0;
}