// HSV colour tracker, uses Hue and Saturation
// histograms. Value (V in HSV) is used to reject
// pixels near the central axis of the HSV cone
// (~= singularity)
//
// The code provides the following tracking modes, all run in parallel
// 1) Camshift tracking (see camshiftdemo.cpp in OpenCV samples)
// 2) Centroid of each frame using cvMoments (no temporal smoothing)
//
// All of the above uses the back projection image as 
// input, so make sure it is reasonably clean and noise-free
//
// Based loosely on camshiftdemo of OpenCV 1.0
// Code by Wai Ho Li

// Uncomment to use webcam (or avi video) as input
//#define CV_CAPTURE

// Uncomment to write results to text files in 
// IMAGE_PATH
// #define WRITE_TRACKING_RESULTS_TO_FILES

#include "cv.h"
#include "highgui.h"

// HSV colour filter
#include "cvaddon_hsv_filter.h"

// Blob extraction library
#include "blob.h"
#include "BlobResult.h"

// Image Sequence Reader
#include "cvaddon_image_reader.h"

// Binary pixel location extraction (edge pixel location)
#include "cvaddon_edge.h"

#include "cvaddon_math.h"

// Block-based motion detection
#include "cvaddon_block_motion.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

IplImage *image = 0, *imageOld = 0, *hsv = 0, *hue = 0;
IplImage *diff, *motionMask;
IplImage *mask = 0, *backproject = 0, *histImg = 0;
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
#ifndef CV_CAPTURE
	int start_frame_step = 0;
	int last_frame = -1, this_frame = 0;
#endif

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
	IplImage* frame = 0;
    
#ifdef CV_CAPTURE
	CvCapture* capture = 0;
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

	frame = cvQueryFrame( capture );
	if(!frame) {
		cerr << "Couldn't get video frame!!!" << endl;
		exit(1);
	}
#else
	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/white_back_50fps/";
	const char* IMAGE_NAME = "default070.bmp";		// Frames overwritten (1001 ==>0001 etc..) until ~069

	// Needs motionMask	
//	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/mixed_back_new_50fps/";
//	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/red_back_new_50fps/";

//	const char* IMAGE_PATH = "F:/_WORK/_PhD/code_and_data/symmetry/images/pendulum_improved/edge_noise_back_new_50fps/";
//	const char* IMAGE_NAME = "default000.bmp";		// Frame skip after 069 for some odd reason

	CvAddonImageReader images(IMAGE_PATH, IMAGE_NAME);

	frame = images.load();
#endif

#ifdef WRITE_TRACKING_RESULTS_TO_FILES
	ofstream pcaFile( (string(IMAGE_PATH) + "/" + "pca_axis.txt").c_str() ); 
