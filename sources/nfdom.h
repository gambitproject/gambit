//
// FILE: nfdom.h -- Compute dominated strategies on normal form
//
// $Id$
//

#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "rational.h"
#include "gstatus.h"
#include "nfplayer.h"

bool Dominates(const Nfg &,
	       const NFSupport &S, int pl, int a, int b, bool strong);

bool Dominates(const NFSupport &S, Strategy *s, Strategy *t, bool strong,
		     gStatus &status);

bool IsDominated(const NFSupport &S, Strategy *s, bool strong, 
		       gStatus &status);

bool ComputeDominated(const Nfg &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status);

NFSupport *ComputeDominated(const Nfg &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status);
