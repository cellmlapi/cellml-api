#define __STDC_FORMAT_MACROS
#include "cda_compiler_support.h"
#include "RDFTest.hpp"
#include "DOMBootstrap.hxx"
#include "CellMLBootstrap.hpp"
#include "RDFBootstrap.hpp"
#include "Utilities.hxx"
#include <fstream>
#include <iostream>

#ifndef PRIxPTR
#define PRIxPTR "x"
#endif

#ifndef BASE_DIRECTORY_NTRIP
#define BASE_DIRECTORY_NTRIP TESTDIR8 "/test_rdf/"
#endif
#ifndef BASE_URI
#ifdef WIN32
#define BASE_URI L"file:///" TESTDIR L"/test_rdf/"
#else
#define BASE_URI L"file://" TESTDIR L"/test_rdf/"
#endif
#endif
#ifndef BASE_DIRECTORY_CELLML
#ifdef WIN32
#define BASE_DIRECTORY_CELLML L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY_CELLML L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( RDFTest );

void
RDFTest::setUp()
{
}

void
RDFTest::tearDown()
{
}

static char*
CDA_wchar_to_UTF8(const wchar_t *str)
{
  uint32_t len = 0;
  const wchar_t* p = str;
  wchar_t c;
  for (; (c = *p); p++)
  {
    if (c <= 0x7F)
      len++;
    else if (c <= 0x7FF)
      len += 2;
#ifndef WCHAR_T_CONSTANT_WIDTH
    else if ((c & 0xFC00) == 0xD800)
      len += 4;
#endif
    else
#if defined(WCHAR_T_CONSTANT_WIDTH) && defined(WCHAR_T_IS_32BIT)
      if (c <= 0xFFFF)
#endif
      len += 3;
#if defined(WCHAR_T_CONSTANT_WIDTH) && defined(WCHAR_T_IS_32BIT)
    else
      len += 4;
#endif
  }
  char* newData = (char*)malloc(len + 1);
  char* np = newData;
  p = str;

  while ((c = *p++))
  {
    if (c <= 0x7F)
      *np++ = (char)c;
    else if (c <= 0x7FF)
    {
      *np++ = (char)(0xC0 | ((c >> 6) & 0x1F));
      *np++ = (char)(0x80 | (c & 0x3F));
    }
#ifndef WCHAR_T_CONSTANT_WIDTH
    else if ((c & 0xFC00) == 0xD800)
    {
      uint16_t u = ((c >> 6) & 0xF) + 1;
      *np++ = 0xF0 | ((u >> 2) & 0x7);
      *np++ = 0x80 | ((u << 4) & 0x30) | ((c >> 2) & 0xF);
      wchar_t c2 = *p++;
      *np++ = 0x80 | ((c << 4) & 0x30) | ((c2 >> 6) & 0xF);
      *np++ = 0x80 | (c2 & 0x3F);
    }
#endif
    else
#if defined(WCHAR_T_CONSTANT_WIDTH) && defined(WCHAR_T_IS_32BIT)
         if (c <= 0xFFFF)
#endif
    {
      *np++ = (char)(0xE0 | ((c >> 12) & 0xF));
      *np++ = (char)(0x80 | ((c >> 6) & 0x3F));
      *np++ = (char)(0x80 | (c & 0x3F));
    }
#if defined(WCHAR_T_CONSTANT_WIDTH) && defined(WCHAR_T_IS_32BIT)
    else
    {
      *np++ = (char)(0xF0 | ((c >> 18) & 0x7));
      *np++ = (char)(0x80 | ((c >> 12) & 0x3F));
      *np++ = (char)(0x80 | ((c >> 6) & 0x3F));
      *np++ = (char)(0x80 | (c & 0x3F));
    }
#endif
  }

  *np++ = 0;
  return newData;
}

std::ostream&
operator<<(std::ostream& aStream, iface::rdf_api::Node* aNode)
{
  DECLARE_QUERY_INTERFACE_OBJREF(res, aNode, rdf_api::Resource);
  if (res != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(ur, aNode, rdf_api::URIReference);
    if (ur != NULL)
    {
      RETURN_INTO_WSTRING(uri, ur->URI());
      char* t = CDA_wchar_to_UTF8(uri.c_str());
      aStream << "<" << t << ">";
      free(t);
    }
    else
    {
      aStream << "<blanknode>";
    }
  }
  else
  {
    DECLARE_QUERY_INTERFACE_OBJREF(pl, aNode, rdf_api::PlainLiteral);
    if (pl != NULL)
    {
      RETURN_INTO_WSTRING(lf, pl->lexicalForm());
      RETURN_INTO_WSTRING(lang, pl->language());
      char* lft = CDA_wchar_to_UTF8(lf.c_str());
      char* langt = CDA_wchar_to_UTF8(lang.c_str());

      aStream << "\"" << lft << "\"@" << langt;

      free(lft);
      free(langt);
    }
    else
    {
      DECLARE_QUERY_INTERFACE_OBJREF(tl, aNode, rdf_api::TypedLiteral);
      RETURN_INTO_WSTRING(lf, tl->lexicalForm());
      RETURN_INTO_WSTRING(dt, tl->datatypeURI());
      char* lft = CDA_wchar_to_UTF8(lf.c_str());
      char* dtt = CDA_wchar_to_UTF8(dt.c_str());

      aStream << "\"" << lft << "\"^^<" << dtt << ">";

      free(lft);
      free(dtt);
    }
  }

  return aStream;
}

template<class T>
class ContainerReleaser
{
public:
  ContainerReleaser(T& aContainer)
    : mContainer(aContainer)
  {
  }

