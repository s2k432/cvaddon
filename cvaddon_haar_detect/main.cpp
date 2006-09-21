// main() to test CvAddonHaarDetector Class
// By Wai Ho Li

#include "cv.h"

#include "cvaddon_haar.h"
#include "cvaddon_display.h"
#include "cvaddon_draw.h"

#include "convert.h"

#include <iostream>
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
	int state = 0;

	cerr << "*** People Detector ***" << endl;
	cerr << "Written by Wai Ho Li" << endl;
	cerr << endl;
	cerr << "Description: " << endl;
	cerr << "Uses a stripped-down version of the OpenCV Haar Detector" << endl;
	cerr << "to find Faces (light blue), Full Bodies (Red), Upper Bodies (Green) and" << endl;
	cerr << "Lower Bodies (Blue) in images" << endl;
	cerr << endl;

	if(argc <= 1) {
		cerr << "USAGE: haar_detect.exe <image_name> <optional: cascade>" << endl;
		cerr << "{cascade} values: " << endl;
		cerr << "   0: Run all detectors: " << endl;
		cerr << "   1: Run FACE detector ONLY: " << endl;
		cerr << "   2: Run BODY detectors only" << endl;
		cerr << "{cascade} defaults to 0" << endl;
		exit(1);
	}

	if(argc > 2) {
		if( !convert(argv[2], state) )
			state = 0;
		if(state < 0 || state > 2) state = 0;
	}

	IplImage *img = cvLoadImage(argv[1]);

	if(img == NULL) {
		cerr << "ERROR: Could not load image: " << argv[1] << endl;
		exit(1);
	}


	int i;
	CvScalar fullBodyColor = CV_RGB(255,0,0);
	CvScalar lowerBodyColor = CV_RGB(0,255,0);
	CvScalar upperBodyColor = CV_RGB(0,0,255);
	CvScalar faceColor = CV_RGB(0,255,255);

	CvAddonHaarDetector hdFullBody(CV_HAAR_FULL_BODY_FILE);
	CvAddonHaarDetector hdLowerBody(CV_HAAR_LOWER_BODY_FILE);
	CvAddonHaarDetector hdUpperBody(CV_HAAR_UPPER_BODY_FILE);
	CvAddonHaarDetector hdFrontFace(CV_HAAR_FRONT_FACE_FILE);
	
	CvAddonHaarDetectResults haarResults;


	if(state <= 1) {
		hdFrontFace.detect(img, haarResults
			, 1.1f
			, 3
			, cvSize(20,20)
			, false
			, false);
		for(i = 0; i < haarResults.numObjects; ++i) {
			cvAddonDrawRectangle(img, haarResults.objects[i], faceColor);
		}
		cerr << "Face Detect Results: " << endl;
		cerr << haarResults << endl;
	}

	if(state == 0 || state == 2) {
		hdFullBody.detect(img, haarResults
			, 1.1f
			, 3
			, cvSize(20,20)
			, false
			, false);

		
		for(i = 0; i < haarResults.numObjects; ++i) {
			cvAddonDrawRectangle(img, haarResults.objects[i], fullBodyColor);
		}
		cerr << "Full Body Detect Results: " << endl;
		cerr << haarResults << endl;



		hdLowerBody.detect(img, haarResults
			, 1.1f
			, 3
			, cvSize(20,20)
			, false
			, false);

		for(i = 0; i < haarResults.numObjects; ++i) {
			cvAddonDrawRectangle(img, haarResults.objects[i], lowerBodyColor);
		}
		cerr << "Lower Body Detect Results: " << endl;
		cerr << haarResults << endl;



		hdUpperBody.detect(img, haarResults
			, 1.1f
			, 3
			, cvSize(20,20)
			, false
			, false);
		
		for(i = 0; i < haarResults.numObjects; ++i) {
			cvAddonDrawRectangle(img, haarResults.objects[i], upperBodyColor);
		}
		cerr << "Upper Body Detect Results: " << endl;
		cerr << haarResults << endl;
	}

	cvAddonShowImageOnce(img);

	return 0;
}