#include "VACSSImpl.hpp"
#include "VACSSBootstrap.hpp"
#include <string>

CDA_VACSService::CDA_VACSService()
  : _cda_refcount(1)
{
}

iface::cellml_services::CellMLValidityErrorSet*
CDA_VACSService::validateModel(iface::cellml_api::Model* aModel)
  throw()
{
  return NULL;
}

uint32_t
CDA_VACSService::getPositionInXML
(
 iface::dom::Node* aNode,
 uint32_t aNodalOffset,
 uint32_t* aColumn
)
  throw()
{
  uint32_t aRow = 1;

  *aColumn = 1;

  RETURN_INTO_OBJREF(doc, iface::dom::Document, aNode->ownerDocument());
  advanceCursorThroughNodeUntil(doc, aRow, *aColumn, aNode, aNodalOffset);

  return aRow;
}

bool
CDA_VACSService::advanceCursorThroughNodeUntil
(
 iface::dom::Node* aNode,
 uint32_t& aRow,
 uint32_t& aCol,
 iface::dom::Node* aUntil,
 uint32_t aUntilOffset
)
{
  bool visitChildren = false;
  uint32_t trailingCharacters = 0;

  bool isUntilNode = (CDA_objcmp(aNode, aUntil) == 0);

  switch (aNode->nodeType())
  {
  case iface::dom::Node::ELEMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, aNode, dom::Element);
      RETURN_INTO_WSTRING(name, el->nodeName());

      if (isUntilNode)
      {
        aCol += 1 + aUntilOffset;
        return true;
      }

      RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap,
                         el->attributes());
      uint32_t nnml = nnm->length();
      uint32_t i;
      for (i = 0; i < nnml; i++)
      {
        // XXX a single space between attributes is a canonical form, but we
        //     don't really know how it is actually laid out.
        if (i != 0)
          aCol++;
        RETURN_INTO_OBJREF(nn, iface::dom::Node, nnm->item(i));
        bool ret =
          advanceCursorThroughNodeUntil(nn, aRow, aCol, aUntil, aUntilOffset);
        if (ret)
          return true;
      }

      RETURN_INTO_OBJREF(elcnl, iface::dom::NodeList, el->childNodes());
      uint32_t elcnll = elcnl->length();

      if (elcnll)
      {
        aCol++; // >
        trailingCharacters = 3 + name.length(); // </name>
        visitChildren = true;
      }
      else
        aCol += 2; // />
    }
    break;
  case iface::dom::Node::ATTRIBUTE_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(attr, aNode, dom::Attr);

      RETURN_INTO_WSTRING(attrName, attr->name());

      // name="
      aCol += attrName.length() + 2;

      RETURN_INTO_WSTRING(attrValue, attr->nodeValue());
      
      advanceCursorThroughString(attrValue, aRow, aCol, isUntilNode,
                                 aUntilOffset);

      if (isUntilNode)
        return true;
      
      // Closing quote...
      aCol += 1;
    }
    break;
  case iface::dom::Node::TEXT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(txt, aNode, dom::Text);
      RETURN_INTO_WSTRING(value, txt->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::CDATA_SECTION_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(cds, aNode, dom::CDATASection);
      // [CDATA[
      aCol += 7;

      RETURN_INTO_WSTRING(value, cds->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::ENTITY_REFERENCE_NODE:
    {
      if (isUntilNode)
        return true;

      RETURN_INTO_WSTRING(value, aNode->nodeName());

      aCol += 1 + value.length();
    }
    break;
  case iface::dom::Node::ENTITY_NODE:
    {
      if (isUntilNode)
        return true;

      // XXX this could be better implemented.
      aCol = 1;
      aRow++;
    }
    break;
  case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(pi, aNode, dom::ProcessingInstruction);

      // <?
      aCol += 2;
      RETURN_INTO_WSTRING(targ, pi->target());
      aCol += targ.length() + 1;
      RETURN_INTO_WSTRING(value, pi->data());      
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::COMMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(comment, aNode, dom::Comment);
      // <!--
      aCol += 4;

      RETURN_INTO_WSTRING(value, comment->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;

      // -->
      aCol += 3;
    }
    break;
  case iface::dom::Node::DOCUMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(doc, aNode, dom::Document);

      if (isUntilNode)
        return true;

      aCol = 1;
      aRow++;

      visitChildren = true;
    }
    break;
  case iface::dom::Node::DOCUMENT_TYPE_NODE:
    {
      // XXX TODO...
      // DECLARE_QUERY_INTERFACE_OBJREF(doctype, aNode, dom::DocumentType);

      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::DOCUMENT_FRAGMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(docfrag, aNode, dom::DocumentFragment);
      visitChildren = true;

      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::NOTATION_NODE:
    {
      // XXX TODO...
      // DECLARE_QUERY_INTERFACE_OBJREF(notat, aNode, dom::Notation);

      if (isUntilNode)
        return true;
    }
    break;
  }

  if (visitChildren)
  {
    ObjRef<iface::dom::Node> child;
    for (child = already_AddRefd<iface::dom::Node>(aNode->firstChild());
         child != NULL;
         child = already_AddRefd<iface::dom::Node>(child->nextSibling()))
    {
      bool ret = advanceCursorThroughNodeUntil(child, aRow, aCol, aUntil,
                                               aUntilOffset);
      if (ret)
        return true;
    }
  }

  aCol += trailingCharacters;

  return false;
}

void
CDA_VACSService::advanceCursorThroughString
(
 const std::wstring& aStr,
 uint32_t& aRow,
 uint32_t& aCol,
 bool aStopHere,
 uint32_t aStopOffset
)
{
  uint32_t i;
  for (i = 0; i < aStr.length(); i++)
  {
    if (aStopHere && aStopOffset-- == 0)
        return;

    wchar_t c = aStr[i];
    switch (c)
    {
    case L'\r':
      // Ignore \r, only consider \n...
      continue;
    case L'\n':
      aCol = 1;
      aRow++;
      continue;
    case L'<':
      aCol += 4; // &lt;
      continue;
    case L'"':
      aCol += 6; // &quot;
      continue;
    default:
      aCol++;
      continue;
    }
  }
}

iface::cellml_services::VACSService*
CreateVACSService(void)
{
  return new CDA_VACSService();
}
