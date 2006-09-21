#ifndef _CVADDON_PRINT_H
#define _CVADDON_PRINT_H

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
	return o << "{" << p.x << ", " << p.y << "}";
}

inline std::ostream& operator<< (std::ostream &o, const CvRect &r)
{
	return o << "{" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << "}";
}

#endif