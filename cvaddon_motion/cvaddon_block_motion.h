#ifndef _CVADDON_BLOCK_MOTION_H
#define _CVADDON_BLOCK_MOTION_H

// Block Motion Masking
// By Wai Ho Li
//
// TODO: Good way of handling colour images (multi-channel diff ==> blockSum directly?)

#include "cv.h"
#include "math.h"


// For making motion mask using rectangle drawing code
#include "cvaddon_draw.h"

// Block Motion Detector
class CvAddonBlockMotionDetector 
{
public:
	CvAddonBlockMotionDetector(const CvSize& srcSize, const int& blockSize);
	~CvAddonBlockMotionDetector();

	///////////////////////////////////////////////////////////////////
	// Block-based Motion Detector
	/////////////////////////////////////////////////////////////////// 
	// Modified version of Paul Fitzpatrick's object motion detector. 
	// Produces a binary (8-bit IplImage) image that represent
	// blockSize x blockSize patches in the image containing motion
	//
	// Input Parameters
	// ---
	// img0, img1 - Time-sequential images
	// motionFactor = Multiple of image motion average above which
	//                blocks are considered moving
	// useDiff - if true, <diff> used as input image difference, instead of 
	//           calculating it with absDiff(img0, img1)
	// invertMotionMask - if false, 255 is used for motion blocks, 0 for static
	//                    blocks. If true, 0 for motion, 255 for static
	//
	//
	// Return Values
	// --- 
	// Number of motion blocks found
	// <motionMask>  - Contains a binary mask for 
	//                 moving parts of the image 
	//                 (255 for movement, 0 for static portions
	//                 unless invertMotionMask = true, then it will reversed)
	// <boundingRect> - Bounding rectangle of motion blocks
	/////////////////////////////////////////////////////////////////// 
	int detect(const IplImage* img0, const IplImage* img1, IplImage *diff
		, IplImage *motionMask, CvRect &boundingRect
		, const float &motionFactor = 1.5f, const bool &useDiff = false
		, const bool &invertMotionMask = false) ;

	// TODO
	// Refine motion using symmetry line (as r, theta)

//	// Gets corner points of bounding rectangle of motion blocks
//	void getBoundingRect(CvPoint& p0, CvPoint& p1);
//
//	// Creates binary mask based on motion blocks
//	bool makeMask(IplImage* mask, bool invert = true, bool useBoundingRect = false) const;

private:
	const int BLOCK_SIZE;	// Patch size used in block processing

	CvSize inputSize;
	CvSize blockImageSize;

	IplImage *blockSum;
	IplImage *tmpBuffer;
	IplImage *result;
	IplImage *gray0, *gray1;
};

