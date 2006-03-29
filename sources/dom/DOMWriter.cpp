#include "Utilities.hxx"
#include <string>
#include "DOMWriter.hxx"

// These tables were adapted from the Mozilla code.
static const wchar_t* kEntities[] = {
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"&amp;", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"&lt;", L"", L"&gt;"
};
static const wchar_t* kAttrEntities[] = {
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"&quot;", L"", L"", L"", L"&amp;", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"", L"", L"", L"", L"", L"", L"", L"", L"", L"",
L"&lt;", L"", L"&gt;"
};

std::wstring
static TranslateEntities(const std::wstring& inp, bool isAttribute = false)
{
  std::wstring o;
  const wchar_t** table;

  if (isAttribute)
    table = kAttrEntities;
  else
    table = kEntities;

  uint32_t i, l = inp.length();
  wchar_t c;
  for (i = 0; i < l; i++)
  {
    c = inp[i];
    if (c > 62 || (table[c][0] == 0))
    {
      o += c;
      continue;
    }
    o += table[c];
  }

  return o;
}

DOMWriter::DOMWriter()
{
}

void
DOMWriter::writeNode(DOMNamespaceContext* dnc, iface::dom::Node* n, std::wstring& appendTo)
  throw(std::exception&)
{
  uint16_t nt = n->nodeType();

#define NODETYPE_CODE(nt, ntn) \
 case iface::dom::Node::nt##_NODE: \
  { \
    DECLARE_QUERY_INTERFACE_OBJREF(t, n, dom::ntn); \
    if (t == NULL) \
      throw iface::dom::DOMException(); \
    write##ntn(dnc, t, appendTo); \
  } \
  break;

  switch (nt)
  {
    NODETYPE_CODE(ELEMENT, Element)
    NODETYPE_CODE(ATTRIBUTE, Attr)
    NODETYPE_CODE(TEXT, Text)
    NODETYPE_CODE(CDATA_SECTION, CDATASection)
    NODETYPE_CODE(ENTITY_REFERENCE, EntityReference)
    NODETYPE_CODE(ENTITY, Entity)
    NODETYPE_CODE(PROCESSING_INSTRUCTION, ProcessingInstruction)
    NODETYPE_CODE(COMMENT, Comment)
    NODETYPE_CODE(DOCUMENT, Document)
    NODETYPE_CODE(DOCUMENT_TYPE, DocumentType)
    NODETYPE_CODE(DOCUMENT_FRAGMENT, DocumentFragment)
    NODETYPE_CODE(NOTATION, Notation)
  }
}

