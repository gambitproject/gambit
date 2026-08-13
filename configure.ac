dnl
dnl This file is part of Gambit
dnl Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
dnl
dnl FILE: configure.ac
dnl autoconf configuration script for Gambit
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl

AC_INIT([gambit],[m4_esyscmd_s([cat build_support/GAMBIT_VERSION])])
AC_CONFIG_SRCDIR([src/gambit.h])
AM_INIT_AUTOMAKE([subdir-objects foreign])
dnl AC_CONFIG_MACRO_DIR([m4])
AM_OPTIONS_WXCONFIG

dnl User-specified options
dnl By default, we build the GUI
AC_ARG_ENABLE(gui,
[  --disable-gui           don't build graphical interface ],
[ case "${enableval}" in
  yes) with_gui=true ;;
  no)  with_gui=false ;;
  *)  AC_MSG_ERROR(bad value ${enableval} for --enable-gui) ;;
 esac], [with_gui=true])
AM_CONDITIONAL(WITH_GUI, test x$with_gui = xtrue)

AC_DEFUN([MINGW_AC_WIN32_NATIVE_HOST],
[AC_CACHE_CHECK([whether we are building for a Win32 host],
                [mingw_cv_win32_host],
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#ifdef _WIN32
choke me
#endif
])], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
dnl LT_INIT
AM_PROG_CC_C_O
MINGW_AC_WIN32_NATIVE_HOST
AM_CONDITIONAL(IS_WIN32, [test x$mingw_cv_win32_host = xyes])

dnl Require C++20 explicitly.
AX_CXX_COMPILE_STDCXX(20, [noext], [mandatory])

if test x$with_gui = xtrue; then
  dnl------------------------
  dnl Checking for wxWidgets
  dnl------------------------

  dnl check for wx-config
  AM_PATH_WXCONFIG(3.2.0, WXCONFIG=1)
  if test "$WXCONFIG" != 1; then
      AC_MSG_WARN([
            Did not find a new enough version of wxWidgets; disabling build of
            graphical interface.  If you want to build the graphical interface,
            please check that wx-config is in path, the directory
            where wxWidgets libraries are installed (returned by
            'wx-config --libs' command) is in LD_LIBRARY_PATH or
            equivalent variable and wxWidgets is version 3.2.0 or above.
      ])
      AM_CONDITIONAL(WITH_GUI, test x0 = x1)
  fi
fi

if test x$with_gui = xtrue; then
  dnl-----------------------------
  dnl Checking for wxWidgets port
  dnl-----------------------------

  AC_MSG_CHECKING(for wxWidgets port)
  if test `echo ${WX_CXXFLAGS} | grep -c __WXX11__` -eq 1; then
    AC_MSG_RESULT(universal)
    AC_MSG_WARN([
            Found universal port of wxWidgets; disabling build of graphical
            interface.  The graphical interface cannot be built with this
            port because this port does not implement drag-and-drop capability.
            Gambit recommends the GTK port of wxWidgets for best results.
    ])
    AM_CONDITIONAL(WITH_GUI, test x0 = x1)
  fi

  if test `echo ${WX_CXXFLAGS} | grep -c __WXMOTIF__` -eq 1; then
    AC_MSG_RESULT(Motif)
    AC_MSG_WARN([
            Found Motif port of wxWidgets; disabling build of graphical
            interface.  The graphical interface cannot be built with this
            port because this port does not implement drag-and-drop capability.
            Gambit recommends the GTK port of wxWidgets for best results.
    ])
    AM_CONDITIONAL(WITH_GUI, test x0 = x1)
  fi

  if test `echo ${WX_CXXFLAGS} | grep -c __WXGTK__` -eq 1; then
    AC_MSG_RESULT(GTK)
    WXPORT="GTK"
  fi

  if test `echo ${WX_CXXFLAGS} | grep -c __WXMSW__` -eq 1; then
    AC_MSG_RESULT(Windows)
    WXPORT="MSW"
    dnl Windows uses the resources file
    dnl RC_OBJECT="gambitrc.o"
    dnl RC_OBJECT_PATH="bitmaps/gambitrc.o"
    dnl AC_SUBST(RC_OBJECT)
    dnl AC_SUBST(RC_OBJECT_PATH)
    dnl We insist on static build on Windows, not depending on
    dnl a shared wxWidgets library.
    WX_CXXFLAGS=`echo $WX_CXXFLAGS | sed 's/-DWXUSINGDLL//g'`
  fi

  if test `echo ${WX_CXXFLAGS} | grep -c __WXMAC__` -eq 1; then
    AC_MSG_RESULT(Macintosh)
    WXPORT="MAC"
  fi

  if test -z "$WXPORT"; then
    AC_MSG_WARN([
            Could not identify wxWidgets port.  Disabling build
            of Gambit graphical interface.
    ])
    AM_CONDITIONAL(WITH_GUI, test x0 = x1)
  fi
