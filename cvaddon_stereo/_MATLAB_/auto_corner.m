% Extracts (x,y) pixel locations of image corners using MATLAB Camera calibration toolbox
% functions from: 
% http://www.vision.caltech.edu/bouguetj/calib_doc/index.html

function corner_points = auto_corner(img_name)%, dX, dY, wintx, winty)

% Size of calibration board cell in mm
dX = 20.0;%19.811;
dY = 20.0;

% Corner search window in pixels = 2*wintx + 1
wintx = 5;
winty = 5;

% Reading input image
I = double(imread(img_name));

if size(I,3) > 1,
   I = 0.299 * I(:,:,1) + 0.5870 * I(:,:,2) + 0.114 * I(:,:,3);
end
 
% Corner Extraction
click_corner

% Results now in grid_pts
corner_points = grid_pts;

end