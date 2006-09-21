// Type conversions, such as integer to string and so on
#ifndef _CSTRING_CONVERT_H
#define _CSTRING_CONVERT_H

// Modified version of code from the C++ FAQ Lite 
// --> http://www.parashift.com/c++-faq-lite/misc-technical-issues.html
// 
// For speed and portability sake, I removed the exception code. 
// REAL engineers don't need exceptions, that is what Seg. Faults and 
// fires are for...

/* --- Usage Examples
#include "convert.h"

void myCode()
{
	Foo x;
	...
	std::string s = "this is a Foo: " + stringify(x);
	...
	std::string s2 = ...a string representation of a Foo...;
	...
	Foo x2;
	convert(s2, x2);
	...
	std::string a = ...string representation of an int...;
	std::string b = ...string representation of an int...;
	...
	if (convertTo<int>(a) < convertTo<int>(b))
	...;

}
*/

#include <iostream>
#include <iomanip>

#include <sstream>
#include <string>
#include <typeinfo>
#include <stdexcept>

using std::setfill;
using std::setw;

// Returns a string version of x.
// Returns an empty string is x cannot be converted
// Optionally, a padding of zeros can be added to the start
// of the value
template<typename T>
inline std::string stringify(const T& x, const int& pad = 0)
{
	std::ostringstream o;

	if(pad <= 0) {
		if (!(o << x))
			o.str("");
	}
	else {
		if (!(o << setfill('0') << setw(pad) << x))
			o.str("");
	}	

	return o.str();
}

// Converts string to type T. Returns by reference
// Returns true on success, else false
template<typename T>
inline bool convert(const std::string& s, T& x,
	bool failIfLeftoverChars = true)
{
	std::istringstream i(s);
	char c;
	if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
		return false;
	return true;
}


// Converts string to type T. Return by value
// convertOK set to true on success
template<typename T>
inline T convertTo(const std::string& s, bool& convertOK, 
                bool failIfLeftoverChars = true)
{
	T x;
	convertOK = convert(s, x, failIfLeftoverChars);
	return x;
}

#endif