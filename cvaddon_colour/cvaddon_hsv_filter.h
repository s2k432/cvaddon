#ifndef _CVADDON_HSV_FILTER_H
#define _CVADDON_HSV_FILTER_H

////////////////////////////////////////////////////////////
//                 CvAddon HSV Colour Filter
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// TODO
////////////////////////////////////////////////////////////

// Macro for easy definition of HSV range limits
#define CV_HSV( h, s, v )  cvScalar( (h), (s), (v) )

// Ohta and Kanade's colour model
#define CV_KLT( I1, I2, I3 )  cvScalar( (I1), (I2), (I3) )	

#include <cv.h>
#include <iostream>
using std::cerr;
using std::endl;
using std::string;

// CvMat Circular buffer
#include "cvaddon_mat_cbuf.h"

// HSV colour filtering
class CvAddonHSVFilter
{
public:
	CvAddonHSVFilter(const int& hBins, const int& sBins);
	CvAddonHSVFilter(const char* filename);

	~CvAddonHSVFilter();

	


	// ******* Public Constants *******
	const int H_BINS;	// Number of hue bins in histogram
	const int S_BINS;	// Number of saturation bins

private:
	void init(const int& h, const int& s);


	// ******* Internal Data *********
	CvHistogram *hist, *tmpHist;
};



#endif