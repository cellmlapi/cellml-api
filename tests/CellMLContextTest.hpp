#ifndef CELLMLCONTEXTTEST_H
#define CELLMLCONTEXTTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include "IfaceCellML_Context.hxx"

class CellMLContextTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CellMLContextTest);
  CPPUNIT_TEST(testCellMLContext);
  CPPUNIT_TEST(testModelList);
  CPPUNIT_TEST(testModelNode);
  CPPUNIT_TEST_SUITE_END();

  iface::cellml_context::CellMLContext* mContext;
  iface::cellml_api::DOMModelLoader* mModelLoader;
  iface::cellml_api::Model* mAchCascade;
  iface::cellml_api::Model* mBeelerReuter;
public:
  void setUp();
  void tearDown();

  void loadAchCascade();
  void loadBeelerReuter();

  void testCellMLContext();
  void testModelList();
  void testModelNode();
};

#endif // CELLMLCONTEXTTEST_H
