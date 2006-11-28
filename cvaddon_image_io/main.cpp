#include "cv.h"
#include "highgui.h"

#include "cvaddon_display.h"

#include "cvaddon_image_reader.h"

#include <iostream>
using std::cerr;
using std::endl;

int main()
{
	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/white_back_50fps/";
	const char* IMAGE_NAME = "default000.bmp";
	CvAddonImageReader images(IMAGE_PATH, IMAGE_NAME);

	IplImage *img;
	cvNamedWindow("img", 1);

	
	while(img = images.load())
	{
		cvShowImage("img", img);
		cvReleaseImage(&img);
		images.next();

		char c = cvWaitKey(10);
		if(c == 'r') images.reset();
		if(c == 'x') break;
	}
	
	return 0;
}