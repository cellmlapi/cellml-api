#ifndef UNITS_HXX
#define UNITS_HXX

#include "TemporaryAnnotation.hxx"
#include "CodeGenerationError.hxx"
#include <map>

/**
 * A representation for any unit. This abstract class is used for every unit,
 * base units, and all built-in units.
 */
class CanonicalUnitRepresentation
  : iface::XPCOM::IObject
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;

  CanonicalUnitRepresentation()
    : _cda_refcount(1)
  {
  }

  virtual ~CanonicalUnitRepresentation()
  {
  }

  /**
   * True if this is a base representation, false otherwise.
   */
  virtual bool base() throw(CodeGenerationError&) = 0;

  /**
   * Returns a null terminated array of of base units, sorted by the address
   * of the pointer. The array is valid until the object is destroyed.
   * The exponents array is of the same size as the array of base units.
   * May return null if this object is not yet ready to provide this
   * information.
   */
  virtual CanonicalUnitRepresentation* const*
  getCanonicalBaseList(const double*& exponents) throw(CodeGenerationError&)
    = 0;

  /**
   * Returns the "overall factor", that is, the factor by which a value in the
   * "base units" without any multiplier/prefix needs to be multiplied to put
   * them into these units. For example, for litre, the base unit
   * representation is metre^3, so the overall factor is 1000.0
   */
  virtual double getOverallFactor() throw(CodeGenerationError&) = 0;

  /**
   * Returns the offset, that is, the amount which is added after
   * multiplication by the overall factor, to get the final value.
   */
  virtual double getOffset() throw(CodeGenerationError&) = 0;
};

/**
 * A representation for a unit scope. There are three types of unit scope:
 * 1) Global scope (for built-in units).
 * 2) Model scope (for units defined in the model).
 * 3) Component scope (for units defined in the component).
 */
class CellMLScope
  : public TemporaryAnnotation
{
public:
  CellMLScope()
    : mParentScope(NULL)
  {
  }

  CellMLScope
  (
   CellMLScope* aParentScope,
   iface::cellml_api::CellMLElement* aCellMLElement,
   const TemporaryAnnotationKey& aKey
  )
    : TemporaryAnnotation(aCellMLElement, aKey),
      mParentScope(aParentScope)
  {
  }

  ~CellMLScope()
  {
    std::map<std::wstring, CanonicalUnitRepresentation*>::iterator i;
    for (i = mUnits.begin(); i != mUnits.end(); i++)
    {
      (*i).second->release_ref();
    }
  }

  void addUnit(const wchar_t* name, CanonicalUnitRepresentation* u)
  {
    u->add_ref();
    mUnits.insert(std::pair<std::wstring, CanonicalUnitRepresentation*>
                  (name, u));
  }

  CanonicalUnitRepresentation* findUnit(const wchar_t* name)
  {
    std::map<std::wstring, CanonicalUnitRepresentation*>::iterator i =
      mUnits.find(name);
    if (i == mUnits.end())
    {
      if (mParentScope != NULL)
        return mParentScope->findUnit(name);
      return NULL;
    }
    (*i).second->add_ref();
    return (*i).second;
  }
private:
  std::map<std::wstring, CanonicalUnitRepresentation*> mUnits;
  CellMLScope* mParentScope;
};
#endif // UNITS_HXX
