// File: efgsolng.h -- declarations for the GUI part of the extensive form
// algorithm solutions.  Each solution algorithm is contained in a separate class
// and the ExtensiveShow does not need to know any details
// $Id$
#ifndef EFGSOLNG_H
#define EFGSOLNG_H

// An interface class between ExtensiveSolutionG (and related) and ExtensiveShow
template <class T> class ExtensiveShowInterf
{
public:
	virtual void PickSolutions(const Efg<T> &,gList<BehavProfile<T> > &,int ) = 0;
	virtual void SetPickSubgame(const Node *n) = 0;
	virtual const gString &Filename(void) const = 0;
	virtual wxFrame *Frame(void) = 0;
};

template <class T> class ExtensiveSolutionG
{
protected:
	const Efg<T> &ef;
	ExtensiveShowInterf<T> *parent;
public:
	ExtensiveSolutionG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const = 0;
	virtual void SolveSetup(void) const = 0;
};

// Extensive Form Liap
template <class T> class EFLiapG : public ExtensiveSolutionG<T>
{
public:
	EFLiapG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Normal Form Liap
template <class T> class NFLiapG : public ExtensiveSolutionG<T>
{
public:
	NFLiapG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Seq Form
template <class T> class SeqFormG : public ExtensiveSolutionG<T>
{
public:
	SeqFormG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Lemke
template <class T> class LemkeG : public ExtensiveSolutionG<T>
{
public:
	LemkeG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Enum Pure
template <class T> class PureNashG : public ExtensiveSolutionG<T>
{
public:
	PureNashG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Enum Mixed
template <class T> class EnumG : public ExtensiveSolutionG<T>
{
public:
	EnumG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// LP (ZSum)
template <class T> class ZSumG : public ExtensiveSolutionG<T>
{
public:
	ZSumG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Simpdiv
template <class T> class SimpdivG : public ExtensiveSolutionG<T>
{
public:
	SimpdivG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// Gobit All
template <class T> class GobitAllG : public ExtensiveSolutionG<T>
{
public:
	GobitAllG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// NGobit
template <class T> class NGobitG : public ExtensiveSolutionG<T>
{
public:
	NGobitG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

// EGobit
template <class T> class EGobitG : public ExtensiveSolutionG<T>
{
public:
	EGobitG(const Efg<T> &E,ExtensiveShowInterf<T> *parent);
	virtual gList<BehavProfile<T> > Solve(void) const;
	virtual void SolveSetup(void) const;
};

#endif
