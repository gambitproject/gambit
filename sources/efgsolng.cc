//
// FILE: efgsolng.cc -- definition of the class dealing with the GUI part of the
// extensive form solutions.
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
#define		SOLN_SECT				"Soln-Defaults"


/************************** EXTENSIVE SOLUTION G ***************************/
EfgSolutionG::EfgSolutionG(const Efg &E,const EFSupport &S,EfgShowInterface *parent_)
												:ef(E),sup(S),parent(parent_)
{ }

/*************************** BY SUBGAME G **********************************/
#define	SELECT_SUBGAME_NUM	10000
void MarkedSubgameRoots(const Efg &efg, gList<Node *> &list); // in efgutils.cc

class BaseBySubgameG
{
protected:
	EfgShowInterface *parent;
	Bool pick_soln;
	gList<Node *> subgame_roots;
	void BaseSelectSolutions(int subg_num,const Efg &ef,gList<BehavSolution> &solns);
	void BaseViewNormal(const Nfg &N, NFSupport *&sup);
public:
	BaseBySubgameG(EfgShowInterface *parent_,const Efg &ef);
};


BaseBySubgameG::BaseBySubgameG(EfgShowInterface *parent_,const Efg &ef)
									:parent(parent_)
{
MarkedSubgameRoots(ef,subgame_roots);
wxGetResource(SOLN_SECT,"Efg-Interactive-Solns",&pick_soln,"gambit.ini");
}


// Pick solutions to go on with, if so requested

void BaseBySubgameG::BaseSelectSolutions(int subg_num,const Efg &ef,gList<BehavSolution> &solns)
{
if (!pick_soln) return;
parent->SetPickSubgame(subgame_roots[subg_num]);

if (solns.Length()==0)
{
	wxMessageBox("No solutions were found for this subgame");
	parent->SetPickSubgame(0);
	return;
}
int num_isets=0;
for (int i=1;i<=ef.NumPlayers();i++)
	num_isets+=ef.Players()[i]->NumInfosets();

if (num_isets)	parent->PickSolutions(ef,solns);

// turn off the subgame picking icon at the last subgame
if (subg_num==subgame_roots.Length()) parent->SetPickSubgame(0);
}

// Eliminated dominanted strats, if so requested
NFSupport *ComputeDominated(const Nfg &, NFSupport &S,
                            const gArray<gNumber> &params, bool strong,
								const gArray<int> &players, gOutput &tracefile,gStatus &gstatus); // in nfdom.cc
#include "elimdomd.h"
#include "nfstrat.h"
void BaseBySubgameG::BaseViewNormal(const Nfg &N, NFSupport *&sup)
{
DominanceSettings DS;
if (!DS.UseElimDom()) return;

gArray<int> players(N.NumPlayers());
for (int i=1;i<=N.NumPlayers();i++) players[i]=i;
gArray<gNumber> values(N.Parameters()->Dmnsn());
for (int i = 1; i <= values.Length(); values[i++] = gNumber(0));
NFSupport *temp_sup=sup,*temp_sup1=0;
if (DS.FindAll())
{
	while ((temp_sup=ComputeDominated(temp_sup->Game(),*temp_sup,values,DS.DomStrong(),players,gnull,gstatus)))
		{if (temp_sup1) delete temp_sup1; temp_sup1=temp_sup;}
	if (temp_sup1) sup=temp_sup1;
}
else
{
	if ((temp_sup=ComputeDominated(temp_sup->Game(),*temp_sup,values,DS.DomStrong(),players,gnull,gstatus)))
		sup=temp_sup;
}
}
/******************************* ALL THE SPECIFIC ALGORITHMS ****************/
#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#define LIAP_PRM_INST
#include "liapprm.h"
// Extensive Form Liap... note that it is not implemented for gRationals
class EFLiapBySubgameG:
						public EFLiapBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{BaseSelectSolutions(n,ef,solns);}
public:
EFLiapBySubgameG(const Efg &E,const EFLiapParams &p,
			const BehavSolution &s, int max = 0,EfgShowInterface *parent_=0)
			:EFLiapBySubgame(E,p,s,max),BaseBySubgameG(parent_,E)
{Solve();}
};

