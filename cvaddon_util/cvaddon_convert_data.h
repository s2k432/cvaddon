#ifndef _CVADDON_CONVERT_DATA_H
#define _CVADDON_CONVERT_DATA_H

////////////////////////////////////////////////////////////
//           CvAddon Type Conversion Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Functions to convert data types, between OpenCV structures, and 
// from non-OpenCV data types to OpenCV types.
////////////////////////////////////////////////////////////
// Usage
// ---
// Functions either have the form fn(src, dst) or 
// dst fn(src, params)
////////////////////////////////////////////////////////////

// CvPoint3D32f .* scale == > CvMat
// Assumes single channel float CvMat
template <typename PointType>
inline void cvAddonPoint3D32f2Mat(const PointType& pt, CvMat* mat, const float& scale = 1.0f)
{
	// Transforming Intersection Point to Arm Coordinate Frame
	CV_MAT_ELEM(*mat, float, 0, 0) = pt.x * scale;
	CV_MAT_ELEM(*mat, float, 1, 0) = pt.y * scale;
	CV_MAT_ELEM(*mat, float, 2, 0) = pt.z * scale;
}

inline void cvAddonPoint2D32f2Mat(const CvPoint2D32f& pt, CvMat* mat, const float& scale = 1.0f)
{
	// Transforming Intersection Point to Arm Coordinate Frame
	CV_MAT_ELEM(*mat, float, 0, 0) = pt.x * scale;
	CV_MAT_ELEM(*mat, float, 1, 0) = pt.y * scale;
}

inline void cvAddonMat2Point2D32f(CvMat* mat, CvPoint2D32f& pt, const float& scale = 1.0f)
{
	pt.x = CV_MAT_ELEM(*mat, float, 0, 0);
	pt.y = CV_MAT_ELEM(*mat, float, 1, 0);
}

inline void cvAddonMat2Point3D32f(CvMat* mat, CvPoint3D32f& pt, const float& scale = 1.0f)
{
	pt.x = CV_MAT_ELEM(*mat, float, 0, 0);
	pt.y = CV_MAT_ELEM(*mat, float, 1, 0);
	pt.z = CV_MAT_ELEM(*mat, float, 2, 0);
}


// Copies array <arr> contents to SINGLE CHANNEL image <img>
// Assumes data types are the same between input array and output image
// Returns true on success, else false
template <typename ArrayType>
inline bool cvAddonCopyArrayToImage_1C(const ArrayType *arr, IplImage *img)
{
	if(arr == NULL) return false;
	
	memcpy(img->imageData, arr, img->imageSize);
	return true;
}

// Creates (allocates on stack) a single channel IplImage
// from a 1D array of data <arr>
inline IplImage *cvAddonCreateImageFromArray_1C(const float *arr, const int& w, const int& h)
{
	if(arr == NULL) return NULL;

	IplImage *img = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 1);
	cvAddonCopyArrayToImage_1C(arr, img);
	return img;
}
inline IplImage *cvAddonCreateImageFromArray_1C(const uchar *arr, const int& w, const int& h)
{
	if(arr == NULL) return NULL;

	IplImage *img = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
	cvAddonCopyArrayToImage_1C(arr, img);
	return img;
}


#endif