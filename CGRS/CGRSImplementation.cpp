#define IN_CGRS_MODULE
#define MODULE_CONTAINS_CGRS
#include "Utilities.hxx"
#include "IfaceCGRS.hxx"
#include "CGRSImplementation.hpp"
#include "CGRSBootstrap.hpp"
#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <dlfcn.h>
#include <sys/utsname.h>
#endif
#ifdef _MSC_VER
#include <errno.h>
#endif

// The generics service is treated as a singleton
// so that each module is globally only loaded once.
static CDA_GenericsService* gCDAGenericsService = NULL;

CDA_GenericsService::CDA_GenericsService()
  : mStringType("string"), mWStringType("wstring"), mShortType("short"),
    mLongType("long"), mLongLongType("long long"), mUShortType("unsigned short"), mULongType("unsigned long"),
    mULongLongType("unsigned long long"),
    mFloatType("float"), mDoubleType("double"), mBooleanType("boolean"),
    mCharType("char"), mOctetType("octet"), mIObjectType("XPCOM::IObject"), mVoidType("void")
{
  registerType(&mStringType);
  registerType(&mWStringType);
  registerType(&mShortType);
  registerType(&mLongType);
  registerType(&mLongLongType);
  registerType(&mUShortType);
  registerType(&mULongType);
  registerType(&mULongLongType);
  registerType(&mFloatType);
  registerType(&mDoubleType);
  registerType(&mBooleanType);
  registerType(&mCharType);
  registerType(&mOctetType);
  registerType(&mIObjectType);
  registerType(&mVoidType);
}

