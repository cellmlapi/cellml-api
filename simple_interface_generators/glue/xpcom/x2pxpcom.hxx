#ifndef INCLUDED_X2PXPCOM_HXX
#define INCLUDED_X2PXPCOM_HXX

// Major hack: nsStringAPI tries to assert that it is compiled with
// -fshort-wchar if Mozilla is configured that way, but we are a bridge between
// short-wchar and normal code and deliberately not compiled with that flag. So
// to get away with it, we pretend that prlog.h was already included by defining
// the guard include, and set PR_STATIC_ASSERT to do nothing.
#define prlog_h___
#define PR_STATIC_ASSERT(x)
#include <string.h>
#include <nsStringAPI.h>

#include <exception>
#include "Utilities.hxx"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <nsISupports.h>
#include "Ifacexpcom.hxx"
#include <nsCOMPtr.h>
#include <nsDataHashtable.h>

#ifdef IN_MODULE_XPCOMSupport
#define XPCOMSUPPORT_PRE CDA_EXPORT_PRE
#define XPCOMSUPPORT_POST CDA_EXPORT_POST
#else
#define XPCOMSUPPORT_PRE CDA_IMPORT_PRE
#define XPCOMSUPPORT_POST CDA_IMPORT_POST
#endif

namespace x2p
{
  namespace XPCOM
  {
    XPCOMSUPPORT_PRE class XPCOMSUPPORT_POST IObject
    // We don't actually need to inherit from nsISupports directly.
    /* : public virtual nsISupports */
    {
    public:
      XPCOMSUPPORT_PRE IObject() XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE IObject(iface::XPCOM::IObject*) XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE virtual ~IObject() XPCOMSUPPORT_POST
      {
        if (mObj != NULL)
          mObj->release_ref();
      };
      XPCOMSUPPORT_PRE nsrefcnt AddRef() XPCOMSUPPORT_POST
      {
        return _refcount++;
      }
      XPCOMSUPPORT_PRE nsrefcnt Release() XPCOMSUPPORT_POST
      {
        _refcount--;
        if (_refcount != 0)
          return _refcount;
        delete this;
        return 0;
      }
      XPCOMSUPPORT_PRE nsresult QueryInterface(REFNSIID aIID, void** aInstancePtr) XPCOMSUPPORT_POST;
    protected:
      iface::XPCOM::IObject* mObj;
    private:
      nsrefcnt _refcount;
    };
  };
};

class X2PISupports
  : public x2p::XPCOM::IObject, public nsISupports
{
public:
  X2PISupports(iface::XPCOM::IObject* aObj)
    : x2p::XPCOM::IObject(aObj)
  {
  }

  ~X2PISupports()
  {
  }

  NS_DECL_ISUPPORTS;
};

#endif // INCLUDED_X2PXPCOM_HXX
