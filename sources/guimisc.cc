// File: guimisc.cc -- overloads some standard functions to provide extra
// functionality in the GUI enviroment.  This is only needed/working under
// windows.
// $Id$
#ifdef __BORLANDC__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "wx.h"
#include "wxio.h"

// This assert will work ONLY under a 16bit compile
extern "C"
void _Cdecl _FARFUNC __assertfail( char _FAR *__msg,
																	 char _FAR *__cond,
																	 char _FAR *__file,
																	 int __line)
{
char tempstr[500];
sprintf(tempstr,"\nAn internal error occured in GAMBIT.\nPlease contact the authors at gambit@hss.caltech.edu\nRecord the following data to help us track the bug.\nAssert :%s \nin file: %s, line=%d",__cond,__file,__line);
wxError(tempstr,"Assertion Failed");
wxFatalError("GAMBIT will now terminate","Fatal Error");
exit(1);
}

// This  assert will work ONLY under a 32bit compile
void _RTLENTRY _EXPFUNC _assert(char * __cond, char * __file, int __line)
{
char tempstr[500];
sprintf(tempstr,"\nAn internal error occured in GAMBIT.\nPlease contact the authors at gambit@hss.caltech.edu\nRecord the following data to help us track the bug.\nAssert :%s \nin file: %s, line=%d",__cond,__file,__line);
wxError(tempstr,"Assertion Failed");
//wxFatalError("GAMBIT will now terminate","Fatal Error");
exit(1);
}


// Take care of the output to stdout
extern "C"
int printf(const char _FAR *fmt, ...)
{
va_list ap;
static char buffer[512];
va_start(ap, fmt);
int ret=wvsprintf(buffer,fmt,ap) ;
(*wout)<<buffer;
return ret;
}

int     _RTLENTRY _EXPFUNC fprintf(FILE _FAR *__stream, const char _FAR *fmt, ...)
{
va_list ap;
static char buffer[512];
va_start(ap, fmt);
int ret;
if (__stream==stdout || __stream==stderr)
{
	ret=vsprintf(buffer,fmt,ap) ;
	(*wout)<<buffer;
}
else
	ret=vfprintf(__stream,fmt,ap);
return ret;
}
#endif
