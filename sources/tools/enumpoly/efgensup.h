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

#include "libgambit/libgambit.h"

class ActionCursorForSupport;

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const gbtEfgSupport &s,
			     gbtEfgSupportWithActiveInfo *sact,
			     ActionCursorForSupport *c,
			     gbtList<gbtEfgSupport> &list);

gbtList<gbtEfgSupport> AllSubsupports(const gbtEfgSupport &S);


// Subsupports of a given support are _path equivalent_ if they
// agree on every infoset that can be reached under either, hence both,
// of them.  The next routine outputs one support for each equivalence
// class by outputting only those subsupports with _no_ active 
// actions at each unreached infoset.  

void AllInequivalentSubsupportsRECURSIVE(const gbtEfgSupport &s,
					 gbtEfgSupportWithActiveInfo *sact,
					 ActionCursorForSupport *c,
					 gbtList<gbtEfgSupport> &list);

gbtList<gbtEfgSupport> AllInequivalentSubsupports(const gbtEfgSupport &S);

// The following routines combine to return all supports that do not 
// exhibit particular type of domination.  This was a prototype for 
// PossibleNashSubsupports, and displays the methods used there,
// but it does NOT do exactly what is advertised with respect to 
// weak domination.  This is because the recursion may eliminate
// an action that is weakly dominated at some stage of the truncation
// process, when, after more truncations, it might be no longer weakly
// dominated, and thus part of an allowed subsupport.

void AllUndominatedSubsupportsRECURSIVE(const gbtEfgSupport &s,
					gbtEfgSupportWithActiveInfo *sact,
					ActionCursorForSupport *c,
					bool strong, bool conditional,
					gbtList<gbtEfgSupport> &list);
  
gbtList<gbtEfgSupport> AllUndominatedSubsupports(const gbtEfgSupport &S,
					       bool strong, bool conditional);

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

void PossibleNashSubsupportsRECURSIVE(const gbtEfgSupport &s,
				      gbtEfgSupportWithActiveInfo *sact,
				      ActionCursorForSupport *c,
				      gbtList<gbtEfgSupport> &list);

gbtList<gbtEfgSupport> SortSupportsBySize(gbtList<gbtEfgSupport> &);
  
gbtList<gbtEfgSupport> PossibleNashSubsupports(const gbtEfgSupport &S);

///////////////// Utility Cursor Class /////////////////////

class ActionCursorForSupport {
protected:
  const gbtEfgSupport *support;
        int pl;
        int iset;
        int act;

public:
  //Constructors and dtor
  ActionCursorForSupport(const gbtEfgSupport &S);
  ActionCursorForSupport(const ActionCursorForSupport &a);
  ~ActionCursorForSupport();

  // Operators
  ActionCursorForSupport &operator =(const ActionCursorForSupport &);
  bool                    operator==(const ActionCursorForSupport &) const;
  bool                    operator!=(const ActionCursorForSupport &) const;

  // Manipulation
  bool GoToNext();

  // Information
  Gambit::GameAction GetAction() const;
  int ActionIndex() const;
  Gambit::GameInfoset GetInfoset() const;
  int InfosetIndex() const;
  Gambit::GamePlayer GetPlayer() const;
  int PlayerIndex() const;

  bool IsLast() const;
  bool IsSubsequentTo(const Gambit::GameAction &) const;

  // Special
  bool InfosetGuaranteedActiveByPriorCommitments(const 
						     gbtEfgSupportWithActiveInfo *,
						 const Gambit::GameInfoset &);
  bool DeletionsViolateActiveCommitments(const gbtEfgSupportWithActiveInfo *,
					 const gbtList<Gambit::GameInfoset> *);
};


