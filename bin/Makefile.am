AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I${top_srcdir}/src -I${top_builddir}/src

bin_PROGRAMS = xmimsim-db xmimsim-harvester xmimsim-test
xmimsim_db_SOURCES = xmimsim-db.F90
xmimsim_db_FCFLAGS = @xraylib_CFLAGS@ @H5FC_INCLUDE@ @OPENMP_FFLAGS@ 
xmimsim_db_LDADD = @H5FC_STATIC@
xmimsim_db_LDFLAGS = @xraylib_LIBS@ -lxrl @H5FC_LINKER@

xmimsim_harvester_SOURCES = xmimsim-harvester.c
xmimsim_harvester_LDADD = ../src/libxmimsim.la


xmimsim_test_SOURCES = xmimsim-test.c
xmimsim_test_LDADD = ../src/libxmimsim.la