//
// FILE: efgsolng.h -- declarations for the GUI part of the extensive form
//                     algorithm solutions. 
//
// $Id$
//

#ifndef EFGSOLNG_H
#define EFGSOLNG_H

#include "behavsol.h"
#include "efgshow.h"

class guiEfgSolution {
protected:
  EfgShow *m_parent;

  bool m_eliminate, m_eliminateAll, m_eliminateWeak, m_eliminateMixed;
  bool m_markSubgames;
  
  gOutput *m_traceFile;
  int m_traceLevel;

public:
  guiEfgSolution(EfgShow *parent);
  virtual ~guiEfgSolution()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const = 0;
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
  guiefgEnumPureEfg(EfgShow *);
  virtual ~guiefgEnumPureEfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgEnumPureNfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgEnumPureNfg(EfgShow *);
  virtual ~guiefgEnumPureNfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
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
  guiefgEnumMixedNfg(EfgShow *);
  guiefgEnumMixedNfg(EfgShow *, int p_stopAfter,
		     gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgEnumMixedNfg()   { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
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
  guiefgLpEfg(EfgShow *);
  guiefgLpEfg(EfgShow *, int p_stopAfter, gPrecision p_precision);
  virtual ~guiefgLpEfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgLpNfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLpNfg(EfgShow *);
  guiefgLpNfg(EfgShow *, int p_stopAfter,
	      gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guiefgLpNfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
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
  guiefgLcpEfg(EfgShow *);
  guiefgLcpEfg(EfgShow *, int p_stopAfter, gPrecision p_precision);
  virtual ~guiefgLcpEfg()   { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgLcpNfg : public guiEfgSolution {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guiefgLcpNfg(EfgShow *);
  virtual ~guiefgLcpNfg()   { } 

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

//--------
// Liap 
//--------

class guiefgLiapEfg : public guiEfgSolution {
private:
  double m_accuracy;
  int m_nTries, m_stopAfter, m_startOption;

public:
  guiefgLiapEfg(EfgShow *);
  guiefgLiapEfg(EfgShow *, int p_stopAfter, int p_nTries);
  virtual ~guiefgLiapEfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgLiapNfg : public guiEfgSolution {
private:
  double m_accuracy;
  int m_nTries, m_startOption, m_stopAfter;

public:
  guiefgLiapNfg(EfgShow *);
  virtual ~guiefgLiapNfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
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
  guiefgSimpdivNfg(EfgShow *);
  guiefgSimpdivNfg(EfgShow *,
		   int p_stopAfter, gPrecision p_precision, 
		   bool m_eliminateWeak);
  virtual ~guiefgSimpdivNfg()  { } 

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

//-----------
// PolEnum
//-----------

class guiefgPolEnumEfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgPolEnumEfg(EfgShow *);
  guiefgPolEnumEfg(EfgShow *, int p_stopAfter);
  virtual ~guiefgPolEnumEfg() { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgPolEnumNfg : public guiEfgSolution {
private:
  int m_stopAfter;

public:
  guiefgPolEnumNfg(EfgShow *);
  virtual ~guiefgPolEnumNfg() { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

//------
// Qre 
//------

class guiefgQreEfg : public guiEfgSolution {
private:
  int m_stopAfter, m_startOption, m_powLam;
  double m_minLam, m_maxLam, m_delLam, m_accuracy;
  gOutput *m_pxiFile;
  bool m_runPxi;
  gText m_pxiCommand, m_pxiFilename;

public:
  guiefgQreEfg(EfgShow *);
  guiefgQreEfg(EfgShow *, int p_stopAfter);
  virtual ~guiefgQreEfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

class guiefgQreNfg : public guiEfgSolution {
private:
  int m_stopAfter, m_startOption, m_powLam;
  double m_minLam, m_maxLam, m_delLam, m_accuracy;
  gOutput *m_pxiFile;
  bool m_runPxi;
  gText m_pxiCommand, m_pxiFilename;

public:
  guiefgQreNfg(EfgShow *);
  virtual ~guiefgQreNfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

//----------
// QreAll
//----------

class guiefgQreAllNfg : public guiEfgSolution {
private:
  int m_startOption, m_powLam;
  double m_minLam, m_maxLam, m_delLam, m_delp1, m_delp2, m_tol1, m_tol2;
  gOutput *m_pxiFile;
  bool m_runPxi, m_multiGrid;
  gText m_pxiCommand, m_pxiFilename;

public:
  guiefgQreAllNfg(EfgShow *);
  virtual ~guiefgQreAllNfg()  { }

  virtual gList<BehavSolution> Solve(const EFSupport &) const;
  virtual bool SolveSetup(void);
};

#endif  // EFGSOLNG_H

