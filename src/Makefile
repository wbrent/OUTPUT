# Makefile for [algo~]

# when compiling for Linux, disable the pd-lib-builder default use of -ffast-math in optimization.flags, which doesn't play nice with exprDouble.h. the other optimization flags can remain, i.e.:
# make optimization.flags='-O3 -funroll-loops -fomit-frame-pointer'

# specify a location for Pd if desired
# PDDIR = /home/yourname/somedirectory/pd-0.50-2

lib.name = algo~

$(lib.name).class.sources = algo~.c

datafiles = $(lib.name)-help.pd

# provide the path to pd-lib-builder
PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
