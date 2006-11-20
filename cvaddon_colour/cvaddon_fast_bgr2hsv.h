#ifndef _CVADDON_FAST_BGR2HSV_H
#define _CVADDON_FAST_BGR2HSV_H

// Uses a large (256^3) lookup table to perform fast
// and accurate conversions from BGR (or RGB) to HSV 
// colour space. How "fast" the conversion is will depend
// on the RAM transfer speed (I think). It definitely runs 
// faster on my Pentium M 1.73GHz laptop, which has a 533MHz
// CPU<=>RAM clock rate.

#include <cv.h>
#include <highgui.h>
#include <fstream>

struct HueSatPair
{
	uchar h;
	uchar s;
};

#define LUT_SIZE 16777216
#define LUT_SIZE_2 33554432

static bool hsvLutFilled = false;
static HueSatPair hsvLut[LUT_SIZE];

// Call this BEFORE using the conversion function
inline void cvAddonInitHsvLut(void) 
{
	IplImage *rgb = cvCreateImage(cvSize(256, 256), IPL_DEPTH_8U, 3);
	IplImage *hsv = cvCreateImage(cvSize(256, 256), IPL_DEPTH_8U, 3);

	int i, j, k;
	uchar *rgbRow, *hsvRow;

	// Filling red and yellow channels
	for(i = 0; i < 256; ++i) {
		rgbRow = (uchar*)(rgb->imageData + rgb->widthStep*i);
		for(j = 0; j < 256; ++j) {
			rgbRow[j*3 + 2] = (uchar)i;
			rgbRow[j*3 + 1] = (uchar)j;
		}
	}
	
	for(k = 0; k < 256; ++k) {
		// Blue colour loop
		for(i = 0; i < 256; ++i) {
			rgbRow = (uchar*)(rgb->imageData + rgb->widthStep*i);
			for(j = 0; j < 256; ++j) {
				rgbRow[j*3] = (uchar)k;
			}
		}
		cvCvtColor(rgb, hsv, CV_BGR2HSV);
		
		for(i = 0; i < 256; ++i) {
			hsvRow = (uchar*)(hsv->imageData + hsv->widthStep*i);
			for(j = 0; j < 256; ++j) {
				hsvLut[ (k*256+j)*256 + i].h = hsvRow[3*j];
				hsvLut[ (k*256+j)*256 + i].s = hsvRow[3*j + 1];
			}
		}
	}
	hsvLutFilled = true;
}

inline void cvAddonBGR2HueSat(const IplImage *src, IplImage *hue, IplImage *sat)
{
	CvSize srcSize;
	CvSize hueSize;
	CvSize satSize;
	
	
	CV_FUNCNAME( "bgr2HueSat" );

	const int B_MULT = 256*256;
	const int G_MULT = 256;

	__BEGIN__;
	if(!hsvLutFilled) {
		CV_ERROR(CV_StsError
			, "LUT not initialized. Use initHsvLut() or readHsvLutFromFile() first!");
	}

	if(src == NULL || hue == NULL || sat == NULL) {
		CV_ERROR(CV_StsNullPtr, "Null Pointer Error: src, hue or sat is NULL");
	}

	if(src->nChannels != 3)
		CV_ERROR(CV_BadNumChannels, "Wrong Channel Number: src should have 3 channels (BGR)");
	if(hue->nChannels != 1)
		CV_ERROR(CV_BadNumChannels, "Wrong Channel Number: hue should have 1 channel only");
	if(sat->nChannels != 1)
		CV_ERROR(CV_BadNumChannels, "Wrong Channel Number: sat should have 1 channel only");

	if(src->depth != IPL_DEPTH_8U)
		CV_ERROR(CV_BadDepth, "Wrong Depth: src should be 8-bit unsigned (IPL_DEPTH_8U)");
	if(hue->depth != IPL_DEPTH_8U)
		CV_ERROR(CV_BadDepth, "Wrong Depth: hue should be 8-bit unsigned (IPL_DEPTH_8U)");
	if(sat->depth != IPL_DEPTH_8U)
		CV_ERROR(CV_BadDepth, "Wrong Depth: sat should be 8-bit unsigned (IPL_DEPTH_8U)");

	srcSize = cvGetSize(src);
	hueSize = cvGetSize(hue);
	satSize = cvGetSize(sat);

	if(srcSize.width != hueSize.width || srcSize.height != hueSize.height) {
		CV_ERROR(CV_StsBadSize, "Bad Size: src and hue mismatch");
	}
	if(srcSize.width != satSize.width || srcSize.height != satSize.height) {
		CV_ERROR(CV_StsBadSize, "Bad Size: src and sat mismatch");
	}

	int i, j, r, g, b;
	uchar *srcRow, *hueRow, *satRow;
	int idx;

	for(i = 0; i < srcSize.height; ++i) {
		srcRow = (uchar*)(src->imageData + src->widthStep*i);
		hueRow = (uchar*)(hue->imageData + hue->widthStep*i);
		satRow = (uchar*)(sat->imageData + sat->widthStep*i);
		for(j = 0; j < srcSize.width; ++j) {
			b = srcRow[j*3];
			g = srcRow[j*3 + 1];
			r = srcRow[j*3 + 2];

			idx = b*B_MULT + g*G_MULT + r;
			hueRow[j] = hsvLut[idx].h;
			satRow[j] = hsvLut[idx].s;
		}
	}

	__END__;
}


inline bool cvAddonSaveHsvLutToFile(const char* filename) 
{
	std::ofstream file(filename, std::ios::out|std::ios::binary);

	if(!file.is_open() || !file.good()) return false;

	file.write(reinterpret_cast<char *>(hsvLut),sizeof(HueSatPair)*LUT_SIZE);
	file.close();
	return true;
}

inline bool cvAddonReadHsvLutFromFile(const char* filename)
{
	std::ifstream file(filename, std::ios::in|std::ios::binary);

	if(!file.is_open() || !file.good()) return false;
	
	file.read(reinterpret_cast<char *>(hsvLut),sizeof(HueSatPair)*LUT_SIZE);
	file.close();
	return (hsvLutFilled = true);
}

#endif
