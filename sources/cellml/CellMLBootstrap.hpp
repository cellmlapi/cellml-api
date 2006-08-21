#ifdef IN_CELLML_MODULE
#define CELLML_PUBLIC CDA_EXPORT
#else
#define CELLML_PUBLIC CDA_IMPORT
#endif

iface::cellml_api::CellMLBootstrap* CreateCellMLBootstrap() CELLML_PUBLIC;
