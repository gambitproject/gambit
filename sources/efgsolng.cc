// File: efgsolng.cc -- definition of the class dealing with the GUI part of the
// extensive form solutions.
// $Id$

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "wxstatus.h"
#include "subsolve.h"
#include "gfunct.h"
#pragma hdrstop
#include "efgsolng.h"
#include "nfgconst.h"

// sections in the defaults file(s)
#define		SOLN_SECT				"Soln-Defaults"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
// Need this to return a type dependent empty list if no solutions were generated

gList<BehavProfile<double> > empty_behav_list_d;
gList<BehavProfile<double> > &EmptyBehavList(double )
{return empty_behav_list_d;}
gList<BehavProfile<gRational> > empty_behav_list_r;
gList<BehavProfile<gRational> > &EmptyBehavList(gRational )
{return empty_behav_list_r;}

void doubles_only(void)
{wxMessageBox("Due to efficiency reasons, this algorithm is only implemented\nin doubles.\nPlease use doubles if you wish to run it");}

TEMPLATE class ExtensiveShowInterf<double>;
TEMPLATE class ExtensiveShowInterf<gRational>;

/************************** EXTENSIVE SOLUTION G ***************************/
template <class T> ExtensiveSolutionG<T>::ExtensiveSolutionG(const Efg<T> &E,ExtensiveShowInterf<T> *parent_)
												:ef(E),parent(parent_)
{ }

TEMPLATE class ExtensiveSolutionG<double>;
TEMPLATE class  ExtensiveSolutionG<gRational>;

/*************************** BY SUBGAME G **********************************/
#define	SELECT_SUBGAME_NUM	10000
void SubgameRoots(const BaseEfg &efg, gList<Node *> &list); // in efgutils.cc

template <class T> class BaseBySubgameG
{
protected:
	ExtensiveShowInterf<T> *parent;
	Bool pick_soln;
	gList<Node *> subgame_roots;
	void BaseSelectSolutions(int subg_num,const Efg<T> &ef,gList<BehavProfile<T> > &solns);
	void BaseViewNormal(const Nfg<T> &N, NFSupport *&sup);
public:
	BaseBySubgameG(ExtensiveShowInterf<T> *parent_,const BaseEfg &ef);
};

template <class T>
BaseBySubgameG<T>::BaseBySubgameG(ExtensiveShowInterf<T> *parent_,const BaseEfg &ef)
									:parent(parent_)
{
SubgameRoots(ef,subgame_roots);
wxGetResource(SOLN_SECT,"Efg-Interactive-Solns",&pick_soln,"gambit.ini");
}


// Pick solutions to go on with, if so requested
// Note the non-standard use of the OnMenuCommand function of the wxFrame.  We
// want to pass the current subgame # to the parent class, but do not want to
// include the entire extshow.h header.  Thus, ExtensiveShow 's OnMenuCommand
// must process menu id's > SELECT_SUBGAME_NUM appropriately.
template <class T>
void BaseBySubgameG<T>::BaseSelectSolutions(int subg_num,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
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
	num_isets+=ef.PlayerList()[i]->NumInfosets();
if (num_isets) parent->PickSolutions(ef,solns,num_isets);
// turn off the subgame picking icon at the last subgame
if (subg_num==subgame_roots.Length()) parent->SetPickSubgame(0);
}

// Eliminated dominanted strats, if so requested
NFSupport *ComputeDominated(NFSupport &S, bool strong,
								const gArray<int> &players, gOutput &tracefile); // in nfdom.cc
#include "elimdomd.h"
template <class T>
void BaseBySubgameG<T>::BaseViewNormal(const Nfg<T> &N, NFSupport *&sup)
{
DominanceSettings DS;
if (!DS.UseElimDom()) return;
printf("Eliminating dominated strategies\n");
gArray<int> players(N.NumPlayers());
for (int i=1;i<=N.NumPlayers();i++) players[i]=i;
NFSupport *temp_sup=sup,*temp_sup1=0;
if (DS.FindAll())
{
	while ((temp_sup=ComputeDominated(*temp_sup,DS.DomStrong(),players,gnull)))
		{if (temp_sup1) delete temp_sup1; temp_sup1=temp_sup;}
	if (temp_sup1) sup=temp_sup1;
}
else
{
	if ((temp_sup=ComputeDominated(*temp_sup,DS.DomStrong(),players,gnull)))
	 sup=temp_sup;
}
}


TEMPLATE class BaseBySubgameG<double>;
TEMPLATE class BaseBySubgameG<gRational>;

/******************************* ALL THE SPECIFIC ALGORITHMS ****************/
#define LIAP_PRM_INST
#include "liapprm.h"
// Extensive Form Liap
template <class T> class EFLiapBySubgameG:
						public EFLiapBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{BaseSelectSolutions(n,ef,solns);}
