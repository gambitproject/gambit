//
// FILE: efgsolng.h -- declarations for the GUI part of the extensive form
//                     algorithm solutions.  Each solution algorithm is
//                     contained in a separate class
//                     and the ExtensiveShow does not need to know any details
//
// $Id$
//

#ifndef EFGSOLNG_H
#define EFGSOLNG_H

#include "behavsol.h"

// An interface class between EfgSolutionG (and related) and ExtensiveShow
class EfgShowInterface {
public:
  virtual BehavProfile<gNumber> CreateStartProfile(int how) = 0;
  virtual const gText &Filename(void) const = 0;
  virtual wxFrame *Frame(void) = 0;

  virtual void PickSolutions(const Efg &, gList<BehavSolution> &) = 0;
  virtual void SetPickSubgame(const Node *n) = 0;
};


class guiEfgSolution {
protected:
  const Efg &ef;
  const EFSupport &sup;
  EfgShowInterface *parent;
  gList<BehavSolution> solns;

  mutable bool eliminate, all;
  mutable int domType, domMethod;
  mutable bool markSubgames;

public:
  guiEfgSolution(const Efg &E, const EFSupport &S, EfgShowInterface *parent);
  virtual ~guiEfgSolution()  { }
  virtual gList<BehavSolution> Solve(void) const = 0;
  virtual bool SolveSetup(void) const = 0;

  bool Eliminate(void) const { return eliminate; }
  bool EliminateAll(void) const { return all; }
  int DominanceType(void) const { return domType; }
  int DominanceMethod(void) const { return domMethod; }

  bool MarkSubgames(void) const { return markSubgames; }
};


//-------------------------------------------------------------------------
//                              LiapSolve
//-------------------------------------------------------------------------

class guiEfgSolveLiap : public guiEfgSolution {
public:
  guiEfgSolveLiap(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgSolveLiap()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class EfgNLiapG : public guiEfgSolution {
public:
  EfgNLiapG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgNLiapG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                              LcpSolve
//-------------------------------------------------------------------------

class EfgSeqFormG : public guiEfgSolution {
public:
  EfgSeqFormG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgSeqFormG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class EfgLemkeG : public guiEfgSolution {
public:
  EfgLemkeG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgLemkeG()   { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                            EnumPureSolve
//-------------------------------------------------------------------------

class EfgPureNashG : public guiEfgSolution {
public:
  EfgPureNashG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgPureNashG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class EfgEPureNashG : public guiEfgSolution {
public:
  EfgEPureNashG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgEPureNashG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                           EnumMixedSolve
//-------------------------------------------------------------------------

class EfgEnumG : public guiEfgSolution {
public:
  EfgEnumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgEnumG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                             LcpSolve
//-------------------------------------------------------------------------

class EfgZSumG : public guiEfgSolution {
public:
  EfgZSumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgZSumG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};


class EfgCSumG : public guiEfgSolution {
public:
  EfgCSumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgCSumG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                            SimpdivSolve
//-------------------------------------------------------------------------

class EfgSimpdivG : public guiEfgSolution {
public:
  EfgSimpdivG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgSimpdivG()  { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                            PolEnumSolve
//-------------------------------------------------------------------------

class guiPolEnumEfg : public guiEfgSolution {
public:
  guiPolEnumEfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiPolEnumEfg() { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class guiPolEnumEfgNfg : public guiEfgSolution {
public:
  guiPolEnumEfgNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiPolEnumEfgNfg() { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

//-------------------------------------------------------------------------
//                             QreSolve
//-------------------------------------------------------------------------

class EfgQreAllG : public guiEfgSolution {
public:
  EfgQreAllG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgQreAllG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class EfgNQreG : public guiEfgSolution {
public:
  EfgNQreG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgNQreG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

class EfgEQreG : public guiEfgSolution {
public:
  EfgEQreG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
  virtual ~EfgEQreG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void) const;
};

#endif  // EFGSOLNG_H

