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
#include "efgsolng.h"
#include "nfgconst.h"

// sections in the defaults file(s)
#define SOLN_SECT "Soln-Defaults"

//=========================================================================
//                     guiEfgSolution: Member functions
//=========================================================================

guiEfgSolution::guiEfgSolution(const EFSupport &p_support,
			       EfgShowInterface *p_parent)
  : m_efg(p_support.Game()), m_support(p_support), m_parent(p_parent)
{ }

//=========================================================================
//                    BaseBySubgameG: Class definition
//=========================================================================

class BaseBySubgameG {
protected:
  EfgShowInterface *m_parent;
  Bool m_pickSoln;
  gList<Node *> m_subgameRoots;
  bool m_eliminate, m_iterative, m_strong;
  
  void BaseSelectSolutions(int, const Efg &, gList<BehavSolution> &);
  void BaseViewNormal(const Nfg &, NFSupport &);
  void BaseViewSubgame(int, const Efg &, EFSupport &);

public:
  BaseBySubgameG(EfgShowInterface *, const Efg &,
		 bool p_eliminate = false, bool p_iterative = false,
		 bool p_strong = false);
  virtual ~BaseBySubgameG() { } 
};

BaseBySubgameG::BaseBySubgameG(EfgShowInterface *p_parent, const Efg &p_efg,
			       bool p_eliminate, bool p_iterative,
			       bool p_strong)
  : m_parent(p_parent), m_eliminate(p_eliminate), m_iterative(p_iterative),
    m_strong(p_strong)
{
  MarkedSubgameRoots(p_efg, m_subgameRoots);
  wxGetResource(SOLN_SECT, "Efg-Interactive-Solns", &m_pickSoln, "gambit.ini");
}

//
// Pick solutions to go on with, if so requested
//
void BaseBySubgameG::BaseSelectSolutions(int p_subgame, const Efg &p_efg, 
					 gList<BehavSolution> &p_solutions)
{
  if (!m_pickSoln || p_solutions.Length() == 0) 
    return;

  if (p_solutions.Length() > 0 && p_efg.TotalNumInfosets() > 0)
    m_parent->PickSolutions(p_efg, m_subgameRoots[p_subgame], p_solutions);
}

void BaseBySubgameG::BaseViewSubgame(int, const Efg &, EFSupport &)
{ 
}

#include "nfstrat.h"

void BaseBySubgameG::BaseViewNormal(const Nfg &p_nfg, NFSupport &p_support)
{
  if (!m_eliminate)  return;

  gArray<int> players(p_nfg.NumPlayers());
  for (int i = 1; i <= p_nfg.NumPlayers(); i++) 
    players[i] = i;


  if (m_iterative) {
    NFSupport *oldSupport = new NFSupport(p_support), *newSupport;
    while ((newSupport = ComputeDominated(oldSupport->Game(), 
					  *oldSupport, m_strong,
					  players, gnull, gstatus)) != 0) {
      delete oldSupport;
      oldSupport = newSupport;
    }

    p_support = *oldSupport;
    delete oldSupport;
  }
  else {
    NFSupport *newSupport;
    if ((newSupport = ComputeDominated(p_support.Game(), 
				       p_support, m_strong,
				       players, gnull, gstatus)) != 0) {
      p_support = *newSupport;
      delete newSupport;
    }
  }
}

//=========================================================================
//                     Algorithm-specific classes
//=========================================================================

//========================================================================
//                              LiapSolve
//========================================================================

#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#include "liapprm.h"

LiapSolveParamsDialog::LiapSolveParamsDialog(wxWindow *p_parent,
					     bool p_subgames, bool p_vianfg)
  : dialogAlgorithm("LiapSolve Parameters", p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
}

LiapSolveParamsDialog::~LiapSolveParamsDialog()
{
}

