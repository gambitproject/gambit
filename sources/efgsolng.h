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
#include "paramsd.h"

// An interface class between EfgSolutionG (and related) and ExtensiveShow
class EfgShowInterface  {
 public:
  virtual ParameterSetList &Parameters(void) = 0;
  virtual BehavProfile<gNumber> CreateStartProfile(int how) = 0;
  virtual const gText &Filename(void) const = 0;
  virtual wxFrame *Frame(void) = 0;

  virtual void PickSolutions(const Efg &,gList<BehavSolution> &) = 0;
  virtual void SetPickSubgame(const Node *n) = 0;
};

class EfgSolutionG  {
 protected:
  const Efg &ef;
  const EFSupport &sup;
  EfgShowInterface *parent;
  gList<BehavSolution> solns;
 public:
  EfgSolutionG(const Efg &E,const EFSupport &S,EfgShowInterface *parent);
  virtual ~EfgSolutionG()  { }
  virtual gList<BehavSolution> Solve(void) const = 0;
  virtual void SolveSetup(void) const = 0;
};

// Extensive Form Liap
class EfgELiapG : public EfgSolutionG   {
 public:
  EfgELiapG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgELiapG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Normal Form Liap
class EfgNLiapG : public EfgSolutionG   {
 public:
  EfgNLiapG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgNLiapG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Seq Form
class EfgSeqFormG : public EfgSolutionG   {
 public:
  EfgSeqFormG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgSeqFormG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Lemke
class EfgLemkeG : public EfgSolutionG  {
 public:
  EfgLemkeG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgLemkeG()   { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Enum Pure
class EfgPureNashG : public EfgSolutionG   {
 public:
  EfgPureNashG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgPureNashG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Efg Pure Nash
class EfgEPureNashG : public EfgSolutionG   {
 public:
  EfgEPureNashG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgEPureNashG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Enum Mixed
class EfgEnumG : public EfgSolutionG  {
 public:
  EfgEnumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgEnumG()   { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// LP (ZSum)
class EfgZSumG : public EfgSolutionG  {
 public:
  EfgZSumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgZSumG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// EfgCSum
class EfgCSumG : public EfgSolutionG  {
 public:
  EfgCSumG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgCSumG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Simpdiv
class EfgSimpdivG : public EfgSolutionG  {
 public:
  EfgSimpdivG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgSimpdivG()  { } 
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// Gobit All
class EfgGobitAllG : public EfgSolutionG  {
 public:
  EfgGobitAllG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgGobitAllG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// NGobit
class EfgNGobitG : public EfgSolutionG   {
 public:
  EfgNGobitG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgNGobitG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

// EGobit  
class EfgEGobitG : public EfgSolutionG  {
 public:
  EfgEGobitG(const Efg &E,const EFSupport &sup,EfgShowInterface *parent);
  virtual ~EfgEGobitG()  { }
  virtual gList<BehavSolution> Solve(void) const;
  virtual void SolveSetup(void) const;
};

#endif

