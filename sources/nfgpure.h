//#
//# FILE: purenash.h -- Interface to pure-strategy Nash equilibrium finder
//#
//# $Id$
//#

#ifndef PURENASH_H
#define PURENASH_H

#include "normal.h"
#include "rational.h"
#include "glist.h"
#include "gtuple.h"

template <class T> int FindPureNash(const NormalForm<T> &N,
				    gList<gPVector<T> > &);


#endif    // PURENASH_H


