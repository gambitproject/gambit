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
class EfgShowInterface  {
public:
  virtual BehavProfile<gNumber> CreateStartProfile(int how) = 0;
  virtual const gText &Filename(void) const = 0;
  virtual wxFrame *Frame(void) = 0;

  virtual void PickSolutions(const Efg &, gList<BehavSolution> &) = 0;
  virtual void SetPickSubgame(const Node *n) = 0;
};


class guiEfgSolutionG {
protected:
  const Efg &m_efg;
  const EFSupport &m_support;
  EfgShowInterface *m_parent;
  gList<BehavSolution> m_solutions;

public:
  guiEfgSolutionG(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgSolutionG()  { }
  virtual gList<BehavSolution> Solve(void) const = 0;
  virtual void SolveSetup(void) const = 0;
};


class guiEfgLiap : public guiEfgSolutionG {
public:
  guiEfgLiap(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgLiap()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgNfgLiap : public guiEfgSolutionG {
public:
  guiEfgNfgLiap(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgNfgLiap()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgSeqForm : public guiEfgSolutionG {
public:
  guiEfgSeqForm(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgSeqForm()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgLemke : public guiEfgSolutionG {
public:
  guiEfgLemke(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgLemke()   { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgNfgPureNash : public guiEfgSolutionG {
public:
  guiEfgNfgPureNash(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgNfgPureNash()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgPureNash : public guiEfgSolutionG {
public:
  guiEfgPureNash(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgPureNash() { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};


class guiEfgEnum : public guiEfgSolutionG {
public:
  guiEfgEnum(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgEnum() { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgZSum : public guiEfgSolutionG {
public:
  guiEfgZSum(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgZSum()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgCSum : public guiEfgSolutionG {
public:
  guiEfgCSum(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgCSum()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgSimpdiv : public guiEfgSolutionG {
public:
  guiEfgSimpdiv(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgSimpdiv()  { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgGobitAll : public guiEfgSolutionG {
public:
  guiEfgGobitAll(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgGobitAll()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};


class guiEfgNGobit : public guiEfgSolutionG  {
public:
  guiEfgNGobit(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgNGobit()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

class guiEfgEGobit : public guiEfgSolutionG {
public:
  guiEfgEGobit(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiEfgEGobit()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

#endif  // EFGSOLNG_H


