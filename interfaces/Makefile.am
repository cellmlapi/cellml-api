include_HEADERS=IfaceDOM-APISPEC.hxx \
		Ifacexpcom.hxx \
		IfaceMathML-content-APISPEC.hxx \
		IfaceCellML-APISPEC.hxx IfaceCellML-Services.hxx

Iface%.hxx: %.idl
	omniidl -p $(top_srcdir)/simple_interface_generators/omniidl_be -bsimple_cpp $<
