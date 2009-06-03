#define IN_GLUE_MODULE
#include "WrapperRepository.hxx"


WrapperRepository &
gWrapperRepository()
{
  static WrapperRepository* wr = new WrapperRepository();
  return *wr;
}

void
WrapperRepository::RegisterCCIFactory(CCIFactory* aCCI)
{
  mCCIFactories.insert(std::pair<std::string, CCIFactory*>
                       (aCCI->Name(), aCCI));
}

void
WrapperRepository::RegisterSCIFactory(SCIFactory* aSCI)
{
  mSCIFactories.insert(std::pair<std::string, SCIFactory*>
                       (aSCI->Name(), aSCI));
}

void*
WrapperRepository::NewCCI(const char* interface, XPCOM::IObject_ptr aObj,
                          PortableServer::POA_ptr aPp)
{
  std::map<std::string, CCIFactory*>::iterator i;
  i = mCCIFactories.find(interface);
  if (i == mCCIFactories.end())
    return NULL;
  return (*i).second->MakeCCI(aObj, aPp);
}

XPCOM::IObject_ptr
WrapperRepository::NewSCI(const char* interface, void* aObj,
                          PortableServer::POA_ptr aPp)
{
  std::map<std::string, SCIFactory*>::iterator i;
  i = mSCIFactories.find(interface);
  if (i == mSCIFactories.end())
    return NULL;
  return (*i).second->MakeSCI(aObj, aPp);
}