void
DOMWriter::writeElement(DOMNamespaceContext* parentContext,
                        iface::dom::Element* el, std::wstring& appendTo)
  throw(std::exception&)
{
  DOMNamespaceContext elementContext(parentContext);

  // Firstly scan for xmlns attributes...
  RETURN_INTO_OBJREF(elnl, iface::dom::NamedNodeMap, el->attributes());
  uint32_t l = elnl->length(), i;

  // See if this element has a prefix...
  RETURN_INTO_WSTRING(elpr, el->prefix());
  RETURN_INTO_WSTRING(elns, el->namespaceURI());
  if (elpr != L"")
  {
    // See if it is already defined...
    std::wstring existns = elementContext.findNamespaceForPrefix(elpr);
    if (existns == L"")
    {
      // They suggested a prefix, and it is available. Add it.
      elementContext.recordPrefix(elpr, elns);
    }
    else if (existns != elns)
    {
      // Can't use the suggested prefix. Do we need a prefix anyway?
      if (elementContext.getDefaultNamespace() == elns)
        elpr = L"";
      else
      {
        elpr = elementContext.findPrefixForNamespace(elns);
        if (elpr == L"")
        {
          // Can't use any existing prefix. Set the default namespace instead.
          elementContext.setDefaultNamespace(elns);
        }
        // otherwise, don't need to do anything as elpr is now suitably defined.
      }
    }
    // otherwise don't need to do anything, as prefix is already valid.
  }
  else
  {
    // Only do anything if the namespace is non-default...
    if (elementContext.getDefaultNamespace() != elns)
    {
      // We have an element in the non-default namespace, with no suggested
      // prefix. See if there is an existing prefix for the namespace...
      elpr = elementContext.findPrefixForNamespace(elns);
      if (elpr == L"")
      {
        // Set the default namespace to the one used for this element.
        elementContext.setDefaultNamespace(elns);
      }
      // otherwise, elpr is valid.
    }
    // otherwise elpr="" and the element is in the default namespace.
  }

  // Start the tag...
  appendTo += L"<";
  std::wstring qname;
  qname += elpr;
  if (elpr != L"")
    qname += L":";
  wchar_t* ln = el->localName();
  if (!wcscmp(ln, L""))
  {
    free(ln);
    ln = el->nodeName();
  }
  qname += ln;
  free(ln);

  appendTo += qname;

  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(atn, iface::dom::Node, elnl->item(i));
    if (atn == NULL)
      break;
    DECLARE_QUERY_INTERFACE_OBJREF(at, atn, dom::Attr);
    if (at == NULL)
      continue;

    // See if the attribute is in the XMLNS namespace...
    RETURN_INTO_WSTRING(nsURI, at->namespaceURI());
    RETURN_INTO_WSTRING(ln, at->localName());
    if (ln == L"")
    {
      wchar_t* nn = at->nodeName();
      ln = nn;
      free(nn);
    }
    if (nsURI == L"http://www.w3.org/2000/xmlns/" ||
        /* This is tecnically incorrect but needed in practice... */
        (nsURI == L"" && ln == L"xmlns"))
    {
      RETURN_INTO_WSTRING(value, at->value());
      if (ln == L"xmlns")
        elementContext.setDefaultNamespace(value);
      else
        elementContext.recordPrefix(ln, value);
    }
  }

  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(atn, iface::dom::Node, elnl->item(i));
    if (atn == NULL)
      break;
    DECLARE_QUERY_INTERFACE_OBJREF(at, atn, dom::Attr);
    if (at == NULL)
      continue;

    // See if the attribute is in the XMLNS namespace...
    RETURN_INTO_WSTRING(nsURI, at->namespaceURI());
    RETURN_INTO_WSTRING(ln, at->localName());
    if (ln == L"")
    {
      wchar_t* nn = at->nodeName();
      ln = nn;
      free(nn);
    }
    if (nsURI == L"http://www.w3.org/2000/xmlns/")
      continue;
    /* This is tecnically incorrect but needed in practice... */
    if (nsURI == L"" && ln == L"xmlns")
      continue;

    // If the attribute has a prefix, see if it is defined...
    RETURN_INTO_WSTRING(atpr, at->prefix());
    RETURN_INTO_WSTRING(atns, at->namespaceURI());

    // If no namespace(like most attributes will probably have) do nothing...
    if (atns == L"")
      continue;

    if (atpr != L"")
    {
      std::wstring existpr = elementContext.findNamespaceForPrefix(atpr);
      if (existpr == L"")
      {
        // They suggested a prefix, and it is available, so use it...
        elementContext.recordPrefix(atpr, atns);
      }
      else if (existpr != atns)
      {
        // Can't use the desired prefix, so see if we can find another suitable
        // prefix...
        atpr = elementContext.findPrefixForNamespace(atns);
        if (atpr == L"")
          elementContext.possiblyInventPrefix(atns);
      }
      // otherwise the prefix is correct and already defined.
    }
    else // no suggested prefix.
    {
      // if can't find a suitable prefix, invent one...
      atpr = elementContext.findPrefixForNamespace(atns);
      if (atpr == L"")
        elementContext.possiblyInventPrefix(atns);
    }
  }

  elementContext.resolveOrInventPrefixes();
  elementContext.writeXMLNS(appendTo);

  // Now once more through the attributes...
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(atn, iface::dom::Node, elnl->item(i));
    if (atn == NULL)
      break;
    DECLARE_QUERY_INTERFACE_OBJREF(at, atn, dom::Attr);
    if (at == NULL)
    {
      continue;
    }

    // See if the attribute is in the XMLNS namespace...
    RETURN_INTO_WSTRING(nsURI, at->namespaceURI());
    RETURN_INTO_WSTRING(ln, at->localName());
    if (ln == L"")
    {
      wchar_t* nn = at->nodeName();
      ln = nn;
      free(nn);
    }
    if (nsURI == L"http://www.w3.org/2000/xmlns/")
      continue;
    /* This is tecnically incorrect but needed in practice... */
    if (nsURI == L"" && ln == L"xmlns")
      continue;

    // Write out this attribute...
    writeAttr(&elementContext, at, appendTo);
  }

  RETURN_INTO_OBJREF(elcnl, iface::dom::NodeList, el->childNodes());  
  l = elcnl->length();

  if (l == 0)
  {
    appendTo += L"/>";
    return;
  }

  appendTo += L">";

  // This time, we write everything except the attributes...
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, elcnl->item(i));
    if (n == NULL)
      break;
    
    // Gdome doesn't put attributes on child nodes. I'm not sure if this is the
    // correct interpretation of the DOM specification.
    //iface::dom::Attr* at = dynamic_cast<iface::dom::Attr*>
    //  (n->query_interface("dom::Attr"));
    //if (at != NULL)
    //  continue;

    writeNode(&elementContext, n, appendTo);
  }

  appendTo += L"</" + qname + L">";
}

