#include "cvaddon_stereo_tri.h"
#include "cvaddon_stereo_calib.h"
#include <cv.h>

#include "../cvaddon_util/cvaddon_print.h"

CvAddonStereoTriangulator::CvAddonStereoTriangulator()
{
	R		= cvCreateMat(3, 3, CV_32FC1);
	R_inv 	= cvCreateMat(3, 3, CV_32FC1);
	T		= cvCreateMat(3, 1, CV_32FC1);
	u		= cvCreateMat(3, 1, CV_32FC1);
	xt		= cvCreateMat(3, 1, CV_32FC1);
	xtt		= cvCreateMat(3, 1, CV_32FC1);
	X1		= cvCreateMat(3, 1, CV_32FC1);	
	X2		= cvCreateMat(3, 1, CV_32FC1);
	XL		= cvCreateMat(3, 1, CV_32FC1);	
	XR		= cvCreateMat(3, 1, CV_32FC1);
	Zt_mat	= cvCreateMat(3, 1, CV_32FC1);	
	Ztt_mat	= cvCreateMat(3, 1, CV_32FC1);
}


CvAddonStereoTriangulator::~CvAddonStereoTriangulator()
{
	cvReleaseMat(&R);
	cvReleaseMat(&R_inv);
	cvReleaseMat(&T);
	cvReleaseMat(&u);
	cvReleaseMat(&xt);
	cvReleaseMat(&xtt);
	cvReleaseMat(&X1);
	cvReleaseMat(&X2);
	cvReleaseMat(&XL);
	cvReleaseMat(&XR);
	cvReleaseMat(&Zt_mat);
	cvReleaseMat(&Ztt_mat);
}


// For some reason, the MATLAB version has R and R_inv the other way around (xR <==> xL probably)
void CvAddonStereoTriangulator::tri(const CvPoint2D32f _xL, const CvPoint2D32f _xR
	, const CvAddonStereoParameters &stereoParams
	, CvPoint3D32f& _XL, CvPoint3D32f& _XR)
{	
	CvPoint2D32f xL, xR;

	// Converting R and T (extrinsics) into CvMat
	int n;
	for(n = 0; n < 3; ++n) 
	{
		CV_MAT_ELEM(*T, float, n, 0) = (float)stereoParams.T[n];
	}
	for(n = 0; n < 9; ++n)
	{
		CV_MAT_ELEM(*R_inv, float, n/3, n%3) = (float)stereoParams.R[n];
	}
	cvInvert(R_inv, R); 


	//%--- Normalize the image projection according to the intrinsic parameters of the left and right cameras
	//xt = normalize(xL,fc_left,cc_left,kc_left,alpha_c_left);
	//xtt = normalize(xR,fc_right,cc_right,kc_right,alpha_c_right);
	normalizePixel(_xL, stereoParams.left, xL);
	normalizePixel(_xR, stereoParams.right, xR);

	//%--- Extend the normalized projections in homogeneous coordinates
	//xt = [xt;ones(1,size(xt,2))];
	//xtt = [xtt;ones(1,size(xtt,2))];
	CV_MAT_ELEM(*xt, float, 0, 0) = xL.x;
	CV_MAT_ELEM(*xt, float, 1, 0) = xL.y;
	CV_MAT_ELEM(*xt, float, 2, 0) = 1.0f;

	CV_MAT_ELEM(*xtt, float, 0, 0) = xR.x;
	CV_MAT_ELEM(*xtt, float, 1, 0) = xR.y;
	CV_MAT_ELEM(*xtt, float, 2, 0) = 1.0f;


	//u = R * xt;
	cvMatMul(R, xt, u);

	//	n_xt2 = dot(xt,xt);
	//	n_xtt2 = dot(xtt,xtt);
	double n_xt2	= cvDotProduct(xt, xt);
	double n_xtt2	= cvDotProduct(xtt, xtt);

	//	DD = n_xt2 .* n_xtt2 - dot(u,xtt).^2;
	double dot_u_xtt = cvDotProduct(u, xtt);
	double DD = n_xt2*n_xtt2 - dot_u_xtt*dot_u_xtt;

	//dot_uT = dot(u,T_vect);
	//dot_xttT = dot(xtt,T_vect);
	//dot_xttu = dot(u,xtt);
	double dot_uT		= cvDotProduct(u, T);
	double dot_xttT		= cvDotProduct(xtt, T);
	double dot_xttu		= cvDotProduct(u, xtt);


	//NN1 = dot_xttu.*dot_xttT - n_xtt2 .* dot_uT;
	//NN2 = n_xt2.*dot_xttT - dot_uT.*dot_xttu;
	double NN1 = dot_xttu*dot_xttT - n_xtt2*dot_uT;
	double NN2 = n_xt2*dot_xttT - dot_uT*dot_xttu;

	// DEBUG
	cerr << "NN1: " << NN1 << endl;
	cerr << "NN2: " << NN2 << endl;

	//Zt = NN1./DD;
	//Ztt = NN2./DD;
	double Zt = NN1 / DD;
	double Ztt = NN2 / DD;

	cerr << "Zt: " << Zt << endl;
	cerr << "Ztt: " << Ztt << endl;

	//X1 = xt .* repmat(Zt,[3 1]);
	//X2 = R'* (xtt.*repmat(Ztt,[3,1])  - T_vect);
	cvSet( Zt_mat, cvScalarAll(Zt) );
	cvSet( Ztt_mat, cvScalarAll(Ztt) );
	cvMul(xt, Zt_mat, X1);

	cvMul(xtt, Ztt_mat, xt);	// xt used as tmp mat
	cvSub(xt, T, xtt);
	cvGEMM(R, xtt, 1, 0, 1, X2, CV_GEMM_A_T );

	//%--- Left coordinates:
	//XL = 1/2 * (X1 + X2);
	 cvAddWeighted( X1, 0.5, X2, 0.5, 0, XL);
	_XL.x = CV_MAT_ELEM(*XL, float, 0, 0);
	_XL.y = CV_MAT_ELEM(*XL, float, 1, 0);
	_XL.z = CV_MAT_ELEM(*XL, float, 2, 0);

	//%--- Right coordinates:
	//XR = R*XL + T_vect;	
	cvMatMul(R, XL, XR);
	_XR.x = CV_MAT_ELEM(*T, float, 0, 0) + CV_MAT_ELEM(*XR, float, 0, 0);
	_XR.y = CV_MAT_ELEM(*T, float, 1, 0) + CV_MAT_ELEM(*XR, float, 1, 0);
	_XR.z = CV_MAT_ELEM(*T, float, 2, 0) + CV_MAT_ELEM(*XR, float, 2, 0);
}


