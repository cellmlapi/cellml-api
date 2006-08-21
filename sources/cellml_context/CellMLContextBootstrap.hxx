#include "IfaceCellML_Context.hxx"
#include "Utilities.hxx"

#ifdef IN_CELLMLCONTEXT_MODULE
#define CELLML_CONTEXT_PUBLIC CDA_EXPORT
#else
#define CELLML_CONTEXT_PUBLIC CDA_IMPORT
#endif

CELLML_CONTEXT_PUBLIC iface::cellml_context::CellMLContext* CreateCellMLContext();
