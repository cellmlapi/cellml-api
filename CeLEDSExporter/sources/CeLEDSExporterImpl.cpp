#define IN_CELEDSEXPORTER_MODULE
#include "CeLEDSExporterImpl.hpp"
#include "CeLEDSExporterBootstrap.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include "CCGSBootstrap.hpp"
#include "CeLEDSBootstrap.hpp"
#include <iostream>
#include <sstream>

template <class X> std::wstring toStr(X in)
  throw(std::exception&)
{
  std::wostringstream strm;
  strm << in;
  return strm.str();
}

CDA_CodeExporter::CDA_CodeExporter(iface::cellml_services::DictionaryGenerator* langDictGen)
  throw()
  : _cda_refcount(1), mLangDictGen(langDictGen), mRangeStart(0.0), mRangeEnd(10.0),
    mAbsTol(1E-6), mRelTol(1E-6), mMaxStep(1.0)
{
  mLangDict = already_AddRefd<iface::cellml_services::LanguageDictionary>
    (mLangDictGen->getDictionary(L"http://www.cellml.org/CeLEDS/1.0#"));
  mCCGSLangDict = already_AddRefd<iface::cellml_services::LanguageDictionary>
    (mLangDictGen->getDictionary(L"http://www.cellml.org/CeLEDS/CCGS/1.0#"));
  RETURN_INTO_WSTRING(codeStyle, mCCGSLangDict->getValue(L"codeStyle"));
  if (codeStyle == L"implicit")
    mCodeStyle = CODESTYLE_IMPLICIT;
  else // default...
    mCodeStyle = CODESTYLE_EXPLICIT;
}

wchar_t* 
CDA_CodeExporter::generateCode(iface::cellml_api::Model* model)
  throw(std::exception&)
{
  std::wstring output(L"");

  switch (mCodeStyle)
  {
  case CODESTYLE_IMPLICIT:
    return generateCodeImplicit(model);
  case CODESTYLE_EXPLICIT:
    return generateCodeExplicit(model);
  }

  return CDA_wcsdup(L"CDA_CodeExporter::generateCode encountered an unknown code style - "
                    L"this shouldn't happen, so please file a bug report.");
}

int
CDA_CodeExporter::generateCodeCommonHeader(std::wstring& output,
                                           iface::cellml_services::CodeGenerator* cg,
                                           iface::cellml_services::CodeInformation* codeinfo)
{
  // Check for errors and list problem variables or equations
  if (codeinfo == NULL)
  {
    output = L"Unexpected error generating code";
    return 1;
  }

  RETURN_INTO_WSTRING(em, codeinfo->errorMessage());
  if (em != L"")
  {
    output = L"Couldn't generate code for the following reason:\n";
    output += em;
    return 1;
  }

  if (codeinfo->constraintLevel() == 0/* UNDERCONSTRAINED */)
  {
    output = L"Model is underconstrained.\n"
             L"The following variables couldn't be defined:\n";
    output += listVariablesByState(codeinfo, -1);
    return 1;
  }
  else if (codeinfo->constraintLevel() == 2/* OVERCONSTRAINED*/)
  {
    output = L"Model is overconstrained.\n"
             L"At the time of the error, the following variables "
             L"were defined:\n";
    output += listVariablesByState(codeinfo, 1);
    output += L"The following equation was unneeded:\n";
    output += listFlaggedEquations(codeinfo);
    return 1;
  }
  else if (codeinfo->constraintLevel() == 1 /* UNSUITABLY_CONSTRAINED*/)
  {
    output = L"Model is unsuitably constrained (i.e. there were "
      L"still some variables left to be computed, and "
      L"some unusable equations left over).\n"
      L"At the time of the error, the following variables "
      L"were found:\n";
    output += listVariablesByState(codeinfo, 0);
    output += L"The following equations couldn't be used:\n";
    output += listFlaggedEquations(codeinfo);
    return 1;
  }

  // insert bracketing strings, replacing with solver params if necessary,
  // and code output from codeinfo
  output += getCodeSection(L"preAlgebraicCount");
  output += toStr(codeinfo->algebraicIndexCount());
  output += getCodeSection(L"postAlgebraicCount");

  output += getCodeSection(L"preStateCount");
  output += toStr(codeinfo->rateIndexCount());
  output += getCodeSection(L"postStateCount");
  
  output += getCodeSection(L"preConstantCount");
  output += toStr(codeinfo->constantIndexCount());
  output += getCodeSection(L"postConstantCount") + getCodeSection(L"topSection");

  // Write a line for each variable
  output += getCodeSection(L"preVarList");
  RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
                     codeinfo->iterateTargets());

  while (true)
  {
    RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget, cti->nextComputationTarget());
    if (ct == NULL)
      break;

    output += getCodeSection(L"varListA");
    {
      RETURN_INTO_WSTRING(n, ct->name());
      output += n; 
    }

    output += getCodeSection(L"varListB");
    output += diffop(ct->degree());
    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable, ct->variable());
    RETURN_INTO_WSTRING(vn, v->name());
    output += vn;
    output += L" in component ";
    RETURN_INTO_WSTRING(vcn, v->componentName());
    output += vcn;
    output += L" (";
    RETURN_INTO_WSTRING(vun, v->unitsName());
    output += vun;
    output += L")";
    output += getCodeSection(L"varListC");
    output += L"\n";
  }
  output += getCodeSection(L"postVarList");

  // Write functions for calculating constants, rates and variables
  output += getCodeSection(L"preInitConsts");
  RETURN_INTO_WSTRING(ics, codeinfo->initConstsString());
  output += ics;
  output += getCodeSection(L"postInitConsts") + getCodeSection(L"preRates");
  RETURN_INTO_WSTRING(rs, codeinfo->ratesString());
  output += rs;
  output += getCodeSection(L"postRates") + getCodeSection(L"preVariables");
  // Repeat algebraic variable calculations used in rate calculations
  RETURN_INTO_WSTRING(avnp, cg->algebraicVariableNamePattern());
  output += getAlgebraic(rs.c_str(), avnp.c_str());
  RETURN_INTO_WSTRING(vs, codeinfo->variablesString());
  output += vs;
  output += getCodeSection(L"postVariables");

  return 0;
}

