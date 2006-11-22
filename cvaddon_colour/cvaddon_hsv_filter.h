#ifndef _CVADDON_HSV_FILTER_H
#define _CVADDON_HSV_FILTER_H

////////////////////////////////////////////////////////////
//                 CvAddon HSV Colour Filter
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Usage Notes
// 
// User responsible for creating and keeping track of 
// IplImage for src image, hue, saturation and destination
// images. The class will handle the histogram, its accumulation
// and blending, as well as backprojection.
//
// Will support ROI
////////////////////////////////////////////////////////////

// Macro for easy definition of HSV range limits
#define CV_HSV( h, s, v )  cvScalar( (h), (s), (v) )

#include <cv.h>
#include <iostream>
using std::cerr;
using std::endl;

// HSV colour filtering
class CvAddonHSVFilter
{
public:
	// Normal Constructor
	CvAddonHSVFilter(const int& hBins, const int& sBins);

	// Use this constructor to load histogram data from 
	// file (generated in a previous buildHist() call)
	CvAddonHSVFilter(const char* filename);

	~CvAddonHSVFilter();

	// ************ MAIN FUNCTIONS ************
	bool buildHist(const IplImage *src, IplImage *H, IplImage *S
		, const CvScalar &thresh0, const CvScalar &thresh1
		, const IplImage *mask = NULL);

	bool blendHist(const IplImage *src, IplImage *H, IplImage *S
		, const CvScalar &thresh0, const CvScalar &thresh1
		, const IplImage *mask = NULL);

	bool backProject(const IplImage *src, IplImage *H, IplImage *S
		, const CvScalar &thresh0, const CvScalar &thresh1
		, const IplImage *mask = NULL);


	// ************ SUPPORT FUNCTIONS ************
	// Writes histogram data to file (OpenCV's XML format)
	bool save(const char* filename);

	// Paints an OpenCV image with histogram values
	void drawHist(IplImage *dst);

	// ******* Public Constants *******
	const int H_BINS;	// Number of hue bins in histogram
	const int S_BINS;	// Number of saturation bins

private:
	void init(const int& h, const int& s);

	// Internal function to build histograms
	void hist(const IplImage *H, const IplImage *S, const IplImage *mask);


	// ******* Internal Data *********
	CvHistogram *hist, *newHist;
};



#endif
