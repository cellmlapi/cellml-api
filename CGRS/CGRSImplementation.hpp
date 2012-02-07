#include "CGRSBootstrap.hpp"
#include "IfaceCGRS.hxx"
#include <map>

class CDA_GenericPrimitiveType
  : public iface::CGRS::GenericType
{
public:
  CDA_GenericPrimitiveType(const std::string& aTypeName) : mTypeName(aTypeName) {}
  ~CDA_GenericPrimitiveType() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericType);

  std::string asString() throw(std::exception&) { return mTypeName; }

private:
  std::string mTypeName;
};

class CDA_GenericVoidValue
  : public iface::CGRS::GenericValue
{
public:
  CDA_GenericVoidValue() {}
  ~CDA_GenericVoidValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericValue);

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw();
};

class CDA_GenericsService
  : public iface::CGRS::GenericsService
{
public:
  CDA_GenericsService();
  ~CDA_GenericsService() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericsService);

  CGRS_PUBLIC_PRE void registerType(iface::CGRS::GenericType* aType) CGRS_PUBLIC_POST;
  CGRS_PUBLIC_PRE already_AddRefd<iface::CGRS::GenericType> getTypeByName(const std::string& aName) throw(iface::CGRS::CGRSError&) CGRS_PUBLIC_POST;

  void loadGenericModule(const std::string& aModulePath) throw(std::exception&);
  CGRS_PUBLIC_PRE void registerBootstrap(const std::string& aBootstrapName, iface::CGRS::GenericValue* aValue) throw(std::exception&) CGRS_PUBLIC_POST;
  already_AddRefd<iface::CGRS::GenericValue> getBootstrapByName(const std::string& aBootstrapName) throw(std::exception&);
  CGRS_PUBLIC_PRE void registerInterface(const std::string& aBootstrapName, iface::CGRS::GenericInterface* aIface) throw(std::exception&) CGRS_PUBLIC_POST;
  already_AddRefd<iface::CGRS::GenericInterface> getInterfaceByName(const std::string& aInterfaceName) throw(std::exception&);
  already_AddRefd<iface::CGRS::StringValue> makeString(const std::string& val) throw(std::exception&);
  already_AddRefd<iface::CGRS::WStringValue> makeWString(const std::wstring& val) throw(std::exception&);
  already_AddRefd<iface::CGRS::ShortValue> makeShort(int16_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::LongValue> makeLong(int32_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::LongLongValue> makeLongLong(int64_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::UShortValue> makeUShort(uint16_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::ULongValue> makeULong(uint32_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::ULongLongValue> makeULongLong(uint64_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::FloatValue> makeFloat(float val) throw(std::exception&);
  already_AddRefd<iface::CGRS::DoubleValue> makeDouble(double val) throw(std::exception&);
  already_AddRefd<iface::CGRS::BooleanValue> makeBoolean(bool val) throw(std::exception&);
  already_AddRefd<iface::CGRS::CharValue> makeChar(char val) throw(std::exception&);
  already_AddRefd<iface::CGRS::OctetValue> makeOctet(uint8_t val) throw(std::exception&);
  already_AddRefd<iface::CGRS::SequenceValue> makeSequence(iface::CGRS::GenericType* innerType) throw(std::exception&);
  CGRS_PUBLIC_PRE already_AddRefd<iface::CGRS::SequenceType> makeSequenceType(iface::CGRS::GenericType* innerType) throw(std::exception&) CGRS_PUBLIC_POST;
  already_AddRefd<iface::CGRS::EnumValue> makeEnumFromString(iface::CGRS::EnumType* etype, const std::string& name) throw(std::exception&);
  already_AddRefd<iface::CGRS::EnumValue> makeEnumFromIndex(iface::CGRS::EnumType* etype, int32_t index) throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericValue> makeObject(iface::XPCOM::IObject* value) throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericValue> makeVoid() throw(std::exception&);

private:
  std::map<std::string, iface::CGRS::GenericType*> mTypeRegistry;
  std::map<std::string, iface::CGRS::GenericValue*> mValueRegistry;
  std::map<std::string, iface::CGRS::GenericInterface*> mInterfaceRegistry;
  CDA_GenericPrimitiveType mStringType, mWStringType, mShortType, mLongType, mLongLongType,
    mUShortType, mULongType, mULongLongType, mFloatType, mDoubleType, mBooleanType, mCharType,
    mOctetType, mIObjectType, mVoidType;
  CDA_GenericVoidValue mVoid;
};

CGRS_PUBLIC_PRE class CGRS_PUBLIC_POST CDA_GenericInterfaceBase
  : public iface::CGRS::GenericInterface
{
public:
  virtual void* makeCallbackProxy(iface::CGRS::CallbackObjectValue* aObjectValue) = 0;
};

#if 0 // This is what generated implementations look like...
class CDA_GenericInterface
  : public CDA_GenericInterfaceBase
{
public:
  CDA_GenericInterface() {}
  ~CDA_GenericInterface() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericInterface);

  int32_t baseCount() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericInterface>  getBase(int32_t aBaseNumber) throw(std::exception&);
  int32_t attributeCount() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericAttribute>  getAttributeByIndex(int32_t aAttributeNumber) throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericAttribute>  getAttributeByName(const std::string& aAttributeName) throw(std::exception&);
  int32_t operationCount() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericMethod>  getOperationByIndex(int32_t aOperationNumber) throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericMethod>  getOperationByName(const std::string& aOperationName) throw(std::exception&);
};
#endif

class CDA_GenericSequenceType
  : public iface::CGRS::SequenceType
{
public:
  CDA_GenericSequenceType(iface::CGRS::GenericType* aInnerType) : mInnerType(aInnerType) {};
  ~CDA_GenericSequenceType() {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::SequenceType, CGRS::GenericType);

  std::string
  asString()
    throw(std::exception&)
  {
    return "sequence<" + mInnerType->asString() + ">";
  }

  already_AddRefd<iface::CGRS::GenericType>
  innerType()
    throw(std::exception&)
  {
    mInnerType->add_ref();
    return mInnerType.getPointer();
  }

private:
  ObjRef<iface::CGRS::GenericType> mInnerType;
};

class CDA_GenericEnumType
  : public iface::CGRS::EnumType
{
public:
  CDA_GenericEnumType(char* aName, char** aList, size_t aListSize)
    : mName(aName), mMaxIndex(aListSize - 1)
  {
    for (size_t i = 0; i < aListSize; i++)
    {
      mForwardMap.insert(std::pair<std::string, int32_t>(aList[i], i));
      mReverseMap.insert(std::pair<int32_t, std::string>(i, aList[i]));
    }
  }
  ~CDA_GenericEnumType() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::EnumType, CGRS::GenericType);

  std::string asString() throw(std::exception&) { return mName; }
  int32_t maxIndex() throw(std::exception&) { return mMaxIndex; }
  std::string indexToName(int32_t index) throw(std::exception&)
  {
    std::map<int32_t, std::string>::iterator i = mReverseMap.find(index);
    if (i == mReverseMap.end())
      throw iface::CGRS::CGRSError();

    return (*i).second;
  }
  int32_t nameToIndex(const std::string& name) throw(std::exception&)
  {
    std::map<std::string, int32_t>::iterator i = mForwardMap.find(name);
    if (i == mForwardMap.end())
      throw iface::CGRS::CGRSError();

    return (*i).second;
  }

private:
  const char* mName;
  std::map<std::string, int32_t> mForwardMap;
  std::map<int32_t, std::string> mReverseMap;
  int32_t mMaxIndex;
};

class CDA_GenericStringValue
  : public iface::CGRS::StringValue
{
public:
  CDA_GenericStringValue(const std::string& aValue) : mValue(aValue) {}
  ~CDA_GenericStringValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::StringValue);

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  std::string asString() throw() { return mValue; }
  void asString(const std::string& attr) throw() { mValue = attr; }

private:
  std::string mValue;
};

class CDA_GenericWStringValue
  : public iface::CGRS::WStringValue
{
public:
  CDA_GenericWStringValue(const std::wstring& aValue) : mValue(aValue) {}
  ~CDA_GenericWStringValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::WStringValue)

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  std::wstring asWString() throw(std::exception&) { return mValue; }
  void asWString(const std::wstring& attr) throw(std::exception&) { mValue = attr; }

private:
  std::wstring mValue;
};

class CDA_GenericShortValue
  : public iface::CGRS::ShortValue
{
public:
  CDA_GenericShortValue(int16_t aValue) : mValue(aValue) {}
  ~CDA_GenericShortValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::ShortValue);

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  int16_t asShort() throw(std::exception&) { return mValue; }
  void asShort(int16_t aValue) throw(std::exception&) { mValue = aValue; }

private:
  int16_t mValue;
};

