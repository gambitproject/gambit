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

