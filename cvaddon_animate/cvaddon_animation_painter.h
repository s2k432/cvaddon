#ifndef _CVADDON_ANIMATION_PAINTER_H
#define _CVADDON_ANIMATION_PAINTER_H

#include <cv.h>

// Draws animation to an image
#include "filename.h"
#include "cvaddon_animate.h"

//#include "cvaddon_display.h"

class CvAddonAnimationPainter
{
	
public:
	CvAddonAnimationPainter(const char *filename, const int &numImages);
	~CvAddonAnimationPainter();
	bool paint(IplImage *img, const CvRect &rect, const int& index);

	whFilename file;	// Filename
	int numOfImages;			// Number of images

private:
	CvAddonImageBuf *imgBuf;
	CvAddonAnimFrameInfo info;
	IplImage **images;
	IplImage **masks;

	IplImage *img;
	IplImage *mask;

//	imgBuf = cvAddonCreateImageBuf(10);
//
//	img = cvLoadImage("C:/Alan/research/mycode/SIFT/test1.png", 1);
//	mask = cvLoadImage("C:/Alan/research/mycode/SIFT/test1.png", 0);
//	cvAddonAddFrame(imgBuf, img, mask);
//	img = cvLoadImage("C:/Alan/research/mycode/SIFT/test2.png", 1);
//	mask = cvLoadImage("C:/Alan/research/mycode/SIFT/test2.png", 0);
//	cvAddonAddFrame(imgBuf, img, mask);
//	img = cvLoadImage("C:/Alan/research/mycode/SIFT/test3.png", 1);
//	mask = cvLoadImage("C:/Alan/research/mycode/SIFT/test3.png", 0);
//	cvAddonAddFrame(imgBuf, img, mask);
//	
//	img = cvLoadImage("C:/Alan/research/rawdata/panvids/with_odom/capture1_frame1.png", 1);
//	img2 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
//
//	int ret, i;
//	CvAddonAnimFrameInfo info;
//	for (i = 0; i < 100; ++i) {
//		cvCopy(img, img2);
//		info.dstRect = cvRect(10,10,300,300);
//		info.frameIndex = i%3;
//		ret = cvAddonPaintFrame(img2, imgBuf, info);
//		cvShowImage("window1", img2);
//		CV_PAUSE;
//	}


};

CvAddonAnimationPainter :: CvAddonAnimationPainter(const char *filename, const int &numImages)
: file(filename), mask(NULL), img(NULL)
{
	if(numImages <= 0) exit(1);

	imgBuf = cvAddonCreateImageBuf(numImages);
	images = new IplImage*[numImages];
	masks = new IplImage*[numImages];

	int i = 0;
	while( img = cvLoadImage(file.str().c_str(), 1) )
	{
//		whShowImageOnce(img);
//
		cerr << file.str() << endl;
		mask = cvLoadImage(file.str().c_str(), 0);
		
		// This will hopefully prevent black borders 
		// appearing because of the image resizing
		cvErode(mask, mask);

// DEBUG
//		IplImage *tmp = cvCloneImage(img);
//		cvSet(tmp, cvScalarAll(255));
//
//		cvCopy(img, tmp, mask);
//
//		cvAddonShowImageOnce(tmp);

		images[i] = img;
		masks[i] = mask;

		cvAddonAddFrame(imgBuf, img, mask);

		++i;
		file.number++;

//		whShowImageOnce(img);
	}
	numOfImages = i;
}

CvAddonAnimationPainter :: ~CvAddonAnimationPainter()
{
	int i;
	for(i = 0; i < numOfImages; ++i) {
		cvReleaseImage(&(images[i]) );
		cvReleaseImage(&(masks[i]) );
	}
//	delete [] images;
//	delete [] masks;
}

bool CvAddonAnimationPainter :: paint(IplImage *dst, const CvRect &rect, const int& index)
{
	if(index < 0) return false;

//	memset(&info, 0, sizeof(info));
	info.dstRect = rect;	
	info.frameIndex = index % numOfImages;
	cvAddonPaintFrame(dst, imgBuf, info);

	if(index >= numOfImages) return false;
	
	return true;
}

#endif