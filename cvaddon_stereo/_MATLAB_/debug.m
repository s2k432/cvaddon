% For debugging problem with grid orientation (correspondance between arm
% base frame points and stereo triangulation points)

% figure;


for i = 1:size(points3D_base, 2)
%    plot3(points3D_Right_trans(1,i), points3D_Right_trans(2,i), points3D_Right_trans(3,i), 'ro');
%    plot3(points3D_base(1,i), points3D_base(2,i), points3D_base(3,i), 'ro');
%     plot3(points3D_Right(1,i), points3D_Right(2,i), points3D_Right(3,i), 'ro');

%     plot(xR(1,i), xR(2, i), 'ro');
    plot(xL(1,i), xL(2, i), 'ro');
    hold on;
    pause
end