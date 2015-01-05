#include <exception>
#include <map>
#include <string>
#include "cda_compiler_support.h"
#include "Ifacexpcom.hxx"
#include "xpcom.hh"

#ifdef IN_GLUE_MODULE
#define PUBLIC_CORBA_GLUE_PRE CDA_EXPORT_PRE
#define PUBLIC_CORBA_GLUE_POST CDA_EXPORT_POST
#else
#define PUBLIC_CORBA_GLUE_PRE CDA_IMPORT_PRE
#define PUBLIC_CORBA_GLUE_POST CDA_IMPORT_POST
#endif

PUBLIC_CORBA_GLUE_PRE
class PUBLIC_CORBA_GLUE_POST CCIFactory
{
public:
  CCIFactory() {}
  /* Mainly just to keep the compiler happy. */
  virtual ~CCIFactory() {}

  /* Find the name of the interface(namespace::interface) */
  virtual const char* Name() const = 0;
  virtual void* MakeCCI(XPCOM::IObject_ptr aObj,
                        PortableServer::POA_ptr aPp)
    const = 0;
};

PUBLIC_CORBA_GLUE_PRE
class PUBLIC_CORBA_GLUE_POST SCIFactory
{
public:
  SCIFactory() {}
  /* Mainly just to keep the compiler happy. */
  virtual ~SCIFactory() {}

  /* Find the name of the interface(namespace::interface) */
  virtual const char* Name() const = 0;
  virtual XPCOM::IObject_ptr MakeSCI(void* aObj,
                                     PortableServer::POA_ptr aPp) const = 0;
};

class WrapperRepository
{
public:
  PUBLIC_CORBA_GLUE_PRE void RegisterCCIFactory(CCIFactory* aCCI) PUBLIC_CORBA_GLUE_POST;
  PUBLIC_CORBA_GLUE_PRE void RegisterSCIFactory(SCIFactory* aSCI) PUBLIC_CORBA_GLUE_POST;
  PUBLIC_CORBA_GLUE_PRE
    void* NewCCI(const char* interfaceName,
                 XPCOM::IObject_ptr aObj,
                 PortableServer::POA_ptr aPp)
  PUBLIC_CORBA_GLUE_POST;
  PUBLIC_CORBA_GLUE_PRE
    XPCOM::IObject_ptr NewSCI(const char* interfaceName,
                              void* aObj,
                              PortableServer::POA_ptr aPp)
  PUBLIC_CORBA_GLUE_POST;

private:
  std::map<std::string, CCIFactory*> mCCIFactories;
  std::map<std::string, SCIFactory*> mSCIFactories;
};

PUBLIC_CORBA_GLUE_PRE
  WrapperRepository& gWrapperRepository()
PUBLIC_CORBA_GLUE_POST;
