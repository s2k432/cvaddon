#include "cvaddon_fast_sym_detect.h"
#include "cvaddon_edge.h"
#include "cvaddon_math.h"	// for sub-pixel localization
#include "cvaddon_draw.h"	// for suppression (drawing rectangles) during peak find - getResult()

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
, I2R_C( IMG_DIAG / ((float)R_DIVS - 1.0f) )	// -1 needed... ummm...
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
	// Includes padding for wrap around along theta
	H = cvCreateMat(THETA_DIVS + 2, R_DIVS, CV_32FC1);
	HBackUp = cvCreateMat(THETA_DIVS + 2, R_DIVS, CV_32FC1);
	HMask = cvCreateMat(THETA_DIVS + 2, R_DIVS, CV_8UC1);
	
	if(H == NULL || HBackUp == NULL || HMask == NULL)
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate Hough Accumulators");

	// Creating Hough Accumulator Mask (to exclude padding)
	cvSet(HMask, cvScalarAll(255));
	uchar *HRow = HMask->data.ptr;
	int MStep = HMask->step / sizeof(uchar);
	for(i = 0; i < MStep ; ++i) HRow[i] = 0;
	HRow += MStep * (THETA_DIVS+1);
	for(i = 0; i < MStep ; ++i) HRow[i] = 0;

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
	peakNBH = cvCreateMat(3, 3, CV_32FC1);

	__END__;
}

