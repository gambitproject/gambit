//#
//# FILE: purenash.h -- Interface to pure-strategy Nash equilibrium finder
//#
//# $Id$
//#

#ifndef PURENASH_H
#define PURENASH_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"
#include "rational.h"
#include "glist.h"
#include "gtuple.h"

template <class T> int FindPureNash(const NormalForm<T> &N,
				    gList<gTuple<int> > &);


#endif    // PURENASH_H


