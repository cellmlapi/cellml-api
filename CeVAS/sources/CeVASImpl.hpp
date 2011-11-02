#ifndef _CeVASImplementation_hpp
#define _CeVASImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeVAS.hxx"
#include "Utilities.hxx"
#include <sstream>
#include <vector>
#include <list>
#include <map>

template<class C> class CleanupList
  : public std::list<C>
{
public:
  ~CleanupList()
  {
    typename CleanupList<C>::iterator i;
    for (i = CleanupList<C>::begin(); i != CleanupList<C>::end(); i++)
      (*i)->release_ref();
  }
};

template<class C> class AutoList
  : public std::list<C>
{
public:
  ~AutoList()
  {
    typename AutoList<C>::iterator i;
    for (i = AutoList<C>::begin(); i != AutoList<C>::end(); i++)
      delete (*i);
  }
};

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

class CDAConnectedVariableSet
  : public iface::cellml_services::ConnectedVariableSet
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::ConnectedVariableSet);
  CDA_IMPL_REFCOUNT;

  CDAConnectedVariableSet(iface::cellml_api::CellMLVariable*) throw();
  ~CDAConnectedVariableSet() throw();

  already_AddRefd<iface::cellml_api::CellMLVariable> sourceVariable() throw(std::exception&);
  uint32_t length() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> getVariable(uint32_t aIndex)
    throw(std::exception&);

  void addVariable(iface::cellml_api::CellMLVariable* v);

private:
  // Not refcounted, because the variable is also on the list (which is
  // refcounted).
  iface::cellml_api::CellMLVariable* mSource;
  CleanupVector<iface::cellml_api::CellMLVariable*> mVariables;
};

class CDACeVAS
  : public iface::cellml_services::CeVAS
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CeVAS);
  CDA_IMPL_REFCOUNT;

  CDACeVAS(iface::cellml_api::Model* aModel) throw();
  ~CDACeVAS() throw();

  std::wstring modelError() throw();
  already_AddRefd<iface::cellml_api::CellMLComponentIterator> iterateRelevantComponents()
    throw(std::exception&);
  already_AddRefd<iface::cellml_services::ConnectedVariableSet> findVariableSet(
    iface::cellml_api::CellMLVariable* aVariable)
    throw(std::exception&);
  uint32_t length() throw(std::exception&);
  already_AddRefd<iface::cellml_services::ConnectedVariableSet> getVariableSet(uint32_t aIndex)
    throw(std::exception&);

private:
  void ComputeConnectedVariables(std::list<iface::cellml_api::Model*>&);

  std::wstring mErrorDescription;
  CleanupList<iface::cellml_api::CellMLComponent*> mRelevantComponents;
  typedef std::map<iface::cellml_api::CellMLVariable*,
                   CDAConnectedVariableSet*,XPCOMComparator> maptype;
  CleanupVector<CDAConnectedVariableSet*> mSetList;
  maptype mVariableSets;
};

class CDACeVASBootstrap
  : public iface::cellml_services::CeVASBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CeVASBootstrap);
  CDA_IMPL_REFCOUNT;

  CDACeVASBootstrap()
    throw()
  {}

  ~CDACeVASBootstrap() throw() {}

  already_AddRefd<iface::cellml_services::CeVAS>
  createCeVASForModel(iface::cellml_api::Model* aModel)
    throw(std::exception&)
  {
    return new CDACeVAS(aModel);
  }
};

#endif // _CeVASImplementation_hpp
