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
#include "rational.h"
#include "gstatus.h"
#include "nfplayer.h"
#include "mixed.h"

bool Dominates(const Nfg &,const NFSupport &S, int pl, int a, int b, 
	       const gRectArray<gNumber> *, bool strong);

bool Dominates(const Nfg &,const NFSupport &S, int pl, int a, int b, 
	       const bool strong);

bool Dominates(const NFSupport &S, Strategy *s, Strategy *t, bool strong,
	       const gStatus &status);

bool Dominates(const NFSupport &S, Strategy *s, Strategy *t, bool strong);

bool IsDominated(const NFSupport &S, Strategy *s, bool strong, 
		 const gStatus &status);

void InitializePaytable(const Nfg &N, gRectArray<gNumber> *paytable); 

bool ComputeDominated(const Nfg &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status);

NFSupport *ComputeDominated(const Nfg &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status);


NFSupport *ComputeMixedDominated(NFSupport &, bool strong,
				 gPrecision precision,
				 const gArray<int> &players,
				 gOutput &, gStatus &status);
bool IsMixedDominated(const NFSupport &S,Strategy *str,
			   bool strong, gPrecision precision,
			   gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol, int pl,
		 bool strong, gPrecision precision, gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol,
		 bool strong, gPrecision precision, gOutput &tracefile);

#endif    // NFDOM_H


