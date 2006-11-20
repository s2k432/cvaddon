#ifndef _CVADDON_MAT_CBUF_H
#define _CVADDON_MAT_CBUF_H


// Circular buffer of OpenCV CvMat Matrices
// by Wai Ho Li
//
// This class was written for use with colour/HSVFilter to 
// store a recent history of past histograms (which are basically matrices)
//
// Handles allocation of CvMat elements and
// provides a simple circular buffer, with the following
// features:
// 1) Adding elements via add(), overwritting oldest element if buffer is full
// 2) Emptying buffer via clear
// 3) Getting an ordered array (vector) of buffer elements via getPointers()
// 4) Printing all matrices in buffer via print() 


#include <iostream>
#include <vector>

#include <cv.h>

//#include "cvutil.h"		// For whPrintMat

// use getOrdered() to get an ordered vector
// of CvMat pointers. 
class CvAddonMatCBuf
{
public:
	CvAddonMatCBuf(const int& maxSize, const int& matRow, const int& matCol);
	virtual ~CvAddonMatCBuf();
	void clear();
	bool add(const CvMat *mat, const CvMat* mask = NULL);		// Returns true if overwriting
	bool getPointers(std::vector<CvMat*>& matArr);		// Returns true if successful
	void print();

	inline int getSize() { return size; }
	inline bool isEmpty() { return (size <= 0 ? true : false ); }
	inline bool isFull() { return (size >= MAX_SIZE ? true : false ); }

	const int MAX_SIZE;
	const int MAT_ROW;
	const int MAT_COL;

private:
	CvMat **arr;
	int size;
	int start, end;
};

inline CvAddonMatCBuf::CvAddonMatCBuf(const int& maxSize, const int& matRow, const int& matCol)
: MAX_SIZE(maxSize), arr(NULL), MAT_ROW(matRow), MAT_COL(matCol)
, size(0), start(0), end(maxSize - 1)
{
	arr = new CvMat*[MAX_SIZE];
	int i;
	for(i = 0; i < MAX_SIZE; ++i) {
		arr[i] = cvCreateMat(matRow, matCol, CV_32F);
	}
}

inline CvAddonMatCBuf::~CvAddonMatCBuf()
{
	int i;
	for(i = 0; i < MAX_SIZE; ++i) {
		cvReleaseMat( &(arr[i]) );
	}	
	delete [] arr;
}

inline void CvAddonMatCBuf::clear()
{
	start = 0;
	end = MAX_SIZE - 1;
	size = 0;
}

inline bool CvAddonMatCBuf::add(const CvMat *mat, const CvMat* mask) {
	bool overwrite = false;
	if(size < MAX_SIZE) {
		end = (end + 1) % MAX_SIZE;
		++size;
	}
	else {
		start = (start + 1) % MAX_SIZE;
		end = (end + 1) % MAX_SIZE;
		overwrite = true;
	}
	cvCopy(mat, arr[end], mask);
	return overwrite;
}

inline bool CvAddonMatCBuf::getPointers(std::vector<CvMat*>& matArr)
{
	matArr.clear();
	
	if(size <= 0) return false;

	int i;
	for(i = start; matArr.size() < size; i = (i+1)%MAX_SIZE) {
		matArr.push_back(arr[i]);
	}
	return true;
}

//inline void CvAddonMatCBuf::print()
//{
//	int i,j;
//	for(i = start, j = 0; j < size; i = (i+1)%MAX_SIZE, ++j) {
//		cerr << "-== CvMat " << j << "(Index: " << i << ") ==-" << endl;
//		whPrintMat<float>(arr[i]);
//	}
//
//	if(size <= 0) cerr << "** EMPTY **" << endl;
//	else cerr << "==> SIZE: " << size << endl; 
//}

#endif