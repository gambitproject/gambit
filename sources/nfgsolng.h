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
  virtual bool SolveSetup(void) = 0;
};

class guinfgLiap : public NfgSolutionG   {
private:
  double m_tolND, m_tol1D;
  int m_maxitsND, m_maxits1D, m_nTries;
  int m_startOption;

public:
  guinfgLiap(const Nfg &, const NFSupport &, NfgShowInterface *);
  virtual ~guinfgLiap()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guinfgLcp : public NfgSolutionG   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgLcp(const Nfg &, const NFSupport &, NfgShowInterface *);
  virtual ~guinfgLcp()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guinfgEnumPure : public NfgSolutionG  {
private:
  int m_stopAfter;

public:
  guinfgEnumPure(const NFSupport &, NfgShowInterface *);
  virtual ~guinfgEnumPure()   { }

  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class NfgQreAllG : public NfgSolutionG  {
 public:
  NfgQreAllG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgQreAllG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class NfgQreG : public NfgSolutionG   {
private:
  int m_startOption;

public:
  NfgQreG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgQreG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class NfgSimpdivG : public NfgSolutionG   {
private:
  int m_stopAfter, m_nRestarts, m_leashLength;
  gPrecision m_precision;

public:
  NfgSimpdivG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgSimpdivG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class NfgEnumG : public NfgSolutionG  {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  NfgEnumG(const Nfg &E,const NFSupport &sup,NfgShowInterface *parent);
  virtual ~NfgEnumG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiPolEnumNfg : public NfgSolutionG {
private:
  int m_stopAfter;

public:
  guiPolEnumNfg(const Nfg &, const NFSupport &, NfgShowInterface *);
  virtual ~guiPolEnumNfg() { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class NfgZSumG : public NfgSolutionG   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  NfgZSumG(const Nfg &, const NFSupport &, NfgShowInterface *);
  virtual ~NfgZSumG()   { }
  virtual gList<MixedSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

#endif  // NFGSOLNG_H