void
DOMWriter::writeAttr(DOMNamespaceContext* dnc, iface::dom::Attr* at, std::wstring& appendTo)
  throw(std::exception&)
{
  // Always put a space first...
  appendTo += L" ";
  
  // Next, we might need a prefix.
  RETURN_INTO_WSTRING(atpr, at->prefix());
  RETURN_INTO_WSTRING(atns, at->namespaceURI());

  // If no namespace(like most attributes will probably have) use empty prefix.
  if (atns == L"")
    atpr = L"";
  else if (atpr != L"")
  {
    std::wstring existpr = dnc->findNamespaceForPrefix(atpr);
    if (existpr != atns)
    {
      atpr = dnc->findPrefixForNamespace(atpr);
    }
    // otherwise we use the specified prefix.
  }
  else
    atpr = dnc->findPrefixForNamespace(atns);

  if (atpr != L"")
  {
    appendTo += atpr;
    appendTo += L":";
  }
  wchar_t* ln = at->localName();
  if (ln == NULL || !wcscmp(ln, L""))
  {
    free(ln);
    ln = at->nodeName();
  }
  appendTo += ln;
  free(ln);

  appendTo += L"=\"";
  wchar_t* value = at->value();
  appendTo += TranslateEntities(value, true);
  free(value);
  appendTo += L"\"";
}

void
DOMWriter::writeText(DOMNamespaceContext* dnc, iface::dom::Text* txt,
                     std::wstring& appendTo)
  throw(std::exception&)
{
  wchar_t* data = txt->data();
  appendTo += TranslateEntities(data);
  free(data);
}

void
DOMWriter::writeCDATASection
(DOMNamespaceContext* dnc, iface::dom::CDATASection* cds, std::wstring& appendTo)
  throw(std::exception&)
{
  appendTo += L"<![CDATA[";
  wchar_t* data = cds->data();
  appendTo += data;
  free(data);
  appendTo += L"]]>";
}

void
DOMWriter::writeEntity(DOMNamespaceContext* dnc, iface::dom::Entity* er,
                       std::wstring& appendTo)
  throw(std::exception&)
{
}

void
DOMWriter::writeEntityReference(DOMNamespaceContext* dnc,
                                iface::dom::EntityReference* er,
                                std::wstring& appendTo)
  throw(std::exception&)
{
}

void
DOMWriter::writeProcessingInstruction(DOMNamespaceContext* dnc,
                                      iface::dom::ProcessingInstruction* proci,
                                      std::wstring& appendTo)
  throw(std::exception&)
{
  appendTo += L"<?";
  wchar_t* target = proci->target();
  appendTo += target;
  free(target);

  wchar_t* data = proci->data();
  if (data[0] != 0)
  {
    appendTo += L" ";
    appendTo += data;
  }
  free(data);

  appendTo += L"?>";
}

void
DOMWriter::writeComment(DOMNamespaceContext* dnc, iface::dom::Comment* comment,
                        std::wstring& appendTo)
  throw(std::exception&)
{
  appendTo += L"<!--";
  wchar_t* data = comment->data();
  appendTo += data;
  free(data);
  appendTo += L"-->";
}

void
DOMWriter::writeDocument(DOMNamespaceContext* dnc,
                         iface::dom::Document* doc, std::wstring& appendTo)
  throw(std::exception&)
{
  // Firstly write the header...
  appendTo += L"<?xml version=\"1.0\"?>\n";

  RETURN_INTO_OBJREF(elnl, iface::dom::NodeList, doc->childNodes());
  uint32_t l = elnl->length(), i;

  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, elnl->item(i));
    if (n == NULL)
      break;
    
    writeNode(dnc, n, appendTo);
  }
}

void
DOMWriter::writeDocumentType(DOMNamespaceContext* dnc,
                             iface::dom::DocumentType* dt,
                             std::wstring& appendTo)
  throw(std::exception&)
{
}

void
DOMWriter::writeDocumentFragment(DOMNamespaceContext* dnc,
                                 iface::dom::DocumentFragment* df,
                                 std::wstring& appendTo)
  throw(std::exception)
{
  RETURN_INTO_OBJREF(elnl, iface::dom::NodeList, df->childNodes());
  uint32_t l = elnl->length(), i;

  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, elnl->item(i));
    if (n == NULL)
      break;
    
    writeNode(dnc, n, appendTo);
  }
}

