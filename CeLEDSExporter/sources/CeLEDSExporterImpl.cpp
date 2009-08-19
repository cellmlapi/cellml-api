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
}

wchar_t* 
CDA_CodeExporter::generateCode(iface::cellml_api::Model* model)
  throw(std::exception&)
{
  std::wstring output(L"");
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator,
      getCodeGenerator());
  if (cg == NULL) {return L"Could not access CCGS information.\n";}

  RETURN_INTO_OBJREF(codeinfo, iface::cellml_services::CodeInformation,
      cg->generateCode(model));

  // Check for errors and list problem variables or equations
  if (codeinfo == NULL) {return L"Unexpected error generating code";}
  if (*(codeinfo->errorMessage()) != L'\0') {
    output = L"Couldn't generate code for the following reason:\n";
    output += codeinfo->errorMessage();
    return CDA_wcsdup(output.c_str());
  }
  if (codeinfo->constraintLevel() == 0/* UNDERCONSTRAINED */) {
    output = L"Model is underconstrained.\n"
      L"The following variables couldn't be defined:\n";
    output += listVariablesByState(codeinfo, -1);
    return CDA_wcsdup(output.c_str());
  }
  else if (codeinfo->constraintLevel() == 2/* OVERCONSTRAINED*/) {
    output = L"Model is overconstrained.\n"
      L"At the time of the error, the following variables "
      L"were defined:\n";
    output += listVariablesByState(codeinfo, 1);
    output += L"The following equation was unneeded:\n";
    output += listFlaggedEquations(codeinfo);
    return CDA_wcsdup(output.c_str());
  }
  else if (codeinfo->constraintLevel() == 1 /* UNSUITABLY_CONSTRAINED*/) {
    output = L"Model is unsuitably constrained (i.e. there were "
      L"still some variables left to be computed, and "
      L"some unusable equations left over).\n"
      L"At the time of the error, the following variables "
      L"were found:\n";
    output += listVariablesByState(codeinfo, 0);
    output += L"The following equations couldn't be used:\n";
    output += listFlaggedEquations(codeinfo);
    return CDA_wcsdup(output.c_str());
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
  ObjRef<iface::cellml_services::ComputationTarget> ct;
  ct = cti->nextComputationTarget();
  while (ct != NULL) {
    output += getCodeSection(L"varListA");
    output += ct->name(); 
    output += getCodeSection(L"varListB");
    output += diffop(ct->degree());
    output += ct->variable()->name();
    output += L" in component ";
    output += ct->variable()->componentName();
    output += L" (";
    output += ct->variable()->unitsName();
    output += L")";
    output += getCodeSection(L"varListC");
    output += L"\n";
    ct = cti->nextComputationTarget();
  }
  output += getCodeSection(L"postVarList");

  // Write functions for calculating constants, rates and variables
  output += getCodeSection(L"preInitConsts");
  output += codeinfo->initConstsString();
  output += getCodeSection(L"postInitConsts") + getCodeSection(L"preRates");
  output += codeinfo->ratesString();
  output += getCodeSection(L"postRates") + getCodeSection(L"preVariables");
  // Repeat algebraic variable calculations used in rate calculations
  output += getAlgebraic(codeinfo->ratesString(), cg->algebraicVariableNamePattern());
  output += codeinfo->variablesString(),
  output += getCodeSection(L"postVariables");
  output += codeinfo->functionsString();

  // insert extra function definitions if required
  output += defineExtraFunctions(output);

  output += getCodeSection(L"endSection");
  
  return CDA_wcsdup(output.c_str());
}

