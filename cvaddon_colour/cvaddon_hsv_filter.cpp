// TODO
// ERROR CHECKS

#include "cvaddon_fast_bgr2hsv.h"	// LUT-based fast HSV conversion
#include "cvaddon_math.h"
#include "cvaddon_hsv_filter.h"

CvAddonHSVFilter::CvAddonHSVFilter(const int& hBins, const int& sBins)
: H_BINS(hBins), S_BINS(sBins)
{
	sizes[0] = hBins;
	sizes[1] = sBins;

	hist = cvCreateHist(2, sizes, CV_HIST_ARRAY);
	oldHist = cvCreateHist(2, sizes, CV_HIST_ARRAY);

	cvClearHist(hist);
	cvClearHist(oldHist);

	// Init Look up table for BGR ==> HSV
	cvAddonInitHsvLut();
}

CvAddonHSVFilter::~CvAddonHSVFilter()
{
	cvReleaseHist(&hist);
	cvReleaseHist(&oldHist);
}

void CvAddonHSVFilter::buildHist(const IplImage *src, IplImage *H, IplImage *S, IplImage *V
		, const CvScalar &thresh0, const CvScalar &thresh1, const IplImage *mask)
{
	CvScalar lower, upper;
	// DO ERROR CHECK
	
	cvAddonBGR2HSV_LUT(src, H, S, V);

	lowerUpper(thresh0, thresh1, lower, upper);

	cvInRangeS(V, cvScalarAll(lower.val[2]), cvScalarAll(upper.val[2]), V);

	// Combining mask and V threshold
	if(mask != NULL)
		cvOr(V, mask, V);

	planes[0] = H; 
	planes[1] = S;
	cvCalcHist(planes, hist, 0, V);

	cvNormalizeHist(hist, 255.0);
}

void CvAddonHSVFilter::blendHist(const IplImage *src, IplImage *H, IplImage *S, IplImage *V
		, const CvScalar &thresh0, const CvScalar &thresh1
		, const IplImage *mask, const double& alpha)
{
	CvMat mat, oldMat;	// Matrices for weighted add

	cvCopyHist(hist, &oldHist);
	buildHist(src, H, S, V, thresh0, thresh1, mask);

	// Blending histograms
    cvGetMat( hist->bins, &mat, 0, 1 );
	cvGetMat( oldHist->bins, &oldMat, 0, 1 );
	cvAddWeighted(&mat, alpha, &oldMat, 1.0-alpha, 0.0, &mat);

	cvNormalizeHist(hist, 255.0);
}


void CvAddonHSVFilter::backProject(const IplImage *src, IplImage *H, IplImage *S, IplImage *V
	, IplImage *dst
	, const CvScalar &thresh0, const CvScalar &thresh1, const IplImage *mask)
{
	CvScalar lower, upper;
	// DO ERROR CHECK
	
	cvAddonBGR2HSV_LUT(src, H, S, V);

	lowerUpper(thresh0, thresh1, lower, upper);

	cvInRangeS(V, cvScalarAll(lower.val[2]), cvScalarAll(upper.val[2]), V);

	// Combining mask and V threshold
	if(mask != NULL)
		cvOr(V, mask, V);
	
	planes[0] = H; 
	planes[1] = S;
	cvCalcBackProject(planes, dst, hist);

	// Masking Result
	cvAnd(V, dst, dst);
}
