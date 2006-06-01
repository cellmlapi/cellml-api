#include "Units.hxx"
#include <math.h>
#include "CodeGenerationState.hxx"

/**
 * A representation for one of the 7 built in base units...
 */
class ImplicitBaseUnit
  : public CanonicalUnitRepresentation
{
public:
  ImplicitBaseUnit()
  {
    mExponents[0] = 1.0;
    mUnits[0] = this;
    mUnits[1] = NULL;
  }

  bool base()
    throw(CodeGenerationError&)
  {
    return true;
  }

  CanonicalUnitRepresentation* const*
  getCanonicalBaseList(const double*& exponents)
    throw(CodeGenerationError&)
  {
    exponents = mExponents;
    return mUnits;
  }

  double
  getOverallFactor()
    throw(CodeGenerationError&)
  {
    return 1.0;
  }

  double
  getOffset()
    throw(CodeGenerationError&)
  {
    return 0.0;
  }
private:
  CanonicalUnitRepresentation* mUnits[2];
  double mExponents[1];
};

struct UnitSortData
{
  uint32_t idx;
  CanonicalUnitRepresentation* const* mUnits;
};

int CompareSortData(const void* a1, const void* a2)
{
  UnitSortData* s1 = (UnitSortData*)a1;
  UnitSortData* s2 = (UnitSortData*)a2;
  return ((uint32_t)s1->mUnits[s1->idx]) - ((uint32_t)s2->mUnits[s2->idx]);
}

/**
 * A representation for a built-in derived unit.
 * Warning: This does *not* add_ref the units, it instead assumes that the
 * global scope will outlive the ImplicitDerivedUnit. This saves lots of
 * add_refs.
 */
class ImplicitDerivedUnit
  : public CanonicalUnitRepresentation
{
public:
  ImplicitDerivedUnit(const double* aExponents,
                      CanonicalUnitRepresentation* const* aUnits,
                      double aFactor = 1.0, double aOffset=0.0)
    : CanonicalUnitRepresentation(), mFactor(aFactor), mOffset(aOffset)
  {
    // We have to sort the units array, because we can't guarantee they are in
    // the correct canonical order (and if they aren't, this will confuse the
    // efficient compare/merge code).
    CanonicalUnitRepresentation* const* unitEnd;
    for (unitEnd = aUnits; *unitEnd != NULL; unitEnd++)
      ;
    uint32_t unitCount = unitEnd - aUnits;
    UnitSortData sortData[unitCount];
    uint32_t i;
    for (i = 0; i < unitCount; i++)
    {
      sortData[i].idx = i;
      sortData[i].mUnits = aUnits;
    }

    // Now compute the permutation. If we wrote our own version of qsort, we
    // could directly manipulate the two arrays, but otherwise we don't have
    // control over how swapping happens.
    qsort(sortData, unitCount, sizeof(UnitSortData), CompareSortData);

    // We don't expect the array to be huge, so better just copy it rather than
    // use a permutation in place algorithm...
    mExponents = new double[unitCount];
    mUnits = new CanonicalUnitRepresentation*[unitCount + 1];
    for (i = 0; i < unitCount; i++)
    {
      mExponents[i] = aExponents[sortData[i].idx];
      mUnits[i] = aUnits[sortData[i].idx];
    }
    mUnits[unitCount] = NULL;
  }

  ~ImplicitDerivedUnit()
  {
    delete [] mExponents;
    delete [] mUnits;
  }

  bool base()
    throw(CodeGenerationError&)
  {
    return false;
  }

  CanonicalUnitRepresentation* const*
  getCanonicalBaseList(const double*& exponents)
    throw(CodeGenerationError&)
  {
    exponents = mExponents;
    return mUnits;
  }

  double
  getOverallFactor()
    throw(CodeGenerationError&)
  {
    return mFactor;
  }

  double
  getOffset()
    throw(CodeGenerationError&)
  {
    return mOffset;
  }
private:
  double* mExponents;
  CanonicalUnitRepresentation** mUnits;
  double mFactor, mOffset;
};

class CellMLScope;

/**
 * A representation for a user-defined unit...
 */
