//#
//# FILE: mygmisc.h -- Miscellaneous generally-useful functions (for AMM)
//#
//# @(#)mygmisc.h	1.0 8/15/96
//#

#ifndef MYGMISC_H
#define MYGMISC_H

#include "gmisc.h"
#include "double.h"
#include "gstring.h"

//
// Converting between strings and other datatypes
//

gString         ToString(gDouble);
//double		FromString(const gString &s, gDouble &d);
gDouble          TOgDouble(const gString &);

#endif    // MYGMISC_H
