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
    m_parent->PickSolutions(p_efg, p_solutions);
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
