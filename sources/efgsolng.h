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

  virtual void PickSolutions(const Efg &, Node *, gList<BehavSolution> &) = 0;
};


class guiEfgSolution {
protected:
  const Efg &m_efg;
  const EFSupport &m_support;
  EfgShowInterface *m_parent;

  bool m_eliminate, m_eliminateAll, m_eliminateWeak, m_eliminateMixed;
  bool m_markSubgames;
  
  gOutput *m_traceFile;
  int m_traceLevel;

public:
  guiEfgSolution(const EFSupport &S, EfgShowInterface *parent);
  virtual ~guiEfgSolution()  { }

  virtual gList<BehavSolution> Solve(void) const = 0;
  virtual bool SolveSetup(void) = 0;

  bool MarkSubgames(void) const { return m_markSubgames; }
};

//------------
// EnumPure
//------------

class guiefgEnumPureEfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgEnumPureEfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgEnumPureEfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgEnumPureNfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgEnumPureNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgEnumPureNfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//-------------
// EnumMixed
//-------------

class guiefgEnumMixedNfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgEnumMixedNfg(const EFSupport &, EfgShowInterface *);
  guiefgEnumMixedNfg(const EFSupport &, EfgShowInterface *, int p_stopAfter,
		     gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgEnumMixedNfg()   { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//------
// Lp
//------

class guiefgLpEfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLpEfg(const EFSupport &, EfgShowInterface *);
  guiefgLpEfg(const EFSupport &, EfgShowInterface *, int p_stopAfter,
	      gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgLpEfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgLpNfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLpNfg(const EFSupport &, EfgShowInterface *);
  guiefgLpNfg(const EFSupport &, EfgShowInterface *, int p_stopAfter,
	      gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgLpNfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//-------
// Lcp
//-------

class guiefgLcpEfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLcpEfg(const EFSupport &, EfgShowInterface *);
  guiefgLcpEfg(const EFSupport &, EfgShowInterface *, int p_stopAfter,
	       gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgLcpEfg()   { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgLcpNfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLcpNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgLcpNfg()   { } 

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//--------
// Liap 
//--------

class guiefgLiapEfg : public guiEfgSolution {
private:
  double m_tolND, m_tol1D;
  int m_maxitsND, m_maxits1D, m_nTries;
  int m_stopAfter, m_startOption;

public:
  guiefgLiapEfg(const EFSupport &, EfgShowInterface *);
  guiefgLiapEfg(const EFSupport &, EfgShowInterface *,
		int p_stopAfter, bool m_eliminateWeak);
  virtual ~guiefgLiapEfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgLiapNfg : public guiEfgSolution {
private:
  double m_tolND, m_tol1D;
  int m_maxitsND, m_maxits1D, m_nTries;
  int m_startOption;

public:
  guiefgLiapNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgLiapNfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//-----------
// Simpdiv
//-----------

class guiefgSimpdivNfg : public guiEfgSolution {
private:
  int m_stopAfter, m_nRestarts, m_leashLength;
  gPrecision m_precision;

public:
  guiefgSimpdivNfg(const EFSupport &, EfgShowInterface *);
  guiefgSimpdivNfg(const EFSupport &, EfgShowInterface *,
		   int p_stopAfter, gPrecision p_precision, 
		   bool m_eliminateWeak);
  virtual ~guiefgSimpdivNfg()  { } 

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//-----------
// PolEnum
//-----------

class guiefgPolEnumEfg : public guiEfgSolution {
public:
  guiefgPolEnumEfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgPolEnumEfg() { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgPolEnumNfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgPolEnumNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgPolEnumNfg() { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//------
// Qre 
//------

class guiefgQreEfg : public guiEfgSolution {
private:
  int m_stopAfter, m_startOption;

public:
  guiefgQreEfg(const EFSupport &, EfgShowInterface *);
  guiefgQreEfg(const EFSupport &, EfgShowInterface *, int p_stopAfter,
	       bool p_eliminateWeak);
  virtual ~guiefgQreEfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

class guiefgQreNfg : public guiEfgSolution {
private:
  int m_startOption;

public:
  guiefgQreNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgQreNfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

//----------
// QreAll
//----------

class guiefgQreAllNfg : public guiEfgSolution {
public:
  guiefgQreAllNfg(const EFSupport &, EfgShowInterface *);
  virtual ~guiefgQreAllNfg()  { }

  virtual gList<BehavSolution> Solve(void) const;
  virtual bool SolveSetup(void);
};

#endif  // EFGSOLNG_H

