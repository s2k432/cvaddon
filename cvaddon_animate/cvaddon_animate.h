#ifndef CVADDONANIMATE_H
#define CVADDONANIMATE_H

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




