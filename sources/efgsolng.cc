//
// FILE: efgsolng.cc -- definition of the class dealing with the GUI part
//                      of the extensive form solutions.
//
// $Id$
//

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "wxstatus.h"
#include "subsolve.h"
#include "gfunc.h"
#include "gambit.h"
#include "efgsolng.h"
#include "nfgconst.h"

//=========================================================================
//                     guiEfgSolution: Member functions
//=========================================================================

guiEfgSolution::guiEfgSolution(const EFSupport &p_support,
			       EfgShowInterface *p_parent)
  : m_efg(p_support.Game()), m_support(p_support), m_parent(p_parent),
    m_traceFile(&gnull), m_traceLevel(0)
{ }

//=========================================================================
//                    guiSubgameSolver: Class definition
//=========================================================================

class guiSubgameSolver {
protected:
  EfgShowInterface *m_parent;
  Bool m_pickSoln;
  gList<Node *> m_subgameRoots;
  bool m_eliminate, m_iterative, m_strong;
  
  void BaseSelectSolutions(int, const Efg &, gList<BehavSolution> &);

public:
  guiSubgameSolver(EfgShowInterface *, const Efg &,
		 bool p_eliminate = false, bool p_iterative = false,
		 bool p_strong = false);
  virtual ~guiSubgameSolver() { } 
};

guiSubgameSolver::guiSubgameSolver(EfgShowInterface *p_parent, const Efg &p_efg,
			       bool p_eliminate, bool p_iterative,
			       bool p_strong)
  : m_parent(p_parent), m_pickSoln(false),
    m_eliminate(p_eliminate), m_iterative(p_iterative), m_strong(p_strong)
{
  MarkedSubgameRoots(p_efg, m_subgameRoots);
  wxGetResource("Soln-Defaults", "Efg-Interactive-Solns",
		&m_pickSoln, gambitApp.ResourceFile());
}

//
// Pick solutions to go on with, if so requested
//
void guiSubgameSolver::BaseSelectSolutions(int p_subgame, const Efg &p_efg, 
					 gList<BehavSolution> &p_solutions)
{
  if (!m_pickSoln || p_solutions.Length() == 0) 
    return;

  if (p_solutions.Length() > 0 && p_efg.TotalNumInfosets() > 0)
    m_parent->PickSolutions(p_efg, m_subgameRoots[p_subgame], p_solutions);
}

#include "efdom.h"
#include "nfdom.h"

class guiSubgameViaEfg : public guiSubgameSolver {
protected:
  void BaseViewSubgame(int, const Efg &, EFSupport &);

public:
  guiSubgameViaEfg(EfgShowInterface *p_parent, const Efg &p_efg,
		   bool p_eliminate, bool p_iterative,
		   bool p_strong)
    : guiSubgameSolver(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
  virtual ~guiSubgameViaEfg() { } 
};

void guiSubgameViaEfg::BaseViewSubgame(int, const Efg &p_efg,
				       EFSupport &p_support)
{ 
  if (!m_eliminate)  return;

  gArray<int> players(p_efg.NumPlayers());
  for (int i = 1; i <= p_efg.NumPlayers(); i++) 
    players[i] = i;

  if (m_iterative) {
    EFSupport *oldSupport = new EFSupport(p_support), *newSupport;
    while ((newSupport = oldSupport->Undominated(m_strong, false,
						 players, gnull, gstatus)) != 0) {
      delete oldSupport;
      oldSupport = newSupport;
    }

    p_support = *oldSupport;
    delete oldSupport;
  }
  else {
    EFSupport *newSupport;
    if ((newSupport = p_support.Undominated(m_strong, false,
					    players, gnull, gstatus)) != 0) {
      p_support = *newSupport;
      delete newSupport;
    }
  }
}

class guiSubgameViaNfg : public guiSubgameSolver {
protected:
  bool m_mixed;

