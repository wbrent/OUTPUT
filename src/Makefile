# Makefile for timbreIDLib

# specify a location for Pd if desired
# PDDIR = /home/yourname/somedirectory/pd-0.50-0

lib.name = outputAlgo~

# specify the location and name of the FFTW library
# ldlibs = -L/usr/local/lib -lfftw3f
 
# specify the location of FFTW header file
# cflags = -Iinclude -I/usr/local/include

lib.setup.sources = $(lib.name).c

class.sources = outputAlgo~.c

# common.sources = src/tIDLib.c

datafiles = outputAlgo~-help.pd

# build a multi-object library
make-lib-executable=no

# provide the path to pd-lib-builder
PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
