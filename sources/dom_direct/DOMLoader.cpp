#include <inttypes.h>
#include "DOMImplementation.hpp"
#include <stdexcept>

#ifdef _WIN32
#define LIBXML_STATIC
#endif
#include <libxml/tree.h>

#ifdef _WIN32
#define swprintf _snwprintf
#endif

struct CDA_utf8_data_t
{
  uint8_t len;
  uint8_t mask;
};

CDA_utf8_data_t CDA_utf8_data[] =
{
#define D {1, 0x7F},
/* 0x00 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
/* 0x23 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
/* 0x46 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
/* 0x69 */ D D D D D D D D D D D D D D D D D D D D D D D
#undef D
#define D {0, 0x00},
/* 0x80 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
/* 0xA3 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
#undef D
#define D {2, 0x1F},
/* 0xC0 */ D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D D
#undef D
#define D {3, 0x0F},
/* 0xE0 */ D D D D D D D D D D D D D D D D
#undef D
#define D {4, 0x07},
/* 0xF0 */ D D D D D D D D
#undef D
#define D {0, 0x00},
/* 0xF8 */ D D D D D D D {0, 0x00}
};

static void
operator+=(std::wstring& data, const char* str)
{
  uint8_t c;
  wchar_t buf[512];
  const char* p = str;
  wchar_t* np = buf;

  if (str == NULL)
    return;

  while ((c = *p++))
  {
    uint8_t l = CDA_utf8_data[c].len;

    if (((uint32_t)(np - buf)) > ((sizeof(buf)/sizeof(wchar_t)) - 4))
    {
      *np = 0;
      data += buf;
      np = buf;
    }

#ifndef WCHAR_T_CONSTANT_WIDTH
    if (l == 4)
    {
      uint8_t c2 = (uint8_t)*p++;
      uint8_t c3 = (uint8_t)*p++;
      uint8_t w = ((c << 2) | ((c2 >> 4) & 3)) - 1;
      *np++ = (0xD800 | (w << 6) | ((c2 << 2) & 0x3A) | ((c3 >> 4) & 0x3));
      uint8_t c4 = (uint8_t)*p++;
      *np = (0xDC00 | ((c3 << 6) & 0x3A0) | (c4 & 0x3F));
    }
    else
    {
#endif
      *np = CDA_utf8_data[c].mask & c;
      while (--l)
      {
        *np <<= 6;
        *np |= ((*p++) & 0x3F);
      }
#ifndef WCHAR_T_CONSTANT_WIDTH
    }
#endif

    np++;
  }
  *np = 0;
  data += buf;
}

char*
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
    else if (c <= 0xFFFF)
      len += 3;
    else
      len += 4;
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
#ifdef WCHAR_T_CONSTANT_WIDTH
         if (c <= 0xFFFF)
#endif
    {
      *np++ = (char)(0xD0 | ((c >> 12) & 0xF));
      *np++ = (char)(0x80 | ((c >> 6) & 0x3F));
      *np++ = (char)(0x80 | (c & 0x3F));
    }
#ifdef WCHAR_T_CONSTANT_WIDTH
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