void LiapSolveParamsDialog::AlgorithmFields(void)
{
  m_nTries = new wxText(this, 0, "nTries");
  NewLine();
  m_tolND = new wxText(this, 0, "Tol n-D");
  NewLine();
  m_tol1D = new wxText(this, 0, "Tol 1-D");
  NewLine();
  m_maxitsND = new wxText(this, 0, "Iterations n-D");
  NewLine();
  m_maxits1D = new wxText(this, 0, "Iterations 1-D");
  NewLine();

  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", -1, -1, -1, -1,
				 3, startOptions);
  NewLine();

  Go();
}

//---------------------
// Liapunov on efg
//---------------------

class EFLiapBySubgameG : public efgLiapSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  EFLiapBySubgameG(const Efg &p_efg, const EFLiapParams &p_params,
		   const BehavSolution &p_start, int p_max = 0,
		   EfgShowInterface *p_parent = 0)
    : efgLiapSolve(p_efg, p_params, 
		   BehavProfile<gNumber>(p_start), p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiefgLiapEfg::guiefgLiapEfg(const EFSupport &p_support,
			     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLiapEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LiapSolve Progress");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(m_startOption);

  EFLiapParams params(status);
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.nTries = m_nTries;

  try {
    return EFLiapBySubgameG(m_efg, params, start,
			    0, m_parent).Solve(EFSupport(m_efg));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapEfg::SolveSetup(void)
{ 
  LiapSolveParamsDialog dialog(m_parent->Frame(), true);

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

    return true;
  }
  else
    return false;
}

//---------------------
// Liapunov on nfg
//---------------------

class NFLiapBySubgameG : public efgLiapNfgSolve, public BaseBySubgameG {
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
		   int p_max = 0, 
		   EfgShowInterface *p_parent = 0)
    : efgLiapNfgSolve(p_efg, p_params,
		      BehavProfile<gNumber>(p_start), p_max),
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
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
  
  try {
    return NFLiapBySubgameG(m_efg, params, start, Eliminate(), EliminateAll(),
			    EliminateWeak(), 0, m_parent).Solve(EFSupport(m_efg));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLiapNfg::SolveSetup(void)
{
  LiapSolveParamsDialog dialog(m_parent->Frame(), true);

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

    return true;
  }
  else
    return false;
}

//========================================================================
//                               LcpSolve
//========================================================================

//---------------------
// LCP on efg
//---------------------

#include "seqform.h"
#include "seqfprm.h"

LcpSolveDialog::LcpSolveDialog(wxWindow *p_parent, bool p_subgames,
			       bool p_vianfg)
  : dialogAlgorithm("LcpSolve Parameters", p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

void LcpSolveDialog::AlgorithmFields(void)
{
  char *precisionChoices[2] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices);
  NewLine();
  m_maxDepth = new wxText(this, 0, "Max depth");
  NewLine();
  m_stopAfter = new wxText(this, 0, "Stop after");
  NewLine();
}

class SeqFormBySubgameG : public efgLcpSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }
  
public:
  SeqFormBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : efgLcpSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiefgLcpEfg::guiefgLcpEfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLcpEfg::Solve(void) const
{
  if (m_efg.NumPlayers() != 2) {
    wxMessageBox("Only valid for two-player games");
    return gList<BehavSolution>();
  }

  wxStatus status(m_parent->Frame(), "LcpSolve Progress");
  SeqFormParams params(status);
  params.stopAfter = m_stopAfter;
  params.maxdepth = m_maxDepth;
  params.precision = m_precision;

  try {
    return SeqFormBySubgameG(m_efg, m_support, params,
			     0, m_parent).Solve(m_support);
  }
  catch (gSignalBreak &) { }

  return gList<BehavSolution>();
}

bool guiefgLcpEfg::SolveSetup(void)
{ 
  LcpSolveDialog dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    m_maxDepth = dialog.MaxDepth();
    return true;
  }
  else
    return false;
}


//---------------------
// LCP on nfg
//---------------------

