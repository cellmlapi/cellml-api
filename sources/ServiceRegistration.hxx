#include <inttypes.h>
#include <exception>
#include "IfaceCellML_Context.hxx"

// These must be provided by the service...
int do_registration(iface::cellml_context::CellMLContext* aContext,
                    iface::cellml_context::CellMLModuleManager* aModuleManager);
void do_deregistration(iface::cellml_context::CellMLModuleManager*
                       aModuleManager);

// This next signature is provided by the runner...
void UnloadService();
