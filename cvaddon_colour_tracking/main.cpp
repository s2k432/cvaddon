// Based loosely on camshiftdemo of OpenCV 1.0
// Code by Wai Ho Li

#include "cv.h"
#include "highgui.h"

// HSV colour filter
#include "cvaddon_hsv_filter.h"

// Blob extraction library
#include "blob.h"
#include "BlobResult.h"

#include <iostream>
using std::cerr;
using std::endl;

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histImg = 0;
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
int show_blob = 1;

// Default trackbar values
int vmin = 16, vmax = 240, smin = 30;

// Constants
const float HIST_BLEND_ALPHA = 0.2f;

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
    CvCapture* capture = 0;
	IplImage* frame = 0;
    
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

    cerr << "Hot Keys: " << "\n";
	cerr << "\t" << "Quit - ESC or q" << endl;
//	cerr << "\t" << "Start/Pause video - Spacebar" << endl;
//	cerr << "\t" << "Step through video frame by frame - Left/Right Arrows" << endl;
	cerr << "\t" << "Cycle through tracking modes - t" << endl;
	cerr << "\t" << "Restart at first frame - r" << endl;
	cerr << "\t" << "Hide/Show Histogram - h" << endl;
	cerr << "\t" << "Hide/Show Blob Results - b" << endl;
		
    cvNamedWindow( "HSV Histogram", 1 );
    cvNamedWindow( "Colour Object", 1 );
	cvNamedWindow( "Back Projection", 1 );

    cvSetMouseCallback( "Colour Object", mouseCB, 0 );
    cvCreateTrackbar( "Vmin", "Back Projection", &vmin, 256, 0 );
    cvCreateTrackbar( "Vmax", "Back Projection", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "Back Projection", &smin, 256, 0 );

	frame = cvQueryFrame( capture );
	if(!frame) {
		cerr << "Couldn't get video frame!!!" << endl;
		exit(1);
	}

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
	
	// My own hsv filter
	CvAddonHSVFilter hsvFilter(45, 8);

	// cvBlobsList
	CBlobResult blobs;
	CBlob blob1;
	CBlob blob2;

    for(;;)
    {
        
        int i, c;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;

		image->origin = frame->origin;
        cvCopy( frame, image, 0 );
       
        if( track_object )
        {
            int _vmin = vmin, _vmax = vmax, _smin = smin;

            if( track_object < 0 )
            {
//                float max_val = 0.f;
                cvSetImageROI( image, selection );
                cvSetImageROI( H, selection );
				cvSetImageROI( S, selection );
				cvSetImageROI( V, selection );

				if(blend_hist)
					hsvFilter.blendHist(image, H, S, V, CV_HSV(-1, _smin, _vmin), CV_HSV(256, 256, _vmax), NULL, HIST_BLEND_ALPHA );
				else
					hsvFilter.buildHist(image, H, S, V, CV_HSV(-1, _smin, _vmin), CV_HSV(256, 256, _vmax) );
                
                cvResetImageROI(image);
                cvResetImageROI(H);
				cvResetImageROI(S);
				cvResetImageROI(V);

                track_window = selection;
                track_object = 1;

				hsvFilter.drawHist(histImg);
            }

			cvZero(bp);
			bp->origin = image->origin;
			hsvFilter.backProject(image, H, S, V, bp, CV_HSV(-1,_smin, _vmin), CV_HSV(256, 256, _vmax) );

			// Extract the blobs using a threshold of 100 in the image
			blobs = CBlobResult( bp, NULL, 25, true );

			// ( the criteria to filter can be any class derived from COperadorBlob ) 
			blobs.Filter( blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, 500 );


			// from the filtered blobs, get the blob with biggest perimeter
			blobs.GetNthBlob( CBlobGetArea(), 0, blob1 );
			blobs.GetNthBlob( CBlobGetArea(), 1, blob2 );

			// plot the selected blobs in a output image
			if(show_blob) {
				if(blobs.GetNumBlobs() >= 2) {
					blob1.FillBlob( image, CV_RGB( 255, 0, 0 ));
					blob2.FillBlob( image, CV_RGB( 0, 255, 0 ));
				}
			}
//			blobWithLessArea.FillBlob( image, CV_RGB( 0, 255, 0 ));



////            cvCalcBackProject( &hue, backproject, hist );
////            cvAnd( backproject, mask, backproject, 0 );
//            
//			cvCamShift( bp, track_window,
//                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
//                        &track_comp, &track_box );
//            track_window = track_comp.rect;
//            
////            if( backproject_mode )
////                cvCvtColor( backproject, image, CV_GRAY2BGR );
//            if( image->origin )
//                track_box.angle = -track_box.angle;
//
//			// TODO: Bugs out if colour object disappears off the screen quickly 
//			// as it tries to draw outside the window (very rare) 
//            cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );

			CvPoint centroid = cvPoint(blob2.MinX() + (( blob2.MaxX() - blob2.MinX() ) / 2.0), blob2.MinY() + (( blob2.MaxY() - blob2.MinY() ) / 2.0));
			cvCircle(image, centroid, 4, CV_RGB(0,0,255));
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
//        case 'b':
//            backproject_mode ^= 1;
//            break;
//        case 'c':
//            track_object = 0;
//            cvZero( histImg );
//            break;
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
        default:
            ;
        }
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("Back Projection");
    return 0;
}

