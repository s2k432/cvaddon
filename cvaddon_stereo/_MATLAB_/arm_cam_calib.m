% Solving Camera-Manipulator Calibration
% using 3D point pairs in the camera's coordinate
% system (rel. left camera according to triangulation)
% and Arm's base coordinate system, with z=0 at table plane, 
% x axis going outwards and y point directly up shaft to joint1

% Method from
% "Least-Squares Fitting of Two 3-D Point Sets" by Arun et al

% p1 and p2 are two corresponding 3D point sets
% R and T are the rotation and translation matrices
function [R_est,T_est] = arm_cam_calib(p1, p2)

% Step 1
% finding centroids
p1_c = mean(p1, 2);
p2_c = mean(p2, 2);

N = size(p1, 2);

% subtracting mean
q1 = p1 - repmat(p1_c, 1, N);
q2 = p2 - repmat(p2_c, 1, N);

% Step 2

H = zeros(3,3);
for i = 1:N
    H = H + q1(:,i) * q2(:,i)';
end

H

% Step 3
[U,S,V] = svd(H);

% ??? Need to transpose V?

% Step 4
X = V*U';

% Step 5
detX = det(X);
if(detX > 0) R_est = X;
else 
    VV = V;
    
    % Special case where (assuming relatively low noise), we have a
    % reflection (see section IV and V)
    sing_val = diag(H);
    
    if sing_val(1) == 0
        VV = [-VV(:,1) VV(:,2) VV(:,3)];
    elseif sing_val(2) == 0
        VV = [VV(:,1) -VV(:,2) VV(:,3)];            
    elseif sing_val(3) == 0
        VV = [VV(:,1) VV(:,2) -VV(:,3)];        
    else
        disp('Degenerate Case with no solution. GG NO RE');
    end
    
    R_est = VV * U';
end




% Step 6 - Finding T
T_est = p2_c - R_est*p1_c;