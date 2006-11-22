/*

// Uncomment to use LUT (~16MB in size) for BGR to HSV colour conversions
// This is ~4 times faster on a Pentium M 1.73GHz laptop
// Requires cvaddon_fast_bgr2hsv.h
#define USE_HSV_LUT

#include "cvaddon_hsv_filter.h"

#include <iostream>
using std::cerr;
using std::endl;
using std::string;

#ifdef USE_HSV_LUT
	#include "cvaddon_fast_bgr2hsv.h"
#endif

// Scale factor used for visualization (size of each histogram bin)
// for CvAddonHSVFilter::draw()
static const int SCALE = 8;

CvAddonHSVFilter::CvAddonHSVFilter(const int& hBins, const int& sBins)
: H_BINS(hBins), S_BINS(sBins)
, hsv(NULL), hPlane(NULL), sPlane(NULL), vPlane(NULL)
{
	CV_FUNCNAME( "CvAddonHSVFilter Constructor" );

	__BEGIN__;

//	size = imgSize;
//
//    hsv = cvCreateImage(size, 8, 3 );
//	hsvMask = cvCreateImage(size, 8, 1);
//	externalMask = cvCreateImage(size, 8, 1);
//
//    hPlane = cvCreateImage(size, 8, 1 );
//    sPlane = cvCreateImage(size, 8, 1 );
//	vPlane = cvCreateImage(size, 8, 1 );
	
//	planes[0] = hPlane;		planes[1] = sPlane;
    
	HIST_SIZE[0] = H_BINS;	HIST_SIZE[1] = S_BINS;
	
	hist = cvCreateHist( 2, HIST_SIZE, CV_HIST_ARRAY, RANGES, 1 );

	memset(&mat, 0, sizeof(CvMat));

	H_RANGES[0] = 0;		H_RANGES[1] = 180;
	S_RANGES[0] = 0;		S_RANGES[1] = 255;
	RANGES[0] = H_RANGES;	RANGES[1] = S_RANGES;

    histImg = cvCreateImage( cvSize(H_BINS*SCALE, S_BINS*SCALE), 8, 3 );
	histImgRGB = cvCreateImage( cvSize(H_BINS*SCALE, S_BINS*SCALE), 8, 3 );

#ifdef USE_HSV_LUT
	cvAddonInitHsvLut();
#endif
}


CvAddonHSVFilter::~CvAddonHSVFilter() 
{
	if(hsv != NULL) cvReleaseImage(&hsv);
//	cvReleaseImage(&hsvMask);
//	cvReleaseImage(&externalMask);
	
	if(hPlane != NULL) cvReleaseImage(&hPlane);
	if(sPlane != NULL) cvReleaseImage(&sPlane);
	if(vPlane != NULL) cvReleaseImage(&vPlane);
	
	cvReleaseImage(&histImg);
	cvReleaseImage(&histImgRGB);
	cvReleaseHist(&hist);
}

void CvAddonHSVFilter::accum(const IplImage* img, const IplImage* mask)
{
	// Allocate images here!

	// NEW
	planes[0] = hPlane;		planes[1] = sPlane;


	// RGB ==> HSV colour space
	cvCvtColor( img, hsv, CV_BGR2HSV );

	// Rejecting hsv values outside specified range
	cvInRangeS( hsv, hsvMin, hsvMax, hsvMask );

	// Separating out HSV channels. Ignoring Value for now
	cvCvtPixToPlane( hsv, hPlane, sPlane, vPlane, 0 );

	IplImage* maskPtr = NULL;
	if(mask != NULL) {
		cvAnd(hsvMask, mask, externalMask);
		maskPtr = externalMask;
	}

	// Calculating histogram for img
	cvCalcHist( planes, hist, 1, maskPtr );
}


void CvAddonHSVFilter::blend(const IplImage* img, const IplImage* mask)
{
	// RGB ==> HSV colour space
	cvCvtColor( img, hsv, CV_BGR2HSV );

	// Rejecting hsv values outside specified range
    cvInRangeS( hsv, hsvMin, hsvMax, hsvMask );

	// Separating out HSV channels. Ignoring Value for now
	cvCvtPixToPlane( hsv, hPlane, sPlane, vPlane, 0 );

	IplImage* maskPtr = NULL;
	if(mask != NULL) {
		cvAnd(hsvMask, mask, externalMask);
		maskPtr = externalMask;
	}

	// Calculating histogram for img
	cvCalcHist( planes, hist, 0, maskPtr );

	// Add new hist to mat array
	CvMat *curHist = getMat();
	oldHist.add( curHist );

	// blend hist
	std::vector<CvMat*> matArr;
	oldHist.getPointers(matArr);

	cvZero(curHist);
	std::vector<CvMat*>::iterator it;
	for(it = matArr.begin(); it < matArr.end(); ++it) {
		cvAddWeighted(curHist, 1
			, *it, alpha[it - matArr.begin()]
			, 0, curHist);
	}

	float factor = 255;
	cvNormalizeHist(hist, factor);
}


void CvAddonHSVFilter::backProject(IplImage* result, const IplImage* mask, const IplImage* src)
{
	// Updating HSV planes
	if(src != NULL) {

#ifdef USE_HSV_LUT
		cvAddonBGR2HueSat(src, hPlane, sPlane);
#else
		// RGB ==> HSV colour space
		cvCvtColor( src, hsv, CV_BGR2HSV );
		
		// Rejecting hsv values outside specified range
		cvInRangeS( hsv, hsvMin, hsvMax, hsvMask );
		
		// Separating out HSV channels. Ignoring Value for now
		cvCvtPixToPlane( hsv, hPlane, sPlane, vPlane, 0 );
#endif
		
	}
	cvCalcBackProject( planes, result, hist);

#ifndef USE_HSV_LUT
	cvAnd( result, hsvMask, result);
#endif

	if(mask != NULL)
		cvAnd(result, mask, result);
}


// Modified version of histogram draw function from OpenCV doc
IplImage* CvAddonHSVFilter::draw(void)
{
	float maxValue;
	cvGetMinMaxHistValue( hist, 0, &maxValue, 0, 0 );

    cvZero( histImg );
	int h,s;
    for( h = 0; h < H_BINS; h++ )
    {
        for( s = 0; s < S_BINS; s++ )
        {
            float binVal = cvQueryHistValue_2D( hist, h, s );
            int intensity = cvRound(binVal*255/maxValue);
            cvRectangle( histImg, cvPoint( h*SCALE, s*SCALE ),
                         cvPoint( (h+1)*SCALE - 1, (s+1)*SCALE - 1),
                         cvScalar( h / (float)H_BINS * H_RANGES[1]
							, s / (float)S_BINS * S_RANGES[1]
							, 255 - intensity
							, 0 ), 
                         CV_FILLED );
        }
    }
	cvCvtColor(histImg, histImgRGB, CV_HSV2BGR);
	return histImgRGB;
}

//// Saves file as <name>.xml
//// Data structure has <name> as its structure name
//void CvAddonHSVFilter::saveToFile(const string& name, const string& path) {
//	CvMat* mat = getMat();
//	CvFileStorage* fs = cvOpenFileStorage( (path + "/" + name + ".xml").c_str() , 0, CV_STORAGE_WRITE );
//	cvWrite( fs, name.c_str(), mat, cvAttrList(0,0) );
//	cvReleaseFileStorage( &fs );
//}
//
//// Load File into Histogram ( file stored using saveToFile() )
//// Returns false if data size mismatch
//bool CvAddonHSVFilter::loadFromFile(const string& name, const string& path) {
//	CvMat* mat = getMat();
//	CvFileStorage* fs = cvOpenFileStorage( (path + "/" + name + ".xml").c_str() , 0, CV_STORAGE_READ );
//
//	CvMat *readMat = (CvMat*)cvReadByName( fs, NULL, name.c_str(), NULL);
//
//	if(readMat == NULL || mat == NULL)
//		return false;
//
//	CvSize matSize = cvGetSize(mat);
//	CvSize readSize = cvGetSize(readMat);
//
//	if(matSize.width != readSize.width || matSize.height != matSize.width)
//		return false;
//
//	cvCopy(readMat, mat);
//	cvReleaseFileStorage( &fs );		
//	return true;
//}

*/