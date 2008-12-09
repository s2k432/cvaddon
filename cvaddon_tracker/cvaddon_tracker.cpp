#include "cvaddon_tracker.h"

#include "../cvaddon_util/cvaddon_draw.h"					// Visualization of Tracking Results

#include <iostream>
using std::cerr;
using std::endl;

// Number of standard deviations used in providing predicted r and theta ranges
static const float NUM_DEV = 3.0f;

// Measurement Error
static const float R[NUM_KF_MEASUREMENT*NUM_KF_MEASUREMENT] = 
{
	9.0f,		0,
	0, 		0.0027f
};
const float R_SCALE = 1.0f;

// System Model Error (Plant Error)
static const float Q[NUM_KF_STATE*NUM_KF_STATE] = 
{
	0.01,		0,			0,		0,			0,		0,
	0,			0.00003,	0,		0,			0,		0,
	0,			0,			0.1,	0,			0,		0,
	0,			0,			0,		0.0002f,	0,		0,
	0,			0,			0,		0,			0.1,	0,
	0,			0,			0,		0,			0,		0.0002f
};
const float Q_SCALE = 1.0f;

// System Model (Plant)
//static const float A[NUM_KF_STATE*NUM_KF_STATE] = 
//{	
//	1, 0, 1, 0,
//	0, 1, 0, 1,
//	0, 0, 1, 0,
//	0, 0, 0, 1
//};

static const float A[NUM_KF_STATE*NUM_KF_STATE] = 
{	
	1, 0, 1, 0, 0.5, 0,
	0, 1, 0, 1, 0, 0.5,
	0, 0, 1, 0, 1, 0,
	0, 0, 0, 1, 0, 1,
	0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 1
};


CvAddonSymmetryTracker :: CvAddonSymmetryTracker()
{
	// Creating Kalman Filter
	kalman = cvCreateKalman(NUM_KF_STATE, NUM_KF_MEASUREMENT, 0);	
	newMeasurement = cvCreateMat(NUM_KF_MEASUREMENT, 1, CV_32FC1);

	// Validation gate
	S = cvCreateMat(NUM_KF_MEASUREMENT, NUM_KF_MEASUREMENT, CV_32FC1);
	S_invert = cvCreateMat(NUM_KF_MEASUREMENT, NUM_KF_MEASUREMENT, CV_32FC1);
	v = cvCreateMat(NUM_KF_MEASUREMENT, 1, CV_32FC1);
	v_times_S_invert = cvCreateMat(1, NUM_KF_MEASUREMENT, CV_32FC1);
	z = cvCreateMat(NUM_KF_MEASUREMENT, 1, CV_32FC1);
	H_times_P = cvCreateMat(NUM_KF_MEASUREMENT, NUM_KF_STATE, CV_32FC1);
	vg_error = cvCreateMat(1, 1, CV_32FC1);
}

CvAddonSymmetryTracker :: ~CvAddonSymmetryTracker() 
{
	cvReleaseKalman(&kalman);
	cvReleaseMat(&newMeasurement);
	cvReleaseMat(&S);
	cvReleaseMat(&v);
	cvReleaseMat(&z);
	cvReleaseMat(&H_times_P);
	cvReleaseMat(&v_times_S_invert);
	cvReleaseMat(&vg_error);
	cvReleaseMat(&S_invert);
}


// Setting Kalman Filter Estimate (post-measurement state) to <startingState>
// Also clears and resets filter matrices A, R, Q, H and P
void CvAddonSymmetryTracker :: init(CvMat* startingState) 
{
	cvCopy(startingState, kalman->state_post);

	memcpy( kalman->transition_matrix->data.fl, A, sizeof(A));			// A
	cvSetIdentity( kalman->measurement_matrix, cvRealScalar(1) );		// H
	cvSetIdentity( kalman->error_cov_post, cvRealScalar(1));			// P

	memcpy ( kalman->measurement_noise_cov->data.fl, R, sizeof(R));		// R
	memcpy( kalman->process_noise_cov->data.fl, Q, sizeof(Q) );			// Q		

	// Testing - tweaking Q and R values
	cvScale(kalman->process_noise_cov, kalman->process_noise_cov, R_SCALE);
	cvScale(kalman->measurement_noise_cov, kalman->measurement_noise_cov, Q_SCALE);
}


