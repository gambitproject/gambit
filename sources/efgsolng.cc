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

guiEfgSolution::guiEfgSolution(const Efg &p_efg, const EFSupport &p_support,
			       EfgShowInterface *p_parent)
  : ef(p_efg), sup(p_support), parent(p_parent)
{ }

//=========================================================================
//                    BaseBySubgameG: Class definition
//=========================================================================

#define SELECT_SUBGAME_NUM  10000

// in efgutils.cc
extern void MarkedSubgameRoots(const Efg &efg, gList<Node *> &list); 

class BaseBySubgameG {
protected:
  EfgShowInterface *m_parent;
  Bool m_pickSoln;
  gList<Node *> m_subgameRoots;
  bool m_eliminate, m_iterative, m_strong;
  
  void BaseSelectSolutions(int, const Efg &, gList<BehavSolution> &);
  void BaseViewNormal(const Nfg &, NFSupport *&);

public:
  BaseBySubgameG(EfgShowInterface *, const Efg &,
		 bool p_eliminate = false, bool p_iterative = false,
		 bool p_strong = false);
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
  if (!m_pickSoln) 
    return;

  m_parent->SetPickSubgame(m_subgameRoots[p_subgame]);

  if (p_solutions.Length() == 0) {
    wxMessageBox("No solutions were found for this subgame");
    m_parent->SetPickSubgame(0);
    return;
  }
  
  if (p_efg.TotalNumInfosets() > 0)
    m_parent->PickSolutions(p_efg, p_solutions);

  // turn off the subgame picking icon at the last subgame
  if (p_subgame == m_subgameRoots.Length()) 
    m_parent->SetPickSubgame(0);
}

// Eliminated dominanted strats, if so requested
extern NFSupport *ComputeDominated(const Nfg &, NFSupport &, bool strong,
				   const gArray<int> &players,
				   gOutput &tracefile, gStatus &gstatus);

#include "nfstrat.h"

void BaseBySubgameG::BaseViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
{
  if (!m_eliminate)  return;

  gArray<int> players(p_nfg.NumPlayers());
  for (int i = 1; i <= p_nfg.NumPlayers(); i++) 
    players[i] = i;


  if (m_iterative) {
    NFSupport *oldSupport = p_support, *newSupport;
    while ((newSupport = ComputeDominated(oldSupport->Game(), 
					  *oldSupport, m_strong,
					  players, gnull, gstatus)) != 0) {
      delete oldSupport;
      oldSupport = newSupport;
    }

    p_support = oldSupport;
  }
  else {
    NFSupport *newSupport;
    if ((newSupport = ComputeDominated(p_support->Game(), 
				       *p_support, m_strong,
				       players, gnull, gstatus)) != 0) {
      delete p_support;
      p_support = newSupport;
    }
  }
}

//=========================================================================
//                     Algorithm-specific classes
//=========================================================================

#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#include "liapprm.h"

LiapParamsSettings::LiapParamsSettings(void)
{
  wxGetResource(PARAMS_SECTION,"Liap-Ntries",&nTries,defaults_file);
  wxGetResource(PARAMS_SECTION,"Func-tolN",&tolN,defaults_file);
  wxGetResource(PARAMS_SECTION,"Func-tol1",&tol1,defaults_file);
  wxGetResource(PARAMS_SECTION,"Func-maxitsN",&maxitsN,defaults_file);
  wxGetResource(PARAMS_SECTION,"Func-maxits1",&maxits1,defaults_file);
  wxGetResource(PARAMS_SECTION,"Start-Option",&start_option,defaults_file);
}

void LiapParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION,"Liap-Ntries",nTries,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Func-tolN",tolN,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Func-tol1",tol1,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Func-maxitsN",maxitsN,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Func-maxits1",maxits1,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Start-Option",start_option,defaults_file);
}

LiapParamsSettings::~LiapParamsSettings(void)
{ SaveDefaults(); }

