#ifndef _TemporaryAnnotation_hxx
#define _TemporaryAnnotation_hxx
#include <list>
#include <string>
#include "Utilities.hxx"
#include "IfaceCellML_APISPEC.hxx"

// Win32 hack...
#ifdef _WIN32
#define swprintf _snwprintf
#endif

/**
 * Temporary annotation manager is a utility class used to set annotations on
 * the model intended for internal use only, and keep track of them so they can
 * be automatically removed when they are no longer needed..
 */
class TemporaryAnnotation;
typedef std::list<TemporaryAnnotation*> AnnotationList;

class TemporaryAnnotationKey
{
public:
  TemporaryAnnotationKey()
    : mAnnotationKey(L"http://www.cellml.org/tools/ccgs/tam#rand=")
  {
    wchar_t buf[9];
    swprintf(buf, 9, L"%08X", mersenne_genrand_int32());
    mAnnotationKey += buf;
    swprintf(buf, 9, L"%08X", mersenne_genrand_int32());
    mAnnotationKey += buf;
    swprintf(buf, 9, L"%08X", mersenne_genrand_int32());
    mAnnotationKey += buf;
    swprintf(buf, 9, L"%08X", mersenne_genrand_int32());
    mAnnotationKey += buf;
  }

  TemporaryAnnotationKey(const TemporaryAnnotationKey& copy)
    : mAnnotationKey(copy.str())
  {
  }

  const std::wstring&
  str() const
  {
    return mAnnotationKey;
  }

private:
  std::wstring mAnnotationKey;
};

class TemporaryAnnotation
  : public iface::cellml_api::UserData
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_api::UserData);

  TemporaryAnnotation()
    : _cda_refcount(1)
  {
  }

  TemporaryAnnotation(
                      iface::cellml_api::CellMLElement* aCellMLElement
                     )
    : _cda_refcount(1), mCellMLElement(aCellMLElement)
  {
    mCellMLElement->setUserData(mKey.str().c_str(), this);
  }

  TemporaryAnnotation(
                      iface::cellml_api::CellMLElement* aCellMLElement,
                      const TemporaryAnnotationKey& aKey
                     )
    : _cda_refcount(1), mCellMLElement(aCellMLElement), mKey(aKey)
  {
    mCellMLElement->setUserData(mKey.str().c_str(), this);
  }

  virtual ~TemporaryAnnotation()
  {
  }

protected:
  friend class TemporaryAnnotationManager;
  ObjRef<iface::cellml_api::CellMLElement> mCellMLElement;
  TemporaryAnnotationKey mKey;
};

class TemporaryAnnotationManager
{
public:
  TemporaryAnnotationManager()
  {
  }

  ~TemporaryAnnotationManager()
  {
    AnnotationList::iterator i;
    for (i = mActiveAnnotations.begin(); i != mActiveAnnotations.end(); i++)
    {
      TemporaryAnnotation* ta = (*i);
      try
      {
        ta->mCellMLElement->setUserData(ta->mKey.str().c_str(), NULL);
      }
      catch (...)
      {
      }
      ta->release_ref();
    }
  }

  void addAnnotation(TemporaryAnnotation* mAnnotation)
  {
    mAnnotation->add_ref();
    mActiveAnnotations.push_back(mAnnotation);
  }
private:
  AnnotationList mActiveAnnotations;
};

#endif // _TemporaryAnnotation_hxx
