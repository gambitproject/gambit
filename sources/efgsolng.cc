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

//------------------------------------------------------------------------
//                    guiEfgSolutionG: Member functions
//------------------------------------------------------------------------

guiEfgSolutionG::guiEfgSolutionG(const Efg &p_efg, const EFSupport &p_support,
				 EfgShowInterface *p_parent)
  : m_efg(p_efg), m_support(p_support), m_parent(p_parent)
{ }

//------------------------------------------------------------------------
//                   guiBaseBySubgameG: Member functions
//------------------------------------------------------------------------

#define SELECT_SUBGAME_NUM  10000
void MarkedSubgameRoots(const Efg &efg, gList<Node *> &list); // in efgutils.cc

class guiBaseBySubgameG {
protected:
  EfgShowInterface *m_parent;
  Bool m_pickSolution;
  gList<Node *> m_subgameRoots;
  void SelectSolutions(int, const Efg &, gList<BehavSolution> &p_solutions);
  void BaseViewNormal(const Nfg &, NFSupport *&);

public:
  guiBaseBySubgameG(EfgShowInterface *, const Efg &);
  virtual ~guiBaseBySubgameG() { }
};

guiBaseBySubgameG::guiBaseBySubgameG(EfgShowInterface *p_parent,
				     const Efg &p_efg)
  : m_parent(p_parent)
{
  MarkedSubgameRoots(p_efg, m_subgameRoots);
  wxGetResource(SOLN_SECT, "Efg-Interactive-Solns", &m_pickSolution,
		"gambit.ini");
}


// Pick solutions to go on with, if so requested
void guiBaseBySubgameG::SelectSolutions(int p_subgame, const Efg &p_efg,
					gList<BehavSolution> &p_solutions)
{
  if (!m_pickSolution) 
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
NFSupport *ComputeDominated(const Nfg &, NFSupport &S, bool strong,
                            const gArray<int> &players, gOutput &tracefile, 
			    gStatus &gstatus); // in nfdom.cc

#include "elimdomd.h"
#include "nfstrat.h"

void guiBaseBySubgameG::BaseViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
{
  DominanceSettings DS;

  if (!DS.UseElimDom()) 
    return;

  gArray<int> players(p_nfg.NumPlayers());

  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) 
    players[pl] = pl;

  NFSupport *thisSupport = p_support, *nextSupport = 0;

  if (DS.FindAll()) {
    while ((nextSupport = ComputeDominated(thisSupport->Game(), 
					   *thisSupport, DS.DomStrong(), 
					   players, gnull, gstatus))) {
      delete thisSupport;
      thisSupport = nextSupport;
    }

    p_support = thisSupport;
  }
  else {
    if ((nextSupport = ComputeDominated(thisSupport->Game(), 
					*thisSupport, DS.DomStrong(), 
					players, gnull, gstatus))) {
      p_support = nextSupport;
    }
  }
}

//=========================================================================
//                 Implementations for specific algorithms
//=========================================================================

//-------------------------------------------------------------------------
//                                Liapunov
//-------------------------------------------------------------------------

#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#define LIAP_PRM_INST
#include "liapprm.h"

//--------------------------------------
// Liapunov on extensive form natively
//--------------------------------------

