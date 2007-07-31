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
};

class UnsuitablyConstrainedError
  : public std::exception
{
public:
  UnsuitablyConstrainedError()
  {
  }
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
   : mEqn(aCopy.mEqn)
  {
    mEqn->add_ref();
  }

  ~OverconstrainedError()
    throw()
  {
    mEqn->release_ref();
  }

  iface::dom::Element* mEqn;
};
#endif // _CodeGenerationError_hxx
