//#
//# FILE: purenash.h -- Interface to pure-strategy Nash equilibrium finder
//#
//# @(#)purenash.h	1.3 1/26/95
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
				    gList<gPVector<T> > &);


#endif    // PURENASH_H


