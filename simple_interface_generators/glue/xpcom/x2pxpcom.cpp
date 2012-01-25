#include "x2pxpcom.hxx"
#include "X2PFactory.hxx"
#include <stdio.h>
#include <assert.h>
#include "IWrappedPCM.h"

nsDataHashtable<nsIDHashKey,X2PFactory*>* X2PFactory::mEntries;

x2p::XPCOM::IObject::IObject(iface::XPCOM::IObject* aObj)
  : mObj(aObj), _refcount(0)
{
  mObj->add_ref();
}

x2p::XPCOM::IObject::IObject()
{
  assert(0 /* Incorrect IObject constructor called. */);
}

class WrappedPCM
  : public IWrappedPCM
{
public:
  WrappedPCM(iface::XPCOM::IObject* aObj)
    : mObj(aObj)
  {
    mObj->add_ref();
  }

  ~WrappedPCM()
  {
    mObj->release_ref();
  }

  NS_DECL_ISUPPORTS
  NS_DECL_IWRAPPEDPCM

private:
  iface::XPCOM::IObject* mObj;
};

NS_IMPL_ISUPPORTS1(WrappedPCM, IWrappedPCM)

NS_IMETHODIMP
WrappedPCM::GetPcmObj(iface::XPCOM::IObject** aObj)
{
  *aObj = mObj;
  (*aObj)->add_ref();

  return NS_OK;
}

NS_IMETHODIMP
WrappedPCM::GetObjid(nsACString& aObjid)
{
  char* objid = mObj->objid();

  if (objid == nsnull)
    return NS_ERROR_FAILURE;

  aObjid.Assign(objid);
  free(objid);
  
  return NS_OK;
}

nsresult
x2p::XPCOM::IObject::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  if (aIID.Equals(NS_GET_IID(IWrappedPCM)))
  {
    WrappedPCM* pcm = new WrappedPCM(mObj);
    NS_ADDREF(pcm);
    *aInstancePtr = static_cast<IWrappedPCM*>(pcm);
    return NS_OK;
  }

  X2PFactory* f = X2PFactory::FindFactory(aIID);
  if (f == nsnull)
    return NS_ERROR_NO_INTERFACE;
  
  void* obj = mObj->query_interface(f->GetID());
  if (obj == nsnull)
    return NS_ERROR_NO_INTERFACE;

  *aInstancePtr = f->MakeX2P(obj);

  return NS_OK;
}

NS_IMPL_ISUPPORTS_INHERITED0(X2PISupports, x2p::XPCOM::IObject);

class X2PFactory_X2PISupports
  : public X2PFactory
{
public:
  X2PFactory_X2PISupports()
    : X2PFactory("XPCOM::IObject", NS_GET_IID(nsISupports))
  {
  }

  void* MakeX2P(void* aObj)
  {
    iface::XPCOM::IObject* obji =
      reinterpret_cast<iface::XPCOM::IObject*>(aObj);
    X2PISupports* ptr = new X2PISupports(obji);
    NS_ADDREF(ptr);
    obji->release_ref();
    return static_cast<nsISupports*>(ptr);
  }
};
X2PFactory_X2PISupports _the_X2PFactory_X2pISupports;
