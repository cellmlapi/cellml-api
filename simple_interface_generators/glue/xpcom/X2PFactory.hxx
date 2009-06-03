#ifdef IN_MODULE_XPCOMSupport
#define XPCOMSUPPORT_PRE CDA_EXPORT_PRE
#define XPCOMSUPPORT_POST CDA_EXPORT_POST
#else
#define XPCOMSUPPORT_PRE CDA_IMPORT_PRE
#define XPCOMSUPPORT_POST CDA_IMPORT_POST
#endif
#include <stdio.h>

class X2PFactory
{
public:
  XPCOMSUPPORT_PRE X2PFactory(const char* aInterface, nsIID aIID) XPCOMSUPPORT_POST
    : mInterface(aInterface)
  {
    if (mEntries == nsnull)
    {
      mEntries = new nsDataHashtable<nsIDHashKey,X2PFactory*>();
      mEntries->Init();
    }
    mEntries->Put(aIID, this);
  }

  XPCOMSUPPORT_PRE static X2PFactory* FindFactory(nsIID aIID) XPCOMSUPPORT_POST
  {
    if (mEntries == nsnull)
    {
      printf("Serious problem with XPCOM bridge: X2PFactory::FindFactory "
             "called but mEntries doesn't exist\n");
      char* x = NULL;

      // Ensure we crash here even in production mode
      // because we will crash later in any case and this will result in a more
      // useful error report.
      abort();

      return nsnull;
    }

    X2PFactory* xf = nsnull;
    mEntries->Get(aIID, &xf);
    if (xf == nsnull)
      return nsnull;

    return xf;
  }

  XPCOMSUPPORT_PRE const char* GetID() XPCOMSUPPORT_POST
  {
    return mInterface;
  }

  XPCOMSUPPORT_PRE virtual void* MakeX2P(void* aObj) XPCOMSUPPORT_POST = 0;

private:
  XPCOMSUPPORT_PRE static nsDataHashtable<nsIDHashKey,X2PFactory*>* mEntries XPCOMSUPPORT_POST;
  const char* mInterface;
};
