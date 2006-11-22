#ifndef _CVADDON_FAST_BGR2HSV_H
#define _CVADDON_FAST_BGR2HSV_H

////////////////////////////////////////////////////////////
//   CvAddon Look up table (LUT) BGR==>HSV Colour Convert
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Uses a large 3*sizeof(uchar)*(256^3) lookup table to perform fast
// and accurate conversions from BGR (or RGB) to HSV 
// colour space. How "fast" the conversion is will depend
// on the RAM transfer speed (I think). It definitely runs 
// faster on my Pentium M 1.73GHz laptop, which has a 533MHz
// CPU<=>RAM clock rate.
//
// **Requires OpenCV to generate the LUT
////////////////////////////////////////////////////////////
// TODO
// ---
// - Fast read/write table to and from disk
////////////////////////////////////////////////////////////

#include <cv.h>
#include <highgui.h>
#include <fstream>

// Call this BEFORE using the conversion function
void cvAddonInitHsvLut(void);

// Colour Conversion function
void cvAddonBGR2HSV_LUT(const IplImage *src, IplImage *hue, IplImage *sat, IplImage *val);

// Read and write to files
bool cvAddonSaveHsvLutToFile(const char* filename);
bool cvAddonReadHsvLutFromFile(const char* filename);

#endif