class CellMLUnitDefinition
  : public CanonicalUnitRepresentation
{
public:
  CellMLUnitDefinition
  (
   CellMLScope* aScope,
   iface::cellml_api::Units* aUnits
  )
    : mFactor(1.0), mOffset(0.0), mCellML(aUnits), mScope(aScope),
      mIsBase(aUnits->isBaseUnits()), mIsReentrant(false), mUnitsCapacity(0),
      mExponents(NULL), mUnits(NULL)
  {
  }

  ~CellMLUnitDefinition()
  {
    if (mExponents != NULL)
      delete [] mExponents;
    if (mUnits != NULL)
      delete [] mUnits;
  }

  void ComputeBaseUnits() throw(CodeGenerationError&);

  bool base()
    throw(CodeGenerationError&)
  {
    return mIsBase;
  }

  CanonicalUnitRepresentation* const*
  getCanonicalBaseList(const double*& exponents)
    throw(CodeGenerationError&)
  {
    if (mUnits == NULL)
      ComputeBaseUnits();
    exponents = mExponents;
    return mUnits;
  }

  double
  getOverallFactor()
    throw(CodeGenerationError&)
  {
    if (mUnits == NULL)
      ComputeBaseUnits();
    return mFactor;
  }

  double
  getOffset()
    throw(CodeGenerationError&)
  {
    if (mUnits == NULL)
      ComputeBaseUnits();
    return mOffset;
  }
private:
  double mFactor, mOffset;
  // Not a strong reference, because the units element belongs to the same
  // refcount pool as the variable element, which holds a strong reference
  // to the scope user data, which in turn references this C.U.D. We instead
  // assume that the model won't change, and since part of the model was passed
  // in up the stack (so the caller holds a reference), we know the whole
  // model won't disappear underneath us. Likewise, the scope is owned by the
  // CellML element tree, so the same logic applies.
  iface::cellml_api::Units* mCellML;
  CellMLScope* mScope;
  bool mIsBase, mIsReentrant;
  size_t mUnitsCapacity;
  double* mExponents;
  CanonicalUnitRepresentation** mUnits;

  void EnsureCapacity(size_t minCap)
  {
    if (mUnitsCapacity == 0)
    {
      // There are rarely more than 7 base units + 1 terminating null (SI only
      // defines seven, so there can only be more if there are user defined
      // base units).
      mUnits = new CanonicalUnitRepresentation*[8];
      mExponents = new double[8];
      mUnitsCapacity = 8;
    }

    if (mUnitsCapacity >= minCap)
      return;
    size_t mOldCapacity = mUnitsCapacity;
    while (mUnitsCapacity < minCap)
      mUnitsCapacity *= 2;
    CanonicalUnitRepresentation** mUnitsOld = mUnits;
    double* mExponentsOld = mExponents;
    mExponents = new double[mUnitsCapacity];
    mUnits = new CanonicalUnitRepresentation*[mUnitsCapacity];
    memcpy(mExponents, mExponentsOld, mOldCapacity * sizeof(double));
    delete [] mExponentsOld;
    memcpy(mUnits, mUnitsOld,
           mOldCapacity * sizeof(CanonicalUnitRepresentation*));
    delete [] mUnitsOld;
  }
};

class CellMLImportUnitDefinition
  : public CanonicalUnitRepresentation
{
public:
  CellMLImportUnitDefinition
  (
   iface::cellml_api::ImportUnits* aUnits,
   TemporaryAnnotationKey& aScopeKey
  )
    : mUnits(aUnits), mScopeKey(aScopeKey)
  {
  }

  ~CellMLImportUnitDefinition()
  {
  }

  bool base()
    throw(CodeGenerationError&)
  {
    if (!mBacking)
      FetchBacking();
    return mBacking->base();
  }

  CanonicalUnitRepresentation* const*
  getCanonicalBaseList(const double*& exponents)
    throw(CodeGenerationError&)
  {
    if (!mBacking)
      FetchBacking();
    return mBacking->getCanonicalBaseList(exponents);
  }

  double
  getOverallFactor()
    throw(CodeGenerationError&)
  {
    if (!mBacking)
      FetchBacking();
    return mBacking->getOverallFactor();
  }

  double
  getOffset()
    throw(CodeGenerationError&)
  {
    if (!mBacking)
      FetchBacking();
    return mBacking->getOffset();
  }
private:
  void FetchBacking();

  // No need to hold a reference to mUnits, because the model is held further
  // up the stack than the state class.
  iface::cellml_api::ImportUnits* mUnits;
  CanonicalUnitRepresentation* mBacking;
  TemporaryAnnotationKey mScopeKey;

};

