#ifndef _MaLaESImplementation_hpp
#define _MaLaESImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeVAS.hxx"
#include "Utilities.hxx"
#include <vector>
#include "IfaceMaLaES.hxx"
#include "IfaceCUSES.hxx"
#include "IfaceAnnoTools.hxx"
#include <string>
#include <map>
#include <set>

template<class C> class CleanupVector
  : public std::vector<C>
{
public:
  ~CleanupVector()
  {
    typename CleanupVector<C>::iterator i;
    for (i = CleanupVector<C>::begin(); i != CleanupVector<C>::end(); i++)
      (*i)->release_ref();
  }
};

class CDAMaLaESTransform;

class CDAMaLaESResult
  : public iface::cellml_services::MaLaESResult
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::MaLaESResult);

  CDAMaLaESResult(CDAMaLaESTransform* aTransform,
                  iface::cellml_services::CeVAS* aCeVAS,
                  iface::cellml_services::CUSES* aCUSES,
                  iface::cellml_services::AnnotationSet* aAnnos,
                  iface::cellml_api::CellMLElement* aContext,
                  bool aVarFromSource);
  ~CDAMaLaESResult();

  wchar_t* compileErrors() throw(std::exception&);
  wchar_t* expression() throw(std::exception&);
  uint32_t supplementariesLength() throw(std::exception&);
  wchar_t* getSupplementary(uint32_t aIndex) throw(std::exception&);
  iface::cellml_api::CellMLVariableIterator* iterateInvolvedVariables()
    throw(std::exception&);
  iface::cellml_api::CellMLVariableIterator* iterateBoundVariables()
    throw(std::exception&);
  iface::cellml_services::DegreeVariableIterator*
    iterateInvolvedVariablesByDegree() throw(std::exception&);
  bool involvesExternalCode() throw(std::exception&);
  
  void finishTransform();

  double startConversionMode(iface::mathml_dom::MathMLCiElement* aCI,
                             double& offset, bool aIsBound = false);
  bool writeConvertedVariable();
  void endConversionMode();

  bool pushPrecedence(uint32_t outer, uint32_t inner);
  void popPrecedence();

  void appendString(const std::wstring& aArg,
                    std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                    std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                    iface::mathml_dom::MathMLElement* degree,
                    iface::mathml_dom::MathMLElement* logbase);
  void appendExprs(const std::wstring& aArg,
                   std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                   std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                   iface::mathml_dom::MathMLElement* degree,
                   iface::mathml_dom::MathMLElement* logbase);
  void appendExpr(const std::wstring& aArg,
                  std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                  std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                  iface::mathml_dom::MathMLElement* degree,
                  iface::mathml_dom::MathMLElement* logbase);
  void appendDegree(const std::wstring& aArg,
                    std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                    std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                    iface::mathml_dom::MathMLElement* degree,
                    iface::mathml_dom::MathMLElement* logbase);
  void appendLogbase(const std::wstring& aArg,
                     std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                     std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                     iface::mathml_dom::MathMLElement* degree,
                     iface::mathml_dom::MathMLElement* logbase);
  void appendBvarIndex(const std::wstring& aArg,
                       std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                       std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                       iface::mathml_dom::MathMLElement* degree,
                       iface::mathml_dom::MathMLElement* logbase);
  void appendDiffVariable(const std::wstring& aArg,
                          std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                          std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                          iface::mathml_dom::MathMLElement* degree,
                          iface::mathml_dom::MathMLElement* logbase);
  void pushSupplement(const std::wstring& aArg,
                      std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                      std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                      iface::mathml_dom::MathMLElement* degree,
                      iface::mathml_dom::MathMLElement* logbase);
  void popSupplement(const std::wstring& aArg,
                     std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                     std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                     iface::mathml_dom::MathMLElement* degree,
                     iface::mathml_dom::MathMLElement* logbase);
  void appendUnique(const std::wstring& aArg,
                    std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                    std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                    iface::mathml_dom::MathMLElement* degree,
                    iface::mathml_dom::MathMLElement* logbase);
  void appendCount(const std::wstring& aArg,
                   std::vector<iface::mathml_dom::MathMLElement*>& aArgs,
                   std::vector<iface::mathml_dom::MathMLBvarElement*>& aBvars,
                   iface::mathml_dom::MathMLElement* degree,
                   iface::mathml_dom::MathMLElement* logbase);
  
  double parseConstant(iface::mathml_dom::MathMLCnElement* cnEl);
  void appendConstant(iface::mathml_dom::MathMLCnElement* cnEl);
  void setInvolvesExternalCode() { mInvolvesExternalCode = true; }
  uint32_t getDiffDegree(iface::cellml_api::CellMLVariable* aVar)
    throw(std::exception&);

  void gotError(const std::wstring& msg)
  {
    mError += msg;
    mError += L"; ";
  }