CDA_Node*
WrapXML2Node
(
 CDA_Document* doc,
 xmlNode* x2node
)
{
  ObjRef<CDA_Document> docR;
  ObjRef<CDA_Node> n;

  std::wstring localName, namespaceURI;

  if (x2node->name != NULL)
  {
    localName += (const char*)x2node->name;
  }

  bool setNS = false;

  bool isNodeStructure = false;
  switch (x2node->type)
  {
  case XML_ELEMENT_NODE:
    {
      setNS = true;
      if (x2node->ns && x2node->ns->href)
        namespaceURI += (const char*)x2node->ns->href;
      isNodeStructure = true;
      RETURN_INTO_OBJREF(el, CDA_Element, CDA_NewElement(doc, namespaceURI.c_str(),
                                                         localName.c_str()));
      n = el;
      xmlAttr* at;
      for (at = x2node->properties; at != NULL; at = at->next)
      {
        RETURN_INTO_OBJREF(cattr, CDA_Attr, new CDA_Attr(doc));

        std::wstring nsURI, prefix, name;
        if (at->ns && at->ns->href)
        {
          nsURI += (const char*)at->ns->href;
          prefix += (const char*)at->ns->prefix;
        }
        name += (const char*)at->name;

        cattr->mLocalName = name;
        if (prefix != L"")
          cattr->mNodeName = prefix + L":" + name;
        else
          cattr->mNodeName = name;
        cattr->mNamespaceURI = nsURI;

        xmlNode* tmp;
        for (tmp = at->children; tmp != NULL; tmp = tmp->next)
        {
          RETURN_INTO_OBJREF(nw, CDA_Node, WrapXML2Node(doc, tmp));
          if (nw != NULL)
            cattr->appendChild(nw)->release_ref();
        }
        // Finally, fetch the value as a single string...
        char *txt;
        if (at->ns != NULL && at->ns->href != NULL)
          txt = (char*)xmlGetNsProp(at->parent, at->name, at->ns->href);
        else
          txt = (char*)xmlGetNoNsProp(at->parent, at->name);

        cattr->mNodeValue += txt;
        xmlFree(txt);

        el->attributeMapNS.insert
          (
           std::pair<std::pair<std::wstring, std::wstring>, CDA_Attr*>
           (std::pair<std::wstring, std::wstring>(nsURI, name), cattr)
          );
        el->attributeMap.insert(std::pair<std::wstring, CDA_Attr*>
                                (cattr->mNodeName, cattr));
        el->insertBeforePrivate(cattr, NULL)->release_ref();
      }
    }
    break;
  case XML_TEXT_NODE:
    {
      isNodeStructure = true;
      CDA_Text* txt = new CDA_Text(doc);
      n = already_AddRefd<CDA_Text>(txt);
    }
    break;
  case XML_CDATA_SECTION_NODE:
    {
      isNodeStructure = true;
      n = already_AddRefd<CDA_CDATASection>(new CDA_CDATASection(doc));
    }
    break;
  case XML_ENTITY_REF_NODE:
    {
      isNodeStructure = true;
      n = already_AddRefd<CDA_EntityReference>(new CDA_EntityReference(doc));
    }
    break;
  case XML_ENTITY_NODE:
    {
      xmlEntity* se = (xmlEntity*)x2node;
      std::wstring notName, pubID, sysID;
      notName += (const char*)se->name;
      pubID += (const char*)se->ExternalID;
      sysID += (const char*)se->SystemID;
      n = already_AddRefd<CDA_Entity>
          (new CDA_Entity(doc, pubID, sysID, notName));
    }
    break;
  case XML_PI_NODE:
    {
      isNodeStructure = true;
      n =
        already_AddRefd<CDA_ProcessingInstruction>
        (new CDA_ProcessingInstruction(doc, L"", L""));
    }
    break;
  case XML_COMMENT_NODE:
    {
      isNodeStructure = true;
      n = already_AddRefd<CDA_Comment>(new CDA_Comment(doc));
    }
    break;

  case XML_DOCUMENT_NODE:
  case XML_DOCB_DOCUMENT_NODE:
  case XML_HTML_DOCUMENT_NODE:
    {
      // We need to decide what type of document before we add the children to
      // it(e.g. for MathML support), so do a preliminary scan...
      xmlNode* tnode = x2node->children;
      std::wstring rootNS;
      for (; tnode; tnode = tnode->next)
      {
        if (tnode->type != XML_ELEMENT_NODE)
          continue;
        if (tnode->ns == NULL || tnode->ns->href == NULL)
          break;
        rootNS += (const char*)tnode->ns->href;
        break;
      }

      docR = already_AddRefd<CDA_Document>(CDA_NewDocument(rootNS.c_str()));
      doc = docR;
      xmlDoc* sd = (xmlDoc*)x2node;
      xmlDtd* dt = sd->intSubset;
      if (dt != NULL)
      {
        RETURN_INTO_OBJREF(dtwrap, CDA_Node, WrapXML2Node(doc, (xmlNode*)dt));
        if (dtwrap != NULL)
          doc->insertBeforePrivate(dtwrap, NULL)->release_ref();
      }
      n = doc;
    }
    break;

  case XML_DOCUMENT_TYPE_NODE:
    {
      xmlDtd* dt = (xmlDtd*)x2node;
      std::wstring pubID, sysID;
      pubID += (const char*)dt->ExternalID;
      sysID += (const char*)dt->SystemID;
      n = already_AddRefd<CDA_DocumentType>
          (new CDA_DocumentType(doc, localName, pubID, sysID));
    }
    break;

  case XML_DOCUMENT_FRAG_NODE:
    {
      isNodeStructure = true;
      n = already_AddRefd<CDA_DocumentFragment>(new CDA_DocumentFragment(doc));
    }
    break;

    // Handled as part of element...
  case XML_ATTRIBUTE_NODE:
    // Handled as part of document type...
  case XML_NOTATION_NODE:
    // The remaining types are not part of the standard DOM...
  case XML_DTD_NODE:
  case XML_ELEMENT_DECL:
  case XML_ATTRIBUTE_DECL:
  case XML_ENTITY_DECL:
  case XML_NAMESPACE_DECL:
  case XML_XINCLUDE_START:
  case XML_XINCLUDE_END:
    return NULL;
  }

  if (isNodeStructure)
  {
    if (!setNS)
    {
      if (x2node->ns && x2node->ns->href)
      {
        namespaceURI += (const char*)x2node->ns->href;
      }
    }

    n->mLocalName = localName;
    if (x2node->ns && x2node->ns->href)
    {
      n->mNamespaceURI = namespaceURI;
    }
    if (x2node->ns && x2node->ns->prefix)
    {
      std::wstring prefix;
      prefix += (const char*)x2node->ns->prefix;
      n->mNodeName = prefix + L":" + localName;
    }
    else
      n->mNodeName = localName;

    if (x2node->content)
    {
      std::wstring content;
      content += (const char*)x2node->content;
      n->mNodeValue = content;
    }
  }

  // Now visit children...
  x2node = x2node->children;
  while (x2node)
  {
    RETURN_INTO_OBJREF(wn, CDA_Node, WrapXML2Node(doc, x2node));
    if (wn != NULL)
      n->insertBeforePrivate(wn, NULL)->release_ref();
    x2node = x2node->next;
  }

  n->add_ref();
  return n;
}

