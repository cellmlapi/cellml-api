#define IN_MODULE_XPCOMSupport
#include "p2xxpcom.hxx"
#include "P2XFactory.hxx"
#include <stdio.h>
#include <assert.h>

nsDataHashtable<nsCStringHashKey,P2XFactory*>* P2XFactory::mEntries;

p2x::XPCOM::IObject::IObject(nsISupports* aObj)
  : mObj(aObj), _refcount(1)
{
}

p2x::XPCOM::IObject::IObject()
{
  assert(0); /* Wrong IObject virtual base constructor called. */
}

void
p2x::XPCOM::IObject::add_ref()
  throw(std::exception&)
{
  _refcount++;
}

void
p2x::XPCOM::IObject::release_ref()
  throw(std::exception&)
{
  _refcount--;
  if (_refcount == 0)
    delete this;
}

void*
p2x::XPCOM::IObject::query_interface(const char* id)
  throw(std::exception&)
{
  nsCOMPtr<nsISupports> qires;
  P2XFactory* f = P2XFactory::FindFactory(id);
  if (f == nsnull)
    return nsnull;
  
  mObj->QueryInterface(f->GetIID(), getter_AddRefs(qires));
  if (qires == nsnull)
    return nsnull;
  
  return f->MakeP2X(qires);
}

char*
p2x::XPCOM::IObject::objid()
  throw(std::exception&)
{
  // We are never double wrapped, so this is a local XPCOM object.
  nsISupports* isupports = mObj;
#ifndef ASSUME_UNBROKEN_DYNAMIC_CAST_VOID
  void* p = reinterpret_cast<void*>(isupports);
#else
  void* p = dynamic_cast<void*>(isupports);
#endif
  char buf[15];
  sprintf(buf, "xpcom:%08X", p);
  return CDA_strdup(buf);
}

namespace p2x
{
  namespace XPCOM
  {
    class IObjectFactory
      : public P2XFactory
    {
    public:
      IObjectFactory()
        : P2XFactory("xpcom::IObject", NS_GET_IID(nsISupports))
      {
      }

      void*
      MakeP2X(nsISupports* aObj)
      {
        return static_cast<iface::XPCOM::IObject*>(new p2x::XPCOM::IObject(aObj));
      }
    };

    static p2x::XPCOM::IObjectFactory gIObjectFactory;
  };
};
