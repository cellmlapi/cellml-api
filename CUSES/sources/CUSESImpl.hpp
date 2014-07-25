#ifndef CUSESImpl_hpp
#define CUSESImpl_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCUSES.hxx"
#include "Utilities.hxx"
#include <vector>
#include <string>
#include <map>
#include "IfaceAnnoTools.hxx"

class CDAUserBaseUnit
  : public iface::cellml_services::UserBaseUnit
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(cellml_services::BaseUnit, cellml_services::UserBaseUnit);

  CDAUserBaseUnit(iface::cellml_api::Units* aBaseUnits) throw();
  ~CDAUserBaseUnit() throw();

  std::wstring name() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Units> cellmlUnits() throw(std::exception&);

private:
  ObjRef<iface::cellml_api::Units> mBaseUnits;
};

class CDABaseUnitInstance
  : public iface::cellml_services::BaseUnitInstance
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::BaseUnitInstance);

  CDABaseUnitInstance(iface::cellml_services::BaseUnit* aBU,
                      double aPrefix,
                      double aOffset,
                      double aExponent) throw();
  ~CDABaseUnitInstance() throw();

  already_AddRefd<iface::cellml_services::BaseUnit> unit() throw(std::exception&);
  double prefix() throw(std::exception&);
  double offset() throw(std::exception&);
  double exponent() throw(std::exception&);

private:
  ObjRef<iface::cellml_services::BaseUnit> mBaseUnit;
  double mPrefix, mOffset, mExponent;
};

class CDACanonicalUnitRepresentation
  : public iface::cellml_services::CanonicalUnitRepresentation
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CanonicalUnitRepresentation);

  CDACanonicalUnitRepresentation(bool aStrict) throw();
  ~CDACanonicalUnitRepresentation() throw();

  uint32_t length() throw(std::exception&);
  already_AddRefd<iface::cellml_services::BaseUnitInstance> fetchBaseUnit(uint32_t aIndex)
    throw(std::exception&);
  bool compatibleWith(iface::cellml_services::CanonicalUnitRepresentation* aCompareWith)
    throw(std::exception&);
  double convertUnits(iface::cellml_services::CanonicalUnitRepresentation* aConvertTo,
                      double* aOffset)
    throw(std::exception&);
  double siConversion(double* aOffset)
    throw(std::exception&);

  void canonicalise() throw(std::exception&);
  void addBaseUnit(iface::cellml_services::BaseUnitInstance* baseUnit) throw();
  already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
  mergeWith(double aThisExponent,
            iface::cellml_services::CanonicalUnitRepresentation* aOther,
            double aOtherExponent)
    throw(std::exception&);

  double carry() const { return mCarry; }
  void carry(double c) { mCarry = c; }

private:
  bool mStrict;
  double mCarry;
  std::vector<iface::cellml_services::BaseUnitInstance*> baseUnits;
};

#include <typeinfo>

class CDACUSES
  : public iface::cellml_services::CUSES
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CUSES);

  CDACUSES(iface::cellml_api::Model* aModel, bool aStrict) throw();
  ~CDACUSES() throw();

  already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation> getUnitsByName
  (iface::cellml_api::CellMLElement* aContext, const std::wstring& aName)
    throw(std::exception&);

  std::wstring modelError() throw(std::exception&);
  already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation> createEmptyUnits()
    throw(std::exception&);

private:
  std::wstring errorDescription;
  std::wstring warningDescription;

  static std::wstring getUnitScope(iface::cellml_api::CellMLElement* el);
  void PopulateBuiltinUnits();
  static bool BuiltinUnit(std::wstring& aName);

  template<class C>
  class ScopeMap
  {
    typedef std::multimap<std::wstring, C*> maptype;
  public:
    ScopeMap()
      : mNeedClean(true) {}

    ~ScopeMap()
    {
      cleanNow();
    }

    void cleanNow()
    {
      if (!mNeedClean)
        return;
      typename maptype::iterator i;
      for (i = mMap.begin(); i != mMap.end(); i++)
      {
        C* obj = (*i).second;
        obj->release_ref();
      }
      mNeedClean = false;
    }

    maptype*
    operator -> ()
    {
      return &mMap;
    }
  private:
    bool mNeedClean;
    maptype mMap;
  };

  template<class C> static C*
    scopedFind(
               ScopeMap<C>& aMap,
               iface::cellml_api::CellMLElement* aContext,
               const std::wstring& aName
              );

  class UnitDependencies
    : public iface::XPCOM::IObject
  {
  public:
    CDA_IMPL_ID;
    CDA_IMPL_REFCOUNT;
    CDA_IMPL_QI0;

    UnitDependencies() : seen(false), done(false) {}

    ~UnitDependencies()
    {
      std::list<iface::cellml_api::Units*>::iterator i;
      for (i = dependencies.begin(); i != dependencies.end(); i++)
        (*i)->release_ref();
    }

    void addDependency(iface::cellml_api::Units* dep)
    {
      dep->add_ref();
      dependencies.push_back(dep);
    }

    std::list<iface::cellml_api::Units*> dependencies;
    std::list<std::wstring> scopes;

    bool seen, done;
    std::wstring name;
  };

  void PopulateBuiltins();
  bool dfsResolveUnits(iface::cellml_services::AnnotationSet* cusesAS,
                       UnitDependencies* search);
  void ComputeUnits(UnitDependencies*, iface::cellml_api::Units*);
  bool mStrict;
  ScopeMap<CDACanonicalUnitRepresentation> mUnitsMap;
};

class CDACUSESBootstrap
  : public iface::cellml_services::CUSESBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CUSESBootstrap);

  CDACUSESBootstrap()
  {
  }

  already_AddRefd<iface::cellml_services::CUSES>
  createCUSESForModel(iface::cellml_api::Model* aModel,
                      bool aStrict)
    throw(std::exception&)
  {
    return new CDACUSES(aModel, aStrict);
  }
};

#endif // CUSESImpl_hpp