void
CDA_DOMImplementation::ProcessContextError(std::wstring& aErrorMessage,
                                           xmlParserCtxtPtr ctxt)
{
  wchar_t buf[20];

  xmlErrorPtr err = xmlCtxtGetLastError(ctxt);
  std::wstring fname, msg;
  if (err == NULL)
  {
    xmlFreeParserCtxt(ctxt);
    aErrorMessage = L"Could not fetch the error message.";
    return;
  }
  if (err->file)
  {
    std::wstring fname;
    fname += (const char*)err->file;
  }
  if (err->message)
  {
    std::wstring msg;
    msg += (const char*)err->message;
  }
  
  switch (err->code)
  {
  case XML_ERR_INTERNAL_ERROR:
    aErrorMessage = L"internalerror";
    break;
  case XML_ERR_NO_MEMORY:
    aErrorMessage = L"nomemory";
    break;
  case XML_ERR_DOCUMENT_START:
  case XML_ERR_DOCUMENT_EMPTY:
  case XML_ERR_DOCUMENT_END:
  case XML_ERR_INVALID_HEX_CHARREF:
  case XML_ERR_INVALID_DEC_CHARREF:
  case XML_ERR_INVALID_CHARREF:
  case XML_ERR_INVALID_CHAR:
  case XML_ERR_CHARREF_AT_EOF:
  case XML_ERR_CHARREF_IN_PROLOG:
  case XML_ERR_CHARREF_IN_EPILOG:
  case XML_ERR_CHARREF_IN_DTD:
  case XML_ERR_ENTITYREF_AT_EOF:
  case XML_ERR_ENTITYREF_IN_PROLOG:
  case XML_ERR_ENTITYREF_IN_EPILOG:
  case XML_ERR_ENTITYREF_IN_DTD:
  case XML_ERR_PEREF_AT_EOF:
  case XML_ERR_PEREF_IN_PROLOG:
  case XML_ERR_PEREF_IN_EPILOG:
  case XML_ERR_PEREF_IN_INT_SUBSET:
  case XML_ERR_ENTITYREF_NO_NAME:
  case XML_ERR_ENTITYREF_SEMICOL_MISSING:
  case XML_ERR_PEREF_NO_NAME:
  case XML_ERR_PEREF_SEMICOL_MISSING:
  case XML_ERR_UNDECLARED_ENTITY:
  case XML_WAR_UNDECLARED_ENTITY:
  case XML_ERR_UNPARSED_ENTITY:
  case XML_ERR_ENTITY_IS_EXTERNAL:
  case XML_ERR_ENTITY_IS_PARAMETER:
  case XML_ERR_UNKNOWN_ENCODING:
  case XML_ERR_UNSUPPORTED_ENCODING:
  case XML_ERR_STRING_NOT_STARTED:
  case XML_ERR_STRING_NOT_CLOSED:
  case XML_ERR_NS_DECL_ERROR:
  case XML_ERR_ENTITY_NOT_STARTED:
  case XML_ERR_ENTITY_NOT_FINISHED:
  case XML_ERR_LT_IN_ATTRIBUTE:
  case XML_ERR_ATTRIBUTE_NOT_STARTED:
  case XML_ERR_ATTRIBUTE_NOT_FINISHED:
  case XML_ERR_ATTRIBUTE_WITHOUT_VALUE:
  case XML_ERR_ATTRIBUTE_REDEFINED:
  case XML_ERR_LITERAL_NOT_STARTED:
  case XML_ERR_LITERAL_NOT_FINISHED:
  case XML_ERR_COMMENT_NOT_FINISHED:
  case XML_ERR_PI_NOT_STARTED:
  case XML_ERR_PI_NOT_FINISHED:
  case XML_ERR_NOTATION_NOT_STARTED:
  case XML_ERR_NOTATION_NOT_FINISHED:
  case XML_ERR_ATTLIST_NOT_STARTED:
  case XML_ERR_ATTLIST_NOT_FINISHED:
  case XML_ERR_MIXED_NOT_STARTED:
  case XML_ERR_MIXED_NOT_FINISHED:
  case XML_ERR_ELEMCONTENT_NOT_STARTED:
  case XML_ERR_ELEMCONTENT_NOT_FINISHED:
  case XML_ERR_XMLDECL_NOT_STARTED:
  case XML_ERR_XMLDECL_NOT_FINISHED:
  case XML_ERR_CONDSEC_NOT_STARTED:
  case XML_ERR_CONDSEC_NOT_FINISHED:
  case XML_ERR_EXT_SUBSET_NOT_FINISHED:
  case XML_ERR_DOCTYPE_NOT_FINISHED:
  case XML_ERR_MISPLACED_CDATA_END:
  case XML_ERR_CDATA_NOT_FINISHED:
  case XML_ERR_RESERVED_XML_NAME:
  case XML_ERR_SPACE_REQUIRED:
  case XML_ERR_SEPARATOR_REQUIRED:
  case XML_ERR_NMTOKEN_REQUIRED:
  case XML_ERR_NAME_REQUIRED:
  case XML_ERR_PCDATA_REQUIRED:
  case XML_ERR_URI_REQUIRED:
  case XML_ERR_PUBID_REQUIRED:
  case XML_ERR_LT_REQUIRED:
  case XML_ERR_GT_REQUIRED:
  case XML_ERR_LTSLASH_REQUIRED:
  case XML_ERR_EQUAL_REQUIRED:
  case XML_ERR_TAG_NAME_MISMATCH:
  case XML_ERR_TAG_NOT_FINISHED:
  case XML_ERR_STANDALONE_VALUE:
  case XML_ERR_ENCODING_NAME:
  case XML_ERR_HYPHEN_IN_COMMENT:
  case XML_ERR_INVALID_ENCODING:
  case XML_ERR_EXT_ENTITY_STANDALONE:
  case XML_ERR_CONDSEC_INVALID:
  case XML_ERR_VALUE_REQUIRED:
  case XML_ERR_NOT_WELL_BALANCED:
  case XML_ERR_EXTRA_CONTENT:
  case XML_ERR_ENTITY_CHAR_ERROR:
  case XML_ERR_ENTITY_PE_INTERNAL:
  case XML_ERR_ENTITY_LOOP:
  case XML_ERR_ENTITY_BOUNDARY:
#if LIBXML_VERSION > 20403
  case XML_WAR_CATALOG_PI:
#endif
#if LIBXML_VERSION > 20404
  case XML_ERR_NO_DTD:
#endif
  case XML_ERR_CONDSEC_INVALID_KEYWORD:
  case XML_ERR_VERSION_MISSING:
  case XML_WAR_UNKNOWN_VERSION:
  case XML_WAR_LANG_VALUE:
  case XML_WAR_NS_URI:
  case XML_WAR_NS_URI_RELATIVE:
  case XML_ERR_MISSING_ENCODING:
  case XML_WAR_SPACE_VALUE:
  case XML_ERR_NOT_STANDALONE:
  case XML_ERR_ENTITY_PROCESSING:
  case XML_ERR_NOTATION_PROCESSING:
  case XML_WAR_NS_COLUMN:
  case XML_WAR_ENTITY_REDEFINED:
    // case XML_NS_ERR_XML:
#if LIBXML_VERSION > 20600
  case XML_NS_ERR_UNDEFINED_NAMESPACE:
  case XML_NS_ERR_QNAME:
  case XML_NS_ERR_ATTRIBUTE_REDEFINED:
#endif
  case XML_NS_ERR_EMPTY:
    // case XML_DTD_ATTRIBUTE:
  case XML_DTD_ATTRIBUTE_REDEFINED:
  case XML_DTD_ATTRIBUTE_VALUE:
  case XML_DTD_CONTENT_ERROR:
  case XML_DTD_CONTENT_MODEL:
  case XML_DTD_CONTENT_NOT_DETERMINIST:
  case XML_DTD_DIFFERENT_PREFIX:
  case XML_DTD_ELEM_DEFAULT_NAMESPACE:
  case XML_DTD_ELEM_NAMESPACE:
  case XML_DTD_ELEM_REDEFINED:
  case XML_DTD_EMPTY_NOTATION:
  case XML_DTD_ENTITY_TYPE:
  case XML_DTD_ID_FIXED:
  case XML_DTD_ID_REDEFINED:
  case XML_DTD_ID_SUBSET:
  case XML_DTD_INVALID_CHILD:
  case XML_DTD_INVALID_DEFAULT:
  case XML_DTD_LOAD_ERROR:
  case XML_DTD_MISSING_ATTRIBUTE:
  case XML_DTD_MIXED_CORRUPT:
  case XML_DTD_MULTIPLE_ID:
  case XML_DTD_NO_DOC:
  case XML_DTD_NO_DTD:
  case XML_DTD_NO_ELEM_NAME:
  case XML_DTD_NO_PREFIX:
  case XML_DTD_NO_ROOT:
  case XML_DTD_NOTATION_REDEFINED:
  case XML_DTD_NOTATION_VALUE:
  case XML_DTD_NOT_EMPTY:
  case XML_DTD_NOT_PCDATA:
  case XML_DTD_NOT_STANDALONE:
  case XML_DTD_ROOT_NAME:
  case XML_DTD_STANDALONE_WHITE_SPACE:
  case XML_DTD_UNKNOWN_ATTRIBUTE:
  case XML_DTD_UNKNOWN_ELEM:
  case XML_DTD_UNKNOWN_ENTITY:
  case XML_DTD_UNKNOWN_ID:
  case XML_DTD_UNKNOWN_NOTATION:
  case XML_DTD_STANDALONE_DEFAULTED:
  case XML_DTD_XMLID_VALUE:
  case XML_DTD_XMLID_TYPE:
  case XML_XINCLUDE_PARSE_VALUE:
  case XML_XINCLUDE_ENTITY_DEF_MISMATCH:
  case XML_XINCLUDE_NO_HREF:
  case XML_XINCLUDE_NO_FALLBACK:
  case XML_XINCLUDE_HREF_URI:
  case XML_XINCLUDE_TEXT_FRAGMENT:
  case XML_XINCLUDE_TEXT_DOCUMENT:
  case XML_XINCLUDE_INVALID_CHAR:
  case XML_XINCLUDE_BUILD_FAILED:
  case XML_XINCLUDE_UNKNOWN_ENCODING:
  case XML_XINCLUDE_MULTIPLE_ROOT:
  case XML_XINCLUDE_XPTR_FAILED:
  case XML_XINCLUDE_XPTR_RESULT:
  case XML_XINCLUDE_INCLUDE_IN_INCLUDE:
  case XML_XINCLUDE_FALLBACKS_IN_INCLUDE:
  case XML_XINCLUDE_FALLBACK_NOT_IN_INCLUDE:
  case XML_XINCLUDE_DEPRECATED_NS:
  case XML_XINCLUDE_FRAGMENT_ID:
    // case XML_CATALOG_MISSING:
  case XML_CATALOG_ENTRY_BROKEN:
  case XML_CATALOG_PREFER_VALUE:
  case XML_CATALOG_NOT_CATALOG:
  case XML_CATALOG_RECURSION:
    // case XML_SCHEMAP_PREFIX:
  case XML_SCHEMAP_ATTRFORMDEFAULT_VALUE:
  case XML_SCHEMAP_ATTRGRP_NONAME_NOREF:
  case XML_SCHEMAP_ATTR_NONAME_NOREF:
  case XML_SCHEMAP_COMPLEXTYPE_NONAME_NOREF:
  case XML_SCHEMAP_ELEMFORMDEFAULT_VALUE:
  case XML_SCHEMAP_ELEM_NONAME_NOREF:
  case XML_SCHEMAP_EXTENSION_NO_BASE:
  case XML_SCHEMAP_FACET_NO_VALUE:
  case XML_SCHEMAP_FAILED_BUILD_IMPORT:
  case XML_SCHEMAP_GROUP_NONAME_NOREF:
  case XML_SCHEMAP_IMPORT_NAMESPACE_NOT_URI:
  case XML_SCHEMAP_IMPORT_REDEFINE_NSNAME:
  case XML_SCHEMAP_IMPORT_SCHEMA_NOT_URI:
  case XML_SCHEMAP_INVALID_BOOLEAN:
  case XML_SCHEMAP_INVALID_ENUM:
  case XML_SCHEMAP_INVALID_FACET:
  case XML_SCHEMAP_INVALID_FACET_VALUE:
  case XML_SCHEMAP_INVALID_MAXOCCURS:
  case XML_SCHEMAP_INVALID_MINOCCURS:
  case XML_SCHEMAP_INVALID_REF_AND_SUBTYPE:
  case XML_SCHEMAP_INVALID_WHITE_SPACE:
  case XML_SCHEMAP_NOATTR_NOREF:
  case XML_SCHEMAP_NOTATION_NO_NAME:
  case XML_SCHEMAP_NOTYPE_NOREF:
  case XML_SCHEMAP_REF_AND_SUBTYPE:
  case XML_SCHEMAP_RESTRICTION_NONAME_NOREF:
  case XML_SCHEMAP_SIMPLETYPE_NONAME:
  case XML_SCHEMAP_TYPE_AND_SUBTYPE:
  case XML_SCHEMAP_UNKNOWN_ALL_CHILD:
  case XML_SCHEMAP_UNKNOWN_ANYATTRIBUTE_CHILD:
  case XML_SCHEMAP_UNKNOWN_ATTR_CHILD:
  case XML_SCHEMAP_UNKNOWN_ATTRGRP_CHILD:
  case XML_SCHEMAP_UNKNOWN_ATTRIBUTE_GROUP:
  case XML_SCHEMAP_UNKNOWN_BASE_TYPE:
  case XML_SCHEMAP_UNKNOWN_CHOICE_CHILD:
  case XML_SCHEMAP_UNKNOWN_COMPLEXCONTENT_CHILD:
  case XML_SCHEMAP_UNKNOWN_COMPLEXTYPE_CHILD:
  case XML_SCHEMAP_UNKNOWN_ELEM_CHILD:
  case XML_SCHEMAP_UNKNOWN_EXTENSION_CHILD:
  case XML_SCHEMAP_UNKNOWN_FACET_CHILD:
  case XML_SCHEMAP_UNKNOWN_FACET_TYPE:
  case XML_SCHEMAP_UNKNOWN_GROUP_CHILD:
  case XML_SCHEMAP_UNKNOWN_IMPORT_CHILD:
  case XML_SCHEMAP_UNKNOWN_LIST_CHILD:
  case XML_SCHEMAP_UNKNOWN_NOTATION_CHILD:
  case XML_SCHEMAP_UNKNOWN_PROCESSCONTENT_CHILD:
  case XML_SCHEMAP_UNKNOWN_REF:
  case XML_SCHEMAP_UNKNOWN_RESTRICTION_CHILD:
  case XML_SCHEMAP_UNKNOWN_SCHEMAS_CHILD:
  case XML_SCHEMAP_UNKNOWN_SEQUENCE_CHILD:
  case XML_SCHEMAP_UNKNOWN_SIMPLECONTENT_CHILD:
  case XML_SCHEMAP_UNKNOWN_SIMPLETYPE_CHILD:
  case XML_SCHEMAP_UNKNOWN_TYPE:
  case XML_SCHEMAP_UNKNOWN_UNION_CHILD:
  case XML_SCHEMAP_ELEM_DEFAULT_FIXED:
  case XML_SCHEMAP_REGEXP_INVALID:
  case XML_SCHEMAP_FAILED_LOAD:
  case XML_SCHEMAP_NOTHING_TO_PARSE:
  case XML_SCHEMAP_NOROOT:
  case XML_SCHEMAP_REDEFINED_GROUP:
  case XML_SCHEMAP_REDEFINED_TYPE:
  case XML_SCHEMAP_REDEFINED_ELEMENT:
  case XML_SCHEMAP_REDEFINED_ATTRGROUP:
  case XML_SCHEMAP_REDEFINED_ATTR:
  case XML_SCHEMAP_REDEFINED_NOTATION:
  case XML_SCHEMAP_FAILED_PARSE:
  case XML_SCHEMAP_UNKNOWN_PREFIX:
  case XML_SCHEMAP_DEF_AND_PREFIX:
  case XML_SCHEMAP_UNKNOWN_INCLUDE_CHILD:
  case XML_SCHEMAP_INCLUDE_SCHEMA_NOT_URI:
  case XML_SCHEMAP_INCLUDE_SCHEMA_NO_URI:
  case XML_SCHEMAP_NOT_SCHEMA:
  case XML_SCHEMAP_UNKNOWN_MEMBER_TYPE:
  case XML_SCHEMAP_INVALID_ATTR_USE:
  case XML_SCHEMAP_RECURSIVE:
  case XML_SCHEMAP_SUPERNUMEROUS_LIST_ITEM_TYPE:
  case XML_SCHEMAP_INVALID_ATTR_COMBINATION:
  case XML_SCHEMAP_INVALID_ATTR_INLINE_COMBINATION:
  case XML_SCHEMAP_MISSING_SIMPLETYPE_CHILD:
  case XML_SCHEMAP_INVALID_ATTR_NAME:
  case XML_SCHEMAP_REF_AND_CONTENT:
  case XML_SCHEMAP_CT_PROPS_CORRECT_1:
  case XML_SCHEMAP_CT_PROPS_CORRECT_2:
  case XML_SCHEMAP_CT_PROPS_CORRECT_3:
  case XML_SCHEMAP_CT_PROPS_CORRECT_4:
  case XML_SCHEMAP_CT_PROPS_CORRECT_5:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_1:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_1:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_2:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_2:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_3:
  case XML_SCHEMAP_WILDCARD_INVALID_NS_MEMBER:
  case XML_SCHEMAP_INTERSECTION_NOT_EXPRESSIBLE:
  case XML_SCHEMAP_UNION_NOT_EXPRESSIBLE:
  case XML_SCHEMAP_SRC_IMPORT_3_1:
  case XML_SCHEMAP_SRC_IMPORT_3_2:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_1:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_2:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_4_3:
  case XML_SCHEMAP_COS_CT_EXTENDS_1_3:
    // case XML_SCHEMAV:
  case XML_SCHEMAV_UNDECLAREDELEM:
  case XML_SCHEMAV_NOTTOPLEVEL:
  case XML_SCHEMAV_MISSING:
  case XML_SCHEMAV_WRONGELEM:
  case XML_SCHEMAV_NOTYPE:
  case XML_SCHEMAV_NOROLLBACK:
  case XML_SCHEMAV_ISABSTRACT:
  case XML_SCHEMAV_NOTEMPTY:
  case XML_SCHEMAV_ELEMCONT:
  case XML_SCHEMAV_HAVEDEFAULT:
  case XML_SCHEMAV_NOTNILLABLE:
  case XML_SCHEMAV_EXTRACONTENT:
  case XML_SCHEMAV_INVALIDATTR:
  case XML_SCHEMAV_INVALIDELEM:
  case XML_SCHEMAV_NOTDETERMINIST:
  case XML_SCHEMAV_CONSTRUCT:
  case XML_SCHEMAV_INTERNAL:
  case XML_SCHEMAV_NOTSIMPLE:
  case XML_SCHEMAV_ATTRUNKNOWN:
  case XML_SCHEMAV_ATTRINVALID:
  case XML_SCHEMAV_VALUE:
  case XML_SCHEMAV_FACET:
  case XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_1:
  case XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_2:
  case XML_SCHEMAV_CVC_DATATYPE_VALID_1_2_3:
  case XML_SCHEMAV_CVC_TYPE_3_1_1:
  case XML_SCHEMAV_CVC_TYPE_3_1_2:
  case XML_SCHEMAV_CVC_FACET_VALID:
  case XML_SCHEMAV_CVC_LENGTH_VALID:
  case XML_SCHEMAV_CVC_MINLENGTH_VALID:
  case XML_SCHEMAV_CVC_MAXLENGTH_VALID:
  case XML_SCHEMAV_CVC_MININCLUSIVE_VALID:
  case XML_SCHEMAV_CVC_MAXINCLUSIVE_VALID:
  case XML_SCHEMAV_CVC_MINEXCLUSIVE_VALID:
  case XML_SCHEMAV_CVC_MAXEXCLUSIVE_VALID:
  case XML_SCHEMAV_CVC_TOTALDIGITS_VALID:
  case XML_SCHEMAV_CVC_FRACTIONDIGITS_VALID:
  case XML_SCHEMAV_CVC_PATTERN_VALID:
  case XML_SCHEMAV_CVC_ENUMERATION_VALID:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_2_1:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_2_2:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_2_3:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_2_4:
  case XML_SCHEMAV_CVC_ELT_1:
  case XML_SCHEMAV_CVC_ELT_2:
  case XML_SCHEMAV_CVC_ELT_3_1:
  case XML_SCHEMAV_CVC_ELT_3_2_1:
  case XML_SCHEMAV_CVC_ELT_3_2_2:
  case XML_SCHEMAV_CVC_ELT_4_1:
  case XML_SCHEMAV_CVC_ELT_4_2:
  case XML_SCHEMAV_CVC_ELT_4_3:
  case XML_SCHEMAV_CVC_ELT_5_1_1:
  case XML_SCHEMAV_CVC_ELT_5_1_2:
  case XML_SCHEMAV_CVC_ELT_5_2_1:
  case XML_SCHEMAV_CVC_ELT_5_2_2_1:
  case XML_SCHEMAV_CVC_ELT_5_2_2_2_1:
  case XML_SCHEMAV_CVC_ELT_5_2_2_2_2:
  case XML_SCHEMAV_CVC_ELT_6:
  case XML_SCHEMAV_CVC_ELT_7:
  case XML_SCHEMAV_CVC_ATTRIBUTE_1:
  case XML_SCHEMAV_CVC_ATTRIBUTE_2:
  case XML_SCHEMAV_CVC_ATTRIBUTE_3:
  case XML_SCHEMAV_CVC_ATTRIBUTE_4:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_3_1:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_3_2_1:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_3_2_2:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_4:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_5_1:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_5_2:
  case XML_SCHEMAV_ELEMENT_CONTENT:
  case XML_SCHEMAV_DOCUMENT_ELEMENT_MISSING:
  case XML_SCHEMAV_CVC_COMPLEX_TYPE_1:
  case XML_SCHEMAV_CVC_AU:
  case XML_SCHEMAV_CVC_TYPE_1:
  case XML_SCHEMAV_CVC_TYPE_2:
  case XML_SCHEMAV_CVC_IDC:
  case XML_SCHEMAV_CVC_WILDCARD:
    // case XML_XPTR_UNKNOWN:
  case XML_XPTR_CHILDSEQ_START:
  case XML_XPTR_EVAL_FAILED:
  case XML_XPTR_EXTRA_OBJECTS:
    // case XML_C14N_CREATE:
  case XML_C14N_REQUIRES_UTF8:
  case XML_C14N_CREATE_STACK:
  case XML_C14N_INVALID_NODE:
  case XML_C14N_UNKNOW_NODE:
  case XML_C14N_RELATIVE_NAMESPACE:
  case XML_SCHEMAP_SRC_SIMPLE_TYPE_2:
  case XML_SCHEMAP_SRC_SIMPLE_TYPE_3:
  case XML_SCHEMAP_SRC_SIMPLE_TYPE_4:
  case XML_SCHEMAP_SRC_RESOLVE:
  case XML_SCHEMAP_SRC_RESTRICTION_BASE_OR_SIMPLETYPE:
  case XML_SCHEMAP_SRC_LIST_ITEMTYPE_OR_SIMPLETYPE:
  case XML_SCHEMAP_SRC_UNION_MEMBERTYPES_OR_SIMPLETYPES:
  case XML_SCHEMAP_ST_PROPS_CORRECT_1:
  case XML_SCHEMAP_ST_PROPS_CORRECT_2:
  case XML_SCHEMAP_ST_PROPS_CORRECT_3:
  case XML_SCHEMAP_COS_ST_RESTRICTS_1_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_1_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_1_3_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_1_3_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_1_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_1_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_3:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_4:
  case XML_SCHEMAP_COS_ST_RESTRICTS_2_3_2_5:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_1_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_2:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_1:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_3:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_4:
  case XML_SCHEMAP_COS_ST_RESTRICTS_3_3_2_5:
  case XML_SCHEMAP_COS_ST_DERIVED_OK_2_1:
  case XML_SCHEMAP_COS_ST_DERIVED_OK_2_2:
  case XML_SCHEMAP_S4S_ELEM_NOT_ALLOWED:
  case XML_SCHEMAP_S4S_ELEM_MISSING:
  case XML_SCHEMAP_S4S_ATTR_NOT_ALLOWED:
  case XML_SCHEMAP_S4S_ATTR_MISSING:
  case XML_SCHEMAP_S4S_ATTR_INVALID_VALUE:
  case XML_SCHEMAP_SRC_ELEMENT_1:
  case XML_SCHEMAP_SRC_ELEMENT_2_1:
  case XML_SCHEMAP_SRC_ELEMENT_2_2:
  case XML_SCHEMAP_SRC_ELEMENT_3:
  case XML_SCHEMAP_P_PROPS_CORRECT_1:
  case XML_SCHEMAP_P_PROPS_CORRECT_2_1:
  case XML_SCHEMAP_P_PROPS_CORRECT_2_2:
  case XML_SCHEMAP_E_PROPS_CORRECT_2:
  case XML_SCHEMAP_E_PROPS_CORRECT_3:
  case XML_SCHEMAP_E_PROPS_CORRECT_4:
  case XML_SCHEMAP_E_PROPS_CORRECT_5:
  case XML_SCHEMAP_E_PROPS_CORRECT_6:
  case XML_SCHEMAP_SRC_INCLUDE:
  case XML_SCHEMAP_SRC_ATTRIBUTE_1:
  case XML_SCHEMAP_SRC_ATTRIBUTE_2:
  case XML_SCHEMAP_SRC_ATTRIBUTE_3_1:
  case XML_SCHEMAP_SRC_ATTRIBUTE_3_2:
  case XML_SCHEMAP_SRC_ATTRIBUTE_4:
  case XML_SCHEMAP_NO_XMLNS:
  case XML_SCHEMAP_NO_XSI:
  case XML_SCHEMAP_COS_VALID_DEFAULT_1:
  case XML_SCHEMAP_COS_VALID_DEFAULT_2_1:
  case XML_SCHEMAP_COS_VALID_DEFAULT_2_2_1:
  case XML_SCHEMAP_COS_VALID_DEFAULT_2_2_2:
  case XML_SCHEMAP_CVC_SIMPLE_TYPE:
  case XML_SCHEMAP_COS_CT_EXTENDS_1_1:
  case XML_SCHEMAP_SRC_IMPORT_1_1:
  case XML_SCHEMAP_SRC_IMPORT_1_2:
  case XML_SCHEMAP_SRC_IMPORT_2:
  case XML_SCHEMAP_SRC_IMPORT_2_1:
  case XML_SCHEMAP_SRC_IMPORT_2_2:
  case XML_SCHEMAP_INTERNAL:
  case XML_SCHEMAP_NOT_DETERMINISTIC:
  case XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_1:
  case XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_2:
  case XML_SCHEMAP_SRC_ATTRIBUTE_GROUP_3:
  case XML_SCHEMAP_MG_PROPS_CORRECT_1:
  case XML_SCHEMAP_MG_PROPS_CORRECT_2:
  case XML_SCHEMAP_SRC_CT_1:
  case XML_SCHEMAP_DERIVATION_OK_RESTRICTION_2_1_3:
  case XML_SCHEMAP_AU_PROPS_CORRECT_2:
  case XML_SCHEMAP_A_PROPS_CORRECT_2:
  case XML_SCHEMAP_C_PROPS_CORRECT:
  case XML_SCHEMAP_SRC_REDEFINE:
#if XML_VERSION > 20621
  case XML_SCHEMAP_SRC_IMPORT:
  case XML_SCHEMAP_WARN_SKIP_SCHEMA:
  case XML_SCHEMAP_WARN_UNLOCATED_SCHEMA:
  case XML_SCHEMAP_WARN_ATTR_REDECL_PROH:
  case XML_SCHEMAP_WARN_ATTR_POINTLESS_PROH:
  case XML_SCHEMAP_AG_PROPS_CORRECT:
  case XML_SCHEMAP_COS_CT_EXTENDS_1_2:
  case XML_SCHEMAP_AU_PROPS_CORRECT:
  case XML_SCHEMAP_A_PROPS_CORRECT_3:
  case XML_SCHEMAP_COS_ALL_LIMITED:
#endif
  case XML_MODULE_OPEN:
  case XML_MODULE_CLOSE:
  case XML_CHECK_FOUND_ATTRIBUTE:
  case XML_CHECK_FOUND_TEXT:
  case XML_CHECK_FOUND_CDATA:
  case XML_CHECK_FOUND_ENTITYREF:
  case XML_CHECK_FOUND_ENTITY:
  case XML_CHECK_FOUND_PI:
  case XML_CHECK_FOUND_COMMENT:
  case XML_CHECK_FOUND_DOCTYPE:
  case XML_CHECK_FOUND_FRAGMENT:
  case XML_CHECK_FOUND_NOTATION:
  case XML_CHECK_UNKNOWN_NODE:
  case XML_CHECK_ENTITY_TYPE:
  case XML_CHECK_NO_PARENT:
  case XML_CHECK_NO_DOC:
  case XML_CHECK_NO_NAME:
  case XML_CHECK_NO_ELEM:
  case XML_CHECK_WRONG_DOC:
  case XML_CHECK_NO_PREV:
  case XML_CHECK_WRONG_PREV:
  case XML_CHECK_NO_NEXT:
  case XML_CHECK_WRONG_NEXT:
  case XML_CHECK_NOT_DTD:
  case XML_CHECK_NOT_ATTR:
  case XML_CHECK_NOT_ATTR_DECL:
  case XML_CHECK_NOT_ELEM_DECL:
  case XML_CHECK_NOT_ENTITY_DECL:
  case XML_CHECK_NOT_NS_DECL:
  case XML_CHECK_NO_HREF:
  case XML_CHECK_WRONG_PARENT:
  case XML_CHECK_NS_SCOPE:
  case XML_CHECK_NS_ANCESTOR:
  case XML_CHECK_NOT_UTF8:
  case XML_CHECK_NO_DICT:
  case XML_CHECK_NOT_NCNAME:
  case XML_CHECK_OUTSIDE_DICT:
  case XML_CHECK_WRONG_NAME:
  case XML_CHECK_NAME_NOT_NULL:
  case XML_I18N_NO_HANDLER:
  case XML_I18N_EXCESS_HANDLER:
  case XML_I18N_CONV_FAILED:
  case XML_I18N_NO_OUTPUT:
    aErrorMessage = L"badxml/";
    swprintf(buf, 20, L"%d", err->line);
    aErrorMessage += buf;
    aErrorMessage += L"/0/";
    aErrorMessage += fname + L"/";
    aErrorMessage += msg;
    break;
  case XML_IO_ENCODER:
  case XML_IO_FLUSH:
  case XML_IO_WRITE:
  case XML_IO_BUFFER_FULL:
  case XML_IO_LOAD_ERROR:
    aErrorMessage = L"servererror";
    break;
  case XML_IO_EACCES:
    aErrorMessage = L"servererror/EACCESS";
    break;
  case XML_IO_EAGAIN:
    aErrorMessage = L"servererror/AGAIN";
    break;
  case XML_IO_EBADF:
    aErrorMessage = L"servererror/BADF";
    break;
  case XML_IO_EBADMSG:
    aErrorMessage = L"servererror/BADMSG";
    break;
  case XML_IO_EBUSY:
    aErrorMessage = L"servererror/BUSY";
    break;
  case XML_IO_ECANCELED:
    aErrorMessage = L"servererror/CANCELED";
    break;
  case XML_IO_ECHILD:
    aErrorMessage = L"servererror/CHILD";
    break;
  case XML_IO_EDEADLK:
    aErrorMessage = L"servererror/DEADLK";
    break;
  case XML_IO_EDOM:
    aErrorMessage = L"servererror/DOM";
    break;
  case XML_IO_EEXIST:
    aErrorMessage = L"servererror/EXIST";
    break;
  case XML_IO_EFAULT:
    aErrorMessage = L"servererror/FAULT";
    break;
  case XML_IO_EFBIG:
    aErrorMessage = L"servererror/FBIG";
    break;
  case XML_IO_EINPROGRESS:
    aErrorMessage = L"servererror/INPROGRESS";
    break;
  case XML_IO_EINTR:
    aErrorMessage = L"servererror/INTR";
    break;
  case XML_IO_EINVAL:
    aErrorMessage = L"servererror/INVAL";
    break;
  case XML_IO_EIO:
    aErrorMessage = L"servererror/IO";
    break;
  case XML_IO_EISDIR:
    aErrorMessage = L"servererror/ISDIR";
    break;
  case XML_IO_EMFILE:
    aErrorMessage = L"servererror/MFILE";
    break;
  case XML_IO_EMLINK:
    aErrorMessage = L"servererror/MLINK";
    break;
  case XML_IO_EMSGSIZE:
    aErrorMessage = L"servererror/MSGSIZE";
    break;
  case XML_IO_ENAMETOOLONG:
    aErrorMessage = L"servererror/NAMETOOLONG";
    break;
  case XML_IO_ENFILE:
    aErrorMessage = L"servererror/NFILE";
    break;
  case XML_IO_ENODEV:
    aErrorMessage = L"servererror/NODEV";
    break;
  case XML_IO_ENOENT:
    aErrorMessage = L"servererror/NOENT";
    break;
  case XML_IO_ENOEXEC:
    aErrorMessage = L"servererror/NOEXEC";
    break;
  case XML_IO_ENOLCK:
    aErrorMessage = L"servererror/NOLCK";
    break;
  case XML_IO_ENOMEM:
    aErrorMessage = L"servererror/NOMEM";
    break;
  case XML_IO_ENOSPC:
    aErrorMessage = L"servererror/NOSPC";
    break;
  case XML_IO_ENOSYS:
    aErrorMessage = L"servererror/NOSYS";
    break;
  case XML_IO_ENOTDIR:
    aErrorMessage = L"servererror/NOTDIR";
    break;
  case XML_IO_ENOTEMPTY:
    aErrorMessage = L"servererror/NOTEMPTY";
    break;
  case XML_IO_ENOTSUP:
    aErrorMessage = L"servererror/NOTSUP";
    break;
  case XML_IO_ENOTTY:
    aErrorMessage = L"servererror/NOTTY";
    break;
  case XML_IO_ENXIO:
    aErrorMessage = L"servererror/NXIO";
    break;
  case XML_IO_EPERM:
    aErrorMessage = L"servererror/PERM";
    break;
  case XML_IO_EPIPE:
    aErrorMessage = L"servererror/PIPE";
    break;
  case XML_IO_ERANGE:
    aErrorMessage = L"servererror/RANGE";
    break;
  case XML_IO_EROFS:
    aErrorMessage = L"servererror/ROFS";
    break;
  case XML_IO_ESPIPE:
    aErrorMessage = L"servererror/SPIPE";
    break;
  case XML_IO_ESRCH:
    aErrorMessage = L"servererror/SRCH";
    break;
  case XML_IO_ETIMEDOUT:
    aErrorMessage = L"servererror/TIMEDOUT";
    break;
  case XML_IO_EXDEV:
    aErrorMessage = L"servererror/XDEV";
    break;
  case XML_IO_NO_INPUT:
    aErrorMessage = L"servererror/NOINPUT";
    break;
  case XML_IO_ENOTSOCK:
    aErrorMessage = L"servererror/NOTSOCK";
    break;
  case XML_IO_EISCONN:
    aErrorMessage = L"servererror/ISCONN";
    break;
  case XML_IO_ECONNREFUSED:
    aErrorMessage = L"servererror/CONNREFUSED";
    break;
  case XML_IO_ENETUNREACH:
    aErrorMessage = L"servererror/NETUNREACHABLE";
    break;
  case XML_IO_EADDRINUSE:
    aErrorMessage = L"servererror/ADDRINUSE";
    break;
  case XML_IO_EALREADY:
    aErrorMessage = L"servererror/ALREADY";
    break;
  case XML_IO_EAFNOSUPPORT:
    aErrorMessage = L"servererror/AFNOSUPPORT";
    break;
  case XML_FTP_EPSV_ANSWER:
    aErrorMessage = L"servererror/EPSV_ANSWER";
    break;
  case XML_FTP_ACCNT:
    aErrorMessage = L"servererror/FTPACCOUNT";
    break;
  case XML_HTTP_USE_IP:
    aErrorMessage = L"servererror/USE_IP";
    break;
  case XML_HTTP_UNKNOWN_HOST:
    aErrorMessage = L"servererror/UNKNOWNHOST";
    break;

  case XML_ERR_INVALID_URI:
  case XML_ERR_URI_FRAGMENT:
  case XML_FTP_URL_SYNTAX:
    aErrorMessage = L"badurl";
    break;

  case XML_IO_NETWORK_ATTEMPT:
    aErrorMessage = L"noperm";
    break;

  default:
    aErrorMessage = L"unexpectedxml2error";
  }
  xmlFreeParserCtxt(ctxt);
}

