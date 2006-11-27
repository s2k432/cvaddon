#include "cvaddon_fast_bgr2hsv.h"

#define LUT_SIZE 16777216

static bool hsvLutFilled = false;

struct HueSatVal
{
	uchar h;
	uchar s;
	uchar v;
};
static HueSatVal hsvLut[LUT_SIZE];


// Call this BEFORE using the conversion function
void cvAddonInitHsvLut(void) 
{
	int i, j, k;
	uchar *rgbRow, *hsvRow;
	IplImage *rgb;
	IplImage *hsv;
		
	if(!hsvLutFilled) {
		rgb = cvCreateImage(cvSize(256, 256), IPL_DEPTH_8U, 3);
		hsv = cvCreateImage(cvSize(256, 256), IPL_DEPTH_8U, 3);


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
					int idx = (k*256+j)*256 + i;
					hsvLut[ idx ].h = hsvRow[3*j];
					hsvLut[ idx ].s = hsvRow[3*j + 1];
					hsvLut[ idx ].v = hsvRow[3*j + 2];
				}
			}
		}
		cvReleaseImage(&rgb);
		cvReleaseImage(&hsv);
		hsvLutFilled = true;
	}
}


void cvAddonBGR2HSV_LUT(const IplImage *src, IplImage *hue, IplImage *sat, IplImage *val)
{
	CvSize srcSize;
	CvSize hueSize;
	CvSize satSize;
	CvSize valSize;	
	
	CV_FUNCNAME( "BGR2HSV LUT" );

	const int B_MULT = 256*256;
	const int G_MULT = 256;

	__BEGIN__;
	if(!hsvLutFilled) {
		CV_ERROR(CV_StsError
			, "LUT not initialized. Use initHsvLut() or readHsvLutFromFile() first!");
	}

	if(src == NULL || hue == NULL || sat == NULL || val == NULL) {
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
	valSize = cvGetSize(val);

	if(srcSize.width != hueSize.width || srcSize.height != hueSize.height) {
		CV_ERROR(CV_StsBadSize, "Bad Size: src and hue mismatch");
	}
	if(srcSize.width != satSize.width || srcSize.height != satSize.height) {
		CV_ERROR(CV_StsBadSize, "Bad Size: src and sat mismatch");
	}
	if(srcSize.width != valSize.width || srcSize.height != valSize.height) {
		CV_ERROR(CV_StsBadSize, "Bad Size: src and val mismatch");
	}

	int i, j;
	int r, g, b;
	int idx;
	uchar *srcRow, *hueRow, *satRow, *valRow;

	for(i = 0; i < srcSize.height; ++i) {
		srcRow = (uchar*)(src->imageData + src->widthStep*i);
		hueRow = (uchar*)(hue->imageData + hue->widthStep*i);
		satRow = (uchar*)(sat->imageData + sat->widthStep*i);
		valRow = (uchar*)(val->imageData + val->widthStep*i);

		for(j = 0; j < srcSize.width; ++j) {
			b = srcRow[j*3];
			g = srcRow[j*3 + 1];
			r = srcRow[j*3 + 2];

			idx = b*B_MULT + g*G_MULT + r;		

			hueRow[j] = hsvLut[idx].h;
			satRow[j] = hsvLut[idx].s;
			valRow[j] = hsvLut[idx].v;			
		}
	}

	__END__;
}

bool cvAddonSaveHsvLutToFile(const char* filename) 
{
	std::ofstream file(filename, std::ios::out|std::ios::binary);

	if(!file.is_open() || !file.good()) return false;

	file.write(reinterpret_cast<char *>(hsvLut),sizeof(HueSatVal)*LUT_SIZE);
	file.close();
	return true;
}

bool cvAddonReadHsvLutFromFile(const char* filename)
{
	std::ifstream file(filename, std::ios::in|std::ios::binary);

	if(!file.is_open() || !file.good()) return false;
	
	file.read(reinterpret_cast<char *>(hsvLut),sizeof(HueSatVal)*LUT_SIZE);
	file.close();
	return (hsvLutFilled = true);
}
