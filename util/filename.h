// Filename manipulation
#ifndef _WAIHO_FILENAME_H
#define _WAIHO_FILENAME_H

////////////////////////////////////////////////////////////
//             Filename Utility Class
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Easy parsing and incrementing of filenames
////////////////////////////////////////////////////////////
// Usage Example
// ---
//  whFilename filename("test002.png");
//
//  cerr << filename.base; // Prints "test"
//  cerr << filename.ext; // Prints ".png"
//  cerr << filename.digits; // Prints 3
//  cerr << filename.number; // Prints 2
//
//  ++filename.number;
//  filename.digits = 6;
//
//  cerr << filename;	// Prints test000003.png
////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
using std::string;
using std::cerr;
using std::cout;
using std::endl;


// Type conversions, including strings
#include "convert.h"

// Filenames
struct whFilename {
	string ext;
	string base;
	string name;
	size_t dotPos;	// location of '.' in the filename

	// For numbered files. If digits = 0, file not numbered
	int digits;
	int number;

	whFilename(string startName) : digits(0), number(-1)
		, name(startName)
	{
		if(name.empty()) {
			cerr << "Empty filename string!" << endl;
			exit(1);
		}	
		dotPos = name.find(".");
		
		ext = name.substr(dotPos, name.length());

		base = name.substr(0, dotPos);

		size_t len = base.length();
		string::reverse_iterator rev;
		//for(digits = 0, rev = base.rbegin(); rev != base.rend(); rev++, digits++) {
		for(rev = base.rbegin(); rev != base.rend(); rev++) {
			if(!isdigit(*rev))
				break;
		}	
		digits = rev - base.rbegin();

		if(digits > 0) {
			bool dummy;
			number = convertTo<int>(base.substr(len - digits, len), dummy);
			base.erase(len - digits, len);	//Removing number from base string
		}
	}

	inline string str(void) {
		if(digits <= 0)
			return base + ext;
		return base + stringify(number, digits) + ext;
	}

	inline string strNoExt(void) {
		if(digits <= 0)
			return base;
		return base + stringify(number, digits);
	}


	// C++ style printing
	friend std::ostream& operator<< (std::ostream& o, const whFilename& f);
};

inline std::ostream& operator<< (std::ostream& o, const whFilename& f)
{
	if(f.digits <= 0)
		return o << (f.base + f.ext);
	return o << (f.base + stringify(f.number, f.digits) + f.ext);
}





#endif