int
CDA_CodeExporter::generateCodeCommonFooter
(
 std::wstring& output,
 iface::cellml_services::CodeInformation* codeinfo
)
{
  RETURN_INTO_WSTRING(fs, codeinfo->functionsString());
  output += fs;

  // insert extra function definitions if required
  output += defineExtraFunctions(output);

  output += getCodeSection(L"endSection");

  return 0;
}

wchar_t*
CDA_CodeExporter::generateCodeExplicit(iface::cellml_api::Model* model)
{
  std::wstring output;

  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator,
                     getExplicitCodeGenerator());

  if (cg == NULL)
    return CDA_wcsdup(L"Could not access CCGS information.\n");

  RETURN_INTO_OBJREF(codeinfo, iface::cellml_services::CodeInformation,
      cg->generateCode(model));

  if (generateCodeCommonHeader(output, cg, codeinfo))
    return CDA_wcsdup(output.c_str());

  generateCodeCommonFooter(output, codeinfo);
  
  return CDA_wcsdup(output.c_str());
}

wchar_t*
CDA_CodeExporter::generateCodeImplicit(iface::cellml_api::Model* model)
{
  std::wstring output;

  RETURN_INTO_OBJREF(cg, iface::cellml_services::IDACodeGenerator,
                     getImplicitCodeGenerator());

  if (cg == NULL)
    return CDA_wcsdup(L"Could not access CCGS information.\n");

  RETURN_INTO_OBJREF(codeinfo, iface::cellml_services::IDACodeInformation,
                     cg->generateIDACode(model));

  if (generateCodeCommonHeader(output, cg, codeinfo))
    return CDA_wcsdup(output.c_str());

  output += getCodeSection(L"preEssentialVariables");
  RETURN_INTO_WSTRING(evs, codeinfo->essentialVariablesString());
  output += evs;
  output += getCodeSection(L"postEssentialVariables") + getCodeSection(L"preStateInformation");
  RETURN_INTO_WSTRING(sis, codeinfo->stateInformationString());
  output += sis;
  output += getCodeSection(L"postStateInformation");

  generateCodeCommonFooter(output, codeinfo);
  
  return CDA_wcsdup(output.c_str());
}

