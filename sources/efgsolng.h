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

public:
  guiEfgSolution(const Efg &E, const EFSupport &S, EfgShowInterface *parent);
  virtual ~guiEfgSolution()  { }
  virtual gList<BehavSolution> Solve(void) const = 0;
  virtual void SolveSetup(void) const = 0;
};


// Extensive Form Liap
class EfgELiapG : public guiEfgSolution
{
public:
    EfgELiapG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgELiapG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Normal Form Liap
class EfgNLiapG : public guiEfgSolution
{
public:
    EfgNLiapG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgNLiapG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Seq Form
class EfgSeqFormG : public guiEfgSolution
{
public:
    EfgSeqFormG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgSeqFormG()   { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Lemke
class EfgLemkeG : public guiEfgSolution
{
public:
    EfgLemkeG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgLemkeG()   { } 
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Enum Pure
class EfgPureNashG : public guiEfgSolution
{
public:
    EfgPureNashG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgPureNashG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Efg Pure Nash
class EfgEPureNashG : public guiEfgSolution
{
public:
    EfgEPureNashG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgEPureNashG()   { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Enum Mixed
class EfgEnumG : public guiEfgSolution
{
public:
    EfgEnumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgEnumG()   { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// LP (ZSum)
class EfgZSumG : public guiEfgSolution
{
public:
    EfgZSumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgZSumG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// EfgCSum
class EfgCSumG : public guiEfgSolution
{
public:
    EfgCSumG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgCSumG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Simpdiv
class EfgSimpdivG : public guiEfgSolution
{
public:
    EfgSimpdivG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgSimpdivG()  { } 
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// Gobit All
class EfgGobitAllG : public guiEfgSolution
{
public:
    EfgGobitAllG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgGobitAllG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// NGobit
class EfgNGobitG : public guiEfgSolution
{
public:
    EfgNGobitG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgNGobitG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};


// EGobit  
class EfgEGobitG : public guiEfgSolution
{
public:
    EfgEGobitG(const Efg &E, const EFSupport &sup, EfgShowInterface *parent);
    virtual ~EfgEGobitG()  { }
    virtual gList<BehavSolution> Solve(void) const;
    virtual void SolveSetup(void) const;
};

#endif

