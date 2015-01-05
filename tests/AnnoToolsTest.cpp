#include "AnnoToolsTest.hpp"
#include "AnnoToolsBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

#ifdef HAVE_STD_CODECVT
#include "CppUnitWstringSupport.hpp"
#endif // HAVE_STD_CODECVT

#include "cda_compiler_support.h"
#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( AnnoToolsTest );

void
AnnoToolsTest::setUp()
{
  mATS = CreateAnnotationToolService();
  if (mATS != NULL)
    mAS = mATS->createAnnotationSet();
  else
    mAS = NULL;
}

void
AnnoToolsTest::tearDown()
{
  if (mATS != NULL)
    mATS->release_ref();
  if (mAS != NULL)
    mAS->release_ref();
}

void
AnnoToolsTest::testASBootstrap()
{
  CPPUNIT_ASSERT(mATS);
}

void
AnnoToolsTest::testAnnotationToolService()
{
  CPPUNIT_ASSERT(mATS);
}

void
AnnoToolsTest::testAnnotationSet()
{
  std::wstring str = mAS->prefixURI();
#define PREFIX L"http://www.cellml.org/tools/annotools/set"
  CPPUNIT_ASSERT(!wcsncmp(str.c_str(), PREFIX, sizeof(PREFIX)/sizeof(PREFIX[0]) - 1));
#undef PREFIX
}

void
AnnoToolsTest::testObjectAnnotation()
{
  iface::cellml_api::CellMLBootstrap* bs = CreateCellMLBootstrap();
  iface::cellml_api::DOMModelLoader* ml = bs->modelLoader();
  bs->release_ref();
  iface::cellml_api::Model* m =
    ml->loadFromURL(BASE_DIRECTORY L"Ach_cascade_1995.xml");

  CPPUNIT_ASSERT_NO_THROW(mAS->setObjectAnnotation(m, L"theloader", ml));

  iface::XPCOM::IObject* oa = mAS->getObjectAnnotation(m, L"theloader");
  {
    CPPUNIT_ASSERT(oa);
    std::string o1 = oa->objid();
    std::string o2 = ml->objid();
    CPPUNIT_ASSERT(o1 == o2);
  }
  oa->release_ref();

  oa = mAS->getObjectAnnotationWithDefault(m, L"theloader", NULL);
  {
    CPPUNIT_ASSERT(oa);
    std::string o1 = oa->objid();
    std::string o2 = ml->objid();
    CPPUNIT_ASSERT(o1 == o2);
  }
  oa->release_ref();
  oa = mAS->getObjectAnnotationWithDefault(m, L"theloaderno", NULL);
  CPPUNIT_ASSERT(!oa);

  std::wstring fstr = mAS->prefixURI();
  fstr += L"theloader";

  oa = m->getUserData(fstr.c_str());
  CPPUNIT_ASSERT(oa);
  oa->release_ref();

  mAS->release_ref();
  mAS = NULL;

  CPPUNIT_ASSERT_THROW(oa = m->getUserData(fstr.c_str()), iface::cellml_api::CellMLException);

  ml->release_ref();
  m->release_ref();
}

void
AnnoToolsTest::testStringAnnotation()
{
  iface::cellml_api::CellMLBootstrap* bs = CreateCellMLBootstrap();
  iface::cellml_api::DOMModelLoader* ml = bs->modelLoader();
  bs->release_ref();
  iface::cellml_api::Model* m =
    ml->loadFromURL(BASE_DIRECTORY L"Ach_cascade_1995.xml");

  CPPUNIT_ASSERT_NO_THROW(mAS->setStringAnnotation(m, L"thestring", L"hello"));

  std::wstring anno = mAS->getStringAnnotation(m, L"thestring");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"hello"), anno);

  std::wstring anno2 = mAS->getStringAnnotationWithDefault(m, L"thestring", L"mystring");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"hello"), anno2);
  std::wstring anno3 = mAS->getStringAnnotationWithDefault(m, L"thestringno", L"mystring");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"mystring"), anno3);


  std::wstring fstr = mAS->prefixURI();
  fstr += L"thestring";

  iface::cellml_api::UserData* oa = m->getUserData(fstr.c_str());
  CPPUNIT_ASSERT(oa);
  oa->release_ref();

  mAS->release_ref();
  mAS = NULL;

  CPPUNIT_ASSERT_THROW(oa = m->getUserData(fstr.c_str()), iface::cellml_api::CellMLException);

  ml->release_ref();
  m->release_ref();
}
