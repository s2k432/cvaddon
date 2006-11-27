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

// TODO: ROI support needs checking and work
void CvAddonHSVFilter::buildHist(const IplImage *src, IplImage *H, IplImage *S, IplImage *V
		, const CvScalar &thresh0, const CvScalar &thresh1, const IplImage *mask)
{
	int i, j;
	int minS;
	CvMat mat;
	CvScalar lower, upper;
	
	// DO ERROR CHECK
	
	cvAddonBGR2HSV_LUT(src, H, S, V);

//	cerr << "Sat:" << (int)CV_IMAGE_ELEM(S, uchar, 1, 1) << endl;

	// Creating mask from V plane of HSV
	lowerUpper(thresh0, thresh1, lower, upper);
	cvInRangeS(V, cvScalarAll(lower.val[2]), cvScalarAll(upper.val[2]), V);

	// Combining mask and V threshold
	if(mask != NULL)
		cvOr(V, mask, V);

	planes[0] = H; 
	planes[1] = S;
	
	cvCalcHist(planes, hist, 0, V);

	// Clearing bins that are outside the S limits
	cvGetMat( hist->bins, &mat, 0, 1 );
	//lower.val[1] upper.val[1]

	minS = cvRound(lower.val[1] * (float)S_BINS / 255.0);

//	cerr << minS << endl;
	for(i = 0; i < H_BINS; ++i)
	{
		for(j = 0; j < minS; ++j)
			CV_MAT_VAL(&mat, float, i, j) = 0;	
	}

	if(cvSum(&mat).val[0] >= 1.0)
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

// ROI not tested. But, it *should* all work, as the BGR2HSV function supports ROI
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

void CvAddonHSVFilter::drawHist(IplImage *dst)
{
	// TODO error check

	int h,s;
	float SCALE_H, SCALE_S;

	float maxValue;
	cvGetMinMaxHistValue( hist, 0, &maxValue, 0, 0 );

    cvZero( dst );

	SCALE_H = dst->height / H_BINS;
	SCALE_S = dst->width / S_BINS;
    for( h = 0; h < H_BINS; h++ )
    {
        for( s = 0; s < S_BINS; s++ )
        {

            float binVal = cvQueryHistValue_2D( hist, h, s );
            int intensity = cvRound(binVal * 255.0 / maxValue);
            cvRectangle( dst, cvPoint(  s*SCALE_S, h*SCALE_H ),
                         cvPoint( (s+1)*SCALE_S - 1, (h+1)*SCALE_H - 1),
                         cvScalar( h / (float)H_BINS * 179
							, s / (float)S_BINS * 255
							, 255 - intensity
							, 0 ), 
                         CV_FILLED );
        }
    }
	cvCvtColor(dst, dst, CV_HSV2BGR);
}