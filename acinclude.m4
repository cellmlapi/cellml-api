AC_DEFUN([CHECK_SHORT_WCHAR_OPTION],
[# Now, see if a -fshort-wchar option is available...
SAVE_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$SAVE_CXXFLAGS -fshort-wchar"
AC_CACHE_CHECK([for -fshort-wchar], short_wchar_option,
  [AC_LANG_PUSH(C++)
   AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
                  [short_wchar_option="-fshort-wchar"],[short_wchar_option=])
   AC_LANG_POP])
CXXFLAGS="$SAVE_CXXFLAGS"
]) # CHECK_SHORT_WCHAR_OPTION

m4_include(m4/ltversion.m4)
m4_include(m4/libtool.m4)
m4_include(m4/ltoptions.m4)
m4_include(m4/ltsugar.m4)
m4_include(m4/lt~obsolete.m4)
m4_include(m4/ltdl.m4)
