#ifdef IN_MODULE_XPCOMSupport
#define XPCOMSUPPORT_PRE CDA_EXPORT_PRE
#define XPCOMSUPPORT_POST CDA_EXPORT_POST
#else
#define XPCOMSUPPORT_PRE CDA_IMPORT_PRE
#define XPCOMSUPPORT_POST CDA_IMPORT_POST
#endif
#include <stdio.h>

class P2XFactory
{
public:
  P2XFactory(const char* aInterface, nsIID aIID)
    : mIID(aIID)
  {
    if (mEntries == nsnull)
    {
      mEntries = new nsDataHashtable<nsCStringHashKey,P2XFactory*>();
      mEntries->Init();
    }
    nsCString cs(aInterface);
    mEntries->Put(cs, this);
  }

  static P2XFactory* FindFactory(const char* aName)
  {
    nsCString cs(aName);
    if (mEntries == nsnull)
    {
      printf("Serious problem with XPCOM bridge: P2XFactory::FindFactory "
             "called but mEntries is null.\n");

      // Ensure we crash here even in production mode
      // because we will crash later in any case and this will result in a more
      // useful error report.
      abort();

      return nsnull;
    }

    P2XFactory* pf = nsnull;
    mEntries->Get(cs, &pf);
    if (pf == nsnull)
    {
      printf("Serious problem with XPCOM bridge: P2XFactory::FindFactory "
             "called for an interface name that we don't recognise (%s)\n",
             aName);

      // Ensure we crash here even in production mode
      // because we will crash later in any case and this will result in a more
      // useful error report.
      abort();

      return nsnull;
    }
    return pf;
  }

  nsIID GetIID() { return mIID; }

  virtual void* MakeP2X(nsISupports* aObj) = 0;
private:
  XPCOMSUPPORT_PRE static nsDataHashtable<nsCStringHashKey,P2XFactory*>* mEntries XPCOMSUPPORT_POST;
  nsIID mIID;
};
