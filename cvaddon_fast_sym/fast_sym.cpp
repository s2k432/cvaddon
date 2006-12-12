#include "fast_sym.h"
//#include "windows_fast_timer.h"

#include "cvaddon_math.h"


void calculateFastSymConstants(const FastSymInitParams &p, FastSymConstants &c);

FastSymDetector::FastSymDetector(const FastSymInitParams &params)
	: H(NULL), houghBackUp(NULL), RMatrices(NULL)
	, diff(NULL), hessian(NULL), shift(NULL)
{
	int i;
	CvMat *rotMat;	// 2x3 rotation matrix
	double rotInc;

	CV_FUNCNAME("FastSymDetector::FastSymDetector");

	__BEGIN__;

	if(params.imgHeight == 0 || params.imgWidth == 0)
		CV_ERROR(CV_StsBadArg, "Bad Image Size");

	// There is  no point having 1 hough bin -_-
	if(params.rDivs < 1 || params.thetaDivs < 1)
		CV_ERROR(CV_StsBadArg, "Bad Hough Divisions");

	// Calculating constants such as theta increments and
	// image diagonal length
	initParams = params;
	calculateFastSymConstants(initParams,  C);

	// **** Allocating Memmory ****
	// Hough Accumulators
	allocate2DArrayBlock(H, initParams.thetaDivs, initParams.rDivs);
	if(H == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <H> matrix");

	allocate2DArrayBlock(houghBackUp, initParams.thetaDivs, initParams.rDivs);	
	if(houghBackUp == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <houghBackUp> matrix");

	HRow = new AccumType[initParams.rDivs];
	if(HRow == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <HRow> vector");


	allocate2DArray(RMatrices, initParams.thetaDivs, 4);
	if(RMatrices == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <RMatrices> matrix");

	allocate2DArray(rotatedEdges, C.ROTATED_EDGES_DIVS, C.IMG_DIAGONAL);
	if(rotatedEdges == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <rotatedEdges> matrix");
	
	reRowPtrs = new RotatedDataType*[C.ROTATED_EDGES_DIVS];
	if(reRowPtrs == NULL) 
		CV_ERROR(CV_StsNoMem, "Out of Memory. Couldn't allocate <reRowPtrs> vector");

	// *** Calculate Rotation Matrices
	rotMat = cvCreateMat(2, 3, CV_32FC1);
	rotInc = C.THETA_INC_DEG;
	for(i = 0; i < initParams.thetaDivs; ++i)
	{		
		double angle = rotInc * ((double)i-(double)initParams.thetaDivs / 2.0);
		cv2DRotationMatrix(cvPoint2D32f(0.0, 0.0), angle, 1, rotMat);

		RMatrices[i][0] = (rotMat->data.fl)[0] * C.R2I_C_2 ;
		RMatrices[i][1] = (rotMat->data.fl)[1] * C.R2I_C_2 ;
		RMatrices[i][2] = (rotMat->data.fl)[3] / initParams.pixelResolution;
		RMatrices[i][3] = (rotMat->data.fl)[4] / initParams.pixelResolution;
	}
	cvReleaseMat(&rotMat);
	
	// Matrices for sub-index localization of peaks in hough space
	diff = cvCreateMat(2, 1, CV_32FC1);
	hessian = cvCreateMat(2, 2, CV_32FC1);
	shift = cvCreateMat(2, 1, CV_32FC1);

	__END__;

	// TOOD clean up 2D arrays?
}

FastSymDetector::~FastSymDetector()
{
	delete2DArrayBlock(H);
	delete2DArrayBlock(houghBackUp);

	delete2DArray(RMatrices, initParams.thetaDivs);
	delete2DArray(rotatedEdges, C.ROTATED_EDGES_DIVS);
	
	if(reRowPtrs != NULL) delete [] reRowPtrs;	
	if(HRow != NULL) delete [] HRow;

	cvReleaseMat(&diff);
	cvReleaseMat(&hessian);
	cvReleaseMat(&shift);
}

void FastSymDetector::writeParamsToFile(const char *comment, const char *filename) const
{
	std::ofstream file;

	CV_FUNCNAME("FastSymDetector::writeParamsToFile");

	__BEGIN__;

	file.open(filename, std::ios::app);

	if(!file.is_open())
		CV_ERROR(CV_StsBadArg, "Could not open output file" );

	if(comment != NULL && strlen(comment) > 0) 
		file << comment << endl;

	file << "Initialization Parameters" << endl;
	file << initParams << endl;
	file << "Calculation Constants" << endl;
	file << C << endl;
	file << "Hough Accumulation Parameters" << endl;
	file << accumParams << endl;
	file << "Peak Finding Parameters" << endl;
	file << peakFindParams << endl;

	__END__;

	if(file.is_open()) file.close();
}

void calculateFastSymConstants(const FastSymInitParams &p, FastSymConstants &c)
{
	c.IMG_DIAGONAL			= ceilf( sqrt(p.imgHeight*p.imgHeight + p.imgWidth*p.imgWidth) );
	c.ROTATED_EDGES_DIVS	= (float)(c.IMG_DIAGONAL) / p.pixelResolution;
	c.ROTATED_EDGES_DIVS_2	= (float)(c.IMG_DIAGONAL) / p.pixelResolution / 2.0f;

	c.X_OFFSET =		( (float)p.imgWidth - 1.0f) / 2.0f;
	c.Y_OFFSET =		( (float)p.imgHeight - 1.0f) / 2.0f;

	c.THETA_INC =		(float)(CV_PI / ((float)(p.thetaDivs )) );
	c.THETA_INC_DEG =	(float)(180.0f / ((float)(p.thetaDivs )) );

	c.R2I_C =			(float)p.rDivs / (float)c.IMG_DIAGONAL;
	c.R2I_C_2 =			(float)c.R2I_C / 2.0f;
	
	c.I2R_C =			(float)c.IMG_DIAGONAL / (float)p.rDivs;
//	c.I2R_C_2 =			(float)c.IMG_DIAGONAL / (float)p.rDivs / 2.0f;
	
	c.R_DIVS_2 =		(float)(p.rDivs-1) / 2.0f;
	c.R_DIVS_4 =		(float)(p.rDivs-1) / 4.0f;

	c.TH_DIVS_2 =		(float)(p.thetaDivs-1) / 2.0f;
	c.I2TH_C =			(float)(CV_PI) / (float)p.thetaDivs;
//	c.I2TH_C_2 =		(float)(CV_PI) / (float)p.thetaDivs / 2.0f;
}

inline void FastSymDetector::rotateStoredEdges(const unsigned int& thetaIdx)
{
	int i, j;
	float *R;
	float xx, yy;

	const RotatedDataType C_R_DIVS_4 = C.R_DIVS_4;
	const int C_ROTATED_EDGES_DIVS_2 = C.ROTATED_EDGES_DIVS_2;
	const int C_ROTATED_EDGES_DIVS = C.ROTATED_EDGES_DIVS;
	CvPoint2D32f *e;
	float *ee;

	CV_FUNCNAME("FastSymDetector::rotateAndStoreEdges");

	__BEGIN__;
	
	if(thetaIdx > initParams.thetaDivs) {
		CV_ERROR(CV_StsBadArg, "Bad theta index" );		
	}

	R = RMatrices[thetaIdx];

	for(i = 0; i < C_ROTATED_EDGES_DIVS; ++i)
	{
		reRowPtrs[i] = rotatedEdges[i];
	}
	
	for(e = edges; e != edges + numEdges; ++e)
	{
		xx = e->x;
		yy = e->y;

		int y = cvRound( R[2] * xx + R[3] * yy + C_ROTATED_EDGES_DIVS_2 );
		*(reRowPtrs[ y ]++) = R[0] * xx + R[1] * yy + C_R_DIVS_4;
	}
	__END__;
}

// This functions works for coloured images as well, 
// and is meant for visualization.
inline void FastSymDetector::drawRotatedEdges(IplImage *img, const CvScalar &color)
{
	int i;	
	RotatedDataType *jj;
	for(i = 0; i < C.ROTATED_EDGES_DIVS; ++i) 
	{
		for(jj = rotatedEdges[i]; jj != reRowPtrs[i]; ++jj)
		{
			CvPoint edgeStart = cvPoint( 
				cvRound( (*jj - C.R_DIVS_4) / C.R2I_C_2 + C.X_OFFSET )
				, cvRound( initParams.pixelResolution * ((float)i - C.ROTATED_EDGES_DIVS/2.0f)) + C.Y_OFFSET);


			CvPoint edgeEnd = cvPoint(edgeStart.x, edgeStart.y);

			cvLine(img, edgeStart, edgeEnd, color);
		}
	}
}

// Gaussian Kernel
static const float G[] = {
	1.0f, 2.0f, 1.0f, 
	2.0f, 4.0f, 2.0f, 
	1.0f, 2.0f, 1.0f
};

AccumType **FastSymDetector::accum(const IplImage *edgeImg
	, const FastSymAccumParams &params)
{
	
	const int C_ROTATED_EDGES_DIVS = C.ROTATED_EDGES_DIVS;
	const RotatedDataType minDist = params.minPairDist * C.R2I_C_2;
	const RotatedDataType maxDist = params.maxPairDist * C.R2I_C_2;
	RotatedDataType *x0, *x1;
	
	// For voting
	AccumType *HRowPtrs[3];

	int i, j, thetaIdx;

	int rowSize;
	float rowMean;
	int rowCount;
	float thresh;
	RotatedDataType *rRow, *rEnd;	// Rows of <rotatedEdges>

	CV_FUNCNAME("FastSymDetector::accum");

	__BEGIN__;

	// TODO ***ERROR CHECKS HERE!!

	accumParams = params;

	// Clearing Hough Accumulator
	for(i = 0; i < initParams.thetaDivs; ++i)
	{
		memset(H[i], 0, sizeof(AccumType)*initParams.rDivs);
	}

	// Extracting locations of edge pixels from edge image
	numEdges = cvAddonFindNonZeroPixels<uchar>(edgeImg, edges, MAX_EDGE_PIXELS);

	// TOO MANY EDGES
	if(numEdges == -1) return NULL;

	// TESTING - shifting edges to be relative to center
	for(i = 0; i < numEdges; ++i)
	{
		edges[i].x -= C.X_OFFSET;
		edges[i].y -= C.Y_OFFSET;
	}	

	// Processing image at each angle (Hough Indices)
	for(thetaIdx = params.startThetaIdx; thetaIdx <= params.endThetaIdx; ++thetaIdx) 
	{
		// Rotating input edges and placing them into the 
		// <rotatedEdges> 2D array
		rotateStoredEdges(thetaIdx);

		rowMean = 0;
		rowCount = 0;
		for(i = 0; i < C_ROTATED_EDGES_DIVS; ++i)
		{
			rowSize = reRowPtrs[i] - rotatedEdges[i];

			// Rejecting empty (no edge pair) rows
			// Rejecting rows with too many edges
			if(rowSize <= 1 || rowSize >= params.rowCountThresh) {
				reRowPtrs[i] = rotatedEdges[i];
			}
			else {
				rowMean += rowSize;
				++rowCount;
			}
		}
		rowMean /= (float)rowCount;

		// Ratio-based straight line rejection
		thresh = params.rowCountThreshRatio * rowMean;	
		for(i = 0; i < C_ROTATED_EDGES_DIVS; ++i)
		{
			rowSize = reRowPtrs[i] - rotatedEdges[i];
			
			if(rowSize >= thresh) {
				reRowPtrs[i] = rotatedEdges[i];
			}
		}

		memset(HRow, 0, sizeof(AccumType)*initParams.rDivs);
		for(i = 0; i < C_ROTATED_EDGES_DIVS; ++i)
		{
			rRow = rotatedEdges[i];
			rEnd = reRowPtrs[i];

			if(rEnd != rRow)
			{
				sort(rRow, rEnd);

				for(x0 = rRow; x0 != rEnd-1; ++x0)
				{
					for(x1 = x0+1; x1 != rEnd; ++x1)
					{
						RotatedDataType dist = *x1 - *x0;

						if(dist >= maxDist) {
							break;
						}
						else if( dist > minDist) {		
#ifdef _INTERPOLATE_VOTE
							float tmp = *x0 + *x1;
							float high = ceilf(tmp);
							float low = floorf(tmp);
							HRow[ (int)high ] += 1.0f - (high - tmp);
							HRow[ (int)low ] += 1.0f - (tmp - low);
#else
							++(HRow[ (int)(*x0 + *x1) ] );
#endif
						}
					}
				}
			}
		}

/*
// Special voting scheme
//		AccumType *HRowPtr = H[thetaIdx];
//		memset(HRowPtr, 0, sizeof(AccumType)*initParams.rDivs);

		memset(HRow, 0, sizeof(AccumType)*initParams.rDivs);

		// Processing Each Row
		for(i = 0; i < C_ROTATED_EDGES_DIVS; ++i)
		{	
// Special voting scheme
//			memset(HRow, 0, sizeof(AccumType)*initParams.rDivs);
			
			rRow = rotatedEdges[i];
			rEnd = reRowPtrs[i];

			if(rEnd != rRow)
			{
				sort(rRow, rEnd);

				for(x0 = rRow; x0 < rEnd-1; ++x0)
				{
					for(x1 = x0+1; x1 != rEnd; ++x1)
					{
						RotatedDataType dist = *x1 - *x0;

						if( dist > minDist && dist < maxDist ) {							
//							++(HRow[ cvRound(*x0 + *x1) ] );
							++(HRow[ (int)(*x0 + *x1) ] );

// Special voting scheme
//							(HRow[ cvRound(*x0 + *x1) ] ) = 1;
						}
					}
				}
			}

// Special voting scheme
//			for(int h = 0; h < initParams.rDivs; ++h)
//			{
//				if(HRow[h]) ++HRowPtr[h];
//			}
		}

		// Copying results into Hough Accumulator
//		memcpy(H[thetaIdx], HRow, sizeof(AccumType)*initParams.rDivs);
*/

//static FastTimer timer;
//timer.getLoopTime();
#ifdef _SPREAD_VOTES
		if(thetaIdx != params.startThetaIdx
			&& params.endThetaIdx != thetaIdx){

			for(int r = 0; r < 3; ++r) {
				HRowPtrs[r] = H[thetaIdx + r - 1];
			}
			for(int h = 1; h < initParams.rDivs-1; ++h)
			{
				if(HRow[h] > 0) {
					for(int r = 0; r < 3; ++r) {
						HRowPtrs[r][h-1] += G[3*r + 0] * HRow[h]; //0.5f * HRow[h];
						HRowPtrs[r][h] += G[3*r + 1] * HRow[h]; //HRow[h];
						HRowPtrs[r][h+1] += G[3*r + 2] * HRow[h]; //0.5f * HRow[h];
					}
				}
			}
		}
#else
	memcpy(H[thetaIdx], HRow, sizeof(AccumType)*initParams.rDivs);
#endif


//static double time = timer.getLoopTime();
//cerr << "**T** Final Voting took: " << time << endl;


//#ifdef _DEBUG
//		cerr << "THETA IDX: " << thetaIdx << endl;
//#endif
	}

#ifdef _DEBUG
/*
	IplImage *tmp = cvCreateImage(cvSize(initParams.rDivs, initParams.thetaDivs)
		, IPL_DEPTH_32F, 1);
	cvZero(tmp);
	
	int start = params.startThetaIdx;
	int end = params.endThetaIdx;

	for(int t = start; t <= end; ++t)
	{
		for(int r = 0; r < initParams.rDivs; ++r)
		{
//			if(H[t][r] > 0) cerr << t << ": " << H[t][r] << endl;
			cvLine(tmp, cvPoint(r, t), cvPoint(r, t), cvScalarAll( H[t][r] ) );
		}			
	}

	cvAddonShowImageOnce(tmp);
	cvReleaseImage(&tmp);
*/

//	IplImage *foo = cvCreateImage(cvSize(255, 255), IPL_DEPTH_8U, 1);
//	cvZero(foo);
//
//	rotateStoredEdges(179);
//	drawRotatedEdges(foo);
//	cvAddonShowImageOnce(foo);
//
//	cvReleaseImage(&foo);

#endif

// DEBUG
	/*
	cvNamedWindow("ee", 1);
	cvShowImage("ee", edgeImg);
	cvWaitKey(0);

	IplImage *resImg = cvCloneImage(edgeImg);
	cvZero(resImg);

	IplImage *diffImg = cvCloneImage(edgeImg);
	cvZero(diffImg);

	rotateStoredEdges(135);
	drawRotatedEdges(resImg);

	cvShowImage("ee", resImg);
	cvWaitKey(0);

//	cvSaveImage("e_problem.png", resImg);

	cvAbsDiff(edgeImg, resImg, diffImg);

	cvShowImage("ee", diffImg);
	cvWaitKey(0);
*/

	__END__;

	return H;
}


//struct FastSymPeakFindParams
//{
//	int maxPeaksFound;		// Max number of peaks returned
//	float thresholdRatio;			// Ratio of peak used to threshold accumulator
//	int thetaNBH, rNBH;	// Width (from centre point to edge) of suppression neighbourhoods
//	int startThetaIdx, endThetaIdx;	// Angle Limit Indices (can wrap around)
//	int startRIdx, endRIdx;			// Radius Limit Indices (can wrap around)
//};
//struct FastSymPeakFindResults
//{
//	CvPoint2D32f *peaks;
//	float *peakVals;
//	unsigned int numPeaks;
//};

void FastSymDetector::findPeak(const FastSymPeakFindParams &params
	, FastSymPeakFindResults &results, bool destroyHoughAccum)
{
	int p, i, j, m, n;
	int r, th;
	int w, h;
	AccumType *HPtr;
	AccumType thresh = 0.0f;

	int accumSize = initParams.rDivs*initParams.thetaDivs;
	
	// Peak Sub-Index Localization
	float *peakShift;
	AccumType peakNBVals[9];

	CV_FUNCNAME("FastSymDetector::findPeak");

	__BEGIN__;

	if(params.maxPeaksFound <= 0)
		CV_ERROR(CV_StsBadArg, "<maxPeakFound> <= 0");

	// Storing input parameters into class state
	peakFindParams = params;
	w = initParams.rDivs;
	h = initParams.thetaDivs;


	// Backing up Hough Accumulator
	if(!destroyHoughAccum)
		memcpy(*houghBackUp, *H, sizeof(AccumType)*accumSize);

	
	results.numPeaks = 0;
	for(p = 0; p < params.maxPeaksFound; ++p)
	{
		HPtr = *H;
		AccumType max = -1;
		int idx = -1;	
		
		// Finding Maximum
		for(i = 0; i < accumSize; ++i)
		{
			if( HPtr[i] > max)
			{
				max = HPtr[i];
				idx = i;
			}
		}
		
		// Empty accumlator (or just a "WTF" moment...)
		if(max <= 0) return;

		// First peak used to set threshold
		if(results.numPeaks == 0)
			thresh = params.thresholdRatio * max;
		else if(max < thresh) return;
		
		// R, Theta of Hough Peaks
		r = idx % w;
		th = idx / w;

		// Copying results
		results.peaks[results.numPeaks].x = r;
		results.peaks[results.numPeaks].y = th;
		results.rawPeaks[results.numPeaks].x = r;
		results.rawPeaks[results.numPeaks].y = th;
		results.peakVals[results.numPeaks] = max;
		

		// *** Localizing Peak ***
		// Don't localize if r index is near the border
		// There is probably something wrong if this happened!!!
		if(r == params.startRIdx || r == params.endRIdx) {
			continue;
		}

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
//			cerr << "STANDARD NBH LOC" << endl;

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

	__END__;
}