#endif

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
	cvNamedWindow( "Motion", 1 );

    cvSetMouseCallback( "Colour Object", mouseCB, 0 );
    cvCreateTrackbar( "Vmin", "Back Projection", &vmin, 256, 0 );
    cvCreateTrackbar( "Vmax", "Back Projection", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "Back Projection", &smin, 256, 0 );



	CvSize imgSize = cvGetSize(frame);
    image = cvCreateImage( imgSize, IPL_DEPTH_8U, 3 );
	imageOld = cvCreateImage( imgSize, IPL_DEPTH_8U, 3 );
	cvZero(imageOld);

	CvPoint2D32f imgOrigin = cvPoint2D32f( 
		((float)imgSize.width - 1.0f) / 2.0f, ((float)imgSize.height - 1.0f) / 2.0f);

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

	// Block Motion
	CvAddonBlockMotionDetector blockMotion(imgSize, 8);
	CvRect boundingRect;
	diff = cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
	motionMask = cvCreateImage( imgSize, IPL_DEPTH_8U, 1 );

    for(;;)
    {
        int c;

#ifdef CV_CAPTURE
        frame = cvQueryFrame( capture );
#else
		if(this_frame != last_frame) {
			cvReleaseImage(&frame);
			frame = images.load();
			this_frame = images.number();
		}
#endif
        if( !frame ) break;

		image->origin = frame->origin;
        cvCopy( frame, image, 0 );

		// TESTING
		int motionCount = blockMotion.detect(image, imageOld, diff, motionMask, boundingRect, 1.5f);

		cvCopy(image, imageOld);


//		cerr << "Motion Count: " << motionCount << endl;

//		cvZero(motionMask);
//		cvAddonDrawRectangle(motionMask, boundingRect, CV_RGB(180, 180, 180), CV_FILLED);

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


			// MOTION MASKING - for similar-coloured background
			if(motionCount > 100) {
				cvAnd(bp, motionMask, bp);
			}


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
//					blob1.FillBlob( image, CV_RGB( 255, 0, 0 ));
//					blob2.FillBlob( image, CV_RGB( 0, 255, 0 ));

					CvPoint centroid = cvPoint(blob2.MinX() + (( blob2.MaxX() - blob2.MinX() ) / 2.0), blob2.MinY() + (( blob2.MaxY() - blob2.MinY() ) / 2.0));

					CvPoint a = cvPoint(blob2.MinX(), blob2.MinY());
					CvPoint b = cvPoint(blob2.MaxX(), blob2.MaxY());

//					cvCircle(image, centroid, 4, CV_RGB(0,0,255), CV_FILLED);
//					cvRectangle(image, a, b, CV_RGB(0,0,255), 2, CV_AA);

					cvZero(V);
					blob2.FillBlob( V, CV_RGB(255,255,255));
	
					const int MAX_BINARY_POINTS = 60000;
					CvPoint nonZero[MAX_BINARY_POINTS];
					int numPoints = cvAddonFindNonZeroPixels<uchar, CvPoint>(V, nonZero, MAX_BINARY_POINTS);

					if(numPoints > 0) {
						
						CvMat *pcaData = cvCreateMat( 2, numPoints, CV_32F );
						CvMat *pcaAvg = cvCreateMat( 2, 1, CV_32F );
						CvMat* eigenVectors = cvCreateMat( 2, 2, CV_32F );
						CvMat* eigenVectorsMax = cvCreateMat( 1, 2, CV_32F );
						CvMat* eigenVectorsMin = cvCreateMat( 1, 2, CV_32F );
						CvMat* eigenValues = cvCreateMat( 2, 1, CV_32F );

						CvMat* pcaProj = cvCreateMat( numPoints, 1, CV_32F );
						CvMat* pcaResults = cvCreateMat( 2, numPoints, CV_32F );

						CvMat *imgCenter = cvCreateMat(2, 1, CV_32F);
						CvMat* centerProj = cvCreateMat( 1, 1, CV_32F );
						CvMat* centerResults = cvCreateMat( 2, 1, CV_32F );

						CV_MAT_VAL(imgCenter, float, 0, 0) = ((float)image->width - 1.0f)/2.0f;
						CV_MAT_VAL(imgCenter, float, 1, 0) = ((float)image->height - 1.0f)/2.0f;

						int i;
						for(i = 0; i < numPoints; ++i)
						{

							// 1st Row (x)
							( (float*)(pcaData->data.ptr) )[i] = nonZero[i].x;

							// 2nd Row (y)
							( (float*)(pcaData->data.ptr + pcaData->step) )[i] = nonZero[i].y;
						}

						// PCA
						cvCalcPCA(pcaData, pcaAvg, eigenValues, eigenVectors, CV_PCA_DATA_AS_COL );

						cerr << CV_MAT_VAL(eigenValues, float, 0, 0) << "," << CV_MAT_VAL(eigenValues, float, 1, 0) << endl;
						cerr << CV_MAT_VAL(eigenVectors, float, 0, 0) << " | " << CV_MAT_VAL(eigenVectors, float, 0, 1) << endl;
						cerr << CV_MAT_VAL(eigenVectors, float, 1, 0) << " | " << CV_MAT_VAL(eigenVectors, float, 1, 1) << endl;

						if(CV_MAT_VAL(eigenValues, float, 0, 0) > CV_MAT_VAL(eigenValues, float, 1, 0)) {
							CV_MAT_VAL(eigenVectorsMax, float, 0, 0) = CV_MAT_VAL(eigenVectors, float, 0, 0);
							CV_MAT_VAL(eigenVectorsMax, float, 0, 1) = CV_MAT_VAL(eigenVectors, float, 0, 1);

							CV_MAT_VAL(eigenVectorsMin, float, 0, 0) = CV_MAT_VAL(eigenVectors, float, 1, 0);
							CV_MAT_VAL(eigenVectorsMin, float, 0, 1) = CV_MAT_VAL(eigenVectors, float, 1, 1);
						}
						else {
							CV_MAT_VAL(eigenVectorsMax, float, 0, 0) = CV_MAT_VAL(eigenVectors, float, 1, 0);
							CV_MAT_VAL(eigenVectorsMax, float, 0, 1) = CV_MAT_VAL(eigenVectors, float, 1, 1);

							CV_MAT_VAL(eigenVectorsMin, float, 0, 0) = CV_MAT_VAL(eigenVectors, float, 0, 0);
							CV_MAT_VAL(eigenVectorsMin, float, 0, 1) = CV_MAT_VAL(eigenVectors, float, 0, 1);
						}

						for(i = 0; i < numPoints; ++i)
						{
							int x = ( (float*)(pcaData->data.ptr) )[i];
							int y = ( (float*)(pcaData->data.ptr + pcaData->step) )[i];

							cvLine(image, cvPoint(x,y), cvPoint(x,y), CV_RGB(255,0,0), 1);
						}

//						cvProjectPCA(pcaData, pcaAvg, eigenVectors, pcaProj );
						cvProjectPCA(pcaData, pcaAvg, eigenVectorsMax, pcaProj );
						
//						cvBackProjectPCA( pcaProj, pcaAvg, eigenVectors, pcaResults);
						cvBackProjectPCA( pcaProj, pcaAvg, eigenVectorsMax, pcaResults);

						for(i = 0; i < numPoints; ++i)
						{
							int x = ( (float*)(pcaResults->data.ptr) )[i];
							int y = ( (float*)(pcaResults->data.ptr + pcaResults->step) )[i];

							cvLine(image, cvPoint(x,y), cvPoint(x,y), CV_RGB(0,0,255), 3, CV_AA);
						}	

						// Projecting image center
						cvProjectPCA(imgCenter, pcaAvg, eigenVectorsMax, centerProj );
						cvBackProjectPCA( centerProj, pcaAvg, eigenVectorsMax, centerResults);

						float x = ( (float*)(centerResults->data.ptr) )[0];
						float y = ( (float*)(centerResults->data.ptr + centerResults->step) )[0];

//						cvCircle(image, cvPoint(x,y), 3, CV_RGB(0,255,0), CV_FILLED);


#ifdef WRITE_TRACKING_RESULTS_TO_FILES
						// Converting projected center (pivot) to {r,theta} line
						float r, theta;
						CvPoint2D32f pivot = cvPoint2D32f(x,y);
						cvAddonFindPolarLineFromPivot(imgOrigin, pivot, r, theta);

						cvAddonDrawPolarLine(image, r, theta, CV_RGB(0,255,0), 2);

						cerr << r << "," << theta << endl;
						pcaFile << images.number() << "\t" << r << "\t" << theta << endl;


#endif

////						cvProjectPCA(pcaData, pcaAvg, eigenVectors, pcaProj );
//						cvProjectPCA(pcaData, pcaAvg, eigenVectorsMin, pcaProj );
//						
////						cvBackProjectPCA( pcaProj, pcaAvg, eigenVectors, pcaResults);
//						cvBackProjectPCA( pcaProj, pcaAvg, eigenVectorsMin, pcaResults);
//
//						for(i = 0; i < numPoints; ++i)
//						{
//							int x = ( (float*)(pcaResults->data.ptr) )[i];
//							int y = ( (float*)(pcaResults->data.ptr + pcaResults->step) )[i];
//
//							cvLine(image, cvPoint(x,y), cvPoint(x,y), CV_RGB(0,255,0), 1);
//						}	


						cvReleaseMat(&pcaData);
						cvReleaseMat(&pcaAvg);
						cvReleaseMat(&eigenVectors);
						cvReleaseMat(&eigenVectorsMax);
						cvReleaseMat(&eigenVectorsMin);
						cvReleaseMat(&eigenValues);
						cvReleaseMat(&pcaProj);
						cvReleaseMat(&pcaResults);
						cvReleaseMat(&imgCenter);
					}
				}

				cvCircle(image, cvPointFrom32f( findCentroid(bp) ), 3, CV_RGB(0,0,0), CV_FILLED);
			}
