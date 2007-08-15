// TODO: UPDATE to use new fast sym etc...

// Functions dealing with symmetry matching and triangulation 
// between stereo images

#include <cv.h>
#include <math.h>

#include "cvaddon_stereo_sym.h"

#include "../cvaddon_fast_sym/cvaddon_fast_sym_detect.h"
#include "cvaddon_geometry_data.h"
#include "cvaddon_stereo_calib.h"
#include "cvaddon_geometry3D.h"
#include "cvaddon_triangle_intersect.h"

// To find end points of symmetry lines in image
#include "../cvaddon_util/cvaddon_draw.h"

#include <iostream>
#include <vector>
#include <utility>
using std::cerr;
using std::endl;
using std::vector;
using std::pair;

// Allowable symmetry angles given Table Plane
static const float MAX_ANGLE_DEVIATION = 0.17453292519943f;	// 10 degrees deviation from vertical allowed
//static const float MAX_ANGLE_DEVIATION = 0.08726646259972f;	// 5 degrees deviation from vertical allowed

// Distance Threholds for considering 3D symmetry axes as being valid
static const float MIN_TRIANGULATION_DISTANCE = 300.0f;		// Minimum distance considered
static const float MAX_TRIANGULATION_DISTANCE = 1500.0f;		// Max distance considered

// Distance used to generate 3D triangular planes to find left-right sym plane intersections
static const float SYM_Z = MAX_TRIANGULATION_DISTANCE;		

// NEW - Similarity Threshold (proportional ratio) to 
static const float SYM_SIM_RATIO = 0.75f;

//inline void findSymEndPoints(const CvPoint& peak, const FastSymDetector &fastSym, CvPoint2D32f &pt1, CvPoint2D32f &pt2)
//{
//	float r = fastSym.rIndex2Pixel(peak.x);
//	float th = fastSym.thetaIndex2Radians(peak.y);
//
//	float w = fastSym.imgWidth;
//	float h = fastSym.imgHeight;
//
//	pt1.x = r*cosf(th) + fastSym.IMG_DIAG/2*cosf(th - PI_2);
//	pt1.y = r*sinf(th) + fastSym.IMG_DIAG/2*sinf(th - PI_2);
//	pt2.x = r*cosf(th) - fastSym.IMG_DIAG/2*cosf(th - PI_2);
//	pt2.y = r*sinf(th) - fastSym.IMG_DIAG/2*sinf(th - PI_2);
//
//	pt1.x += (float)(w - 1.0f)/2.0f;
//	pt1.y += (float)(h - 1.0f)/2.0f;
//	pt2.x += (float)(w - 1.0f)/2.0f;
//	pt2.y += (float)(h - 1.0f)/2.0f;
//}

// Normalizes 2D locations of pixels given camera instrinsics
// Based on normalize.m and comp_distortion_oulu.m from 
// the MATLAB calibration toolbox
inline void normalizePixel(const CvPoint2D32f &pt, const CvAddonCameraIntrinsics &params, CvPoint2D32f &result)
{
	static const int UNDISTORT_ITERATIONS = 20;

	result.x = (pt.x - params.cc[0]) / params.fc[0];
	result.y = (pt.y - params.cc[1]) / params.fc[1];

	// Lens Distortion
    float k1 = params.kc[0];
    float k2 = params.kc[1];
    float k3 = params.kc[4];
    float p1 = params.kc[2];
    float p2 = params.kc[3];
    
	// Iterative undistort
	CvPoint2D32f tmp = result;	// Initial guess
	CvPoint2D32f delta;
	int i;
	float r_2, k_radial;
    for(i = 0; i < UNDISTORT_ITERATIONS; ++i) 
	{
		// r_2 = sum(x.^2);
		r_2 = tmp.x*tmp.x + tmp.y*tmp.y;

		// k_radial =  1 + k1 * r_2 + k2 * r_2.^2 + k3 * r_2.^3;
		k_radial = 1 + k1*r_2 + k2*r_2*r_2 + k3*r_2*r_2*r_2;

        //delta_x = [2*p1*x(1,:).*x(2,:) + p2*(r_2 + 2*x(1,:).^2);
		//		p1 * (r_2 + 2*x(2,:).^2)+2*p2*x(1,:).*x(2,:)];
		delta.x = 2*p1*tmp.x*tmp.y + p2*(r_2 + 2*tmp.x*tmp.x);
		delta.y = p1*(r_2 + 2*tmp.y*tmp.y) + 2*p2*tmp.x*tmp.y;

        //x = (xd - delta_x)./(ones(2,1)*k_radial);
		tmp.x = (result.x - delta.x) / k_radial;
		tmp.y = (result.y - delta.y) / k_radial;
    }
	result = tmp;
}

template <typename TT>
inline Point3D<TT> affineTransform3D( const Point3D<TT> &src, const CvAddonStereoParameters &params)
{
	Point3D<TT> res;
	res.x = src.x*params.R[0] + src.y*params.R[3] + src.z*params.R[6] + params.T[0];
	res.y = src.x*params.R[1] + src.y*params.R[4] + src.z*params.R[7] + params.T[1];
	res.z = src.x*params.R[2] + src.y*params.R[5] + src.z*params.R[8] + params.T[2];

	return res;
}