std::wstring
CDA_CodeExporter::getCodeSection(const wchar_t* name)
  throw(std::exception&)
{
  uint32_t i;
  if (mLangDict == NULL)
    return std::wstring(L"");

  RETURN_INTO_OBJREF(entries, iface::dom::NodeList,
                     mLangDict->getMappings());
  if (entries == NULL)
    return std::wstring(L"");

  for (i=0; i < entries->length(); i++)
  {
    RETURN_INTO_OBJREF(currentNode, iface::dom::Node, entries->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(currentEl, currentNode, dom::Element);
    if (currentEl == NULL)
      continue;
    RETURN_INTO_WSTRING(keyn, currentEl->getAttribute(L"keyname"));

    // If keyname matches
    if (keyn == name)
    {
      RETURN_INTO_WSTRING(nodeContents, getTextContents(currentNode));
      RETURN_INTO_WSTRING(params, currentEl->getAttribute(L"solverparameters"));

      // If text contains solver parameters that need to be replaced
      if (params == L"true" || params == L"1")
      {
        replaceParameter(nodeContents, L"<RANGESTART>", mRangeStart);
        replaceParameter(nodeContents, L"<RANGEEND>", mRangeEnd);
        replaceParameter(nodeContents, L"<ABSTOL>", mAbsTol);
        replaceParameter(nodeContents, L"<RELTOL>", mRelTol);
        replaceParameter(nodeContents, L"<MAXSTEP>", mMaxStep);
      }

      return nodeContents;
    }
  }
  return std::wstring(L"");
}

std::wstring 
CDA_CodeExporter::getAlgebraic(const wchar_t* ratesCalc, const wchar_t* algebraicNamePattern)
  throw(std::exception&)
{
  std::wstring algebraicCalc(L"");
  std::wstring rc(ratesCalc);
  // remove from part of pattern replaced by CCGS
  std::wstring an(algebraicNamePattern);

  // -3 needed to remove [ or (:, to match line using rootfind
  an.erase(an.find(L"%") - 3);
  size_t start = 0, end = 0, linebegin, algebraic;

  // Find lines beginning with algebraic name
  while (start < rc.length() && end != std::wstring::npos)
  {
    end = rc.find(L"\n", start);
    if (end != std::wstring::npos)
    {
      algebraic = rc.find(an, start);
      linebegin = rc.find_first_not_of(L" \r\n\t", start);
      if (algebraic == linebegin && algebraic != std::wstring::npos)
        algebraicCalc.append(ratesCalc, start, end - start + 1);
    }
    else
      break;
    start = end + 1;
  }
  return algebraicCalc;
}

void
CDA_CodeExporter::replaceParameter(std::wstring &code, const wchar_t* parameter, double value)
  throw(std::exception&)
{
  std::wstring param(parameter);
  size_t pos = code.find(parameter);
  while (pos != std::wstring::npos)
  {
    code.replace(pos, param.length(), toStr(value));
    pos = code.find(parameter);
  }
}

std::wstring
CDA_CodeExporter::diffop(uint32_t deg)
  throw(std::exception&)
{
  if (deg == 0)
    return std::wstring(L"");

  // XXX the bound variable isn't always 't'...
  if (deg == 1)
    return std::wstring(L"d/dt ");

  std::wstring diff(L"d^");
  diff += toStr(deg);
  diff += L"/dt";
  return diff;
}

std::wstring
CDA_CodeExporter::defineExtraFunctions(const std::wstring output)
  throw(std::exception&)
{
  uint32_t i, len;
  std::wstring definitions(L"");
  // Get extra function definitions
  RETURN_INTO_OBJREF(extraFuncs, iface::dom::Element,
                     mLangDictGen->getElementNS(L"http://www.cellml.org/CeLEDS/1.0#", L"extrafunctions"));
  if (extraFuncs == NULL)
    return definitions;
  RETURN_INTO_OBJREF(functions, iface::dom::NodeList,
                     extraFuncs->getElementsByTagNameNS
                     (L"http://www.cellml.org/CeLEDS/1.0#", L"function"));
  if (functions->length() == 0)
    return definitions;

  len = functions->length();
  for (i=0; i < len; i++)
  {
    RETURN_INTO_OBJREF(currentNode, iface::dom::Node, functions->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(currentEl, currentNode, dom::Element);
    if (currentEl == NULL)
      continue;

    RETURN_INTO_WSTRING(searchName, currentEl->getAttribute(L"searchname"));

    // look for searchname in generated code
    if (output.find(searchName) != std::wstring::npos)
    {
      // if found, add function definition
      RETURN_INTO_WSTRING(tc, getTextContents(currentNode));
      definitions += tc;
    }
  }
  return definitions;
}

std::wstring
CDA_CodeExporter::listVariablesByState(iface::cellml_services::CodeInformation *cci, int definedFilter)
  throw(std::exception&)
{
  std::wstring str(L"");
  bool defined;
  RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
      cci->iterateTargets());

  while (true)
  {
    RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget,
                       cti->nextComputationTarget());
    if (ct == NULL)
      break;

    defined = (ct->type() != 4/*FLOATING*/);
    if (defined && definedFilter < 0)
      continue;

    if (!defined && definedFilter > 0)
      continue;

    str += L" * ";
    if (definedFilter == 0)
      str += defined ? L"Defined: " : L"Undefined: ";

    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable,
                       ct->variable());
    
    str += diffop(ct->degree());
    RETURN_INTO_WSTRING(n, v->name());
    str += n;
    str += L" in component ";
    RETURN_INTO_WSTRING(cn, v->componentName());
    str += cn;
    
    DECLARE_QUERY_INTERFACE_OBJREF(vElement, v, cellml_api::CellMLElement);
    RETURN_INTO_OBJREF(model, iface::cellml_api::Model, vElement->modelElement());
    RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, model->baseURI());
    RETURN_INTO_WSTRING(tbu, bu->asText());
    if (!tbu.empty()) {
      str += L" in model ";
      str += tbu;
    }
    str += L"\n";
  }
  return str;
}

