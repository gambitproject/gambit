//
// FILE: nfgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef NFGALLEQ_H
#define NFGALLEQ_H

#include "base/base.h"
#include "game/nfg.h"
#include "gstatus.h"
#include "game/mixed.h"
#include "game/mixedsol.h"
#include "polenum.h" 
#include "nfgensup.h"

int AllNashSolve(const NFSupport &, const PolEnumParams &, 
		 gList<MixedSolution> &, gStatus &, 
		 long &nevals, double &time,
		 gList<const NFSupport> &singular_supports);

#endif    // NFGALLEQ_H