#include "lemkesub.h"

class LemkeBySubgameG : public efgLcpNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  LemkeBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		  const LemkeParams &p_params, bool p_eliminate,
		  bool p_iterative, bool p_strong, int p_max = 0,
		  EfgShowInterface *p_parent = 0)
    : efgLcpNfgSolve(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLcpNfg::guiefgLcpNfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLcpNfg::Solve(void) const
{
  if (m_efg.NumPlayers() != 2)  {
    wxMessageBox("LCP algorithm only works on 2 player games.",
		 "Algorithm Error");
    return gList<BehavSolution>();
  }

  wxStatus status(m_parent->Frame(), "LcpSolve Progress");

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  try {
    LemkeBySubgameG M(m_efg, m_support, params, Eliminate(), EliminateAll(),
		      EliminateWeak(), 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &)  {
    return gList<BehavSolution>();
  }
}

bool guiefgLcpNfg::SolveSetup(void)
{
  LcpSolveDialog dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    m_maxDepth = dialog.MaxDepth();
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
			      public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiSubgameEnumPureNfg(const Efg &p_efg, const EFSupport &p_support,
			bool p_eliminate, bool p_iterative, bool p_strong,
			int p_stopAfter, gStatus &p_status,
			EfgShowInterface *p_parent = 0)
    : efgEnumPureNfgSolve(p_support, p_stopAfter, p_status),
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
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
    return guiSubgameEnumPureNfg(m_efg, m_support, Eliminate(), EliminateAll(),
				 EliminateWeak(), m_stopAfter,
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

class guiEnumPureEfgSubgame : public efgEnumPure, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewSubgame(int p_subgame, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_subgame, p_efg, p_support); }

public:
  guiEnumPureEfgSubgame(const Efg &p_efg, const EFSupport &p_support,
			int p_stopAfter, gStatus &p_status,
			EfgShowInterface *p_parent = 0)
    : efgEnumPure(p_stopAfter, p_status), BaseBySubgameG(p_parent, p_efg)
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
    m_eliminateMixed = dialog.EliminateMixed();
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

#include "enumprm.h"

EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *p_parent,
					     bool p_subgames,
					     bool p_vianfg)
  : dialogAlgorithm("EnumMixedSolve Params", p_parent, ENUMMIXED_HELP)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

EnumSolveParamsDialog::~EnumSolveParamsDialog()
{
}

void EnumSolveParamsDialog::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters:");
  NewLine();

  m_stopAfter = new wxText(this, 0, "Stop after");
  NewLine();
  char *precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices);
  NewLine();
}


wxEnumStatus::wxEnumStatus(wxFrame *p_parent)
  : wxStatus(p_parent, "EnumMixedSolve"), pass(0)
{ }

void wxEnumStatus::SetProgress(double p_value)
{
  if (p_value > -.5)  {
    // actually, as long as its not -1.0, but floating point ==
    gauge->SetValue((int) ((p_value + pass) / 3.0 *100.0));
  }
  else {
    pass++;
  }
  wxYield();
}

//---------------------
// EnumMixed on efg
//---------------------

#include "enumsub.h"