  ~ContainerReleaser()
  {
    for (iterator i = mContainer.begin(); i != mContainer.end(); i++)
      (*i)->release_ref();
  }

private:
  T& mContainer;
  typedef typename T::iterator iterator;
};

class NTripleFile
{
public:
  NTripleFile(iface::rdf_api::Bootstrap* aBS, const std::string& testId,
              uint32_t aTestNo)
    : mTestNo(aTestNo)
  {
    std::string path(BASE_DIRECTORY_NTRIP);
    path += testId;
    path += ".nt";
    std::ifstream f(path.c_str());

    mDataSource = already_AddRefd<iface::rdf_api::DataSource>
      (aBS->createDataSource());

    while (f.good())
    {
      std::string l;
      std::getline(f, l);

      if (l[0] == '#')
        continue;
      std::string ws(" \t\r\n");
      std::string::iterator i;
      for (i = l.begin(); i != l.end(); i++)
        if (ws.find(*i) == std::string::npos)
          break;
      if (i == l.end())
        continue;

      ObjRef<iface::rdf_api::Node> subject, predicate, object;
      iface::rdf_api::Node* n;

      l = parsePart(l, &n);
      subject = already_AddRefd<iface::rdf_api::Node>(n);
      DECLARE_QUERY_INTERFACE_OBJREF(subjectr, subject, rdf_api::Resource);
      if (subjectr == NULL)
      {
        std::cout << "Invalid subject in ntriples." << std::endl;
        continue;
      }

      l = parsePart(l, &n);
      predicate = already_AddRefd<iface::rdf_api::Node>(n);
      DECLARE_QUERY_INTERFACE_OBJREF(predicater, predicate, rdf_api::Resource);
      if (predicater == NULL)
      {
        std::cout << "Invalid predicate in ntriples." << std::endl;
        continue;
      }

      parsePart(l, &n);
      object = already_AddRefd<iface::rdf_api::Node>(n);
      if (object == NULL)
      {
        std::cout << "Invalid object in ntriples." << std::endl;
        continue;
      }

      subjectr->createTripleOutOf(predicater, object);
    }
  }

  ~NTripleFile()
  {
    for (std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
           mNTripleBlankNodeByName.begin();
         i != mNTripleBlankNodeByName.end();
         i++)
      (*i).second->release_ref();
  }

  void
  compareTriples(iface::rdf_api::DataSource* aDataSource)
  {
    // std::cout << "Entering compareTriples" << std::endl;
    mErrorCount = 0;

    // We go through every triple in aDataSource, and compare it with
    // mDataSource. We deleted from mDataSource as we go, so anything left
    // over at the end is missing from aDataSource...
    {
      std::list<iface::rdf_api::Triple*> ambigs;
      ContainerReleaser<std::list<iface::rdf_api::Triple*> > ct(ambigs);

      RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet, aDataSource->getAllTriples());
      RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator, ts->enumerateTriples());
      while (true)
      {
        RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
        if (t == NULL)
          break;
        
        if (checkTriple(t, false))
        {
          t->add_ref();
          ambigs.push_back(t);
        }
      }

      for (
           std::list<iface::rdf_api::Triple*>::iterator i = ambigs.begin();
           i != ambigs.end();
           i++
          )
      {
        checkTriple(*i, true);
      }
    }

    // Now, any triples left over in mDataSource are missing from the RDF/XML
    // parse...
    {
      RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet, mDataSource->getAllTriples());
      RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator, ts->enumerateTriples());
      while (true)
      {
        RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
        if (t == NULL)
          break;

        RETURN_INTO_OBJREF(s, iface::rdf_api::Node, t->subject());
        RETURN_INTO_OBJREF(p, iface::rdf_api::Node, t->predicate());
        RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
        
        std::cerr << "RDF/XML Test " << mTestNo
                  << ": Triple missing from RDF/XML parse results: "
                  << s << " " << p << " " << o << " ." << std::endl;
        mErrorCount++;
      }
    }

    for (std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
           mNTripleBlankNodeByXMLBlankNode.begin();
         i != mNTripleBlankNodeByXMLBlankNode.end();
         i++)
      (*i).second->release_ref();
    for (std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
           mXMLBlankNodeByNTripleBlankNode.begin();
         i != mXMLBlankNodeByNTripleBlankNode.end();
         i++)
      (*i).second->release_ref();

    CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(0), mErrorCount);
  }
