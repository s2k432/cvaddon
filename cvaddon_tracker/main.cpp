// Kalman filter Symmetry line tracker test code

#include <iostream>
using std::cerr;
using std::endl;

#include "cvaddon_tracker.h"

#include <cv.h>
#include "../cvaddon_util/cvaddon_print.h"

#define VAL(x, i, j) CV_MAT_ELEM(*(x), float, (i), (j) )

int main()
{
	CvAddonSymmetryTrackerEstimate prediction, estimate;
	CvAddonFastSymResults fastSymResults(1);
	CvAddonFastSymLine symLine;

	CvMat *startingState = cvCreateMat(6, 1, CV_32FC1);
//	CvMat *measurement = cvCreateMat(2, 1, CV_32FC1);
	cvZero(startingState);
//	cvZero(measurement);

	VAL(startingState, 0, 0) = 5.0f;
	VAL(startingState, 1, 0) = 1.0f;

	cerr << "Starting State: " << startingState << endl;

	CvAddonSymmetryTracker tracker;

	tracker.init(startingState);

	cerr << "Kalman state-pre: " << endl;
	cerr << tracker.kalman->state_pre << endl;


//	VAL(measurement, 0, 0) = 4.0f; 
//	VAL(measurement, 1, 0) = 1.0f;

	float minTheta, maxTheta;
	prediction = tracker.predict(minTheta, maxTheta);

//	cerr << "Prediction: " << prediction.r << ", " << prediction.theta << endl;
//	cerr << "Prediction for Measurement Theta: " << minTheta << " < " << maxTheta << endl;
//
//	cerr << "Kalman state-pre: " << endl;
//	cerr << tracker.kalman->state_pre << endl;
//
//	cerr << "Kalman pre-noise: " << endl;
//	cerr << tracker.kalman->error_cov_pre << endl;



	int i;
	for(i = 0; i < 50000; ++i) 
	{
		prediction = tracker.predict(minTheta, maxTheta);
		
		if(i % 10000 == 0) {
			cerr << "Prediction: " << prediction.r << ", " << prediction.theta << endl;
			cerr << "Prediction for Measurement Theta: " << minTheta << " < " << maxTheta << endl;

			cerr << "S: " << endl;
			cerr << tracker.S << endl;
			cerr << "P(k-1): " << endl;
			cerr << tracker.kalman->error_cov_pre << endl;
			cerr << "P(k): " << endl;
			cerr << tracker.kalman->error_cov_post << endl;
		}

//		symLine.r = 5.0f - (float)i;
//		symLine.theta = 1.0f - ((float)i) / 60.0f;

		symLine.r = 5.0f;
		symLine.theta = 1.0f;

		fastSymResults.symLines[0] = symLine;
		fastSymResults.numSym  = 1; 


	estimate = tracker.update(fastSymResults);



		
//		cerr << "estimate: " << estimate.r << ", " << estimate.theta << endl;


		if(i % 10000 == 0) {
			cerr << "New Measurement: " << symLine.r << ", " << symLine.theta << endl;
			cerr << "state: " << endl;
			cerr << tracker.kalman->state_post << endl;

			cerr << "=======================" << endl;
		}
	}


	return 0;
}