EfgELiapG::EfgELiapG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }

gList<BehavSolution> EfgELiapG::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfile<gNumber> start=parent->CreateStartProfile(LPS.StartOption());
EFLiapParams P(status);
LPS.GetParams(&P);
EFLiapBySubgameG M(ef,P,start,LPS.MaxSolns(),parent);
return M.GetSolutions();
}

void EfgELiapG::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

// Normal Form Liap... note that it is not implemented for gRationals
class NFLiapBySubgameG:
						public NFLiapBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
NFLiapBySubgameG(const Efg &E, const NFLiapParams &p,
			const BehavSolution &s, int max = 0,EfgShowInterface *parent_=0)
			:NFLiapBySubgame(E,p,s,max),BaseBySubgameG(parent_,E)
{Solve();}
};


EfgNLiapG::EfgNLiapG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgNLiapG::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfile<gNumber> start=parent->CreateStartProfile(LPS.StartOption());;
NFLiapParams P(status);
LPS.GetParams(&P);
NFLiapBySubgameG M(ef,P,start,LPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgNLiapG::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

// SeqForm
#include "seqform.imp"
#define SEQF_PRM_INST
#include "seqfprm.h"
class SeqFormBySubgameG:
						     public SeqFormBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
SeqFormBySubgameG(const Efg &E, const EFSupport &S,const SeqFormParams &P,
                  int max = 0,EfgShowInterface *parent_=0):
                  SeqFormBySubgame(S,parent_->Parameters().CurSet(),P,max),
                  BaseBySubgameG(parent_,E)
{Solve();}
};