int cvAddonTriangluateSymLines(CvAddonFastSymResults &leftSymResults
	, CvAddonFastSymResults &rightSymResults
	, const PlaneHessian3D<float> &tablePlane
	, const CvSize imgSize, const CvAddonStereoParameters &stereoParams
	, vector< Line3D<float> > &symLines3D)
{
	CvPoint2D32f tmp[2];
	Triangle3D<float> leftTri, rightTri;

	// Right Triangle first point is always at {0, 0, 0}
	memset( &(rightTri.x0), 0, sizeof(rightTri.x0) );

	// Calculating Right Camera location and setting 
	// first right triangle point to it
	// right_sym_3D_drawn = [(R'*-T)'; (R'*right_sym_3D(1,:)' - T)'; (R'*right_sym_3D(2,:)' - T)'; (R'*-T)'];
	leftTri.x0.x = stereoParams.T[0];
	leftTri.x0.y = stereoParams.T[1];
	leftTri.x0.z = stereoParams.T[2];

	int leftLen = leftSymResults.numSym;
	int rightLen = rightSymResults.numSym;

	int matchCount = 0;
	int i, j;
	for(i = 0; i < leftLen; ++i)
	{
		CvPoint2D32f x0[2], dummy;
		cvAddonFindPolarLineEndPoints(imgSize
			, leftSymResults.symLines[i].r
			, leftSymResults.symLines[i].theta
			, dummy, x0[0], x0[1]);

		// Normalizing pixels of symmetry line
		normalizePixel(x0[0], stereoParams.left, tmp[0]);
		normalizePixel(x0[1], stereoParams.left, tmp[1]);

		leftTri.x1.x = tmp[0].x * SYM_Z;
		leftTri.x1.y = tmp[0].y * SYM_Z;
		leftTri.x1.z = SYM_Z;

		leftTri.x2.x = tmp[1].x * SYM_Z;
		leftTri.x2.y = tmp[1].y * SYM_Z;
		leftTri.x2.z = SYM_Z;

		// Transforming 3D locations of pixels to be relative to 
		// Right camera's coordinate system
		leftTri.x1 = affineTransform3D(leftTri.x1, stereoParams);
		leftTri.x2 = affineTransform3D(leftTri.x2, stereoParams);

		for(j = 0; j < rightLen; ++j)
		{
			CvPoint2D32f x1[2];

			// TODO optimize this, pre-calculate outside the loop
			cvAddonFindPolarLineEndPoints(imgSize
				, rightSymResults.symLines[j].r
				, rightSymResults.symLines[j].theta
				, dummy, x1[0], x1[1]);

			// Normalizing pixels of symmetry line
			normalizePixel(x1[0], stereoParams.right, tmp[0]);
			normalizePixel(x1[1], stereoParams.right, tmp[1]);

			rightTri.x1.x = tmp[0].x * SYM_Z;
			rightTri.x1.y = tmp[0].y * SYM_Z;
			rightTri.x1.z = SYM_Z;

			rightTri.x2.x = tmp[1].x * SYM_Z;
			rightTri.x2.y = tmp[1].y * SYM_Z;
			rightTri.x2.z = SYM_Z;

			// Finding 3D intersect
			int isCoplanar = 0;
			float V0[3], V1[3], V2[3], U0[3], U1[3], U2[3], intersect0[3], intersect1[3];

			memcpy(V0, &leftTri.x0, sizeof(V0));
			memcpy(V1, &leftTri.x1, sizeof(V1));
			memcpy(V2, &leftTri.x2, sizeof(V2));
			memcpy(U0, &rightTri.x0, sizeof(U0));
			memcpy(U1, &rightTri.x1, sizeof(U1));
			memcpy(U2, &rightTri.x2, sizeof(U2));

			int hasIntersect = tri_tri_intersect_with_isectline(V0, V1, V2, U0, U1, U2, &isCoplanar, intersect0, intersect1);

			if(isCoplanar) {
				continue;
			}
			else if(hasIntersect) {
				Line3D<float> symLine;
				memcpy(&(symLine.x0), intersect0, sizeof(intersect0) );
				memcpy(&(symLine.x1), intersect1, sizeof(intersect1) );


				// ========= Rejecting False Matches =========
				float angle;
				Point3D<float> intersect;
				int hasIntersect = linePlaneNormalAngle(symLine, tablePlane, angle, intersect);

//				if(i == 1 && j == 1) 
//				{
//					cerr << "----" << angle <<"----" << endl;
//					printPoint3D(symLine.x0);
//					printPoint3D(symLine.x1);
//					cerr << endl;
//				}

				if(angle > CV_PI/2) {
					angle = fabsf(angle - CV_PI);
				}

				// Checking to see if symmetry line intersects table AND
				// Checking to see if symmetry line is roughly perpendicular to table
				if(hasIntersect == 0 || angle >= MAX_ANGLE_DEVIATION) {
					continue;
				}

				// Checking to see if object (intersect point) is too far
				float dist = sqrtf(intersect.x*intersect.x + intersect.y*intersect.y + intersect.z*intersect.z);
				if(dist > MAX_TRIANGULATION_DISTANCE || dist < MIN_TRIANGULATION_DISTANCE) {
					continue;
				}				

				// New - Checking to see if symmetry lines are of roughly equal strength
				int wL, wR;

				wL = leftSymResults.symLines[i].numOfVotes;
				wR = rightSymResults.symLines[j].numOfVotes;
				
				if(wL > wR && wL * SYM_SIM_RATIO > wR) continue;
				else if(wR > wL && wR * SYM_SIM_RATIO > wL) continue;
				// DEBUG
				else {
					cerr << "wL: " << wL << endl;;
					cerr << "wR: " << wR << endl;;
				}
				
				// ============ Storing Result ===============
				// resizing symLines result vector
				if(matchCount+1 >= symLines3D.size()) {
					symLines3D.reserve(symLines3D.size() + 10);
				}
				//symLines3D.push_back(tmp);
				symLines3D[matchCount++] = symLine;
			}
			else {
				continue;
			}
		}
	}

	symLines3D.resize(matchCount);

//	cerr << matchCount << endl;
//	cerr << matchCount << endl;
//	cerr << matchCount << endl;

	return matchCount;
}
