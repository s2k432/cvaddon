% Heavily modified version of ext_calib_stereo.m from the MATLAB
% calibration toolbox. Basically, the visualization code and be 
% cut out and modified to show my own 3D data obtained via 
% triangulation

%%%%%%%%%%%%%%%%%%%% SHOW EXTRINSIC RESULTS %%%%%%%%%%%%%%%%%%%%%%%%

% Checking to see left and right focal lengths are available
if ~exist('fc_right')|~exist('fc_left'),
   fprintf(1,'No stereo calibration data available.\n');
   return;
end;

% If 1, tries to draw points as a grid
draw_grid = 1;

% Color code for each image:
colors = 'brgkcm';

% Grid size CHANGED to 20mm
dX = 20;
dY = dX;

% Image Pixel Dimensions
nx = 640;
ny = 480;

normT = 2*dX;

IP_left = normT *[1 -alpha_c_left 0;0 1 0;0 0 1]*[1/fc_left(1) 0 0;0 1/fc_left(2) 0;0 0 1]*[1 0 -cc_left(1);0 1 -cc_left(2);0 0 1]*[0 nx-1 nx-1 0 0 ; 0 0 ny-1 ny-1 0;1 1 1 1 1];
BASE_left = normT *([0 1 0 0 0 0;0 0 0 1 0 0;0 0 0 0 0 1]);
IP_left  = reshape([IP_left ;BASE_left(:,1)*ones(1,5);IP_left ],3,15);

IP_right = normT *[1 -alpha_c_right 0;0 1 0;0 0 1]*[1/fc_right(1) 0 0;0 1/fc_right(2) 0;0 0 1]*[1 0 -cc_right(1);0 1 -cc_right(2);0 0 1]*[0 nx-1 nx-1 0 0 ; 0 0 ny-1 ny-1 0;1 1 1 1 1];
IP_right = reshape([IP_right;BASE_left(:,1)*ones(1,5);IP_right],3,15);

% Relative position of right camera wrt left camera: (om,T)
R = rodrigues(om);

% Change of reference:
BASE_right = R'*(BASE_left - repmat(T,[1 6]));
IP_right = R'*(IP_right - repmat(T,[1 15]));

if ishandle(4),
	figure(4);
   [a,b] = view;
else
   figure(4);
   a = 50;
   b = 20;
end;

 
figure(4);
plot3(BASE_left(1,:),BASE_left(3,:),-BASE_left(2,:),'b-','linewidth',2');
hold on;
plot3(IP_left(1,:),IP_left(3,:),-IP_left(2,:),'r-','linewidth',2);
text(BASE_left(1,2),BASE_left(3,2),-BASE_left(2,2),'X','HorizontalAlignment','center','FontWeight','bold');
text(BASE_left(1,6),BASE_left(3,6),-BASE_left(2,6),'Z','HorizontalAlignment','center','FontWeight','bold');
text(BASE_left(1,4),BASE_left(3,4),-BASE_left(2,4),'Y','HorizontalAlignment','center','FontWeight','bold');
text(BASE_left(1,1),BASE_left(3,1),-BASE_left(2,1),'Left Camera','HorizontalAlignment','center','FontWeight','bold');
plot3(BASE_right(1,:),BASE_right(3,:),-BASE_right(2,:),'b-','linewidth',2');
plot3(IP_right(1,:),IP_right(3,:),-IP_right(2,:),'r-','linewidth',2);
text(BASE_right(1,2),BASE_right(3,2),-BASE_right(2,2),'X','HorizontalAlignment','center','FontWeight','bold');
text(BASE_right(1,6),BASE_right(3,6),-BASE_right(2,6),'Z','HorizontalAlignment','center','FontWeight','bold');
text(BASE_right(1,4),BASE_right(3,4),-BASE_right(2,4),'Y','HorizontalAlignment','center','FontWeight','bold');
text(BASE_right(1,1),BASE_right(3,1),-BASE_right(2,1),'Right Camera','HorizontalAlignment','center','FontWeight','bold');

YYx = points3D_Left(1,:);
YYy = points3D_Left(2,:);
YYz = points3D_Left(3,:);   

figure(4);
plot3(YYx, YYz, -YYy,['.' colors(1)]);

figure(4);rotate3d on;
axis('equal');
title('Extrinsic parameters');
view(a,b);
grid on;
hold off;
axis vis3d;
axis tight;
set(4,'color',[1 1 1]);

set(4,'Name','3D','NumberTitle','off');

if exist('h_switch')==1,
    if ishandle(h_switch),
        delete(h_switch);
    end;
end;

if exist('h_switch2')==1,
    if ishandle(h_switch2),
        delete(h_switch2);
    end;
end;

% pause;
% 
% close(4);