void
DOMWriter::writeNotation(DOMNamespaceContext* dnc,
                         iface::dom::Notation* nt, std::wstring& appendTo)
  throw(std::exception)
{
}

DOMNamespaceContext::DOMNamespaceContext(DOMNamespaceContext* aParent)
  : mParent(aParent), mOverrideDefaultNamespace(false)
{
}

void
DOMNamespaceContext::setDefaultNamespace(const std::wstring& newNS)
{
  if (!mOverrideDefaultNamespace)
  {
    mOverrideDefaultNamespace = true;
    defaultNamespace = newNS;
  }
}

std::wstring
DOMNamespaceContext::getDefaultNamespace()
{
  if (mOverrideDefaultNamespace)
    return defaultNamespace;
  if (mParent == NULL)
    return L"";
  return mParent->getDefaultNamespace();
}

std::wstring
DOMNamespaceContext::findNamespaceForPrefix(const std::wstring& prefix)
{
  std::map<std::wstring,std::wstring>::iterator i;
  i = URIfromPrefix.find(prefix);
  if (i != URIfromPrefix.end())
    return (*i).second;

  if (mParent == NULL)
    return L"";
  return mParent->findNamespaceForPrefix(prefix);
}

std::wstring
DOMNamespaceContext::findPrefixForNamespace(const std::wstring& ns)
{
  std::map<std::wstring,std::wstring>::iterator i;
  i = PrefixfromURI.find(ns);
  if (i != PrefixfromURI.end())
    return (*i).second;

  if (mParent == NULL)
  {
    // Some prefixes are built in...
    if (ns == L"http://www.w3.org/XML/1998/namespace")
      return L"xml";
    return L"";
  }
  return mParent->findPrefixForNamespace(ns);
}

void
DOMNamespaceContext::recordPrefix(const std::wstring& prefix,
                                  const std::wstring& ns)
{
  URIfromPrefix.insert(std::pair<std::wstring,std::wstring>(prefix, ns));
  PrefixfromURI.insert(std::pair<std::wstring,std::wstring>(ns, prefix));
}

void
DOMNamespaceContext::possiblyInventPrefix(const std::wstring& prefix)
{
  NamespacesNeedingPrefixes.push_back(prefix);
}

void
DOMNamespaceContext::resolveOrInventPrefixes()
{
  while (!NamespacesNeedingPrefixes.empty())
  {
    std::wstring nnp = NamespacesNeedingPrefixes.back();
    NamespacesNeedingPrefixes.pop_back();

    // If nnp resolves, we are done...
    if (findPrefixForNamespace(nnp) != L"")
      continue;

    std::wstring suggestion;

    // We now need to invent a name. If we see a well-known namespace, we try
    // to use a common prefix first.
    if (nnp == L"http://www.cellml.org/cellml/1.0#" ||
        nnp == L"http://www.cellml.org/cellml/1.1#")
      suggestion = L"cellml";
    else if (nnp == L"http://www.cellml.org/metadata/1.0#")
      suggestion = L"cmeta";
    else if (nnp == L"http://www.w3.org/1998/Math/MathML")
      suggestion = L"mathml";
    else if (nnp == L"http://www.w3.org/1999/02/22-rdf-syntax-ns#")
      suggestion = L"rdf";
    else if (nnp == L"http://www.w3.org/1999/xlink")
      suggestion = L"xlink";
    else if (nnp == L"http://www.w3.org/1999/xhtml")
      suggestion = L"html";
    else
      suggestion = L"ns";
    
    std::wstring attempt = suggestion;
    uint32_t attemptCount = 0;
    while (findPrefixForNamespace(attempt) != L"")
    {
      attempt = suggestion;
      wchar_t buf[24];
      swprintf(buf, 24, L"%u", attemptCount++);
      attempt += buf;
    }
    // We found a unique prefix...
    recordPrefix(attempt, nnp);
  }
}

void
DOMNamespaceContext::writeXMLNS(std::wstring& appendTo)
{
  std::map<std::wstring,std::wstring>::iterator i;

  // If we have changed our default prefix, set that here...
  if (mOverrideDefaultNamespace)
    appendTo += L" xmlns=\"" + TranslateEntities(defaultNamespace, true) +
      L"\"";

  for (i = URIfromPrefix.begin(); i != URIfromPrefix.end(); i++)
  {
    appendTo += L" " + (*i).first + L":xmlns=\"" +
      TranslateEntities((*i).second, true) + L"\"";
  }
}