struct ExponentUnitQueue
{
  const double* nextExponent;
  CanonicalUnitRepresentation* const* nextUnit;
  double commonExponent;
};

void
CellMLUnitDefinition::ComputeBaseUnits()
  throw(CodeGenerationError&)
{
  if (mIsBase)
  {
    mUnits = new CanonicalUnitRepresentation*[2];
    mExponents = new double[1];
    mUnits[0] = this;
    mUnits[1] = NULL;
    mExponents[0] = 1.0;
    mFactor = 1.0;
    mOffset = 0.0;
    return;
  }
  
  if (mIsReentrant)
  {
    std::wstring emsg = L"Circular definition of units: ";
    RETURN_INTO_WSTRING(name, mCellML->name());
    emsg += name;
    throw CodeGenerationError(emsg);
  }
  mIsReentrant = true;
  
  // Not a base unit. We call down the chain, and this will result in
  // recursive computation of any referenced uncomputed units.
  RETURN_INTO_OBJREF(uc, iface::cellml_api::UnitSet,
                     mCellML->unitCollection());
  RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitIterator, uc->iterateUnits());
  std::list<CanonicalUnitRepresentation*> parentReps;
  bool offsetEligible = true;
  std::list<ExponentUnitQueue> unitMultiplexer;
  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Unit, ui->nextUnit());
    if (u == NULL)
      break;
    RETURN_INTO_WSTRING(uname, u->units());
    // Lookup in the scope...
    RETURN_INTO_OBJREF(ur, CanonicalUnitRepresentation,
                       mScope->findUnit(uname.c_str()));
    if (ur == NULL)
    {
      std::wstring em = L"Unit element references units ";
      em += uname;
      em += L", but no units with this name were found.";
      throw CodeGenerationError(em);
    }
    // See if there is an offset...
    bool offset = u->offset() + ur->getOffset();

    if (offset != 0.0)
    {
      mOffset = offset;
    }

    if (mOffset != 0.0 && !offsetEligible)
    {
      std::wstring em = L"Units element has more than one unit child, but ";
      switch (((u->offset() == 0.0) ? 0 : 1) +
              ((ur->getOffset() == 0.0) ? 0 : 2))
      {
      case 1:
        em += L"has a unit element with the offset element";
        break;
      case 2:
        em += L"references a unit defined with an offset";
        break;
      case 3:
        em += L"has a unit element with the offset element (and also "
          L"references a unit defined with an offset)";
        break;
      }
      em += L" in units definition of ";
      em += uname;
      throw CodeGenerationError(em);
    }
    offsetEligible = false;

    // Now multiply the prefix...
    mFactor /= u->multiplier() *
      pow(ur->getOverallFactor() * pow(10.0, u->prefix()), u->exponent());

    // Push the prefix onto the multiplexer...
    ExponentUnitQueue entry;
    entry.commonExponent = u->exponent();
    entry.nextUnit = ur->getCanonicalBaseList(entry.nextExponent);
    unitMultiplexer.push_back(entry);
  }
  
  size_t size = 0;
  double nextExponent = 0.0; // initialised to suppress false warning.
  std::list<ExponentUnitQueue>::iterator i;
  while (true)
  {
    CanonicalUnitRepresentation* nextUnit = NULL;
    // We are merging multiple sorted lists, with the possibility of duplicates
    // (which have to be merged).
    for (i = unitMultiplexer.begin(); i != unitMultiplexer.end(); i++)
    {
      // Should we clear these from the list?
      if (*((*i).nextUnit) == NULL)
        continue;
      if (nextUnit == NULL ||
          ((unsigned int)(*((*i).nextUnit))) < (unsigned int)nextUnit)
      {
        nextUnit = *((*i).nextUnit);
        nextExponent = *((*i).nextExponent) * (*i).commonExponent;
      }
      else if (nextUnit == *((*i).nextUnit))
      {
        nextExponent += *((*i).nextExponent) * (*i).commonExponent;
      }
    }
    // Break out if we are done...
    if (nextUnit == NULL)
      break;
    
    // Make sure we have space for the unit, plus a terminating null...
    EnsureCapacity(size + 2);
    mUnits[size] = nextUnit;
    mExponents[size] = nextExponent;

    size++;

    // Advance all pointers to nextUnit/nextExponent...
    for (i = unitMultiplexer.begin(); i != unitMultiplexer.end(); i++)
    {
      if ((!(*i).nextUnit) || ((*((*i).nextUnit)) != nextUnit))
        continue;
      (*i).nextUnit++;
      (*i).nextExponent++;
    }
  }
  // Put the terminating NULL on...

  mIsReentrant = false;
  mUnits[size] = NULL;
}

