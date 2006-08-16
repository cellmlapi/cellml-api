noinst_LTLIBRARIES += libltdlc.la $(LT_DLLOADERS)

libltdlc_la_SOURCES := \
  libltdl/libltdl/lt__alloc.h \
  libltdl/libltdl/lt__dirent.h \
  libltdl/libltdl/lt__glibc.h \
  libltdl/libltdl/lt__private.h \
  libltdl/libltdl/lt__strl.h \
  libltdl/libltdl/lt_dlloader.h \
  libltdl/libltdl/lt_error.h \
  libltdl/libltdl/lt_system.h \
  libltdl/libltdl/slist.h \
  libltdl/loaders/preopen.c \
  libltdl/lt__alloc.c \
  libltdl/lt_argz.c \
  libltdl/lt_dlloader.c \
  libltdl/lt_error.c \
  libltdl/ltdl.c \
  libltdl/ltdl.h \
  libltdl/slist.c

libltdlc_la_CPPFLAGS		:= -DLTDL -DLTDLOPEN=libltdlc $(AM_CPPFLAGS) -I$(top_srcdir)/libltdl -I$(top_srcdir)/libltdl/libltdl
libltdlc_la_LDFLAGS		:= $(AM_LDFLAGS) $(LTDL_VERSION_INFO) $(LT_DLPREOPEN)
libltdlc_la_LIBADD		:= $(LTLIBOBJS)
libltdlc_la_DEPENDENCIES	:= $(LT_DLLOADERS) $(LTLIBOBJS)

EXTRA_LTLIBRARIES	       += dlopen.la \
				  dld_link.la \
				  dyld.la \
				  load_add_on.la \
				  loadlibrary.la \
				  shl_load.la

dlopen_la_SOURCES	:= libltdl/loaders/dlopen.c
dlopen_la_LDFLAGS	:= -module -avoid-version
dlopen_la_LIBADD 	:= $(LIBADD_DLOPEN)
dlopen_la_CPPFLAGS      := $(libltdlc_la_CPPFLAGS)

dld_link_la_SOURCES	:= libltdl/loaders/dld_link.c
dld_link_la_LDFLAGS	:= -module -avoid-version
dld_link_la_LIBADD	:= -ldld
dld_link_la_CPPFLAGS      := $(libltdlc_la_CPPFLAGS)

dyld_la_SOURCES		:= libltdl/loaders/dyld.c
dyld_la_LDFLAGS		:= -module -avoid-version
dyld_la_CPPFLAGS      := $(libltdlc_la_CPPFLAGS)

load_add_on_la_SOURCES	:= libltdl/loaders/load_add_on.c
load_add_on_la_LDFLAGS	:= -module -avoid-version
load_add_on_la_CPPFLAGS := $(libltdlc_la_CPPFLAGS)

loadlibrary_la_SOURCES	:= libltdl/loaders/loadlibrary.c
loadlibrary_la_LDFLAGS	:= -module -avoid-version
loadlibrary_la_CPPFLAGS := $(libltdlc_la_CPPFLAGS)

shl_load_la_SOURCES	:= libltdl/loaders/shl_load.c
shl_load_la_LDFLAGS	:= -module -avoid-version
shl_load_la_LIBADD	:= $(LIBADD_SHL_LOAD)
shl_load_la_CPPFLAGS    := $(libltdlc_la_CPPFLAGS)

## Make sure these will be cleaned even when they're not built by default:
CLEANFILES += \
  libltdl.la \
  libltdlc.la \
  libdlloader.la

$(libltdlc_la_OBJECTS): libtool
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
