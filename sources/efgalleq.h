//
// FILE: efgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef EFGALLEQ_H
#define EFGALLEQ_H

#include "efg.h"
#include "glist.h"
#include "gstatus.h"
#include "behav.h"
#include "behavsol.h"
#include "epolenum.h" 
#include "efgensup.h"

int AllEFNashSolve(const EFSupport &, const EfgPolEnumParams &, 
		 gList<BehavSolution> &, long &nevals, double &time,
		 gList<const EFSupport> &singular_supports);

#endif    // EFGALLEQ_H




