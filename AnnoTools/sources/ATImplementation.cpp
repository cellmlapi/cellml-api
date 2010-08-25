#define IN_ANNOTOOLS_MODULE
#include "ATImplementation.hpp"
#include "AnnoToolsBootstrap.hpp"

CDAStringAnnotationImpl::CDAStringAnnotationImpl(const wchar_t* aValue)
  throw()
  : _cda_refcount(1), mString(aValue)
{
}

void
CDAStringAnnotationImpl::value(const wchar_t* aValue)
  throw(std::exception&)
{
  mString = aValue;
}

wchar_t*
CDAStringAnnotationImpl::value()
  throw(std::exception&)
{
  return CDA_wcsdup(mString.c_str());
}

CDAObjectAnnotationImpl::CDAObjectAnnotationImpl(iface::XPCOM::IObject* aValue)
  throw()
  : _cda_refcount(1), mObject(aValue)
{
}

void
CDAObjectAnnotationImpl::value(iface::XPCOM::IObject* aValue)
  throw(std::exception&)
{
  mObject = aValue;
}

iface::XPCOM::IObject*
CDAObjectAnnotationImpl::value()
  throw(std::exception&)
{
  mObject->add_ref();
  return mObject;
}

CDAAnnotationSetImpl::CDAAnnotationSetImpl()
  throw()
  : _cda_refcount(1)
{
  mPrefixURI = L"http://www.cellml.org/tools/annotools/set";
  size_t i;
#define CHARS L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_."
  for (i = 0; i < 10; i++)
  {
    unsigned long r = mersenne_genrand_int32();
    mPrefixURI += CHARS[r & 0x3F];
    r >>= 6;
    mPrefixURI += CHARS[r & 0x3F];
    r >>= 6;
    mPrefixURI += CHARS[r & 0x3F];
    r >>= 6;
    mPrefixURI += CHARS[r & 0x3F];
    r >>= 6;
    mPrefixURI += CHARS[r & 0x3F];
  }
#undef CHARS

  mPrefixURI += L"/";
}

CDAAnnotationSetImpl::~CDAAnnotationSetImpl()
  throw()
{
  std::list<std::pair<std::wstring, iface::cellml_api::CellMLElement*> >::iterator
    i = mAnnotations.begin();
  for (; i != mAnnotations.end(); i++)
  {
    std::wstring key = mPrefixURI + (*i).first;
    (*i).second->setUserData(key.c_str(), NULL);
    (*i).second->release_ref();
  }
}

wchar_t*
CDAAnnotationSetImpl::prefixURI()
  throw(std::exception&)
{
  return CDA_wcsdup(mPrefixURI.c_str());
}

wchar_t*
CDAAnnotationSetImpl::getStringAnnotation
(
 iface::cellml_api::CellMLElement* aElement,
 const wchar_t* aKey
)
  throw(std::exception&)
{
  std::wstring key = mPrefixURI;
  key += aKey;

  try
  {
    RETURN_INTO_OBJREF(ud, iface::cellml_api::UserData,
                       aElement->getUserData(key.c_str()));
    DECLARE_QUERY_INTERFACE_OBJREF(sa, ud, cellml_services::StringAnnotation);
    
    if (sa == NULL)
      return CDA_wcsdup(L"");
    
    return sa->value();
  }
  catch (...)
  {
    return CDA_wcsdup(L"");
  }
}

void
CDAAnnotationSetImpl::setStringAnnotation
(
 iface::cellml_api::CellMLElement* aElement,
 const wchar_t* aKey,
 const wchar_t* aValue
)
  throw(std::exception&)
{
  std::wstring key = mPrefixURI;
  key += aKey;
  
  aElement->add_ref();
  mAnnotations.push_back(std::pair<std::wstring,
                         iface::cellml_api::CellMLElement*>(aKey, aElement));

  RETURN_INTO_OBJREF(anno, iface::cellml_services::StringAnnotation,
                     new CDAStringAnnotationImpl(aValue));
  aElement->setUserData(key.c_str(), anno);
}

iface::XPCOM::IObject*
CDAAnnotationSetImpl::getObjectAnnotation
(
 iface::cellml_api::CellMLElement* aElement,
 const wchar_t* aKey
)
  throw(std::exception&)
{
  std::wstring key = mPrefixURI;
  key += aKey;

  try
  {
    RETURN_INTO_OBJREF(ud, iface::cellml_api::UserData,
                       aElement->getUserData(key.c_str()));
    DECLARE_QUERY_INTERFACE_OBJREF(oa, ud, cellml_services::ObjectAnnotation);
    
    if (oa == NULL)
      return NULL;
    
    return oa->value();
  }
  catch (...)
  {
    return NULL;
  }
}

void
CDAAnnotationSetImpl::setObjectAnnotation
(
 iface::cellml_api::CellMLElement* aElement,
 const wchar_t* aKey,
 iface::XPCOM::IObject* aValue
)
  throw(std::exception&)
{
  std::wstring key = mPrefixURI;
  key += aKey;
  
  if (aValue == NULL)
  {
    // Just clear the key. No point cleaning up mAnnotations, because this
    // object is supposed to be fairly short lived anyway.
    aElement->setUserData(key.c_str(), NULL);
    return;
  }

  aElement->add_ref();
  mAnnotations.push_back(std::pair<std::wstring,
                         iface::cellml_api::CellMLElement*>(aKey, aElement));

  RETURN_INTO_OBJREF(anno, iface::cellml_services::ObjectAnnotation,
                     new CDAObjectAnnotationImpl(aValue));
  aElement->setUserData(key.c_str(), anno);
}

iface::cellml_services::AnnotationToolService*
CreateAnnotationToolService(void)
{
  return new CDAAnnotationToolServiceImpl();
}
