#ifndef _CVADDON_HAAR_H
#define _CVADDON_HAAR_H

////////////////////////////////////////////////////////////
//                 CvAddonHaarDetector Class
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// C++ wrapper for OpenCV's 
// Haar Detector
////////////////////////////////////////////////////////////
// Usage:
// ---
// Create the Haar Detector with the constructor. Make sure 
// the input path points to a valid XML cascade (as used by 
// OpenCV's haar classifier). Detection is performed by the 
// detect() function, which operates on IplImages. See
// the class comments for the effects of the input parameters.
//
// The CvAddonHaarDetectResults class is used to store the
// detection results. A C++ iostream operator (<<) has been 
// defined, so the results can be printed using cerr << results
////////////////////////////////////////////////////////////


#include <cv.h>
#include "cvaddon_print.h"

// Maximum number of objects returned
const int MAX_HAAR_OBJECTS_FOUND = 250;

// Standard OpenCV Haar Classifier XML filenames
#define CV_HAAR_FRONT_FACE_FILE "C:/Program Files/OpenCV/data/haarcascades/haarcascade_frontalface_default.xml"
#define CV_HAAR_PROFILE_FACE_FILE "C:/Program Files/OpenCV/data/haarcascades/haarcascade_profileface.xml"
#define CV_HAAR_FULL_BODY_FILE "C:/Program Files/OpenCV/data/haarcascades/haarcascade_fullbody.xml"
#define CV_HAAR_LOWER_BODY_FILE "C:/Program Files/OpenCV/data/haarcascades/haarcascade_lowerbody.xml"
#define CV_HAAR_UPPER_BODY_FILE "C:/Program Files/OpenCV/data/haarcascades/haarcascade_upperbody.xml"

// Haar Detection Results structure
struct CvAddonHaarDetectResults
{
	CvRect objects[MAX_HAAR_OBJECTS_FOUND];	// Object Bounding Boxes
	int numObjects;	// Number of objects found (always <= MAX_HAAR_OBJECTS_FOUND)
};


// Functions to intialize a boosted haar classifier and 
// to detect objects using the OpenCV Haar Detector
class CvAddonHaarDetector
{
public:
	// Initalization (loading xml cascade) done in constructor
	CvAddonHaarDetector(const char *xmlFilename);
	~CvAddonHaarDetector();

	// Perform Haar Detection on src image
	void detect( 
	const IplImage *src		// Source Image (IPL_DEPTH_8U)
	, CvAddonHaarDetectResults& results	// Detection Results
	, const float &scaleFactor = 1.2f	// Distance between scales
	, const int &minNeigbours = 3		// Min. Neigbours allowed (rest rejected)
	, const CvSize &minWindowSize = cvSize(20,20)	// Smallest window size examined
	, const bool &doCannyPrune = true	// Do Canny Prunning to speed things up (for faces usually)
	, const bool &downSample = true);	// Downsample image to speed things up (cvPyrDown() used)

private:
	CvHaarClassifierCascade* cascade;	// Haar Cascade
	CvMemStorage *storage;	// Haar Detection buffer
};


inline std::ostream& operator<< (std::ostream &o, const CvAddonHaarDetectResults &h)
{
	int i;
	o << h.numObjects << "\n";
	
	for(i = 0; i < h.numObjects; ++i) {
		o << h.objects[i] << "\n";
	}
	return o;
}

#endif


