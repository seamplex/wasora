//
SetFactory("OpenCASCADE");
a = 1;

Rectangle(1) = {-a/2, -a/2, 0, a, a};
Physical Surface("bulk", 1) = {1};

Mesh.CharacteristicLengthMax = a/3;
Mesh.CharacteristicLengthMin = a/3;

Mesh.Algorithm = 6;
Mesh.RecombineAll = 1;
Mesh.ElementOrder = 2;

