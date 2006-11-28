#ifndef _CVADDON_IMAGE_READER_H
#define _CVADDON_IMAGE_READER_H

// Reads images from disk into OpenCV IplImages
// Can also read a sequence of images based on an 
// initial filename (like img00.png, img01.png ...etc)
//
// NOTE!: It is up to the USER to DEALLOCATE images created using 
//        load()!!!
//
// Assumptions:
// 1) Filename number >= 0
// 2) Image file is a type that can be loaded using cvLoadImage()
//
//  Usage Example:
//  ---
//	const char* IMAGE_PATH = "C:/my_pics";
//	const char* IMAGE_NAME = "pic000.bmp";
//	CvAddonImageReader images(IMAGE_PATH, IMAGE_NAME);
//
//	IplImage *img;
//	cvNamedWindow("img", 1);
//
//	
//	while(img = images.load())
//	{
//		cvShowImage("img", img);
//		cvReleaseImage(&img);
//		images.next();
//
//		char c = cvWaitKey(10);
//		if(c == 'r') images.reset();
//		if(c == 'x') break;
//	}


// Filename parsing
#include "filename.h"
#include <string>
using std::string;

#include "highgui.h"

class CvAddonImageReader
{
public:
	CvAddonImageReader(const char* path, const char* name);
	~CvAddonImageReader();

	IplImage* load();
	
	inline void next() { filename.number++; }

	inline bool prev()
	{
		filename.number--;
		if(filename.number < 0) {
			filename.number = 0;
			return false;
		}
		return true;
	}

	// Start at the first file again
	inline void reset() { filename.number = firstNum; }

private:
	whFilename filename;
	string imagePath;
	int firstNum;
};

CvAddonImageReader::CvAddonImageReader(const char* path, const char* name)
: filename(string(name)), imagePath(path), firstNum(filename.number)
{

}

CvAddonImageReader::~CvAddonImageReader()
{

}

inline IplImage* CvAddonImageReader::load()
{
#ifdef _DEBUG
	cerr << "Name of Image Loaded: " << (imagePath + "/" + filename.str()) << endl;
#endif
	return cvLoadImage( (imagePath + "/" + filename.str()).c_str() );
}

#endif