//
// FILE: enum.h -- Interface to Nash Enum solution module
//
// $Id$
//

//
// Enum implements the procedure described in 
// Mangasarian, O. L., "Equilibrium points of bimatrix games", 
// SIAM 12 (1964): 778-780 for finding all extreme points of 
// the Nash equilibrium components of a two person game. 
// 
// added comment to test RCS

#ifndef ENUM_H
#define ENUM_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "mixed.h"
#include "mixedsol.h"
#include "vertenum.h"

class EnumParams     {
public:
  int trace, stopAfter;
  Precision precision;
  gOutput *tracefile;
  gStatus &status;

  EnumParams(gStatus &status_=gstatus);
};


int Enum(const NFSupport &, const EnumParams &,
	 gList<MixedSolution> &, long &npivots, double &time);

#endif    // ENUM_H