CvAddonSymmetryTrackerEstimate CvAddonSymmetryTracker :: predict(float& minTheta, float& maxTheta
	, float& minR, float& maxR) 
{
	CvAddonSymmetryTrackerEstimate estimate;
	cvKalmanPredict(kalman);

	// Validation Gate Calculations
	// (H*P_)
	cvGEMM( kalman->measurement_matrix, kalman->error_cov_pre
		, 1 ,NULL, 0, H_times_P, 0);

	// S = (H*P_)*H' + R
	cvGEMM(H_times_P, kalman->measurement_matrix
		, 1 ,kalman->measurement_noise_cov, 1, S, CV_GEMM_B_T);		


	predictThetaRange(minTheta, maxTheta);
	predictRRange(minR, maxR);

	estimate.r = CV_MAT_ELEM(*(kalman->state_pre), float, 0, 0);
	estimate.theta = CV_MAT_ELEM(*(kalman->state_pre), float, 1, 0);

	return estimate;
}

// Overloaded version - doesn't provides min/max values
CvAddonSymmetryTrackerEstimate CvAddonSymmetryTracker :: predict()
{
	CvAddonSymmetryTrackerEstimate estimate;
	cvKalmanPredict(kalman);

	// Validation Gate Calculations
	// (H*P_)
	cvGEMM( kalman->measurement_matrix, kalman->error_cov_pre
		, 1 ,NULL, 0, H_times_P, 0);

	// S = (H*P_)*H' + R
	cvGEMM(H_times_P, kalman->measurement_matrix
		, 1 ,kalman->measurement_noise_cov, 1, S, CV_GEMM_B_T);		

	estimate.r = CV_MAT_ELEM(*(kalman->state_pre), float, 0, 0);
	estimate.theta = CV_MAT_ELEM(*(kalman->state_pre), float, 1, 0);

	return estimate;
}

CvAddonSymmetryTrackerEstimate CvAddonSymmetryTracker :: update(const CvAddonFastSymResults &measurements)
{
	CvAddonSymmetryTrackerEstimate estimate;
	float minError;
	int minIndex;

	// find best measurement using validate()
	// If nothing is valid, no measurement used
	if( validate(measurements, minError, minIndex) )
	{
		// Copying
		CV_MAT_ELEM(*newMeasurement, float, 0, 0) = measurements.symLines[minIndex].r;
		CV_MAT_ELEM(*newMeasurement, float, 1, 0) = measurements.symLines[minIndex].theta;

		cvKalmanCorrect(kalman, newMeasurement);
	}
	else {
		//cerr << "CvAddonSymmetryTracker::update(): No Valid Measurements!" << endl;
	
		cvCopy(kalman->state_pre, kalman->state_post);
	}

	//cerr << "CvAddonSymmetryTracker::update(): Min Error is " << minError << endl;

	estimate.r = CV_MAT_ELEM(*(kalman->state_post), float, 0, 0);
	estimate.theta = CV_MAT_ELEM(*(kalman->state_post), float, 1, 0);

	return estimate;
}


// Gets 3-standard-deviation predicted range for theta 
// RUN predict() BEFORE running this!!!
void CvAddonSymmetryTracker :: predictThetaRange(float& thMin, float& thMax) 
{
	float thRange = sqrtf( CV_MAT_ELEM(*S, float, 1, 1) ) * NUM_DEV;		
	float thPre = CV_MAT_ELEM(*(kalman->state_pre), float, 1, 0);

	//std::cerr << "thRange: " << thRange << std::endl;

	thMin = thPre - thRange;
	thMax = thPre + thRange;
}


// Gets 3-standard-deviation predicted range for r
// RUN predict() BEFORE running this!!!
void CvAddonSymmetryTracker :: predictRRange(float& rMin, float& rMax) 
{
	float rRange = sqrtf( CV_MAT_ELEM(*S, float, 0, 0) ) * NUM_DEV;		
	float rPre = CV_MAT_ELEM(*(kalman->state_pre), float, 0, 0);

	//std::cerr << "thRange: " << thRange << std::endl;

	rMin = rPre - rRange;
	rMax = rPre + rRange;
}