public:
EFLiapBySubgameG(const Efg<T> &E, const EFLiapParams &p,
			const BehavProfile<T> &s, int max = 0,ExtensiveShowInterf<T> *parent_=0)
			:EFLiapBySubgame<T>(E,p,s,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class EFLiapBySubgameG<double>;

template <class T>
EFLiapG<T>::EFLiapG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > EFLiapG<T>::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfile<T> start(ef);
start.Centroid();
EFLiapParams P(status);
LPS.GetParams(P);
EFLiapBySubgameG<T> M(ef,P,start,LPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void EFLiapG<T>::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

TEMPLATE class EFLiapG<double>;

EFLiapG<gRational>::EFLiapG(const Efg<gRational> &E,ExtensiveShowInterf<gRational> *parent):ExtensiveSolutionG<gRational>(E,parent)
{ }
void EFLiapG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavProfile<gRational> > EFLiapG<gRational>::Solve(void) const
{doubles_only(); return empty_behav_list_r;}

TEMPLATE class EFLiapG<gRational>;
// Normal Form Liap
template <class T> class NFLiapBySubgameG:
						public NFLiapBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
NFLiapBySubgameG(const Efg<T> &E, const NFLiapParams &p,
			const BehavProfile<T> &s, int max = 0,ExtensiveShowInterf<T> *parent_=0)
			:NFLiapBySubgame<T>(E,p,s,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class NFLiapBySubgameG<double>;

template <class T>
NFLiapG<T>::NFLiapG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > NFLiapG<T>::Solve(void) const
{
LiapParamsSettings LPS;
wxStatus status(parent->Frame(),"Liap Algorithm");
BehavProfile<T> start(ef);
start.Centroid();
NFLiapParams P(status);
LPS.GetParams(P);
NFLiapBySubgameG<T> M(ef,P,start,LPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void NFLiapG<T>::SolveSetup(void) const
{LiapSolveParamsDialog LSPD(parent->Frame(),true);}

TEMPLATE class NFLiapG<double>;

NFLiapG<gRational>::NFLiapG(const Efg<gRational> &E,ExtensiveShowInterf<gRational> *parent):ExtensiveSolutionG<gRational>(E,parent)
{ }
void NFLiapG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavProfile<gRational> > NFLiapG<gRational>::Solve(void) const
{doubles_only();return empty_behav_list_r;}

TEMPLATE class NFLiapG<gRational>;

// SeqForm
#define SEQF_PRM_INST
#include "seqfprm.h"
template <class T> class SeqFormBySubgameG:
													public SeqFormBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
public:
SeqFormBySubgameG(const Efg<T> &E, const SeqFormParams &P,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													SeqFormBySubgame<T>(E,P,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class SeqFormBySubgameG<double>;
TEMPLATE class SeqFormBySubgameG<gRational>;

template <class T>
SeqFormG<T>::SeqFormG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > SeqFormG<T>::Solve(void) const
{
wxStatus status(parent->Frame(),"LCP Algorithm");
SeqFormParamsSettings SFPS;
SeqFormParams P(status);
SFPS.GetParams(P);
SeqFormBySubgameG<T> M(ef,P,SFPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void SeqFormG<T>::SolveSetup(void) const
{SeqFormParamsDialog SFPD(parent->Frame(),true);}

TEMPLATE class SeqFormG<double>;
TEMPLATE class SeqFormG<gRational>;

// Lemke
#define LEMKE_PRM_INST
#include "lemkeprm.h"
template <class T> class LemkeBySubgameG:
													public LemkeBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
LemkeBySubgameG(const Efg<T> &E, const LemkeParams &P,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													LemkeBySubgame<T>(E,P,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class LemkeBySubgameG<double>;
TEMPLATE class LemkeBySubgameG<gRational>;

template <class T>
LemkeG<T>::LemkeG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > LemkeG<T>::Solve(void) const
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
LemkeBySubgameG<T> M(ef,P,LPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void LemkeG<T>::SolveSetup(void) const
{LemkeSolveParamsDialog LSPD(parent->Frame(),true);}

TEMPLATE class LemkeG<double>;
TEMPLATE class LemkeG<gRational>;


// Pure Nash
#define PUREN_PRM_INST
#include "purenprm.h"
template <class T> class PureNashBySubgameG:
													public PureNashBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
PureNashBySubgameG(const Efg<T> &E,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													PureNashBySubgame<T>(E,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class PureNashBySubgameG<double>;
TEMPLATE class PureNashBySubgameG<gRational>;

template <class T>
PureNashG<T>::PureNashG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > PureNashG<T>::Solve(void) const
{
PureNashParamsSettings PNPS;
wxStatus status(parent->Frame(),"EnumPure Algorithm");
status<<"Progress not implemented\n"<<"Cancel button disabled\n";
PureNashBySubgameG<T> M(ef,PNPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void PureNashG<T>::SolveSetup(void) const
{PureNashSolveParamsDialog PNPD(parent->Frame(),true);}

TEMPLATE class PureNashG<double>;
TEMPLATE class PureNashG<gRational>;

// Enum Mixed
#define ENUM_PRM_INST
#include "enumprm.h"
template <class T> class EnumBySubgameG:
													public EnumBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
EnumBySubgameG(const Efg<T> &E,const EnumParams &P,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													EnumBySubgame<T>(E,P,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class EnumBySubgameG<double>;
TEMPLATE class EnumBySubgameG<gRational>;


template <class T>
EnumG<T>::EnumG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > EnumG<T>::Solve(void) const
{
EnumParamsSettings EPS;
wxEnumStatus status(parent->Frame());
EnumParams P(status);
EPS.GetParams(P);
EnumBySubgameG<T> M(ef,P,EPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void EnumG<T>::SolveSetup(void) const
{EnumSolveParamsDialog ESPD(parent->Frame(),true);}

TEMPLATE class EnumG<double>;
TEMPLATE class EnumG<gRational>;

// LP (ZSum)
#define CSUM_PRM_INST
#include "csumprm.h"
template <class T> class ZSumBySubgameG:
													public ZSumBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
ZSumBySubgameG(const Efg<T> &E,const ZSumParams &P,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													ZSumBySubgame<T>(E,P,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class ZSumBySubgameG<double>;
TEMPLATE class ZSumBySubgameG<gRational>;


template <class T>
ZSumG<T>::ZSumG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > ZSumG<T>::Solve(void) const
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
LPPS.GetParams(P);
ZSumBySubgameG<T> M(ef,P,LPPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void ZSumG<T>::SolveSetup(void) const
{LPSolveParamsDialog ZSPD(parent->Frame(),true);}

TEMPLATE class ZSumG<double>;
TEMPLATE class ZSumG<gRational>;

// Simpdiv
#define SIMP_PRM_INST
#include "simpprm.h"
template <class T> class SimpdivBySubgameG:
													public SimpdivBySubgame<T>,public BaseBySubgameG<T>
{
protected:
void SelectSolutions(int n,const Efg<T> &ef,gList<BehavProfile<T> > &solns)
{ BaseSelectSolutions(n,ef,solns);}
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
SimpdivBySubgameG(const Efg<T> &E,const SimpdivParams &P,
													int max = 0,ExtensiveShowInterf<T> *parent_=0):
													SimpdivBySubgame<T>(E,P,max),BaseBySubgameG<T>(parent_,E)
{Solve();}
};

TEMPLATE class SimpdivBySubgameG<double>;
TEMPLATE class SimpdivBySubgameG<gRational>;

template <class T>
SimpdivG<T>::SimpdivG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }

template <class T>
gList<BehavProfile<T> > SimpdivG<T>::Solve(void) const
{
SimpdivParamsSettings SPS;
wxStatus status(parent->Frame(),"Simpdiv Algorithm");
SimpdivParams P(status);
SPS.GetParams(P);
SimpdivBySubgameG<T> M(ef,P,SPS.MaxSolns(),parent);
return M.GetSolutions();
}

template <class T>
void SimpdivG<T>::SolveSetup(void) const
{SimpdivSolveParamsDialog SDPD(parent->Frame(),true);}

TEMPLATE class SimpdivG<double>;
TEMPLATE class SimpdivG<gRational>;

// NF Gobit
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
// Note that we are defining the gRational and double functions separately
// since Gobit is not meant to be implemented in gRationals at all
#include "egobit.h"
#include "ngobit.h"
#define GOBIT_PRM_INST
#include "gobitprm.h"
template <class T> class NGobitBySubgameG:
													public BaseBySubgameG<T>
{
private:
	gList<BehavProfile<T> > solns;
protected:
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
NGobitBySubgameG(const Efg<T> &E,ExtensiveShowInterf<T> *parent_=0):
													BaseBySubgameG<T>(parent_,E)
{
GobitParamsSettings<T> GSPD(parent->Filename());
wxStatus status(parent->Frame(),"Gobit Algorithm");
NFGobitParams<T> P(status);
GSPD.GetParams(P);

Nfg<T> *N = MakeReducedNfg((Efg<T> &)E);
NFSupport *S=new NFSupport(*N);
ViewNormal(*N,S);

MixedProfile<T> start(*N,*S);
start.Centroid();
NFGobitModule<T> M(*N,P,start);
M.Gobit(1/*P.nequilib*/);
GSPD.RunPxi();

for (int i = 1; i <= M.GetSolutions().Length(); i++)
{
	BehavProfile<T> bp(E);
	MixedToBehav(*N, M.GetSolutions()[i], E, bp);
	solns.Append(bp);
}
delete N;
delete S;
}

gList<BehavProfile<T> > GetSolutions(void) const {return solns;}
};

TEMPLATE class NGobitBySubgameG<double>;


NGobitG<double>::NGobitG(const Efg<double> &E,ExtensiveShowInterf<double> *parent):ExtensiveSolutionG<double>(E,parent)
{ }

gList<BehavProfile<double> > NGobitG<double>::Solve(void) const
{
NGobitBySubgameG<double> M(ef,parent);
return M.GetSolutions();
}

void NGobitG<double>::SolveSetup(void) const
{GobitSolveParamsDialog<double> GSPD(parent->Frame(),parent->Filename());}


NGobitG<gRational>::NGobitG(const Efg<gRational> &E,ExtensiveShowInterf<gRational> *parent):ExtensiveSolutionG<gRational>(E,parent)
{ }
void NGobitG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavProfile<gRational> > NGobitG<gRational>::Solve(void) const
{doubles_only(); return empty_behav_list_r;}

TEMPLATE class NGobitG<double>;
TEMPLATE class NGobitG<gRational>;

// EF Gobit
// This algorithm does not support solving by subgames.

EGobitG<double>::EGobitG(const Efg<double> &E,ExtensiveShowInterf<double> *parent):ExtensiveSolutionG<double>(E,parent)
{ }

gList<BehavProfile<double> > EGobitG<double>::Solve(void) const
{
GobitParamsSettings<double> GSPD(parent->Filename());
wxStatus status(parent->Frame(),"Gobit Algorithm");
BehavProfile<double> start(ef);
start.Centroid();
EFGobitParams<double> P(status);
GSPD.GetParams(P);
EFGobitModule<double> M(ef,P,start);
M.Gobit(1);
GSPD.RunPxi();
return M.GetSolutions();
}

void EGobitG<double>::SolveSetup(void) const
{GobitSolveParamsDialog<double> GSPD(parent->Frame(),parent->Filename());}


EGobitG<gRational>::EGobitG(const Efg<gRational> &E,ExtensiveShowInterf<gRational> *parent):ExtensiveSolutionG<gRational>(E,parent)
{ }
void EGobitG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavProfile<gRational> > EGobitG<gRational>::Solve(void) const
{doubles_only();return empty_behav_list_r;}

TEMPLATE class EGobitG<double>;
TEMPLATE class EGobitG<gRational>;

// Gobit All
// This algorithm does not support solving by subgames.  However I will still
// derive a solution class from the BaseBySubgameG to maintain uniformity.
// Also note that this algorithm is unique in that it does not return any
// solutions but creates a file.
#define GRID_PRM_INST
#include "grid.h"
#include "gridprm.h"

template <class T> class GobitAllBySubgameG:
													public BaseBySubgameG<T>
{
protected:
void ViewNormal(const Nfg<T> &N, NFSupport *&sup)
{BaseViewNormal(N,sup);}
public:
GobitAllBySubgameG(const Efg<T> &E,ExtensiveShowInterf<T> *parent_):
													BaseBySubgameG<T>(parent_,E)
{
GridParamsSettings<T> GSPD(parent->Filename());
wxStatus status(parent->Frame(),"GobitAll Solve");
GridParams<T> P(status);
GSPD.GetParams(P);

Nfg<T> *N = MakeReducedNfg((Efg<T> &)E);
NFSupport *S=new NFSupport(*N);
ViewNormal(*N,S);
GridSolveModule<T> M(*N,P,*S);
int ok=M.GridSolve();
if (ok) GSPD.RunPxi();
delete N;
delete S;
}
};

TEMPLATE class GobitAllBySubgameG<double>;


template <class T>
GobitAllG<T>::GobitAllG(const Efg<T> &E,ExtensiveShowInterf<T> *parent):ExtensiveSolutionG<T>(E,parent)
{ }


template <class T>
gList<BehavProfile<T> > GobitAllG<T>::Solve(void) const
{
GobitAllBySubgameG<T> M(ef,parent);
return EmptyBehavList((T)1);;
}

template <class T>
void GobitAllG<T>::SolveSetup(void) const
{GridSolveParamsDialog<T> GSPD(parent->Frame(),parent->Filename());}

TEMPLATE class GobitAllG<double>;

GobitAllG<gRational>::GobitAllG(const Efg<gRational> &E,ExtensiveShowInterf<gRational> *parent):ExtensiveSolutionG<gRational>(E,parent)
{ }
void GobitAllG<gRational>::SolveSetup(void) const
{doubles_only();}
gList<BehavProfile<gRational> > GobitAllG<gRational>::Solve(void) const
{doubles_only(); return empty_behav_list_r;}

TEMPLATE class GobitAllG<gRational>;