class guiEFLiapBySubgame : public EFLiapBySubgame, public guiBaseBySubgameG {
public:
  guiEFLiapBySubgame(const Efg &p_efg, const EFLiapParams &p_params,
		      const BehavSolution &p_solution, int p_max = 0,
		      EfgShowInterface *p_parent = 0)
    : EFLiapBySubgame(p_efg, p_params, BehavProfile<gNumber>(p_solution),
		      p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiEFLiapBySubgame() { }
};

guiEfgLiap::guiEfgLiap(const Efg &p_efg, const EFSupport &p_support,
		       EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgLiap::Solve(void) const
{
  LiapParamsSettings LPS;
  wxStatus status(m_parent->Frame(), "Liap Algorithm");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(LPS.StartOption());
  EFLiapParams P(status);
  LPS.GetParams(&P);
  try {
    guiEFLiapBySubgame M(m_efg, P, start, LPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgLiap::SolveSetup(void) const
{
  LiapSolveParamsDialog LSPD(m_parent->Frame(), true);
}

//-----------------------
// Liap on efg via nfg
//-----------------------

class guiNFLiapBySubgame : public NFLiapBySubgame, public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiNFLiapBySubgame(const Efg &p_efg, const NFLiapParams &p_params,
                     const BehavSolution &p_solution, int p_max = 0, 
		     EfgShowInterface *p_parent = 0)
    : NFLiapBySubgame(p_efg, p_params, BehavProfile<gNumber>(p_solution),
		      p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiNFLiapBySubgame() { }
};


guiEfgNfgLiap::guiEfgNfgLiap(const Efg &p_efg, const EFSupport &p_support, 
			     EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgNfgLiap::Solve(void) const
{
  LiapParamsSettings LPS;
  wxStatus status(m_parent->Frame(), "Liap Algorithm");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(LPS.StartOption());
  NFLiapParams P(status);
  LPS.GetParams(&P);
  try {
    guiNFLiapBySubgame M(m_efg, P, start, LPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgNfgLiap::SolveSetup(void) const
{ 
  LiapSolveParamsDialog LSPD(m_parent->Frame(), true);
}

 //-------------------------------------------------------------------------
 //                    Linear Complementaity Program
 //-------------------------------------------------------------------------

 #include "seqform.h"
 #define SEQF_PRM_INST
 #include "seqfprm.h"

 //------------------------
 // LCP via sequence form
 //------------------------

 class guiSeqFormBySubgame : public SeqFormBySubgame, public guiBaseBySubgameG {
 public:
   guiSeqFormBySubgame(const Efg &p_efg, const EFSupport &p_support,
		       const SeqFormParams &p_params,
		       int p_max = 0, EfgShowInterface *p_parent = 0)
     : SeqFormBySubgame(p_support, p_params, p_max),
       guiBaseBySubgameG(p_parent, p_efg)
     { Solve(); }
   virtual ~guiSeqFormBySubgame() { }
 };

 guiEfgSeqForm::guiEfgSeqForm(const Efg &p_efg, const EFSupport &p_support, 
			      EfgShowInterface *p_parent)
   : guiEfgSolutionG(p_efg, p_support, p_parent)
 { }

 gList<BehavSolution> guiEfgSeqForm::Solve(void) const
 {
   wxStatus status(m_parent->Frame(), "LCP Algorithm");
   SeqFormParamsSettings SFPS;
   SeqFormParams P(status);
   SFPS.GetParams(P);
   try {
     guiSeqFormBySubgame M(m_efg, m_support, P, SFPS.MaxSolns(), m_parent);
     return M.GetSolutions();
   }
   catch (gSignalBreak &) {
     return gList<BehavSolution>();
   }
 }

 void guiEfgSeqForm::SolveSetup(void) const
 { 
   SeqFormParamsDialog SFPD(m_parent->Frame(), true);
 }

 //----------------------
 // LCP via normal form
 //----------------------

 #include "lemkesub.h"
 #define LEMKE_PRM_INST
 #include "lemkeprm.h"

 class guiLemkeBySubgame : public LemkeBySubgame, public guiBaseBySubgameG {
 protected:
   void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
     { BaseViewNormal(p_nfg, p_support); }

 public:
   guiLemkeBySubgame(const Efg &p_efg, const EFSupport &p_support,
		     const LemkeParams &p_params,
		     int p_max = 0, EfgShowInterface *p_parent = 0)
     : LemkeBySubgame(p_support, p_params, p_max),
       guiBaseBySubgameG(p_parent, p_efg)
     { Solve(); }
};

guiEfgLemke::guiEfgLemke(const Efg &p_efg, const EFSupport &p_support, 
			 EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgLemke::Solve(void) const
{
  wxStatus status(m_parent->Frame(), "LCP Algorithm");

  if (m_efg.NumPlayers() != 2) {
    wxMessageBox("LCP algorithm only works on 2 player games.", 
		 "Algorithm Error");
    return m_solutions;
  }

  LemkeParamsSettings LPS;
  LemkeParams P(status);
  LPS.GetParams(P);
  try {
    guiLemkeBySubgame M(m_efg, m_support, P, LPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgLemke::SolveSetup(void) const
{
  LemkeSolveParamsDialog LSPD(m_parent->Frame(), true);
}

//-------------------------------------------------------------------------
//                    Enumeration (of pure strategies)
//-------------------------------------------------------------------------

//----------------------------
// Enumerate via normal form
//----------------------------

#include "psnesub.h"
#define PUREN_PRM_INST
#include "purenprm.h"

class guiNfgEnumPureBySubgame : public PureNashBySubgame, 
				public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiNfgEnumPureBySubgame(const Efg &p_efg, const EFSupport &p_support,
			  gStatus &p_status, int p_max = 0,
			  EfgShowInterface *p_parent = 0)
    : PureNashBySubgame(p_support, p_status, p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiNfgEnumPureBySubgame() { }
};

guiEfgNfgPureNash::guiEfgNfgPureNash(const Efg &p_efg,
				     const EFSupport &p_support, 
				     EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgNfgPureNash::Solve(void) const
{
  PureNashParamsSettings PNPS;
  wxStatus status(m_parent->Frame(), "EnumPure Algorithm");
  try {
    guiNfgEnumPureBySubgame M(m_efg, m_support, status, PNPS.MaxSolns(),
			      m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgNfgPureNash::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(m_parent->Frame(), true);
}


//---------------------------------
// Enumeration via extensive form
//---------------------------------

#include "efgpure.h"

class guiEfgEnumPureBySubgame : public EfgPSNEBySubgame,
				public guiBaseBySubgameG {
public:
  guiEfgEnumPureBySubgame(const Efg &p_efg, const EFSupport &p_support,
			  int p_max = 0, EfgShowInterface *p_parent = 0)
    : EfgPSNEBySubgame(p_support, p_max), guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiEfgEnumPureBySubgame() { }
};


guiEfgPureNash::guiEfgPureNash(const Efg &p_efg,
				     const EFSupport &p_support, 
				     EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgPureNash::Solve(void) const
{
  PureNashParamsSettings PNPS;
  wxStatus status(m_parent->Frame(), "Efg PureNash");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  try {
    guiEfgEnumPureBySubgame M(m_efg, m_support, PNPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgPureNash::SolveSetup(void) const
{
  PureNashSolveParamsDialog PNPD(m_parent->Frame(), true); 
}


//-------------------------------------------------------------------------
//                    Enumeration (of mixed strategies)
//-------------------------------------------------------------------------

#include "enumsub.h"
#define ENUM_PRM_INST
#include "enumprm.h"

class guiEnumBySubgame : public EnumBySubgame, public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiEnumBySubgame(const Efg &p_efg, const EFSupport &p_support,
		   const EnumParams &p_params,
                   int p_max = 0, EfgShowInterface *p_parent = 0)
    : EnumBySubgame(p_efg, p_params, p_max), guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiEnumBySubgame() { }
};


guiEfgEnum::guiEfgEnum(const Efg &p_efg, const EFSupport &p_support,
		       EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgEnum::Solve(void) const
{
  EnumParamsSettings EPS;
  wxEnumStatus status(m_parent->Frame());
  EnumParams P(status);
  EPS.GetParams(P);
  try {
    guiEnumBySubgame M(m_efg, m_support, P, EPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgEnum::SolveSetup(void) const
{ 
  EnumSolveParamsDialog ESPD(m_parent->Frame(), true);
}

//------------------------------------------------------------------------
//                            Linear program
//------------------------------------------------------------------------

#include "csumsub.h"
#include "efgcsum.h"
#define CSUM_PRM_INST
#include "csumprm.h"

class guiZSumBySubgame : public ZSumBySubgame, public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiZSumBySubgame(const Efg &p_efg, const EFSupport &p_support,
		   const ZSumParams &p_params,
		   int p_max = 0, EfgShowInterface *p_parent = 0)
    : ZSumBySubgame(p_support, p_params, p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiZSumBySubgame() { }
};

guiEfgZSum::guiEfgZSum(const Efg &p_efg, const EFSupport &p_support,
		       EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgZSum::Solve(void) const
{
  if (m_efg.NumPlayers() > 2 || !m_efg.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return m_solutions;
  }

  LPParamsSettings LPPS;
  wxStatus status(m_parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  ZSumParams P;
  LPPS.GetParams(&P);
  try {
    guiZSumBySubgame M(m_efg, m_support, P, LPPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgZSum::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(m_parent->Frame(), true);
}


class guiEfgCSumBySubgame : public CSSeqFormBySubgame,
			    public guiBaseBySubgameG {
public:
  guiEfgCSumBySubgame(const Efg &p_efg, const EFSupport &p_support,
		      const CSSeqFormParams &p_params,
                      int p_max = 0, EfgShowInterface *p_parent = 0)
    : CSSeqFormBySubgame(p_support, p_params, p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiEfgCSumBySubgame() { }
};

guiEfgCSum::guiEfgCSum(const Efg &p_efg, const EFSupport &p_support,
		       EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgCSum::Solve(void) const
{
  if (m_efg.NumPlayers() > 2 || !m_efg.IsConstSum()) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return m_solutions;
  }

  LPParamsSettings LPPS;
  wxStatus status(m_parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";
  CSSeqFormParams P(status);
  LPPS.GetParams(&P);
  try {
    guiEfgCSumBySubgame M(m_efg, m_support, P, LPPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgCSum::SolveSetup(void) const
{
  LPSolveParamsDialog ZSPD(m_parent->Frame(), true);
}

//------------------------------------------------------------------------
//                        Simplicial Subdivision
//------------------------------------------------------------------------

#include "simpsub.h"
#define SIMP_PRM_INST
#include "simpprm.h"

class guiSimpdivBySubgame : public SimpdivBySubgame,
			    public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiSimpdivBySubgame(const Efg &p_efg, const EFSupport &p_support,
		      const SimpdivParams &p_params,
		      int p_max = 0, EfgShowInterface *p_parent = 0)
    : SimpdivBySubgame(p_support, p_params, p_max),
      guiBaseBySubgameG(p_parent, p_efg)
    { Solve(); }
  virtual ~guiSimpdivBySubgame() { }
};


guiEfgSimpdiv::guiEfgSimpdiv(const Efg &p_efg, const EFSupport &p_support, 
			     EfgShowInterface *p_parent):
  guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgSimpdiv::Solve(void) const
{
  SimpdivParamsSettings SPS;
  wxStatus status(m_parent->Frame(), "Simpdiv Algorithm");
  SimpdivParams P(status);
  SPS.GetParams(P);
  try {
    guiSimpdivBySubgame M(m_efg, m_support, P, SPS.MaxSolns(), m_parent);
    return M.GetSolutions();
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgSimpdiv::SolveSetup(void) const
{
  SimpdivSolveParamsDialog SDPD(m_parent->Frame(), true);
}

//-------------------------------------------------------------------------
//                                Gobit
//-------------------------------------------------------------------------

#include "egobit.h"
#include "ngobit.h"
#define GOBIT_PRM_INST
#include "gobitprm.h"

class guiNGobitBySubgame : public guiBaseBySubgameG {
private:
  gList<BehavSolution> m_solutions;

protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiNGobitBySubgame(const Efg &, EfgShowInterface * = 0);
  virtual ~guiNGobitBySubgame() { }

  gList<BehavSolution> GetSolutions(void) const { return m_solutions; }
};

guiNGobitBySubgame::guiNGobitBySubgame(const Efg &p_efg,
				       EfgShowInterface *p_parent)
  : guiBaseBySubgameG(p_parent, p_efg)
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
  
  BehavToMixed(startb, startm);
	
  long nevals, nits;
  gList<MixedSolution> nfg_solns;
  try {
    Gobit(*N, P, startm, nfg_solns, nevals, nits);
    GSPD.RunPxi();
  
    for (int i = 1; i <= nfg_solns.Length(); i++) {
      MixedToBehav(nfg_solns[i], startb);
      m_solutions.Append(BehavSolution(startb, EfgAlg_GOBIT));
    }

    delete N;
    delete S;
  }
  catch (gSignalBreak &) {
    delete N;
    delete S;
  }
}

guiEfgNGobit::guiEfgNGobit(const Efg &p_efg, const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgNGobit::Solve(void) const
{
  guiNGobitBySubgame M(m_efg, m_parent);
  return M.GetSolutions();
}

void guiEfgNGobit::SolveSetup(void) const
{
  GobitSolveParamsDialog GSPD(m_parent->Frame(), m_parent->Filename());
}


guiEfgEGobit::guiEfgEGobit(const Efg &p_efg, const EFSupport &p_support, 
			   EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgEGobit::Solve(void) const
{
  GobitParamsSettings GSPD(m_parent->Filename());
  wxStatus status(m_parent->Frame(), "Gobit Algorithm");
  BehavProfile<gNumber> start = m_parent->CreateStartProfile(GSPD.StartOption());
  EFGobitParams P(status);
  GSPD.GetParams(&P);
  long nevals, nits;
  gList<BehavSolution > solns;
  try {
    Gobit(m_efg, P, start, solns, nevals, nits);
  
    if (!solns[1].IsSequential()) {
      wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\n"
		   "Returning last value.\n");
    }
    
    GSPD.RunPxi();
    return solns;
  }
  catch (gSignalBreak &) {
    return gList<BehavSolution>();
  }
}

void guiEfgEGobit::SolveSetup(void) const
{ 
  GobitSolveParamsDialog GSPD(m_parent->Frame(), m_parent->Filename());
}

#define GRID_PRM_INST
#include "grid.h"
#include "gridprm.h"

class guiGobitAllBySubgame : public guiBaseBySubgameG {
protected:
  void ViewNormal(const Nfg &p_nfg, NFSupport *&p_support)
    { BaseViewNormal(p_nfg, p_support); }

public:
  guiGobitAllBySubgame(const Efg &, const EFSupport &, EfgShowInterface *);
  virtual ~guiGobitAllBySubgame() { }
};

guiGobitAllBySubgame::guiGobitAllBySubgame(const Efg &p_efg,
					   const EFSupport &p_support,
					   EfgShowInterface *p_parent)
  : guiBaseBySubgameG(p_parent, p_efg)
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

    GSPD.RunPxi();
  }
  catch (gSignalBreak &) { }

  delete N;
  delete S;
}


guiEfgGobitAll::guiEfgGobitAll(const Efg &p_efg, const EFSupport &p_support, 
			       EfgShowInterface *p_parent)
  : guiEfgSolutionG(p_efg, p_support, p_parent)
{ }

gList<BehavSolution> guiEfgGobitAll::Solve(void) const
{
  guiGobitAllBySubgame M(m_efg, m_support, m_parent);
  return m_solutions;
}

void guiEfgGobitAll::SolveSetup(void) const
{ 
  GridSolveParamsDialog GSPD(m_parent->Frame(), m_parent->Filename());
}
