#ifndef _CVADDON_FAST_SYM_DETECT_H
#define _CVADDON_FAST_SYM_DETECT_H

//////////////////////////////////////////////////
// Version 3 (well, 2.5...) of Fast Symmetry 
// detection code, making use of OpenCV and
// cvaddon functions
//
// By Wai Ho Li
//////////////////////////////////////////////////
// *** USAGE NOTES ***
// - Recommended R bins: Image diagonal (+1 if result is even)
// - Recommended Theta bins: 180 (1 degree resolution)
//
// *** TODO ***
// - Non-Maxima suppression in Peak Find
// - Angle limits in voting AND peak find
// - R limits in peak find
// - Row thresholds for voting (Row masking?)
// - Row Sampling (Row masking?)
// 
// *** POSSIBLE TODOS
// - Speed up rotation (use integer pixel values, quick rotation etc...)
// - Reject large abs(r) or abs(x) votes (most likely garbage) <-- needs maths
// 
// *** FEATURES ***
// - Wraps around theta (using two rows of padding in Hough Accumulator)
//   during peak finding
// - Peak finding can also blur accumulator to allow for less aliased results
//
// *** IMPLEMENTATION NOTES ***
// A padded (+1 row on top and bottom) Hough accumulator is used to allow 
// wrap around of theta (angle) from -90 <--> 90 degrees. This allows for 
// sub-pixel localization of peaks right up to the edge of the accumulator 
// in angle. This is the reason for the -1 in getRadiansFromIndex()
//
// To ensure symmetric voting along r, about the center of the Hough accumulator, we
// subtracted 1 from the calculation of I2R_C. The +0.5 in getPixelFromIndex() is for
// centering purposes, to effectively give rIdx - (R - 1)/2. 
//
// *** IMPLEMENTATION & DESIGN DESICISIONS ***
// Spread Voting (with any kernel?) with Gaussian found to be the same as doing single votes
// followed by blurring after the votes.

// Interpolating along r is *probably* not worthwhile (see version 2 code with ACRA stereo paper)
// as theta accuracy is generally the dominant error source (+- 1 degree is much worst than 1 pixel error in R)
//
// Also tried using a sort(rRow, rEnd) so that the fabsf() call in the inner loop is not needed. However, 
// it seems to only slow things down on my test images. Hard to say which one is faster in practice, as
// the sort() speed will depend on memory access and the library implementation. fabsf() also depends on 
// implementation, but I feel requires fewer assumptions and has a more consistent speed. 
//
// *** Things I have Noticed ***
// - Symmetry detection accuracy in theta decreases when using sub-pixel localization 
//   with blurring when operating on synthetic images at "round" angles such as 0, 45 and 90
//   Overall, I am not sure whether sub-pixel refinement (using parabola hessian) is worthwhile.
//   Background noise, which in general, is very high, maybe causing the drop in quality.


#include "cv.h"

// Forward Declaration
struct CvAddonFastSymResults;

#ifndef MAX_EDGE_PIXELS
	#define MAX_EDGE_PIXELS 60000
#endif

class CvAddonFastSymDetector
{
public:
	// houghSize specified with r = width, theta = height
	CvAddonFastSymDetector(const CvSize &imgSize, const CvSize &houghSize, const int &rotEdgeRowHeight = 1);
	~CvAddonFastSymDetector();	

	// Voting (Hough Accumulation) function
	void vote( const IplImage *src
		, const int &minPairDist, const int &maxPairDist
		, const bool &angleLimits = false, const float &minThetaDeg = 0.0f, const float &maxThetaDeg = 0.0f
		, const float &sampleRotEdgeRatio = -1.0f	// if > 0 && < 1, forces subsampling of rotated edge pixels after discretization
		, const int &maxRotEdgeRowSize = -1			// if > 0, specifies maximum number of rotated edge pixels allowed per row
		, const float &maxRowEdgeRowRatio = -1.0f	// if > 0, specifies maximum number of rotated edge pixels allowed per row rel. to global mean
		);

	// Finds peaks in Hough Accumulator. Only call this after at least one vote() call. 
	// rNBHDivs and thetaNBHDivs controls the suppression neighbourhood size (as R_DIVS/rNBHDivs etc)
	void getResult(const int &maxPeaksFound, CvAddonFastSymResults &dst
		, const float &rNBHDivs = 10.0f, const float &thetaNBHDivs = 10.0f
		, const bool &smoothBins = true		// Smoothing Hough accum before peak find
		, const bool &limitRange = false
		, const float &r0 = 0.0f,  const float &r1 = 0.0f
		, const float &th0 = 0.0f,  const float &th1 = 0.0f
		, const float &threshToZero = -1.0f		// if > 0, peaks with height < threshToZero are set to 0
		, const float &threshRelMaxPeak = -1.0f	// if > 0, peaks < threshRelMaxPeak * maxPeak are set to 0
		);

	float getRadiansFromIndex(const float &thetaIndex) {
		return (thetaIndex - THETA_DIVS_2  - 1.0f) * I2TH_C;	// Bins centered at 0.0 
	}
	float getPixelFromIndex(const float &rIndex) {
		return (rIndex - R_DIVS_2 + 0.5f) * I2R_C;
	}

	// Constants
	const unsigned int IMG_W, IMG_H;
	const float IMG_CEN_X, IMG_CEN_Y;	// Image Center
	const float IMG_DIAG, IMG_DIAG_2;
	
	const float ROT_RES;			// Pixel Resolution of rotEdge rows 
	const unsigned int ROT_DIVS; 	// Rows of Rot (rotated edge array)
	const float ROT_DIVS_2;

	const unsigned int R_DIVS, THETA_DIVS;
	const float R_DIVS_2, R_DIVS_4, THETA_DIVS_2;
	const float THETA_INC, THETA_INC_DEG;

	// DEBUG
	CvMat *H, *HBackUp, *HMask;	// Hough Accumulator

private:
//	CvMat *H, *HBackUp;	// Hough Accumulator

	CvMat **rotMats;		// Rotation Matrices
	CvMat *rotatedEdges;	// Edge pixel rotation and discretization
	float **reRowPtrs;		// Pointers to rows of rotEdge

	CvMat *peakNBH, *diff, *hessian, *shift; // Peak Sub-pixel Localization

	// Push-forward constants (moving calculation outside vote inner loop)
	const float R2I_C, R2I_C_2, I2R_C, I2TH_C;

	// Buffer for edge pixel locations
	CvPoint2D32f edges[MAX_EDGE_PIXELS];

	void rotateStoredEdges(const unsigned int& thetaIdx, const unsigned int& numEdges);
};


// Fast Symmetry Line data structure
struct CvAddonFastSymLine
{
	float numOfVotes;	// Number of votes for this symmetry line
	float r, theta;		// r in pixels, theta in radians (from refined indices)
	int rIndexRaw, thetaIndexRaw;	// Unrefined Indices
	float rIndex, thetaIndex;		// Sub-pixel (hence the float) refined indices
};

// Fast Symmetry Detection Results
struct CvAddonFastSymResults
{
	int numSym;	// Number of symmetry lines found
	int maxSym;	// Number of symmetry lines allocated for (for getResult() mainly)
	CvAddonFastSymLine *symLines;	// Allocated on the heap

	CvAddonFastSymResults() : numSym(-1), maxSym(0) {}
	CvAddonFastSymResults(const int& size) : numSym(-1)
	{
		if(size > 0) {
			maxSym = size;
			symLines = new CvAddonFastSymLine[size];
		}
	}
	~CvAddonFastSymResults() { if(maxSym > 0) delete [] symLines; }
};

#endif