lib_LTLIBRARIES += libcis.la

CVODE_SOURCES = \
  $(top_srcdir)/CIS/sources/sundials/cvode_band.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_bandpre.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_bbdpre.c \
  $(top_srcdir)/CIS/sources/sundials/cvode.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_dense.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_diag.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_io.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_spbcgs.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_spgmr.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_spils.c \
  $(top_srcdir)/CIS/sources/sundials/cvode_sptfqmr.c \
  $(top_srcdir)/CIS/sources/sundials/nvector_serial.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_band.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_dense.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_iterative.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_math.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_nvector.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_smalldense.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_spbcgs.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_spgmr.c \
  $(top_srcdir)/CIS/sources/sundials/sundials_sptfqmr.c

libcis_la_SOURCES = \
  $(top_srcdir)/CIS/sources/CISImplementation.cxx \
  $(top_srcdir)/CIS/sources/CISSolve.cxx \
  $(CVODE_SOURCES)

libcis_la_LIBADD = \
  $(top_builddir)/libccgs.la -lgsl $(STLLINK)

libcis_la_CXXFLAGS = \
  -Wall -ggdb -I$(top_srcdir)/sources -I$(top_builddir)/interfaces \
  -I$(top_srcdir)/CIS/sources/sundials $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCIS.hxx \
  $(top_srcdir)/CIS/sources/CISBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCIS.hxx

include $(top_srcdir)/CIS/tests/build.mk