private:
  CDAMaLaESTransform* mTransform;
  iface::cellml_services::CeVAS* mCeVAS;
  iface::cellml_services::CUSES* mCUSES;
  iface::cellml_services::AnnotationSet* mAnnos;
  ObjRef<iface::cellml_api::CellMLComponent> mContext;
  std::list<std::pair<uint32_t, bool> > mPrec;
  std::wstring mError, mActive;
  std::list<std::wstring> mInactive;
  std::vector<std::wstring> mSupplementaries;
  std::map<std::wstring, uint32_t> mUniqueAssignments;
  uint32_t mLastUnique;
  std::set<iface::cellml_api::CellMLVariable*,XPCOMComparator> mInvolvedSet;
  std::set<std::pair<uint32_t, iface::cellml_api::CellMLVariable*> > mInvolvedDegSet;
  CleanupVector<iface::cellml_api::CellMLVariable*> mInvolved;
  std::vector<std::pair<uint32_t, iface::cellml_api::CellMLVariable*> > mInvolvedDeg;

  std::vector<iface::cellml_api::CellMLVariable*> mBoundVars;
  std::map<iface::cellml_api::CellMLVariable*, uint32_t> mHighestDegree;
  ObjRef<iface::cellml_api::CellMLVariable> processingVariable;
  bool boundVariable;
  uint32_t degree;
  double boundMup;
  bool mVariablesFromSource, mInvolvesExternalCode;
};

class CDAMaLaESTransform
  : public iface::cellml_services::MaLaESTransform
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::MaLaESTransform);
  
  CDAMaLaESTransform(const wchar_t* aSpec);
  ~CDAMaLaESTransform();

  wchar_t* compileErrors() throw(std::exception&);
  iface::cellml_services::MaLaESResult*
  transform(iface::cellml_services::CeVAS* aCeVAS,
            iface::cellml_services::CUSES* aCUSES,
            iface::cellml_services::AnnotationSet* aAnnos,
            iface::mathml_dom::MathMLElement* aMathML,
            iface::cellml_api::CellMLElement* aContext,
            iface::cellml_api::CellMLVariable* aUnitsOf,
            iface::cellml_api::CellMLVariable* aBoundUnitsOf,
            uint32_t aUnitsDiffDegree)
    throw(std::exception&);

  void RunTransformOnOperator(CDAMaLaESResult* aResult,
                              iface::mathml_dom::MathMLElement* aEl);
  void WriteConversion(CDAMaLaESResult* aResult,
                       iface::mathml_dom::MathMLCiElement* ci);
private:
  typedef std::pair<std::wstring, std::wstring> stringpair;
  typedef std::list<stringpair> stringpairlist;

  typedef void (CDAMaLaESResult::* appender)
    (const std::wstring&, std::vector<iface::mathml_dom::MathMLElement*>&,
     std::vector<iface::mathml_dom::MathMLBvarElement*>&,
     iface::mathml_dom::MathMLElement*, iface::mathml_dom::MathMLElement*);
  typedef std::pair<appender,std::wstring> command;
  typedef std::list<command> commandlist;

  struct Operator
  {
  public:
    Operator(const commandlist& aProgram, uint32_t aPrecInner,
             uint32_t aPrecOuter, int aMaxArg)
      : program(aProgram), precInner(aPrecInner), precOuter(aPrecOuter),
        maxarg(aMaxArg) {}
    commandlist program;
    uint32_t precInner, precOuter;
    int maxarg;
  };

  std::map<std::wstring, Operator> operMap;

  void GetTagsForSpec(const wchar_t* aSpec, stringpairlist& aTags);
  void ParseError(const wchar_t* msg, size_t lineno);
  void OpError(const wchar_t* msg, const std::wstring& op);
  void AddOperator(const std::wstring& aOpName, const std::wstring& aOpValue);
  void AppendCommandToProgram(commandlist& aProgram, const std::wstring& aCmd,
                              const std::wstring& aArg, int& maxarg);
  void ExecuteTransform(CDAMaLaESResult* aResult,
                        const std::wstring& aOpName,
                        std::vector<iface::mathml_dom::MathMLElement*>& args,
                        std::vector<iface::mathml_dom::MathMLBvarElement*>& bvars,
                        iface::mathml_dom::MathMLElement* degree,
                        iface::mathml_dom::MathMLElement* logbase);

  std::wstring mMessage, openGroup, closeGroup;
  bool mVariablesFromSource;
};

class CDAMaLaESBootstrap
  : public iface::cellml_services::MaLaESBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::MaLaESBootstrap);

  CDAMaLaESBootstrap();
  ~CDAMaLaESBootstrap();
  iface::cellml_services::MaLaESTransform* compileTransformer(const wchar_t*)
    throw(std::exception&);
};

#endif // _MaLaESImplementation_hpp
