% Draw Sym Line
% Mainly for debugging atm

% Visualizing Symmetry line
sym_z = 1.0 * 1e3;    % 1 metre distance, just incase...
left_sym = [450 0; 382 479];
right_sym = [426 0; 434 479];

left_sym_norm = normalize(left_sym',fc_left,cc_left,kc_left,alpha_c_left);
right_sym_norm = normalize(right_sym',fc_right,cc_right,kc_right,alpha_c_right);

left_sym_3D = [ left_sym_norm' [1;1] ] * sym_z;
right_sym_3D = [ right_sym_norm' [1;1] ] * sym_z;

% left_sym_x = left_sym - repmat(cc_left', 2, 1);
% left_sym_3D = [left_sym_x(1,1)/fc_left(1)*sym_z, left_sym_x(1,2)/fc_left(2)*sym_z, sym_z; ...
%     left_sym_x(2,1)/fc_left(1)*sym_z, left_sym_x(2,2)/fc_left(2)*sym_z, sym_z];
% 
% right_sym_x = right_sym - repmat(cc_right', 2, 1);
% right_sym_3D = [right_sym_x(1,1)/fc_right(1)*sym_z, right_sym_x(1,2)/fc_right(2)*sym_z, sym_z; ...
%     right_sym_x(2,1)/fc_right(1)*sym_z, right_sym_x(2,2)/fc_right(2)*sym_z, sym_z];


hold on;



left_sym_3D_drawn = [0 0 0; left_sym_3D; 0 0 0];

% XR = R * XL + T
% ==> XL = R' * (XR - T) // as inv(R) = R'
right_sym_3D_drawn = [(R'*-T)'; (R'* (right_sym_3D(1,:)' - T))'; (R'* (right_sym_3D(2,:)' - T))'; (R'*-T)'];

% Again, flipped axis due to draw_camera_and_points way of drawing
% plot3(left_sym_3D_drawn(:,1), left_sym_3D_drawn(:,3), -left_sym_3D_drawn(:,2), 'o-');
% plot3(right_sym_3D_drawn(:,1), right_sym_3D_drawn(:,3), -right_sym_3D_drawn(:,2), 'o-');


% Result from C code that finds intesection of triangles

% Sym Line from manually found end points
% sym_line_3D = [94.7634, -178.328, 455.35; 68.3981, 296.568, 720.543];
% plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'r-','linewidth',2');

% sym_line_3D_rel_right = [48.9968 -271.701 369.273; 151.525 628.307 958.261];

sym_line_3D_rel_right = [50.4165, -279.574, 379.972; 139.64, 579.023, 883.096];
sym_line_3D = [ (R' * (sym_line_3D_rel_right(1,:)' - T))';  (R' * (sym_line_3D_rel_right(2,:)' - T))'];


plane_left = [left_sym_3D_drawn(1, :); sym_line_3D; left_sym_3D_drawn(1, :)];
plane_right = [right_sym_3D_drawn(1, :); sym_line_3D; right_sym_3D_drawn(1, :)];

% Planes
plot3(plane_left(:,1), plane_left(:,3), -plane_left(:,2), 'o-');
plot3(plane_right(:,1), plane_right(:,3), -plane_right(:,2), 'o-');

% Sym Line
plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'r-','linewidth',2');

% sym_line_3D_rel_right = [-172.039, -177.107, 299.703; -120.68, 406.721, 613.75];
% sym_line_3D = [ (R' * (sym_line_3D_rel_right(1,:)' - T))';  (R' * (sym_line_3D_rel_right(2,:)' - T))'];
% plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'b-','linewidth',2');


% sym_line_3D_rel_right = [-81.7797, -90.171, 147.908; -62.5253, 198.751, 292.886];
% sym_line_3D = [ (R' * (sym_line_3D_rel_right(1,:)' - T))';  (R' * (sym_line_3D_rel_right(2,:)' - T))'];
% plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'r-','linewidth',2');
% 
% sym_line_3D_rel_right = [-148.673, -163.928, 268.891; -116.425, 341.822, 526.385];
% sym_line_3D = [ (R' * (sym_line_3D_rel_right(1,:)' - T))';  (R' * (sym_line_3D_rel_right(2,:)' - T))'];
% plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'r-','linewidth',2');
% 
% sym_line_3D_rel_right = [-174.068, -191.929, 314.821; -204.054, 571.413, 903.978];
% sym_line_3D = [ (R' * (sym_line_3D_rel_right(1,:)' - T))';  (R' * (sym_line_3D_rel_right(2,:)' - T))'];
% plot3(sym_line_3D(:,1),sym_line_3D(:,3),-sym_line_3D(:,2),'r-','linewidth',2');