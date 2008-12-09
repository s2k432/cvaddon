#pragma once

// Templated structures for geometry, and misc.
// functions to convert between them

template <class T>
struct Point3D
{
	T x, y, z;
};

// Plane in the form a*x + b*y + c*z + d = 0
template <class T>
struct PlaneHessian3D
{
	T a, b, c, d;
};

template <class T>
struct Line3D
{
	Point3D<T> x0;
	Point3D<T> x1;
};

template <class T>
struct Triangle3D
{
	Point3D<T> x0;
	Point3D<T> x1;
	Point3D<T> x2;
};

template <class T>
struct Normal3D
{
	T x, y, z;
};

// Returns unit normal
template <typename T>
inline void planeToNormal(const PlaneHessian3D<T> &plane, Normal3D<T> &normal)
{
	T sqrtABC = sqrt(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c);
	normal.x = plane.a / sqrtABC;
	normal.y = plane.b / sqrtABC;
	normal.z = plane.c / sqrtABC;
}

// Testing Template specialization
// Using sqrtf() instead of sqrt()
template <>
inline void planeToNormal(const PlaneHessian3D<float> &plane, Normal3D<float> &normal)
{
	float sqrtABC = sqrtf(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c);
	normal.x = plane.a / sqrtABC;
	normal.y = plane.b / sqrtABC;
	normal.z = plane.c / sqrtABC;	
}

// Coverts terminated line to unit normal ALONG its length (parallel to the line)
// Normal points from x0 to x1
template <typename T>
inline void lineToNormal(const Line3D<T> &line, Normal3D<float> &normal)
{
	T x = line.x1.x - line.x0.x;
	T y = line.x1.y - line.x0.y;
	T z = line.x1.z - line.x0.z;
	T len = sqrt(x*x + y*y + z*z);

	normal.x = x / len;
	normal.y = y / len;
	normal.z = z / len;
}

// Mainly for debugging
template <typename T>
void printPoint3D( T &p)//Point3D<T> &p )
{
	cerr << "{ ";
	cerr << p.x << " , ";
	cerr << p.y << " , ";
	cerr << p.z ;
	cerr << " }";
}