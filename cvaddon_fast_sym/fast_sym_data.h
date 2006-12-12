#ifndef _FAST_SYM_DATA_H
#define _FAST_SYM_DATA_H
// ----------------------------
// Fast Symmetry Detection Data
// ----------------------------
// Written by Wai Ho Li
// First Written on 23 Aug 2006
// Last Modification 24 Aug 2006

#include <cv.h>

#include <iostream>
using std::endl;

// Parameters that must be defined before 
// creating detector
struct FastSymInitParams
{
	int imgWidth, imgHeight;	// Input image size	
	int rDivs, thetaDivs;		// Hough accumulator size
	float pixelResolution;	// Pixel resolution of <rotateEdges>
};

inline std::ostream& operator<< (std::ostream &o, const FastSymInitParams &f)
{
	return o << "Image Size: " << f.imgWidth << " by " << f.imgHeight << "\n"
		<< "Hough Accum. Dim. (R by THETA): " << f.rDivs << " by " << f.thetaDivs << "\n"; 
}


// Parameters required for Hough accumulation
struct FastSymAccumParams
{
	int minPairDist, maxPairDist;		// Distance Thresholds for edge pairs
	int startThetaIdx, endThetaIdx;	// Angle Limit Indices (can wrap around)
	float rowCountThreshRatio;					// Maximum allowable pixels per row (% of global average)
	int rowCountThresh;							// Hard threshold of allowable pixels per row
//	float sampleRatio;							// Sampling Ratio for reducing the number of edge pixels used
};

inline std::ostream& operator<< (std::ostream &o, const FastSymAccumParams &f)
{
	return o << "Pairing Distance Thresholds: " << f.minPairDist << ", " << f.maxPairDist << "\n"
		<< "Theta Limits (Indices): " << f.startThetaIdx << " to " << f.endThetaIdx << "\n"
		<< "Row Count Threshold Ratio: " << f.rowCountThreshRatio << "\n"
		<< "Row Count Threshold (NOT RATIO!): " << f.rowCountThresh << "\n";
//		<< "Sample Ratio: " << f.sampleRatio;
}


// Parameters for Peak Finding in Hough space

struct FastSymPeakFindParams
{
	int maxPeaksFound;		// Max number of peaks returned
	float thresholdRatio;			// Ratio of peak used to threshold accumulator
	int rNBH, thetaNBH;	// Width (from centre point to edge) of suppression neighbourhoods
	
	int startThetaIdx, endThetaIdx;	// Angle Limit Indices (can wrap around)
	int startRIdx, endRIdx;			// Radius Limit Indices (can wrap around)
};

inline std::ostream& operator<< (std::ostream &o, const FastSymPeakFindParams &f)
{
	return o << "Maximum Peaks Found: " << f.maxPeaksFound << "\n"
		<< "Threshold Ratio: " << f.thresholdRatio << "\n"
		<< "Suppression Neighbourhoods {R, THETA}: " << f.rNBH << ", " << f.thetaNBH<< "\n"
		<< "Theta Limits (Indices)" << f.startThetaIdx << " to " << f.endThetaIdx << "\n"
		<< "R Limits (Indices)" << f.startRIdx << " to " << f.endRIdx;
}


// Results of Peak Finding in Hough Space (Symmetry Results
// PointType must have members named x and y (locations of peak)
// AccumType should be either int or float
struct FastSymPeakFindResults
{
	CvPoint2D32f *peaks;	// Sub-index localized peaks
	CvPoint *rawPeaks;		// Raw peak indices
	float *peakVals;		// Values (height) of peaks
	int numPeaks;
	int maxPeaks;

	FastSymPeakFindResults(const int& _maxPeaks)
		: numPeaks(0), maxPeaks(_maxPeaks)
	{
		peaks = new CvPoint2D32f[maxPeaks];
		rawPeaks = new CvPoint[maxPeaks];
		peakVals = new float[maxPeaks];
	}

	~FastSymPeakFindResults()
	{
		delete [] peaks;
		delete [] rawPeaks;
		delete [] peakVals;
	}
};

inline std::ostream& operator<< (std::ostream &o, const FastSymPeakFindResults &f)
{
	int i;
	if(f.peaks == NULL || f.numPeaks == 0) return o << "NO PEAKS";

	for(i = 0; i < f.numPeaks; ++i) {
		o << i << ": " << f.peaks[i].x << "\t" << f.peaks[i].y << "\t" << f.peakVals[i] << endl;
	}
	return o;
}

struct FastSymConstants
{
	int IMG_DIAGONAL;				// Diagonal of input image in pixels (rounded up)
	int ROTATED_EDGES_DIVS;		// Number of divisons in <rotatedEdges>
	float ROTATED_EDGES_DIVS_2;		// ROTATED_EDGES_DIVS / 2
	
	int X_OFFSET;			// Center of input image
	int Y_OFFSET;			// Center of input image

	float THETA_INC;		// Theta increment in radians
	float THETA_INC_DEG;	// Theta increment in degrees

	float R2I_C;			// r to r index constant
	float R2I_C_2;			// R2I_C / 2
	float I2R_C;			// r index to r constant

	float I2TH_C;			// theta index to theta in RADIANS
	float I2TH_C_2;			// ^above / 2

	float TH_DIVS_2;
	float R_DIVS_2;
	float R_DIVS_4;
};

inline std::ostream& operator<< (std::ostream &o, const FastSymConstants &f)
{
	return o 
		<< "Image Diagonal: " << f.IMG_DIAGONAL << "\n"
		<< "Number of Divisions in <rotatedEdges>: " << f.ROTATED_EDGES_DIVS << "\n"
		<< "Number of Divisions in <rotatedEdges> / 2: " << f.ROTATED_EDGES_DIVS_2 << "\n"
		
		<< "X Offset in Pixels (~= center of image): " << f.X_OFFSET << "\n"
		<< "Y Offset in Pixels (~= center of image): " << f.Y_OFFSET << "\n"

		<< "Theta Incrememt (in radians): " << f.THETA_INC << "\n"
		<< "Theta Incrememt (in degrees): " << f.THETA_INC_DEG << "\n"
		;

	// TODO print rest of members
}


#endif