void LiapParamsSettings::GetParams(EFLiapParams &p_params)
{
  p_params.tol1 = tol1;
  p_params.tolN = tolN;
  p_params.maxits1 = maxits1;
  p_params.maxitsN = maxitsN;
  p_params.stopAfter = StopAfter();
  p_params.nTries = nTries;

  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

void LiapParamsSettings::GetParams(NFLiapParams &p_params)
{
  p_params.tol1 = tol1;
  p_params.tolN = tolN;
  p_params.maxits1 = maxits1;
  p_params.maxitsN = maxitsN;
  p_params.stopAfter = StopAfter();
  p_params.nTries = nTries;

  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

LiapSolveParamsDialog::LiapSolveParamsDialog(wxWindow *p_parent,
					     bool p_subgames)
  : OutputParamsDialog("LiapSolve Parameters", p_parent)
{
  MakeCommonFields(true, p_subgames);

  Add(wxMakeFormShort("Max # Tries", &nTries, wxFORM_DEFAULT, NULL, NULL,
		      wxVERTICAL, 100));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormFloat("Tolerance n-D", &tolN, wxFORM_DEFAULT, NULL, NULL,
		      wxVERTICAL, 100));
  Add(wxMakeFormFloat("Tolerance 1-D", &tol1, wxFORM_DEFAULT, NULL, NULL,
		      wxVERTICAL, 100));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormShort("Iterations n-D", &maxitsN, wxFORM_DEFAULT, NULL, NULL,
		      wxVERTICAL, 100));
  Add(wxMakeFormShort("Iterations 1-D", &maxits1, wxFORM_DEFAULT, NULL, NULL,
		      wxVERTICAL, 100));
  Add(wxMakeFormNewLine());
  wxStringList *start_option_list=new wxStringList("Default", "Saved",
						   "Prompt", 0);
  char *start_option_str = new char[20];
  strcpy(start_option_str,
	 (char *) start_option_list->Nth(start_option)->Data());
  Add(wxMakeFormString("Start", &start_option_str, wxFORM_RADIOBOX, 
		       new wxList(wxMakeConstraintStrings(start_option_list),
				  0), 0, wxVERTICAL));
  Add(wxMakeFormNewLine());

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();

  start_option = wxListFindString(start_option_list, start_option_str);
  delete [] start_option_str;
  delete start_option_list;
}


//---------------------
// Liapunov on efg
//---------------------

class EFLiapBySubgameG : public efgLiapSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }

