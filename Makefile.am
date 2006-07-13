# Pre-initialise so we can append...
include_HEADERS :=
lib_LTLIBRARIES :=
noinst_LTLIBRARIES :=
noinst_PROGRAMS :=
bin_PROGRAMS :=
BUILT_SOURCES :=

# Parts needed to build the interface generates...
include $(top_srcdir)/simple_interface_generators/build.mk

# Parts needed to build and test the core...
include $(top_srcdir)/interfaces/build.mk
include $(top_srcdir)/sources/build.mk
include $(top_srcdir)/tests/build.mk

# These parts relate to CORBA...
if ENABLE_CORBA
include $(top_srcdir)/interfaces/corba_build.mk
endif

# These parts relate to the CellML Context...
if ENABLE_CONTEXT
include $(top_srcdir)/sources/cellml_context/build.mk
endif

# CellML Context and also CORBA...
if ENABLE_CONTEXT_AND_CORBA
include $(top_srcdir)/sources/cellml_context/corba_build.mk
endif

if ENABLE_CELLML_CORBA_SERVER
include $(top_srcdir)/libltdl/build.mk
include $(top_srcdir)/sources/cellml_corba_server/build.mk
endif

if ENABLE_CCGS
include $(top_srcdir)/CCGS/build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CCGS
include $(top_srcdir)/CCGS/corba_build.mk
endif

if ENABLE_CIS
include $(top_srcdir)/CIS/build.mk
endif
if ENABLE_CONTEXT_AND_CORBA_AND_CIS
include $(top_srcdir)/CIS/corba_build.mk
endif

CLEANFILES=sources/dom/*.lo sources/dom_direct/*.lo sources/mathml/*.lo sources/cellml/*.lo source/dom_direct/*.lo sources/Utilities.lo simple_interface_generators/glue/corba_support/*.lo