class CDA_GenericLongValue
  : public iface::CGRS::LongValue
{
public:
  CDA_GenericLongValue(int32_t aValue) : mValue(aValue) {}
  ~CDA_GenericLongValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::LongValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  int32_t asLong() throw(std::exception&) { return mValue; }
  void asLong(int32_t attr) throw(std::exception&) { mValue = attr; }

private:
  int32_t mValue;
};

class CDA_GenericLongLongValue
  : public iface::CGRS::LongLongValue
{
public:
  CDA_GenericLongLongValue(int64_t aValue) : mValue(aValue) {}
  ~CDA_GenericLongLongValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::LongLongValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  int64_t asLongLong() throw(std::exception&) { return mValue; }
  void asLongLong(int64_t attr) throw(std::exception&) { mValue = attr; }

private:
  int64_t mValue;
};

class CDA_GenericUShortValue
  : public iface::CGRS::UShortValue
{
public:
  CDA_GenericUShortValue(uint16_t aAttr) : mAttr(aAttr) {}
  ~CDA_GenericUShortValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::UShortValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  uint16_t asUShort() throw(std::exception&) { return mAttr; }
  void asUShort(uint16_t attr) throw(std::exception&) { mAttr = attr; }

private:
  uint16_t mAttr;
};

