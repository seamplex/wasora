//
SetFactory("OpenCASCADE");
a = 1;

Rectangle(1) = {-a/2, -a/2, 0, a, a};
Physical Surface(1) = {1};

//Mesh.ElementOrder = 2;
Mesh.RecombineAll = 1;
Mesh.RecombinationAlgorithm = 1 ;
//Mesh.SecondOrderIncomplete = 1;

Mesh.CharacteristicLengthMax = a/2;
