//
// FILE: efgsolng.cc -- definition of the class dealing with the GUI part
//                      of the extensive form solutions.
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"
#include "guishare/wxstatus.h"
#include "gnullstatus.h"
#include "subsolve.h"
#include "gfunc.h"
#include "gambit.h"
#include "efgsolng.h"
#include "nfgconst.h"

//=========================================================================
//                     guiEfgSolution: Member functions
//=========================================================================

guiEfgSolution::guiEfgSolution(EfgShow *p_parent)
  : m_parent(p_parent), m_traceFile(&gnull), m_traceLevel(0)
{ }

//=========================================================================
//                    guiSubgameSolver: Class definition
//=========================================================================

class guiSubgameSolver {
protected:
  EfgShow *m_parent;
  bool m_pickSoln;
  gList<Node *> m_subgameRoots;
  bool m_eliminate, m_iterative, m_strong;
  
  void BaseSelectSolutions(int, const Efg::Game &, gList<BehavSolution> &);

public:
  guiSubgameSolver(EfgShow *, const Efg::Game &,
		 bool p_eliminate = false, bool p_iterative = false,
		 bool p_strong = false);
  virtual ~guiSubgameSolver() { } 
};

guiSubgameSolver::guiSubgameSolver(EfgShow *p_parent, const Efg::Game &p_efg,
			       bool p_eliminate, bool p_iterative,
			       bool p_strong)
  : m_parent(p_parent), m_pickSoln(false),
    m_eliminate(p_eliminate), m_iterative(p_iterative), m_strong(p_strong)
{
  MarkedSubgameRoots(p_efg, m_subgameRoots);
#ifdef NOT_PORTED_YET
  wxGetResource("Soln-Defaults", "Efg-Interactive-Solns",
		&m_pickSoln, gambitApp.ResourceFile());
#endif // NOT_PORTED_YET
}

//
// Pick solutions to go on with, if so requested
//
void guiSubgameSolver::BaseSelectSolutions(int p_subgame, const Efg::Game &p_efg, 
					 gList<BehavSolution> &p_solutions)
{
  if (!m_pickSoln || p_solutions.Length() == 0) 
    return;

  if (p_solutions.Length() > 0 && p_efg.NumPlayerInfosets() > 0)
    m_parent->PickSolutions(p_efg, m_subgameRoots[p_subgame], p_solutions);
}

#include "efdom.h"
#include "nfdom.h"