class CDA_GenericULongValue
  : public iface::CGRS::ULongValue
{
public:
  CDA_GenericULongValue(uint32_t aValue) : mValue(aValue) {}
  ~CDA_GenericULongValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::ULongValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  uint32_t asULong() throw(std::exception&) { return mValue; }
  void asULong(uint32_t aValue) throw(std::exception&) { mValue = aValue; }

private:
  uint32_t mValue;
};

class CDA_GenericULongLongValue
  : public iface::CGRS::ULongLongValue
{
public:
  CDA_GenericULongLongValue(int64_t aValue) : mValue(aValue) {}
  ~CDA_GenericULongLongValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::ULongLongValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  uint64_t asULongLong() throw(std::exception&) { return mValue; }
  void asULongLong(uint64_t attr) throw(std::exception&) { mValue = attr; }

private:
  uint64_t mValue;
};

class CDA_GenericFloatValue
  : public iface::CGRS::FloatValue
{
public:
  CDA_GenericFloatValue(float aValue) : mValue(aValue) {}
  ~CDA_GenericFloatValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::FloatValue)

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  float asFloat() throw(std::exception&) { return mValue; }
  void asFloat(float aValue) throw(std::exception&) { mValue = aValue; }

private:
  float mValue;
};

class CDA_GenericDoubleValue
  : public iface::CGRS::DoubleValue
{
public:
  CDA_GenericDoubleValue(double aValue) : mValue(aValue) {}
  ~CDA_GenericDoubleValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::DoubleValue)

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  double asDouble() throw(std::exception&) { return mValue; }
  void asDouble(double aValue) throw(std::exception&) { mValue = aValue; }

private:
  double mValue;
};

class CDA_GenericBooleanValue
  : public iface::CGRS::BooleanValue
{
public:
  CDA_GenericBooleanValue(bool aValue) : mValue(aValue) {}
  ~CDA_GenericBooleanValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::BooleanValue);

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);
  bool asBoolean() throw(std::exception&) { return mValue; }
  void asBoolean(bool attr) throw(std::exception&) { mValue = attr; }

private:
  bool mValue;
};

class CDA_GenericCharValue
  : public iface::CGRS::CharValue
{
public:
  CDA_GenericCharValue(char aValue) : mValue(aValue) {}
  ~CDA_GenericCharValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::CharValue);

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  char asChar() throw(std::exception&) { return mValue; }
  void asChar(char c) throw(std::exception&) { mValue = c; }

private:
  char mValue;
};

