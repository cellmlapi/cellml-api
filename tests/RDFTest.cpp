#include "RDFTest.hpp"
#include "IfaceRDF_APISPEC.hxx"
#include "DOMBootstrap.hxx"
#include "RDFBootstrap.hpp"
#include "Utilities.hxx"
#include <fstream>
#include <iostream>

#include "cda_config.h"

#ifndef BASE_DIRECTORY
#define BASE_DIRECTORY TESTDIR8 "/test_rdf/"
#ifdef WIN32
#define BASE_URI L"file:///" TESTDIR L"/test_rdf/"
#else
#define BASE_URI L"file://" TESTDIR L"/test_rdf/"
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
    std::string path(BASE_DIRECTORY);
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

    CPPUNIT_ASSERT_EQUAL(0, mErrorCount);
  }
private:
  std::string
  parsePart(const std::string& aStr, iface::rdf_api::Node** aNode)
  {
    *aNode = NULL;

    if (aStr[0] == '<')
    {
      uint32_t eos = aStr.find('>');
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
        uint32_t eos = aStr.find('>', pos);
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
    swprintf(wbuf, 30, L"test%u.rdf", i);
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