CvAddonFastSymDetector::~CvAddonFastSymDetector()
{	
	int i;
	cvReleaseMat(&H);
	cvReleaseMat(&HBackUp);
	cvReleaseMat(&HMask);

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
	float *HRow;	// A Row in H accumulator (theta)
	int HStep = H->step / sizeof(float);
	float *rRow, *rEnd;	// Rows of <rotatedEdges>
	float *rotEdgesPtr;
	int rotStep = rotatedEdges->step / sizeof(float);

	const float minDist = minPairDist * R2I_C_2;
	const float maxDist = maxPairDist * R2I_C_2;

	float *x0, *x1;

	int startThetaIdx = 0, endThetaIdx = THETA_DIVS-1;

	float thresh;


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
	HRow = H->data.fl+ (startThetaIdx+1) * HStep;


	for(thetaIdx = startThetaIdx; thetaIdx <= endThetaIdx; ++thetaIdx, HRow += HStep) 
	{

		// Rotating input edges and placing them into the 
		// <rotatedEdges> 2D array
		rotateStoredEdges(thetaIdx, numEdges);

		int rowMean = 0;
		int rowCount = 0;
		for(i = 0, rotEdgesPtr = rotatedEdges->data.fl; i < ROT_DIVS; ++i, rotEdgesPtr += rotStep)
		{
			int rowSize = reRowPtrs[i] - rotEdgesPtr;

			// Rejecting empty (no edge pair) rows
			if(rowSize <= 1) {// || rowSize >= params.rowCountThresh) {// Rejecting rows with too many edges
				reRowPtrs[i] = rotEdgesPtr;
			}
			else {
				rowMean += rowSize;
				++rowCount;
			}
		}
		if(rowCount > 0)
			rowMean /= rowCount;

		// Ratio-based straight line rejection
		thresh = 2.0f * rowMean;	
		for(i = 0, rotEdgesPtr = rotatedEdges->data.fl; i < ROT_DIVS; ++i, rotEdgesPtr += rotStep)
		{
			int rowSize = reRowPtrs[i] - rotEdgesPtr;			
			if(rowSize >= thresh) {
				reRowPtrs[i] = rotEdgesPtr;
			}
		}



		for(i = 0, rotEdgesPtr = rotatedEdges->data.fl; i < ROT_DIVS; ++i, rotEdgesPtr += rotStep)
		{
			rRow = rotEdgesPtr;
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
	}


	// Wrapping theta rows around into padding
	float *HRowPad;
	float *HRowSrc;

	// Copying last theta row (before padding) to row 0 (padding)
	HRowPad = H->data.fl + 0;
	HRowSrc = H->data.fl + HStep * (THETA_DIVS);
	for(i = 0; i < HStep; ++i) {
		HRowPad[i] = HRowSrc[i];
	}
	// Copying first theta row (after padding) to last row (padding)
	HRowPad = H->data.fl + HStep * (THETA_DIVS + 1);
	HRowSrc = H->data.fl + HStep * 1;
	for(i = 0; i < HStep; ++i) {
		HRowPad[i] = HRowSrc[i];
	}
}

void CvAddonFastSymDetector::getResult(const int &maxPeaksFound, CvAddonFastSymResults &dst
	, const float &rNBHDivs, const float &thetaNBHDivs
	, const bool &smoothBins
	, const bool &limitRange, const float &r0, const float &r1
	, const float &th0, const float &th1
	, const float &threshToZero, const float &threshRelMaxPeak
	)
{
	int i,j;
	int HStep = H->step / sizeof(float);

	int nPeaks = maxPeaksFound;
	if(nPeaks <= 0) nPeaks = 1;

	// Calculating suppression neighbourhood
	int rNBH, thetaNBH;
	if(rNBHDivs <= 2 || thetaNBHDivs <= 2) {
		rNBH = cvRound(R_DIVS / 20.0f);
		thetaNBH = cvRound(THETA_DIVS / 20.0f);
	}
	else {
		rNBH = cvRound((float)R_DIVS / rNBHDivs / 2.0f);
		thetaNBH = cvRound((float)THETA_DIVS / thetaNBHDivs / 2.0f);
	}

	cerr << rNBH << "," << thetaNBH << endl;

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
	if(smoothBins)
		cvSmooth(H, HBackUp);
	else
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
			cvMinMaxLoc(HBackUp, NULL, &maxVal, NULL, &maxLoc, HMask);

			int rIdx = maxLoc.x;
			int tIdx = maxLoc.y;

			// No more non-zero peaks left
			if(maxVal <= 0) break;
		
			// Chuck results (and hence skip sub-pixel localization)
			// if r values too large (most likely erroneous)
			// TODO increase ignored margins (too close to diagonal?)
			if(rIdx <= 0 || rIdx >= R_DIVS - 1) break;

			// A "WTF" moment, as theta index is in the padding...
			if(tIdx <= 0 || tIdx >= THETA_DIVS+1) {
				break;
			}

			// Copying raw results
			dst.symLines[p].numOfVotes		= maxVal;
			dst.symLines[p].rIndexRaw		= rIdx;
			dst.symLines[p].thetaIndexRaw	= tIdx;
			
			//  *** Sub-pixel localization of peak ***
			// Note: Padding rows already wrapped around theta during voting
			CvRect NBH = cvRect(rIdx-1, tIdx-1, 3, 3);
			cvGetSubRect(HBackUp, peakNBH, NBH);

			// Copying data to float array
			for(i = 0; i < peakNBH->height; ++i) 
			{
				for(j = 0; j < peakNBH->width; ++j)
				{
					peakNBVals[3*i + j] = CV_MAT_VAL(peakNBH, float, i, j);
				}
			}

			// Solving for peak
			peakShift = cvAddonFindExtrema3x3_2D(peakNBVals, diff, hessian, shift);

			// Ignore refinement shift if it is too large (probably a bad hessian fit)
			if(fabsf(peakShift[0]) >= 1.0f || fabsf(peakShift[1]) >= 1.0f) {
				dst.symLines[p].rIndex		= (float)rIdx;
				dst.symLines[p].thetaIndex	= (float)tIdx;
			}
			else {
				// Adjusting peak location
				dst.symLines[p].rIndex		= (float)rIdx + peakShift[0];
				dst.symLines[p].thetaIndex	= (float)tIdx + peakShift[1];
			}

			// Finding theta in radians, r in pixels
			dst.symLines[p].r			= getPixelFromIndex(dst.symLines[p].rIndex);
			dst.symLines[p].theta		= getRadiansFromIndex(dst.symLines[p].thetaIndex);

			
			// Suppressing Peak Neighbourhood by zeroing mask
			bool wrapMaskTheta = false;
			int r0 = (rIdx - rNBH) < 0 ? 0 : rIdx - rNBH;
			int r1 = (rIdx + rNBH) > R_DIVS-1 ? R_DIVS-1 : rIdx + rNBH;
			int t0 = tIdx - thetaNBH;
			int t1 = tIdx + thetaNBH;

			if(t0 <= 0 || t1 >= THETA_DIVS+1) wrapMaskTheta = true;

			if(wrapMaskTheta) {
//				// DEBUG
//				cerr << "WRAP" << endl;

				CvRect NBHSub0, NBHSub1;

				// Wrapping from 0 --> THETA_DIVS
				// Note that wrapping theta also requires negating r (as r changes signs when theta -90 <--> +90)
				if(t0 <= 0) {
					int NBHWidth = r1-r0;
					NBHSub0 = cvRect(r0, 0, NBHWidth, t1);

					// When r0 ~= R_DIVS_2 - 0.5f (center), the suppression region is pretty much 
					// being flipped across theta only. So, not worrying about -0.5f
					r1 = R_DIVS - r1;

					NBHSub1 = cvRect(r1, t0 + (THETA_DIVS - 1), NBHWidth, THETA_DIVS + 1);
				}

				if(t1 >= THETA_DIVS+1) {
					NBHSub0 = cvRect(r0, t0, r1-r0, THETA_DIVS + 1);
					NBHSub1 = cvRect(r0, 0, r1-r0, t1 - (THETA_DIVS - 1));
				}

				cvAddonDrawRectangle(HMask, NBHSub0, CV_RGB(0,0,0), CV_FILLED);
				cvAddonDrawRectangle(HMask, NBHSub1, CV_RGB(0,0,0), CV_FILLED);
			}
			else {
				CvRect NBHSub = cvRect(r0, t0, r1-r0, t1-t0);
				cvAddonDrawRectangle(HMask, NBHSub, CV_RGB(0,0,0), CV_FILLED);
			}
		}
		dst.numSym = p;
	}

}