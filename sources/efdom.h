//
// FILE: efdom.h -- Compute dominated strategies on extensive form
//
// $Id: efdom.h
//

#include "efg.h"
#include "gstatus.h"

class efgDominanceException : public gException {
private:
  gText m_description;

public:
  efgDominanceException(const gText &);
  virtual ~efgDominanceException();

  gText Description(void) const;
};


// The following computes whether action a dominates action b.
// If `conditional' is false, then the computation is with respect
// to whether a dominates b with respect to all possibilities for 
// the game allowed by the support.  The argument `strong' describes
// whether domination is supposed to be strong or weak.  In particular,
// if strong is true and conditional is false, true cannot be
// returned unless all plays (given the support) go through the
// infoset where a and b might be chosen.

bool Dominates(const EFSupport &S, 
	       const int pl, 
	       const int iset, 
	       const int a, const int b, 
	       const bool strong,
	       const bool conditional,
	       const gStatus &status);

// Another window to the computation above.

bool Dominates(const EFSupport &S, 
	       const Action *a, const Action *b,
	       const bool strong,
	       const bool conditional,
	       const gStatus &status);

bool SomeListElementDominates(const EFSupport &S, 
			      const gList<Action *> &l,
			      const Action *a, 
			      const bool strong,
			      const bool conditional,
			      const gStatus &status);

bool InfosetHasDominatedElement(const EFSupport &S, 
				const Infoset *i,
				const bool strong,
				const bool conditional,
				const gStatus &status);

bool ElimDominatedInInfoset(const EFSupport &S, EFSupport &T,
			     const int pl, 
			     const int iset, 
			     const bool strong,
			     const bool conditional,
		                   gStatus &status);

bool ElimDominatedForPlayer(const EFSupport &S, EFSupport &T,
			    const int pl, 
			    const bool strong,
			    const bool conditional,
		                  gStatus &status);

EFSupport *SupportWithoutDominatedOfPlayerList(const EFSupport &S, 
					       const bool strong,
					       const bool conditional,
					       const gArray<int> &players,
					             gOutput &, // tracefile 
					             gStatus &status);

EFSupport *DominanceTruncatedSupport(const EFSupport &S, 
				     const bool strong,
				     const bool conditional,
				           gOutput & out, // tracefile 
				           gStatus &status);

EFSupport *UnconditionalDominanceTruncatedSupport(const EFSupport &S, 
						  const bool strong,
						   gOutput & out, // tracefile 
						   gStatus &status);
