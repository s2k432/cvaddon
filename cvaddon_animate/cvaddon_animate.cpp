#include "cvaddon_animate.h"
#include <iostream.h>


CvAddonImageBuf* cvAddonCreateImageBuf(int maxNumImgsT)
{
	CvAddonImageBuf* buf = new CvAddonImageBuf;
	buf->maxNumImgs = maxNumImgsT;
	buf->numImgs = 0;
	buf->imgs = new IplImage*[buf->maxNumImgs];
	buf->masks= new IplImage*[buf->maxNumImgs];
	return buf;
}
void cvAddonReleaseImageBuf(CvAddonImageBuf** buf)
{
	delete [] (*buf)->imgs;
	delete [] (*buf)->masks;
	delete *buf;
	*buf = NULL;
}

int cvAddonAddFrame(CvAddonImageBuf* buf, IplImage* img, IplImage* mask)
{
	if (buf->numImgs >= buf->maxNumImgs) {
		return 0;
	}
	if (mask->nChannels != 1) {
		return 0;
	}
	buf->imgs[buf->numImgs] = img;
	buf->masks[buf->numImgs] = mask;
	++(buf->numImgs);
	return 1;
}

int cvAddonPaintFrame(IplImage* img, CvAddonImageBuf* buf, CvAddonAnimFrameInfo info)
{
	static IplImage* frameT = NULL;
	static IplImage* maskT = NULL;
	CvRect imgRoiOld, imgRoiNew, frameRoi;
	if (frameT == NULL || frameT->width < info.dstRect.width || frameT->height < info.dstRect.height) {	
		cvReleaseImage(&frameT);
		frameT = cvCreateImage(cvSize(info.dstRect.width, info.dstRect.height), IPL_DEPTH_8U, 3);
		cvReleaseImage(&maskT);
		maskT = cvCreateImage(cvSize(info.dstRect.width, info.dstRect.height), IPL_DEPTH_8U, 1);
		cerr << "cvAddonPaintFrame(): image created" << endl;
	}

	if (info.frameIndex >= buf->numImgs) return 0;
	cvSetImageROI(frameT, cvRect(0,0,info.dstRect.width,info.dstRect.height));
	cvResize(buf->imgs[info.frameIndex], frameT);
	cvSetImageROI(maskT, cvRect(0,0,info.dstRect.width,info.dstRect.height));
	cvResize(buf->masks[info.frameIndex], maskT);

	// makes sure we remain within image boundaries
	imgRoiNew = info.dstRect;
	frameRoi = cvRect(0,0,info.dstRect.width,info.dstRect.height);
	if (imgRoiNew.x < 0) {
		imgRoiNew.width += imgRoiNew.x;
		frameRoi.x -= imgRoiNew.x;
		frameRoi.width = imgRoiNew.width;
		imgRoiNew.x = 0;
	}
	if (imgRoiNew.y < 0) {
		imgRoiNew.height += imgRoiNew.y;
		frameRoi.y -= imgRoiNew.y;
		frameRoi.height = imgRoiNew.height;
		imgRoiNew.y = 0;
	}
	if (imgRoiNew.x+imgRoiNew.width > img->width) {
		imgRoiNew.width = img->width - imgRoiNew.x;
		frameRoi.width = imgRoiNew.width;

	}
	if (imgRoiNew.y+imgRoiNew.height > img->height) {
		imgRoiNew.height = img->height - imgRoiNew.y;
		frameRoi.height = imgRoiNew.height;
	}

	imgRoiOld = cvGetImageROI(img);
	cvSetImageROI(img, imgRoiNew);
	cvSetImageROI(frameT, frameRoi);
	cvSetImageROI(maskT, frameRoi);

//  DEBUG	
//	cerr << "----" << endl;
//	cerr << frameT->width << "," << frameT->height << endl;
//	cerr << maskT->width << "," << maskT->height << endl;
//	cerr << frameRoi.width << "," << frameRoi.height << endl;
//	cerr << "--" << endl;
//	cerr << img->width << "," << img->height << endl;
//	cerr << imgRoiNew.width << "," << imgRoiNew.height << endl;

	cvCopy(frameT, img, maskT); // copy the resized animation frame into the dst image, black pixels are transparent
	
	cvSetImageROI(img, imgRoiOld);

	return 1;
}




















