# Interfaces module...
include_HEADERS += \
  $(top_builddir)/interfaces/IfaceDOM-APISPEC.hxx \
  interfaces/IfaceDOM-events.hxx \
  interfaces/Ifacexpcom.hxx \
  interfaces/IfaceMathML-content-APISPEC.hxx \
  interfaces/IfaceCellML-APISPEC.hxx \
  interfaces/IfaceCellML-events.hxx

$(top_builddir)/interfaces/Iface%.hxx \
$(top_builddir)/interfaces/CCI%.hxx \
$(top_builddir)/interfaces/SCI%.hxx \
$(top_builddir)/interfaces/CCI%.cxx \
$(top_builddir)/interfaces/SCI%.cxx: $(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	omniidl -p $$SAVEDIR/simple_interface_generators/omniidl_be -bsimple_cpp $$SAVEDIR/$< && \
	cd $$SAVEDIR
