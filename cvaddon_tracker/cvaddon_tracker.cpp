#include "cvaddon_tracker.h"

const unsigned int NUM_KF_STATE = 6;
const unsigned int NUM_KF_MEASUREMENT = 2;

#include <iostream>
using std::cerr;
using std::endl;

// Measurement Error
static const float R[NUM_KF_MEASUREMENT*NUM_KF_MEASUREMENT] = 
{
	9.0f, 0,
	0, 	0.0027f //9.0f
};

// System Model (Plant)
static const float A[NUM_KF_STATE*NUM_KF_STATE] = 
{	
	1, 0, 1, 0, 0.5, 0,
	0, 1, 0, 1, 0, 0.5,
	0, 0, 1, 0, 1, 0,
	0, 0, 0, 1, 0, 1,
	0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 1
};

// System Model Error (Plant Error)
static const float Q[NUM_KF_STATE*NUM_KF_STATE] = 
{
	0.01,		0,			0,		0,			0,		0,
	0,		0.00003,	0,		0,			0,		0,
	0,		0,			0.1,		0,			0,		0,
	0,		0,			0,		0.0002f,	0,		0,
	0,		0,			0,		0,			0.1,		0,
	0,		0,			0,		0,			0,		0.0002f
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
	memcpy ( kalman->measurement_noise_cov->data.fl, R, sizeof(R));		// R
	memcpy( kalman->process_noise_cov->data.fl, Q, sizeof(Q) );			// Q		
	cvSetIdentity( kalman->measurement_matrix, cvRealScalar(1) );		// H
	cvSetIdentity( kalman->error_cov_post, cvRealScalar(1));			// P
}


CvAddonSymmetryTrackerEstimate CvAddonSymmetryTracker :: predict(float& minTheta, float& maxTheta) 
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
		cvCopy(kalman->state_pre, kalman->state_post);
	}

	estimate.r = CV_MAT_ELEM(*(kalman->state_post), float, 0, 0);
	estimate.theta = CV_MAT_ELEM(*(kalman->state_post), float, 1, 0);

	return estimate;
}



// Gets 3-standard-deviation predicted range for r,theta 
// RUN predict() BEFORE running this!!!
void CvAddonSymmetryTracker :: predictThetaRange(float& thMin, float& thMax) 
{
	const float NUM_DEV = 3.0f;
	float thRange = sqrtf( CV_MAT_ELEM(*S, float, 1, 1) ) * NUM_DEV;		
	float thPre = CV_MAT_ELEM(*(kalman->state_pre), float, 1, 0);

	//std::cerr << "thRange: " << thRange << std::endl;

	thMin = thPre - thRange;
	thMax = thPre + thRange;
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
	minError = SYMMETRY_TRACKER_CHI_ERROR;
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
	// 2DOF 0.01 Chi Square used
	if(minError > SYMMETRY_TRACKER_CHI_ERROR)
		return false;

	return true;
}

