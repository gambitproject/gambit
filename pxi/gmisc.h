//
// File: gmisc.h--a header file defining some general purpose functions that
//

// should be a part of the g* package.  Currently includes conversions for
// gNumber and gString

#include <stdio.h>		// for the ?printf and ?scanf functions
#include <stdlib.h>		// for the strtod functions
#include "gstring.h"

int FindStringInFile(gInput &in,const char *s);
//--------------------------------------------------------------------------
//                      Simple mathematical functions
//--------------------------------------------------------------------------

template <class T> T gmin(const T &a, const T &b)
{if (a < b)   return a;   else return b;}

template <class T> T gmax(const T &a, const T &b)
{if (a > b)   return a;   else return b;}