//			blobWithLessArea.FillBlob( image, CV_RGB( 0, 255, 0 ));



//            cvCalcBackProject( &hue, backproject, hist );
//            cvAnd( backproject, mask, backproject, 0 );
            
			cvCamShift( bp, track_window,
                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                        &track_comp, &track_box );
            track_window = track_comp.rect;
            
//            if( backproject_mode )
//                cvCvtColor( backproject, image, CV_GRAY2BGR );
            if( image->origin )
                track_box.angle = -track_box.angle;

			if(show_camshift) {
				// TODO: Bugs out if colour object disappears off the screen quickly 
				// as it tries to draw outside the window (very rare) 
				cvEllipseBox( image, track_box, CV_RGB(0,255,0), 3, CV_AA, 0 );
				cvCircle(image, cvPointFrom32f(track_box.center), 4, CV_RGB(0,255,0), CV_FILLED);
			}

//			// Center of Gravity
//			CvMoments moments;
//			cvMoments(bp, &moments, 0);
//			float dXCenter = moments.m10 / moments.m00;
//			float dYCenter = moments.m01 / moments.m00;

			

//			cvCircle(image, cvPointFrom32f( findCentroid(bp) ), 3, CV_RGB(0,0,0), CV_FILLED);
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
		
		cvShowImage( "Motion", motionMask);

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
			break;

