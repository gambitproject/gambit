dnl
dnl $Source$
dnl $Date$
dnl $Revision$
dnl
dnl DESCRIPTION:
dnl Local macro definitions for Gambit configure.in
dnl

dnl ---------------------------------------------------------------------------
dnl WX_PATH_WXCONFIG(VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Test for wxWindows, and define WX_CFLAGS and WX_LIBS. Set WX_CONFIG
dnl environment variable to override the default name of the wx-config script
dnl to use.
dnl ---------------------------------------------------------------------------

AC_DEFUN(WX_PATH_WXCONFIG,
[
dnl 
dnl Get the cflags and libraries from the wx-config script
dnl
AC_ARG_WITH(wx-prefix, [  --with-wx-prefix=PREFIX   Prefix where wxWindows is in
stalled (optional)],
            wx_config_prefix="$withval", wx_config_prefix="")
AC_ARG_WITH(wx-exec-prefix,[  --with-wx-exec-prefix=PREFIX Exec prefix where wxW
indows is installed (optional)],
            wx_config_exec_prefix="$withval", wx_config_exec_prefix="")

  dnl deal with optional prefixes
  if test x$wx_config_exec_prefix != x ; then
     wx_config_args="$wx_config_args --exec-prefix=$wx_config_exec_prefix"
     if test x${WX_CONFIG+set} != xset ; then
        WX_CONFIG=$wx_config_exec_prefix/bin/wx-config
     fi
  fi
  if test x$wx_config_prefix != x ; then
     wx_config_args="$wx_config_args --prefix=$wx_config_prefix"
     if test x${WX_CONFIG+set} != xset ; then
        WX_CONFIG=$wx_config_prefix/bin/wx-config
     fi
  fi

  AC_PATH_PROG(WX_CONFIG, wx-config, no)
  min_wx_version=ifelse([$1], ,2.3.2,$1)
  AC_MSG_CHECKING(for wxWindows version >= $min_wx_version)
  no_wx=""
  if test "$WX_CONFIG" = "no" ; then
    no_wx=yes
  else
    WX_CFLAGS=`$WX_CONFIG $wx_config_args --cflags`
    WX_LIBS=`$WX_CONFIG $wx_config_args --libs`
    wx_config_major_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_config_minor_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_config_micro_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi

  if test "x$no_wx" = x ; then
     AC_MSG_RESULT(yes (version $wx_config_major_version.$wx_config_minor_version.$wx_config_micro_version))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     WX_CFLAGS=""
     WX_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(WX_CFLAGS)
  AC_SUBST(WX_LIBS)
])

