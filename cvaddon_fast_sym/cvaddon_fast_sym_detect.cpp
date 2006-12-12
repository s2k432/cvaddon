#include "cvaddon_fast_sym_detect.h"
#include "cvaddon_edge.h"

#include <algorithm>
using std::sort;

#ifdef _DEBUG
	#include <iostream>
	using std::endl;
	using std::cerr;
#endif

CvAddonFastSymDetector::CvAddonFastSymDetector(const CvSize &imgSize
	, const CvSize &houghSize, const int &rotEdgeRowHeight)
: IMG_W(imgSize.width), IMG_H(imgSize.height)
, IMG_CEN_X( ((float)IMG_W - 1.0f) / 2.0f)
, IMG_CEN_Y( ((float)IMG_H - 1.0f) / 2.0f)
, IMG_DIAG( sqrtf(IMG_H*IMG_H + IMG_W*IMG_W) ), IMG_DIAG_2(IMG_DIAG/2.0f)

, ROT_RES( rotEdgeRowHeight )
, ROT_DIVS( cvRound(IMG_DIAG / ROT_RES) ), ROT_DIVS_2( (float)ROT_DIVS / 2.0f)

// "Pushed forward" constants
, R2I_C( (float)R_DIVS / IMG_DIAG )
, R2I_C_2( R2I_C / 2.0f)
, I2R_C( IMG_DIAG / (float)R_DIVS )
, I2TH_C( CV_PI / (float)THETA_DIVS )

, R_DIVS(houghSize.width), THETA_DIVS(houghSize.height)
, R_DIVS_2( (float)R_DIVS / 2.0f ), R_DIVS_4( (float)R_DIVS / 4.0f )
, THETA_DIVS_2( (float)THETA_DIVS / 2.0f )
, THETA_INC( CV_PI / (float)THETA_DIVS ), THETA_INC_DEG(THETA_INC / CV_PI * 180.0f)
{
	int i;
	CvMat *rotMat;	// 2x3 rotation matrix
	double rotInc;

	CV_FUNCNAME("CvAddonFastSymDetector Constructor");

	__BEGIN__;

	if(IMG_W <= 0 || IMG_H <= 0)
		CV_ERROR(CV_StsBadArg, "Bad Image Size");

	if(R_DIVS <= 0 || THETA_DIVS <= 0)
		CV_ERROR(CV_StsBadArg, "Bad Hough Divisions");

	// **** Allocating Memmory ****
	// Hough Accumulators - theta = rows
	H = cvCreateMat(THETA_DIVS, R_DIVS, CV_32FC1);
	HBackUp = cvCreateMat(THETA_DIVS, R_DIVS, CV_32FC1);
	
	if(H == NULL || HBackUp == NULL)
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate Hough Accumulators");

	// TODO Error Check?
	rotMats = new CvMat* [THETA_DIVS];
	for(i = 0; i < THETA_DIVS; ++i)
	{
		rotMats[i] = cvCreateMat(2, 2, CV_32FC1);
	}

	// *** Calculate Rotation Matrices
	rotMat = cvCreateMat(2, 3, CV_32FC1);
	rotInc = THETA_INC_DEG;
	for(i = 0; i < THETA_DIVS; ++i)
	{		
		double angle = rotInc * ((double)i - (double)THETA_DIVS / 2.0);
		cv2DRotationMatrix(cvPoint2D32f(0.0, 0.0), angle, 1, rotMat);

		(rotMats[i]->data.fl)[0] = (rotMat->data.fl)[0] * R2I_C_2;
		(rotMats[i]->data.fl)[1] = (rotMat->data.fl)[1] * R2I_C_2;
		(rotMats[i]->data.fl)[2] = (rotMat->data.fl)[3] / ROT_RES;
		(rotMats[i]->data.fl)[3] = (rotMat->data.fl)[4] / ROT_RES;
	}
	cvReleaseMat(&rotMat);	

	reRowPtrs = new float*[ROT_DIVS];
	rotatedEdges = cvCreateMat(ROT_DIVS, IMG_DIAG, CV_32FC1);
	if(rotatedEdges == NULL)
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate rotatedEdges (Rot)");

	// Matrices for sub-index localization of peaks in hough space
	diff = cvCreateMat(2, 1, CV_32FC1);
	hessian = cvCreateMat(2, 2, CV_32FC1);
	shift = cvCreateMat(2, 1, CV_32FC1);

	__END__;
}

CvAddonFastSymDetector::~CvAddonFastSymDetector()
{	
	int i;
	cvReleaseMat(&H);
	cvReleaseMat(&HBackUp);

	cvReleaseMat(&rotatedEdges);
	delete [] reRowPtrs;
	for(i = 0; i < THETA_DIVS; ++i)
	{
		cvReleaseMat(&rotMats[i]);
	}
	delete [] rotMats;

	cvReleaseMat(&diff);
	cvReleaseMat(&hessian);
	cvReleaseMat(&shift);
}


