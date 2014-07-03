#ifndef CELLMLTEST_H
#define CELLMLTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "CppUnitWstringSupport.hpp"
#include "cda_compiler_support.h"
#include "IfaceMathML_content_APISPEC.hxx"
#include "IfaceCellML_APISPEC.hxx"

class CellMLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CellMLTest);
  CPPUNIT_TEST(testCellMLBootstrap);
  CPPUNIT_TEST(testModelLoader);
  CPPUNIT_TEST(testDOMModelLoader);
  CPPUNIT_TEST(testDOMURLLoader);
  CPPUNIT_TEST(testRDFRepresentation);
  CPPUNIT_TEST(testRDFXMLDOMRepresentation);
  CPPUNIT_TEST(testRDFXMLStringRepresentation);
  CPPUNIT_TEST(testURI);
  CPPUNIT_TEST(testCellMLElement);
  CPPUNIT_TEST(testCellMLDOMElement);
  CPPUNIT_TEST(testNamedCellMLElement);
  CPPUNIT_TEST(testModel);
  CPPUNIT_TEST(testMathContainer);
  CPPUNIT_TEST(testCellMLComponent);
  CPPUNIT_TEST(testUnits);
  CPPUNIT_TEST(testUnit);
  CPPUNIT_TEST(testCellMLImport);
  CPPUNIT_TEST(testImportComponent);
  CPPUNIT_TEST(testImportUnits);
  CPPUNIT_TEST(testCellMLVariable);
  CPPUNIT_TEST(testComponentRef);
  CPPUNIT_TEST(testRelationshipRef);
  CPPUNIT_TEST(testGroup);
  CPPUNIT_TEST(testConnection);
  CPPUNIT_TEST(testReaction);
  CPPUNIT_TEST(testExtensionElementList);
  CPPUNIT_TEST(testExtensionAttributeSet);
  CPPUNIT_TEST(testCellMLElementSet);
  CPPUNIT_TEST(testIteratorLiveness);
  CPPUNIT_TEST(testRelativeImports);
  CPPUNIT_TEST(testImportClone);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void tearDown();
  void loadAchCascade();
  void loadBeelerReuter();
  void loadTenTusscher();
  void loadGlycolysis();
  void loadRelativeURIModel();

  void testCellMLBootstrap();
  void testDOMModelLoader();
  void testRDFRepresentation();
  void testRDFXMLDOMRepresentation();
  void testRDFXMLStringRepresentation();
  void testURI();
  void testCellMLElement();
  void testCellMLDOMElement();
  void testNamedCellMLElement();
  void testModel();
  void testMathContainer();
  void testCellMLComponent();
  void testUnits();
  void testUnit();
  void testCellMLImport();
  void testImportComponent();
  void testImportUnits();
  void testCellMLVariable();
  void testComponentRef();
  void testRelationshipRef();
  void testGroup();
  void testConnection();
  void testReaction();
  void testModelLoader();
  void testDOMURLLoader();
  void testExtensionElementList();
  void testExtensionAttributeSet();
  void testCellMLElementSet();
  void testIteratorLiveness();
  void testRelativeImports();
  void testImportClone();
private:
  iface::cellml_api::CellMLBootstrap* mBootstrap;
  iface::cellml_api::DOMModelLoader* mModelLoader;
  iface::dom::DOMImplementation* mDOMImplementation;
  iface::cellml_api::DOMURLLoader* mLocalURLLoader;
  iface::cellml_api::Model* mAchCascade;
  iface::cellml_api::Model* mBeelerReuter;
  iface::cellml_api::Model* mTenTusscher;
  iface::cellml_api::Model* mGlycolysis;
  iface::cellml_api::Model* mRelativeURI;
};

#endif // CELLMLTEST_H
