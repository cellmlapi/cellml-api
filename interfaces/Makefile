all: IfaceDOM-APISPEC.hxx Ifacexpcom.hxx IfaceMathML-content-APISPEC.hxx IfaceCellML-APISPEC.hxx

Iface%.hxx: %.idl
	omniidl -p ../../simple_interface_generators/omniidl_be -bsimple_cpp $<