inline int CvAddonBlockMotionDetector::detect(const IplImage* img0, const IplImage* img1, IplImage *diff
	, IplImage *motionMask, CvRect &boundingRect
	, const float &motionFactor, const bool &useDiff
	, const bool &invertMotionMask)
{
	int i,j;	// diff indices
	int ii,jj;	// blockSum indices
	int m,n;	// Block indices

	// NEW - Ignore pixels with difference value less than this (noise)
	const uchar NOISE_THRESH = 40;

	// TODO image size, channel and type check
	// TODO optimize, and don't use CV_IMAGE_ELEM()

	if(!useDiff) {
		if(img0->nChannels > 1 || img1->nChannels > 1) {
			cvCvtColor(img0, gray0, CV_BGR2GRAY);
			cvCvtColor(img1, gray1, CV_BGR2GRAY);

			cvAbsDiff(gray0, gray1, diff);
		}
		else
			cvAbsDiff(img0, img1, diff);
	}

	cvZero(blockSum);
	float total = 0;
	for(ii = 0, i = 0; ii < blockImageSize.height; ii++) {
		m = i;
		i += BLOCK_SIZE;
		for(; m < i; m++) {
			for(jj = 0, j = 0; jj < blockImageSize.width; jj++) {
				n = j;
				j += BLOCK_SIZE;
				for(; n < j; n++) 
				{
					uchar tmpVal = CV_IMAGE_ELEM(diff, uchar, m, n);

					if(tmpVal > NOISE_THRESH) {
						CV_IMAGE_ELEM(blockSum, float, ii, jj)
							+= (float)tmpVal;
						total += (float)tmpVal;
					}
				}
			}
		}
	}
	float avgSum = total / (blockImageSize.width * blockImageSize.height);

	//// DEBUG
	//cerr << "Total!!! " << total << endl;

	cvThreshold(blockSum, tmpBuffer, motionFactor*avgSum, 255, CV_THRESH_BINARY);

	// Rejecting noisy blocks (small specular reflections, camera noise)
	cvSmooth(tmpBuffer, result, CV_MEDIAN, 3, 3);
	cvDilate(result, result);

	// Getting Bounding Rectangle
	CvPoint2D32f p0, p1;
	int left = blockImageSize.width, right = -1;
	int bottom = blockImageSize.height, top = -1;
	for(ii = 0; ii < blockImageSize.height; ii++) {
		for(jj = 0; jj < blockImageSize.width; jj++) {
			if( CV_IMAGE_ELEM(result, uchar, ii, jj) ) {
				if(left > jj)
					left = jj;
				if(right < jj)
					right = jj;
				if(bottom > ii)
					bottom = ii;
				if(top < ii)
					top = ii;
			}		
		}
	}
	p0.x = (right+1)*BLOCK_SIZE; p0.y = (top+1)*BLOCK_SIZE;
	p1.x = left*BLOCK_SIZE; p1.y = bottom*BLOCK_SIZE;

	boundingRect.x = cvRound(p1.x);
	boundingRect.y = cvRound(p1.y);
	boundingRect.width = cvRound( fabsf(p0.x - p1.x) );
	boundingRect.height = cvRound( fabsf(p0.y - p1.y) );

//	// Making binary mask
	uchar rectVal;
	if(invertMotionMask) { 
		cvSet(motionMask, cvScalarAll(255));
		rectVal = 0;
	}
	else {
		cvZero(motionMask);
		rectVal = 255;
	}
			
	for(ii = 0; ii < blockImageSize.height; ii++) {
		for(jj = 0; jj < blockImageSize.width; jj++) {
			if( CV_IMAGE_ELEM(result, uchar, ii, jj) )
			{			
				cvAddonDrawRectangle(motionMask
					, cvRect(jj*BLOCK_SIZE, ii*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE)
					, CV_RGB(rectVal, rectVal, rectVal), CV_FILLED);
			}
		}
	}

	// TESTING QUICK WAY
	//	cvResize(result, motionMask, CV_INTER_NN);
	
	//// DEBUG
	//CvSize foo = cvGetSize(result);
	//cerr << foo.width << ",," << foo.height << endl;

	return cvCountNonZero(result);
}

inline CvAddonBlockMotionDetector::CvAddonBlockMotionDetector(const CvSize& srcSize, const int& blockSize)
	: BLOCK_SIZE(blockSize)
{
	inputSize.width = srcSize.width;
	inputSize.height = srcSize.height;
	blockImageSize.width = srcSize.width / BLOCK_SIZE;
	blockImageSize.height = srcSize.height / BLOCK_SIZE;

	// Incase grayscale conversion is needed
	gray0 = cvCreateImage( inputSize, IPL_DEPTH_8U, 1);
	gray1 = cvCreateImage( inputSize, IPL_DEPTH_8U, 1);

	blockSum = cvCreateImage( blockImageSize, IPL_DEPTH_32F, 1);
	tmpBuffer = cvCreateImage( blockImageSize, IPL_DEPTH_8U, 1);
	result = cvCreateImage( blockImageSize, IPL_DEPTH_8U, 1);
}

inline CvAddonBlockMotionDetector::~CvAddonBlockMotionDetector() 
{
	cvReleaseImage(&gray0);
	cvReleaseImage(&gray1);
	cvReleaseImage(&blockSum);
	cvReleaseImage(&tmpBuffer);
	cvReleaseImage(&result);
}



