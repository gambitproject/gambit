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
  
  void BaseSelectSolutions(int, const Efg &, gList<BehavSolution> &);
  void BaseViewNormal(const Nfg &, NFSupport *&);

public:
  BaseBySubgameG(EfgShowInterface *, const Efg &);
};

BaseBySubgameG::BaseBySubgameG(EfgShowInterface *p_parent, const Efg &p_efg)
    : m_parent(p_parent)
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

#include "elimdomd.h"
#include "nfstrat.h"

void BaseBySubgameG::BaseViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
{
  DominanceSettings DS;

  if (!DS.UseElimDom()) 
    return;

  gArray<int> players(p_nfg.NumPlayers());
  for (int i = 1; i <= p_nfg.NumPlayers(); i++) 
    players[i] = i;

  NFSupport *temp_sup = p_support, *temp_sup1 = 0;

  if (DS.FindAll()) {
    while ((temp_sup = ComputeDominated(temp_sup->Game(), 
					*temp_sup, DS.DomStrong(), 
					players, gnull, gstatus))) {
      if (temp_sup1) 
	delete temp_sup1;
      
      temp_sup1 = temp_sup;
    }

    if (temp_sup1) 
      p_support = temp_sup1;
  }
  else {
    if ((temp_sup = ComputeDominated(temp_sup->Game(), 
				     *temp_sup, DS.DomStrong(), 
				     players, gnull, gstatus))) {
      p_support = temp_sup;
    }
  }
}

//=========================================================================
//                     Algorithm-specific classes
//=========================================================================

#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#define LIAP_PRM_INST
#include "liapprm.h"

//---------------------
// Liapunov on efg
//---------------------

class EFLiapBySubgameG : public EFLiapBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }

public:
  EFLiapBySubgameG(const Efg &p_efg, const EFLiapParams &p_params,
		   const BehavSolution &p_solution, int p_max = 0,
		   EfgShowInterface *p_parent = 0)
    : EFLiapBySubgame(p_efg, p_params, 
		      BehavProfile<gNumber>(p_solution), p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
};

