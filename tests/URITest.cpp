#include "URITest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( URITest );

void
URITest::setUp()
{
}

void
URITest::tearDown()
{
}

void
URITest::testURIParse()
{
  URI u1(L"http://www.ics.uci.edu/pub/ietf/uri/#Related");
  CPPUNIT_ASSERT(u1.scheme == L"http");
  CPPUNIT_ASSERT(u1.authority == L"www.ics.uci.edu");
  CPPUNIT_ASSERT(u1.path == L"/pub/ietf/uri/");
  CPPUNIT_ASSERT(u1.query == L"");
  CPPUNIT_ASSERT(u1.fragment == L"Related");

  URI u2(L"http://www.cellml.org/tools/api/blargh?abc/#def?");
  CPPUNIT_ASSERT(u2.scheme == L"http");
  CPPUNIT_ASSERT(u2.authority == L"www.cellml.org");
  CPPUNIT_ASSERT(u2.path == L"/tools/api/blargh");
  CPPUNIT_ASSERT(u2.query == L"abc/");
  CPPUNIT_ASSERT(u2.fragment == L"def?");

  URI u3(L"file:///a/b/c/d/e/f");
  CPPUNIT_ASSERT(u3.scheme == L"file");
  CPPUNIT_ASSERT(u3.authority == L"");
  CPPUNIT_ASSERT(u3.path == L"/a/b/c/d/e/f");
  CPPUNIT_ASSERT(u3.query == L"");
  CPPUNIT_ASSERT(u3.fragment == L"");
}

void
URITest::testURIResolve()
{
  URI base(L"http://a/b/c/d;p?q");

#define TEST_RESOLVE(a, b) \
  {\
    URI rel(a); \
    URI absu(base, rel); \
    CPPUNIT_ASSERT(absu.absoluteURI() == b); \
  }

  // All the test-cases for URL resolution from RFC2396...
  TEST_RESOLVE(L"g:h", L"g:h");
  TEST_RESOLVE(L"g", L"http://a/b/c/g");
  TEST_RESOLVE(L"./g", L"http://a/b/c/g");
  TEST_RESOLVE(L"g/", L"http://a/b/c/g/");
  TEST_RESOLVE(L"/g", L"http://a/g");
  TEST_RESOLVE(L"//g", L"http://g");
  TEST_RESOLVE(L"?y", L"http://a/b/c/?y");
  TEST_RESOLVE(L"g?y", L"http://a/b/c/g?y");
  TEST_RESOLVE(L"#s", L"http://a/b/c/d;p?q#s");
  TEST_RESOLVE(L"g#s", L"http://a/b/c/g#s");
  TEST_RESOLVE(L"g?y#s", L"http://a/b/c/g?y#s");
  TEST_RESOLVE(L";x", L"http://a/b/c/;x");
  TEST_RESOLVE(L"g;x", L"http://a/b/c/g;x");
  TEST_RESOLVE(L"g;x?y#s", L"http://a/b/c/g;x?y#s");
  TEST_RESOLVE(L".", L"http://a/b/c/");
  TEST_RESOLVE(L"./", L"http://a/b/c/");
  TEST_RESOLVE(L"..", L"http://a/b/");
  TEST_RESOLVE(L"../", L"http://a/b/");
  TEST_RESOLVE(L"../g", L"http://a/b/g");
  TEST_RESOLVE(L"../..", L"http://a/");
  TEST_RESOLVE(L"../../", L"http://a/");
  TEST_RESOLVE(L"../../g", L"http://a/g");
  TEST_RESOLVE(L"", L"http://a/b/c/d;p?q");

}

void
URITest::testURISerialise()
{
  URI u1(L"http://www.ics.uci.edu/pub/ietf/uri/#Related");
  CPPUNIT_ASSERT(u1.absoluteURI() == L"http://www.ics.uci.edu/pub/ietf/uri/#Related");
  URI u2(L"http://www.cellml.org/tools/api/blargh?abc/#def?");
  CPPUNIT_ASSERT(u2.absoluteURI() == L"http://www.cellml.org/tools/api/blargh?abc/#def?");
  URI u3(L"file:///a/b/c/d/e/f");
  CPPUNIT_ASSERT(u3.absoluteURI() == L"file:///a/b/c/d/e/f");
}

void
URITest::testURISerialiseRelative()
{
  URI base(L"http://www.cellml.org/tools/api/blargh?abc/#def?");
  URI r1(L"http://www.cellml.org/tools/pcenv/index_html?foo#bar");
  CPPUNIT_ASSERT(r1.relativeURI(base) == L"../pcenv/index_html?foo#bar");
  URI r2(L"http://www.cellml.org/index_html");
  CPPUNIT_ASSERT(r2.relativeURI(base) == L"../../index_html");
  URI r3(L"file:///a/b/c/d/e/f");
  CPPUNIT_ASSERT(r3.relativeURI(base) == L"file:///a/b/c/d/e/f");
}
