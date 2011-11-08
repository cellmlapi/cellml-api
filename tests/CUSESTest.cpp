#include "CUSESTest.hpp"
#include "CUSESBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

#include "cda_compiler_support.h"
#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( CUSESTest );

void
CUSESTest::setUp()
{
  mCUSESBootstrap = CreateCUSESBootstrap();
  mBootstrap = CreateCellMLBootstrap();
  mModelLoader = mBootstrap->modelLoader();
}

void
CUSESTest::tearDown()
{
  if (mCUSESBootstrap != NULL)
    mCUSESBootstrap->release_ref();
  if (mBootstrap != NULL)
    mBootstrap->release_ref();
  if (mModelLoader != NULL)
    mModelLoader->release_ref();
}

void
CUSESTest::testCUSESBootstrap()
{
  CPPUNIT_ASSERT(mCUSESBootstrap);
}

void
CUSESTest::testCUSESObject()
{
  iface::cellml_api::Model* tenTusscher =
    mModelLoader->loadFromURL
    (L"http://www.cellml.org/Members/miller/andres_models/"
     L"2004_tenTusscher/experiments/periodic-stimulus-endocardial.xml");
  iface::cellml_services::CUSES* c =
    mCUSESBootstrap->createCUSESForModel(tenTusscher, false);

  std::wstring str = c->modelError();
  // This happens due to an error in the coding of the model.
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; Found a unit with no units attribute in units mm_per_ms; "), str);

  iface::cellml_services::CanonicalUnitRepresentation* cu1
    = c->getUnitsByName(tenTusscher, L"mM_per_ms");

  iface::cellml_services::CanonicalUnitRepresentation* cu2
    = c->getUnitsByName(tenTusscher, L"mM_per_s");

  iface::cellml_services::CanonicalUnitRepresentation* cu3
    = c->getUnitsByName(tenTusscher, L"celsius");

  uint32_t l = cu1->length();
  CPPUNIT_ASSERT_EQUAL(3, (int)l);
  iface::cellml_services::BaseUnitInstance
    * bui1 = cu1->fetchBaseUnit(0),
    * bui2 = cu1->fetchBaseUnit(1),
    * bui3 = cu1->fetchBaseUnit(2), *buitmp;

  iface::cellml_services::BaseUnit * bu1 = bui1->unit(),
    * bu2 = bui2->unit(), * bu3 = bui3->unit(), * butmp;

  std::wstring str1 = bu1->name(), str2 = bu2->name(), str3 = bu3->name(), strtmp;
  
#define ASSIGN(n1, n2) \
  bui##n1 = bui##n2; \
  bu##n1 = bu##n2; \
  str##n1 = str##n2;
#define SWAP(n1, n2) \
  ASSIGN(tmp, n1); \
  ASSIGN(n1, n2); \
  ASSIGN(n2, tmp);

  if (str2 == L"mole")
  {
    SWAP(1, 2);
  }
  else if (str3 == L"mole")
  {
    SWAP(1, 3);
  }
  if (str2 == L"second")
  {
    SWAP(2, 3);
  }
#undef SWAP
#undef ASSIGN

  CPPUNIT_ASSERT_EQUAL(std::wstring(L"mole"), str1);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"metre"), str2);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"second"), str3);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(bui1->exponent(), 1.0, 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(bui2->exponent(), -3.0, 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(bui3->exponent(), -1.0, 1E-10);

  bu1->release_ref();
  bu2->release_ref();
  bu3->release_ref();
  bui1->release_ref();
  bui2->release_ref();
  bui3->release_ref();

  CPPUNIT_ASSERT(cu1->compatibleWith(cu1));
  CPPUNIT_ASSERT(cu2->compatibleWith(cu2));
  CPPUNIT_ASSERT(cu3->compatibleWith(cu3));
  CPPUNIT_ASSERT(cu1->compatibleWith(cu2));
  CPPUNIT_ASSERT(cu2->compatibleWith(cu1));
  CPPUNIT_ASSERT(!cu1->compatibleWith(cu3));
  CPPUNIT_ASSERT(!cu2->compatibleWith(cu3));
  CPPUNIT_ASSERT(!cu3->compatibleWith(cu1));
  CPPUNIT_ASSERT(!cu3->compatibleWith(cu2));

  double offset;

  // 1000 mM_per_ms in 1 mM_per_s
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, cu1->convertUnits(cu2, &offset), 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, offset, 1E-10);

  // 1/1000th mM_per_s in 1 mM_per_ms
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.001, cu2->convertUnits(cu1, &offset), 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, offset, 1E-10);

  // 1 mM_per_ms = 1 mmole / L / ms = 1O^-3 mole / (10^-3 m^3) / (10^-3 s) = 10^3 mole/m^3/s
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, cu1->siConversion(&offset), 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, offset, 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, cu2->siConversion(&offset), 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, offset, 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, cu3->siConversion(&offset), 1E-10);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(273.15, offset, 1E-10);

  cu1->release_ref();
  cu2->release_ref();
  cu3->release_ref();

  c->release_ref();

  tenTusscher->release_ref();
}