public:
  EFLiapBySubgameG(const Efg &p_efg, const EFLiapParams &p_params,
		   const BehavSolution &p_start, int p_max = 0,
		   EfgShowInterface *p_parent = 0)
    : efgLiapSolve(p_efg, p_params, 
		   BehavProfile<gNumber>(p_start), p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

guiEfgSolveLiap::guiEfgSolveLiap(const Efg &p_efg, const EFSupport &p_support,
				 EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgSolveLiap::Solve(void) const
{
  LiapParamsSettings LPS;
  wxStatus status(parent->Frame(), "LiapSolve");
  BehavProfile<gNumber> start = parent->CreateStartProfile(LPS.StartOption());
  EFLiapParams P(status);
  LPS.GetParams(P);
  try {
    EFLiapBySubgameG M(ef, P, start, LPS.MaxSolns(), parent);
    return M.Solve(EFSupport(ef));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiEfgSolveLiap::SolveSetup(void) const
{ 
  LiapSolveParamsDialog LSPD(parent->Frame(), true);

  if (LSPD.Completed() == wxOK) {
    eliminate = LSPD.Eliminate();
    all = LSPD.EliminateAll();
    domType = LSPD.DominanceType();
    domMethod = LSPD.DominanceMethod();
    markSubgames = LSPD.MarkSubgames();

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
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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

EfgNLiapG::EfgNLiapG(const Efg &p_efg, const EFSupport &p_support, 
		     EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgNLiapG::Solve(void) const
{
  LiapParamsSettings LPS;
  wxStatus status(parent->Frame(), "Liap Algorithm");
  BehavProfile<gNumber> start = parent->CreateStartProfile(LPS.StartOption());
  NFLiapParams P(status);
  LPS.GetParams(P);
  try {
    NFLiapBySubgameG M(ef, P, start, Eliminate(), EliminateAll(),
		       DominanceType(), LPS.MaxSolns(), parent);
    return M.Solve(EFSupport(ef));
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgNLiapG::SolveSetup(void) const
{
  LiapSolveParamsDialog LSPD(parent->Frame(), true);

  if (LSPD.Completed() == wxOK)  {
    eliminate = LSPD.Eliminate();
    all = LSPD.EliminateAll();
    domType = LSPD.DominanceType();
    domMethod = LSPD.DominanceMethod();
    markSubgames = LSPD.MarkSubgames();
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

SeqFormParamsSettings::SeqFormParamsSettings(void)
{
  wxGetResource(PARAMS_SECTION, "SeqForm-dup_strat", &dup_strat, 
		defaults_file);
  wxGetResource(PARAMS_SECTION, "SeqForm-maxdepth", &maxdepth, defaults_file);
}

void SeqFormParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION, "SeqForm-dup_strat" ,dup_strat,
		  defaults_file);
  wxWriteResource(PARAMS_SECTION, "SeqForm-maxdepth", maxdepth, defaults_file);
}

SeqFormParamsSettings::~SeqFormParamsSettings(void)
{ SaveDefaults(); }

void SeqFormParamsSettings::GetParams(SeqFormParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.maxdepth = maxdepth;
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

SeqFormParamsDialog::SeqFormParamsDialog(wxWindow *p_parent /* =0 */,
					 bool p_subgames /* = false */)
  : OutputParamsDialog("LcpSolve Params", p_parent)
{
  MakeCommonFields(true, p_subgames);

  Add(wxMakeFormBool("All Solutions", &dup_strat));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormShort("Max depth", &maxdepth));

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD | PRECISION_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
}

class SeqFormBySubgameG : public efgLcpSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  
public:
  SeqFormBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : efgLcpSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

EfgSeqFormG::EfgSeqFormG(const Efg &p_efg, const EFSupport &p_support, 
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgSeqFormG::Solve(void) const
{
  wxStatus status(parent->Frame(), "LCP Algorithm");
  SeqFormParamsSettings SFPS;
  SeqFormParams P(status);
  SFPS.GetParams(P);
  try {
    SeqFormBySubgameG M(ef, sup, P, SFPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgSeqFormG::SolveSetup(void) const
{ 
  SeqFormParamsDialog SFPD(parent->Frame(), true);

  if (SFPD.Completed() == wxOK)  {
    eliminate = SFPD.Eliminate();
    all = SFPD.EliminateAll();
    domType = SFPD.DominanceType();
    domMethod = SFPD.DominanceMethod();
    markSubgames = SFPD.MarkSubgames();
    return true;
  }
  else
    return false;
}


//---------------------
// LCP on nfg
//---------------------

#include "lemkesub.h"
#include "lemkeprm.h"

LemkeParamsSettings::LemkeParamsSettings(void)
{ }

void LemkeParamsSettings::SaveDefaults(void)
{ }

void LemkeParamsSettings::GetParams(LemkeParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *p_parent /* = 0 */,
					       bool p_subgames /* = false */)
  : OutputParamsDialog("LcpSolve Params", p_parent, LCP_HELP)
{
  MakeCommonFields(true, p_subgames);

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD | PRECISION_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
}

class LemkeBySubgameG : public efgLcpNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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

EfgLemkeG::EfgLemkeG(const Efg &p_efg, const EFSupport &p_support, 
		     EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgLemkeG::Solve(void) const
{
  wxStatus status(parent->Frame(), "LCP Algorithm");

  if (ef.NumPlayers() != 2)  {
    wxMessageBox("LCP algorithm only works on 2 player games.",
					 "Algorithm Error");
    return solns;
  }

  LemkeParamsSettings LPS;
  LemkeParams P(status);
  LPS.GetParams(P);
  try {
    LemkeBySubgameG M(ef, sup, P, Eliminate(), EliminateAll(), DominanceType(),
		      LPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &)  {
    return gList<BehavSolution>();
  }
}

bool EfgLemkeG::SolveSetup(void) const
{
  LemkeSolveParamsDialog LSPD(parent->Frame(), true); 

  if (LSPD.Completed() == wxOK) {
    eliminate = LSPD.Eliminate();
    all = LSPD.EliminateAll();
    domType = LSPD.DominanceType();
    domMethod = LSPD.DominanceMethod();
    markSubgames = LSPD.MarkSubgames();
    return true;
  }
  else
    return false;
}


//========================================================================
//                            EnumPureSolve
//========================================================================

//---------------------
// EnumPure on nfg
//---------------------

#include "psnesub.h"
#include "purenprm.h"

PureNashSolveParamsDialog::PureNashSolveParamsDialog(wxWindow *p_parent /*=0*/,
						     bool p_subgames/*=false*/)
  : OutputParamsDialog("EnumPureSolve Params", p_parent)
{
  MakeCommonFields(true, p_subgames);

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
}

class PureNashBySubgameG : public efgEnumPureNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  PureNashBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		     bool p_eliminate, bool p_iterative, bool p_strong,
		     int p_max = 0, EfgShowInterface *p_parent = 0)
    : efgEnumPureNfgSolve(p_support, gstatus, p_max),
      BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
    { }
};

EfgPureNashG::EfgPureNashG(const Efg &p_efg, const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgPureNashG::Solve(void) const
{
  PureNashParamsSettings PNPS;
  wxStatus status(parent->Frame(), "EnumPure Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  try {
    PureNashBySubgameG M(ef, sup, Eliminate(), EliminateAll(),
			 DominanceType(), PNPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgPureNashG::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(parent->Frame(), true); 

  if (PNPD.Completed() == wxOK) {
    eliminate = PNPD.Eliminate();
    all = PNPD.EliminateAll();
    domType = PNPD.DominanceType();
    domMethod = PNPD.DominanceMethod();
    markSubgames = PNPD.MarkSubgames();
    return true;
  }
  else
    return false;
}


//---------------------
// EnumPure on efg
//---------------------

#include "efgpure.h"

class EPureNashBySubgameG : public efgEnumPure, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }

public:
  EPureNashBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		      int p_max = 0, EfgShowInterface *p_parent = 0)
    : efgEnumPure(p_max), BaseBySubgameG(p_parent, p_efg)
    { }
};

EfgEPureNashG::EfgEPureNashG(const Efg &p_efg, const EFSupport &p_support, 
			     EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgEPureNashG::Solve(void) const
{
  PureNashParamsSettings PNPS;
  wxStatus status(parent->Frame(), "Efg PureNash");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  try {
    EPureNashBySubgameG M(ef, sup, PNPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgEPureNashG::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(parent->Frame(), true); 

  if (PNPD.Completed() == wxOK) {
    eliminate = PNPD.Eliminate();
    all = PNPD.EliminateAll();
    domType = PNPD.DominanceType();
    domMethod = PNPD.DominanceMethod();
    markSubgames = PNPD.MarkSubgames();
    return true;
  }
  else
    return false;
}

//========================================================================
//                          EnumMixedSolve
//========================================================================

#include "enumprm.h"

EnumParamsSettings::EnumParamsSettings(void)
{ }

void EnumParamsSettings::SaveDefaults(void)
{ }

void EnumParamsSettings::GetParams(EnumParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *p_parent,
					     bool p_subgames)
  : OutputParamsDialog("EnumMixedSolve Params", p_parent, ENUMMIXED_HELP)
{
  MakeCommonFields(true, p_subgames);

  Add(wxMakeFormNewLine());

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD | PRECISION_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
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
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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

EfgEnumG::EfgEnumG(const Efg &p_efg, const EFSupport &p_support,
		   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgEnumG::Solve(void) const
{
  EnumParamsSettings EPS;
  wxEnumStatus status(parent->Frame());
  EnumParams P(status);
  EPS.GetParams(P);

  try {
    EnumBySubgameG M(ef, sup, P, Eliminate(), EliminateAll(),
		     DominanceType(), EPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgEnumG::SolveSetup(void) const
{
  EnumSolveParamsDialog ESPD(parent->Frame(), true); 

  if (ESPD.Completed() == wxOK) {
    eliminate = ESPD.Eliminate();
    all = ESPD.EliminateAll();
    domType = ESPD.DominanceType();
    domMethod = ESPD.DominanceMethod();
    markSubgames = ESPD.MarkSubgames();
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

LPParamsSettings::LPParamsSettings(void)
{ }

void LPParamsSettings::SaveDefaults(void)
{ }

void LPParamsSettings::GetParams(ZSumParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

void LPParamsSettings::GetParams(CSSeqFormParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

LPSolveParamsDialog::LPSolveParamsDialog(wxWindow *p_parent, bool p_subgames)
  : OutputParamsDialog("LpSolve Params", p_parent, LP_HELP)
{
  MakeCommonFields(true, p_subgames);
  Add(wxMakeFormNewLine());

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD | PRECISION_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
}

//---------------------
// Lp on nfg
//---------------------


class ZSumBySubgameG : public efgLpNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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

EfgZSumG::EfgZSumG(const Efg &p_efg, const EFSupport &p_support,
		   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgZSumG::Solve(void) const
{
  if (ef.NumPlayers() > 2 || !ef.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return solns;
  }

  LPParamsSettings LPPS;
  wxStatus status(parent->Frame(), "LPSolve Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams P;
  LPPS.GetParams(P);

  try {
    ZSumBySubgameG M(ef, sup, P, Eliminate(), EliminateAll(), DominanceType(),
		     LPPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgZSumG::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(parent->Frame(), true); 

  if (ZSPD.Completed() == wxOK) {
    eliminate = ZSPD.Eliminate();
    all = ZSPD.EliminateAll();
    domType = ZSPD.DominanceType();
    domMethod = ZSPD.DominanceMethod();
    markSubgames = ZSPD.MarkSubgames();
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

public:
  EfgCSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const CSSeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : efgLpSolve(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { }
};

EfgCSumG::EfgCSumG(const Efg &p_efg, const EFSupport &p_support,
		   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgCSumG::Solve(void) const
{
  if (ef.NumPlayers() > 2 || !ef.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return solns;
  }
  
  LPParamsSettings LPPS;
  wxStatus status(parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  CSSeqFormParams P(status);
  LPPS.GetParams(P);
 
  try {
    EfgCSumBySubgameG M(ef, sup, P, LPPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgCSumG::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(parent->Frame(), true);

  if (ZSPD.Completed() == wxOK) {
    eliminate = ZSPD.Eliminate();
    all = ZSPD.EliminateAll();
    domType = ZSPD.DominanceType();
    domMethod = ZSPD.DominanceMethod();
    markSubgames = ZSPD.MarkSubgames();
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

SimpdivParamsSettings::SimpdivParamsSettings(void)
{
  wxGetResource(PARAMS_SECTION, "Simpdiv-nRestarts", &nRestarts,
		defaults_file);
  wxGetResource(PARAMS_SECTION, "Simpdiv-leashLength", &leashLength,
		defaults_file);
}

void SimpdivParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION, "Simpdiv-nRestarts", nRestarts,
		  defaults_file);
  wxWriteResource(PARAMS_SECTION, "Simpdiv-leashLength", leashLength,
		  defaults_file);
}

void SimpdivParamsSettings::GetParams(SimpdivParams &p_params)
{
  p_params.nRestarts = nRestarts;
  p_params.leashLength = leashLength;
  p_params.stopAfter = StopAfter();
  p_params.precision = Precision();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *p_parent /*=0*/,
						   bool p_subgames /*=false*/)
  : OutputParamsDialog("SimpdivSolve Params", p_parent, SIMPDIV_HELP)
{
  MakeCommonFields(true, p_subgames);

  Add(wxMakeFormNewLine());
  Add(wxMakeFormShort("# Restarts", &nRestarts));
  Add(wxMakeFormShort("Leash", &leashLength));

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD | PRECISION_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
}

class SimpdivBySubgameG : public efgSimpDivNfgSolve, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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

EfgSimpdivG::EfgSimpdivG(const Efg &p_efg, const EFSupport &p_support, 
			 EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgSimpdivG::Solve(void) const
{
  SimpdivParamsSettings SPS;
  wxStatus status(parent->Frame(), "Simpdiv Algorithm");
  SimpdivParams P(status);
  SPS.GetParams(P);

  try {
    SimpdivBySubgameG M(ef, sup, P, Eliminate(), EliminateAll(),
			DominanceType(), SPS.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool EfgSimpdivG::SolveSetup(void) const
{
  SimpdivSolveParamsDialog SDPD(parent->Frame(), true); 

  if (SDPD.Completed() == wxOK) {
    eliminate = SDPD.Eliminate();
    all = SDPD.EliminateAll();
    domType = SDPD.DominanceType();
    domMethod = SDPD.DominanceMethod();
    markSubgames = SDPD.MarkSubgames();
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

guiPolEnumParamsSettings::guiPolEnumParamsSettings(void)
{ }

void guiPolEnumParamsSettings::SaveDefaults(void)
{ }

void guiPolEnumParamsSettings::GetParams(PolEnumParams &p_params)
{
  p_params.stopAfter = StopAfter();
  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

guiPolEnumParamsDialog::guiPolEnumParamsDialog(wxWindow *p_parent,
					       bool p_subgames)
  : OutputParamsDialog("PolEnumSolve Parameters", p_parent, LP_HELP)
{
  MakeCommonFields(true, p_subgames);
  Add(wxMakeFormNewLine());

  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD |
		   ((p_subgames) ? SPS_FIELD : 0));
  Go();
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

guiPolEnumEfgNfg::guiPolEnumEfgNfg(const EFSupport &p_support, 
				   EfgShowInterface *p_parent)
  : guiEfgSolution(p_support.Game(), p_support, p_parent)
{ }

gList<BehavSolution> guiPolEnumEfgNfg::Solve(void) const
{
  guiPolEnumParamsSettings PES;
  wxStatus status(parent->Frame(), "PolEnum Algorithm");
  PolEnumParams P(status);
  PES.GetParams(P);

  try {
    guiPolEnumEfgByNfgSubgame M(ef, sup, P, PES.MaxSolns(), parent);
    return M.Solve(sup);
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

bool guiPolEnumEfgNfg::SolveSetup(void) const
{
  guiPolEnumParamsDialog D(parent->Frame(), true); 

  if (D.Completed() == wxOK) {
    eliminate = D.Eliminate();
    all = D.EliminateAll();
    domType = D.DominanceType();
    domMethod = D.DominanceMethod();
    markSubgames = D.MarkSubgames();
    return true;
  }
  else
    return false;
}

//------------------
// PolEnum on efg
//------------------

guiPolEnumEfg::guiPolEnumEfg(const EFSupport &p_support, 
			     EfgShowInterface *p_parent)
  : guiEfgSolution(p_support.Game(), p_support, p_parent)
{ }

gList<BehavSolution> guiPolEnumEfg::Solve(void) const
{
  return gList<BehavSolution>();
}

bool guiPolEnumEfg::SolveSetup(void) const
{
  guiPolEnumParamsDialog D(parent->Frame(), true); 

  if (D.Completed() == wxOK) {
    eliminate = D.Eliminate();
    all = D.EliminateAll();
    domType = D.DominanceType();
    domMethod = D.DominanceMethod();
    markSubgames = D.MarkSubgames();
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

QreParamsSettings::QreParamsSettings(const char *p_filename)
  : PxiParamsSettings("Qre", p_filename)
{
  wxGetResource(PARAMS_SECTION, "Qre-minLam", &minLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Qre-maxLam", &maxLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Qre-delLam", &delLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Func-tolN", &tolN, defaults_file);
  wxGetResource(PARAMS_SECTION, "Func-tol1", &tol1, defaults_file);
  wxGetResource(PARAMS_SECTION, "Func-maxitsN", &maxitsN, defaults_file);
  wxGetResource(PARAMS_SECTION, "Func-maxits1", &maxits1, defaults_file);
  wxGetResource(PARAMS_SECTION, "Start-Option", &start_option, defaults_file);
}

void QreParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION, "Qre-minLam", minLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Qre-maxLam", maxLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Qre-delLam", delLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Func-tolN", tolN, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Func-tol1", tol1, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Func-maxitsN", maxitsN, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Func-maxits1", maxits1, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Start-Option", start_option, defaults_file);
}

QreParamsSettings::~QreParamsSettings(void)
{ SaveDefaults(); }

void QreParamsSettings::GetParams(EFQreParams &p_params)
{
  p_params.minLam = minLam;
  p_params.maxLam = maxLam;
  p_params.delLam = delLam;
  p_params.tol1 = tol1;
  p_params.tolN = tolN;
  p_params.maxits1 = maxits1;
  p_params.maxitsN = maxitsN;

  p_params.powLam = PxiType();
  p_params.pxifile = PxiFile();

  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}

void QreParamsSettings::GetParams(NFQreParams &p_params)
{
  p_params.minLam = minLam;
  p_params.maxLam = maxLam;
  p_params.delLam = delLam;
  p_params.tol1 = tol1;
  p_params.tolN = tolN;
  p_params.maxits1 = maxits1;
  p_params.maxitsN = maxitsN;

  p_params.powLam = PxiType();
  p_params.pxifile = PxiFile();

  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}


QreSolveParamsDialog::QreSolveParamsDialog(wxWindow *p_parent,
					       const gText p_filename)
  : PxiParamsDialog("Qre","QRESolve Params", p_filename, p_parent,
		    QRE_HELP),
    QreParamsSettings(p_filename), PxiParamsSettings("Qre", p_filename)
{
  MakeCommonFields(true, false);

  Add(wxMakeFormFloat("minLam", &minLam, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormFloat("maxLam", &maxLam, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormFloat("delLam", &delLam, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormFloat("Tolerance n-D", &tolN, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormFloat("Tolerance 1-D", &tol1, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormShort("Iterations n-D", &maxitsN, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormShort("Iterations 1-D", &maxits1, wxFORM_DEFAULT,
		      NULL, NULL, wxVERTICAL, 100));
  Add(wxMakeFormNewLine());

  wxStringList *start_option_list = new wxStringList("Default", "Saved",
						     "Prompt", 0);
  char *start_option_str = new char[20];
  strcpy(start_option_str,
	 (char *)start_option_list->Nth(start_option)->Data());
  Add(wxMakeFormString("Start", &start_option_str, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(start_option_list),
				  0), 0, wxVERTICAL));

  MakePxiFields();
  MakeOutputFields();
  Go();

  start_option = wxListFindString(start_option_list, start_option_str);
  delete [] start_option_str;
  delete start_option_list;
}

//---------------------
// Qre on nfg
//---------------------

//
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
//

class NQreBySubgameG : public BaseBySubgameG {
private:
  gList<BehavSolution> m_solutions;

protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  NQreBySubgameG(const Efg &, bool p_eliminate, bool p_iterative,
		   bool p_strong, EfgShowInterface * = 0);

  gList<BehavSolution> GetSolutions(void) const { return m_solutions; }
};

NQreBySubgameG::NQreBySubgameG(const Efg &p_efg,
				   bool p_eliminate, bool p_iterative,
				   bool p_strong,
				   EfgShowInterface *p_parent /*= 0*/)
  : BaseBySubgameG(p_parent, p_efg, p_eliminate, p_iterative, p_strong)
{
  QreParamsSettings GSPD(m_parent->Filename());
  wxStatus status(m_parent->Frame(), "QRE Algorithm");
  NFQreParams P(status);
  GSPD.GetParams(P);

  EFSupport ES = EFSupport(p_efg);
  Nfg *N = MakeReducedNfg(ES);
  NFSupport *S = new NFSupport(*N);
  ViewNormal(*N, S);

  BehavProfile<gNumber> startb = m_parent->CreateStartProfile(GSPD.StartOption());
  MixedProfile<gNumber> startm(*N);

  BehavToMixed(p_efg, startb, *N, startm);
  
  long nevals, nits;
  gList<MixedSolution> nfg_solns;

  try {
    Qre(*N, P, startm, nfg_solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  GSPD.RunPxi();

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    MixedToBehav(*N, nfg_solns[i], p_efg, startb);
    m_solutions.Append(BehavSolution(startb, EfgAlg_QRE));
  }

  delete N;
  delete S;
}

EfgNQreG::EfgNQreG(const Efg &p_efg, const EFSupport &p_support, 
		       EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgNQreG::Solve(void) const
{
  // exception handler is located in the ctor
  NQreBySubgameG M(ef, Eliminate(), EliminateAll(), DominanceType(), parent);
  return M.GetSolutions();
}

bool EfgNQreG::SolveSetup(void) const
{
  QreSolveParamsDialog GSPD(parent->Frame(), parent->Filename());

  if (GSPD.Completed() == wxOK) {
    eliminate = GSPD.Eliminate();
    all = GSPD.EliminateAll();
    domType = GSPD.DominanceType();
    domMethod = GSPD.DominanceMethod();
    markSubgames = GSPD.MarkSubgames();
    return true;
  }
  else
    return false;
}

//---------------------
// Qre on efg
//---------------------

EfgEQreG::EfgEQreG(const Efg &p_efg, const EFSupport &p_support, 
		       EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgEQreG::Solve(void) const
{
  QreParamsSettings GSPD(parent->Filename());
  wxStatus status(parent->Frame(), "QRE Algorithm");
  BehavProfile<gNumber> start = parent->CreateStartProfile(GSPD.StartOption());
  EFQreParams P(status);
  GSPD.GetParams(P);
  long nevals, nits;
  gList<BehavSolution> solns;

  try {
    Qre(ef, P, start, solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (!solns[1].IsSequential()) {
    wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		 "Returning last value.\n");
  }

  GSPD.RunPxi();
  return solns;
}

bool EfgEQreG::SolveSetup(void) const
{ 
  QreSolveParamsDialog GSPD(parent->Frame(), parent->Filename()); 

  if (GSPD.Completed() == wxOK) {
    eliminate = GSPD.Eliminate();
    all = GSPD.EliminateAll();
    domType = GSPD.DominanceType();
    domMethod = GSPD.DominanceMethod();
    markSubgames = GSPD.MarkSubgames();
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

GridParamsSettings::GridParamsSettings(const char *fn)
  : PxiParamsSettings("grid", fn)
{
  wxGetResource(PARAMS_SECTION, "Grid-minLam", &minLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-maxLam", &maxLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-delLam", &delLam, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-delp1", &delp1, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-tol1", &tol1, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-delp2", &delp2, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-tol2", &tol2, defaults_file);
  wxGetResource(PARAMS_SECTION, "Grid-multigrid", &multi_grid, defaults_file);
}

void GridParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION, "Grid-minLam", minLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-maxLam", maxLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-delLam", delLam, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-delp1", delp1, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-tol1", tol1, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-delp2", delp2, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-tol2", tol2, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Grid-multigrid", multi_grid, defaults_file);
}

GridParamsSettings::~GridParamsSettings(void)
{
  SaveDefaults();
}

void GridParamsSettings::GetParams(GridParams &p_params)
{
  p_params.minLam = minLam;
  p_params.maxLam = maxLam;
  p_params.delLam = delLam;
  p_params.tol1 = tol1;
  p_params.delp1 = delp1;
  p_params.tol2 = tol2;
  p_params.delp2 = delp2;
  p_params.multi_grid = multi_grid;

  p_params.powLam = PxiType();
  p_params.pxifile = PxiFile();

  p_params.trace = TraceLevel();
  p_params.tracefile = OutFile();
}


GridSolveParamsDialog::GridSolveParamsDialog(wxWindow *p_parent,
					     const gText &p_filename)
  : MyDialogBox(p_parent, "QREGridSolve Params", QRE_HELP),
    GridParamsSettings(p_filename), PxiParamsSettings("Grid", p_filename)
{
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
  tracefilet->SetValue(outname);
  tracet->SetStringSelection(trace_str);
  pxitypet->SetStringSelection(type_str);
  pxifilet->SetValue(pxiname);
  next_typet->SetStringSelection(name_option_str);
  pxi_commandt->SetValue(pxi_command);
  run_boxt->SetValue(run_pxi);
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

    strcpy(outname,         tracefilet->GetValue());
    strcpy(trace_str,       tracet->GetStringSelection());
    strcpy(type_str,        pxitypet->GetStringSelection());
    strcpy(pxiname,         pxifilet->GetValue());
    strcpy(name_option_str, next_typet->GetStringSelection());
    strcpy(pxi_command,     pxi_commandt->GetValue());
    
    run_pxi    = run_boxt->GetValue();
    multi_grid = multigridt->GetValue();
  }
}

class QreAllBySubgameG : public BaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
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
  GridParamsSettings GSPD(m_parent->Filename());
  wxStatus status(m_parent->Frame(), "QRE All Solve");
  GridParams P(status);
  GSPD.GetParams(P);

  Nfg *N = MakeReducedNfg(p_support);
  NFSupport *S = new NFSupport(*N);
  ViewNormal(*N, S);

  gList<MixedSolution> solns;
  try {
    GridSolve(*S, P, solns);
  }
  catch (gSignalBreak &) { }

  GSPD.RunPxi();
  delete N;
  delete S;
}

EfgQreAllG::EfgQreAllG(const Efg &p_efg, const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgQreAllG::Solve(void) const
{
  QreAllBySubgameG M(ef, sup, Eliminate(), EliminateAll(),
		       DominanceType(), parent);
  return solns;
}

bool EfgQreAllG::SolveSetup(void) const
{
  GridSolveParamsDialog GSPD(parent->Frame(), parent->Filename()); 

  if (GSPD.Completed() == wxOK) {
    eliminate = GSPD.Eliminate();
    all = GSPD.EliminateAll();
    domType = GSPD.DominanceType();
    domMethod = GSPD.DominanceMethod();
    markSubgames = GSPD.MarkSubgames();
    return true;
  }
  else
    return false;
}
