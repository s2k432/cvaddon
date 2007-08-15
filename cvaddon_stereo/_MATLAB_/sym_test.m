% Testing symmetry stuff
xx = [1 2 5 8 9]'

n = 1;
m = 1;
for i=1:size(xx, 1)-1
    a = xx(i);
    for j=i+1:size(xx,1)
        b = xx(j);
        
        dist = b-a;
        if abs(dist) > 1
            mid(n) = (b+a)/2;
            n = n+1;
        end
    end
end

mid = sort(mid);
mid