private:
  std::string
  parsePart(const std::string& aStr, iface::rdf_api::Node** aNode)
  {
    *aNode = NULL;

    if (aStr[0] == '<')
    {
      size_t eos = aStr.find('>');
      if (eos == std::string::npos)
      {
        std::cout << "Unterminated URI reference" << std::endl;
        return "";
      }

      std::wstring URI = unescape(aStr.substr(1, eos - 1));
      *aNode = mDataSource->getURIReference(URI.c_str());

      eos++;
      while (aStr[eos] == ' ' || aStr[eos] == '\t')
        eos++;

      return aStr.substr(eos);
    }
    else if (aStr[0] == '_')
    {
      if (aStr[1] != ':')
      {
        std::cout << "Malformed blank node specification." << std::endl;
        return "";
      }

      size_t pos = aStr.find_first_of(" \t", 2);
      std::string name, rem;
      if (pos == std::string::npos)
      {
        name = aStr.substr(2);
        rem = "";
      }
      else
      {
        name = aStr.substr(2, pos - 2);

        while (aStr[pos] == ' ' || aStr[pos] == '\t')
          pos++;

        rem = aStr.substr(pos);
      }

      *aNode = findOrCreateBlankNode(name);

      return rem;
    }
    else if (aStr[0] == '"')
    {
      size_t pos = 0;
      while (true)
      {
        pos = aStr.find('"', pos + 1);
        if (pos == std::string::npos)
        {
          std::cout << "Unterminated string in " << aStr << std::endl;
          return "";
        }
        // This isn't strictly correct, but it gets us through the tests...
        if (aStr[pos - 1] != '\\')
          break;
      }

      std::wstring raw = unescape(aStr.substr(1, pos - 1));
      pos++;
      if (aStr[pos] == '^')
      {
        pos++;
        if (aStr[pos] != '^')
        {
          std::cout << "Malformed typed literal." << std::endl;
          return "";
        }

        pos++;
        if (aStr[pos] != '<')
        {
          std::cout << "Expected data type after typed literal marker." << std::endl;
          return "";
        }

        pos++;
        size_t eos = aStr.find('>', pos);
        if (eos == std::string::npos)
        {
          std::cout << "Unterminated typed literal datatype." << std::endl;
          return "";
        }

        std::wstring URI = unescape(aStr.substr(pos, eos - pos));

        *aNode = mDataSource->getTypedLiteral(raw.c_str(), URI.c_str());

        eos++;
        while (aStr[eos] == ' ' || aStr[eos] == '\t')
          eos++;

        return aStr.substr(eos);
      }
      
      std::wstring language = L"en";
      if (aStr[pos] == '@')
      {
        pos++;
        size_t end = aStr.find_first_of(" \t");
        if (end == std::string::npos)
        {
          language = unescape(aStr.substr(pos));
          pos = aStr.length();
        }
        else
        {
          language = unescape(aStr.substr(pos, end - pos));
          pos = end;
        }
      }

      *aNode = mDataSource->getPlainLiteral(raw.c_str(), language.c_str());

      while (aStr[pos] == ' ' || aStr[pos] == '\t')
        pos++;

      return aStr.substr(pos);
    }

    std::cout << "Unexpected start of string: " << aStr[0] << std::endl;
    return "";
  }

  iface::rdf_api::BlankNode*
  findOrCreateBlankNode(const std::string& aName)
  {
    std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
      mNTripleBlankNodeByName.find(aName);
    if (i != mNTripleBlankNodeByName.end())
    {
      (*i).second->add_ref();
      return (*i).second;
    }

    iface::rdf_api::BlankNode* n = mDataSource->createBlankNode();
    mNTripleBlankNodeByName.insert(std::pair<std::string, iface::rdf_api::BlankNode*>
                                   (aName, n));

    n->add_ref();
    return n;
  }

  std::wstring
  unescape(const std::string& aEscaped)
  {
    std::wstring output;

    for (std::string::const_iterator i = aEscaped.begin();
         i != aEscaped.end();
         i++)
      if (*i != '\\')
        output += *i;
      else
      {
        i++;
        if (i == aEscaped.end())
          break;
        if (*i == '\\')
          output += L'\\';
        else if (*i == 't')
          output += L'\t';
        else if (*i == 'r')
          output += L'\r';
        else if (*i == 'n')
          output += L'\n';
        else if (*i == '"')
          output += L'"';
        else if (*i == 'u')
        {
          if ((aEscaped.end() - i) < 4)
            break;
          char buf[5];
          buf[0] = *i++;
          buf[1] = *i++;
          buf[2] = *i++;
          buf[3] = *i++;
          buf[4] = 0;
          output += (wchar_t)strtoul(buf, NULL, 16);
        }
        else if (*i == 'U')
        {
          if ((aEscaped.end() - i) < 8)
            break;

          char buf[9];
          buf[0] = *i++;
          buf[1] = *i++;
          buf[2] = *i++;
          buf[3] = *i++;
          buf[4] = *i++;
          buf[5] = *i++;
          buf[6] = *i++;
          buf[7] = *i++;
          buf[8] = 0;
          output += (wchar_t)strtoul(buf, NULL, 16);
        }
      }

    return output;
  }

  bool
  checkTriple(iface::rdf_api::Triple* aTriple, bool allowAmbiguous)
  {
    RETURN_INTO_OBJREF(xmlSubj, iface::rdf_api::Resource, aTriple->subject());
    RETURN_INTO_OBJREF(xmlPredicate, iface::rdf_api::Resource, aTriple->predicate());
    RETURN_INTO_OBJREF(xmlObject, iface::rdf_api::Node, aTriple->object());

    ObjRef<iface::rdf_api::Resource> ntSubj, ntPredicate;
    ObjRef<iface::rdf_api::Node> ntObject;

    DECLARE_QUERY_INTERFACE_OBJREF(xmlSubjURI, xmlSubj, rdf_api::URIReference);
    if (xmlSubjURI != NULL)
    {
      RETURN_INTO_WSTRING(uri, xmlSubjURI->URI());
      ntSubj = already_AddRefd<iface::rdf_api::Resource>
        (mDataSource->getURIReference(uri.c_str()));
    }
    else
    {
      char* objid = xmlSubj->objid();
      std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
        mNTripleBlankNodeByXMLBlankNode.find(objid);
      free(objid);

      if (i != mNTripleBlankNodeByXMLBlankNode.end())
        ntSubj = (*i).second;
    }
    DECLARE_QUERY_INTERFACE_OBJREF(xmlPredURI, xmlPredicate, rdf_api::URIReference);

    if (xmlPredURI != NULL)
    {
      RETURN_INTO_WSTRING(uri, xmlPredURI->URI());
      ntPredicate = already_AddRefd<iface::rdf_api::Resource>
        (mDataSource->getURIReference(uri.c_str()));
    }
    else
    {
      char* objid = xmlPredicate->objid();
      std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
        mNTripleBlankNodeByXMLBlankNode.find(objid);
      free(objid);

      if (i != mNTripleBlankNodeByXMLBlankNode.end())
        ntPredicate = (*i).second;
    }

    DECLARE_QUERY_INTERFACE_OBJREF(xmlObjURI, xmlObject, rdf_api::URIReference);
    if (xmlObjURI != NULL)
    {
      RETURN_INTO_WSTRING(uri, xmlObjURI->URI());
      ntObject = already_AddRefd<iface::rdf_api::Resource>
        (mDataSource->getURIReference(uri.c_str()));
    }
    else
    {
      DECLARE_QUERY_INTERFACE_OBJREF(xmlObjPL, xmlObject, rdf_api::PlainLiteral);
      if (xmlObjPL != NULL)
      {
        RETURN_INTO_WSTRING(form, xmlObjPL->lexicalForm());
        RETURN_INTO_WSTRING(lang, xmlObjPL->language());
        ntObject = already_AddRefd<iface::rdf_api::Node>
          (mDataSource->getPlainLiteral(form.c_str(), lang.c_str()));
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(xmlObjTL, xmlObject, rdf_api::TypedLiteral);
        if (xmlObjTL != NULL)
        {
          RETURN_INTO_WSTRING(form, xmlObjTL->lexicalForm());
          RETURN_INTO_WSTRING(type, xmlObjTL->datatypeURI());
          ntObject = already_AddRefd<iface::rdf_api::Node>
            (mDataSource->getTypedLiteral(form.c_str(), type.c_str()));
        }
        else
        {
          char* objid = xmlObject->objid();
          std::map<std::string, iface::rdf_api::BlankNode*>::iterator i =
            mNTripleBlankNodeByXMLBlankNode.find(objid);
          free(objid);
          
          if (i != mNTripleBlankNodeByXMLBlankNode.end())
            ntObject = (*i).second;
        }
      }
    }

    return checkAndEraseTriple(ntSubj, ntPredicate, ntObject, xmlSubj,
                               xmlPredicate, xmlObject, allowAmbiguous);
  }

  bool
  checkAndEraseTriple(
                      iface::rdf_api::Resource* ntSubj,
                      iface::rdf_api::Resource* ntPredicate,
                      iface::rdf_api::Node* ntObject,
                      iface::rdf_api::Resource* xmlSubj,
                      iface::rdf_api::Resource* xmlPredicate,
                      iface::rdf_api::Node* xmlObject,
                      bool allowAmbiguous
                     )
  {
    std::vector<iface::rdf_api::Triple*> matches;
    ContainerReleaser<std::vector<iface::rdf_api::Triple*> > ct(matches);

    // Enumerate all triples...
    RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet, mDataSource->getAllTriples());
    RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator, ts->enumerateTriples());
    while (true)
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());

      if (t == NULL)
        break;

      // std::cout << "Triple for comparison" << std::endl;

      RETURN_INTO_OBJREF(tripleSubj, iface::rdf_api::Resource, t->subject());
      if (ntSubj)
      {
        if (CDA_objcmp(ntSubj, tripleSubj))
        {
          // std::cout << "Subject mismatch." << std::endl;
          continue;
        }
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(blank, tripleSubj, rdf_api::BlankNode);
        if (blank == NULL)
          continue;

        char* objid = blank->objid();
        bool known = (mXMLBlankNodeByNTripleBlankNode.find(objid) !=
                      mXMLBlankNodeByNTripleBlankNode.end());
        free(objid);

        if (known)
          continue;
      }

      RETURN_INTO_OBJREF(triplePredicate, iface::rdf_api::Resource, t->predicate());
      if (ntPredicate)
      {
        if (CDA_objcmp(ntPredicate, triplePredicate))
        {
          // std::cout << "Predicate mismatch." << std::endl;
          continue;
        }
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(blank, triplePredicate,
                                       rdf_api::BlankNode);
        if (blank == NULL)
          continue;

        char* objid = blank->objid();
        bool known = (mXMLBlankNodeByNTripleBlankNode.find(objid) !=
                      mXMLBlankNodeByNTripleBlankNode.end());
        free(objid);

        if (known)
          continue;
      }

      RETURN_INTO_OBJREF(tripleObject, iface::rdf_api::Node, t->object());
      if (ntObject)
      {
        if (CDA_objcmp(ntObject, tripleObject))
        {
          // std::cout << "Objects known and different." << std::endl;
          continue;
        }
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(blank, tripleObject,
                                       rdf_api::BlankNode);
        if (blank == NULL)
        {
          // std::cout << "Object QI to blank node failed." << std::endl;
          continue;
        }

        char* objid = blank->objid();
        bool known = (mXMLBlankNodeByNTripleBlankNode.find(objid) !=
                      mXMLBlankNodeByNTripleBlankNode.end());
        free(objid);

        if (known)
        {
          // std::cout << "Object blank node already known." << std::endl;
          continue;
        }
      }
      // std::cout << "Performing blank equality tests..." << std::endl;

      // We have a plausible match so far, but we also need to check that
      // previously unseen blank nodes are the same if and only if they are the
      // same in the RDF/XML we are comparing with...

      if (!ntSubj && !ntPredicate)
      {
        if ((CDA_objcmp(xmlSubj, xmlPredicate) != 0) !=
            (CDA_objcmp(tripleSubj, triplePredicate) != 0))
          continue;
      }
      if (!ntSubj && !ntObject)
      {
        if ((CDA_objcmp(xmlSubj, xmlObject) != 0) !=
            (CDA_objcmp(tripleSubj, tripleObject) != 0))
          continue;
      }
      if (!ntPredicate && !ntObject)
      {
        if ((CDA_objcmp(xmlPredicate, xmlObject) != 0) !=
            (CDA_objcmp(triplePredicate, tripleObject) != 0))
          continue;
      }

      // If we get here, we have a match. But it might not be unique...
      t->add_ref();
      matches.push_back(t);
    }

    if (matches.size() == 0)
    {
      mErrorCount++;
      std::cerr << "RDF/XML Test " << mTestNo
                << ": Extraneous triple in XML parse results: ";
      if (ntSubj)
        std::cerr << ntSubj << " ";
      else
        std::cerr << "<blank> ";
      if (ntPredicate)
        std::cerr << ntPredicate << " ";
      else
        std::cerr << "<blank> ";
      if (ntObject)
        std::cerr << ntObject << " ." << std::endl;
      else
        std::cerr << "<blank> ." << std::endl;

      return false;
    }

    if (matches.size() > 1 && !allowAmbiguous)
      return true;

    iface::rdf_api::Triple* t = matches[0];
    RETURN_INTO_OBJREF(tripleSubj, iface::rdf_api::Resource, t->subject());
    RETURN_INTO_OBJREF(triplePredicate, iface::rdf_api::Resource, t->predicate());
    RETURN_INTO_OBJREF(tripleObject, iface::rdf_api::Node, t->object());

    // Save any blank node matches...
    iface::rdf_api::Node* tripleNodes[] = {tripleSubj, triplePredicate, tripleObject};
    iface::rdf_api::Node* xmlNodes[] = {xmlSubj, xmlPredicate, xmlObject};
    iface::rdf_api::Node* ntNodes[] = {ntSubj, ntPredicate, ntObject};
    
    for (uint32_t i = 0; i < 3; i++)
    {
      if (!ntNodes[i])
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tripleBN, tripleNodes[i], rdf_api::BlankNode);
        DECLARE_QUERY_INTERFACE_OBJREF(xmlBN, xmlNodes[i], rdf_api::BlankNode);
        
        char* objid = xmlBN->objid();
        bool done = true;
        if (
            mNTripleBlankNodeByXMLBlankNode.find(objid) ==
            mNTripleBlankNodeByXMLBlankNode.end()
           )
        {
          done = false;
          mNTripleBlankNodeByXMLBlankNode.insert
            (std::pair<std::string, iface::rdf_api::BlankNode*>(objid, tripleBN));
          tripleBN->add_ref();
        }
        free(objid);
        if (!done)
        {
          objid = tripleBN->objid();
          mXMLBlankNodeByNTripleBlankNode.insert
            (std::pair<std::string, iface::rdf_api::BlankNode*>(objid, xmlBN));
          xmlBN->add_ref();
          free(objid);
        }
      }
    }

    // Now we delete the triple...
    t->unassert();
    return false;
  }

  ObjRef<iface::rdf_api::DataSource> mDataSource;
  std::map<std::string, iface::rdf_api::BlankNode*> mNTripleBlankNodeByName;
  std::map<std::string, iface::rdf_api::BlankNode*> mNTripleBlankNodeByXMLBlankNode;
  std::map<std::string, iface::rdf_api::BlankNode*> mXMLBlankNodeByNTripleBlankNode;
  uint32_t mTestNo;
  int32_t mErrorCount;
};

