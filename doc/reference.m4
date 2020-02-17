changequote([!,!])dnl
% Wasora esyscmd([!git describe | sed 's/-/./' | tr -d '\n'!]) reference sheet

include(reference-toc.md)

# Keywords

esyscmd([!./reference.sh parser kw!])

--------------

# Mesh-related keywords

esyscmd([!./reference.sh mesh/parser kw!])

--------------

# Variables

esyscmd([!./reference.sh init va!])

--------------

# Mesh-related variables

esyscmd([!./reference.sh mesh/init va!])

--------------

# Functions

esyscmd([!./reference.sh builtinfunctions fn!])

--------------

# Functionals

esyscmd([!./reference.sh builtinfunctionals fu!])

--------------

# Vector functions

esyscmd([!./reference.sh builtinvectorfunctions fv!])

--------------