//
//// Returns bounding rectangle for motion blocks
//inline void CvAddonBlockMotionDetector::getBoundingRect(CvPoint& p0, CvPoint& p1) 
//const
//{
//	int left = blockImageSize.width, right = -1;
//	int bottom = blockImageSize.height, top = -1;
//	int ii, jj;
//	for(ii = 0; ii < blockImageSize.height; ii++) {
//		for(jj = 0; jj < blockImageSize.width; jj++) {
//			if( WH_GET_PIXEL(result, uchar, ii, jj) ) {
//				if(left > jj)
//					left = jj;
//				if(right < jj)
//					right = jj;
//				if(bottom > ii)
//					bottom = ii;
//				if(top < ii)
//					top = ii;
//			}		
//		}
//	}
//	p0.x = (right+1)*BLOCK_SIZE; p0.y = (top+1)*BLOCK_SIZE;
//	p1.x = left*BLOCK_SIZE; p1.y = bottom*BLOCK_SIZE;
//}

//inline bool CvAddonBlockMotionDetector::makeMask(IplImage* mask, bool invert, bool useBoundingRect)
//const
//{
//	CvSize maskSize = cvGetSize(mask);
//	int ii,jj;
//
//	if(maskSize.width != inputSize.width
//		|| maskSize.height != inputSize.height)
//		return false;
//
//	cvCopy(result, tmpBuffer);
//	if(useBoundingRect) {
//		int left = blockImageSize.width, right = -1;
//		int bottom = blockImageSize.height, top = -1;
//		for(ii = 0; ii < blockImageSize.height; ii++) {
//			for(jj = 0; jj < blockImageSize.width; jj++) {
//				if( WH_GET_PIXEL(result, uchar, ii, jj) ) {
//					if(left > jj) left = jj;
//					if(right < jj) right = jj;
//					if(bottom > ii) bottom = ii;
//					if(top < ii) top = ii;
//				}		
//			}
//		}	
//		cvRectangle( result, cvPoint(left, top)
//					, cvPoint(right, bottom)
//					, cvScalarAll(255), CV_FILLED );
//	}
//
//	uchar rectVal;
//	if(invert) { 
//		cvSet(mask, cvScalarAll(255));
//		rectVal = 0;
//	}
//	else {
//		cvZero(mask);
//		rectVal = 255;
//	}
//			
//	for(ii = 0; ii < blockImageSize.height; ii++) {
//		for(jj = 0; jj < blockImageSize.width; jj++) {
//			if( WH_GET_PIXEL(result, uchar, ii, jj) )
//			{			
//				whDrawRect(mask, cvRect(jj*BLOCK_SIZE
//						, ii*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE), rectVal);
//			}
//		}
//	}
//	return true;
//}





//#include "../fast_sym/fast_sym.h"


