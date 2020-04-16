# Keywords

esyscmd([. ./reference.sh parser kw  | sed 's/^#/##/' .])

# Mesh keywords

esyscmd([. ./reference.sh parser kw  | sed 's/^#/##/' .])


# Variables

esyscmd([. ./reference.sh init va | sed 's/^#/##/' .])


# Mesh-related variables

esyscmd([. ./reference.sh mesh/init va | sed 's/^#/##/' .])

--------------

# Functions

esyscmd([. ./reference.sh builtinfunctions fn | sed 's/^#/##/' .])

--------------

# Functionals

esyscmd([. ./reference.sh builtinfunctionals fu | sed 's/^#/##/' .])

--------------

# Vector functions

esyscmd([. ./reference.sh builtinvectorfunctions fv | sed 's/^#/##/' .])

--------------

dnl ## Fino variables

dnl el segundo sed es para reemplazar \text{} por \r{} que es algo de texinfo
dnl esyscmd([. ../wasora/doc/reference.sh init va  | sed 's/^#/##/' | sed 's/\\text/\\r/' .])