void CvAddonFastSymDetector::rotateStoredEdges(const unsigned int& thetaIdx, const unsigned int& numEdges)
{
	int i;

	float* ptr = rotatedEdges->data.fl;
	int step = rotatedEdges->step / sizeof(float);
	const int _LOCAL_ROT_DIVS = ROT_DIVS;
	const int _LOCAL_NUM_EDGES = numEdges;

	float *R = (rotMats[thetaIdx])->data.fl;
	const float R0 = R[0];
	const float R1 = R[1];
	const float R2 = R[2];
	const float R3 = R[3];

	for(i = 0; i < _LOCAL_ROT_DIVS; ++i)
	{
		reRowPtrs[i] = ptr + i*step;
	}
	
	for(i = 0; i <  _LOCAL_NUM_EDGES; ++i)
	{
//		int y = cvRound( R2 * edges[i].x + R3 * edges[i].y + ROT_DIVS_2 );
		int y = (int)( R2 * edges[i].x + R3 * edges[i].y + ROT_DIVS_2 );
		*(reRowPtrs[ y ]++) = R0 * edges[i].x + R1 * edges[i].y + R_DIVS_4;
	}
}


// Voting (Hough Accumulation) function
// TODO angle limits, sample rows and hard threshold
void CvAddonFastSymDetector::vote( const IplImage *src
	, const int &minPairDist, const int &maxPairDist
	, const bool &angleLimits, const float &minThetaDeg, const float &maxThetaDeg
	, const float &sampleRotEdgeRatio
	, const int &maxRotEdgeRowSize
	, const float &maxRowEdgeRowRatio
	)
{
	int i, thetaIdx;
	float thresh;
	float *rRow, *rEnd;	// Rows of <rotatedEdges>

	const float minDist = minPairDist * R2I_C_2;
	const float maxDist = maxPairDist * R2I_C_2;

	float *x0, *x1;

	int startThetaIdx = 0, endThetaIdx = THETA_DIVS-1;

	// Clearing Hough Accumulator
	cvZero(H);

	// Extracting locations of edge pixels from edge image
	int numEdges = cvAddonFindNonZeroPixels<uchar>(src, edges, MAX_EDGE_PIXELS);

	// TODO better way of handling edge overflow, sampling?
	if(numEdges == -1) numEdges = MAX_EDGE_PIXELS;

	for(i = 0; i < numEdges; ++i)
	{
		edges[i].x -= IMG_CEN_X;
		edges[i].y -= IMG_CEN_Y;
	}	

	// Processing image at each angle (Hough Indices)
	for(thetaIdx = startThetaIdx; thetaIdx <= endThetaIdx; ++thetaIdx) 
	{
		// Rotating input edges and placing them into the 
		// <rotatedEdges> 2D array
		rotateStoredEdges(thetaIdx, numEdges);

		int rowMean = 0;
		int rowCount = 0;
		for(i = 0; i < ROT_DIVS; ++i)
		{
			int rowSize = reRowPtrs[i] - (float*)(rotatedEdges->data.ptr + i*rotatedEdges->step);

			// Rejecting empty (no edge pair) rows
			
			if(rowSize <= 1) {// || rowSize >= params.rowCountThresh) {// Rejecting rows with too many edges
				reRowPtrs[i] = (float*)(rotatedEdges->data.ptr + i*rotatedEdges->step);
			}
			else {
				rowMean += rowSize;
				++rowCount;
			}
		}
		rowMean /= rowCount;

		// Ratio-based straight line rejection
		thresh = 2.0f * rowMean;	
		for(i = 0; i < ROT_DIVS; ++i)
		{
			int rowSize = reRowPtrs[i] - (float*)(rotatedEdges->data.ptr + i*rotatedEdges->step);			
			if(rowSize >= thresh) {
				reRowPtrs[i] = (float*)(rotatedEdges->data.ptr + i*rotatedEdges->step);
			}
		}

		float *HRow = (float*)(H->data.ptr + thetaIdx * H->step);
		for(i = 0; i < ROT_DIVS; ++i)
		{
			rRow = (float*)(rotatedEdges->data.ptr + i*rotatedEdges->step);
			rEnd = reRowPtrs[i];

			if(rEnd != rRow)
			{
				for(x0 = rRow; x0 != rEnd-1; ++x0)
				{
					for(x1 = x0+1; x1 != rEnd; ++x1)
					{
						float dist = fabsf(*x1 - *x0);
						if(dist > maxDist || dist < minDist) break;
						else {
							++HRow[ (int)(*x0 + *x1) ];		// Truncates in R only, not theta
						}
					}
				}
			}
		
		}

		// Interpolating along r is *probably* not worthwhile (see version 2 code with ACRA stereo paper)
		// as theta accuracy is generally the dominant error source (+- 1 degree is much worst than 1 pixel error in R)
		//
		// Also tried using a sort(rRow, rEnd) so that the fabsf() call in the inner loop is not needed. However, 
		// it seems to only slow things down on my test images. Hard to say which one is faster in practice, as
		// the sort() speed will depend on memory access and the library implementation. fabsf() also depends on 
		// implementation, but I feel requires fewer assumptions and has a more consistent speed. 

//#ifdef _SPREAD_VOTES
//		if(thetaIdx != params.startThetaIdx
//			&& params.endThetaIdx != thetaIdx){
//
//			for(int r = 0; r < 3; ++r) {
//				HRowPtrs[r] = H[thetaIdx + r - 1];
//			}
//			for(int h = 1; h < initParams.rDivs-1; ++h)
//			{
//				if(HRow[h] > 0) {
//					for(int r = 0; r < 3; ++r) {
//						HRowPtrs[r][h-1] += G[3*r + 0] * HRow[h]; //0.5f * HRow[h];
//						HRowPtrs[r][h] += G[3*r + 1] * HRow[h]; //HRow[h];
//						HRowPtrs[r][h+1] += G[3*r + 2] * HRow[h]; //0.5f * HRow[h];
//					}
//				}
//			}
//		}
//#else
//	memcpy(H[thetaIdx], HRow, sizeof(AccumType)*initParams.rDivs);
//#endif

	}
}

