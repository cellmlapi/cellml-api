#include "IfaceCellML_APISPEC.hxx"
#include "Utilities.hxx"
#include <map>
#include <list>

class TeLICeMSParseTarget;

static std::wstring
convertStringToWString(const std::string& aString)
{
  wchar_t* buf = new wchar_t[aString.size() + 1];
  mbstowcs(buf, aString.c_str(), aString.size() + 1);
  std::wstring s(buf);
  delete [] buf;
  return buf;
}

template<class C>
class RefCountList
{
public:
  RefCountList()
  {
  }

  RefCountList(const RefCountList<C>& aClone)
  {
    for (typename std::list<C*>::iterator i(aClone.mList.begin());
         i != aClone.mList.end();
         i++)
    {
      (*i)->add_ref();
      mList.push_back(*i);
    }
  }

  ~RefCountList()
  {
    clear();
  }

  void
  clear()
  {
    for (typename std::list<C*>::iterator i = mList.begin();
         i != mList.end(); i++)
      (*i)->release_ref();
    mList.clear();
  }

  void
  add(C* aPtr)
  {
    aPtr->add_ref();
    mList.push_back(aPtr);
  }

  std::list<C*> mList;
};

class TeLICeMSLValue
{
public:
  TeLICeMSLValue()
    : mDifferentiator(NullType)
  {
  }

  ~TeLICeMSLValue()
  {
    release_storage();
  }

  void
  release_storage()
  {
    mString = "";
    mPropertyMap.clear();
    mMath = NULL;
    mElement = NULL;
    mMathList.clear();
  }

  void
  reactionRoleType(iface::cellml_api::Role::RoleType v)
  {
    release_storage();
    mDifferentiator = ReactionRoleType;
    mData.mReactionRoleType = v;
  }

  iface::cellml_api::Role::RoleType
  reactionRoleType() const
  {
    if (mDifferentiator != ReactionRoleType)
      throw iface::cellml_api::CellMLException();
    return mData.mReactionRoleType;
  }

  void
  reactionDirectionType(iface::cellml_api::Role::DirectionType v)
  {
    release_storage();
    mDifferentiator = ReactionDirectionType;
    mData.mReactionDirectionType = v;
  }

  iface::cellml_api::Role::DirectionType
  reactionDirectionType() const
  {
    if (mDifferentiator != ReactionDirectionType)
      throw iface::cellml_api::CellMLException();
    return mData.mReactionDirectionType;
  }

  void
  siPrefix(int prefix)
  {
    release_storage();
    mDifferentiator = SIPrefix;
    mData.mSIPrefix = prefix;
  }

  int
  siPrefix() const
  {
    if (mDifferentiator != SIPrefix)
      throw iface::cellml_api::CellMLException();
    return mData.mSIPrefix;
  }

  void
  number(double value)
  {
    release_storage();
    mDifferentiator = Number;
    mData.mNumber = value;
  }

  double number() const
  {
    if (mDifferentiator != Number)
      throw iface::cellml_api::CellMLException();
    return mData.mNumber;
  }

  void
  string(const std::string& aStr)
  {
    release_storage();
    mDifferentiator = String;
    mString = aStr;
  }

  const std::string&
  string() const
  {
    if (mDifferentiator != String)
      throw iface::cellml_api::CellMLException();
    return mString;
  }

  const std::wstring
  widestring() const
  {
    if (mDifferentiator != String)
      throw iface::cellml_api::CellMLException();
    return convertStringToWString(mString);
  }

  void
  plusstring(char c)
  {
    release_storage();
    if (mDifferentiator != String)
      throw iface::cellml_api::CellMLException();
    mString += c;
  }

  iface::cellml_api::VariableInterface
  variableInterfaceType() const
  {
    if (mDifferentiator != InterfaceDirection)
      throw iface::cellml_api::CellMLException();
    return mData.mInterfaceType;
  }

  void
  variableInterfaceType(iface::cellml_api::VariableInterface aIface)
  {
    release_storage();
    mDifferentiator = InterfaceDirection;
    mData.mInterfaceType = aIface;
  }

  bool
  boolean() const
  {
    if (mDifferentiator != Boolean)
      throw iface::cellml_api::CellMLException();
    return mData.mBoolean;
  }

  void
  boolean(bool aBool)
  {
    release_storage();
    mDifferentiator = Boolean;
    mData.mBoolean = aBool;
  }

  std::map<std::string, std::string>
  propertyMap() const
  {
    if (mDifferentiator != PropertyMap)
      throw iface::cellml_api::CellMLException();
    return mPropertyMap;
  }

  void
  propertyMap(std::map<std::string, std::string>& aMap)
  {
    release_storage();
    mDifferentiator = PropertyMap;
    mPropertyMap = aMap;
  }

  // Warning: Doesn't addref - caller expected to.
  iface::mathml_dom::MathMLContentElement*
  math()
  {
    if (mDifferentiator != Math)
      throw iface::cellml_api::CellMLException();
    return mMath;
  }

  void
  math(iface::mathml_dom::MathMLContentElement* el)
  {
    release_storage();
    mDifferentiator = Math;
    mMath = el;
  }

  void
  mathFromDOM(iface::dom::Element* el)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(mathEl, el, mathml_dom::MathMLContentElement);
    math(mathEl);
  }

  std::list<iface::mathml_dom::MathMLContentElement*>&
  mathList()
  {
    if (mDifferentiator != MathList)
      throw iface::cellml_api::CellMLException();
    return mMathList.mList;
  }

  void
  clearMathList()
  {
    mMathList.clear();
  }


  void
  makeMathList()
  {
    release_storage();
    mDifferentiator = MathList;
  }

  void
  addToMathList(iface::mathml_dom::MathMLContentElement* aEl)
  {
    mMathList.add(aEl);
  }

  void
  element(iface::cellml_api::CellMLElement* aEl)
  {
    release_storage();
    mDifferentiator = Element;
    mElement = aEl;
  }

  iface::cellml_api::CellMLElement*
  element()
  {
    if (mDifferentiator != Element)
      throw iface::cellml_api::CellMLException();
    return mElement;
  }

  bool mOverrideBuiltin;
private:
  enum {
    NullType,
    ReactionRoleType,
    ReactionDirectionType,
    SIPrefix,
    Number,
    String,
    InterfaceDirection,
    Boolean,
    PropertyMap,
    Math,
    MathList,
    Element
  } mDifferentiator;
  union {
    iface::cellml_api::Role::RoleType mReactionRoleType;
    iface::cellml_api::Role::DirectionType mReactionDirectionType;
    int mSIPrefix;
    double mNumber;
    iface::cellml_api::VariableInterface mInterfaceType;
    bool mBoolean;
  } mData;
  std::string mString;
  std::map<std::string, std::string> mPropertyMap;
  ObjRef<iface::mathml_dom::MathMLContentElement> mMath;
  ObjRef<iface::cellml_api::CellMLElement> mElement;
  RefCountList<iface::mathml_dom::MathMLContentElement> mMathList;
};

class TeLICeMStateScan
  : public TeLICeMFlexLexer
{
public:
  TeLICeMStateScan(std::stringstream* aStr, TeLICeMSParseTarget* aTarg);

  int yylex();
  void adjustColumn();
  void newRow();

  TeLICeMSParseTarget* mTarget;
  bool mThisLexIsNewRow;
  bool mEOF;
  TeLICeMSLValue* mLValue;

  void lexerError(const char* m);
};
