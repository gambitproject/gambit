//
// FILE: nfgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef NFGALLEQ_H
#define NFGALLEQ_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "mixed.h"
#include "mixedsol.h"
#include "polenum.h" 
#include "nfgensup.h"

int AllNashSolve(const NFSupport &, const PolEnumParams &, 
		 gList<MixedSolution> &, gStatus &, 
		 long &nevals, double &time,
		 gList<const NFSupport> &singular_supports);

#endif    // NFGALLEQ_H




