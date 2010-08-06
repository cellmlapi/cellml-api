#ifndef ANNOTOOLSTEST_H
#define ANNOTOOLSTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "cda_config.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "IfaceCellML_APISPEC.hxx"
#include "IfaceAnnoTools.hxx"

class AnnoToolsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AnnoToolsTest);
  CPPUNIT_TEST(testASBootstrap);
  CPPUNIT_TEST(testAnnotationToolService);
  CPPUNIT_TEST(testAnnotationSet);
  CPPUNIT_TEST(testObjectAnnotation);
  CPPUNIT_TEST(testStringAnnotation);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testASBootstrap();
  void testAnnotationToolService();
  void testAnnotationSet();
  void testObjectAnnotation();
  void testStringAnnotation();

private:
  iface::cellml_services::AnnotationToolService* mATS;
  iface::cellml_services::AnnotationSet* mAS;
};

#endif // ANNOTOOLSTEST_H
