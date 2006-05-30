#include "CCGSImplementation.hpp"
#include "CodeGenerationError.hxx"

iface::cellml_services::CCodeInformation*
CDA_CGenerator::generateCode(iface::cellml_api::Model* aSourceModel)
  throw (std::exception&)
{
  // Pass the work to the constructor...
  try
  {
    return new CDA_CCodeInformation(aSourceModel);
  }
  catch (CodeGenerationError& cge)
  {
    throw iface::cellml_api::CellMLException();
  }
}

CDA_CCodeInformation::~CDA_CCodeInformation()
{
  CCVL_t::iterator i;
  for (i = mVariables.begin(); i != mVariables.end(); i++)
    (*i)->release_ref();
  MNL_t::iterator i2;
  for (i2 = mFlaggedEquations.begin(); i2 != mFlaggedEquations.end(); i2++)
    (*i2)->release_ref();
}

iface::cellml_services::ModelConstraintLevel
CDA_CCodeInformation::constraintLevel()
  throw (std::exception&)
{
  return mConstraintLevel;
}

uint32_t
CDA_CCodeInformation::variableCount()
  throw (std::exception&)
{
  return mVariableCount;
}

uint32_t
CDA_CCodeInformation::rateVariableCount()
  throw (std::exception&)
{
  return mRateVariableCount;
}

char*
CDA_CCodeInformation::fixedConstantFragment()
  throw (std::exception&)
{
  return strdup(mFixedConstantFragment.str().c_str());
}

char*
CDA_CCodeInformation::computedConstantFragment()
  throw (std::exception&)
{
  return strdup(mComputedConstantFragment.str().c_str());
}

char*
CDA_CCodeInformation::rateCodeFragment()
  throw (std::exception&)
{
  return strdup(mRateCodeFragment.str().c_str());
}

char*
CDA_CCodeInformation::variableCodeFragment()
  throw (std::exception&)
{
  return strdup(mVariableCodeFragment.str().c_str());
}

char*
CDA_CCodeInformation::functionsFragment()
  throw (std::exception&)
{
  return strdup(mFunctionsFragment.str().c_str());
}

iface::cellml_services::CCodeVariableIterator*
CDA_CCodeInformation::iterateVariables()
  throw (std::exception&)
{
  return new CDA_CCodeVariableIterator(this, mVariables.begin(),
                                       mVariables.end());
}

iface::mathml_dom::MathMLNodeList*
CDA_CCodeInformation::flaggedEquations()
  throw (std::exception&)
{
  return new CDA_CCodeMathList(this, mFlaggedEquations);
}
