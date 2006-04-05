include_HEADERS=IfaceDOM-APISPEC.hxx \
		IfaceDOM-events.hxx \
		Ifacexpcom.hxx \
		IfaceMathML-content-APISPEC.hxx \
		IfaceCellML-APISPEC.hxx IfaceCellML-Services.hxx

Iface%.hxx: $(top_srcdir)/interfaces/%.idl
	omniidl -p $(top_srcdir)/simple_interface_generators/omniidl_be -bsimple_cpp $<
