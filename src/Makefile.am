AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS)
bin_PROGRAMS = wasora
wasora_LDADD = $(WASORALIBS) $(all_libraries) 

# para exportar simbolos para los shared objects
# wasora_LDFLAGS = -rdynamic

wasora_SOURCES = \
./history.c \
./io.c \
./interface.c \
./print.c \
./realtime.c \
./builtindecl.h \
./algebra.c \
./vector.c \
./builtinfunctions.c \
./randomline.c \
./builtinfunctionals.c \
./flow.c \
./error.c \
./builtin.h \
./shmem.c \
./cleanup.c \
./parseaux.c \
./wasora.h \
./shell.c \
./fit.c \
./assignment.c \
./version.c \
./file.c \
./parser.c \
./init.c \
./matrix.c \
./multiminf.c \
./interface.h \
./function.c \
./thirdparty/utlist.h \
./thirdparty/kdtree.h \
./thirdparty/kdtree.c \
./thirdparty/uthash.h \
./builtinvectorfunctions.c \
./multiminfdf.c \
./alias.c \
./m4.c \
./getptr.c \
./call.c \
./debug.c \
./parametric.c \
./dae.c \
./version-vcs.h \
./multiroot.c \
./minimize.c \
./mesh/element.c \
./mesh/findmax.c \
./mesh/fem.c \
./mesh/fillvector.c \
./mesh/neighbors.c \
./mesh/point.c \
./mesh/integrate.c \
./mesh/prism6.c \
./mesh/prism15.c \
./mesh/vtk.c \
./mesh/parser.c \
./mesh/init.c \
./mesh/geom.c \
./mesh/cell.c \
./mesh/line2.c \
./mesh/line3.c \
./mesh/interpolate.c \
./mesh/tet4.c \
./mesh/tet10.c \
./mesh/hexa8.c \
./mesh/hexa27.c \
./mesh/hexa20.c \
./mesh/struct.c \
./mesh/post.c \
./mesh/gmsh.c \
./mesh/calculix.c \
./mesh/quad4.c \
./mesh/quad8.c \
./mesh/quad9.c \
./mesh/mesh.c \
./mesh/gauss.c \
./mesh/define.c \
./mesh/triang3.c \
./mesh/triang6.c \
./mesh/quality.c \
./plugin.c \
./define.c \
./wasora.c

# version.$(OBJEXT): version.h
# version.h: Makefile
# 	./version.sh
