#include "VACSSTest.hpp"
#include "VACSSBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

#include "cda_config.h"
#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( VACSSTest );

void
VACSSTest::setUp()
{
  mVACSService = CreateVACSService();
}

void
VACSSTest::tearDown()
{
  if (mVACSService != NULL)
    mVACSService->release_ref();
}

void
VACSSTest::testVACSService()
{
  CPPUNIT_ASSERT(mVACSService);
}

void
VACSSTest::testGetPositionInXML()
{
  iface::cellml_api::CellMLBootstrap* cellbs = CreateCellMLBootstrap();
  iface::cellml_api::DOMURLLoader* ml = cellbs->localURLLoader();
  cellbs->release_ref();

  iface::dom::Document* d = ml->loadDocument
    (BASE_DIRECTORY L"glycolysis_pathway_1997.xml");
  ml->release_ref();

  //    /**
  //     * Retrieves the position of a given node in the serialised XML
  //     * representation.
  //     * @param node The node to which this applies.
  //     * @param nodalOffset An offset into the nodal data.
  //     * @param column Output for the column number.
  //     * @return The line number.
  //     */
  //    unsigned long getPositionInXML(in dom::Node node,
  //                                   in unsigned long nodalOffset,
  //                                   out unsigned long column);
  
  uint32_t row, col;
  row = mVACSService->getPositionInXML(d, 0, &col);
  CPPUNIT_ASSERT_EQUAL(1, (int)row);
  CPPUNIT_ASSERT_EQUAL(1, (int)col);

  iface::dom::Element* de = d->documentElement();

  row = mVACSService->getPositionInXML(de, 0, &col);
  CPPUNIT_ASSERT_EQUAL(24, (int)row);
  CPPUNIT_ASSERT_EQUAL(5, (int)col);

  iface::dom::NodeList* nl = de->getElementsByTagNameNS
    (L"http://www.cellml.org/cellml/1.0#", L"component");
  de->release_ref();
  iface::dom::Node* el = nl->item(0);
  nl->release_ref();

  row = mVACSService->getPositionInXML(el, 0, &col);
  CPPUNIT_ASSERT_EQUAL(233, (int)row);
  CPPUNIT_ASSERT_EQUAL(4, (int)col);

  el->release_ref();
  d->release_ref();
}
