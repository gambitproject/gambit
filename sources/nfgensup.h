//
// FILE: nfgensup.h --Enumerate (Undominated) Subsupports of a Support
//
// $Id$
//

#include "nfg.h"
#include "rational.h"
#include "gstatus.h"
#include "glist.h"
#include "nfdom.h"
#include "nfgciter.h"

class StrategyCursorForSupport;

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const NFSupport *s,
			     NFSupport *sact,
			     StrategyCursorForSupport *c,
			     gList<const NFSupport> *list);

gList<const NFSupport> AllSubsupports(const NFSupport &S);

// Subsupports of a given support are _valid_ if each agent has an action.

void AllValidSubsupportsRECURSIVE(const NFSupport *s,
					 NFSupport *sact,
					 StrategyCursorForSupport *c,
					 gList<const NFSupport> *list);

gList<const NFSupport> AllValidSubsupports(const NFSupport &S);

// The following routines combine to return all supports that do not 
// exhibit particular type of domination.  This was a prototype for 
// PossibleNashSubsupports, and displays the methods used there,
// but it does NOT do exactly what is advertised with respect to 
// weak domination.  This is because the recursion may eliminate
// a strategy that is weakly dominated at some stage of the truncation
// process, when, after more truncations, it might be no longer weakly
// dominated, and thus part of an allowed subsupport.

void AllUndominatedSubsupportsRECURSIVE(const NFSupport *s,
					      NFSupport *sact,
					      StrategyCursorForSupport *c,
					const bool strong,
					      gList<const NFSupport> *list,
					gStatus &status);
  
gList<const NFSupport> AllUndominatedSubsupports(const NFSupport &S,
						 const bool strong,
						 const bool conditional,
						 gStatus &status);

// The following two routines combine to produce all subsupports that could
// host the path of a behavioral Nash equilibrium.  These are subsupports
// that have no strategy, at an active infoset, that is weakly dominated by
// another active strategy, either in the conditional sense (for any active
// node in the infoset) or the unconditional sense.  In addition we 
// check for domination by strategys that are inactive, but whose activation
// would not activate any currently inactive infosets, so that the
// subsupport resulting from activation is consistent, in the sense
// of having active strategys at all active infosets, and not at other
// infosets.

void PossibleNashSubsupportsRECURSIVE(const NFSupport *s,
					    NFSupport *sact,
				            StrategyCursorForSupport *c,
					    gList<const NFSupport> *list,
				      gStatus &status);
  
gList<const NFSupport> PossibleNashSubsupports(const NFSupport &S,
					       gStatus &status);

///////////////// Utility Cursor Class /////////////////////

class StrategyCursorForSupport {
protected:
  const NFSupport *support;
  int pl;
  int strat;

public:
  //Constructors and dtor
  StrategyCursorForSupport(const NFSupport &S);
  StrategyCursorForSupport(const StrategyCursorForSupport &s);
  ~StrategyCursorForSupport();

  // Operators
  StrategyCursorForSupport &operator =(const StrategyCursorForSupport &);
  bool                    operator==(const StrategyCursorForSupport &) const;
  bool                    operator!=(const StrategyCursorForSupport &) const;

  // Manipulation
  bool GoToNext();

  // Information
  const Strategy *GetStrategy() const;
  int StrategyIndex() const;
  const NFPlayer *GetPlayer() const;
  int PlayerIndex() const;

  bool IsLast() const;
  bool IsSubsequentTo(const Strategy *) const;
};


//////////////////////Testing////////////////////

//void AndyTest(const NFSupport &S, gStatus &status);
