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
      mFlags(0), mDegree(1)
  {
    wchar_t* tmp = aSourceVariable->name();
    mName = tmp;
    free(tmp);
  }

  enum
  {
    SUBJECT_OF_DIFF   = 0x0001,
    BVAR_OF_DIFF      = 0x0002,
    OTHER_BVAR        = 0x0004, /* e.g. definite integral */
    HAS_INITIAL_VALUE = 0x0008, /* Compatible with SUBJECT_OF_DIFF. */
    NONBVAR_USE       = 0x0010,
    PRECOMPUTED       = 0x0020  /* Set on constants and computed constants. */
  };

  enum
  {
    INDEPENDENT,
    DEPENDENT_AND_RATE,
    DEPENDENT,
    CONSTANT
  };

  void SetFlag(uint32_t flag) throw(CodeGenerationError&);
  bool IsFlagged(uint32_t flag) { return ((mFlags & flag) != 0); }
  uint32_t GetDegree() { return mDegree; }
  void SetDegree(uint32_t degree)
  {
    mDegree = degree;
  }

  void SetIndex(uint32_t index)
  {
    mIndex = index;
  }
  uint32_t GetIndex() { return mIndex; }

  void SetArray(uint32_t aArray) { mArrayType = aArray; }
  uint32_t GetArray() { return mArrayType; }
  void SetInitialValue(double aVal) { mInitialValue = aVal; }
  double GetInitialValue() { return mInitialValue; }
  iface::cellml_api::CellMLVariable* GetSourceVariable() { return mSource; }
private:
  iface::cellml_api::CellMLVariable* mSource;
  double mInitialValue;
  uint32_t mFlags, mDegree, mIndex;
  std::wstring mName;
  uint32_t mArrayType;
};

#endif // _Variables_hxx
