#include "CeVASTest.hpp"
#include "CeVASBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"
#include <set>

#include "cda_config.h"
#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( CeVASTest );

void
CeVASTest::setUp()
{
  mCeVASBootstrap = CreateCeVASBootstrap();
  mBootstrap = CreateCellMLBootstrap();
  mModelLoader = mBootstrap->modelLoader();
}

void
CeVASTest::tearDown()
{
  if (mCeVASBootstrap != NULL)
    mCeVASBootstrap->release_ref();
  if (mBootstrap != NULL)
    mBootstrap->release_ref();
  if (mModelLoader != NULL)
    mModelLoader->release_ref();
}

void
CeVASTest::testCeVASBootstrap()
{
  CPPUNIT_ASSERT(mCeVASBootstrap);
}

void
CeVASTest::testCeVASCore()
{
  iface::cellml_api::Model* tenTusscher = mModelLoader->loadFromURL
    (L"http://www.bioeng.auckland.ac.nz/people/nickerso/research/models/"
     L"2004_tenTusscher/experiments/periodic-stimulus-endocardial.xml");

//  interface CeVASBootstrap
//    : XPCOM::IObject
//  {
//    /**
//     * Creates a CeVAS object for a given model. The CeVAS object is not live,
//     * and the model should not be changed while the CeVAS object is in use.
//     * @param aModel The top-level model on which CeVAS should operate.
//     * @returns A CeVAS object.
//     */
//    CeVAS createCeVASForModel(in cellml_api::Model aModel);
  iface::cellml_services::CeVAS* c = mCeVASBootstrap->createCeVASForModel(tenTusscher);
  CPPUNIT_ASSERT(c);

//  };
//
//  interface CeVAS
//    : XPCOM::IObject
//  {
//    /**
//     * A description of any connection or interface errors found in the model.
//     * If no error is found, this will be the empty string. The results of any
//     * other operation on this object is undefined if there is an error, and so
//     * should not be called.
//     */
//    readonly attribute wstring modelError;
  wchar_t* str = c->modelError();
  CPPUNIT_ASSERT(!wcscmp(str, L""));
  free(str);

//
//    /**
//     * Creates a (non-live) iterator of components which are relevant to this
//     * model. This includes imported components, but not components which are
//     * not directly imported and are not included by encapsulation.
//     */
//    cellml_api::CellMLComponentIterator iterateRelevantComponents();
  iface::cellml_api::CellMLComponentIterator* cci =
    c->iterateRelevantComponents();
  std::set<iface::cellml_api::CellMLComponent*> relevant;
  iface::cellml_api::CellMLComponent* comp;
  size_t count = 0;
  while ((comp = cci->nextComponent()) != NULL)
  {
    count++;
    CPPUNIT_ASSERT_EQUAL(0, (int)relevant.count(comp));
    relevant.insert(comp);
  }

  cci->release_ref();

  // XXX the number 41 came from running the code being tested. If anyone has
  // time to actually check this manually, please report to
  // ak.miller@auckland.ac.nz.
  CPPUNIT_ASSERT_EQUAL(41, (int)count);

  std::set<iface::cellml_api::CellMLComponent*>::iterator scci;
  for (scci = relevant.begin(); scci != relevant.end(); scci++)
    (*scci)->release_ref();

//
//    /**
//     * Fetches the ConnectedVariableSet that a given variable belongs to.
//     * @param aVariable The variable to look for. This variable must be
//     *                  in the model this CeVAS object was created for.
//     * @returns The corresponding ConnectedVariableSet.
//     */
//    ConnectedVariableSet findVariableSet(in cellml_api::CellMLVariable aVariable);
  iface::cellml_api::CellMLComponentSet* ccs = tenTusscher->modelComponents();
  iface::cellml_api::CellMLComponent* cc = ccs->getComponent(L"model");
  ccs->release_ref();
  iface::cellml_api::CellMLVariableSet* cvs = cc->variables();
  iface::cellml_api::CellMLVariable* cv = cvs->getVariable(L"Cai");
  cvs->release_ref();
  cc->release_ref();

  // We are testing the ConnectedVariableSet itself later...
  iface::cellml_services::ConnectedVariableSet* cvs1 = c->findVariableSet(cv);

//
//    /**
//     * Fetches a count of the number of disjoint sets of variables.
//     */
//    readonly attribute unsigned long length;
  // XXX another value which has come from the code being tested for now.
  //   If you have time, please count them manually (or help me to write a
  //   simpler test so it is more obvious).
  CPPUNIT_ASSERT_EQUAL(165, (int)c->length());

//
//    /**
//     * Fetches a variable set by index (order isn't important).
//     * @param index The zero-based index of the variable set.
//     */
//    ConnectedVariableSet getVariableSet(in unsigned long index);
  size_t i;
  std::set<iface::cellml_services::ConnectedVariableSet*> cvss;
  for (i = 0; i < 165; i++)
  {
    iface::cellml_services::ConnectedVariableSet* cvst = c->getVariableSet(i);
    CPPUNIT_ASSERT_EQUAL(0, (int)cvss.count(cvst));
    cvss.insert(cvst);
  }

  CPPUNIT_ASSERT_EQUAL(1, (int)cvss.count(cvs1));

  std::set<iface::cellml_services::ConnectedVariableSet*>::iterator cvssi;
  for (cvssi = cvss.begin(); cvssi != cvss.end(); cvssi++)
    (*cvssi)->release_ref();

//  };
//  interface ConnectedVariableSet
//    : XPCOM::IObject
//  {
//    /**
//     * The source variable (that is, the variable with no public or private
//     * interfaces).
//     */
//    readonly attribute cellml_api::CellMLVariable sourceVariable;
  iface::cellml_api::CellMLVariable* cv2 = cvs1->sourceVariable();
  str = cv2->name();
  CPPUNIT_ASSERT(!wcscmp(str, L"Cai"));
  free(str);
  str = cv2->componentName();
  CPPUNIT_ASSERT(!wcscmp(str, L"calcium_dynamics"));
  free(str);
  cv2->release_ref();

//
//    /**
//     * Returns the number of variables in this set.
//     */
//    readonly attribute unsigned long length;
  CPPUNIT_ASSERT_EQUAL(11, (int)cvs1->length());

//
//    /**
//     * Fetches the variable by a zero-based index. Order is not important.
//     */
//    cellml_api::CellMLVariable getVariable(in unsigned long index);
  std::set<std::pair<std::wstring,std::wstring> > sv;
  for (i = 0; i < 11; i++)
  {
    iface::cellml_api::CellMLVariable* v = cvs1->getVariable(i);
    str = v->name();
    wchar_t* str2 = v->componentName();
    std::pair<std::wstring,std::wstring> p(str, str2);
    free(str);
    free(str2);

    CPPUNIT_ASSERT_EQUAL(0, (int)sv.count(p));
    sv.insert(p);
    v->release_ref();
  }

  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"ICaL")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"IpCa")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"INaCa")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"calcium_dynamics")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"Jleak")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"Jup")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"Jrel")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"fCa_gate")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"g_gate")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>(L"Cai", L"interface")));
  CPPUNIT_ASSERT_EQUAL(1, (int)sv.count(std::pair<std::wstring,
                                                  std::wstring>
                                        (L"intracellular_concentration", L"one_ion")));

//  };
//

  cv->release_ref();
  cvs1->release_ref();
  c->release_ref();
  tenTusscher->release_ref();
}