class CDA_GenericOctetValue
  : public iface::CGRS::OctetValue
{
public:
  CDA_GenericOctetValue(uint8_t aValue) : mValue(aValue) {}
  ~CDA_GenericOctetValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::OctetValue);

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  uint8_t asOctet() throw(std::exception&) { return mValue; }
  void asOctet(uint8_t c) throw(std::exception&) { mValue = c; }

private:
  uint8_t mValue;
};

class CDA_GenericObjectValue
  : public iface::CGRS::ObjectValue
{
public:
  CDA_GenericObjectValue(iface::XPCOM::IObject* aObject) : mObject(aObject) {}
  ~CDA_GenericObjectValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::ObjectValue);

  already_AddRefd<iface::CGRS::GenericType> typeOfValue() throw(std::exception&);

  already_AddRefd<iface::XPCOM::IObject> asObject() throw(std::exception&) { mObject->add_ref(); return mObject.getPointer(); }
  void asObject(iface::XPCOM::IObject* aObject) throw(std::exception&) { mObject = aObject; }

private:
  ObjRef<iface::XPCOM::IObject> mObject;
};

class CDA_GenericSequenceValue
  : public iface::CGRS::SequenceValue
{
public:
  CDA_GenericSequenceValue(iface::CGRS::GenericType* aInnerType) : mInnerType(aInnerType) {}
  ~CDA_GenericSequenceValue();

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::SequenceValue);

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&);
  int32_t valueCount() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericValue> getValueByIndex(int32_t index) throw(std::exception&);
  void appendValue(iface::CGRS::GenericValue* v) throw(std::exception&);

private:
  ObjRef<iface::CGRS::GenericType> mInnerType;
  std::vector<iface::CGRS::GenericValue*> mVec;
};

class CDA_GenericEnumValue
  : public iface::CGRS::EnumValue
{
public:
  CDA_GenericEnumValue(iface::CGRS::EnumType* aEtype, int32_t aValue)
    : mValue(aValue), mEtype(aEtype) {}
  ~CDA_GenericEnumValue() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(CGRS::GenericValue, CGRS::EnumValue);

  already_AddRefd<iface::CGRS::GenericType>  typeOfValue() throw(std::exception&) { mEtype->add_ref(); return mEtype.getPointer(); }
  std::string asString() throw(std::exception&) { return mEtype->indexToName(mValue); }
  void asString(const std::string& aValue) throw(std::exception&) { mValue = mEtype->nameToIndex(aValue); }
  int32_t asLong() throw(std::exception&) { return mValue; }
  void asLong(int32_t aValue) throw(std::exception&) { mValue = aValue; }

private:
  int32_t mValue;
  ObjRef<iface::CGRS::EnumType> mEtype;
};

#if 0 // Here solely to document what is implemented in CGRS modules
class CDA_GenericAttribute
  : public iface::CGRS::GenericAttribute
{
public:
  CDA_GenericAttribute() {}
  ~CDA_GenericAttribute() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericAttribute);

  bool isReadonly() throw(std::exception&);
  std::string name() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericType> type() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericMethod> getter() throw(std::exception&);
  already_AddRefd<iface::CGRS::GenericMethod> setter() throw(std::exception&);
};
#endif

class CDA_GenericParameter
  : public iface::CGRS::GenericParameter
{
public:
  CDA_GenericParameter(bool aIsIn, bool aIsOut, const std::string& aName) :
    mIsIn(aIsIn), mIsOut(aIsOut), mName(aName) {}
  ~CDA_GenericParameter() {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(CGRS::GenericParameter);

  bool isIn() throw(std::exception&) { return mIsIn; };
  bool isOut() throw(std::exception&) { return mIsOut; };
  std::string name() throw(std::exception&) { return mName; };

private:
  bool mIsIn, mIsOut;
  std::string mName;
};

CGRS_PUBLIC_PRE
class CGRS_PUBLIC_POST CGRSCallback
{
public:
  virtual already_AddRefd<iface::CGRS::CallbackObjectValue> unwrap() = 0;
};

CGRS_PUBLIC_PRE
  already_AddRefd<CDA_GenericsService>
  CreateGenericsServiceInternal(void)
CGRS_PUBLIC_POST;
