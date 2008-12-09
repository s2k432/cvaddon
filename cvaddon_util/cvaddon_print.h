#pragma once

////////////////////////////////////////////////////////////
//                 CvAddon Printing Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// C++ iostream (<<) operators for OpenCV data types, such
// as CvPoint and CvRect
////////////////////////////////////////////////////////////
// Usage Example:
// ---
// cerr << cvPoint(5,5) << endl; 
////////////////////////////////////////////////////////////

#include <iostream>
#include <cv.h>


inline std::ostream& operator<< (std::ostream &o, const CvPoint &p)
{
	return o << "[" << p.x << ", " << p.y << "]";
}

inline std::ostream& operator<< (std::ostream &o, const CvPoint2D32f &p)
{
	return o << "{" << p.x << ", " << p.y << "}";
}

inline std::ostream& operator<< (std::ostream &o, const CvPoint3D32f &p)
{
	return o << "{" << p.x << ", " << p.y << ", " << p.z << "}";
}


inline std::ostream& operator<< (std::ostream &o, const CvSize &s)
{
	return o << "{" << s.width << ", " << s.height << "}";
}

inline std::ostream& operator<< (std::ostream &o, const CvRect &r)
{
	return o << "{" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << "}";
}

// Only use this for small matrices!
inline std::ostream& operator<< (std::ostream &o, const CvMat *m)
{
	int i, j;
	if(m != NULL) {		
		for(i = 0; i < m->rows; ++i) {
			o << "[ ";
			for(j = 0; j < m->cols; ++j) {
				o << cvGetReal2D(m, i, j) << "  ";  
			}
			o << "]\n";
		}
	}
	return o;
}
