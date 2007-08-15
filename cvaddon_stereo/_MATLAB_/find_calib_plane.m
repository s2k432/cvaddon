% Extract corners of checkerboard calibration patter
% and using stereo calibration data, performs
% least squares estimation of plane parameters
% relative to right camera

% clear all
% 
calib_file_name = 'Calib_Results_stereo.mat';

if exist(calib_file_name)~=2,
    fprintf(1, ['\nStereo calibration file ' calib_file_name 'not found!\n']);
    return;
end;

fprintf(1, ['Loading stereo calibration results from ' calib_file_name '...\n'] );
eval( ['load ' calib_file_name] );

out_fid = fopen('calib_planes.txt', 'w');

% Only loading what we need for triangulation
fprintf(1, ['Loading stereo calibration results from ' calib_file_name '...\n'] );

left_calib_names = dir('*_calib_left_*.bmp');
right_calib_names = dir('*_calib_right_*.bmp');

for i = 1:size(left_calib_names, 1)
       i
    
    % Points from left and right image
    left_image_name = left_calib_names(i).name;
    right_image_name = right_calib_names(i).name;

    % Finding corners
    xL = auto_corner(left_image_name);
    xR = auto_corner(right_image_name);

    % Triangulation
    [points3D_Left, points3D_Right] = stereo_triangulation( ...
        xL,xR,om,T,fc_left,cc_left,kc_left,alpha_c_left,fc_right,cc_right,kc_right,alpha_c_right)

    % Fitting plane to grid points
    % fitplane function returns Hessian form of the plane
    calib_plane = fitplane(points3D_Right);

    fprintf(out_fid, '%s %f %f %f %f\n', right_image_name, calib_plane(1) ...
        , calib_plane(2), calib_plane(3), calib_plane(4) );
end;
    
fclose(out_fid);
% sqrt_abc = sqrt(a^2 + b^2 + c^2);
% 
% calib_plane_normal = [a/sqrt_abc, b/sqrt_abc, c/sqrt_abc];
% calib_plane_p = d/sqrt_abc;