% Calculates errors for data sets

calib_file_name = 'Calib_Results_stereo.mat';

if exist(calib_file_name)~=2,
    fprintf(1, ['\nStereo calibration file ' calib_file_name 'not found!\n']);
    return;
end;

fprintf(1, ['Loading stereo calibration results from ' calib_file_name '...\n'] );
eval( ['load ' calib_file_name] );

out_fid = fopen('stereo_sym_min_dist_error.txt', 'w');

% Only loading what we need for triangulation
fprintf(1, ['Loading stereo calibration results from ' calib_file_name '...\n'] );



inter_fid = fopen('TriIntersects.txt', 'r');
TTT = textscan(inter_fid, '%s %f %f %f');

names = TTT{1};
inter = [TTT{2} TTT{3} TTT{4}];

last_base_name = ' ';

for i = 1:size(names, 1)
    str = names{i};
    
    clear base_name;
    base_name = str(1:end-12)
    
    if strcmp(base_name, last_base_name) == 0
         % Points from left and right image
        left_image_name = [base_name 'calib_left_00.bmp'];
        right_image_name = [base_name 'calib_right_00.bmp'];

        % Finding corners
        xL = auto_corner(left_image_name);
        xR = auto_corner(right_image_name);       
        
        % Triangulation
        [points3D_Left, points3D_Right] = stereo_triangulation( ...
            xL,xR,om,T,fc_left,cc_left,kc_left,alpha_c_left,fc_right,cc_right,kc_right,alpha_c_right);      
        
        % corners of grid
        x = [points3D_Right(:,1) points3D_Right(:,6) points3D_Right(:,end-5) points3D_Right(:,end)]
        
        
        hold off;
        plot3(points3D_Right(1,:), points3D_Right(2,:), points3D_Right(3,:), 'x');
        hold on;
        
        clear last_base_name;
        last_base_name = base_name;
    end;
    
    plot3(inter(i, 1), inter(i,2), inter(i,3), 'rx');

    min_dist = 1000000;
    min_idx = -1;
    for k = 1:4
        dist = abs(norm(x(:,k) - inter(i,:)'));
        if(dist < min_dist)
            min_dist = dist; 
            min_idx = k;
        end
    end

    fprintf(out_fid, '%s %d %d\n', names{i}, min_dist, min_idx);
end;
    
fclose(inter_fid);


