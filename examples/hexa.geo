SetFactory("OpenCASCADE");

a = 1;
n = 10;

Point(1) = {0.0,0.0,0.0,1};

line[] = Extrude{a, 0, 0} { Point{1}; Layers{n}; Recombine; };
face[] = Extrude{0, a, 0} { Line{line[0]}; Layers{n}; Recombine; };
vol[]  = Extrude{0, 0, a} { Surface{face[1]}; Layers{n}; Recombine; };
Physical Volume(1) = {vol[1]};
Mesh.ElementOrder = 2;
//Comment the next line to get hexahedron 27 elements
Mesh.SecondOrderIncomplete = 1;
