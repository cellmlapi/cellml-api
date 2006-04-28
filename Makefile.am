# Pre-initialise so we can append...
include_HEADERS :=
lib_LTLIBRARIES :=
noinst_LTLIBRARIES :=
BUILT_SOURCES :=

# Parts needed to build the interface generates...
include $(top_srcdir)/simple_interface_generators/build.mk

# Parts needed to build and test the core...
include $(top_srcdir)/interfaces/build.mk
include $(top_srcdir)/sources/build.mk
include $(top_srcdir)/tests/build.mk

# These parts relate to CORBA...
include $(top_srcdir)/interfaces/corba_build.mk
include $(top_srcdir)/sources/cellml_corba_server/build.mk

# These parts relate to the CellML Context...
include $(top_srcdir)/sources/cellml_context/build.mk

# CellML Context and also CORBA...
include $(top_srcdir)/sources/cellml_context/corba_build.mk