void
CDA_GenericsService::registerType(iface::CGRS::GenericType* aType)
{
  std::string tname = aType->asString();
  if (mTypeRegistry.find(tname) != mTypeRegistry.end())
    return;

  aType->add_ref();
  mTypeRegistry.insert(std::pair<std::string, iface::CGRS::GenericType*>(tname, aType));
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericsService::getTypeByName(const std::string& aTypeName) throw(iface::CGRS::CGRSError&)
{
  std::map<std::string, iface::CGRS::GenericType*>::iterator i = mTypeRegistry.find(aTypeName);
  if (i == mTypeRegistry.end())
    throw iface::CGRS::CGRSError();

  (*i).second->add_ref();
  return (*i).second;
}

void
CDA_GenericsService::loadGenericModule(const std::string& aModulePath)
  throw(std::exception&)
{
#ifdef WIN32
  void* s = LoadLibrary(aModulePath.c_str());
  if (s == NULL)
  {
    std::string tmp;
    tmp = aModulePath + ".dll";
    s = LoadLibrary(tmp.c_str());
    if (s == NULL)
    {
      tmp = "lib" + tmp;
      s = LoadLibrary(tmp.c_str());
    }
  }
#else
  void* s = dlopen(aModulePath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (s == NULL)
  {
    std::string tmp;
    tmp = "lib" + aModulePath + ".so";
    s = dlopen(tmp.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  }
#endif

  if (s == NULL)
    throw iface::CGRS::CGRSError();

  void* addr =
#ifdef WIN32
    reinterpret_cast<void*>(GetProcAddress((HMODULE)s, "init_cgrsmodule"));
#else
    reinterpret_cast<void*>(dlsym(s, "init_cgrsmodule"));
#endif
  if (addr == NULL)
    throw iface::CGRS::CGRSError();

  return reinterpret_cast<void (*)(CDA_GenericsService*)>(addr)(this);
}

void
CDA_GenericsService::registerBootstrap(const std::string& aBootstrapName, iface::CGRS::GenericValue* aBootstrap)
  throw(std::exception&)
{
  if (mValueRegistry.find(aBootstrapName) != mValueRegistry.end())
    return;

  aBootstrap->add_ref();
  mValueRegistry.insert(std::pair<std::string, iface::CGRS::GenericValue*>(aBootstrapName, aBootstrap));
}

already_AddRefd<iface::CGRS::GenericValue>
CDA_GenericsService::getBootstrapByName(const std::string& aBootstrapName)
  throw(std::exception&)
{
  std::map<std::string, iface::CGRS::GenericValue*>::iterator i = mValueRegistry.find(aBootstrapName);
  if (i == mValueRegistry.end())
    throw iface::CGRS::CGRSError();

  (*i).second->add_ref();
  return (*i).second;
}

void
CDA_GenericsService::registerInterface(const std::string& aIfaceName, iface::CGRS::GenericInterface* aIface)
  throw(std::exception&)
{
  if (mInterfaceRegistry.find(aIfaceName) != mInterfaceRegistry.end())
    return;

  aIface->add_ref();
  mInterfaceRegistry.insert(std::pair<std::string, iface::CGRS::GenericInterface*>(aIfaceName, aIface));
}

already_AddRefd<iface::CGRS::GenericInterface>
CDA_GenericsService::getInterfaceByName(const std::string& aInterfaceName)
  throw(std::exception&)
{
  std::map<std::string, iface::CGRS::GenericInterface*>::iterator i = mInterfaceRegistry.find(aInterfaceName);
  if (i == mInterfaceRegistry.end())
    throw iface::CGRS::CGRSError();

  (*i).second->add_ref();
  return (*i).second;
}

already_AddRefd<iface::CGRS::StringValue>
CDA_GenericsService::makeString(const std::string& val) throw(std::exception&)
{
  return new CDA_GenericStringValue(val);
}

already_AddRefd<iface::CGRS::WStringValue>
CDA_GenericsService::makeWString(const std::wstring& val) throw(std::exception&)
{
  return new CDA_GenericWStringValue(val);
}

already_AddRefd<iface::CGRS::ShortValue>
CDA_GenericsService::makeShort(int16_t val) throw(std::exception&)
{
  return new CDA_GenericShortValue(val);
}

already_AddRefd<iface::CGRS::LongValue>
CDA_GenericsService::makeLong(int32_t val) throw(std::exception&)
{
  return new CDA_GenericLongValue(val);
}

already_AddRefd<iface::CGRS::LongLongValue>
CDA_GenericsService::makeLongLong(int64_t val) throw(std::exception&)
{
  return new CDA_GenericLongLongValue(val);
}

already_AddRefd<iface::CGRS::UShortValue>
CDA_GenericsService::makeUShort(uint16_t val) throw(std::exception&)
{
  return new CDA_GenericUShortValue(val);
}

already_AddRefd<iface::CGRS::ULongValue>
CDA_GenericsService::makeULong(uint32_t val) throw(std::exception&)
{
  return new CDA_GenericULongValue(val);
}

already_AddRefd<iface::CGRS::ULongLongValue>
CDA_GenericsService::makeULongLong(uint64_t val) throw(std::exception&)
{
  return new CDA_GenericULongLongValue(val);
}

already_AddRefd<iface::CGRS::FloatValue>
CDA_GenericsService::makeFloat(float val) throw(std::exception&)
{
  return new CDA_GenericFloatValue(val);
}

already_AddRefd<iface::CGRS::DoubleValue>
CDA_GenericsService::makeDouble(double val) throw(std::exception&)
{
  return new CDA_GenericDoubleValue(val);
}

already_AddRefd<iface::CGRS::BooleanValue>
CDA_GenericsService::makeBoolean(bool val) throw(std::exception&)
{
  return new CDA_GenericBooleanValue(val);
}

already_AddRefd<iface::CGRS::CharValue>
CDA_GenericsService::makeChar(char val) throw(std::exception&)
{
  return new CDA_GenericCharValue(val);
}

already_AddRefd<iface::CGRS::OctetValue>
CDA_GenericsService::makeOctet(uint8_t val) throw(std::exception&)
{
  return new CDA_GenericOctetValue(val);
}

already_AddRefd<iface::CGRS::SequenceValue>
CDA_GenericsService::makeSequence(iface::CGRS::GenericType* aInnerType) throw(std::exception&)
{
  return new CDA_GenericSequenceValue(aInnerType);
}

already_AddRefd<iface::CGRS::SequenceType>
CDA_GenericsService::makeSequenceType(iface::CGRS::GenericType* aInnerType) throw(std::exception&)
{
  return new CDA_GenericSequenceType(aInnerType);
}

already_AddRefd<iface::CGRS::EnumValue>
CDA_GenericsService::makeEnumFromString(iface::CGRS::EnumType* etype, const std::string& name)
  throw(std::exception&)
{
  return new CDA_GenericEnumValue(etype, etype->nameToIndex(name));
}

already_AddRefd<iface::CGRS::EnumValue>
CDA_GenericsService::makeEnumFromIndex(iface::CGRS::EnumType* etype, int32_t index)
  throw(std::exception&)
{
  return new CDA_GenericEnumValue(etype, index);
}

already_AddRefd<iface::CGRS::GenericValue>
CDA_GenericsService::makeObject(iface::XPCOM::IObject* value)
  throw(std::exception&)
{
  if (value == NULL)
    return makeVoid();
  return new CDA_GenericObjectValue(value);
}

already_AddRefd<iface::CGRS::GenericValue>
CDA_GenericsService::makeVoid()
  throw(std::exception&)
{
  mVoid.add_ref();
  return &mVoid;
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericStringValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("string");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericWStringValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("wstring");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericShortValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("short");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericLongValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("long");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericLongLongValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("long long");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericUShortValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("unsigned short");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericULongValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("unsigned long");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericULongLongValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("unsigned long long");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericFloatValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("float");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericDoubleValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("double");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericBooleanValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("boolean");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericCharValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("char");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericOctetValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("octet");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericObjectValue::typeOfValue()
  throw(std::exception&)
{
  return gCDAGenericsService->getTypeByName("XPCOM::IObject");
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericVoidValue::typeOfValue()
  throw()
{
  return gCDAGenericsService->getTypeByName("void");
}

CDA_GenericSequenceValue::~CDA_GenericSequenceValue()
{
  for (std::vector<iface::CGRS::GenericValue*>::iterator i = mVec.begin(); i != mVec.end(); i++)
    (*i)->release_ref();
}

already_AddRefd<iface::CGRS::GenericType>
CDA_GenericSequenceValue::typeOfValue()
  throw(std::exception&)
{
  return new CDA_GenericSequenceType(mInnerType);
}

int32_t
CDA_GenericSequenceValue::valueCount() throw(std::exception&)
{
  return mVec.size();
}

already_AddRefd<iface::CGRS::GenericValue>
CDA_GenericSequenceValue::getValueByIndex(int32_t aIndex)
  throw(std::exception&)
{
  if (aIndex < 0 || aIndex >= mVec.size())
    throw iface::CGRS::CGRSError();

  iface::CGRS::GenericValue* v = mVec[aIndex];
  v->add_ref();
  return v;
}

void
CDA_GenericSequenceValue::appendValue(iface::CGRS::GenericValue* aValue)
  throw(std::exception&)
{
  aValue->add_ref();
  mVec.push_back(aValue);
}

already_AddRefd<iface::CGRS::GenericsService>
CreateGenericsService(void)
{
  if (gCDAGenericsService == NULL)
    gCDAGenericsService = new CDA_GenericsService();

  gCDAGenericsService->add_ref();
  return gCDAGenericsService;
}

already_AddRefd<CDA_GenericsService>
CreateGenericsServiceInternal(void)
{
  if (gCDAGenericsService == NULL)
    gCDAGenericsService = new CDA_GenericsService();

  gCDAGenericsService->add_ref();
  return gCDAGenericsService;
}
