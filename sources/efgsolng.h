// File: efgsolng.h -- declarations for the GUI part of the extensive form
// algorithm solutions.  Each solution algorithm is contained in a separate class
// and the ExtensiveShow does not need to know any details
// $Id$
#ifndef EFGSOLNG_H
#define EFGSOLNG_H
#include "behavsol.h"
typedef BehavSolution<gNumber> BehavSolutionT;
typedef BehavProfile<gNumber> BehavProfileT;

// An interface class between ExtensiveSolutionG (and related) and ExtensiveShow
class EfgShowInterface
{
public:
	virtual void PickSolutions(const Efg &,gList<BehavSolutionT > &) = 0;
	virtual BehavProfileT CreateStartProfile(int how) = 0;
	virtual void SetPickSubgame(const Node *n) = 0;
	virtual const gString &Filename(void) const = 0;
	virtual wxFrame *Frame(void) = 0;
};

class ExtensiveSolutionG
{
protected:
	const Efg &ef;
	const EFSupport &sup;
	EfgShowInterface *parent;
public:
	ExtensiveSolutionG(const Efg &E,const EFSupport &S,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const = 0;
	virtual void SolveSetup(void) const = 0;
};

// Extensive Form Liap
class EFLiapG : public ExtensiveSolutionG
{
public:
	EFLiapG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Normal Form Liap
class NFLiapG : public ExtensiveSolutionG
{
public:
	NFLiapG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Seq Form
class SeqFormG : public ExtensiveSolutionG
{
public:
	SeqFormG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Lemke
class LemkeG : public ExtensiveSolutionG
{
public:
	LemkeG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Enum Pure
class PureNashG : public ExtensiveSolutionG
{
public:
	PureNashG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};
// Efg Pure Nash
class EPureNashG : public ExtensiveSolutionG
{
public:
	EPureNashG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Enum Mixed
class EnumG : public ExtensiveSolutionG
{
public:
	EnumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// LP (ZSum)
class ZSumG : public ExtensiveSolutionG
{
public:
	ZSumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// EfgCSum
class EfgCSumG : public ExtensiveSolutionG
{
public:
	EfgCSumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Simpdiv
class SimpdivG : public ExtensiveSolutionG
{
public:
	SimpdivG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Gobit All
class GobitAllG : public ExtensiveSolutionG
{
public:
	GobitAllG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// NGobit
class NGobitG : public ExtensiveSolutionG
{
public:
	NGobitG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// EGobit
class EGobitG : public ExtensiveSolutionG
{
public:
	EGobitG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
	virtual gList<BehavSolutionT > Solve(void) const;
	virtual void SolveSetup(void) const;
};

#endif
