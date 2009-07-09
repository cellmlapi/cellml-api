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
  $(top_srcdir)/sources/cellml/CellMLBootstrapXPCOM.cpp \
  $(top_builddir)/interfaces/p2xRDF_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pRDF_APISPEC.cpp

libcellml_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libXPCOMSupport.la
libcellml_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libcellml_xpcom_bridge_la_CXXFLAGS = \
  $(XPCOM_CXXFLAGS) \
  -DMODULE_CONTAINS_xpcom -DMODULE_CONTAINS_DOMAPISPEC \
  -DMODULE_CONTAINS_MathMLcontentAPISPEC -DMODULE_CONTAINS_CellMLAPISPEC \
  -DMODULE_CONTAINS_CellMLevents -DMODULE_CONTAINS_RDFAPISPEC

$(top_builddir)/interfaces/p2x%.cpp \
$(top_builddir)/interfaces/p2x%.hxx \
$(top_builddir)/interfaces/x2p%.cpp \
$(top_builddir)/interfaces/x2p%.hxx \
$(top_builddir)/interfaces/I%.h \
$(top_builddir)/interfaces/I%.xpt: \
$(top_srcdir)/interfaces/%.idl \
$(top_srcdir)/simple_interface_generators/omniidl_be/xpcom/p2xgen.py \
$(top_srcdir)/simple_interface_generators/omniidl_be/xpcom/x2pgen.py
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p$$SAVEDIR/$(top_srcdir)/simple_interface_generators/omniidl_be \
         -bxpcom $$SAVEDIR/$< && \
        $(CYGWIN_WRAPPER) $(MOZILLA_DIR)/bin/xpidl -m header \
          -I$(MOZILLA_DIR)/idl \
          $$SAVEDIR/$(top_builddir)/interfaces/I$(notdir $<) && \
        $(CYGWIN_WRAPPER) $(MOZILLA_DIR)/bin/xpidl -m typelib \
          -I$(MOZILLA_DIR)/idl \
          $$SAVEDIR/$(top_builddir)/interfaces/I$(notdir $<) && \
	cd $$SAVEDIR

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xDOM_events.cpp \
  $(top_builddir)/interfaces/p2xMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/p2xCellML_events.cpp \
  $(top_builddir)/interfaces/p2xRDF_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pDOM_events.cpp \
  $(top_builddir)/interfaces/x2pMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/x2pCellML_events.cpp \
  $(top_builddir)/interfaces/x2pRDF_APISPEC.cpp
