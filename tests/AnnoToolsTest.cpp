#include "AnnoToolsTest.hpp"
#include "AnnoToolsBootstrap.hpp"
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
  wchar_t* str = mAS->prefixURI();
  CPPUNIT_ASSERT(str);
#define PREFIX L"http://www.cellml.org/tools/annotools/set"
  CPPUNIT_ASSERT(!wcsncmp(str, PREFIX, sizeof(PREFIX)/sizeof(PREFIX[0]) - 1));
#undef PREFIX
  free(str);
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

  CPPUNIT_ASSERT(oa);
  char* o1 = oa->objid();
  char* o2 = ml->objid();
  CPPUNIT_ASSERT(!strcmp(o1, o2));
  free(o1);
  free(o2);

  oa->release_ref();

  wchar_t* str = mAS->prefixURI();
  std::wstring fstr = str;
  free(str);
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

  wchar_t* anno = mAS->getStringAnnotation(m, L"thestring");
  CPPUNIT_ASSERT(!wcscmp(anno, L"hello"));
  free(anno);

  wchar_t* str = mAS->prefixURI();
  std::wstring fstr = str;
  free(str);
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