void
RDFTest::testW3CSuite()
{
  RETURN_INTO_OBJREF(bs, iface::rdf_api::Bootstrap, CreateRDFBootstrap());
  RETURN_INTO_OBJREF(cdibs, CellML_DOMImplementationBase, CreateDOMImplementation());

  std::map<uint32_t, const wchar_t*> baseURIOverrides;
#define BASE_URI_OVERRIDE(n, uri) baseURIOverrides.insert(std::pair<uint32_t, const wchar_t*>(n, uri));
  BASE_URI_OVERRIDE(20, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-containers-syntax-vs-schema/test004.rdf");
  BASE_URI_OVERRIDE(21, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-containers-syntax-vs-schema/test006.rdf");
  BASE_URI_OVERRIDE(67, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-syntax-incomplete/test004.rdf");
  BASE_URI_OVERRIDE(69, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-not-id-and-resource-attr/test001.rdf");
  BASE_URI_OVERRIDE(71, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-not-id-and-resource-attr/test004.rdf");
  BASE_URI_OVERRIDE(72, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-not-id-and-resource-attr/test005.rdf");
  BASE_URI_OVERRIDE(75, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-ns-prefix-confusion/test0004.rdf");
  BASE_URI_OVERRIDE(80, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-ns-prefix-confusion/test0011.rdf");
  BASE_URI_OVERRIDE(81, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-ns-prefix-confusion/test0012.rdf");
  BASE_URI_OVERRIDE(82, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-ns-prefix-confusion/test0013.rdf");
  BASE_URI_OVERRIDE(83, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdf-ns-prefix-confusion/test0014.rdf");
  BASE_URI_OVERRIDE(85, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfs-no-cycles-in-subPropertyOf/test001");
  BASE_URI_OVERRIDE(90, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-empty-property-elements/test005.rdf");
  BASE_URI_OVERRIDE(91, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-empty-property-elements/test006.rdf");
  BASE_URI_OVERRIDE(96, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-empty-property-elements/test011.rdf");
  BASE_URI_OVERRIDE(97, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-empty-property-elements/test012.rdf");
  BASE_URI_OVERRIDE(108, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-difference-between-ID-and-about/test1.rdf");
  BASE_URI_OVERRIDE(109, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-difference-between-ID-and-about/test2.rdf");
  BASE_URI_OVERRIDE(110, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-difference-between-ID-and-about/test3.rdf");
  BASE_URI_OVERRIDE(112, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfms-xml-literal-namespaces/test001.rdf");
  BASE_URI_OVERRIDE(125, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/xmlbase/test014.rdf");
  BASE_URI_OVERRIDE(128, L"http://www.w3.org/2000/10/rdf-tests/rdfcore/rdfs-no-cycles-in-subClassOf/test001");

  std::set<uint32_t> exclusions;
#define DELIBERATE_EXCLUSION(x, reason) exclusions.insert(x);
  DELIBERATE_EXCLUSION(113, "Proper XC14N canonicalisation is not implemented yet because no one wants it.");
  DELIBERATE_EXCLUSION(126, "Proper XC14N canonicalisation is not implemented yet because no one wants it.");

  for (uint32_t i = 1; i < 131; i++)
  {
    if (exclusions.count(i) != 0)
      continue;

    char buf[30];
    sprintf(buf, "test%u", i);

    // Parse the triple file...
    NTripleFile ntf(bs, buf, i);

    std::wstring url(BASE_URI);
    wchar_t wbuf[30];
    any_swprintf(wbuf, 30, L"test%u.rdf", i);
    url += wbuf;

    std::wstring msg;

    RETURN_INTO_OBJREF(doc, iface::dom::Document, cdibs->loadDocument
                       (url.c_str(), msg));
    RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
    RETURN_INTO_OBJREF(ds, iface::rdf_api::DataSource, bs->createDataSource());

    std::map<uint32_t, const wchar_t*>::iterator uo = baseURIOverrides.find(i);
    const wchar_t* uriStr = url.c_str();
    if (uo != baseURIOverrides.end())
      uriStr = (*uo).second;
    bs->parseIntoDataSource(ds, de, uriStr);

    // Now, we are going to convert it back to a DOM representation...
    RETURN_INTO_OBJREF(doc2, iface::dom::Document, bs->getDOMForDataSource(ds, uriStr));

    // And parse it again...
    RETURN_INTO_OBJREF(de2, iface::dom::Element, doc2->documentElement());
    RETURN_INTO_OBJREF(ds2, iface::rdf_api::DataSource, bs->createDataSource());
    bs->parseIntoDataSource(ds2, de2, uriStr);

    RETURN_INTO_WSTRING(s, bs->serialiseDataSource(ds, uriStr));

    ntf.compareTriples(ds2);
  }
}

void
debug_PrintNode(iface::rdf_api::Node* aNode)
{
  DECLARE_QUERY_INTERFACE_OBJREF(ur, aNode, rdf_api::URIReference);
  if (ur != NULL)
  {
    RETURN_INTO_WSTRING(s, ur->URI());
    printf("<URI: %S> ", s.c_str());
    return;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(bn, aNode, rdf_api::BlankNode);
  if (bn != NULL)
  {
    printf("<BlankNode %08" PRIxPTR ">", reinterpret_cast<uintptr_t>(static_cast<iface::rdf_api::BlankNode*>(bn)));
    return;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(pl, aNode, rdf_api::PlainLiteral);
  if (pl != NULL)
  {
    RETURN_INTO_WSTRING(lf, pl->lexicalForm());
    RETURN_INTO_WSTRING(l, pl->language());
    printf("<PlainLiteral: %S language %S> ", lf.c_str(), l.c_str());
    return;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(tl, aNode, rdf_api::TypedLiteral);
  if (tl != NULL)
  {
    RETURN_INTO_WSTRING(lf, tl->lexicalForm());
    RETURN_INTO_WSTRING(l, tl->datatypeURI());
    printf("<TypedLiteral: %S type %S> ", lf.c_str(), l.c_str());
    return;
  }
}

void
RDFTest::testRDFAPIImplementation()
{
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader,
                     cb->modelLoader());
  RETURN_INTO_OBJREF(m, iface::cellml_api::Model,
                     ml->loadFromURL
                     (BASE_DIRECTORY_CELLML L"Ach_cascade_1995.xml"));
  CPPUNIT_ASSERT(m != NULL);
  RETURN_INTO_OBJREF(rr, iface::cellml_api::RDFRepresentation, m->getRDFRepresentation(L"http://www.cellml.org/RDF/API"));
  CPPUNIT_ASSERT(rr != NULL);
  DECLARE_QUERY_INTERFACE_OBJREF(rar, rr, rdf_api::RDFAPIRepresentation);
  CPPUNIT_ASSERT(rar != NULL);
  RETURN_INTO_OBJREF(ds, iface::rdf_api::DataSource, rar->source());
  CPPUNIT_ASSERT(ds != NULL);

#if 0
  {
    RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet, ds->getAllTriples());
    RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator, ts->enumerateTriples());
    while (true)
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
      if (t == NULL) break;
      RETURN_INTO_OBJREF(s, iface::rdf_api::Resource, t->subject());
      debug_PrintNode(s);
      RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
      debug_PrintNode(p);
      RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
      debug_PrintNode(o);
      printf("\n");
    }
  }
#endif

  RETURN_INTO_OBJREF(modur, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://www.example.org/repository/Ach_cascade_1995.xml"));
  CPPUNIT_ASSERT(modur != NULL);
  RETURN_INTO_OBJREF(dcpublisher, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://purl.org/dc/elements/1.1/publisher"));
  RETURN_INTO_OBJREF(dcp, iface::rdf_api::Triple, modur->getTripleOutOfByPredicate(dcpublisher));
  CPPUNIT_ASSERT(dcp != NULL);
  RETURN_INTO_OBJREF(dcpo, iface::rdf_api::Node, dcp->object());
  CPPUNIT_ASSERT(dcpo != NULL);
  DECLARE_QUERY_INTERFACE_OBJREF(pl, dcpo, rdf_api::PlainLiteral);
  RETURN_INTO_WSTRING(lf, pl->lexicalForm());
  CPPUNIT_ASSERT(lf == L"\n        The University of Auckland, Bioengineering Institute\n      ");
}

void
RDFTest::assertContainerContents
(
 iface::rdf_api::Container* aCT,
 const wchar_t** aExpect
)
{

  RETURN_INTO_OBJREF(ni, iface::rdf_api::NodeIterator, aCT->iterateChildren());
  while (true)
  {
    RETURN_INTO_OBJREF(n, iface::rdf_api::Node, ni->getNextNode());
    if (*aExpect == NULL)
    {
      CPPUNIT_ASSERT(n == NULL);
      return;
    }
    else
    {
      CPPUNIT_ASSERT(n);
    }

    DECLARE_QUERY_INTERFACE_OBJREF(pl, n, rdf_api::PlainLiteral);
    CPPUNIT_ASSERT(pl);
    RETURN_INTO_WSTRING(lf, pl->lexicalForm());
    // printf("lf: %S, expect: %S\n", lf.c_str(), *aExpect);
    CPPUNIT_ASSERT(lf == *aExpect);
    aExpect++;
  }
}

void
RDFTest::testContainerLibrary()
{
  RETURN_INTO_OBJREF(bs, iface::rdf_api::Bootstrap, CreateRDFBootstrap());
  RETURN_INTO_OBJREF(cdibs, CellML_DOMImplementationBase, CreateDOMImplementation());

  std::wstring url(BASE_URI);
  url += L"test131.rdf";

  std::wstring msg;
  
  RETURN_INTO_OBJREF(doc, iface::dom::Document, cdibs->loadDocument
                     (url.c_str(), msg));
  RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
  RETURN_INTO_OBJREF(ds, iface::rdf_api::DataSource, bs->createDataSource());
  
  const wchar_t* uriStr = url.c_str();
  bs->parseIntoDataSource(ds, de, uriStr);
  
//    /**
//     * Retrieves a 'Container' interface around this Resource.
//     */
//    readonly attribute Container correspondingContainer;
  RETURN_INTO_OBJREF(ur, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://example.org/thing"));
  RETURN_INTO_OBJREF(cc, iface::rdf_api::Container, ur->correspondingContainer());
  CPPUNIT_ASSERT(cc);

//    /**
//     * Retrieves the resource this container is built on.
//     */
//    readonly attribute Resource correspondingResource;
  RETURN_INTO_OBJREF(cr, iface::rdf_api::Resource, cc->correspondingResource());
  CPPUNIT_ASSERT(!CDA_objcmp(cr, ur));

//    /**
//     * Finds the type of container (or one of the types, if there are multiple
//     * types).
//     */
//    attribute Resource containerType;
  RETURN_INTO_OBJREF(ct, iface::rdf_api::Resource, cc->containerType());
  CPPUNIT_ASSERT(ct);
  RETURN_INTO_OBJREF(bag, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#Bag"));
  RETURN_INTO_OBJREF(seq, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#Seq"));  
  CPPUNIT_ASSERT(!CDA_objcmp(ct, bag));
  cc->containerType(seq);
  ct = already_AddRefd<iface::rdf_api::Resource>(cc->containerType());
  CPPUNIT_ASSERT(ct);
  CPPUNIT_ASSERT(!CDA_objcmp(ct, seq));

//
//    /**
//     * Iterates through all children in this container.
//     */
//    NodeIterator iterateChildren();
  // printf("Initial container iteration.\n");
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!", NULL
      };
    assertContainerContents(cc, expect);
  }

//
//    /**
//     * Appends a child to this container. The child will have the first free
//     * index, even if there is a higher index already in use.
//     */
//    void appendChild(in Node aChild);

  RETURN_INTO_OBJREF(word1, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"We", L"en"));
  cc->appendChild(word1);
  RETURN_INTO_OBJREF(word2, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"think", L"en"));
  cc->appendChild(word2);
  RETURN_INTO_OBJREF(word3, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"it", L"en"));
  cc->appendChild(word3);
  RETURN_INTO_OBJREF(word4, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"most", L"en"));
  cc->appendChild(word4);
  RETURN_INTO_OBJREF(word5, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"certainly", L"en"));
  cc->appendChild(word5);
  RETURN_INTO_OBJREF(word6, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"seems", L"en"));
  cc->appendChild(word6);
  RETURN_INTO_OBJREF(word7, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"to", L"en"));
  cc->appendChild(word7);
  RETURN_INTO_OBJREF(word8, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"work", L"en"));
  cc->appendChild(word8);
  RETURN_INTO_OBJREF(word9, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"!", L"en"));
  cc->appendChild(word9);
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!", L"We", L"think", L"it", L"most", L"certainly",
        L"seems", L"to", L"work", L"!", NULL
      };
    assertContainerContents(cc, expect);
  }

//
//    /**
//     * Removes a child. If aDoRenumbering is true, will also reduce the value of
//     * any existing indices above the one of the child being removed by one.
//     * Otherwise, leaves a gap in the numbering.
//     */
//    void removeChild(in Node aChild, in boolean aDoRenumbering);
  cc->removeChild(word1, false);
  cc->removeChild(word2, false);
  cc->removeChild(word3, false);
  cc->removeChild(word4, false);
  cc->removeChild(word5, false);
  // printf("Did removeChild(,false) calls\n");
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"seems", L"to", L"work", L"!", NULL
      };
    assertContainerContents(cc, expect);
  }
  // Also check it wasn't renumbered.
  RETURN_INTO_OBJREF(twelth, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#_12"));  
  RETURN_INTO_OBJREF(t12, iface::rdf_api::Triple, cr->getTripleOutOfByPredicate(twelth));
  CPPUNIT_ASSERT(t12);
  RETURN_INTO_OBJREF(t12o, iface::rdf_api::Node, t12->object());
  CPPUNIT_ASSERT(!CDA_objcmp(t12o, word6));

  cc->removeChild(word6, true);
  // printf("Did removeChild(,true) call\n");
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!", NULL
      };
    assertContainerContents(cc, expect);
  }
  t12 = already_AddRefd<iface::rdf_api::Triple>(cr->getTripleOutOfByPredicate(twelth));
  CPPUNIT_ASSERT(t12);
  t12o = already_AddRefd<iface::rdf_api::Node>(t12->object());
  CPPUNIT_ASSERT(!CDA_objcmp(t12o, word7));

//
//    /**
//     * Renumbers all indices in the container to be consecutive starting from 1.
//     */
//    void renumberContainer();
  cc->renumberContainer();
  // printf("Did renumberContainer() call\n");
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!", NULL
      };
    assertContainerContents(cc, expect);
  }
  RETURN_INTO_OBJREF(ninth, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#_9"));
  RETURN_INTO_OBJREF(t9, iface::rdf_api::Triple, cr->getTripleOutOfByPredicate(ninth));
  CPPUNIT_ASSERT(t9);
  RETURN_INTO_OBJREF(t9o, iface::rdf_api::Node, t9->object());
  CPPUNIT_ASSERT(!CDA_objcmp(t9o, word9));
 
//
//    /**
//     * Creates a merged container. This does not change the RDF, but merely
//     * creates a merged view.
//     * Merged containers return the same correspondingResource and type as
//     * this (not aContainer). Iterating children will iterate
//     * the children of this, and then the children of aContainer (with
//     * repetition possible). appendChild will have the same effect as performing
//     * appendChild on this. removeChild will have the same effect as performing
//     * removeChild on this and aContainer. renumberContainer will have the same
//     * effect as performing the renumber on each container separately. Merged
//     * containers can in turn be further merged.
//     */
//    Container mergeWith(in Container aContainer);
  RETURN_INTO_OBJREF(ur2, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://example.org/thing2"));
  RETURN_INTO_OBJREF(cc2, iface::rdf_api::Container, ur2->correspondingContainer());
  CPPUNIT_ASSERT(cc2);
  RETURN_INTO_OBJREF(ccm, iface::rdf_api::Container, cc->mergeWith(cc2));

  RETURN_INTO_OBJREF(crm, iface::rdf_api::Resource, ccm->correspondingResource());
  CPPUNIT_ASSERT(!CDA_objcmp(crm, ur));

  
  RETURN_INTO_OBJREF(ctm, iface::rdf_api::Resource, ccm->containerType());
  CPPUNIT_ASSERT(ctm);
  CPPUNIT_ASSERT(!CDA_objcmp(ctm, seq));
  ccm->containerType(bag);
  ctm = already_AddRefd<iface::rdf_api::Resource>(ccm->containerType());
  CPPUNIT_ASSERT(ctm);
  CPPUNIT_ASSERT(!CDA_objcmp(ctm, bag));

  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!",
        L"Testing", L"Things:", L"It's", L"Good", L"When It", L"Succeeds!",
        NULL
      };
    // printf("Merge container test 1\n");
    assertContainerContents(ccm, expect);
  }

  ccm->appendChild(word1);
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!", L"We",
        L"Testing", L"Things:", L"It's", L"Good", L"When It", L"Succeeds!",
        NULL
      };
    // printf("Merge container test 2\n");
    assertContainerContents(ccm, expect);
  }
  
  ccm->removeChild(word1, false);
  RETURN_INTO_OBJREF(succeeds, iface::rdf_api::PlainLiteral, ds->getPlainLiteral(L"Succeeds!", L"en"));
  ccm->removeChild(succeeds, true);
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!",
        L"Testing", L"Things:", L"It's", L"Good", L"When It",
        NULL
      };
    // printf("Merge container test 3\n");
    assertContainerContents(ccm, expect);
  }

  ccm->renumberContainer();
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!",
        L"Testing", L"Things:", L"It's", L"Good", L"When It",
        NULL
      };
    // printf("Merge container test 4\n");
    assertContainerContents(ccm, expect);
  }

//    /**
//     * Finds or makes a particular container out of 'this', with the
//     * specified predicate, and the specified type. If there are multiple
//     * existing containers, the returned container is a merged view of all
//     * containers. The underlying RDF is only changed if the container doesn't
//     * already exist, in which case:
//     *  A new triple (subject=this) (predicate=aPredicate) (object=new blank node 'A')
//     *   is made.
//     *  A new triple (subject=the new blank node 'A') (predicate=RDF type) (object=aContainerType)
//     *   is made.
//     *  The returned Container is the correspondingContainer around the new blank node 'A'.
//     */
//    Container findOrMakeContainer(in Resource aPredicate,
//                                  in Resource aContainerType);
  
  RETURN_INTO_OBJREF(ur3, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://example.org/myitem"));
  RETURN_INTO_OBJREF(cont, iface::rdf_api::Container, ur3->findOrMakeContainer(ur, bag));
  {
    const wchar_t* expect[] =
      {
        L"Hello", L"World", L"I", L"Hope", L"This",
        L"Works!",
        L"to", L"work", L"!", NULL
      };
    // printf("findOrMakeContainer test 1\n");
    assertContainerContents(cont, expect);
  }

  RETURN_INTO_OBJREF(ur4, iface::rdf_api::URIReference,
                     ds->getURIReference(L"http://example.org/myitem2"));
  RETURN_INTO_OBJREF(cont2, iface::rdf_api::Container, ur4->findOrMakeContainer(ur, bag));
  cont2->appendChild(word1);
  {
    // printf("findOrMakeContainer test 2\n");
    const wchar_t* expect[] =
      {
        L"We",
        NULL
      };
    assertContainerContents(cont2, expect);
  }
  RETURN_INTO_OBJREF(ct3, iface::rdf_api::Resource, cc->containerType());
  CPPUNIT_ASSERT(!CDA_objcmp(ct3, bag));
}