void CvAddonFastSymDetector::getResult(const int &maxPeaksFound, CvAddonFastSymResults &dst
	, const bool &limitRange, const float &r0, const float &r1
	, const float &th0, const float &th1
	, const float &threshToZero, const float &threshRelMaxPeak
	)
{
	int nPeaks = maxPeaksFound;
	if(nPeaks <= 0) nPeaks = 1;

	// Not enough room in results structure
	if(dst.maxSym < nPeaks) {
		// Deleting already allocated data
		if(dst.maxSym > 0) {
			delete [] dst.symLines;
		}
		dst.symLines = new CvAddonFastSymLine[nPeaks];
		dst.maxSym = nPeaks;
		dst.numSym = 0;
	}

	// Making copy of Hough Accumulator 
	// (as peak find process destroys the bin values)
	cvCopy(H, HBackUp);

	int p;

	// Peak Sub-Index Localization
	float *peakShift;
	float peakNBVals[9];

	// Simple global peak search
	if(!limitRange) {
		for(p = 0; p < nPeaks; ++p)
		{	
			double maxVal;
			CvPoint maxLoc;
			cvMinMaxLoc(HBackUp, NULL, &maxVal, NULL, &maxLoc);

			// No more non-zero peaks left
			if(maxVal <= 0) break;
				
			// Copying raw results
			dst.symLines[p].numOfVotes		= maxVal;
			dst.symLines[p].rIndexRaw		= maxLoc.x;
			dst.symLines[p].thetaIndexRaw	= maxLoc.y;
			
			// Sub-pixel localization of peak
			// TODO
		}
		dst.numSym = p;
	}

/*
		// Also handling boundary Theta values by
		// Duplicating in R direction
		if(th == params.startThetaIdx) {
			// Using middle row as first row
			for(n = 0; n <= 2; ++n)
				peakNBVals[0 + n] = H[th][r + n -1];

			for(m = 1; m <= 2; ++m)
			{
				for(n = 0; n <= 2; ++n)
					peakNBVals[3*m + n] = H[(th + m - 1)][r + n -1];
			}			
		}
		else if( th == params.endThetaIdx ) {
			for(m = 0; m <= 1; ++m)
			{
				for(n = 0; n <= 2; ++n)
					peakNBVals[3*m + n] = H[(th + m - 1)][r + n -1];
			}

			// Using middle row as last row
			for(n = 0; n <= 2; ++n)
				peakNBVals[6 + n] = H[th][r + n -1];
		}
		else {
			for(m = 0; m <= 2; ++m)
			{
				for(n = 0; n <= 2; ++n)
					peakNBVals[3*m + n] = H[(th + m - 1)][r + n -1];
			}
		}

		// Solving for peak
		peakShift = cvAddonFindExtrema3x3_2D(peakNBVals, diff, hessian, shift);

		// Adjusting peak location
		results.peaks[results.numPeaks].x += peakShift[0];

		if(th != params.startThetaIdx  && th != params.endThetaIdx)
			results.peaks[results.numPeaks].y += peakShift[1];

		// *** Suppressing Neigbourhood ***
		// Finding boundaries of suppression area. We are NOT WRAPPING around
		// TODO should we wrap along theta?
		int thStartIdx = ( th - params.thetaNBH >= 0 ? th - params.thetaNBH : 0);
		int thEndIdx = ( th + params.thetaNBH < initParams.thetaDivs ? th + params.thetaNBH : initParams.thetaDivs - 1);

		int rStartIdx = ( r - params.rNBH >= 0 ? r - params.rNBH : 0);
		int rEndIdx = ( r + params.rNBH < initParams.rDivs ? r + params.rNBH : initParams.rDivs - 1);

		for(i = thStartIdx; i <= thEndIdx; ++i)
		{
			HPtr = H[i];
			for(j = rStartIdx; j <= rEndIdx; j++)
			{
				HPtr[j] = 0;
			}
		}

		++results.numPeaks;
	}

	// Restoring Hough Accumulator
	if(!destroyHoughAccum)
		memcpy(*houghBackUp, *H, sizeof(AccumType)*accumSize);
*/

}