//
// FILE: nfgsolng.h -- Interface to solution algorithms for normal form
//
// $Id$
//

#ifndef NFGSOLNG_H
#define NFGSOLNG_H

#include "mixedsol.h"
#include "nfgshow.h"

class guiNfgSolution   {
protected:
  NfgShow *m_parent;

  bool m_eliminate, m_eliminateAll, m_eliminateWeak, m_eliminateMixed;
  gOutput *m_traceFile;
  int m_traceLevel;

public:
  guiNfgSolution(NfgShow *);
  virtual ~guiNfgSolution()   { }

  NFSupport Eliminate(const NFSupport &);
  virtual gList<MixedSolution> Solve(const NFSupport &) = 0;
  virtual bool SolveSetup(void) = 0;
};

class guinfgEnumMixed : public guiNfgSolution  {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgEnumMixed(NfgShow *);
  guinfgEnumMixed(NfgShow *,
		  int p_stopAfter, gPrecision p_precision,
		  bool p_eliminateWeak);
  virtual ~guinfgEnumMixed()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgLp : public guiNfgSolution   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgLp(NfgShow *);
  guinfgLp(NfgShow *,
	   int p_stopAfter, gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guinfgLp()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgLcp : public guiNfgSolution   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgLcp(NfgShow *);
  guinfgLcp(NfgShow *,
	    int p_stopAfter, gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guinfgLcp()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgLiap : public guiNfgSolution   {
private:
  double m_accuracy;
  int m_nTries, m_stopAfter, m_startOption;

public:
  guinfgLiap(NfgShow *);
  guinfgLiap(NfgShow *,
	     int p_stopAfter, int p_nTries, bool p_eliminateWeak);
  virtual ~guinfgLiap()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgSimpdiv : public guiNfgSolution   {
private:
  int m_stopAfter, m_nRestarts, m_leashLength;
  gPrecision m_precision;

public:
  guinfgSimpdiv(NfgShow *parent);
  guinfgSimpdiv(NfgShow *,
		int p_stopAfter, gPrecision p_precision,
		bool p_eliminateWeak);
  virtual ~guinfgSimpdiv()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgPolEnum : public guiNfgSolution {
private:
  int m_stopAfter;

public:
  guinfgPolEnum(NfgShow *);
  guinfgPolEnum(NfgShow *,
		int p_stopAfter, bool p_eliminateWeak);
  virtual ~guinfgPolEnum() { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgQre : public guiNfgSolution   {
private:
  int m_stopAfter, m_startOption, m_powLam;
  double m_minLam, m_maxLam, m_delLam, m_accuracy;
  gOutput *m_pxiFile;
  bool m_runPxi;
  gText m_pxiCommand, m_pxiFilename;

public:
  guinfgQre(NfgShow *);
  virtual ~guinfgQre()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

class guinfgQreAll : public guiNfgSolution  {
private:
  int m_powLam;
  double m_minLam, m_maxLam, m_delLam, m_delp1, m_delp2, m_tol1, m_tol2;
  gOutput *m_pxiFile;
  bool m_runPxi, m_multiGrid;
  gText m_pxiCommand, m_pxiFilename;

public:
  guinfgQreAll(NfgShow *parent);
  virtual ~guinfgQreAll()   { }

  virtual gList<MixedSolution> Solve(const NFSupport &);
  virtual bool SolveSetup(void);
};

#endif  // NFGSOLNG_H