EfgSeqFormG::EfgSeqFormG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):
             EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgSeqFormG::Solve(void) const
{
wxStatus status(parent->Frame(),"LCP Algorithm");
SeqFormParamsSettings SFPS;
SeqFormParams P(status);
SFPS.GetParams(P);
SeqFormBySubgameG M(ef,sup,P,SFPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgSeqFormG::SolveSetup(void) const
{SeqFormParamsDialog SFPD(parent->Frame(),true);}

// Lemke
#include "lemkesub.imp"
#define LEMKE_PRM_INST
#include "lemkeprm.h"
class LemkeBySubgameG:
							public LemkeBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
LemkeBySubgameG(const Efg &E,const EFSupport &S, const LemkeParams &P,
                int max = 0,EfgShowInterface *parent_=0):
                LemkeBySubgame(S,P,parent_->Parameters().CurSet(),max),
                BaseBySubgameG(parent_,E)
{Solve();}
};

EfgLemkeG::EfgLemkeG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgLemkeG::Solve(void) const
{
wxStatus status(parent->Frame(),"LCP Algorithm");
if (ef.NumPlayers()!=2)
{
	wxMessageBox("LCP algorithm only works on 2 player games.","Algorithm Error");
	return solns;
}

LemkeParamsSettings LPS;
LemkeParams P(status);
LPS.GetParams(P);
LemkeBySubgameG M(ef,sup,P,LPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgLemkeG::SolveSetup(void) const
{LemkeSolveParamsDialog LSPD(parent->Frame(),true);}


// Pure Nash
#include "psnesub.imp"
#define PUREN_PRM_INST
#include "purenprm.h"
class PureNashBySubgameG:
                        public PureNashBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
PureNashBySubgameG(const Efg &E,const EFSupport &S,
                   int max = 0,EfgShowInterface *parent_=0):
                   PureNashBySubgame(S,parent_->Parameters().CurSet(),max),
                   BaseBySubgameG(parent_,E)
{Solve();}
};


EfgPureNashG::EfgPureNashG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):
              EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgPureNashG::Solve(void) const
{
PureNashParamsSettings PNPS;
wxStatus status(parent->Frame(),"EnumPure Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
PureNashBySubgameG M(ef,sup,PNPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgPureNashG::SolveSetup(void) const
{PureNashSolveParamsDialog PNPD(parent->Frame(),true);}


// Efg Pure Nash
#include "efgpure.imp"
class EPureNashBySubgameG:
                         public EfgPSNEBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
EPureNashBySubgameG(const Efg &E,const EFSupport &S,
                    int max = 0,EfgShowInterface *parent_=0):
                    EfgPSNEBySubgame(S,parent_->Parameters().CurSet(),max),
                    BaseBySubgameG(parent_,E)
{Solve();}
};


EfgEPureNashG::EfgEPureNashG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):
               EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgEPureNashG::Solve(void) const
{
PureNashParamsSettings PNPS;
wxStatus status(parent->Frame(),"Efg PureNash");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
EPureNashBySubgameG M(ef,sup,PNPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgEPureNashG::SolveSetup(void) const
{PureNashSolveParamsDialog PNPD(parent->Frame(),true);}

// Enum Mixed
#include "enumsub.imp"
#define ENUM_PRM_INST
#include "enumprm.h"
class EnumBySubgameG:
							public EnumBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
EnumBySubgameG(const Efg &E,const EFSupport &S,const EnumParams &P,
               int max = 0,EfgShowInterface *parent_=0):
               EnumBySubgame(S,parent_->Parameters().CurSet(),P,max),
               BaseBySubgameG(parent_,E)
{Solve();}
};


EfgEnumG::EfgEnumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):
          EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgEnumG::Solve(void) const
{
EnumParamsSettings EPS;
wxEnumStatus status(parent->Frame());
EnumParams P(status);
EPS.GetParams(P);
EnumBySubgameG M(ef,sup,P,EPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgEnumG::SolveSetup(void) const
{EnumSolveParamsDialog ESPD(parent->Frame(),true);}


// LP (ZSum)
#include "csumsub.imp"
#include "efgcsum.imp"
#define CSUM_PRM_INST
#include "csumprm.h"
class ZSumBySubgameG:
							public ZSumBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
ZSumBySubgameG(const Efg &E,const EFSupport &S,const ZSumParams &P,
               int max = 0,EfgShowInterface *parent_=0):
               ZSumBySubgame(S,P,parent_->Parameters().CurSet(),max),
               BaseBySubgameG(parent_,E)
{Solve();}
};


EfgZSumG::EfgZSumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):
          EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgZSumG::Solve(void) const
{
if (ef.NumPlayers() > 2 || !ef.IsConstSum())
{
	wxMessageBox("Only valid for two-person zero-sum games");
	return solns;
}

LPParamsSettings LPPS;
wxStatus status(parent->Frame(),"LP Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
ZSumParams P;
LPPS.GetParams(&P);
ZSumBySubgameG M(ef,sup,P,LPPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgZSumG::SolveSetup(void) const
{LPSolveParamsDialog ZSPD(parent->Frame(),true);}


// Efg Csum
class EfgCSumBySubgameG:
								public CSSeqFormBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
EfgCSumBySubgameG(const Efg &E,const EFSupport &S,const CSSeqFormParams &P,
                  int max = 0,EfgShowInterface *parent_=0):
                  CSSeqFormBySubgame(S,parent_->Parameters().CurSet(),P,max),
                  BaseBySubgameG(parent_,E)
{Solve();}
};



EfgCSumG::EfgCSumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgCSumG::Solve(void) const
{
if (ef.NumPlayers() > 2 || !ef.IsConstSum())
{
	wxMessageBox("Only valid for two-person zero-sum games");
   return solns;
}

LPParamsSettings LPPS;
wxStatus status(parent->Frame(),"LP Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
CSSeqFormParams P(status);
LPPS.GetParams(&P);
EfgCSumBySubgameG M(ef,sup,P,LPPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgCSumG::SolveSetup(void) const
{LPSolveParamsDialog ZSPD(parent->Frame(),true);}

// Simpdiv
#include "simpsub.imp"
#define SIMP_PRM_INST
#include "simpprm.h"
class SimpdivBySubgameG:
								public SimpdivBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution> &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
SimpdivBySubgameG(const Efg &E,const EFSupport &S,const SimpdivParams &P,
                  int max = 0,EfgShowInterface *parent_=0):
                  SimpdivBySubgame(S,parent_->Parameters().CurSet(),P,max),
                  BaseBySubgameG(parent_,E)
{Solve();}
};


EfgSimpdivG::EfgSimpdivG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }


gList<BehavSolution> EfgSimpdivG::Solve(void) const
{
SimpdivParamsSettings SPS;
wxStatus status(parent->Frame(),"Simpdiv Algorithm");
SimpdivParams P(status);
SPS.GetParams(P);
SimpdivBySubgameG M(ef,sup,P,SPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EfgSimpdivG::SolveSetup(void) const
{SimpdivSolveParamsDialog SDPD(parent->Frame(),true);}


// NF Gobit
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
#include "egobit.h"
#include "ngobit.h"
#define GOBIT_PRM_INST
#include "gobitprm.h"
class NGobitBySubgameG:
													public BaseBySubgameG
{
private:
	gList<BehavSolution> solns;
protected:
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
NGobitBySubgameG(const Efg &E,EfgShowInterface *parent_=0):
													BaseBySubgameG(parent_,E)
{
GobitParamsSettings GSPD(parent->Filename());
wxStatus status(parent->Frame(),"Gobit Algorithm");
NFGobitParams P(status);
GSPD.GetParams(&P);

EFSupport ES=EFSupport(E);
Nfg *N = MakeReducedNfg(E,ES);
NFSupport *S=new NFSupport(*N);
ViewNormal(*N,S);

BehavProfile<gNumber> startb=parent->CreateStartProfile(GSPD.StartOption());
MixedProfile<gNumber> startm(*N,parent->Parameters().CurSet());

BehavToMixed(E,startb,*N,startm);


long nevals,nits;
gList<MixedSolution> nfg_solns;
Gobit(*N,P,startm,nfg_solns,nevals,nits);
GSPD.RunPxi();

for (int i = 1; i <= nfg_solns.Length(); i++)
{
	MixedToBehav(*N, nfg_solns[i], E, startb);
	solns.Append(BehavSolution(startb,EfgAlg_GOBIT));
}
delete N;
delete S;
}

gList<BehavSolution> GetSolutions(void) const {return solns;}
};


EfgNGobitG::EfgNGobitG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }

gList<BehavSolution> EfgNGobitG::Solve(void) const
{
NGobitBySubgameG M(ef,parent);
return M.GetSolutions();
}

void EfgNGobitG::SolveSetup(void) const
{GobitSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}


// EF Gobit
// This algorithm does not support solving by subgames.

EfgEGobitG::EfgEGobitG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }

gList<BehavSolution > EfgEGobitG::Solve(void) const
{
GobitParamsSettings GSPD(parent->Filename());
wxStatus status(parent->Frame(),"Gobit Algorithm");
BehavProfile<gNumber> start=parent->CreateStartProfile(GSPD.StartOption());;
EFGobitParams P(status);
GSPD.GetParams(&P);
long nevals,nits;
gList<BehavSolution > solns;
Gobit(ef,P,start,solns,nevals,nits);
if (!solns[1].IsSequential())
  wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\nReturning last value.\n");
GSPD.RunPxi();
return solns;
}

void EfgEGobitG::SolveSetup(void) const
{GobitSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}



// Gobit All
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
// Also note that this algorithm is unique in that it does not return any
// solutions but creates a file.
#define GRID_PRM_INST
#include "grid.h"
#include "gridprm.h"

class GobitAllBySubgameG:
									public BaseBySubgameG
{
protected:
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
GobitAllBySubgameG(const Efg &E,const EFSupport &ES,EfgShowInterface *parent_):
													BaseBySubgameG(parent_,E)
{
GridParamsSettings GSPD(parent->Filename());
wxStatus status(parent->Frame(),"GobitAll Solve");
GridParams P(status);
GSPD.GetParams(P);

Nfg *N = MakeReducedNfg((Efg &)E,ES);
NFSupport *S=new NFSupport(*N);
ViewNormal(*N,S);

gList<MixedSolution> solns;
GridSolve(*S,parent->Parameters().CurSet(),P,solns);

GSPD.RunPxi();
delete N;
delete S;
}
};


EfgGobitAllG::EfgGobitAllG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):EfgSolutionG(E,S,parent)
{ }



gList<BehavSolution> EfgGobitAllG::Solve(void) const
{
GobitAllBySubgameG M(ef,sup,parent);
return solns;
}


void EfgGobitAllG::SolveSetup(void) const
{GridSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}




