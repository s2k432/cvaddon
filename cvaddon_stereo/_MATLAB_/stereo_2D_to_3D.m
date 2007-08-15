% Triangulates points between two images from a 
% calibrated stereo camera rig using modified functions 
% provided by the MATLAB Camera Calibration toolbox at:
% http://www.vision.caltech.edu/bouguetj/calib_doc/index.html

% This is working ^__^

function [points3D_Left points3D_Right] ...
    = stereo_2D_to_3D(left_image_name, right_image_name, calib_file_name)

% clear all
% 
% % Left and Right images
% left_image_name = 'left0.bmp';
% right_image_name = 'right0.bmp';
% 
% calib_file_name = 'Calib_Results_stereo.mat';
% calib_file_name = 'test.mat';

if exist(calib_file_name)~=2,
    fprintf(1, ['\nStereo calibration file ' calib_file_name 'not found!\n']);
    return;
end;

% Only loading what we need for triangulation
fprintf(1, ['Loading stereo calibration results from ' calib_file_name '...\n'] );
% eval( ['load ' calib_file_name ' om T fc_left cc_left kc_left alpha_c_left fc_right cc_right kc_right alpha_c_right']);

eval( ['load ' calib_file_name] );

% % NEW - trying left and right calibration values instead
% load 'Calib_Results_left.mat'
% fc_left = fc;
% cc_left = cc;
% kc_left = kc;
% alpha_c_left = alpha_c;
% 
% load 'Calib_Results_right.mat'
% fc_right = fc;
% cc_right = cc;
% kc_right = kc;
% alpha_c_right = alpha_c;

% Finding corners
xL = auto_corner(left_image_name);
xR = auto_corner(right_image_name);

% Triangulation
[points3D_Left, points3D_Right] = stereo_triangulation( ...
    xL,xR,om,T,fc_left,cc_left,kc_left,alpha_c_left,fc_right,cc_right,kc_right,alpha_c_right);

% Fitting plane to grid points
% fitplane function returns Hessian form of the plane
calib_plane = fitplane(points3D_Right);

a = calib_plane(1);
b = calib_plane(2);
c = calib_plane(3);
d = calib_plane(4);

sqrt_abc = sqrt(a^2 + b^2 + c^2);

calib_plane_normal = [a/sqrt_abc, b/sqrt_abc, c/sqrt_abc];
calib_plane_p = d/sqrt_abc;

% Visualize Results
draw_camera_and_points

end