  void BaseViewNormal(const Nfg &, NFSupport &);

public:
  guiSubgameViaNfg(EfgShowInterface *p_parent, const Efg &p_efg,
		   bool p_eliminate, bool p_iterative,
		   bool p_strong, bool p_mixed)
    : guiSubgameSolver(p_parent, p_efg, p_eliminate, p_iterative, p_strong),
      m_mixed(p_mixed)
    { }
  virtual ~guiSubgameViaNfg() { } 
};

#include "nfstrat.h"

void guiSubgameViaNfg::BaseViewNormal(const Nfg &p_nfg, NFSupport &p_support)
{
  if (!m_eliminate)  return;

  gArray<int> players(p_nfg.NumPlayers());
  for (int i = 1; i <= p_nfg.NumPlayers(); i++) 
    players[i] = i;


  if (m_iterative) {
    if (m_mixed) {
      NFSupport *oldSupport = new NFSupport(p_support), *newSupport;
      while ((newSupport = oldSupport->MixedUndominated(m_strong,
							precRATIONAL,
							players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      p_support = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(p_support), *newSupport;
      while ((newSupport = oldSupport->Undominated(m_strong,
						   players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      p_support = *oldSupport;
      delete oldSupport;
    }
  }
  else {
    if (m_mixed) {
      NFSupport *newSupport;
      if ((newSupport = p_support.MixedUndominated(m_strong,
						    precRATIONAL,
						    players, gnull, gstatus)) != 0) {
	p_support = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      if ((newSupport = p_support.Undominated(m_strong,
					      players, gnull, gstatus)) != 0) {
	p_support = *newSupport;
	delete newSupport;
      }
    }
  }
}

//=========================================================================
//                     Algorithm-specific classes
//=========================================================================

//========================================================================
//                              LiapSolve
//========================================================================

#include "dlliap.h"

//---------------------
// Liapunov on efg
//---------------------

#include "eliap.h"

class EFLiapBySubgameG : public efgLiapSolve, public guiSubgameViaEfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  EFLiapBySubgameG(const Efg &p_efg, const EFLiapParams &p_params,
		   const BehavSolution &p_start, 
		   bool p_eliminate, bool p_iterative, bool p_strong,
		   int p_max = 0,
		   EfgShowInterface *p_parent = 0)
    : efgLiapSolve(p_efg, p_params, 
		   BehavProfile<gNumber>(p_start), p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLiapEfg::guiefgLiapEfg(const EFSupport &p_support,
			     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgLiapEfg::guiefgLiapEfg(const EFSupport &p_support,
			     EfgShowInterface *p_parent,
			     int p_stopAfter, int p_nTries)
  : guiEfgSolution(p_support, p_parent),
    m_nTries(p_nTries), m_stopAfter(p_stopAfter)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;

  m_maxits1D = 100;
  m_maxitsND = 20;
  m_tol1D = 2.0e-10;
  m_tolND = 1.0e-10;
  m_startOption = 0;
}

gList<BehavSolution> guiefgLiapEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LiapSolve Progress");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(m_startOption);

  EFLiapParams params(status);
  params.stopAfter = m_stopAfter;
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return EFLiapBySubgameG(m_efg, params, start, m_eliminate,
			    m_eliminateAll, !m_eliminateWeak,
			    0, m_parent).Solve(EFSupport(m_efg));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapEfg::SolveSetup(void)
{ 
  dialogLiap dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_tol1D = pow(10, -dialog.Tol1D());
    m_tolND = pow(10, -dialog.TolND());
    m_maxits1D = dialog.Maxits1D();
    m_maxitsND = dialog.MaxitsND();
    m_nTries = dialog.NumTries();
    m_startOption = dialog.StartOption();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//---------------------
// Liapunov on nfg
//---------------------

#include "liapsub.h"

class NFLiapBySubgameG : public efgLiapNfgSolve, public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
			gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  NFLiapBySubgameG(const Efg &p_efg, const NFLiapParams &p_params,
		   const BehavSolution &p_start,
		   bool p_eliminate, bool p_iterative, bool p_strong,
		   bool p_mixed, int p_max = 0, 
		   EfgShowInterface *p_parent = 0)
    : efgLiapNfgSolve(p_efg, p_params,
		      BehavProfile<gNumber>(p_start), p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLiapNfg::guiefgLiapNfg(const EFSupport &p_support, 
			     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLiapNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LiapSolve Progress");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(m_startOption);

  NFLiapParams params(status);
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return NFLiapBySubgameG(m_efg, params, start, m_eliminate, m_eliminateAll,
			    !m_eliminateWeak, m_eliminateMixed,
			    0, m_parent).Solve(EFSupport(m_efg));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapNfg::SolveSetup(void)
{
  dialogLiap dialog(m_parent->Frame(), true, true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_tol1D = dialog.Tol1D();
    m_tolND = dialog.TolND();
    m_maxits1D = dialog.Maxits1D();
    m_maxitsND = dialog.MaxitsND();
    m_nTries = dialog.NumTries();
    m_startOption = dialog.StartOption();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//========================================================================
//                               LcpSolve
//========================================================================

#include "dllcp.h"

//---------------------
// LCP on efg
//---------------------

#include "seqform.h"

class SeqFormBySubgameG : public efgLcpSolve, public guiSubgameViaEfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }
  
public:
  SeqFormBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SeqFormParams &p_params,
		    bool p_eliminate, bool p_iterative, bool p_strong,
		    int p_max = 0, EfgShowInterface *p_parent = 0)
    : efgLcpSolve(p_support, p_params, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLcpEfg::guiefgLcpEfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgLcpEfg::guiefgLcpEfg(const EFSupport &p_support,
			   EfgShowInterface *p_parent,
			   int p_stopAfter, gPrecision p_precision)
  : guiEfgSolution(p_support, p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgLcpEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LcpSolve Progress");
  SeqFormParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return SeqFormBySubgameG(m_efg, m_support, params, m_eliminate,
			     m_eliminateAll, !m_eliminateWeak,
			     0, m_parent).Solve(m_support);
  }
  catch (gSignalBreak &) { }

  return gList<BehavSolution>();
}

bool guiefgLcpEfg::SolveSetup(void)
{ 
  dialogLcp dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}


//---------------------
// LCP on nfg
//---------------------

#include "lemkesub.h"

class LemkeBySubgameG : public efgLcpNfgSolve, public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  LemkeBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		  const LemkeParams &p_params, bool p_eliminate,
		  bool p_iterative, bool p_strong, bool p_mixed, int p_max = 0,
		  EfgShowInterface *p_parent = 0)
    : efgLcpNfgSolve(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLcpNfg::guiefgLcpNfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLcpNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LcpSolve Progress");

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    LemkeBySubgameG M(m_efg, m_support, params, m_eliminate, m_eliminateAll,
		      !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &)  {
    return gList<BehavSolution>();
  }
}

bool guiefgLcpNfg::SolveSetup(void)
{
  dialogLcp dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}


//========================================================================
//                            EnumPureSolve
//========================================================================

#include "psnesub.h"
#include "dlenumpure.h"

//---------------------
// EnumPure on nfg
//---------------------

class guiSubgameEnumPureNfg : public efgEnumPureNfgSolve,
			      public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiSubgameEnumPureNfg(const Efg &p_efg, const EFSupport &p_support,
			bool p_eliminate, bool p_iterative, bool p_strong,
			bool p_mixed, int p_stopAfter, gStatus &p_status,
			EfgShowInterface *p_parent = 0)
    : efgEnumPureNfgSolve(p_support, p_stopAfter, p_status),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
  virtual ~guiSubgameEnumPureNfg() { }
};

guiefgEnumPureNfg::guiefgEnumPureNfg(const EFSupport &p_support, 
				     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgEnumPureNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "EnumPureSolve Progress");

  try {
    return guiSubgameEnumPureNfg(m_efg, m_support, m_eliminate, m_eliminateAll,
				 !m_eliminateWeak, m_eliminateMixed,
				 m_stopAfter,
				 status, m_parent).Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumPureNfg::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();
    m_stopAfter = dialog.StopAfter();
    return true;
  }
  else
    return false;
}


//---------------------
// EnumPure on efg
//---------------------

#include "efgpure.h"

class guiEnumPureEfgSubgame : public efgEnumPure, public guiSubgameViaEfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  guiEnumPureEfgSubgame(const Efg &p_efg, const EFSupport &p_support,
			int p_stopAfter, 
			bool p_eliminate, bool p_iterative, bool p_strong,
			gStatus &p_status, EfgShowInterface *p_parent = 0)
    : efgEnumPure(p_stopAfter, p_status),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
  virtual ~guiEnumPureEfgSubgame() { }
};

guiefgEnumPureEfg::guiefgEnumPureEfg(const EFSupport &p_support, 
				     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgEnumPureEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "EnumPureSolve Progress");

  try {
    return guiEnumPureEfgSubgame(m_efg, m_support, m_stopAfter,
				 m_eliminate, m_eliminateAll, !m_eliminateWeak,
				 status, m_parent).Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumPureEfg::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    return true;
  }
  else
    return false;
}

//========================================================================
//                          EnumMixedSolve
//========================================================================

#include "dlenummixed.h"

//---------------------
// EnumMixed on nfg
//---------------------

#include "enumsub.h"

class EnumBySubgameG : public EnumBySubgame, public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  EnumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		 const EnumParams &p_params,
		 bool p_eliminate, bool p_iterative, bool p_strong,
		 bool p_mixed,
		 int p_max = 0, EfgShowInterface *p_parent = 0)
    : EnumBySubgame(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgEnumMixedNfg::guiefgEnumMixedNfg(const EFSupport &p_support,
				       EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgEnumMixedNfg::guiefgEnumMixedNfg(const EFSupport &p_support,
				       EfgShowInterface *p_parent,
				       int p_stopAfter, gPrecision p_precision,
				       bool p_eliminateWeak)
  : guiEfgSolution(p_support, p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgEnumMixedNfg::Solve(void) const
{
  wxEnumStatus status(m_parent->Frame());
  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    EnumBySubgameG M(m_efg, m_support, params, m_eliminate, m_eliminateAll,
		     !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumMixedNfg::SolveSetup(void)
{
  dialogEnumMixed dialog(m_parent->Frame(), true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}


//========================================================================
//                                LpSolve
//========================================================================

#include "dllp.h"

//---------------------
// Lp on nfg
//---------------------

#include "csumsub.h"

class ZSumBySubgameG : public efgLpNfgSolve, public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  ZSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		 const ZSumParams &p_params, bool p_eliminate,
		 bool p_iterative, bool p_strong, bool p_mixed, int p_max = 0,
		 EfgShowInterface *p_parent = 0)
    : efgLpNfgSolve(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLpNfg::guiefgLpNfg(const EFSupport &p_support,
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLpNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    ZSumBySubgameG M(m_efg, m_support, params, m_eliminate, m_eliminateAll,
		     !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpNfg::SolveSetup(void)
{
  dialogLp dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = 1;
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//---------------------
// Lp on efg
//---------------------

#include "efgcsum.h"

class EfgCSumBySubgameG : public efgLpSolve, public guiSubgameViaEfg {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewSubgame(int p_number, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_number, p_efg, p_support); }

public:
  EfgCSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const CSSeqFormParams &p_params, 
		    bool p_eliminate, bool p_iterative, bool p_strong,
		    int p_max = 0, EfgShowInterface *p_parent = 0)
    : efgLpSolve(p_support, p_params, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLpEfg::guiefgLpEfg(const EFSupport &p_support,
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgLpEfg::guiefgLpEfg(const EFSupport &p_support,
			 EfgShowInterface *p_parent,
			 int p_stopAfter, gPrecision p_precision)
  : guiEfgSolution(p_support, p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgLpEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  CSSeqFormParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    EfgCSumBySubgameG M(m_efg, m_support, params,
			m_eliminate, m_eliminateAll, !m_eliminateWeak,
			0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpEfg::SolveSetup(void)
{
  dialogLp dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = 1;
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//========================================================================
//                           SimpdivSolve
//========================================================================

#include "dlsimpdiv.h"

//---------------------
// Simpdiv on nfg
//---------------------

#include "simpsub.h"

class SimpdivBySubgameG : public efgSimpDivNfgSolve, public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  SimpdivBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SimpdivParams &p_params, bool p_eliminate,
		    bool p_iterative, bool p_strong, bool p_mixed, 
		    int p_max = 0, EfgShowInterface *p_parent = 0)
    : efgSimpDivNfgSolve(p_support, p_params, p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgSimpdivNfg::guiefgSimpdivNfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgSimpdivNfg::guiefgSimpdivNfg(const EFSupport &p_support,
				   EfgShowInterface *p_parent,
				   int p_stopAfter, gPrecision p_precision,
				   bool p_eliminateWeak)
  : guiEfgSolution(p_support, p_parent), m_stopAfter(p_stopAfter),
    m_nRestarts(20), m_leashLength(0), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgSimpdivNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "SimpdivSolve Progress");

  SimpdivParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    SimpdivBySubgameG M(m_efg, m_support, params, m_eliminate, m_eliminateAll,
			!m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgSimpdivNfg::SolveSetup(void)
{
  dialogSimpdiv dialog(m_parent->Frame(), true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    m_nRestarts = dialog.NumRestarts();
    m_leashLength = dialog.LeashLength();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();

    return true;
  }
  else
    return false;
}

//========================================================================
//                           PolEnumSolve
//========================================================================

#include "dlpolenum.h"

//------------------
// PolEnum on nfg
//------------------

#include "polensub.h"

class guiPolEnumEfgByNfgSubgame : public efgPolEnumNfgSolve,
				  public guiSubgameViaNfg {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  
public:
  guiPolEnumEfgByNfgSubgame(const Efg &p_efg, const EFSupport &p_support,
			    const PolEnumParams &p_params,
			    bool p_eliminate, bool p_iterative,
			    bool p_strong, bool p_mixed, int p_max = 0,
			    EfgShowInterface *p_parent = 0)
    : efgPolEnumNfgSolve(p_support, p_params, p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgPolEnumNfg::guiefgPolEnumNfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgPolEnumNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "PolEnumSolve Progress");
  status.SetProgress(0.0);
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    guiPolEnumEfgByNfgSubgame M(m_efg, m_support, params,
				m_eliminate, m_eliminateAll,
				!m_eliminateWeak, m_eliminateMixed,
				0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgPolEnumNfg::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//------------------
// PolEnum on efg
//------------------

guiefgPolEnumEfg::guiefgPolEnumEfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgPolEnumEfg::Solve(void) const
{
  return gList<BehavSolution>();
}

bool guiefgPolEnumEfg::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent->Frame(), true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();
    return true;
  }
  else
    return false;
}

//========================================================================
//                               QreSolve
//========================================================================

#include "dlqre.h"

//---------------------
// Qre on nfg
//---------------------

#include "ngobit.h"

guiefgQreNfg::guiefgQreNfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgQreNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");
  BehavProfile<gNumber> startb = m_parent->CreateStartProfile(m_startOption);

  NFQreParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.powLam = m_powLam;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  Nfg *N = MakeReducedNfg(EFSupport(m_efg));

  MixedProfile<gNumber> startm(*N);

  BehavToMixed(m_efg, startb, *N, startm);
  
  long nevals, nits;
  gList<MixedSolution> nfg_solns;

  try {
    Qre(*N, params, startm, nfg_solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (m_runPxi) {
    if (!wxExecute(m_pxiCommand + " " + m_pxiFilename)) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  gList<BehavSolution> solutions;

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    MixedToBehav(*N, nfg_solns[i], m_efg, startb);
    solutions.Append(BehavSolution(startb, algorithmEfg_QRE_NFG));
  }

  delete N;

  return solutions;
}

bool guiefgQreNfg::SolveSetup(void)
{
  dialogQre dialog(m_parent->Frame(), m_parent->Filename(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_tol1D = pow(10, -dialog.Tol1D());
    m_tolND = pow(10, -dialog.TolND());
    m_maxits1D = dialog.Maxits1D();
    m_maxitsND = dialog.MaxitsND();
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//---------------------
// Qre on efg
//---------------------

#include "egobit.h"

guiefgQreEfg::guiefgQreEfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

guiefgQreEfg::guiefgQreEfg(const EFSupport &p_support,
			   EfgShowInterface *p_parent, int p_stopAfter)
  : guiEfgSolution(p_support, p_parent),
    m_stopAfter(p_stopAfter), m_pxiFile(0), m_runPxi(false)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;

  m_minLam = 0.01;
  m_maxLam = 30.0;
  m_delLam = 0.01;
  m_tol1D = 2.0e-10;
  m_tolND = 1.0e-10;
  m_maxits1D = 100;
  m_maxitsND = 20;
  m_powLam = 1;
}

gList<BehavSolution> guiefgQreEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(m_startOption);
  EFQreParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.powLam = m_powLam;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  long nevals, nits;
  gList<BehavSolution> solns;

  try {
    Qre(m_efg, params, start, solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (solns.Length() == 0)  return solns;

  if (!solns[1].IsSequential()) {
    wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		 "Returning last value.\n");
  }
  if (m_runPxi) {
    if (!wxExecute(m_pxiCommand + " " + m_pxiFilename)) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  return solns;
}

bool guiefgQreEfg::SolveSetup(void)
{ 
  dialogQre dialog(m_parent->Frame(), m_parent->Filename()); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_tol1D = dialog.Tol1D();
    m_tolND = dialog.TolND();
    m_maxits1D = dialog.Maxits1D();
    m_maxitsND = dialog.MaxitsND();
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//========================================================================
//                           QreGridSolve
//========================================================================

#include "dlqregrid.h"
#include "grid.h"

guiefgQreAllNfg::guiefgQreAllNfg(const EFSupport &p_support, 
				 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgQreAllNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "QreGridSolve Progress");

  GridParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.tol1 = m_tol1;
  params.tol2 = m_tol2;
  params.delp1 = m_delp1;
  params.delp2 = m_delp2;
  params.powLam = m_powLam;
  params.multi_grid = m_multiGrid;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  Nfg *N = MakeReducedNfg(m_support);
  NFSupport S(*N);

  gList<MixedSolution> nfg_solns;
  try {
    GridSolve(S, params, nfg_solns);
  }
  catch (gSignalBreak &) { }

  if (m_runPxi) {
    if (!wxExecute(m_pxiCommand + " " + m_pxiFilename)) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  gList<BehavSolution> solutions;

  BehavProfile<gNumber> startb(m_support);
  for (int i = 1; i <= nfg_solns.Length(); i++) {
    MixedToBehav(*N, nfg_solns[i], m_efg, startb);
    solutions.Append(BehavSolution(startb, algorithmEfg_QRE_NFG));
  }

  delete N;

  return solutions;
}

bool guiefgQreAllNfg::SolveSetup(void)
{
  dialogQreGrid dialog(m_parent->Frame(), m_parent->Filename()); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_tol1 = dialog.Tol1();
    m_tol2 = dialog.Tol2();
    m_delp1 = dialog.DelP1();
    m_delp2 = dialog.DelP2();
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;
    m_multiGrid = dialog.MultiGrid();

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}
