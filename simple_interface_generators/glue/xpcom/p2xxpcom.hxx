#ifndef INCLUDED_P2XXPCOM_HXX
#define INCLUDED_P2XXPCOM_HXX

// Major hack: nsStringAPI tries to assert that it is compiled with
// -fshort-wchar if Mozilla is configured that way, but we are a bridge between
// short-wchar and normal code and deliberately not compiled with that flag. So
// to get away with it, we pretend that prlog.h was already included by defining
// the guard include, and set PR_STATIC_ASSERT to do nothing.
#define prlog_h___
#define PR_STATIC_ASSERT(x)
#undef __STDC__
#include <string.h>
#include <nsStringAPI.h>

#include <exception>
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

namespace p2x
{
  namespace XPCOM
  {
    XPCOMSUPPORT_PRE class XPCOMSUPPORT_POST IObject
      : public virtual iface::XPCOM::IObject
    {
    public:
      XPCOMSUPPORT_PRE IObject() XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE IObject(nsISupports*) XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE void add_ref() throw(std::exception&) XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE void release_ref() throw(std::exception&) XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE void* query_interface(const char* id)
        throw(std::exception&) XPCOMSUPPORT_POST;
      XPCOMSUPPORT_PRE char* objid() throw(std::exception&) XPCOMSUPPORT_POST;

      XPCOMSUPPORT_PRE nsISupports* GetObject() XPCOMSUPPORT_POST
      {
	nsISupports* obj = mObj;
        NS_ADDREF(obj);
        return mObj;
      }

    private:
      PRUint32 _refcount;
    protected:
      nsCOMPtr<nsISupports> mObj;
    };
  };
};
#endif // INCLUDED_P2XXPCOM_HXX
