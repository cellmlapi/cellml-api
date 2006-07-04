noinst_LTLIBRARIES += libltdlc.la
libltdlc_la_SOURCES := libltdl/ltdl.c
# libltdlc_la_LIBADD := $(LIBADD_DL)

## Because we do not have automatic dependency tracking:
# ltdl.lo: libltdl/ltdl.h config.h

$(libltdlc_la_OBJECTS): libtool
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
