#ifndef _Variables_hxx
#define _Variables_hxx

#include "TemporaryAnnotation.hxx"
#include "CodeGenerationError.hxx"

class VariableInformation
  : public TemporaryAnnotation
{
public:
  VariableInformation(iface::cellml_api::CellMLVariable* aSourceVariable,
                      TemporaryAnnotationKey& aKey)
    : TemporaryAnnotation(aSourceVariable, aKey), mSource(aSourceVariable),
      mFlags(0), mDegree(1), mSourceInformation(NULL)
  {
    wchar_t* tmp = aSourceVariable->name();
    mName = tmp;
    free(tmp);
  }

  VariableInformation(iface::cellml_api::CellMLVariable* aRealVariable,
                      TemporaryAnnotationKey& aKey,
                      VariableInformation* aSourceInformation)
    : TemporaryAnnotation(aRealVariable, aKey),
      mSourceInformation(aSourceInformation)
  {
  }

  enum
  {
    SUBJECT_OF_DIFF   = 0x0001,
    BVAR_OF_DIFF      = 0x0002,
    OTHER_BVAR        = 0x0004, /* e.g. definite integral */
    HAS_INITIAL_VALUE = 0x0008, /* Compatible with SUBJECT_OF_DIFF. */
    NONBVAR_USE       = 0x0010,
    PRECOMPUTED       = 0x0020, /* Set on constants and computed constants. */
    SEEN_INITIAL_VALUE= 0x0040, /* Initial value tested. */
    HAS_INITIAL_ASSIGNMENT = 0x0080 /* Initial value by name. */
  };

  enum
  {
    INDEPENDENT,
    DEPENDENT_AND_RATE,
    DEPENDENT,
    CONSTANT
  };

  void SetFlag(uint32_t flag) throw(CodeGenerationError&);
  bool IsFlagged(uint32_t flag)
  {
    if (mSourceInformation)
      return mSourceInformation->IsFlagged(flag);
    return ((mFlags & flag) != 0);
  }
  uint32_t GetDegree()
  {
    if (mSourceInformation)
      return mSourceInformation->GetDegree();
    return mDegree;
  }
  void SetDegree(uint32_t degree)
  {
    if (mSourceInformation)
      mSourceInformation->SetDegree(degree);
    else
      mDegree = degree;
  }

  void SetIndex(uint32_t index)
  {
    if (mSourceInformation)
      mSourceInformation->SetIndex(index);
    else
      mIndex = index;
  }
  uint32_t GetIndex()
  {
    if (mSourceInformation)
      return mSourceInformation->GetIndex();
    return mIndex;
  }

  void SetArray(uint32_t aArray)
  {
    if (mSourceInformation)
      mSourceInformation->SetArray(aArray);
    else
      mArrayType = aArray;
  }
  uint32_t GetArray()
  {
    if (mSourceInformation)
      return mSourceInformation->GetArray();
    return mArrayType;
  }
  void SetInitialValue(double aVal)
  {
    if (mSourceInformation)
      mSourceInformation->SetInitialValue(aVal);
    else
      mInitialValue = aVal;
  }
  double GetInitialValue()
  {
    if (mSourceInformation)
      return mSourceInformation->GetInitialValue();
    else
      return mInitialValue;
  }
  iface::cellml_api::CellMLVariable* GetSourceVariable()
  {
    if (mSourceInformation)
      return mSourceInformation->GetSourceVariable();
    else
      return mSource;
  }

  const wchar_t* GetName() { return mName.c_str(); }
private:
  iface::cellml_api::CellMLVariable* mSource;
  double mInitialValue;
  uint32_t mFlags, mDegree, mIndex;
  std::wstring mName;
  uint32_t mArrayType;
  VariableInformation* mSourceInformation;
};

#endif // _Variables_hxx
