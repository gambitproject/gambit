//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerate undominated subsupports
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/gstatus.h"
#include "math/rational.h"
#include "efg.h"
#include "efgciter.h"
#include "actiter.h"

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const EFSupport *s,
			     EFSupportWithActiveInfo *sact,
			     gbtAllActionIterator *c,
			     gList<const EFSupport> *list);

gList<const EFSupport> AllSubsupports(const EFSupport &S);


// Subsupports of a given support are _path equivalent_ if they
// agree on every infoset that can be reached under either, hence both,
// of them.  The next routine outputs one support for each equivalence
// class by outputting only those subsupports with _no_ active 
// actions at each unreached infoset.  

void AllInequivalentSubsupportsRECURSIVE(const EFSupport *s,
					 EFSupportWithActiveInfo *sact,
					 gbtAllActionIterator *c,
					 gList<const EFSupport> *list);

gList<const EFSupport> AllInequivalentSubsupports(const EFSupport &S);

// The following routines combine to return all supports that do not 
// exhibit particular type of domination.  This was a prototype for 
// PossibleNashSubsupports, and displays the methods used there,
// but it does NOT do exactly what is advertised with respect to 
// weak domination.  This is because the recursion may eliminate
// an action that is weakly dominated at some stage of the truncation
// process, when, after more truncations, it might be no longer weakly
// dominated, and thus part of an allowed subsupport.

void AllUndominatedSubsupportsRECURSIVE(const EFSupport *s,
					 EFSupportWithActiveInfo *sact,
					 gbtAllActionIterator *c,
					const bool strong,
					const bool conditional,
					 gList<const EFSupport> *list,
					 const gStatus &status);
  
gList<const EFSupport> AllUndominatedSubsupports(const EFSupport &S,
						 const bool strong,
						 const bool conditional,
						 const gStatus &status);

// The following two routines combine to produce all subsupports that could
// host the path of a behavioral Nash equilibrium.  These are subsupports
// that have no action, at an active infoset, that is weakly dominated by
// another active action, either in the conditional sense (for any active
// node in the infoset) or the unconditional sense.  In addition we 
// check for domination by actions that are inactive, but whose activation
// would not activate any currently inactive infosets, so that the
// subsupport resulting from activation is consistent, in the sense
// of having active actions at all active infosets, and not at other
// infosets.

void PossibleNashSubsupportsRECURSIVE(const EFSupport *s,
					    EFSupportWithActiveInfo *sact,
				            gbtAllActionIterator *c,
					    gList<const EFSupport> *list,
				      const gStatus &status);

gList<const EFSupport> SortSupportsBySize(gList<const EFSupport> &);
  
gList<const EFSupport> PossibleNashSubsupports(const EFSupport &S,
					       gStatus &status);