std::wstring
CDA_CodeExporter::listFlaggedEquations(iface::cellml_services::CodeInformation *cci)
  throw(std::exception&)
{
  uint32_t i, l;
  RETURN_INTO_OBJREF(mnl, iface::mathml_dom::MathMLNodeList, 
     cci->flaggedEquations());
  std::wstring str(L"");
  l = mnl->length();
  
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(apply, iface::dom::Node, mnl->item(i));
    RETURN_INTO_OBJREF(math, iface::dom::Node, apply->parentNode());
    DECLARE_QUERY_INTERFACE_OBJREF(mathElement, math, dom::Element);
    DECLARE_QUERY_INTERFACE_OBJREF(applyElement, apply, dom::Element);
    str += L" * Equation";
    RETURN_INTO_WSTRING(applyId, applyElement->getAttribute(L"id"));
    RETURN_INTO_WSTRING(mathId, mathElement->getAttribute(L"id"));
    if (!applyId.empty())
    {
      str += L" in apply element with id ";
      str += applyId;
    }
    if (!mathId.empty())
    {
      str += L" in math element with id ";
      str += mathId;
    }
    if (mathId.empty() && applyId.empty()) {
      str += L" with no id on the apply or math element";
    }
    str += L".\n";
  }
  return str;
}

double CDA_CodeExporter::rangeStart() throw()
{
  return mRangeStart;
}
void CDA_CodeExporter::rangeStart(double aValue) throw()
{
  mRangeStart = aValue;
}
double CDA_CodeExporter::rangeEnd() throw()
{
  return mRangeEnd;
}
void CDA_CodeExporter::rangeEnd(double aValue) throw()
{
  mRangeEnd = aValue;
}
double CDA_CodeExporter::absTol() throw()
{
  return mAbsTol;
}
void CDA_CodeExporter::absTol(double aValue) throw()
{
  mAbsTol = aValue;
}
double CDA_CodeExporter::relTol() throw()
{
  return mRelTol;
}
void CDA_CodeExporter::relTol(double aValue) throw()
{
  mRelTol = aValue;
}
double CDA_CodeExporter::maxStep() throw()
{
  return mMaxStep;
}
void CDA_CodeExporter::maxStep(double aValue) throw()
{
  mMaxStep = aValue;
}

#include <assert.h>

void
CDA_CodeExporter::transferCommonCodeAttributes
(
 iface::cellml_services::CodeGenerator* cg
)
{
#define TRANSFER_ATTRIBUTE(n, x)                 \
  { \
    RETURN_INTO_WSTRING(pattern, mCCGSLangDict->getValue(n)); \
    if (!pattern.empty()) \
      cg->x(pattern.c_str()); \
  }

  TRANSFER_ATTRIBUTE(L"constantPattern", constantPattern);
  TRANSFER_ATTRIBUTE(L"stateVariableNamePattern", stateVariableNamePattern);
  TRANSFER_ATTRIBUTE(L"algebraicVariableNamePattern", algebraicVariableNamePattern);
  TRANSFER_ATTRIBUTE(L"rateNamePattern", rateNamePattern);
  TRANSFER_ATTRIBUTE(L"assignPattern", assignPattern);
  TRANSFER_ATTRIBUTE(L"solvePattern", solvePattern);
  TRANSFER_ATTRIBUTE(L"solveNLSystemPattern", solveNLSystemPattern);
  TRANSFER_ATTRIBUTE(L"temporaryVariablePattern", temporaryVariablePattern);
  TRANSFER_ATTRIBUTE(L"declareTemporaryPattern", declareTemporaryPattern);
  TRANSFER_ATTRIBUTE(L"conditionalAssignmentPattern", conditionalAssignmentPattern);

  {
    RETURN_INTO_WSTRING(pattern, mLangDict->getValue(L"arrayOffset"));
    // convert string to integer
    uint32_t offset = 0;
    std::wistringstream strm(pattern);
    strm >> offset;
    if (!pattern.empty())
      cg->arrayOffset(offset);
  }

  // Set MaLaES transform
  RETURN_INTO_OBJREF(mt, iface::cellml_services::MaLaESTransform, mLangDictGen->getMalTransform());
  cg->transform(mt);
}