void
CodeGenerationState::SetupBuiltinUnits()
{
#define BASE_UNIT(x) \
  ImplicitBaseUnit* ibu_##x = new ImplicitBaseUnit(); \
  mGlobalScope.addUnit(L## #x, ibu_##x); \
  ibu_##x->release_ref();
  BASE_UNIT(ampere);
  BASE_UNIT(candela);
  BASE_UNIT(kelvin);
  BASE_UNIT(kilogram);
  BASE_UNIT(metre);
  BASE_UNIT(mole);
  BASE_UNIT(second);
#define DERIVED_UNIT0(n, f) \
  CanonicalUnitRepresentation* ibuarr_##n[] = {NULL}; \
  const double exparr_##n[] = {}; \
  ImplicitDerivedUnit* idu_##n = new ImplicitDerivedUnit(exparr_##n, \
                                                         ibuarr_##n, f); \
  mGlobalScope.addUnit(L## #n, idu_##n);\
  idu_##n->release_ref()
#define DERIVED_UNIT1(n, lb1, le1, f) \
  CanonicalUnitRepresentation* ibuarr_##n[] = {lb1, NULL}; \
  const double exparr_##n[] = {le1}; \
  ImplicitDerivedUnit* idu_##n = new ImplicitDerivedUnit(exparr_##n, \
                                                         ibuarr_##n, f); \
  mGlobalScope.addUnit(L## #n, idu_##n); \
  idu_##n->release_ref()
#define DERIVED_UNIT2(n, lb1, le1, lb2, le2, f) \
  CanonicalUnitRepresentation* ibuarr_##n[] = {lb1, lb2, NULL}; \
  const double exparr_##n[] = {le1, le2}; \
  ImplicitDerivedUnit* idu_##n = new ImplicitDerivedUnit(exparr_##n, \
                                                         ibuarr_##n, f); \
  mGlobalScope.addUnit(L## #n, idu_##n); \
  idu_##n->release_ref()
#define DERIVED_UNIT3(n, lb1, le1, lb2, le2, lb3, le3, f) \
  CanonicalUnitRepresentation* ibuarr_##n[] = {lb1, lb2, lb3, NULL}; \
  const double exparr_##n[] = {le1, le2, le3}; \
  ImplicitDerivedUnit* idu_##n = new ImplicitDerivedUnit(exparr_##n, \
                                                         ibuarr_##n, f); \
  mGlobalScope.addUnit(L## #n, idu_##n); \
  idu_##n->release_ref()
#define DERIVED_UNIT4(n, lb1, le1, lb2, le2, lb3, le3, lb4, le4, f) \
  CanonicalUnitRepresentation* ibuarr_##n[] = {lb1, lb2, lb3, lb4, NULL}; \
  const double exparr_##n[] = {le1, le2, le3, le4}; \
  ImplicitDerivedUnit* idu_##n = new ImplicitDerivedUnit(exparr_##n, \
                                                         ibuarr_##n, f); \
  mGlobalScope.addUnit(L## #n, idu_##n); \
  idu_##n->release_ref()

  // SI derived units...
  DERIVED_UNIT0(radian, 1.0);
  DERIVED_UNIT0(steradian, 1.0);
  DERIVED_UNIT1(hertz, ibu_second, -1.0, 1.0);
  DERIVED_UNIT3(newton, ibu_metre, 1.0, ibu_kilogram, 1.0, ibu_second, -2.0,
                1.0);
  DERIVED_UNIT3(pascal, ibu_metre, -1.0, ibu_kilogram, 1.0, ibu_second, -2.0,
                1.0);
  DERIVED_UNIT3(joule, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -2.0,
                1.0);
  DERIVED_UNIT3(watt, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -3.0,
                1.0);
  DERIVED_UNIT2(columb, ibu_second, 1.0, ibu_ampere, 1.0, 1.0);
  DERIVED_UNIT4(volt, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -3.0,
                ibu_ampere, -1.0, 1.0);
  DERIVED_UNIT4(farad, ibu_metre, -2.0, ibu_kilogram, -1.0, ibu_second, 4.0,
                ibu_ampere, 2.0, 1.0);
  DERIVED_UNIT4(ohm, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -3.0,
                ibu_ampere, -2.0, 1.0);
  DERIVED_UNIT4(siemens, ibu_metre, -2.0, ibu_kilogram, -1.0, ibu_second,
                3.0, ibu_ampere, 2.0, 1.0);
  DERIVED_UNIT4(weber, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -2.0,
                ibu_ampere, -1.0, 1.0);
  DERIVED_UNIT3(tesla, ibu_kilogram, 1.0, ibu_second, -2.0, ibu_ampere, -1.0,
                1.0);
  DERIVED_UNIT4(henry, ibu_metre, 2.0, ibu_kilogram, 1.0, ibu_second, -2.0,
                ibu_ampere, -2.0, 1.0);
  DERIVED_UNIT1(lumen, ibu_candela, 1.0, 1.0);
  DERIVED_UNIT2(lux, ibu_metre, -2.0, ibu_candela, 1.0, 1.0);
  DERIVED_UNIT1(becquerel, ibu_second, -1.0, 1.0);
  DERIVED_UNIT2(gray, ibu_metre, 2.0, ibu_second, -2.0, 1.0);
  DERIVED_UNIT2(sievert, ibu_metre, 2.0, ibu_second, -2.0, 1.0);
  DERIVED_UNIT2(katal, ibu_second, -1.0, ibu_mole, 1.0, 1.0);
  // Non-SI derived units...
  DERIVED_UNIT0(dimensionless, 1.0);
  DERIVED_UNIT1(gram, ibu_kilogram, 1.0, 1000.0);
  DERIVED_UNIT1(litre, ibu_metre, 3.0, 1000.0);
  // American spellings...
  DERIVED_UNIT1(meter, ibu_metre, 1.0, 1.0);
  DERIVED_UNIT1(liter, ibu_metre, 3.0, 1000.0);
  // Celsius is special because it has an offset...
  CanonicalUnitRepresentation* ibuarr_celsius[] = {ibu_kelvin, NULL};
  const double exparr_celsius[] = {1.0};
  ImplicitDerivedUnit* idu_celsius = new ImplicitDerivedUnit
    (
     exparr_celsius, ibuarr_celsius, 1.0, 273.15
    );
  mGlobalScope.addUnit(L"celsius", idu_celsius);
  idu_celsius->release_ref();
}

void
CellMLImportUnitDefinition::FetchBacking()
{
  // Get the name...
  RETURN_INTO_WSTRING(name, mUnits->name());
  
  // Get the import...
  RETURN_INTO_OBJREF(impel, iface::cellml_api::CellMLElement,
                     mUnits->parentElement());
  if (impel == NULL)
    throw CodeGenerationError(L"Import unit has no parent!");
  DECLARE_QUERY_INTERFACE_OBJREF(import,  impel,
                                 cellml_api::CellMLImport);
  if (impel == NULL)
    throw CodeGenerationError(L"Import unit parent isn't import!");
  RETURN_INTO_OBJREF(mod, iface::cellml_api::Model, import->importedModel());
  if (mod == NULL)
    throw CodeGenerationError(L"Can't find model corresponding to import "
                              L"element.");
  RETURN_INTO_OBJREF(udo, iface::XPCOM::IObject,
                     mod->getUserData(mScopeKey.str().c_str()));
  if (udo == NULL)
    throw CodeGenerationError(L"Can't get scope from model, something is "
                              L"inconsistent.");
  CellMLScope* ctxt = dynamic_cast<CellMLScope*>(udo.getPointer());
  mBacking = ctxt->findUnit(name.c_str());
  // Let the context hold it, it isn't going away until after us.
  mBacking->release_ref();
}

void
CodeGenerationState::PutUnitsIntoScope
(
 CellMLScope* aScope, iface::cellml_api::Units* aUnits
)
{
  RETURN_INTO_WSTRING(name, aUnits->name());
  RETURN_INTO_OBJREF(eu, CanonicalUnitRepresentation,
                     aScope->findUnit(name.c_str()));
  if (eu != NULL)
  {
    std::wstring msg = L"Defining unit ";
    msg += name;
    msg += L" creates a name conflict.";
    throw CodeGenerationError(msg);
  }
  
  RETURN_INTO_OBJREF(ud, CellMLUnitDefinition,
                     new CellMLUnitDefinition(aScope, aUnits));
  aScope->addUnit(name.c_str(), ud);
}
