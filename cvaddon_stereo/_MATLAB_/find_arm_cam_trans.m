% Calibration routine to find tranformation
% between Robot manipulator base frame and
% stereo camera

% Requires:
% 1) Calibration data (.mat) of stereo camera
% 2) Image pair of calibration grid place over points know in base frame.
%    Images should be named left0.bmp and right0.bmp

% Details:
% 1) Calibration done between right camera and base frame
% 2) Transformation provided in two parts, as 3x3 R and 3x1 T matrices

% Instructions:
% 1) Click on the four corners of the calibration grid, in a counter-clockwise
% fashion, making sure both calibration grids have the same 4 corners
% clicked, starting at the top right corner
% 2) Results returned as R_calib and T_calib


% 3D corner triangulation
[pts_l, pts_r] = stereo_2D_to_3D('left0.bmp', 'right0.bmp', 'Calib_Results_stereo.mat');

% Normalizing points from mm ==> metres
points3D_Left = pts_l .* 1e-3;
points3D_Right = pts_r .* 1e-3;

% Grid size - number of grid corners (cells is size-1)
gridSizeX = 8;
gridSizeY = 11;

% % Location of center corner (x=.30, y=0, z=0 with base frame coordinates) 
% % of calibration grid. Corner count starting at the corner furthest from the stereo camera
% nx = 3;
% ny = 3;

%  Grid Cell Size
cellX = 20.0 * 1e-3; %19.811 * 1e-3;
cellY = 20.0 * 1e-3;

% Base frame coordinate at top-right corner (first corner clicked)
cenX = 0.3 + 2*cellX ;
cenY = 0 - 2*cellY;
cenZ = 0;

xRange = cenX - (gridSizeX-1)*cellX : cellX : cenX;
yRange = cenY : cellY : (gridSizeY-1)*cellY + cenY;
zRange = zeros(1, gridSizeX * gridSizeY);
xTmp = repmat(xRange, gridSizeY, 1);

points3D_base = [reshape(xTmp, 1, gridSizeX * gridSizeY); repmat(yRange, 1, gridSizeX); zRange];

% Finding transformation from Right Camera Coordinate Frame ==> Arm Frame
% (see arm_cam_calib.m for details)
[R_calib,T_calib] = arm_cam_calib(points3D_Right, points3D_base);

N = gridSizeX * gridSizeY;

points3D_Right_trans = zeros(size(points3D_Right));
for i = 1:N
    points3D_Right_trans(:,i) = R_calib*points3D_Right(:,i) + T_calib;
end

figure;
plot3(points3D_base(1,:), points3D_base(2,:), points3D_base(3,:), 'r.');
hold on
plot3(points3D_Right_trans(1,:), points3D_Right_trans(2,:), points3D_Right_trans(3,:), 'bx');

err_res = abs(points3D_Right_trans - points3D_base);

err = mean(err_res,2)

R_calib
T_calib

% H =
%   -0.13385003737032  -0.24973766509550                  0
%   -0.07575868691114   0.15777779692198                  0
%    0.10645129537023  -0.19967729308030                  0
% err =
%   1.0e-003 *
%    0.52096005954925
%    0.81871458508954
%    0.45409449175374
% R_calib =
%   -0.71408914854660  -0.40597451502134   0.57031691284862
%   -0.69960768708256   0.44296004493709  -0.56065629646363
%   -0.02501543729333  -0.79935667367110  -0.60033585279783
% T_calib =
%    0.00065777861358
%    0.38256549488501
%    0.41269443737071

% % Test symmetry line
% p1 = [60.8451; -199.405; 462.385] * 1e-3;
% p2 = [70.894; 306.26; 819.465] * 1e-3;
% inter = [65.6027; 39.9983; 631.442] * 1e-3;

% Old way of generating Coordinates (confusing and hacked >_>)
% % Base frame location at center corner
% cenX = 0.3;
% cenY = 0;
% cenZ = 0;
% 
% % xRange = (nx-1)*cellX + cenX : -cellX : (nx-gridSizeY)*cellX + cenX;
% % yRange = -((ny-1)*cellY + cenY) : cellY : -((ny-gridSizeX)*cellY + cenY);
% 
% xRange = (nx-gridSizeX)*cellX + cenX : cellX : (nx-1)*cellX + cenX;
% yRange = -((ny-1)*cellY + cenY) : cellY : -((ny-gridSizeY)*cellY + cenY);
% zRange = zeros(1, gridSizeX * gridSizeY);
% xTmp = repmat(xRange, gridSizeY, 1);
% 
% points3D_base = [reshape(xTmp, 1, gridSizeX * gridSizeY); repmat(yRange,
% 1, gridSizeX); zRange];