std::wstring
CDA_CodeExporter::getCodeSection(const wchar_t* name)
  throw(std::exception&)
{
  uint32_t i;
  std::wstring nodeContents(L"");
  RETURN_INTO_OBJREF(langDict, iface::cellml_services::LanguageDictionary,
      mLangDictGen->getDictionary(L"http://www.cellml.org/CeLEDS/1.0#"));
  if (langDict == NULL) {
    return nodeContents;
  }
  RETURN_INTO_OBJREF(entries, iface::dom::NodeList,
      langDict->getMappings());
  if (entries == NULL) {
    return nodeContents;
  }
  ObjRef<iface::dom::Node> currentNode;
  for (i=0; i < entries->length(); i++) {
    currentNode = entries->item(i);
    RETURN_INTO_OBJREF(keyAttribute, iface::dom::Node,
        currentNode->attributes()->getNamedItem(L"keyname"));
    if (keyAttribute == NULL) continue;
    std::wstring s(keyAttribute->nodeValue());
    // If keyname matches
    if (s.compare(name) == 0) {
      nodeContents = getTextContents(currentNode);
      RETURN_INTO_OBJREF(paramsAttribute, iface::dom::Node,
          currentNode->attributes()->getNamedItem(L"solverparameters"));
      if(paramsAttribute != NULL) {
        s = paramsAttribute->nodeValue();
        // If text contains solver parameters that need to be replaced
        if (s.compare(L"true") == 0 || s.compare(L"1") == 0) {
          replaceParameter(nodeContents, L"<RANGESTART>", mRangeStart);
          replaceParameter(nodeContents, L"<RANGEEND>", mRangeEnd);
          replaceParameter(nodeContents, L"<ABSTOL>", mAbsTol);
          replaceParameter(nodeContents, L"<RELTOL>", mRelTol);
          replaceParameter(nodeContents, L"<MAXSTEP>", mMaxStep);
        }
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
  while (start < rc.length() && end != std::wstring::npos) {
    end = rc.find(L"\n", start);
    if (end != std::wstring::npos) {
      algebraic = rc.find(an, start);
      linebegin = rc.find_first_not_of(L" \r\n\t", start);
      if (algebraic == linebegin && algebraic != std::wstring::npos) {
        algebraicCalc.append(ratesCalc, start, end - start + 1);
      }
    }
    else break;
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
  while (pos != std::wstring::npos) {
    code.replace(pos, param.length(), toStr(value));
    pos = code.find(parameter);
  }
}

std::wstring
CDA_CodeExporter::diffop(uint32_t deg)
  throw(std::exception&) {
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
      extraFuncs->getElementsByTagNameNS(L"http://www.cellml.org/CeLEDS/1.0#", L"function"));
  if (functions->length() == 0) 
    return definitions;
  len = functions->length();
  ObjRef<iface::dom::Node> currentNode;
  for (i=0; i < len; i++) {
    currentNode = functions->item(i);
    RETURN_INTO_OBJREF(searchName, iface::dom::Node,
        currentNode->attributes()->getNamedItem(L"searchname"));
    if (searchName == NULL) continue;
    // look for searchname in generated code
    if (output.find(searchName->nodeValue()) != std::wstring::npos) {
      // if found, add function definition
      definitions.append(getTextContents(currentNode));
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
  ObjRef<iface::cellml_services::ComputationTarget> ct;

  ct = cti->nextComputationTarget();
  while (ct != NULL) {
    defined = (ct->type() != 4/*FLOATING*/);
    if (defined && definedFilter < 0) {
      ct = cti->nextComputationTarget();
      continue;
    }
    if (!defined && definedFilter > 0) {
      ct = cti->nextComputationTarget();
      continue;
    }
    str += L" * ";
    if (definedFilter == 0)
      str += defined ? L"Defined: " : L"Undefined: ";

    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable,
        ct->variable());
    
    str += diffop(ct->degree());
    str += v->name();
    str += L" in component ";
    str += v->componentName();
    
    DECLARE_QUERY_INTERFACE_OBJREF(vElement, v, cellml_api::CellMLElement);
    RETURN_INTO_OBJREF(model, iface::cellml_api::Model, vElement->modelElement());
    std::wstring bu(model->base_uri()->asText());
    if (!bu.empty()) {
      str += L" in model ";
      str += bu;
    }
    str += L"\n";
    ct = cti->nextComputationTarget();
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
  
  ObjRef<iface::dom::Node> apply;
  ObjRef<iface::dom::Node> math;
  std::wstring applyId(L""), mathId(L"");
  for (i = 0; i < l; i++) {
    apply = mnl->item(i);
    math = apply->parentNode();
    DECLARE_QUERY_INTERFACE_OBJREF(mathElement, math, dom::Element);
    DECLARE_QUERY_INTERFACE_OBJREF(applyElement, apply, dom::Element);
    str += L" * Equation";
    applyId = applyElement->getAttribute(L"id");
    mathId = mathElement->getAttribute(L"id");
    if (!applyId.empty()) {
      str += L" in apply element with id ";
      str += applyId;
    }
    if (!mathId.empty()) {
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

iface::cellml_services::CodeGenerator*
CDA_CodeExporter::getCodeGenerator() 
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ccgsDict, iface::cellml_services::LanguageDictionary, 
      mLangDictGen->getDictionary(L"http://www.cellml.org/CeLEDS/CCGS/1.0#"));
  if (ccgsDict == NULL) {return NULL;}
  RETURN_INTO_OBJREF(cgbs, iface::cellml_services::CodeGeneratorBootstrap, 
      CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator, 
      cgbs->createCodeGenerator());
  std::wstring pattern(L"");

  pattern=ccgsDict->getValue(L"constantPattern");
  if (!pattern.empty()) cg->constantPattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"stateVariableNamePattern");
  if (!pattern.empty()) cg->stateVariableNamePattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"algebraicVariableNamePattern");
  if (!pattern.empty()) cg->algebraicVariableNamePattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"rateNamePattern");
  if (!pattern.empty()) cg->rateNamePattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"voiPattern");
  if (!pattern.empty()) cg->voiPattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"assignPattern");
  if (!pattern.empty()) cg->assignPattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"solvePattern");
  if (!pattern.empty()) cg->solvePattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"solveNLSystemPattern");
  if (!pattern.empty()) cg->solveNLSystemPattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"temporaryVariablePattern");
  if (!pattern.empty()) cg->temporaryVariablePattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"declareTemporaryPattern");
  if (!pattern.empty()) cg->declareTemporaryPattern(pattern.c_str());
  pattern=ccgsDict->getValue(L"conditionalAssignmentPattern");
  if (!pattern.empty()) cg->conditionalAssignmentPattern(pattern.c_str());

  pattern=ccgsDict->getValue(L"arrayOffset");
  // convert string to integer
  uint32_t offset = 0;
  std::wistringstream strm(pattern);
  strm >> offset;
  if (!pattern.empty()) cg->arrayOffset(offset);

  // Set MaLaES transform
  cg->transform(mLangDictGen->getMalTransform());

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
  mLoadError = clb->loadError();
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
  mLoadError = clb->loadError();
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
getTextContents(iface::dom::Node* inNode)
  throw(std::exception&)
{
  uint32_t i;
  RETURN_INTO_OBJREF(children, iface::dom::NodeList, inNode->childNodes());
  for(i = 0; i < children->length(); i++) {
    if (children->item(i)->nodeType() == 3 || // Text node or CDATA
        children->item(i)->nodeType() == 4) {
      return children->item(i)->nodeValue();
    }
  }
  return L"";
}

