#ifndef _CVADDON_ANIMATION_PAINTER_H
#define _CVADDON_ANIMATION_PAINTER_H

////////////////////////////////////////////////////////////
//       CvAddon Animation Functions and Classes
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Draws animation to an IplImage image
////////////////////////////////////////////////////////////
// Usage:
// ---
// Create painter instances using the constructor. Pass
// the constructor the path to the series of images used 
// as the animation. The images should have the follow attributes:
// 1) Named <basename><number>.<extension>, such as 
//    cat00.png or dog_1001.jpg. The numbering should be fixed 
//    width and increasing from the first animation frame to 
//    the last.
// 2) Black pixels ( {0,0,0} ) at transparent locations (not 
//    painted by the animation painter
// 3) Images should be readable by OpenCV using the cvLoadImage()
//    function
// 
// During construction, the class will attempt to read 
// <numImages> in series, from disk. For example, with 
// "cat00.png" as the <filename> and <numImages> = 5, 
// cat00.png, cat01.png, cat02.png, cat03.png, cat04.png, cat05.png
// will be read from disk. 
//
// To draw the animation, simply use the paint() member 
// function. The location of the images are specified by 
// <rect> and the image being drawn from the series is controlled
// by index. Note that the index is relative to the first 
// image read, and may not be the same as the number in the image 
// name.
////////////////////////////////////////////////////////////

#include <cv.h>

#include "filename.h"
#include "cvaddon_animate.h"

class CvAddonAnimationPainter
{
	
public:
	CvAddonAnimationPainter(const char *filename, const int &numImages);
	~CvAddonAnimationPainter();
	bool paint(IplImage *img, const CvRect &rect, const int& index);

	whFilename file;	// Filename
	int numOfImages;	// Number of images

private:
	CvAddonImageBuf *imgBuf;
	CvAddonAnimFrameInfo info;
	IplImage **images;
	IplImage **masks;

	IplImage *img;
	IplImage *mask;
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
		cerr << file.str() << endl;
		mask = cvLoadImage(file.str().c_str(), 0);

		images[i] = img;
		masks[i] = mask;

		cvAddonAddFrame(imgBuf, img, mask);

		++i;
		file.number++;
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
}

bool CvAddonAnimationPainter :: paint(IplImage *dst, const CvRect &rect, const int& index)
{
	if(index < 0) return false;

	info.dstRect = rect;	
	info.frameIndex = index % numOfImages;
	cvAddonPaintFrame(dst, imgBuf, info);

	if(index >= numOfImages) return false;
	return true;
}

#endif