class EnumBySubgameG : public EnumBySubgame, public BaseBySubgameG {
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
		 int p_max = 0, EfgShowInterface *p_parent = 0)
    : EnumBySubgame(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgEnumMixedNfg::guiefgEnumMixedNfg(const EFSupport &p_support,
				       EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgEnumMixedNfg::Solve(void) const
{
  wxEnumStatus status(m_parent->Frame());
  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  try {
    EnumBySubgameG M(m_efg, m_support, params, Eliminate(), EliminateAll(),
		     EliminateWeak(), 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgEnumMixedNfg::SolveSetup(void)
{
  EnumSolveParamsDialog dialog(m_parent->Frame(), true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    return true;
  }
  else
    return false;
}


//========================================================================
//                                LpSolve
//========================================================================

#include "csumsub.h"
#include "efgcsum.h"
#include "csumprm.h"

LPSolveParamsDialog::LPSolveParamsDialog(wxWindow *p_parent, bool p_subgames,
					 bool p_vianfg)
  : dialogAlgorithm("LpSolve Parameters", p_parent, LP_HELP)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

LPSolveParamsDialog::~LPSolveParamsDialog()
{
}

void LPSolveParamsDialog::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm Parameters:");
  NewLine();

  m_stopAfter = new wxText(this, 0, "Stop after");
  NewLine();
  char *precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices);
  NewLine();
}

//---------------------
// Lp on nfg
//---------------------


class ZSumBySubgameG : public efgLpNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  ZSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		 const ZSumParams &p_params, bool p_eliminate,
		 bool p_iterative, bool p_strong, int p_max = 0,
		 EfgShowInterface *p_parent = 0)
    : efgLpNfgSolve(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

guiefgLpNfg::guiefgLpNfg(const EFSupport &p_support,
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLpNfg::Solve(void) const
{
  if (m_efg.NumPlayers() > 2 || !m_efg.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return gList<BehavSolution>();
  }

  wxStatus status(m_parent->Frame(), "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  try {
    ZSumBySubgameG M(m_efg, m_support, params, Eliminate(), EliminateAll(),
		     EliminateWeak(), 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpNfg::SolveSetup(void)
{
  LPSolveParamsDialog dialog(m_parent->Frame(), true, true); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    return true;
  }
  else
    return false;
}

//---------------------
// Lp on efg
//---------------------

class EfgCSumBySubgameG : public efgLpSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewSubgame(int p_number, const Efg &p_efg, EFSupport &p_support)
    { BaseViewSubgame(p_number, p_efg, p_support); }

public:
  EfgCSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const CSSeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : efgLpSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiefgLpEfg::guiefgLpEfg(const EFSupport &p_support,
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgLpEfg::Solve(void) const
{
  if (m_efg.NumPlayers() > 2 || !m_efg.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return gList<BehavSolution>();
  }
  
  wxStatus status(m_parent->Frame(), "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  CSSeqFormParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
 
  try {
    EfgCSumBySubgameG M(m_efg, m_support, params, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgLpEfg::SolveSetup(void)
{
  LPSolveParamsDialog dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    return true;
  }
  else
    return false;
}

//========================================================================
//                           SimpdivSolve
//========================================================================

//---------------------
// Simpdiv on nfg
//---------------------

#include "simpsub.h"
#include "simpprm.h"

SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *p_parent /*=0*/,
						   bool p_subgames /*=false*/)
  : dialogAlgorithm("SimpdivSolve Params", p_parent, SIMPDIV_HELP)
{
  MakeCommonFields(true, p_subgames, true);
  Go();
}

void SimpdivSolveParamsDialog::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  m_nRestarts = new wxText(this, 0, "# restarts");
  NewLine();
  m_leashLength = new wxText(this, 0, "Leash");
  NewLine();
  m_stopAfter = new wxText(this, 0, "Stop after");
  NewLine();
  char *precisionChoices[2] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices);
}  

class SimpdivBySubgameG : public efgSimpDivNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  SimpdivBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SimpdivParams &p_params, bool p_eliminate,
		    bool p_iterative, bool p_strong, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : efgSimpDivNfgSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiefgSimpdivNfg::guiefgSimpdivNfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgSimpdivNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "SimpdivSolve Progress");

  SimpdivParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;

  try {
    SimpdivBySubgameG M(m_efg, m_support, params, Eliminate(), EliminateAll(),
			EliminateWeak(), 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgSimpdivNfg::SolveSetup(void)
{
  SimpdivSolveParamsDialog dialog(m_parent->Frame(), true); 

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

    return true;
  }
  else
    return false;
}

//========================================================================
//                           PolEnumSolve
//========================================================================

#include "peprm.h"
#include "polenum.h"
#include "polensub.h"

guiPolEnumParamsDialog::guiPolEnumParamsDialog(wxWindow *p_parent,
					       bool p_subgames,
					       bool p_vianfg)
  : dialogAlgorithm("PolEnumSolve Parameters", p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

void guiPolEnumParamsDialog::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  NewLine();
  m_stopAfter = new wxText(this, 0, "Stop after");
  NewLine();
}

//------------------
// PolEnum on nfg
//------------------

class guiPolEnumEfgByNfgSubgame : public efgPolEnumNfgSolve,
				  public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  
public:
  guiPolEnumEfgByNfgSubgame(const Efg &p_efg, const EFSupport &p_support,
			    const PolEnumParams &p_params, int p_max = 0,
			    EfgShowInterface *p_parent = 0)
    : efgPolEnumNfgSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiefgPolEnumNfg::guiefgPolEnumNfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgPolEnumNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "PolEnum Algorithm");
  status.SetProgress(0.0);
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;

  try {
    guiPolEnumEfgByNfgSubgame M(m_efg, m_support, params, 0, m_parent);
    return M.Solve(m_support);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiefgPolEnumNfg::SolveSetup(void)
{
  guiPolEnumParamsDialog dialog(m_parent->Frame(), true, true); 

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
  guiPolEnumParamsDialog dialog(m_parent->Frame(), true); 

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

#include "gobitprm.h"
#include "ngobit.h"
#include "egobit.h"

QreSolveParamsDialog::QreSolveParamsDialog(wxWindow *p_parent,
					   const gText p_filename,
					   bool p_vianfg)
  : PxiParamsDialog("Qre","QRESolve Params", p_filename, p_parent, QRE_HELP)
{
  MakeCommonFields(true, false, p_vianfg);
  Go();
}

void QreSolveParamsDialog::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  m_minLam = new wxText(this, 0, "minLam");
  m_maxLam = new wxText(this, 0, "maxLam");
  m_delLam = new wxText(this, 0, "delLam");
  NewLine();

  m_tolN = new wxText(this, 0, "Tolerance n-D");
  m_tol1 = new wxText(this, 0, "Tolerance 1-D");
  NewLine();

  m_maxitsN = new wxText(this, 0, "Iterations n-D");
  m_maxits1 = new wxText(this, 0, "Iterations 1-D");
  NewLine();

  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", -1, -1, -1, -1,
				 3, startOptions);
  NewLine();
}  

//---------------------
// Qre on nfg
//---------------------

guiefgQreNfg::guiefgQreNfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgQreNfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");

  NFQreParams params(status);

  Nfg *N = MakeReducedNfg(EFSupport(m_efg));

  BehavProfile<gNumber> startb = m_parent->CreateStartProfile(m_startOption);
  MixedProfile<gNumber> startm(*N);

  BehavToMixed(m_efg, startb, *N, startm);
  
  long nevals, nits;
  gList<MixedSolution> nfg_solns;

  try {
    Qre(*N, params, startm, nfg_solns, nevals, nits);
    //GSPD.RunPxi();
  }
  catch (gSignalBreak &) { }

  gList<BehavSolution> solutions;

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    MixedToBehav(*N, nfg_solns[i], m_efg, startb);
    solutions.Append(BehavSolution(startb, EfgAlg_QRE));
  }

  delete N;

  return solutions;
}

bool guiefgQreNfg::SolveSetup(void)
{
  QreSolveParamsDialog dialog(m_parent->Frame(), m_parent->Filename(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();
    return true;
  }
  else
    return false;
}

//---------------------
// Qre on efg
//---------------------

guiefgQreEfg::guiefgQreEfg(const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgQreEfg::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(m_startOption);
  EFQreParams P(status);
  long nevals, nits;
  gList<BehavSolution> solns;

  try {
    Qre(m_efg, P, start, solns, nevals, nits);
    if (!solns[1].IsSequential()) {
      wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		   "Returning last value.\n");
    }
    //dialog.RunPxi();
  }
  catch (gSignalBreak &) { }

  return solns;
}

bool guiefgQreEfg::SolveSetup(void)
{ 
  QreSolveParamsDialog dialog(m_parent->Frame(), m_parent->Filename()); 

  if (dialog.Completed() == wxOK) {
    m_eliminate =dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();
    m_markSubgames = dialog.MarkSubgames();

    m_startOption = dialog.StartOption();

    return true;
  }
  else
    return false;
}

//========================================================================
//                           QreGridSolve
//========================================================================

//---------------------
// Grid on nfg
//---------------------

#include "grid.h"
#include "gridprm.h"

GridSolveParamsDialog::GridSolveParamsDialog(wxWindow *p_parent,
					     const gText &/*p_filename*/)
  : dialogAlgorithm("QreAllSolve Params", p_parent, QRE_HELP)
{
  MakeCommonFields(true, false, true);
  Go();
}

void GridSolveParamsDialog::AlgorithmFields(void)
{
#ifdef UNUSED
  SetLabelPosition(wxVERTICAL);
  wxText *minLamt = new wxText(this, 0, "minLam", "", 24, 11, 104, 50, 
			       wxVERTICAL_LABEL, "minLam");
  wxText *maxLamt = new wxText(this, 0, "maxLam", "", 339, 14, 118, 52,
			       wxVERTICAL_LABEL, "maxLam");
  wxText *delLamt = new wxText(this, 0, "delLam", "", 188, 13, 126, 52, 
			       wxVERTICAL_LABEL, "delLam");
  SetLabelPosition(wxHORIZONTAL);
  (void)new wxGroupBox(this, "Grid #1", 7, 83, 136, 126, 0, "box1");
  wxText *delp1t = new wxText(this, 0, "Del", "", 13, 107, 112, 30, 0, "delp1");
  wxText *tol1t = new wxText(this, 0, "Tol", "", 16, 156, 106, 30, 0, "tol1");
  (void)new wxGroupBox(this, "Grid # 2", 162, 83, 144, 124, 0, "box2");
  wxText *delp2t = new wxText(this, 0, "Del", "", 181, 109, 102, 30, 0, "delp2");
  wxText *tol2t = new wxText(this, 0, "Tol", "", 182, 151, 104, 30, 0, "tol2");
  char *stringArray46[] = { "Lin", "Log" };
  wxRadioBox *pxitypet = new wxRadioBox(this, 0, "Plot Type", 
					315, 94, -1, -1, 2, stringArray46, 
					2, 0, "pxitype");
  wxCheckBox *multigridt = new wxCheckBox(this, 0, "Use MultiGrid", 
					  317, 171, -1, -1, 0, "multgrid");
  SetLabelPosition(wxVERTICAL);
  (void)new wxGroupBox(this, "PXI Output", 1, 209, 452, 162, 0, "box3");
  wxText *pxifilet = new wxText(this, 0, "Pxi File", "", 
				13, 236, 130, 54, wxVERTICAL_LABEL, "pxfile");
  SetLabelPosition(wxHORIZONTAL);
  char *stringArray47[] = { "Default", "Saved", "Prompt" };
  wxRadioBox *next_typet = new wxRadioBox(this, 0, "Next File", 
					  157, 230, -1, -1, 3,
					  stringArray47, 3, 0, "next_type");
  wxCheckBox *run_boxt = new wxCheckBox(this, 0, "Run PXI", 
					18, 319, -1, -1, 0, "run_box");
  wxText *pxi_commandt = new wxText(this, 0, "Pxi Command", "", 
				    116, 312, 104, 30, 0, "pxi_command");
  char *stringArray48[] = { "1", "2", "3", "4" };
  (void)new wxGroupBox(this, "Debug Output", 2, 373, 456, 68, 0, "box4");
  wxChoice *tracet = new wxChoice(this, 0, "Trace Level", 
				  7, 401, -1, -1, 4, stringArray48, 0, "trace_choice");
  wxText *tracefilet = new wxText(this, 0, "Trace File", "", 
                                    187, 402, 200, 30, 0, "trace_file");

  minLamt->SetValue(ToText(minLam));
  maxLamt->SetValue(ToText(maxLam));
  delLamt->SetValue(ToText(delLam));
  delp1t->SetValue(ToText(delp1));
  delp2t->SetValue(ToText(delp2));
  tol1t->SetValue(ToText(tol1));
  tol2t->SetValue(ToText(tol2));
  //tracefilet->SetValue(outname);
  //tracet->SetStringSelection(trace_str);
  //pxitypet->SetStringSelection(type_str);
  //pxifilet->SetValue(pxiname);
  //next_typet->SetStringSelection(name_option_str);
  //pxi_commandt->SetValue(pxi_command);
  //run_boxt->SetValue(run_pxi);
  multigridt->SetValue(multi_grid);
  Go();

  if (Completed() == wxOK) {
    minLam = strtod(minLamt->GetValue(), 0);
    maxLam = strtod(maxLamt->GetValue(), 0);
    delLam = strtod(delLamt->GetValue(), 0);
    delp1  = strtod(delp1t->GetValue(),  0);
    tol1   = strtod(tol1t->GetValue(),   0);
    delp2  = strtod(delp2t->GetValue(),  0);
    tol2   = strtod(tol2t->GetValue(),   0);

    //strcpy(outname,         tracefilet->GetValue());
    //strcpy(trace_str,       tracet->GetStringSelection());
    //strcpy(type_str,        pxitypet->GetStringSelection());
    //strcpy(pxiname,         pxifilet->GetValue());
    //strcpy(name_option_str, next_typet->GetStringSelection());
    //strcpy(pxi_command,     pxi_commandt->GetValue());
    
    //run_pxi    = run_boxt->GetValue();
    multi_grid = multigridt->GetValue();
  }

#endif // UNUSED
}

class QreAllBySubgameG : public BaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport &p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  QreAllBySubgameG(const Efg &, const EFSupport &, bool p_eliminate,
		   bool p_iterative, bool p_strong, EfgShowInterface *);

};

QreAllBySubgameG::QreAllBySubgameG(const Efg &p_efg, 
				       const EFSupport &p_support,
				       bool p_eliminate, bool p_iterative,
				       bool p_strong,
				       EfgShowInterface *p_parent)
  : BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
{
  wxStatus status(m_parent->Frame(), "QreAllSolve Progress");
  GridParams params(status);

  Nfg *N = MakeReducedNfg(p_support);
  NFSupport S(*N);
  ViewNormal(*N, S);

  gList<MixedSolution> solns;
  try {
    GridSolve(S, params, solns);
  }
  catch (gSignalBreak &) { }

  //dialog.RunPxi();
  delete N;
}

guiefgQreAllNfg::guiefgQreAllNfg(const EFSupport &p_support, 
				 EfgShowInterface *p_parent)
  : guiEfgSolution(p_support, p_parent)
{ }

gList<BehavSolution> guiefgQreAllNfg::Solve(void) const
{
  QreAllBySubgameG M(m_efg, m_support, Eliminate(), EliminateAll(),
		     EliminateWeak(), m_parent);
  return gList<BehavSolution>();
}

bool guiefgQreAllNfg::SolveSetup(void)
{
  GridSolveParamsDialog dialog(m_parent->Frame(), m_parent->Filename()); 

  if (dialog.Completed() == wxOK) {
    m_eliminate = false;
    m_eliminateAll = false;
    m_eliminateWeak = false;
    m_eliminateMixed = false;
    m_markSubgames = false;
    return true;
  }
  else
    return false;
}
