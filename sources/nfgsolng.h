// File: nfgsolng.h -- defines a class to take care of the normal form
// solution algorithms.  It is also used for the NF solution mode in the
// extensive form.  The actual code in in normshow.cc
// $Id$
#ifndef NFGSOLNG_H
#define NFGSOLNG_H
#include "gmisc.h"
#include "gslist.h"
#include "mixedsol.h"

class wxFrame;
class NFSupport;

template <class T> class NormalSolutions: public EfgNfgInterface<T>
{
private:
	wxFrame *&parent_window;
  bool from_efg;
	int	 SolveLCP(const NFSupport *sup);
	int	 SolveEnumPure(const NFSupport *sup);
	int	 SolveGobitAll(const NFSupport *sup);
	int	 SolveGobit(const NFSupport *sup);
	int	 SolveLiap(const NFSupport *sup);
	int	 SolveSimpdiv(const NFSupport *sup);
	int	 SolveEnumMixed(const NFSupport *sup);
	int	 SolveLP(const NFSupport *sup);
// This function is necessary to make sure that the user can not run any
// optimization solutions using gRationals
	void doubles_only(void);
	// Make a support based on the elimdom options
	NFSupport *MakeSolnSupport(void);
protected:
	Nfg<T> &nf;
	gList<int>		got_solns;
	gSortList<MixedSolution<T> > solns;
	struct StartingPoints
	{
		gSortList<MixedSolution<T> > profiles;
		int last;
		StartingPoints() : last(-1) { }
	} starting_points;

	gString filename;
public:
// Constructor
	NormalSolutions(Nfg<T> &N,wxFrame *&parent,EfgNfgInterface<T> *efg,
									const gString &fn,bool from_efg=false);
// Top level
	bool Solve(NfgSolutionT alg,bool to_extensive=false,const NFSupport *sup=0);
	void SolveSetup(NfgSolutionT alg);
// Project solutions to EF.
	void SolutionToExtensive(const MixedSolution<T> &mp,bool set=false);
  virtual MixedProfile<T> CreateStartProfile(int how) = 0;

};

#endif

