#ifndef _CVADDON_FILTER2D_H
#define _CVADDON_FILTER2D_H

////////////////////////////////////////////////////////////
//    GENERIC 2D Image Colour Filter using Histograms
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Used by colour filters such as HSV and rg (normalized RGB)
// for histogram accumlation (building), blending and also
// backprojecting histogram onto an OpenCV IplImage
////////////////////////////////////////////////////////////

#include <cxcore.h>
#include <cv.h>
#include "mem_util.h"
#include "math_macros.h"

#ifdef _DEBUG
	#include <iostream>
	using std::cerr;
	using std::endl;

	#include "cvaddon_display.h"
#endif

static const float MAX_PIXEL_VAL = 255.0f;

// ImgType is either uchar or float
template <typename ImgType>
class CvAddonFilter2D
{
public:
	CvAddonFilter2D(const int& dim0, const int& dim1, const int& i0Max, const int& i1Max);
	~CvAddonFilter2D();

	void buildHist(const IplImage *i0, const IplImage *i1
		, ImgType i0_th0, ImgType i0_th1, ImgType i1_th0, ImgType i1_th1
		, const bool &wrapDim0 = false, const bool &wrapDim1 = false	// Whether to wrap dimensions
		, const IplImage *mask = NULL);

	void blendHist(const IplImage *i0, const IplImage *i1, const float &alpha
		, ImgType i0_th0, ImgType i0_th1, ImgType i1_th0, ImgType i1_th1
		, const bool &wrapDim0 = false, const bool &wrapDim1 = false	// Whether to wrap dimensions
		, const IplImage *mask = NULL);

	const int DIM0, DIM1;
	const int I0_MAX, I1_MAX;

	float **hist, **oldHist;	// Allocated using allocate2DArrayBlock() in mem_util.h 

private:
	float I0_C, I1_C;	// Used to find mapping from pixel value ==> index
	const int SIZE0, SIZE1;	// Actual size of histogram

	// Normalize histogram <h> to have a SUM(bins) == total
	void normHist(float **h, const float& total);
};



// Support Functions
//bool cvAddonSaveFilter2D(const CvAddonFilter2D *ptr, const char* name);
//CvAddonFilter2D* cvAddonLoadFilter2D(const char* name);


// dim0 --> rows, dim1 --> cols
template <typename ImgType>
CvAddonFilter2D<ImgType>::CvAddonFilter2D(const int& dim0, const int& dim1, const int& i0Max, const int& i1Max)
	: DIM0(dim0), DIM1(dim1), I0_MAX(i0Max), I1_MAX(i1Max), SIZE0(DIM0 + 2), SIZE1(DIM1 + 2)
{
	// Histogram padded by 1 bin on all four sides
	allocate2DArrayBlock(hist, SIZE0, SIZE1);	
	allocate2DArrayBlock(oldHist, SIZE0, SIZE1);

	I0_C = (float)I0_MAX / (float)DIM0;
	I1_C = (float)I1_MAX / (float)DIM1;
}

template <typename ImgType>
CvAddonFilter2D<ImgType>::~CvAddonFilter2D()
{
	delete2DArrayBlock(hist);
	delete2DArrayBlock(oldHist);
}

