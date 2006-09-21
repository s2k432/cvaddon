#ifndef _WAIHO_PERMUTATION_H
#define _WAIHO_PERMUTATION_H

// Permutation Template Function

////////////////////////////////////////////////////////////
//         Permutation Generator Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Generates all permutations of input data
////////////////////////////////////////////////////////////
// Usage
// ---
// This function takes a series of input data rows, say
// several measurements taken over a few time periods (n X t)
// and returns ALL possible permutations of the data
// in the SAME TIME ORDER. 
//
// For Example
// --- 
// Data: {1 2} {3 4} {5 6}
// Results: {1 3 5} {1 3 6} {1 4 5} {1 4 6} {2 3 5} ... etc
//
// ** TODO
// ---
// NOTE: Test the order of output data (row,column major) 
//       BEFORE using this in any code
////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>

using std::vector;
using std::next_permutation;


// Note that res is resized by the function, no need to figure out the size
// before hand. 
template <typename T>
inline void genPermutations(const vector< vector<T> >& data, vector< vector<T> >& res)
{
	int dataSize = data.size();
	vector<int> sizes(dataSize);

	int i,j,k,m;
	for(i = 0; i < dataSize; ++i)
		sizes[i] = (data[i]).size();

	int numPerm = 1;
	for(i = 0; i < sizes.size(); ++i)
		numPerm *= sizes[i];

	// Allocating memory for result
	res.resize(numPerm);
	for(i = 0; i < numPerm; ++i)
		res[i].resize( dataSize );

	int repLen = numPerm;
	// looping thru time
	for(j = 0; j < dataSize; ++j) {
		int s = sizes[j];
		repLen /= s;
		int numRepeat = numPerm / (repLen*s);
		
		for(m = 0; m < numPerm; m += repLen*s) {
			for(i = 0; i < sizes[j]; ++i) {
				for(k = 0; k < repLen; ++k) {
					(res[m + i*repLen + k])[j] = (data[j])[i];
				}
			}
		}
	}
}

#endif