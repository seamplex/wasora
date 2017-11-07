//
a = 1 ;

lc = a/1;        // element characteristic length

Point(1) = { 0,  0, 0, lc};
Point(2) = {+a,  0, 0, lc};
Point(3) = {+a, +a, 0, lc};
Point(4) = { 0, +a, 0, lc};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};


Line Loop(29) = {-1, -2, -3, -4};
Plane Surface(30) = {29};
Physical Surface(1) = {30};

Mesh.RecombineAll = 1;
Mesh.RecombinationAlgorithm = 1 ;
Mesh.Recombine3DLevel = 0;
Mesh.ElementOrder = 2;
