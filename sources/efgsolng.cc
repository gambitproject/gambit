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

BehavSolutionT BehavSolution2Num(const BehavSolution<double> &x)
{
BehavSolutionT y(x.Game(),x.Support());
y.SetCreator(x.Creator());
y.SetIsNash(x.IsNash());
y.SetIsSubgamePerfect(x.IsSubgamePerfect());
y.SetIsSequential(x.IsSequential());
y.SetEpsilon(x.Epsilon());
y.SetGobit(x.GobitLambda(),x.GobitValue());
y.SetLiap(x.LiapValue());
y.SetId(x.Id());
gVector<gNumber> &yy=(gVector<gNumber> &)y;
gVector<double> &xx=(gVector<double> &)x;
for (int i=1;i<=yy.Length();i++) yy[i]=xx[i];
return y;
}

gList<BehavSolutionT> BehavSolution2NumL(const gList<BehavSolution<double> > &x)
{
gList<BehavSolutionT> y;
for (int i=1;i<=x.Length();i++) y.Append(BehavSolution2Num(x[i]));
return y;
}

BehavProfile<double> BehavProfile2Dbl(const BehavProfileT &x)
{
BehavProfile<double> y(x.Game(),x.Support());
gVector<double> &yy=(gVector<double> &)y;
gVector<gNumber> &xx=(gVector<gNumber> &)x;
for (int i=1;i<=yy.Length();i++) yy[i]=xx[i];
return y;
}




/************************** EXTENSIVE SOLUTION G ***************************/
ExtensiveSolutionG::ExtensiveSolutionG(const Efg &E,const EFSupport &S,EfgShowInterface *parent_)
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
	void BaseSelectSolutions(int subg_num,const Efg &ef,gList<BehavSolutionT > &solns);
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

void BaseBySubgameG::BaseSelectSolutions(int subg_num,const Efg &ef,gList<BehavSolutionT > &solns)
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
NFSupport *ComputeDominated(const Nfg &, NFSupport &S, bool strong,
								const gArray<int> &players, gOutput &tracefile,gStatus &gstatus); // in nfdom.cc
#include "elimdomd.h"
#include "nfstrat.h"
void BaseBySubgameG::BaseViewNormal(const Nfg &N, NFSupport *&sup)
{
DominanceSettings DS;
if (!DS.UseElimDom()) return;

gArray<int> players(N.NumPlayers());
for (int i=1;i<=N.NumPlayers();i++) players[i]=i;
NFSupport *temp_sup=sup,*temp_sup1=0;
if (DS.FindAll())
{
	while ((temp_sup=ComputeDominated(temp_sup->Game(),*temp_sup,DS.DomStrong(),players,gnull,gstatus)))
		{if (temp_sup1) delete temp_sup1; temp_sup1=temp_sup;}
	if (temp_sup1) sup=temp_sup1;
}
else
{
	if ((temp_sup=ComputeDominated(temp_sup->Game(),*temp_sup,DS.DomStrong(),players,gnull,gstatus)))
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
void SelectSolutions(int n,const Efg &ef,gList<BehavSolutionT > &solns)
{BaseSelectSolutions(n,ef,solns);}
public:
EFLiapBySubgameG(const Efg &E,const EFLiapParams &p,
			const BehavSolution<double> &s, int max = 0,EfgShowInterface *parent_=0)
			:EFLiapBySubgame(E,p,s,max),BaseBySubgameG(parent_,E)
{Solve();}
};

EFLiapG::EFLiapG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }

gList<BehavSolutionT > EFLiapG::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfileT start1=parent->CreateStartProfile(LPS.StartOption());
BehavProfile<double> start=BehavProfile2Dbl(start1);
EFLiapParams P(status);
LPS.GetParams(&P);
EFLiapBySubgameG M(ef,P,start,LPS.MaxSolns(),parent);
return BehavSolution2NumL(M.GetSolutions());
}

void EFLiapG::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

