#ifndef CVADDONANIMATE_H
#define CVADDONANIMATE_H

////////////////////////////////////////////////////////////
//       CvAddon Animation Functions and Classes
////////////////////////////////////////////////////////////
// Original Code By Alan Zhang
// Maintained by Wai Ho Li
////////////////////////////////////////////////////////////
// This code is designed to paint animation onto IplImages
// The code can mask and scale the animation, which is 
// represented by a series of IplImages. 
////////////////////////////////////////////////////////////
// Usage:
// ---
// See cvaddon_animation_painter.h for a class that 
// can load images and disk and paint animation onto 
// IplImages
////////////////////////////////////////////////////////////

#include <cv.h>
#include <cxcore.h>


struct CvAddonImageBuf
{
public:
	int numImgs;
	int maxNumImgs;
	IplImage** imgs;
	IplImage** masks;
};

struct CvAddonAnimFrameInfo
{
public:
	CvRect dstRect;
	int frameIndex;
};

CvAddonImageBuf* cvAddonCreateImageBuf(int maxNumImgsT);
void cvAddonReleaseImageBuf(CvAddonImageBuf** buf);
int cvAddonAddFrame(CvAddonImageBuf* buf, IplImage* img, IplImage* mask);
int cvAddonPaintFrame(IplImage* img, CvAddonImageBuf* buf, CvAddonAnimFrameInfo info);



#endif