//	// Uses symmetry results to refine mask
//	// Symmetry line given as cvPoint(R, theta)
//	inline void refineResults(const CvPoint& symline, const FastSymDetector& fastSym, CvPoint** boundingPoly = NULL)
//	{
//		int rIndex = symline.x;
//		int thIndex = symline.y - fastSym.NUM_H_TH_2;
//
//		float rPixel = fastSym.rIndex2Pixel(rIndex);			
//
//		if(thIndex < 0) {
//			thIndex += fastSym.NUM_H_TH;
//			rPixel *= -1;
//		}
//		rPixel += fastSym.ROT_CEN;
//
//		// Rotating about image center
//		float *R = fastSym.rotMat[thIndex]->data.fl;
//		float R0 = R[0];
//		float R1 = R[1];
//		float R2 = R[2];
//		float R3 = R[3];
//		float R4 = R[4];
//		float R5 = R[5];
//		float RX = R[2] + fastSym.xShift;
//		float RY = R[5] + fastSym.yShift;
//
//		// Reverse Rotation Transform constants
////		float x2 = ( xR2 - RX - (R1/R4)*yR + (R1*RY)/R4 ) / ( R0 - (R1*R3)/R4 );
////		float y2 = ( yR - R3*x2 - RY ) / R4;
//		float C0 = R1/R4;
//		float C1 = R1*RY/R4;
//		float C2 = 1.0f / (R0 - (R1*R3)/R4);
//		float D0 = 1.0f / R4;
//
//		int ii, jj;
//		int n, m;
//		float BLOCK_SIZE_2 = BLOCK_SIZE / 2.0f;
//		for(ii = 0; ii < blockImageSize.height; ii++) {
//			for(jj = 0; jj < blockImageSize.width; jj++) {
//				if( WH_GET_PIXEL(result, uchar, ii, jj) ) {
//					float y = ii * BLOCK_SIZE + BLOCK_SIZE_2;					
//					float x = jj * BLOCK_SIZE + BLOCK_SIZE_2;;
//
//					// Rotating about image centre
//					float xR = R0 * x + R1 * y + RX ;			
//
//					float dx = xR - rPixel;
//
//					if( fabs( dx ) > fastSym.MAX_DIST / 2.0f) {
//						WH_GET_PIXEL(result, uchar, ii, jj) = 0;
//						continue;
//					}
//				
//					float yR = R3 * x + R4 * y + RY ;
//					float xR2 = xR  - dx * 2.0f;
//					float x2 = ( xR2 - RX - C0*yR + C1 ) * C2;
//					float y2 = ( yR - R3*x2 - RY ) * D0;
//
//					int ii2 = (int)( ( y2 - BLOCK_SIZE_2 ) / BLOCK_SIZE );
//					int jj2 = (int)( ( x2 - BLOCK_SIZE_2 ) / BLOCK_SIZE );
//
//					bool reflectionFound = false;
//					for(n = -1; n <= 1; ++n) {
//						if( (n + ii2) >= 0 && (n + ii2) < blockImageSize.height ) {
//							for(m = -1; m <= 1; ++m) {
//								if( (m + jj2) >= 0 && (m + jj2) < blockImageSize.width ) {
//									if( WH_GET_PIXEL(result, uchar, n+ii2, m+jj2) ) {
//										reflectionFound = true;
//										break;
//									}
//								}
//							}
//						}
//						if(reflectionFound) break;
//					}
//					if( !reflectionFound ) WH_GET_PIXEL(result, uchar, ii, jj) = 0;
//				}
//			}
//		}
//
//		// Filling in gaps
//		int votes;
//		for(ii = 1; ii < blockImageSize.height - 1; ii++) {
//			for(jj = 1; jj < blockImageSize.width - 1; jj++) {
//				votes = 0;
//				// Looking at neighbours, 8-connected
//				for(n = -1; n <= 1; ++n) {
//					for(m = -1; m <= 1; ++m) {
//						if(n == 0 && m == 0) continue;
//						
//						if( WH_GET_PIXEL(result, uchar, n+ii, m+jj) ) 
//							++votes;
//					}
//				}
//				if(votes >= 4)
//					WH_GET_PIXEL(result, uchar, ii, jj) = 255;
//			}
//		}
//
//		if(boundingPoly != NULL) {
//			// Finding bounding rectangle (rotated by theta)
//			float minX = inputSize.width * 2.0f, maxX = -1;
//			float minY = inputSize.height * 2.0f, maxY = -1;
//			for(ii = 0; ii < blockImageSize.height; ii++) {
//				for(jj = 0; jj < blockImageSize.width; jj++) {
//					if( WH_GET_PIXEL(result, uchar, ii, jj) ) {
//						float y = ii * BLOCK_SIZE + BLOCK_SIZE_2;					
//						float x = jj * BLOCK_SIZE + BLOCK_SIZE_2;;
//
//						// Rotating about image centre
//						float xR = R0 * x + R1 * y + RX ;			
//						float yR = R3 * x + R4 * y + RY ;
//
//						if(xR > maxX) maxX = xR;
//						if(xR < minX) minX = xR;
//						if(yR > maxY) maxY = yR;
//						if(yR < minY) minY = yR;
//					}
//				}
//			}
//
//			// Rotating bounding points back to original coordinates
//			CvPoint p0, p1, p2, p3;
//			p0.x = ( minX - RX - C0*minY + C1 ) * C2;
//			p0.y = ( minY - R3*p0.x - RY ) * D0;		
//			p1.x = ( minX - RX - C0*maxY + C1 ) * C2;
//			p1.y = ( maxY - R3*p1.x - RY ) * D0;		
//			p2.x = ( maxX - RX - C0*maxY + C1 ) * C2;
//			p2.y = ( maxY - R3*p2.x - RY ) * D0;		
//			p3.x = ( maxX - RX - C0*minY + C1 ) * C2;
//			p3.y = ( minY - R3*p3.x - RY ) * D0;		
//			
//			(*boundingPoly)[0] = p0;
//			(*boundingPoly)[1] = p1;
//			(*boundingPoly)[2] = p2;
//			(*boundingPoly)[3] = p3;
//		}
//	}


#endif