iface::cellml_services::CodeGenerator*
CDA_CodeExporter::getExplicitCodeGenerator()
  throw(std::exception&)
{
  if (mLangDict == NULL)
    return NULL;
  RETURN_INTO_OBJREF(cgbs, iface::cellml_services::CodeGeneratorBootstrap, 
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator, 
                     cgbs->createCodeGenerator());
  transferCommonCodeAttributes(cg);

  cg->add_ref();
  return cg;
}

iface::cellml_services::IDACodeGenerator*
CDA_CodeExporter::getImplicitCodeGenerator()
  throw(std::exception&)
{
  if (mLangDict == NULL)
    return NULL;
  RETURN_INTO_OBJREF(cgbs, iface::cellml_services::CodeGeneratorBootstrap, 
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::IDACodeGenerator, 
                     cgbs->createIDACodeGenerator());

  transferCommonCodeAttributes(cg);

  TRANSFER_ATTRIBUTE(L"residualPattern", residualPattern);
  TRANSFER_ATTRIBUTE(L"constrainedRateStateInfoPattern", constrainedRateStateInfoPattern);
  TRANSFER_ATTRIBUTE(L"unconstrainedRateStateInfoPattern", unconstrainedRateStateInfoPattern);

  cg->add_ref();
  return cg;
}

iface::cellml_services::CodeExporter* 
CDA_CeLEDSExporterBootstrap::createExporter(const wchar_t* URL)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(dg, iface::cellml_services::DictionaryGenerator,
                     createDictGenerator(URL));
  if (dg == NULL)
    return NULL;
  else
    return new CDA_CodeExporter(dg);
}

iface::cellml_services::CodeExporter* 
CDA_CeLEDSExporterBootstrap::createExporterFromText(const wchar_t* XMLText)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(dg, iface::cellml_services::DictionaryGenerator,
                     createDictGeneratorFromText(XMLText));
  if (dg == NULL)
    return NULL;
  else
    return new CDA_CodeExporter(dg);
}

iface::cellml_services::DictionaryGenerator* 
CDA_CeLEDSExporterBootstrap::createDictGenerator(const wchar_t* URL)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(clb, iface::cellml_services::CeLEDSBootstrap,
      CreateCeLEDSBootstrap());
  RETURN_INTO_OBJREF(dg, iface::cellml_services::DictionaryGenerator,
      clb->createDictGenerator(URL));
  RETURN_INTO_WSTRING(mLoadError, clb->loadError());
  if (dg != NULL)
    dg->add_ref();
  return dg;
}

iface::cellml_services::DictionaryGenerator* 
CDA_CeLEDSExporterBootstrap::createDictGeneratorFromText(const wchar_t* XMLText)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(clb, iface::cellml_services::CeLEDSBootstrap,
      CreateCeLEDSBootstrap());
  RETURN_INTO_OBJREF(dg, iface::cellml_services::DictionaryGenerator,
      clb->createDictGeneratorFromText(XMLText));
  RETURN_INTO_WSTRING(le, clb->loadError());
  mLoadError = le;
  if (dg != NULL)
    dg->add_ref();
  return dg;
}

wchar_t* 
CDA_CeLEDSExporterBootstrap::loadError() 
  throw()
{
  return CDA_wcsdup(mLoadError.c_str());
}

iface::cellml_services::CeLEDSExporterBootstrap*
CreateCeLEDSExporterBootstrap()
{
  return new CDA_CeLEDSExporterBootstrap();
}

wchar_t*
CDA_CodeExporter::getTextContents(iface::dom::Node* inNode)
  throw(std::exception&)
{
  uint32_t i;
  RETURN_INTO_OBJREF(children, iface::dom::NodeList, inNode->childNodes());
  for (i = 0; i < children->length(); i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, children->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);

    if (tn != NULL)
      return tn->data();
  }

  return CDA_wcsdup(L"");
}
