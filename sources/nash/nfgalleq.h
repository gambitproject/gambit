//
// FILE: nfgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef NFGALLEQ_H
#define NFGALLEQ_H

#include "base/base.h"
#include "base/gstatus.h"
#include "game/nfg.h"
#include "game/mixed.h"
#include "game/mixedsol.h"
#include "game/nfgensup.h"
#include "polenum.h" 

int AllNashSolve(const NFSupport &, const PolEnumParams &, 
		 gList<MixedSolution> &, gStatus &, 
		 long &nevals, double &time,
		 gList<const NFSupport> &singular_supports);

#endif    // NFGALLEQ_H




