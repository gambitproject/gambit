//
// FILE: nfgsolng.h -- Interface to solution algorithms for normal form
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

class guiNfgSolution   {
protected:
  const Nfg &m_nfg;
  NFSupport m_support;
  NfgShowInterface *m_parent;

  bool m_eliminate, m_eliminateAll, m_eliminateWeak, m_eliminateMixed;
  gOutput *m_traceFile;
  int m_traceLevel;

public:
  guiNfgSolution(const NFSupport &, NfgShowInterface *);
  virtual ~guiNfgSolution()   { }

  void Eliminate(void);
  virtual gList<MixedSolution> Solve(void) = 0;
  virtual bool SolveSetup(void) = 0;
};

class guinfgEnumPure : public guiNfgSolution  {
private:
  int m_stopAfter;

public:
  guinfgEnumPure(const NFSupport &, NfgShowInterface *);
  virtual ~guinfgEnumPure()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgEnumMixed : public guiNfgSolution  {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgEnumMixed(const NFSupport &, NfgShowInterface *);
  guinfgEnumMixed(const NFSupport &, NfgShowInterface *,
		  int p_stopAfter, gPrecision p_precision,
		  bool p_eliminateWeak);
  virtual ~guinfgEnumMixed()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgLp : public guiNfgSolution   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgLp(const NFSupport &, NfgShowInterface *);
  guinfgLp(const NFSupport &, NfgShowInterface *,
	   int p_stopAfter, gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guinfgLp()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgLcp : public guiNfgSolution   {
private:
  int m_stopAfter;
  gPrecision m_precision;

public:
  guinfgLcp(const NFSupport &, NfgShowInterface *);
  guinfgLcp(const NFSupport &, NfgShowInterface *,
	    int p_stopAfter, gPrecision p_precision, bool p_eliminateWeak);
  virtual ~guinfgLcp()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgLiap : public guiNfgSolution   {
private:
  double m_tolND, m_tol1D;
  int m_maxitsND, m_maxits1D, m_nTries;
  int m_stopAfter, m_startOption;

public:
  guinfgLiap(const NFSupport &, NfgShowInterface *);
  guinfgLiap(const NFSupport &, NfgShowInterface *,
	     int p_stopAfter, int p_nTries, bool p_eliminateWeak);
  virtual ~guinfgLiap()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgSimpdiv : public guiNfgSolution   {
private:
  int m_stopAfter, m_nRestarts, m_leashLength;
  gPrecision m_precision;

public:
  guinfgSimpdiv(const NFSupport &, NfgShowInterface *parent);
  guinfgSimpdiv(const NFSupport &, NfgShowInterface *,
		int p_stopAfter, gPrecision p_precision,
		bool p_eliminateWeak);
  virtual ~guinfgSimpdiv()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgPolEnum : public guiNfgSolution {
private:
  int m_stopAfter;

public:
  guinfgPolEnum(const NFSupport &, NfgShowInterface *);
  virtual ~guinfgPolEnum() { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgQre : public guiNfgSolution   {
private:
  int m_startOption;

public:
  guinfgQre(const NFSupport &, NfgShowInterface *);
  virtual ~guinfgQre()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

class guinfgQreAll : public guiNfgSolution  {
public:
  guinfgQreAll(const NFSupport &, NfgShowInterface *parent);
  virtual ~guinfgQreAll()   { }

  virtual gList<MixedSolution> Solve(void);
  virtual bool SolveSetup(void);
};

#endif  // NFGSOLNG_H

