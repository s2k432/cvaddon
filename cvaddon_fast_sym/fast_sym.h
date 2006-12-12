#ifndef _FAST_SYM2_H
#define _FAST_SYM2_H

// TODO: Vectorize everything!
//#ifndef _DEBUG
//	#pragma vector always
//#endif


// -----------------------
// Fast Symmetry Detection (version 2)
// -----------------------
// Written by Wai Ho Li
// First Written on 23 Aug 2006
// Last Modification 24 Aug 2006

#include <cv.h>

#include <fstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <algorithm>
using std::sort;

#include "mem_util.h"

#include "cvaddon_edge.h"
#include "fast_sym_data.h"

//#ifdef _DEBUG
	#include "cvaddon_display.h"
//#endif



// Data Types
#define RotatedDataType float
#define AccumType float

// Maximum number of edge pixels extracted
// from input edge image
#ifndef MAX_EDGE_PIXELS
	#define MAX_EDGE_PIXELS 60000
#endif

// Spread hough votes in 3x3 window
#define _SPREAD_VOTES

// Interpolate votes in r direction
#define _INTERPOLATE_VOTES


// EdgeType should be CvPoint, but can be CvPoint32f if float locations can be found
// RotatedDataType can be either int, float or double
class FastSymDetector
{
public:
	FastSymDetector(const FastSymInitParams &params);
	~FastSymDetector();

	// Accumulates symmetry contributions of edge pixels in <edgeImg>
	// into Hough Accumulator H and returns it
	AccumType **accum(const IplImage *edgeImg, const FastSymAccumParams &params);
	
	// Finds peaks in the Hough Accumulator H. Results are returned in the
	// <results> strucutre. 
	// 
	// Normally, this function will destroy information 
	// in the Hough accumulator. By specifying <destroyHoughAccum> as false,
	// the data in the accumulator remains unchanged.
	void findPeak(const FastSymPeakFindParams &params
		, FastSymPeakFindResults &results
		, bool destroyHoughAccum = true);

	// Dumps last known parameters (state of detector) to a text file
	// <comment> is written to the text file before the parameter data.
	void writeParamsToFile(const char *comment
		, const char *filename = "fast_sym_params.txt") const;

	// Converts hough index to pixel and angle values
	float getRadiansFromIndex(const float &thetaIdx);
	float getPixelFromIndex(const float &rIdx);
	
	// Hough Accumlation
	AccumType **H;				// Hough Accumulator
	AccumType *HRow;			// A row in Hough Accumulator
	AccumType **houghBackUp;	// Copy of Hough Accumulator

private:
	CvPoint2D32f edges[MAX_EDGE_PIXELS];
	int numEdges;

	// Edge Pixel Rotation and Discretization
	float **RMatrices;
	RotatedDataType **rotatedEdges;		// Rotated edge (x-coordinate) accumulator
	RotatedDataType **reRowPtrs;		// Ptr to end of rows in <rotatedEdges>

//	// Hough Accumlation
//	AccumType **H;				// Hough Accumulator
//	AccumType *HRow;			// A row in Hough Accumulator
//	AccumType **houghBackUp;	// Copy of Hough Accumulator
	
	// Peak Finding
	CvMat *diff, *hessian, *shift;

	// Internal Paramters
	FastSymInitParams initParams;			// Set at construction
	FastSymAccumParams accumParams;			// Set when accumulating into H
	FastSymPeakFindParams peakFindParams;	// Set when find peaks in H

	FastSymConstants C;	// Constants used in Accumulation and Peak Finding

	void rotateStoredEdges(const unsigned int& thetaIdx);
	void drawRotatedEdges(IplImage *img, const CvScalar &color = CV_RGB(255, 255, 255));
};

// Theta is shifted (lob-sided) to prevent angle overlap. See rotation matrices
// <RMatrices> that are calculated in the class constructor FastSymDetector()
inline float FastSymDetector::getRadiansFromIndex(const float &thetaIdx)
{
	return (thetaIdx - 0.5f - C.TH_DIVS_2) * C.I2TH_C;
}
	
inline float FastSymDetector::getPixelFromIndex(const float &rIdx)
{
	return (rIdx - C.R_DIVS_2) * C.I2R_C;
}



#endif
