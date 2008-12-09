#pragma once

#include <cv.h>	// OpenCV
#include "../cvaddon_fast_sym/cvaddon_fast_sym_detect.h"		// Symmetry detection

// Symmetry Line
struct CvAddonSymmetryTrackerEstimate
{
	float r, theta;
};

// Symmetry Line tracker using Kalman filter
// 
// Note: Filter should track r and theta values, not their hough indices, which may vary depending on 
// the quantization used
// Note that the states are {r, theta, r', theta', r'', theta''}

// Function Descriptions
// ---
// init(): sets the starting state of the filter. For example, this would be {r, theta, 0, 0, 0, 0} for static objects
// 
// predict(): returns the pre-measurement estimate (prediction) of the filter. It also returns the upper and lower angle
// bounds which a valid measurement must be within. This can be used to limit the voting angles used in the symmetry detector
//
// update(): returns the post-measurement estimate of the filter. The input parameter is simply the result of symmetry detection

// Example:
// 
// init();
// while( img = capture_from_camera() )
// {
//    predict(min, max);
//		symResult = detectSym(min,max);
//    estimate = update(symResults);
// }

class CvAddonSymmetryTracker
{
public:
	CvAddonSymmetryTracker();
	~CvAddonSymmetryTracker();

	// Sets initial state
	// startingState should be a vector (single-column CvMat)
	void init(CvMat* startingState);						

	// Filter prediction
	// Can also give validation gate bounds for next theta (used to speed up symmetry detection)
	CvAddonSymmetryTrackerEstimate predict();	
	CvAddonSymmetryTrackerEstimate predict(float& minTheta, float& maxTheta
		, float& minR, float& maxR);			

	// Updates filter with measurement nearest prediction
	// If no measurements lie within validation gate, uses prediction as updated state
	CvAddonSymmetryTrackerEstimate update(const CvAddonFastSymResults &measurements);

	// Draws Tracking Results onto an image for Visualization
	void CvAddonSymmetryTracker::draw(const CvAddonFastSymResults& measurements, IplImage* dst
		, const float &minTheta, const float& maxTheta
		, const float &minR, const float &maxR
		, const CvScalar& predictionColour = CV_RGB(255,0,0)
		, const CvScalar& estimateColour =	CV_RGB(0,255,0)
		, const CvScalar& thetaRangeColour = CV_RGB(255,128,0)
		, const CvScalar& measurementColour = CV_RGB(255,255,0) );


	CvKalman* kalman;			// OpenCV Kalman filter

	// Validation Gate Matrices
	CvMat *S, *S_invert, *v, *z;		
	CvMat *v_times_S_invert, *H_times_P, *vg_error;


private:
	CvMat* newMeasurement;		// Temp matrix for measurements

	// Finds expected theta range which next valid measurement should be within
	void predictThetaRange(float& thMin, float& thMax);		
	void predictRRange(float& rMin, float& rMax);

	// Validates measurements and finds index and error of best measurement. Used by update()
	// Returns false if no measurement is valid
	bool validate(const CvAddonFastSymResults &measurements, float& minError, int& minIndex);
};

// Chi-Square error used in Validation Gate
// Chi Square 2DOF
const float SYMMETRY_TRACKER_CHI_ERROR = 9.21f;		// 0.01
//const float SYMMETRY_TRACKER_CHI_ERROR = 5.99f;	// 0.05

const unsigned int NUM_KF_STATE = 6;
const unsigned int NUM_KF_MEASUREMENT = 2;





