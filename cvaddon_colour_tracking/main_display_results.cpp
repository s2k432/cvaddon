// Visualization of symmetry lines generated from
// ground truth markers, PCA on colour filter and fast symmetry

#include "cv.h"
#include "highgui.h"

// Image Sequence Reader
#include "cvaddon_image_reader.h"

#include "cvaddon_math.h"

#include "cvaddon_draw.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

// Pauses frame progress when using image sequence
int start_frame_step = 0;
int last_frame = -1, this_frame = 0;


bool init = false;

int main()
{
	IplImage* frame = 0;
	IplImage *image = 0;
	CvSize frameSize;
    
	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/red_back_new_50fps/";
	const char* IMAGE_NAME = "default000.bmp";

	const char *LOG_PATH = IMAGE_PATH;
	const char *LOG_NAME = "ground_truth.txt";

	CvAddonImageReader images(IMAGE_PATH, IMAGE_NAME);

	frame = images.load();
	if(!frame) exit(1);

	frameSize = cvGetSize(frame);
	image = cvCreateImage(frameSize, IPL_DEPTH_8U, 3);

	ifstream dataFile( (string(LOG_PATH) + "/" + string(LOG_NAME) ).c_str() );
	if(!dataFile.is_open()) exit(1);

    cvNamedWindow( "Colour Object", 1 );

	CvPoint2D32f center = cvPoint2D32f( ((float)frame->width-1.0f)/2.0f,  ((float)frame->height-1.0f)/2.0f);
	CvPoint2D32f pt0, pt1;
	float r, theta;
	int imageNum;

	for(;;)
    {
		if(this_frame != last_frame) {
			cvReleaseImage(&frame);
			frame = images.load();
			this_frame = images.number();

			dataFile >> imageNum >> pt0.x >> pt0.y >> pt1.x >> pt1.y;
			cerr << imageNum << endl;
//			cerr << pt0.x << "," << pt0.y << endl;
		}
        if( !frame ) break;

		image->origin = frame->origin;
        cvCopy( frame, image, 0 );

//		cerr << pt0.x << "," << pt0.y << endl;
//		cerr << pt1.x << "," << pt1.y << endl;
//		cerr << endl;

		cvAddonFindPolarLineFromEndPoints(center, pt0, pt1, r, theta);
		cvAddonDrawPolarLine(image, r, theta, CV_RGB(0,0,0), 1);
		
//		cerr << r << "," << theta << endl;
//		cvAddonDrawPolarLine(image, -183.6840, 0.2746, CV_RGB(0,0,0), 1);

		cvCircle(image, cvPointFrom32f(pt0), 1, CV_RGB(0,255,0), CV_FILLED);
		cvCircle(image, cvPointFrom32f(pt1), 1, CV_RGB(255,0,0), CV_FILLED);


		cvShowImage( "Colour Object", image);

        char c = cvWaitKey(10);
        if( (char) c == 27 || (char) c == 'x' || (char) c == 'X')
            break;
				
		if(c == 'r') {
			images.reset();
			init = false;
		}
		if(c == ' ') {
			start_frame_step ^= 1;
//			images.next();			
			init = true;
		}

		last_frame = this_frame;
		if(start_frame_step)
			images.next();

		this_frame = images.number();
	}

	dataFile.close();
    cvDestroyWindow("Colour Object");
	return 0;
}