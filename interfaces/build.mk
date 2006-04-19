# Interfaces module...
include_HEADERS += \
  interfaces/IfaceDOM-APISPEC.hxx \
  interfaces/IfaceDOM-events.hxx \
  interfaces/Ifacexpcom.hxx \
  interfaces/IfaceMathML-content-APISPEC.hxx \
  interfaces/IfaceCellML-APISPEC.hxx \
  interfaces/IfaceCellML-events.hxx

Iface%.hxx CCI%.hxx SCI%.hxx CCI%.cxx SCI%.cxx: $(top_srcdir)/interfaces/%.idl
	omniidl -p $(top_srcdir)/simple_interface_generators/omniidl_be -bsimple_cpp $<
