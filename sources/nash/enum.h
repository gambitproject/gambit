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

#ifndef ENUM_H
#define ENUM_H

#include "base/base.h"
#include "game/nfg.h"
#include "algutils.h"
#include "game/mixed.h"
#include "game/mixedsol.h"
#include "numerical/vertenum.h"

class EnumParams : public AlgParams {
public:
  bool cliques;

  EnumParams(void);
};

int Enum(const NFSupport &, const EnumParams &,
	 gList<MixedSolution> &, gOutput &,
	 gStatus &, long &npivots, double &time);

#endif    // ENUM_H




