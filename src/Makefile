# Makefile for [outputAlgo~]

# specify a location for Pd if desired
# PDDIR = /home/yourname/somedirectory/pd-0.50-0

lib.name = outputAlgo~

lib.setup.sources = $(lib.name).c

class.sources = outputAlgo~.c

common.sources = outputAlgorithms.c

datafiles = outputAlgo~-help.pd

# build a multi-object library
make-lib-executable=no

# provide the path to pd-lib-builder
PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
