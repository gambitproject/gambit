//
// FILE: nfgsolng.h -- defines a class to take care of the normal form
//                     solution algorithms.  It is also used for the normal
//                     form solution mode in the extensive form
//
// $Id$
//

#ifndef NFGSOLNG_H
#define NFGSOLNG_H

#include "mixedsol.h"

// An interface class between NfgSolutionG (and related) and NfgShow
class NfgShowInterface   {
 public:
  virtual ~NfgShowInterface()  { }
  virtual MixedProfile<gNumber> CreateStartProfile(int how) = 0;
  virtual const gText &Filename(void) const = 0;
  virtual wxFrame *Frame(void) = 0;
};

class NfgSolutionG   {
protected:
  const Nfg &nf;
  const NFSupport &sup;
  NfgShowInterface *parent;
  mutable gList<MixedSolution> solns;

public:
  NfgSolutionG(const Nfg &E,const NFSupport &S,NfgShowInterface *parent);
  virtual ~NfgSolutionG()   { }
  virtual gList<MixedSolution> Solve(void) const = 0;
  virtual bool SolveSetup(void) const = 0;
};

// Extensive Form Liap
class NfgLiapG : public NfgSolutionG   {
 public:
  NfgLiapG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgLiapG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgLemkeG : public NfgSolutionG   {
 public:
  NfgLemkeG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgLemkeG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgEnumPureG : public NfgSolutionG  {
 public:
  NfgEnumPureG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgEnumPureG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgQreAllG : public NfgSolutionG  {
 public:
  NfgQreAllG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgQreAllG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgQreG : public NfgSolutionG   {
 public:
  NfgQreG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgQreG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgSimpdivG : public NfgSolutionG   {
 public:
  NfgSimpdivG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgSimpdivG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgEnumG : public NfgSolutionG  {
 public:
  NfgEnumG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgEnumG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class guiPolEnumNfg : public NfgSolutionG {
public:
  guiPolEnumNfg(const Nfg &, const NFSupport &, NfgShowInterface *);
  virtual ~guiPolEnumNfg() { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class NfgZSumG : public NfgSolutionG   {
 public:
  NfgZSumG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgZSumG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

#endif  // NFGSOLNG_H

