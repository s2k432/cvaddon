#ifndef _CVADDON_FILE_IO_H
#define _CVADDON_FILE_IO_H

// For saving and loading OpenCV structures to and from files
// By Wai Ho Li
//
// TODO Error checks

#include <cxcore.h>
#include <cv.h>

#include <string>
using std::string;

inline void cvAddonWriteCvArrXML(const CvArr* src, const char* path, const char* name)
{
	CvFileStorage* fs = cvOpenFileStorage( (string(path) + "/" + string(name)).c_str() , 0, CV_STORAGE_WRITE);
	cvWrite( fs, "CvArr", src, cvAttrList(0,0) );
	cvReleaseFileStorage( &fs );
}

inline CvArr* cvAddonReadCvArrXML(const char* path, const char* name)
{
	CvFileStorage* fs = cvOpenFileStorage( (string(path) + "/" + string(name)).c_str(), 0, CV_STORAGE_READ );
	CvMat *readMat = (CvMat*)cvReadByName( fs, NULL, "CvArr", NULL);
	cvReleaseFileStorage( &fs );
	return readMat;
}

#endif