class guiSubgameViaEfg : public guiSubgameSolver {
protected:
  void BaseViewSubgame(int, const Efg::Game &, EFSupport &);

public:
  guiSubgameViaEfg(EfgShow *p_parent, const Efg::Game &p_efg,
		   bool p_eliminate, bool p_iterative,
		   bool p_strong)
    : guiSubgameSolver(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
  virtual ~guiSubgameViaEfg() { } 
};

void guiSubgameViaEfg::BaseViewSubgame(int, const Efg::Game &p_efg,
				       EFSupport &p_support)
{ 
  if (!m_eliminate)  return;

  gArray<int> players(p_efg.NumPlayers());
  for (int i = 1; i <= p_efg.NumPlayers(); i++) 
    players[i] = i;

  if (m_iterative) {
    EFSupport *oldSupport = new EFSupport(p_support), *newSupport;
    gNullStatus status;
    while ((newSupport = oldSupport->Undominated(m_strong, false,
						 players, gnull, status)) != 0) {
      delete oldSupport;
      oldSupport = newSupport;
    }

    p_support = *oldSupport;
    delete oldSupport;
  }
  else {
    EFSupport *newSupport;
    gNullStatus status;
    if ((newSupport = p_support.Undominated(m_strong, false,
					    players, gnull, status)) != 0) {
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
  guiSubgameViaNfg(EfgShow *p_parent, const Efg::Game &p_efg,
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
      gNullStatus status;
      while ((newSupport = oldSupport->MixedUndominated(m_strong,
							precRATIONAL,
							players, gnull, status)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      p_support = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(p_support), *newSupport;
      gNullStatus status;
      while ((newSupport = oldSupport->Undominated(m_strong,
						   players, gnull, status)) != 0) {
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
      gNullStatus status;
      if ((newSupport = p_support.MixedUndominated(m_strong,
						    precRATIONAL,
						    players, gnull, status)) != 0) {
	p_support = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      gNullStatus status;
      if ((newSupport = p_support.Undominated(m_strong,
					      players, gnull, status)) != 0) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg::Game &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  EFLiapBySubgameG(const Efg::Game &p_efg, const EFLiapParams &p_params,
		   const BehavProfile<gNumber> &p_start, 
		   bool p_eliminate, bool p_iterative, bool p_strong,
		   int p_max = 0,
		   EfgShow *p_parent = 0)
    : efgLiapSolve(p_efg, p_params, p_start, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLiapEfg::guiefgLiapEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgLiapEfg::guiefgLiapEfg(EfgShow *p_parent,
			     int p_stopAfter, int p_nTries)
  : guiEfgSolution(p_parent),
    m_nTries(p_nTries), m_stopAfter(p_stopAfter)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;

  m_accuracy = 1.0e-4;
  m_startOption = 0;
}

gList<BehavSolution> guiefgLiapEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LiapSolve Progress");
  // For the moment, we will always start at the centroid
  BehavProfile<gNumber> start(p_support);

  EFLiapParams params;
  params.stopAfter = m_stopAfter;
  params.SetAccuracy(m_accuracy);
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return EFLiapBySubgameG(p_support.GetGame(), params, start, m_eliminate,
			    m_eliminateAll, !m_eliminateWeak,
			    0, m_parent).Solve(start.Support(), status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapEfg::SolveSetup(void)
{ 
  dialogLiap dialog(m_parent, true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_accuracy = pow(10, -dialog.Accuracy());
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
			gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  NFLiapBySubgameG(const Efg::Game &p_efg, const NFLiapParams &p_params,
		   const BehavSolution &p_start,
		   bool p_eliminate, bool p_iterative, bool p_strong,
		   bool p_mixed, int p_max = 0, 
		   EfgShow *p_parent = 0)
    : efgLiapNfgSolve(p_efg, p_params,
		      BehavProfile<gNumber>(p_start), p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLiapNfg::guiefgLiapNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgLiapNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LiapSolve Progress");
  BehavProfile<gNumber> start(p_support);

  NFLiapParams params;
  params.stopAfter = m_stopAfter;
  params.SetAccuracy(m_accuracy);
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return NFLiapBySubgameG(p_support.GetGame()
			    , params, start, m_eliminate, m_eliminateAll,
			    !m_eliminateWeak, m_eliminateMixed,
			    0, m_parent).Solve(EFSupport(p_support.GetGame()),
							 status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapNfg::SolveSetup(void)
{
  dialogLiap dialog(m_parent, true, true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_accuracy = pow(10, -dialog.Accuracy());
    m_nTries = dialog.NumTries();
    m_startOption = dialog.StartOption();
    m_stopAfter = dialog.StopAfter();

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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg::Game &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }
  
public:
  SeqFormBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		    const SeqFormParams &p_params,
		    bool p_eliminate, bool p_iterative, bool p_strong,
		    int p_max = 0, EfgShow *p_parent = 0)
    : efgLcpSolve(p_support, p_params, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLcpEfg::guiefgLcpEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgLcpEfg::guiefgLcpEfg(EfgShow *p_parent,
			   int p_stopAfter, gPrecision p_precision)
  : guiEfgSolution(p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgLcpEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LcpSolve Progress");
  SeqFormParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    return SeqFormBySubgameG(p_support.GetGame(), p_support, params, m_eliminate,
			     m_eliminateAll, !m_eliminateWeak,
			     0, m_parent).Solve(p_support, status);
  }
  catch (gSignalBreak &) { }

  return gList<BehavSolution>();
}

bool guiefgLcpEfg::SolveSetup(void)
{ 
  dialogLcp dialog(m_parent, true);

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  LemkeBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		  const LemkeParams &p_params, bool p_eliminate,
		  bool p_iterative, bool p_strong, bool p_mixed, int p_max = 0,
		  EfgShow *p_parent = 0)
    : efgLcpNfgSolve(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLcpNfg::guiefgLcpNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgLcpNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LcpSolve Progress");

  LemkeParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    LemkeBySubgameG M(p_support.GetGame(),
		      p_support, params, m_eliminate, m_eliminateAll,
		      !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &)  {
    return gList<BehavSolution>();
  }
}

bool guiefgLcpNfg::SolveSetup(void)
{
  dialogLcp dialog(m_parent, true, true); 

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiSubgameEnumPureNfg(const Efg::Game &p_efg, const EFSupport &p_support,
			bool p_eliminate, bool p_iterative, bool p_strong,
			bool p_mixed, int p_stopAfter, gStatus &p_status,
			EfgShow *p_parent = 0)
    : efgEnumPureNfgSolve(p_support, p_stopAfter),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
  virtual ~guiSubgameEnumPureNfg() { }
};

guiefgEnumPureNfg::guiefgEnumPureNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgEnumPureNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "EnumPureSolve Progress");

  try {
    return guiSubgameEnumPureNfg(p_support.GetGame(),
				 p_support, m_eliminate, m_eliminateAll,
				 !m_eliminateWeak, m_eliminateMixed,
				 m_stopAfter,
				 status, m_parent).Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumPureNfg::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent, true, true); 

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg::Game &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  guiEnumPureEfgSubgame(const Efg::Game &p_efg, const EFSupport &p_support,
			int p_stopAfter, 
			bool p_eliminate, bool p_iterative, bool p_strong,
			gStatus &p_status, EfgShow *p_parent = 0)
    : efgEnumPure(p_stopAfter),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
  virtual ~guiEnumPureEfgSubgame() { }
};

guiefgEnumPureEfg::guiefgEnumPureEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgEnumPureEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "EnumPureSolve Progress");

  try {
    return guiEnumPureEfgSubgame(p_support.GetGame(), p_support, m_stopAfter,
				 m_eliminate, m_eliminateAll, !m_eliminateWeak,
				 status, m_parent).Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumPureEfg::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent, true);

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  EnumBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		 const EnumParams &p_params,
		 bool p_eliminate, bool p_iterative, bool p_strong,
		 bool p_mixed,
		 int p_max = 0, EfgShow *p_parent = 0)
    : EnumBySubgame(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgEnumMixedNfg::guiefgEnumMixedNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgEnumMixedNfg::guiefgEnumMixedNfg(EfgShow *p_parent,
				       int p_stopAfter, gPrecision p_precision,
				       bool p_eliminateWeak)
  : guiEfgSolution(p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution>
guiefgEnumMixedNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "EnumMixedSolve Progress");
  EnumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    EnumBySubgameG M(p_support.GetGame(), 
		     p_support, params, m_eliminate, m_eliminateAll,
		     !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumMixedNfg::SolveSetup(void)
{
  dialogEnumMixed dialog(m_parent, true); 

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_number, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  ZSumBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		 const ZSumParams &p_params, bool p_eliminate,
		 bool p_iterative, bool p_strong, bool p_mixed, int p_max = 0,
		 EfgShow *p_parent = 0)
    : efgLpNfgSolve(p_support, p_params, p_max), 
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgLpNfg::guiefgLpNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgLpNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    ZSumBySubgameG M(p_support.GetGame(), p_support, params, m_eliminate, m_eliminateAll,
		     !m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpNfg::SolveSetup(void)
{
  dialogLp dialog(m_parent, true, true); 

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_number, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewSubgame(int p_number, const Efg::Game &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_number, p_efg, p_support); }

public:
  EfgCSumBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		    const CSSeqFormParams &p_params, 
		    bool p_eliminate, bool p_iterative, bool p_strong,
		    int p_max = 0, EfgShow *p_parent = 0)
    : efgLpSolve(p_support, p_params, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLpEfg::guiefgLpEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgLpEfg::guiefgLpEfg(EfgShow *p_parent,
			 int p_stopAfter, gPrecision p_precision)
  : guiEfgSolution(p_parent), m_stopAfter(p_stopAfter),
    m_precision(p_precision)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgLpEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  CSSeqFormParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    EfgCSumBySubgameG M(p_support.GetGame(), p_support, params,
			m_eliminate, m_eliminateAll, !m_eliminateWeak,
			0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpEfg::SolveSetup(void)
{
  dialogLp dialog(m_parent, true);

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  SimpdivBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		    const SimpdivParams &p_params, bool p_eliminate,
		    bool p_iterative, bool p_strong, bool p_mixed, 
		    int p_max = 0, EfgShow *p_parent = 0)
    : efgSimpDivNfgSolve(p_support, p_params, p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgSimpdivNfg::guiefgSimpdivNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgSimpdivNfg::guiefgSimpdivNfg(EfgShow *p_parent,
				   int p_stopAfter, gPrecision p_precision,
				   bool p_eliminateWeak)
  : guiEfgSolution(p_parent), m_stopAfter(p_stopAfter),
    m_nRestarts(20), m_leashLength(0), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgSimpdivNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "SimpdivSolve Progress");

  SimpdivParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    SimpdivBySubgameG M(p_support.GetGame(), p_support, params, m_eliminate, m_eliminateAll,
			!m_eliminateWeak, m_eliminateMixed, 0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgSimpdivNfg::SolveSetup(void)
{
  dialogSimpdiv dialog(m_parent, true); 

  if (dialog.ShowModal() == wxID_OK) {
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
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  
public:
  guiPolEnumEfgByNfgSubgame(const Efg::Game &p_efg, const EFSupport &p_support,
			    const PolEnumParams &p_params,
			    bool p_eliminate, bool p_iterative,
			    bool p_strong, bool p_mixed, int p_max = 0,
			    EfgShow *p_parent = 0)
    : efgPolEnumNfgSolve(p_support, p_params, p_max),
      guiSubgameViaNfg(p_parent, p_efg,
		       p_eliminate, p_iterative, p_strong, p_mixed)
    { }
};

guiefgPolEnumNfg::guiefgPolEnumNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgPolEnumNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "PolEnumSolve Progress");
  status.SetProgress(0.0);
  PolEnumParams params;
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  try {
    guiPolEnumEfgByNfgSubgame M(p_support.GetGame(), p_support, params,
				m_eliminate, m_eliminateAll,
				!m_eliminateWeak, m_eliminateMixed,
				0, m_parent);
    return M.Solve(p_support, status);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgPolEnumNfg::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent, true, true); 

  if (dialog.ShowModal() == wxID_OK) {
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

#include "efgalleq.h"

class EfgPolEnumBySubgameG : public efgPolEnumSolve, public guiSubgameViaEfg {
protected:
  void SelectSolutions(int p_subgame, const Efg::Game &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg::Game &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }
  
public:
  EfgPolEnumBySubgameG(const Efg::Game &p_efg, const EFSupport &p_support,
		       const EfgPolEnumParams &p_params,
		       bool p_eliminate, bool p_iterative, bool p_strong,
		       int p_max = 0, EfgShow *p_parent = 0)
    : efgPolEnumSolve(p_support, p_params, p_max),
      guiSubgameViaEfg(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgPolEnumEfg::guiefgPolEnumEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgPolEnumEfg::guiefgPolEnumEfg(EfgShow *p_parent, int p_stopAfter)
  : guiEfgSolution(p_parent), m_stopAfter(p_stopAfter)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;
  m_markSubgames = false;
}

gList<BehavSolution> guiefgPolEnumEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "PolEnumSolve Progress");
  status.SetProgress(0.0);
  EfgPolEnumParams params;
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  gList<BehavSolution> solutions;

  try {
    return EfgPolEnumBySubgameG(p_support.GetGame(),
				p_support, params, m_eliminate,
				m_eliminateAll, !m_eliminateWeak,
				0, m_parent).Solve(p_support, status);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guiefgPolEnumEfg::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent, true); 

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
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

#include "nfgqre.h"

guiefgQreNfg::guiefgQreNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgQreNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "QreSolve Progress");
  BehavProfile<gNumber> startb(p_support);

  QreNfg qre;
  qre.SetMaxLambda(m_maxLam);

  Nfg *N = MakeReducedNfg(EFSupport(p_support.GetGame()));

  Correspondence<double, MixedSolution> qreCorresp;
  try {
    qre.Solve(*N, *m_pxiFile, status, qreCorresp);
  }
  catch (gSignalBreak &) { }

  gList<MixedSolution> nfg_solns;
  for (int i = 1; i <= qreCorresp.NumPoints(1); i++) {
    nfg_solns.Append(qreCorresp.GetPoint(i, 1));
  }


  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  gList<BehavSolution> solutions;

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    startb = BehavProfile<gNumber>(nfg_solns[i]);
    solutions.Append(BehavSolution(startb, algorithmEfg_QRE_NFG));
  }

  delete N;

  return solutions;
}

bool guiefgQreNfg::SolveSetup(void)
{
#ifdef UNUSED
  dialogQre dialog(m_parent, m_parent->Filename().c_str(), true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_accuracy = pow(10, -dialog.Accuracy());
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
#endif  // UNUSED
  return false;
}

//---------------------
// Qre on efg
//---------------------

#include "efgqre.h"

guiefgQreEfg::guiefgQreEfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

guiefgQreEfg::guiefgQreEfg(EfgShow *p_parent, int p_stopAfter)
  : guiEfgSolution(p_parent),
    m_stopAfter(p_stopAfter), m_pxiFile(0), m_runPxi(false)
{
  m_eliminate = false;
  m_eliminateAll = false;
  m_eliminateWeak = false;
  m_eliminateMixed = false;

  m_minLam = 0.01;
  m_maxLam = 30.0;
  m_delLam = 0.01;
  m_accuracy = 1.0e-4;
  m_powLam = 1;
}

gList<BehavSolution> guiefgQreEfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "QreSolve Progress");

  QreEfg qre;
  qre.SetMaxLambda(m_maxLam);

  gList<BehavSolution> solns;

  try {
    qre.Solve(p_support.GetGame(), *m_pxiFile, status, solns);
  }
  catch (gSignalBreak &) { }

  if (solns.Length() == 0)  return solns;

  if (!solns[1].IsSequential()) {
    wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		 "Returning last value.\n");
  }
  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  return solns;
}

bool guiefgQreEfg::SolveSetup(void)
{ 
#ifdef UNUSED
  dialogQre dialog(m_parent, m_parent->Filename().c_str()); 

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_accuracy = pow(10, -dialog.Accuracy());
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
#endif  // UNUSED
  return false;
}

//========================================================================
//                           QreGridSolve
//========================================================================

#include "dlqregrid.h"
#include "nfgqregrid.h"

guiefgQreAllNfg::guiefgQreAllNfg(EfgShow *p_parent)
  : guiEfgSolution(p_parent)
{ }

gList<BehavSolution> guiefgQreAllNfg::Solve(const EFSupport &p_support) const
{
  wxStatus status(m_parent, "QreGridSolve Progress");

  QreNfgGrid qre;
  qre.SetMinLambda(m_minLam);
  qre.SetMaxLambda(m_maxLam);
  qre.SetDelLambda(m_delLam);
  qre.SetTol1(m_tol1);
  qre.SetTol2(m_tol2);
  qre.SetDelP1(m_delp1);
  qre.SetDelP2(m_delp2);
  qre.SetPowLambda(m_powLam);

  Nfg *N = MakeReducedNfg(p_support);
  NFSupport S(*N);

  gList<MixedSolution> nfg_solns;
  try {
    qre.Solve(S, *m_pxiFile, status, nfg_solns);
  }
  catch (gSignalBreak &) { }

  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  gList<BehavSolution> solutions;

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    BehavProfile<gNumber> startb(nfg_solns[i]);
    solutions.Append(BehavSolution(startb, algorithmEfg_QRE_NFG));
  }

  delete N;

  return solutions;
}

bool guiefgQreAllNfg::SolveSetup(void)
{
  dialogQreGrid dialog(m_parent, m_parent->Filename().c_str()); 

  if (dialog.ShowModal() == wxID_OK) {
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
