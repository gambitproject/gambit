//
// FILE: nfgalleq.h -- enumerates all Nash equilibria, assuming genericity
//
// $Id$
//

#ifndef NFGALLEQ_H
#define NFGALLEQ_H

#include "polenum.h"
#include "nfgensup.h"


int NfgAllNash(const Nfg &, const PolEnumParams &, 
	       gList<MixedSolution> &, long &nevals, double &time);

#endif    // NFGALLEQ_H




