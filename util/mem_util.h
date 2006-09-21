#ifndef _WAIHO_MEM_UTIL_H
#define _WAIHO_MEM_UTIL_H


////////////////////////////////////////////////////////////
//             Memory Allocation Functions
////////////////////////////////////////////////////////////
// By Wai Ho Li
////////////////////////////////////////////////////////////
// Various C++ memory-related utility functions
////////////////////////////////////////////////////////////
// Usage Notes:
// ---
// Use allocate2DArrayBlock() over allocate2DArray() if 
// the 2D array is small enough to be allocated in one go
////////////////////////////////////////////////////////////

#ifdef _DEBUG
	#include <iostream>
	using std::cerr;
	using std::endl;
#endif



// De-allocates 2D array
// Assumes that un-allocated pointers are set to NULL
template <typename T>
inline void delete2DArray(T ** &arr, const int &rows)
{
	if(arr != NULL) {
		for(int i = 0; i < rows; ++i)
		{
			if(arr[i] != NULL) {
				delete [] arr[i];
			}
		}
	}
	delete [] arr;
}

// Creates 2D array
// Returns array with <rows> pointers to arrays of length <columns> 
// <arr> is returned as NULL if allocation fails
template <typename T>
inline void allocate2DArray(T ** &arr, const int &rows, const int& columns)
{
	arr = new T*[rows];
	if(arr == NULL) {
#ifdef _DEBUG
		cerr << "arr == NULL" << endl;	
#endif
		return;
	}

	for(int i = 0; i < rows; ++i)
	{
		arr[i] = new T[columns];

		if(arr[i] == NULL) {
#ifdef _DEBUG
		cerr << "arr[i] == NULL" << endl;	
#endif
			// De-allocating what we have so far
			delete2DArray(arr, i);	
			arr = NULL;
			return;
		}
	}
}

// Creates 2D array
// Returns array with <rows> pointers to arrays of length <columns> 
// <arr> is returned as NULL if allocation fails
// Memory allocated internally as one continguous block
template <typename T>
inline void allocate2DArrayBlock(T ** &arr, const int &rows, const int& columns)
{
	arr = new T*[rows];
	if(arr == NULL) {
#ifdef _DEBUG
		cerr << "arr == NULL" << endl;	
#endif
		return;
	}

	T *tmp = new T[columns * rows];
	for(int i = 0; i < rows; ++i)
	{
		arr[i] = tmp + i*columns;
	}
}


template <typename T>
inline void delete2DArrayBlock(T ** &arr)
{
	if(arr != NULL) {
		delete [] *arr;
	}
}


#endif