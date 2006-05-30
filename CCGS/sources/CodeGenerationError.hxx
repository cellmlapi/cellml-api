#ifndef _CodeGenerationError_hxx
#define _CodeGenerationError_hxx

/**
 * An error message class, used to get out of code generation with an
 * error.
 */
class CodeGenerationError
  : public std::exception
{
public:
  CodeGenerationError(const std::wstring& aMessage)
    : mMessage(aMessage)
  {
  }
  
  ~CodeGenerationError()
    throw()
  {
  }

  const wchar_t* str()
  {
    return mMessage.c_str();
  }
private:
  std::wstring mMessage;
};

class UnderconstrainedError
  : public std::exception
{
public:
  UnderconstrainedError()
  {
  }

  UnderconstrainedError(const UnderconstrainedError& aCopy)
  {
    std::list<iface::cellml_api::CellMLVariable*>::const_iterator i =
      aCopy.mUnresolvedWanted.begin();
    for (; i != aCopy.mUnresolvedWanted.end(); i++)
    {
      (*i)->add_ref();
      mUnresolvedWanted.push_back(*i);
    }
  }

  ~UnderconstrainedError()
    throw()
  {
    std::list<iface::cellml_api::CellMLVariable*>::iterator i;
    for (i = mUnresolvedWanted.begin(); i != mUnresolvedWanted.end(); i++)
      delete (*i);
  }

  void addUnresolvedWanted(iface::cellml_api::CellMLVariable* aSource)
  {
    aSource->add_ref();
    mUnresolvedWanted.push_back(aSource);
  }

  std::list<iface::cellml_api::CellMLVariable*> mUnresolvedWanted;
};

class OverconstrainedError
  : public std::exception
{
public:
  OverconstrainedError(iface::dom::Element* aEqn)
    : mEqn(aEqn)
  {
    mEqn->add_ref();
  }

  OverconstrainedError(const OverconstrainedError& aCopy)
  {
    mEqn = aCopy.mEqn;
    mEqn->add_ref();
    std::list<iface::cellml_api::CellMLVariable*>::const_iterator i;
    for (i = aCopy.mKnownVariables.begin();
         i != aCopy.mKnownVariables.end(); i++)
    {
      (*i)->add_ref();
      mKnownVariables.push_back(*i);
    }
  }

  ~OverconstrainedError()
    throw()
  {
    mEqn->release_ref();
    std::list<iface::cellml_api::CellMLVariable*>::iterator i;
    for (i = mKnownVariables.begin(); i != mKnownVariables.end(); i++)
      (*i)->release_ref();
  }

  void addKnownVariable(iface::cellml_api::CellMLVariable* aSource)
  {
    aSource->add_ref();
    mKnownVariables.push_back(aSource);
  }

  iface::dom::Element* mEqn;
  std::list<iface::cellml_api::CellMLVariable*> mKnownVariables;
};
#endif // _CodeGenerationError_hxx
