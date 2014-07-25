omniidl -p ../../omniidl_be -bsimple_cpp Test.idl
omniidl -p ../../omniidl_be -bcxx Test.idl
omniidl -p ../../omniidl_be -bsimple_cpp xpcom.idl
omniidl -p ../../omniidl_be -bcxx xpcom.idl
g++ -I. -I../../glue -Wall -ggdb -c CCITest.cxx
g++ -I. -I../../glue -Wall -ggdb -c SCITest.cxx
