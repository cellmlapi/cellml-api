# Interfaces module...
include_HEADERS += \
  $(top_builddir)/interfaces/IfaceDOM_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceDOM_events.hxx \
  $(top_builddir)/interfaces/Ifacexpcom.hxx \
  $(top_builddir)/interfaces/IfaceMathML_content_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceCellML_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceCellML_events.hxx

$(top_builddir)/interfaces/Iface%.hxx \
$(top_builddir)/interfaces/CCI%.hxx \
$(top_builddir)/interfaces/SCI%.hxx \
$(top_builddir)/interfaces/CCI%.cxx \
$(top_builddir)/interfaces/SCI%.cxx: $(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p $$SAVEDIR/$(top_srcdir)/simple_interface_generators/omniidl_be -bsimple_cpp $$SAVEDIR/$< && \
	cd $$SAVEDIR

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceDOM_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceDOM_events.hxx \
  $(top_builddir)/interfaces/Ifacexpcom.hxx \
  $(top_builddir)/interfaces/IfaceMathML_content_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceCellML_APISPEC.hxx \
  $(top_builddir)/interfaces/IfaceCellML_events.hxx