/*
// Normal Form Liap... note that it is not implemented for gRationals
class NFLiapBySubgameG:
						public NFLiapBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
NFLiapBySubgameG(const Efg &E, const NFLiapParams &p,
			const BehavSolution &s, int max = 0,EfgShowInterface *parent_=0)
			:NFLiapBySubgame(E,p,s,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class NFLiapBySubgameG<double>;


NFLiapG::NFLiapG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > NFLiapG::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfile start=parent->CreateStartProfile(LPS.StartOption());;
NFLiapParams P(status);
LPS.GetParams(&P);
NFLiapBySubgameG M(ef,P,start,LPS.MaxSolns(),parent);
return M.GetSolutions();
}


void NFLiapG::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

template class NFLiapG<double>;

NFLiapG<gRational>::NFLiapG(const Efg<gRational> &E,const EFSupport &S,EfgShowInterface<gRational> *parent):ExtensiveSolutionG<gRational>(E,S,parent)
{ }
void NFLiapG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavSolution<gRational> > NFLiapG<gRational>::Solve(void) const
{doubles_only();return empty_behav_list_r;}

template class NFLiapG<gRational>;

// SeqForm
#include "seqform.h"
#define SEQF_PRM_INST
#include "seqfprm.h"
class SeqFormBySubgameG:
													public SeqFormBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
SeqFormBySubgameG(const Efg &E, const EFSupport &S,const SeqFormParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													SeqFormBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class SeqFormBySubgameG<double>;
template class SeqFormBySubgameG<gRational>;


SeqFormG::SeqFormG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > SeqFormG::Solve(void) const
{
wxStatus status(parent->Frame(),"LCP Algorithm");
SeqFormParamsSettings SFPS;
SeqFormParams P(status);
SFPS.GetParams(P);
SeqFormBySubgameG M(ef,sup,P,SFPS.MaxSolns(),parent);
return M.GetSolutions();
}


void SeqFormG::SolveSetup(void) const
{SeqFormParamsDialog SFPD(parent->Frame(),true);}

template class SeqFormG<double>;
template class SeqFormG<gRational>;

// Lemke
#include "lemkesub.h"
#define LEMKE_PRM_INST
#include "lemkeprm.h"
class LemkeBySubgameG:
													public LemkeBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
LemkeBySubgameG(const Efg &E,const EFSupport &S, const LemkeParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													LemkeBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class LemkeBySubgameG<double>;
template class LemkeBySubgameG<gRational>;


LemkeG::LemkeG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > LemkeG::Solve(void) const
{
wxStatus status(parent->Frame(),"LCP Algorithm");
if (ef.NumPlayers()!=2)
{
	wxMessageBox("LCP algorithm only works on 2 player games.","Algorithm Error");
	return EmptyBehavList((T)1);
}

LemkeParamsSettings LPS;
LemkeParams P(status);
LPS.GetParams(P);
LemkeBySubgameG M(ef,sup,P,LPS.MaxSolns(),parent);
return M.GetSolutions();
}


void LemkeG::SolveSetup(void) const
{LemkeSolveParamsDialog LSPD(parent->Frame(),true);}

template class LemkeG<double>;
template class LemkeG<gRational>;


// Pure Nash
#include "psnesub.h"
#define PUREN_PRM_INST
#include "purenprm.h"
class PureNashBySubgameG:
													public PureNashBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
PureNashBySubgameG(const Efg &E,const EFSupport &S,
													int max = 0,EfgShowInterface *parent_=0):
													PureNashBySubgame(E,S,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class PureNashBySubgameG<double>;
template class PureNashBySubgameG<gRational>;


PureNashG::PureNashG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > PureNashG::Solve(void) const
{
PureNashParamsSettings PNPS;
wxStatus status(parent->Frame(),"EnumPure Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
PureNashBySubgameG M(ef,sup,PNPS.MaxSolns(),parent);
return M.GetSolutions();
}


void PureNashG::SolveSetup(void) const
{PureNashSolveParamsDialog PNPD(parent->Frame(),true);}

template class PureNashG<double>;
template class PureNashG<gRational>;

// Efg Pure Nash
#include "efgpure.h"
class EPureNashBySubgameG:
													public EfgPSNEBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
EPureNashBySubgameG(const Efg &E,const EFSupport &S,
													int max = 0,EfgShowInterface *parent_=0):
													EfgPSNEBySubgame(E,S,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class EPureNashBySubgameG<double>;
template class EPureNashBySubgameG<gRational>;


EPureNashG::EPureNashG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > EPureNashG::Solve(void) const
{
PureNashParamsSettings PNPS;
wxStatus status(parent->Frame(),"Efg PureNash");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
EPureNashBySubgameG M(ef,sup,PNPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EPureNashG::SolveSetup(void) const
{PureNashSolveParamsDialog PNPD(parent->Frame(),true);}

template class EPureNashG<double>;
template class EPureNashG<gRational>;

// Enum Mixed
#include "enumsub.h"
#define ENUM_PRM_INST
#include "enumprm.h"
class EnumBySubgameG:
													public EnumBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
EnumBySubgameG(const Efg &E,const EFSupport &S,const EnumParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													EnumBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class EnumBySubgameG<double>;
template class EnumBySubgameG<gRational>;



EnumG::EnumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > EnumG::Solve(void) const
{
EnumParamsSettings EPS;
wxEnumStatus status(parent->Frame());
EnumParams P(status);
EPS.GetParams(P);
EnumBySubgameG M(ef,sup,P,EPS.MaxSolns(),parent);
return M.GetSolutions();
}


void EnumG::SolveSetup(void) const
{EnumSolveParamsDialog ESPD(parent->Frame(),true);}

template class EnumG<double>;
template class EnumG<gRational>;

// LP (ZSum)
#include "csumsub.h"
#include "efgcsum.h"
#define CSUM_PRM_INST
#include "csumprm.h"
class ZSumBySubgameG:
													public ZSumBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
ZSumBySubgameG(const Efg &E,const EFSupport &S,const ZSumParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													ZSumBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class ZSumBySubgameG<double>;
template class ZSumBySubgameG<gRational>;



ZSumG::ZSumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > ZSumG::Solve(void) const
{
if (ef.NumPlayers() > 2 || !ef.IsConstSum())
{
	wxMessageBox("Only valid for two-person zero-sum games");
	return EmptyBehavList((T)1);
}

LPParamsSettings LPPS;
wxStatus status(parent->Frame(),"LP Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
ZSumParams P;
LPPS.GetParams(&P);
ZSumBySubgameG M(ef,sup,P,LPPS.MaxSolns(),parent);
return M.GetSolutions();
}


void ZSumG::SolveSetup(void) const
{LPSolveParamsDialog ZSPD(parent->Frame(),true);}

template class ZSumG<double>;
template class ZSumG<gRational>;

// Efg Csum
class EfgCSumBySubgameG:
													public CSSeqFormBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
EfgCSumBySubgameG(const Efg &E,const EFSupport &S,const CSSeqFormParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													CSSeqFormBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class EfgCSumBySubgameG<double>;
template class EfgCSumBySubgameG<gRational>;



EfgCSumG::EfgCSumG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > EfgCSumG::Solve(void) const
{
if (ef.NumPlayers() > 2 || !ef.IsConstSum())
{
	wxMessageBox("Only valid for two-person zero-sum games");
	return EmptyBehavList((T)1);
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

template class EfgCSumG<double>;
template class EfgCSumG<gRational>;

// Simpdiv
#include "simpsub.h"
#define SIMP_PRM_INST
#include "simpprm.h"
class SimpdivBySubgameG:
													public SimpdivBySubgame,public BaseBySubgameG
{
protected:
void SelectSolutions(int n,const Efg &ef,gList<BehavSolution > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
SimpdivBySubgameG(const Efg &E,const EFSupport &S,const SimpdivParams &P,
													int max = 0,EfgShowInterface *parent_=0):
													SimpdivBySubgame(E,S,P,max),BaseBySubgameG(parent_,E)
{Solve();}
};

template class SimpdivBySubgameG<double>;
template class SimpdivBySubgameG<gRational>;


SimpdivG::SimpdivG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }


gList<BehavSolution > SimpdivG::Solve(void) const
{
SimpdivParamsSettings SPS;
wxStatus status(parent->Frame(),"Simpdiv Algorithm");
SimpdivParams P(status);
SPS.GetParams(P);
SimpdivBySubgameG M(ef,sup,P,SPS.MaxSolns(),parent);
return M.GetSolutions();
}


void SimpdivG::SolveSetup(void) const
{SimpdivSolveParamsDialog SDPD(parent->Frame(),true);}

template class SimpdivG<double>;
template class SimpdivG<gRational>;

// NF Gobit
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
// Note that we are defining the gRational and double functions separately
// since Gobit is not meant to be implemented in gRationals at all
#include "egobit.h"
#include "ngobit.h"
#define GOBIT_PRM_INST
#include "gobitprm.h"
class NGobitBySubgameG:
													public BaseBySubgameG
{
private:
	gList<BehavSolution > solns;
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
Nfg *N = MakeReducedNfg((Efg &)E,ES);
NFSupport *S=new NFSupport(N->GameForm());
ViewNormal(*N,S);

MixedProfile start(*N,*S);

long nevals,nits;
gList<MixedSolution<double> > nfg_solns;
Gobit(*N,P,start,nfg_solns,nevals,nits);
GSPD.RunPxi();

for (int i = 1; i <= nfg_solns.Length(); i++)
{
	BehavSolution bp(E);
	MixedToBehav(*N, nfg_solns[i], E, bp);
	solns.Append(bp);
}
delete N;
delete S;
}

gList<BehavSolution > GetSolutions(void) const {return solns;}
};

template class NGobitBySubgameG<double>;


NGobitG<double>::NGobitG(const Efg<double> &E,const EFSupport &S,EfgShowInterface<double> *parent):ExtensiveSolutionG<double>(E,S,parent)
{ }

gList<BehavSolution<double> > NGobitG<double>::Solve(void) const
{
NGobitBySubgameG<double> M(ef,parent);
return M.GetSolutions();
}

void NGobitG<double>::SolveSetup(void) const
{GobitSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}


NGobitG<gRational>::NGobitG(const Efg<gRational> &E,const EFSupport &S,EfgShowInterface<gRational> *parent):ExtensiveSolutionG<gRational>(E,S,parent)
{ }
void NGobitG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavSolution<gRational> > NGobitG<gRational>::Solve(void) const
{doubles_only(); return empty_behav_list_r;}

template class NGobitG<double>;
template class NGobitG<gRational>;

// EF Gobit
// This algorithm does not support solving by subgames.

EGobitG<double>::EGobitG(const Efg<double> &E,const EFSupport &S,EfgShowInterface<double> *parent):ExtensiveSolutionG<double>(E,S,parent)
{ }

gList<BehavSolution<double> > EGobitG<double>::Solve(void) const
{
GobitParamsSettings GSPD(parent->Filename());
wxStatus status(parent->Frame(),"Gobit Algorithm");
BehavProfile<double> start=parent->CreateStartProfile(GSPD.StartOption());;
EFGobitParams P(status);
GSPD.GetParams(&P);
long nevals,nits;
gList<BehavSolution<double> > solns;
Gobit(ef,P,start,solns,nevals,nits);
if (!solns[1].IsSequential())
  wxMessageBox("Warning:  Algorithm did not converge to sequential equilibrium.\nReturning last value.\n");
GSPD.RunPxi();
return solns;
}

void EGobitG<double>::SolveSetup(void) const
{GobitSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}


EGobitG<gRational>::EGobitG(const Efg<gRational> &E,const EFSupport &S,EfgShowInterface<gRational> *parent):ExtensiveSolutionG<gRational>(E,S,parent)
{ }
void EGobitG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavSolution<gRational> > EGobitG<gRational>::Solve(void) const
{doubles_only();return empty_behav_list_r;}

template class EGobitG<double>;
template class EGobitG<gRational>;

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
NFSupport *S=new NFSupport(N->GameForm());
ViewNormal(*N,S);
GridSolveModule M(*N,P,*S);
M.GridSolve();
GSPD.RunPxi();
delete N;
delete S;
}
};

template class GobitAllBySubgameG<double>;



GobitAllG::GobitAllG(const Efg &E,const EFSupport &S,EfgShowInterface *parent):ExtensiveSolutionG(E,S,parent)
{ }



gList<BehavSolution > GobitAllG::Solve(void) const
{
GobitAllBySubgameG M(ef,sup,parent);
return EmptyBehavList((T)1);;
}


void GobitAllG::SolveSetup(void) const
{GridSolveParamsDialog GSPD(parent->Frame(),parent->Filename());}

template class GobitAllG<double>;

GobitAllG<gRational>::GobitAllG(const Efg<gRational> &E,const EFSupport &S,EfgShowInterface<gRational> *parent):ExtensiveSolutionG<gRational>(E,S,parent)
{ }
void GobitAllG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavSolution<gRational> > GobitAllG<gRational>::Solve(void) const
{doubles_only(); return empty_behav_list_r;}

template class GobitAllG<gRational>;
*/
