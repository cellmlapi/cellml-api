CellML DOM API README
---------------------

This is the CellML DOM API, an implementation of the CellML API built on top
of the W3C Document Object Model.

Please follow the instructions in INSTALL to install this software.

Accessing the API interface reference
-------------------------------------

The CellML API uses IDL (interface definition language) to define all
interfaces available through the API. These IDL files contain documentation of
the interfaces inline, using the doxygen style.

All interfaces live in the interfaces directory, and have a .idl file extension.
For example, the core API interfaces are in interfaces/CellML_APISPEC.idl

You can browse a version of the interface reference documentation (which may
not correspond to the version you have) online at
<http://cellml-api.sourceforge.net/>.
Other documentation, includig information on how to build the API, is available
at that site.

You can also generate the documentation as follows:
  cd interfaces
  doxygen
Documentation will end up in interfaces/docs/html

Using the test programs
-----------------------
You may want to try out the test programs. To simply run the tests, after
configuring and building the API, you simply need to run:
  make check

This will compile and run all tests.

After this, you might want to do some manual tests. Please keep in mind that
the test programs are designed to demonstrate the basics underlying the API, and
not necessarily be useful tools in their own right. For actual modelling work,
we recommend using a tool designed for that purpose, such as OpenCell
(see http://www.opencell.org/).

Some examples:
  To try out the procedural code generation facilities in OpenCell:
    ./CellML2C ./tests/test_xml/SimpleDAE_NonLinear.xml
  You might also want to see that code with names rather than array indices:
    ./CellML2C ./tests/test_xml/SimpleDAE_NonLinear.xml useida
  Or perhaps in IDA style, with names...
    ./CellML2C ./tests/test_xml/SimpleDAE_NonLinear.xml useida usenames
  To see the source code for the CellML2C test program in this file:
    CCGS/tests/CellML2C.cpp

  To try out the CellML Language Export Definition Service (CeLEDS):
    ./testCeLEDS ./tests/test_xml/SimpleDAE_NonLinear.xml ./CeLEDS/languages/MATLAB.xml
  Source code is in CeLEDSExporter/tests/testCeLEDS.cpp

  To try running a simulation:
    LIBRARY_PATH=`pwd`/.libs ./RunCellML tests/test_xml/SimpleDAE_NonLinear.xml step_type ida
  Source code is in CIS/tests/RunCellML.cpp
