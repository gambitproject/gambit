//
// FILE: nfdom.h -- Compute dominated strategies on normal form
//
// $Id$
//

#ifndef NFDOM_H
#define NFDOM_H

#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "math/rational.h"
#include "gstatus.h"
#include "nfplayer.h"
#include "mixed.h"

bool IsMixedDominated(const NFSupport &S,Strategy *str,
		      bool strong, gPrecision precision,
		      gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol, int pl,
		      bool strong, gPrecision precision, gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol,
		      bool strong, gPrecision precision, gOutput &tracefile);

#endif    // NFDOM_H


