#include "cda_compiler_support.h"
#ifdef IN_XPATH_MODULE
#define XPATH_PUBLIC_PRE CDA_EXPORT_PRE
#define XPATH_PUBLIC_POST CDA_EXPORT_POST
#else
#define XPATH_PUBLIC_PRE CDA_IMPORT_PRE
#define XPATH_PUBLIC_POST CDA_IMPORT_POST
#endif

#include "Ifacexpath.hxx"

XPATH_PUBLIC_PRE
  already_AddRefd<iface::xpath::XPathEvaluator> CreateXPathEvaluator()
XPATH_PUBLIC_POST;
