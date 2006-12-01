// Tracks two markers above and below 
// an object to get a ground truth symmetry line
// Code by Wai Ho Li
//
//This code was hacked up in a couple of hours and very, very messy.
//As one of the sequences (new_mixed_50fps) had a specularly reflective
//marker (blue electrical tape), the code was ran twice on the object 
//to get both markers. 


#include "cv.h"
#include "highgui.h"

// HSV colour filter
#include "cvaddon_hsv_filter.h"

// Blob extraction library
#include "blob.h"
#include "BlobResult.h"

// Image Sequence Reader
#include "cvaddon_image_reader.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

#include <string>
using std::string;

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histImg = 0, *HSV = 0;
CvHistogram *hist = 0;

int select_object = 0;
int track_object = 0;
int blend_hist = 0;
CvPoint origin;
CvRect selection;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;

// GUI
int show_hist = 1;
int show_blob = 0;
int show_camshift = 0;

// Pauses frame progress when using image sequence
int start_frame_step = 0;
int last_frame = -1, this_frame = 0;

// DOESNT WORK WELL
//// Default trackbar values (for green tape, white_back_50fps)
//int vmin = 79, vmax = 135;
//int smin = 5, smax = 80;
//int hmin = 30, hmax = 150;

// Default trackbar values (for blue tape, white_50fps_scale)
int vmin = 28, vmax = 120;
int smin = 10, smax = 113;
int hmin = 70, hmax = 135;

//// Default trackbar values (for blue tape, red_back_new_50fps)
//int vmin = 35, vmax = 120;
//int smin = 10, smax = 110;
//int hmin = 75, hmax = 175;

//// Default trackbar values (for blue tape, edge_noise_back_new_50fps)
//int vmin = 36, vmax = 100;
//int smin = 22, smax = 100;
//int hmin = 85, hmax = 155;

//// Default trackbar values (for TOP blue tape, mixed_back_new_50fps)
//int vmin = 17, vmax = 100;
//int smin = 15, smax = 118;
//int hmin = 70, hmax = 165;

//// Default trackbar values (for BOTTOM black blob (weights), mixed_back_new_50fps)
//// Blue tape swinging too fast, and specularly reflective at times...
//int vmin = 0, vmax = 53;
//int smin = 0, smax = 256;
//int hmin = 0, hmax = 180;


void mouseCB( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( select_object )
    {
        selection.x = MIN(x,origin.x);
        selection.y = MIN(y,origin.y);
        selection.width = selection.x + CV_IABS(x - origin.x);
        selection.height = selection.y + CV_IABS(y - origin.y);
        
        selection.x = MAX( selection.x, 0 );
        selection.y = MAX( selection.y, 0 );
        selection.width = MIN( selection.width, image->width );
        selection.height = MIN( selection.height, image->height );
        selection.width -= selection.x;
        selection.height -= selection.y;
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_LBUTTONUP:
        select_object = 0;
        if( selection.width > 0 && selection.height > 0 ) {
            track_object = -1;
			blend_hist = 0;
		}
        break;

	// Blend histogram instead
    case CV_EVENT_RBUTTONDOWN:
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_RBUTTONUP:
        select_object = 0;
        if( selection.width > 0 && selection.height > 0 ) {
            track_object = -1;
			blend_hist = 1;
		}
        break;
    }

}