//		case 's':
//			saveCvAddonHSVFilter(&hsvFilter, IMAGE_PATH, "hsv_filter.xml");
//			break;

		case 's':
			cvSaveImage("src.png", frame);
			cvSaveImage("pca.png", image);
			cvSaveImage("bp.png", bp);
			cvSaveImage("hist.png", histImg);
			cvSaveImage("motion.png", motionMask);
			break;



//		case 'l': 
//			CvAddonHSVFilter *newHSV = loadCvAddonHSVFilter("./", "hsv_filter.xml");
//			if(newHSV) {
//				newHSV->drawHist(histImg);
//				cvShowImage( "HSV Histogram", histImg );
//
//				cvWaitKey(0);
//			}
//			break;
        default:
            ;
        }

// Image Sequence input
#ifndef CV_CAPTURE
//		cvReleaseImage(&frame);
				
		if(c == 'r') images.reset();
		if(c == ' ') start_frame_step ^= 1;

		last_frame = this_frame;

		if(start_frame_step)
			images.next();

		this_frame = images.number();
#endif
	}

#ifdef CV_CAPTURE
    cvReleaseCapture( &capture );
#endif

#ifdef WRITE_TRACKING_RESULTS_TO_FILES
	pcaFile.close();
#endif


    cvDestroyWindow("Back Projection");
    return 0;
}