EfgELiapG::EfgELiapG(const Efg &p_efg, const EFSupport &p_support,
		     EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgELiapG::Solve(void) const
{
  LiapParamsSettings LPS;
  wxStatus status(parent->Frame(), "Liap Algorithm");
  BehavProfile<gNumber> start = parent->CreateStartProfile(LPS.StartOption());
  EFLiapParams P(status);
  LPS.GetParams(&P);
  try {
    EFLiapBySubgameG M(ef, P, start, LPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgELiapG::SolveSetup(void) const
{ 
  LiapSolveParamsDialog LSPD(parent->Frame(), true);
}

//---------------------
// Liapunov on nfg
//---------------------

class NFLiapBySubgameG : public NFLiapBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
			gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  NFLiapBySubgameG(const Efg &p_efg, const NFLiapParams &p_params,
		   const BehavSolution &p_solution, int p_max = 0, 
		   EfgShowInterface *p_parent = 0)
    : NFLiapBySubgame(p_efg, p_params,
		      BehavProfile<gNumber>(p_solution), p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
  LPS.GetParams(&P);
  try {
    NFLiapBySubgameG M(ef, P, start, LPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgNLiapG::SolveSetup(void) const
{
  LiapSolveParamsDialog LSPD(parent->Frame(), true);
}

#include "seqform.h"
#define SEQF_PRM_INST
#include "seqfprm.h"

//---------------------
// LCP on efg
//---------------------

class SeqFormBySubgameG : public SeqFormBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  
public:
  SeqFormBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : SeqFormBySubgame(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgSeqFormG::SolveSetup(void) const
{ 
  SeqFormParamsDialog SFPD(parent->Frame(), true);
}


//---------------------
// LCP on nfg
//---------------------

#include "lemkesub.h"
#define LEMKE_PRM_INST
#include "lemkeprm.h"

class LemkeBySubgameG : public LemkeBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  LemkeBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		  const LemkeParams &p_params, int p_max = 0,
		  EfgShowInterface *p_parent = 0)
    : LemkeBySubgame(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    LemkeBySubgameG M(ef, sup, P, LPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &)  {
    return gList<BehavSolution>();
  }
}

void EfgLemkeG::SolveSetup(void) const
{
  LemkeSolveParamsDialog LSPD(parent->Frame(), true); 
}


//---------------------
// EnumPure on nfg
//---------------------

#include "psnesub.h"
#define PUREN_PRM_INST
#include "purenprm.h"

class PureNashBySubgameG : public PureNashBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  PureNashBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		     int p_max = 0, EfgShowInterface *p_parent = 0)
    : PureNashBySubgame(p_support, gstatus, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    PureNashBySubgameG M(ef, sup, PNPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgPureNashG::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(parent->Frame(), true); 
}


//---------------------
// EnumPure on efg
//---------------------

#include "efgpure.h"

class EPureNashBySubgameG : public EfgPSNEBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }

public:
  EPureNashBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		      int p_max = 0, EfgShowInterface *p_parent = 0)
    : EfgPSNEBySubgame(p_support, p_max), BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgEPureNashG::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(parent->Frame(), true); 
}

//---------------------
// EnumMixed on efg
//---------------------

#include "enumsub.h"
#define ENUM_PRM_INST
#include "enumprm.h"

class EnumBySubgameG : public EnumBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  EnumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		 const EnumParams &p_params, int p_max = 0,
		 EfgShowInterface *p_parent = 0)
    : EnumBySubgame(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    EnumBySubgameG M(ef, sup, P, EPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgEnumG::SolveSetup(void) const
{
  EnumSolveParamsDialog ESPD(parent->Frame(), true); 
}

//---------------------
// Lp on nfg
//---------------------

#include "csumsub.h"
#include "efgcsum.h"
#define CSUM_PRM_INST
#include "csumprm.h"

class ZSumBySubgameG : public ZSumBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  ZSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		 const ZSumParams &p_params, int p_max = 0,
		 EfgShowInterface *p_parent = 0)
    : ZSumBySubgame(p_support, p_params, p_max), 
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
  wxStatus status(parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams P;
  LPPS.GetParams(&P);

  try {
    ZSumBySubgameG M(ef, sup, P, LPPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgZSumG::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(parent->Frame(), true); 
}

//---------------------
// Lp on efg
//---------------------

class EfgCSumBySubgameG : public CSSeqFormBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_number, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_number, p_efg, p_solutions); }

public:
  EfgCSumBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const CSSeqFormParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : CSSeqFormBySubgame(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
  LPPS.GetParams(&P);
 
  try {
    EfgCSumBySubgameG M(ef, sup, P, LPPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgCSumG::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(parent->Frame(), true);
}

//---------------------
// Simpdiv on nfg
//---------------------

#include "simpsub.h"
#define SIMP_PRM_INST
#include "simpprm.h"

class SimpdivBySubgameG : public SimpdivBySubgame, public BaseBySubgameG {
protected:
  void SelectSolutions(int p_subgame, const Efg &p_efg,
		       gList<BehavSolution> &p_solutions)
    { BaseSelectSolutions(p_subgame, p_efg, p_solutions); }
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  SimpdivBySubgameG(const Efg &p_efg, const EFSupport &p_support,
		    const SimpdivParams &p_params, int p_max = 0,
		    EfgShowInterface *p_parent = 0)
    : SimpdivBySubgame(p_support, p_params, p_max),
      BaseBySubgameG(p_parent, p_efg)
    { Solve(); }
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
    SimpdivBySubgameG M(ef, sup, P, SPS.MaxSolns(), parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void EfgSimpdivG::SolveSetup(void) const
{
  SimpdivSolveParamsDialog SDPD(parent->Frame(), true); 
}

//---------------------
// Gobit on nfg
//---------------------

//
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
//

#include "egobit.h"
#include "ngobit.h"
#define GOBIT_PRM_INST
#include "gobitprm.h"

class NGobitBySubgameG : public BaseBySubgameG {
private:
  gList<BehavSolution> m_solutions;

protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  NGobitBySubgameG(const Efg &, EfgShowInterface * = 0);

  gList<BehavSolution> GetSolutions(void) const { return m_solutions; }
};

NGobitBySubgameG::NGobitBySubgameG(const Efg &p_efg,
				   EfgShowInterface *p_parent /*= 0*/)
  : BaseBySubgameG(p_parent, p_efg)
{
  GobitParamsSettings GSPD(m_parent->Filename());
  wxStatus status(m_parent->Frame(), "Gobit Algorithm");
  NFGobitParams P(status);
  GSPD.GetParams(&P);

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
    Gobit(*N, P, startm, nfg_solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  GSPD.RunPxi();

  for (int i = 1; i <= nfg_solns.Length(); i++) {
    MixedToBehav(*N, nfg_solns[i], p_efg, startb);
    m_solutions.Append(BehavSolution(startb, EfgAlg_GOBIT));
  }

  delete N;
  delete S;
}

EfgNGobitG::EfgNGobitG(const Efg &p_efg, const EFSupport &p_support, 
		       EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgNGobitG::Solve(void) const
{
  // exception handler is located in the ctor
  NGobitBySubgameG M(ef, parent);
  return M.GetSolutions();
}

void EfgNGobitG::SolveSetup(void) const
{
  GobitSolveParamsDialog GSPD(parent->Frame(), parent->Filename());
}

//---------------------
// Gobit on nfg
//---------------------

EfgEGobitG::EfgEGobitG(const Efg &p_efg, const EFSupport &p_support, 
		       EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgEGobitG::Solve(void) const
{
  GobitParamsSettings GSPD(parent->Filename());
  wxStatus status(parent->Frame(), "Gobit Algorithm");
  BehavProfile<gNumber> start = parent->CreateStartProfile(GSPD.StartOption());
  EFGobitParams P(status);
  GSPD.GetParams(&P);
  long nevals, nits;
  gList<BehavSolution> solns;

  try {
    Gobit(ef, P, start, solns, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (!solns[1].IsSequential()) {
    wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		 "Returning last value.\n");
  }

  GSPD.RunPxi();
  return solns;
}

void EfgEGobitG::SolveSetup(void) const
{ 
  GobitSolveParamsDialog GSPD(parent->Frame(), parent->Filename()); 
}

//---------------------
// Grid on nfg
//---------------------

#define GRID_PRM_INST
#include "grid.h"
#include "gridprm.h"

class GobitAllBySubgameG : public BaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  GobitAllBySubgameG(const Efg &, const EFSupport &, EfgShowInterface *);

};

GobitAllBySubgameG::GobitAllBySubgameG(const Efg &p_efg, 
				       const EFSupport &p_support, 
				       EfgShowInterface *p_parent)
  : BaseBySubgameG(p_parent, p_efg)
{
  GridParamsSettings GSPD(m_parent->Filename());
  wxStatus status(m_parent->Frame(), "GobitAll Solve");
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

EfgGobitAllG::EfgGobitAllG(const Efg &p_efg, const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolution(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> EfgGobitAllG::Solve(void) const
{
  GobitAllBySubgameG M(ef, sup, parent);
  return solns;
}

void EfgGobitAllG::SolveSetup(void) const
{
  GridSolveParamsDialog GSPD(parent->Frame(), parent->Filename()); 
}
