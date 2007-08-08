#pragma once

// Functions dealing specifically with stereo geometry (between two cameras)

#include "cvaddon_geometry_data.h"

// Returns 1 if an intersect is found, else 0
template <typename T>
inline int linePlaneIntersect(const Line3D<T>& l, const PlaneHessian3D<T>& p, Point3D<T> &result)
{
	T C0 = p.a*(l.x1.x - l.x0.x) + p.b*(l.x1.y - l.x0.y) + p.c*(l.x1.z - l.x0.z);
	if(C0 == 0) return 0;
	
	T C1 = p.a*l.x0.x + p.b*l.x0.y + p.c*l.x0.z + p.d;
	T t = C1 / -C0;

	result.x = (l.x1.x - l.x0.x)*t + l.x0.x;
	result.y = (l.x1.y - l.x0.y)*t + l.x0.y;
	result.z = (l.x1.z - l.x0.z)*t + l.x0.z;

	return 1;
}

// Finds angle between a line, and a plane's normal. If line doesn't intersect plane, 
// returns 0, else, returns 1. Angle is in radians.
template <typename T>
inline int linePlaneNormalAngle(const Line3D<T>& l, const PlaneHessian3D<T>& p, T &angle, Point3D<T> &intersect)
{
	int status = linePlaneIntersect(l, p, intersect);

	if(status == 0) {
//		cerr << " NO INTERSECT " << endl;
		return 0;
	}

	Normal3D<T> planeNorm, lineNorm;
	planeToNormal(p, planeNorm); 
	lineToNormal(l, lineNorm);

	// Dot product
	T dot = planeNorm.x * lineNorm.x 
		+ planeNorm.y * lineNorm.y + planeNorm.z * lineNorm.z;
	angle = acos(dot);
	return 1;
}