fi

AC_SUBST(WX_LIBS)
AC_SUBST(WX_LIBS_STATIC)
AC_SUBST(WX_CXXFLAGS)


REZFLAGS=`echo $CXXFLAGS $WX_CXXFLAGS | sed 's/-mthreads//g' | sed 's/-g//g' | sed 's/-O. / /g' | sed 's/-I/--include-dir /g'`
AC_SUBST(REZFLAGS)

GAMBIT_VERSION=`cat build_support/GAMBIT_VERSION`
AC_SUBST(GAMBIT_VERSION)

dnl WiX requires the Product/@Version attribute to be a purely numeric
dnl "major.minor.build[.revision]" string, which a SemVer prerelease tag
dnl (e.g. "17.0.0-alpha.1") is not.  Windows Installer's ProductVersion is
dnl actually major.minor.build (not major.minor.micro): it only looks at
dnl the first THREE fields, and silently ignores any fourth, so the build
dnl field is where prerelease ordering has to live.
dnl
dnl This matches the alpha/beta/rc nomenclature used by the rest of the
dnl release tooling (see "Making a new release" in
dnl doc/developer.contributing.rst).  The build field is carved into
dnl bands, each with far more headroom than a release cycle needs:
dnl   alpha.N -> 0     + N
dnl   beta.N  -> 10000 + N
dnl   rc.N    -> 20000 + N
dnl   (final) -> 60000 + N
dnl N is the prerelease number (e.g. the "1" in "alpha.1"), with the
dnl GAMBIT_VERSION patch component folded in as a *100 offset within each
dnl band -- a no-op today since Gambit's patch component has always been
dnl 0, but it keeps e.g. a hypothetical "17.0.1-alpha.1" ordered above
dnl "17.0.0" without revisiting this scheme.
GAMBIT_MSI_BASE=${GAMBIT_VERSION%%-*}
GAMBIT_MSI_PRERELEASE=${GAMBIT_VERSION#$GAMBIT_MSI_BASE}

save_IFS=$IFS
IFS=.
set -- $GAMBIT_MSI_BASE
IFS=$save_IFS
GAMBIT_MSI_MAJOR=$1
GAMBIT_MSI_MINOR=$2
GAMBIT_MSI_PATCH=$3

case $GAMBIT_MSI_PRERELEASE in
  -alpha.*) GAMBIT_MSI_STAGE=0;     GAMBIT_MSI_PRENUM=${GAMBIT_MSI_PRERELEASE#-alpha.} ;;
  -beta.*)  GAMBIT_MSI_STAGE=10000; GAMBIT_MSI_PRENUM=${GAMBIT_MSI_PRERELEASE#-beta.} ;;
  -rc.*)    GAMBIT_MSI_STAGE=20000; GAMBIT_MSI_PRENUM=${GAMBIT_MSI_PRERELEASE#-rc.} ;;
  '')       GAMBIT_MSI_STAGE=60000; GAMBIT_MSI_PRENUM=0 ;;
  *)        GAMBIT_MSI_STAGE=0;     GAMBIT_MSI_PRENUM=0 ;;
esac
test "$GAMBIT_MSI_STAGE" -lt 60000 && test "$GAMBIT_MSI_PRENUM" -gt 99 && GAMBIT_MSI_PRENUM=99

GAMBIT_MSI_BUILD=`expr $GAMBIT_MSI_STAGE + $GAMBIT_MSI_PATCH \* 100 + $GAMBIT_MSI_PRENUM`
GAMBIT_MSI_VERSION="$GAMBIT_MSI_MAJOR.$GAMBIT_MSI_MINOR.$GAMBIT_MSI_BUILD"
AC_SUBST(GAMBIT_MSI_VERSION)
AC_SUBST(GAMBIT_MSI_MAJOR)

AC_CONFIG_FILES([Makefile build_support/osx/Info.plist build_support/msw/gambit.wxs])
AC_OUTPUT