iface::dom::Document*
CDA_DOMImplementation::loadDocument
(
 const wchar_t* aURL,
 std::wstring& aErrorMessage
)
  throw(std::exception&)
{
  char* URL = CDA_wchar_to_UTF8(aURL);
  char* protRestrict = getenv("CELLML_RESTRICT_PROTOCOL");
  if (protRestrict != NULL)
  {
    bool isAllowed = false;
    while (protRestrict)
    {
      char* pr = strchr(protRestrict, ',');
      uint32_t l;
      if (pr != NULL)
      {
        l = pr - protRestrict;
        pr++;
      }
      else
        l = strlen(protRestrict);
      if (strlen(URL) > l &&
          URL[l] == ':' &&
          !strncmp(protRestrict, URL, l))
      {
        isAllowed = true;
        break;
      }
      
      protRestrict = pr;
    }
    if (!isAllowed)
    {
      aErrorMessage = L"noperm";
      free(URL);
      return NULL;
    }
  }
  std::string sURL = URL;
  free(URL);


  xmlParserCtxtPtr ctxt =
    xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, sURL.c_str());
  if (ctxt == NULL)
  {
    aErrorMessage = L"nomemory";
    return NULL;
  }
  
  xmlDocPtr xdp =
    xmlCtxtReadFile(ctxt, sURL.c_str(), NULL,
                    XML_PARSE_DTDLOAD | XML_PARSE_DTDATTR |
                    XML_PARSE_NOXINCNODE);
  if (xdp == NULL)
  {
    ProcessContextError(aErrorMessage, ctxt);
    return NULL;
  }
  xmlFreeParserCtxt(ctxt);

  // We just defined an XML2 document xdp. Convert to our data-structure...
  iface::dom::Document* doc =
    static_cast<CDA_Document*>(WrapXML2Node(NULL, (xmlNode*)xdp));
  xmlFreeDoc(xdp);
  return doc;
}

iface::dom::Document*
CDA_DOMImplementation::loadDocumentFromText
(
 const wchar_t* aText,
 std::wstring& aErrorMessage
)
  throw(std::exception&)
{
  char* text = CDA_wchar_to_UTF8(aText);

  xmlParserCtxtPtr ctxt =
    xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);
  if (ctxt == NULL)
  {
    aErrorMessage = L"nomemory";
    free(text);
    return NULL;
  }
  
  xmlDocPtr xdp =
    xmlCtxtReadMemory(ctxt, text, strlen(text), NULL, NULL,
                    XML_PARSE_DTDLOAD | XML_PARSE_DTDATTR |
                    XML_PARSE_NOXINCNODE);
  free(text);
  if (xdp == NULL)
  {
    ProcessContextError(aErrorMessage, ctxt);
    return NULL;
  }
  xmlFreeParserCtxt(ctxt);

  // We just defined an XML2 document xdp. Convert to our data-structure...
  iface::dom::Document* doc =
    static_cast<CDA_Document*>(WrapXML2Node(NULL, (xmlNode*)xdp));
  xmlFreeDoc(xdp);
  return doc;
}
