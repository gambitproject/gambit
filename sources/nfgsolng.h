// File: nfgsolng.h -- dnfines a class to take care of the normal form
// solution algorithms.  It is also used for the NF solution mode in the
// extensive form.
//  $Id$

#ifndef NFGSOLNG_H
#define NFGSOLNG_H
#include "mixedsol.h"
#include "paramsd.h"

// An interface class between NfgSolutionG (and related) and NfgShow
class NfgShowInterface
{
public:
	virtual ParameterSetList &Parameters(void) = 0;
	virtual MixedProfile<gNumber> CreateStartProfile(int how) = 0;
	virtual const gString &Filename(void) const = 0;
	virtual wxFrame *Frame(void) = 0;
};

class NfgSolutionG
{
protected:
	const Nfg &nf;
	const NFSupport &sup;
	NfgShowInterface *parent;
   gList<MixedSolution> solns;
public:
	NfgSolutionG(const Nfg &E,const NFSupport &S,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const = 0;
	virtual void SolveSetup(void) const = 0;
};


// Extensive Form Liap
class NfgLiapG : public NfgSolutionG
{
public:
	NfgLiapG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgLemkeG : public NfgSolutionG
{
public:
	NfgLemkeG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgEnumPureG : public NfgSolutionG
{
public:
	NfgEnumPureG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgGobitAllG : public NfgSolutionG
{
public:
	NfgGobitAllG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgGobitG : public NfgSolutionG
{
public:
	NfgGobitG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgSimpdivG : public NfgSolutionG
{
public:
	NfgSimpdivG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgEnumG : public NfgSolutionG
{
public:
	NfgEnumG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};

class NfgZSumG : public NfgSolutionG
{
public:
	NfgZSumG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
	virtual gList<MixedSolution> Solve(void) const;
	virtual void SolveSetup(void) const;
};


#endif

