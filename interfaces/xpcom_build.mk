# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcellml_xpcom_bridge.la

libcellml_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xDOM_events.cpp \
  $(top_builddir)/interfaces/p2xMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_events.cpp \
  $(top_builddir)/interfaces/x2pDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pDOM_events.cpp \
  $(top_builddir)/interfaces/x2pMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_events.cpp \
  $(top_srcdir)/sources/cellml/CellMLBootstrapXPCOM.cpp

libcellml_xpcom_bridge_la_LIBADD = $(STLLINK)
libcellml_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libcellml_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_xpcom -DMODULE_CONTAINS_DOMAPISPEC \
  -DMODULE_CONTAINS_MathMLcontentAPISPEC -DMODULE_CONTAINS_CellMLAPISPEC \
  -DMODULE_CONTAINS_CellMLevents

$(top_builddir)/interfaces/p2x%.cpp \
$(top_builddir)/interfaces/p2x%.hxx \
$(top_builddir)/interfaces/x2p%.cpp \
$(top_builddir)/interfaces/x2p%.hxx \
$(top_builddir)/interfaces/I%.h: \
$(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p$$SAVEDIR/simple_interface_generators/omniidl_be \
         -bxpcom $$SAVEDIR/$< && \
        $(MOZILLA_DIR)/bin/xpidl -m header \
          -I$(MOZILLA_DIR)/idl \
          $$SAVEDIR/$(top_builddir)/interfaces/I$(notdir $<) && \
	cd $$SAVEDIR

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xDOM_events.cpp \
  $(top_builddir)/interfaces/p2xMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_events.cpp \
  $(top_builddir)/interfaces/x2pDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pDOM_events.cpp \
  $(top_builddir)/interfaces/x2pMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_events.cpp