//// Fast Symmetry Detection Results
//struct CvAddonFastSymResults
//{
//	int numSym;	// Number of symmetry lines found
//	int maxSym;	// Number of symmetry lines allocated for (for getResult() mainly)
//	CvAddonFastSymLine *symLines;	// Allocated on the heap
//
//	CvAddonFastSymResults() : numSym(-1), maxSym(0) {}
//	CvAddonFastSymResults(const int& size) : numSym(-1)
//	{
//		if(size > 0) {
//			maxSym = size;
//			symLines = new CvAddonFastSymLine[size];
//		}
//	}
//	~CvAddonFastSymResults() { if(maxSym > 0) delete [] symLines; }
//};


// Checking measurements using validation gate
// Returns best measurement by value
// RUN predict() BEFORE running this!!!
bool CvAddonSymmetryTracker :: validate(const CvAddonFastSymResults &measurements, float& minError, int& minIndex)
{
	//CvPoint val = cvPoint(-1,-1);		
	minError = 1000000;
	minIndex = -1;
	
	// inv(S)
	cvInvert(S, S_invert);

	int i;
	for(i = 0; i < measurements.numSym; i++)
	{
		// Setting data z to peak value
		((float*)(z->data.ptr + z->step*0))[0] = measurements.symLines[i].r;
		((float*)(z->data.ptr + z->step*1))[0] = measurements.symLines[i].theta;

		// v = -H * x_ + z
		cvGEMM(kalman->measurement_matrix, kalman->state_pre
			, -1 ,z, 1, v, 0);

		// v' * inv(s)
		cvGEMM(v, S_invert, 1 , NULL, 0, v_times_S_invert, CV_GEMM_A_T);

		// error = v' * inv(s) * v
		cvGEMM(v_times_S_invert, v, 1 , NULL, 0, vg_error, 0);
				
		if( CV_MAT_ELEM(*vg_error, float, 0, 0) <= minError )
		{
			minError = CV_MAT_ELEM(*vg_error, float, 0, 0);
			minIndex = i;
		}			
	}

	// Checking error to make sure "best" data is valid
	// NEW - added sqrtf(), not sure if this is right...
	if(sqrtf(minError) >= SYMMETRY_TRACKER_CHI_ERROR)
		return false;

	return true;
}



void CvAddonSymmetryTracker::draw(const CvAddonFastSymResults& measurements, IplImage* dst
	, const float &minTheta, const float& maxTheta
	, const float &minR, const float &maxR
	, const CvScalar& predictionColour, const CvScalar& estimateColour
	, const CvScalar& thetaRangeColour, const CvScalar& measurementColour)
{
#define PRE_R CV_MAT_ELEM(*(kalman->state_pre), float, 0, 0)
#define PRE_TH	CV_MAT_ELEM(*(kalman->state_pre), float, 1, 0)

	cvAddonDrawPolarLine(dst, minR, minTheta
		, thetaRangeColour, 1);
	cvAddonDrawPolarLine(dst, minR, maxTheta
		, thetaRangeColour, 1);
	cvAddonDrawPolarLine(dst, maxR, minTheta
		, thetaRangeColour, 1);
	cvAddonDrawPolarLine(dst, maxR, maxTheta
		, thetaRangeColour, 1);

	cvAddonDrawPolarLine(dst, PRE_R, PRE_TH
		, predictionColour, 2);

	for(int n = 0; n < measurements.numSym; ++n)
	{
		cvAddonDrawPolarLine(dst, measurements.symLines[n].r
			, measurements.symLines[n].theta
			, measurementColour, 1);			
	}	

	cvAddonDrawPolarLine(dst, CV_MAT_ELEM(*(kalman->state_post), float, 0, 0)
		, CV_MAT_ELEM(*(kalman->state_post), float, 1, 0), estimateColour, 2);

#undef PRE_R
#undef PRE_TH
}


