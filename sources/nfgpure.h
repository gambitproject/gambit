//#
//# FILE: purenash.h -- Interface to pure-strategy Nash equilibrium finder
//#
//# $Id$
//#

#ifndef PURENASH_H
#define PURENASH_H

#include "nfg.h"
#include "mixed.h"
#include "glist.h"


template <class T> int FindPureNash(const Nfg<T> &N,
				    gList<MixedProfile<T> > &);


#endif    // PURENASH_H


