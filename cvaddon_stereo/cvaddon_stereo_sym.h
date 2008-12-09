#pragma once
// Functions dealing with symmetry matching and triangulation 
// between stereo images

#include <cv.h>

#include "../cvaddon_fast_sym/cvaddon_fast_sym_detect.h"
#include "cvaddon_stereo_calib.h"
#include "cvaddon_geometry_data.h"
#include <vector>
using std::vector;

// Data structure for holding symmetry line results
// This includes the 3D location of triangulated symmetry axes
// and the correspondences that produce them
struct CvAddonSymTriResults
{
	Line3D<float> line;
	CvAddonFastSymLine leftSym, rightSym;
};

// Function returns the number of matching symmetry lines
//
// Parameters
// ---
// - leftSymLines - symmetry detection results from left camera
// - rightSymLines - symmetry detection results from right camera
// - leftLen - Length of the list of symmetry lines
// - tablePlane - 4-parameter hessian description of 3D plane where objects are placed
// - imgSize - Size of left and right image (assumes left and right image are of the same size)
// - stereoParams - Stereo camera intrinsics and extrinics (see cvaddon_stereo_calib.h);
// - symLines3D - Symmetry lines in 3D, represented by its two end points
//
// NOTES
// ---
// - 3D location triangulated relative to RIGHT CAMERA (XR = R * XL + T)
// - Constants (angle tolerance, distance thresholds) are located in the .cpp file
int cvAddonTriangluateSymLines(CvAddonFastSymResults &leftSymResults
	, CvAddonFastSymResults &rightSymResults
	, const PlaneHessian3D<float> &tablePlane
	, const CvSize imgSize, const CvAddonStereoParameters &stereoParams
	, vector< CvAddonSymTriResults > &symLines3D
);