int main( int argc, char** argv )
{
	IplImage* frame = 0;
    
	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/white_50fps_scale/";
	const char* IMAGE_NAME = "default000.bmp";
	const char *LOG_NAME = "ground_truth.txt";

	CvAddonImageReader images(IMAGE_PATH, IMAGE_NAME);

	frame = images.load();

	string logName = string(IMAGE_PATH) + "/" + string(LOG_NAME);
	ofstream logFile;
	logFile.open(logName.c_str());

	if(!logFile.is_open()) {
		cerr << "Couldn't open Log file. Exiting..." << endl;
		exit(1);
	}
    cerr << "Hot Keys: " << "\n";
	cerr << "\t" << "Quit - ESC or q" << endl;
	cerr << "\t" << "Start/Pause frame stepping (image sequence only) - SPACEBAR" << endl;
	cerr << "\t" << "Restart at first frame (image sequence only) - r" << endl;
	cerr << "\t" << "Hide/Show Histogram - h" << endl;
	cerr << "\t" << "Hide/Show Blob Results - b" << endl;
	cerr << "\t" << "Hide/Show Camshift Results - b" << endl;
		
    cvNamedWindow( "HSV Histogram", 1 );
    cvNamedWindow( "Colour Object", 1 );
	cvNamedWindow( "Back Projection", 1 );

    cvSetMouseCallback( "Colour Object", mouseCB, 0 );
    cvCreateTrackbar( "Vmin", "Back Projection", &vmin, 256, 0 );
    cvCreateTrackbar( "Vmax", "Back Projection", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "Back Projection", &smin, 256, 0 );
	cvCreateTrackbar( "Smax", "Back Projection", &smax, 256, 0 );
    cvCreateTrackbar( "Hmin", "Back Projection", &hmin, 180, 0 );
    cvCreateTrackbar( "Hmax", "Back Projection", &hmax, 180, 0 );
 


	CvSize imgSize = cvGetSize(frame);
    image = cvCreateImage( imgSize, IPL_DEPTH_8U, 3 );

	// HSV Filtering
	IplImage *H, *S, *V, *bp, *mask;
    mask = cvCreateImage( imgSize, IPL_DEPTH_8U, 1 );
    bp = cvCreateImage( imgSize, IPL_DEPTH_8U, 1 );
	histImg = cvCreateImage( cvSize(64, 360), IPL_DEPTH_8U, 3 );

	H = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	S = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	V = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	HSV = cvCreateImage( imgSize, IPL_DEPTH_8U, 3);

	IplImage *blob1Img = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	IplImage *blob2Img = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	
	// My own hsv filter
	CvAddonHSVFilter hsvFilter(45, 8);

	// cvBlobsList
	CBlobResult blobs;
	CBlob blob0;
	CBlob blob1;
	CBlob blob2;

	bool init = false;

    for(;;)
    {
        int c;

		if(this_frame != last_frame) {
			cvReleaseImage(&frame);
			frame = images.load();
			this_frame = images.number();
		}
        if( !frame ) break;

		image->origin = frame->origin;
        cvCopy( frame, image, 0 );
       
            int _vmin = vmin, _vmax = vmax;
			int _smin = smin, _smax = smax;
			int _hmin = hmin, _hmax = hmax;

			cvZero(bp);
			bp->origin = image->origin;

			// GROUND TRUTH MANUAL SEG
			cvCvtColor(image, HSV, CV_BGR2HSV);

			CvScalar lower = cvScalar(_hmin, _smin, _vmin);
			CvScalar upper = cvScalar(_hmax, _smax, _vmax);
			cvInRangeS(HSV, lower, upper, V);

//			cvInRangeS(image, lower, upper, V);


//			CvScalar maskColor = CV_RGB(128,128,128);
			CvScalar maskColor = CV_RGB(0,0,0);

//			cvRectangle(V, cvPoint(0,0), cvPoint(132, imgSize.height-1), maskColor, CV_FILLED);
			
			// For red_back_new_50fps & mixed_back_new_50fps
			cvRectangle(V, cvPoint(imgSize.width-1-50,0), cvPoint(imgSize.width-1, imgSize.height-1), maskColor, CV_FILLED);

//			//for mixed_back_new_50fps
			cvRectangle(V, cvPoint(0,imgSize.height-1), cvPoint(imgSize.width-1, imgSize.height-1-50), maskColor, CV_FILLED);
			cvRectangle(V, cvPoint(0,0), cvPoint(55, imgSize.height), maskColor, CV_FILLED);

//			cvRectangle(V, cvPoint(307,imgSize.height-1 - 42), cvPoint(310, imgSize.height-1 - 26), maskColor, CV_FILLED);

//			cvRectangle(V, cvPoint(390,imgSize.height-1), cvPoint(410, imgSize.height-1 - 50), maskColor, CV_FILLED);
			

			cvSmooth(V, bp, CV_MEDIAN, 5, 5);
			

			if(show_blob) {
				blobs = CBlobResult( bp, NULL, 20, true );
				
				// ( the criteria to filter can be any class derived from COperadorBlob ) 
				blobs.Filter( blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, 30);

				// from the filtered blobs, get the blob with biggest perimeter
				blobs.GetNthBlob( CBlobGetArea(), 0, blob0 );
				blobs.GetNthBlob( CBlobGetMaxY(), 1, blob1 );


				blobs.GetNthBlob( CBlobGetMaxY(), blobs.GetNumBlobs()-1, blob2 );
	
				if(blob2.Area() < 50) { blobs.GetNthBlob( CBlobGetMaxY(), blobs.GetNumBlobs()-2, blob2 ); }

				if(blobs.GetNumBlobs() >= 3) {
					blob0.FillBlob( image, CV_RGB( 0, 0, 255 ));

					blob1.FillBlob( image, CV_RGB(0,255,0));

					blob2.FillBlob( image, CV_RGB(255,0,0));
					
//					if(blob1.MinY() < 200) {
//						cerr << "WTF?" << endl;
//						start_frame_step = 0;
//					}


					cvZero(blob1Img);
					cvZero(blob2Img);
					blob1.FillBlob( blob1Img, CV_RGB(255,255,255));
					blob2.FillBlob( blob2Img, CV_RGB(255,255,255));

					logFile << images.number() << "\t";

					// Center of Gravity
					CvMoments moments;
					float dXCenter;
					float dYCenter;

					cvMoments(blob1Img, &moments, 0);
					dXCenter = moments.m10 / moments.m00;
					dYCenter = moments.m01 / moments.m00;

					logFile << dXCenter << "\t" << dYCenter << "\t";
					cvCircle(image, cvPoint(dXCenter, dYCenter), 3, CV_RGB(0,0,0) , CV_FILLED);

					cvMoments(blob2Img, &moments, 0);
					dXCenter = moments.m10 / moments.m00;
					dYCenter = moments.m01 / moments.m00;

					logFile << dXCenter << "\t" << dYCenter << "\t";
					cvCircle(image, cvPoint(dXCenter, dYCenter), 3, CV_RGB(0,0,0) , CV_FILLED);					

					logFile << endl;
				}
				else {
					start_frame_step = 0;
				}
			}

  
		// Select window (exclusion) effect
        if( select_object && selection.width > 0 && selection.height > 0 )
        {
            cvSetImageROI( image, selection );
            cvXorS( image, cvScalarAll(255), image, 0 );
            cvResetImageROI( image );
        }


		cvShowImage( "Colour Object", image);



        cvShowImage( "Back Projection", bp );
        cvShowImage( "HSV Histogram", histImg );

        c = cvWaitKey(10);
        if( (char) c == 27 || (char) c == 'x' || (char) c == 'X')
            break;

        switch( (char) c )
        {
        case 'h':
            show_hist ^= 1;
            if( !show_hist )
                cvDestroyWindow( "HSV Histogram" );
            else
                cvNamedWindow( "HSV Histogram", 1 );
            break;

        case 'b':
            show_blob ^= 1;
            break;

		case 'c':
			show_camshift ^= 1;
        default:
            ;
        }

// Image Sequence input
				
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
	logFile.close();

    cvDestroyWindow("Back Projection");
    return 0;
}

