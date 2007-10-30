#pragma once

#include <cv.h>

#include <vector>
using std::vector;


#include "../cvaddon_fast_sym/cvaddon_fast_sym_detect.h"


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
//	  symResult = detectSym(min,max);
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
	// Also gives validation gate bounds for next theta (used to speed up symmetry detection)
	CvAddonSymmetryTrackerEstimate predict(float& minTheta, float& maxTheta);			

	// Updates filter with measurement nearest prediction
	// If no measurements lie within validation gate, uses prediction as updated state
	CvAddonSymmetryTrackerEstimate update(const CvAddonFastSymResults &measurements);

	// OpenCV Kalman filter
	// **Make this public for tweaking params during run time if needed
	CvKalman* kalman;			// Filter

	// Validation Gate
	CvMat *S, *S_invert, *v, *z;
	CvMat *v_times_S_invert, *H_times_P, *vg_error;


private:
	CvMat* newMeasurement;		// Temp matrix for measurements


	// Functions
	void predictThetaRange(float& thMin, float& thMax);		// Finds expected theta range which next valid measurement should be within

	// Validates measurements and finds index and error of best measurement. Used by update()
	// Returns false if no measurement is valid
	bool validate(const CvAddonFastSymResults &measurements, float& minError, int& minIndex);
};

// Chi-Square error used in Validation Gate
// Chi Square 2DOF
const float SYMMETRY_TRACKER_CHI_ERROR = 9.21f;		// 0.01
//const float SYMMETRY_TRACKER_CHI_ERROR = 5.99f;		// 0.05