template <typename ImgType>
void CvAddonFilter2D<ImgType>::buildHist(const IplImage *i0, const IplImage *i1
		, ImgType i0_th0, ImgType i0_th1, ImgType i1_th0, ImgType i1_th1
		, const bool &wrapDim0, const bool &wrapDim1
		, const IplImage *mask)
{
	ImgType i0_min, i0_max, i1_min, i1_max;
	ImgType *i0Row, *i1Row, *maskRow;
	int i,j;
	float r0_val, r1_val, c0_val, c1_val;
	CvRect roi, roiTmp;
	CvSize roiSize;

	// For boundary wrapping or accruing (A <==> B, C <==> D)
	float *rowA, *rowB, *rowC, *rowD;

	CV_FUNCNAME( "Filter2D buildHist()" );

	__BEGIN__;

	if(i0 == NULL || i1 == NULL)
		CV_ERROR(CV_StsNullPtr, "Null Pointer Error: i0 or i1 is NULL");
	if(i0->nChannels != 1)
		CV_ERROR(CV_BadNumChannels, "Wrong Channel Number: i0 should have 1 channel only");
	if(i1->nChannels != 1)
		CV_ERROR(CV_BadNumChannels, "Wrong Channel Number: i1 should have 1 channel only");

	roi = cvGetImageROI(i0);
	roiTmp = cvGetImageROI(i1);
	if( memcmp(&roi, &roiTmp, sizeof(roi)) != 0)
		CV_ERROR(CV_BadROISize, "Bad ROI: i0 and i1 ROI does not agree");

	// Clearing Histogram
	memset(*hist, 0, SIZE0*SIZE1*sizeof(float) );

	// Getting thresholds
	i0_max = MAX(i0_th0, i0_th1);
	i0_min = MIN(i0_th0, i0_th1);
	i1_max = MAX(i1_th0, i1_th1);
	i1_min = MIN(i1_th0, i1_th1);

	roiSize = cvSize(roi.width, roi.height);
	for(i = roi.y; i < roi.y + roi.height; ++i)
	{
		i0Row = (uchar*)(i0->imageData + i0->widthStep*i);
		i1Row = (uchar*)(i1->imageData + i1->widthStep*i);
		maskRow = (uchar*)(mask->imageData + mask->widthStep*i);
		for(j = roi.x; j < roi.x + roi.width; ++j)
		{
			if(maskRow[j]) {
				ImgType i0_val = i0Row[j];
				ImgType i1_val = i1Row[j];
				
				if(	i0_val >= i0_min && i0_val <= i0_max && 
					i1_val >= i1_min && i1_val <= i1_max )
				{
					float r = (float)i0_val / I0_C + 0.5f;
					float c = (float)i1_val / I1_C + 0.5f;

					int r0 = (int)r;
					int r1 = r0+1;

					int c0 = (int)c;
					int c1 = c0+1;

					r0_val = (float)r1 - r;
					r1_val = r - (float)r0;
							
					c0_val = (float)c1 - c;
					c1_val = c - (float)c0;
				
					hist[r0][c0] += r0_val*c0_val;
					hist[r0][c1] += r0_val*c1_val;
					hist[r1][c0] += r1_val*c0_val;
					hist[r1][c1] += r1_val*c1_val;
				}
			}
		}
	}

	// Wrapping histogram values (say, for Hue)
	if(wrapDim0) {
		rowA = hist[0];
		rowB = hist[DIM0];
		rowC = hist[SIZE0-1];
		rowD = hist[1];
	}
	else {
		rowA = hist[0];
		rowB = hist[1]; 
		rowC = hist[SIZE0-1];
		rowD = hist[DIM0];	
	}
	for(j = 0; j < SIZE1; ++j)
	{
		rowB[j] += rowA[j];
		rowD[j] += rowC[j];
	}

	if(wrapDim1) {
		for(i = 0; i < SIZE0; ++i)
		{
			hist[i][DIM1] += hist[i][0];
			hist[i][1] += hist[i][SIZE1-1];
		}
	}
	else {
		for(i = 0; i < SIZE0; ++i)
		{
			hist[i][1] += hist[i][0];
			hist[i][DIM1] += hist[i][SIZE1-1];
		}
	}

	normHist(hist, MAX_PIXEL_VAL);

	__END__;
}


template <typename ImgType>
void CvAddonFilter2D<ImgType>::blendHist(const IplImage *i0, const IplImage *i1
		, const float &alpha
		, ImgType i0_th0, ImgType i0_th1, ImgType i1_th0, ImgType i1_th1
		, const bool &wrapDim0, const bool &wrapDim1
		, const IplImage *mask)
{
	int i,j;
	float a,b;

	if(alpha > 1.0f || alpha < 0.0f) a = 0.5f;
	else a = alpha;

	b = 1.0f -  a;

	// Backing up old histogram
	memcpy(*oldHist, *hist, sizeof(float)*SIZE0*SIZE1);

	// Building histogram from i0 and i1
	buildHist(i0, i1, i0_th0, i0_th1, i1_th0, i1_th1, wrapDim0, wrapDim1, mask);

	// Blending results
	for(i = 1; i <= DIM0; ++i)
	{
		for(j = 1; j <= DIM1; ++j)
		{
			hist[i][j] *= a;
			hist[i][j] += b * oldHist[i][j];
		}
	}
}


template <typename ImgType>
inline std::ostream& operator<< (std::ostream &o, const CvAddonFilter2D<ImgType> &filt)
{
	int i,j;
	for(i = 1; i <= filt.DIM0; ++i) {
		for(j = 1; j <= filt.DIM1; ++j) {
			o << filt.hist[i][j] << " , ";
		}
		o << endl;
	}
	return o << endl;
}

template <typename ImgType>
void CvAddonFilter2D<ImgType>::normHist(float **h, const float& total)
{
	int i,j;
	float alpha;
	float sum = 0.0f;

	for(i = 1; i <= DIM0; ++i) {
		for(j = 1; j <= DIM1; ++j) {
			sum += h[i][j];
		}
	}

	if(sum > 0.0f) {
		alpha = total / sum;
		for(i = 1; i <= DIM0; ++i) {
			for(j = 1; j <= DIM1; ++j) {
				h[i][j] *= alpha;
			}
		}
	}
}



#endif