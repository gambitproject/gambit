// File: grid.cc -- Implementation of the GridSolve algorithm
// $Id$
/*
Here is how it should be done for three person games
(extension to more players is done in an analagous fashion.)

Suppose p,q,and r are the probability vectors for players 1, 2, and 3,
respectively.  Then the following equations give the updating rules
for any one of these, given the other two.

p = f_p(q,r)
q = f_q(p,r)
r = f_r(p,q)

Here, f_p(q,r), etc.  are determined by the exponential rules similar
to the two player case.  So, if p = (p_1,p_2,p_3), then

p_j = (f_p(q,r))_j =
			exp(lam*v_1j)/(exp(lam*v_11)+exp(lam*v_12)+exp(lam*v_13))

where v_1j is the value of strategy j for player 1, when plugging in
the probabilities q and r for players 2 and 3.  Similarly for the
other players.

To find the whole QRE correspondence, then pick n-1 players (say 1 and
2), and then use a grid over their strategy spaces.  For any p,q on
the grid, evaluate the following objective function:

v(p,q) = | p - f_p(q,f_r(p,q)) | + | q - f_q(p,f_r(p,q)) |

Then keep any values of (p,q,f_r(p,q)) where v(p,q) < tolerance.
*/

#include <math.h>
#include "mixed.h"
#include "grid.h"
#include "nfg.h"
#include "gwatch.h"

static int max(const gArray<int> &a)
{
int m=a[1];
for (int i=2;i<=a.Length();i++) if (a[i]>m) m=a[i];
return m;
}

// Probability Vector class.  This
// class is used by the GridSolve algorithm to generate a uniform grid of
// probabilities over n players w/ Si strategies each.  The class makes
// sure that the sum of strategy probabilities for each player=MAX_PROB=1

// Note that the grid can be generated for (n-1), not n players but setting
// the static_player member to the nonzero integer corresponding to the
// player whose probability space is NOT to be iterated over.  This is used
// by GridSolve

// The grid step is specified by the step parameter.


static double eps=1e-8;	// epsilon to allow == operator for floating point

class MixedProfileGrid: public MixedProfile<double>
{
private:
	double step;
	gArray<double> sums;
	int static_player;
	bool Inc1(int row);
public:
	MixedProfileGrid(const Nfg<double> &N,const NFSupport &S,double step);
	MixedProfileGrid(const MixedProfileGrid &P);
	bool Inc(void);
	void SetStatic(int static_player);
	int  GetStatic(void) const;
};


MixedProfileGrid::MixedProfileGrid(const Nfg<double> &N,const NFSupport &S, double step_):
											MixedProfile<double>(N,S),step(step_),sums(0)
{
int i;
for (i=1;i<=svlen.Length();i++)
	for (int j=1;j<=svlen[i];j++)
		(*this)(i,j)=(j==svlen[i]) ? 1 : 0;
sums=gArray<double>(svlen.Length());
for (i=1;i<=svlen.Length();i++) sums[i]=0.0;
static_player=0;
}

MixedProfileGrid::MixedProfileGrid(const MixedProfileGrid &P) :
	MixedProfile<double>(P),step(P.step),static_player(P.static_player),
	sums(P.sums)
{ }


bool MixedProfileGrid::Inc1(int row)
{
int dim=svlen[row];
if (dim!=1)		// dim==1 is an annoying special case
{
	double &sum=sums[row];
	do
	{
		for (int i=1;i<=dim-1;i++)
			if ((*this)(row,i)<1-step+eps)
				{(*this)(row,i)+=step;sum+=step;break;}
			else
				{
					sum-=(*this)(row,i);(*this)(row,i)=0;
					if (i==dim-1) {(*this)(row,dim)=1;return false;}
				}
	} while (sum>1+eps);
	(*this)(row,dim)=1-sum;
	return true;
}
else	// dim==1
{
	return false;
}
}

bool MixedProfileGrid::Inc(void)
{
for (int i=1;i<=svlen.Length();i++)
{
	if (i==static_player) continue;
	if (Inc1(i))
		return true;
	else
		if (i==svlen.Length()) return false;
}
return false; // return from here only when static_player==svlen.Length
}

void MixedProfileGrid::SetStatic(int static_player_)
{static_player=static_player_;}

int MixedProfileGrid::GetStatic(void) const
{return static_player;}

GridParams::GridParams(gStatus &st)
	: minLam(.01), maxLam(30), delLam(.01), delp(.01), tol(.01), powLam(1),
		trace(0), tracefile(&gnull), pxifile(&gnull), status(st)
{ }

// ***************************************************************************
// GRID SOLVE
// ***************************************************************************

// Output header
void GridSolveModule::OutputHeader(gOutput &out)
{
out<<"Dimensionality:\n";
out<<N.NumPlayers()<<' ';
for (int pl = 1; pl <= N.NumPlayers(); pl++) out << N.NumStrats(pl) << ' ';
gout << '\n';
if (N.NumPlayers()==2)		// output the matrix in case of a 2x2 square game
	if (N.NumStrats(1)==N.NumStrats(2))
	{
		out<<"Game:\n";
		out<<"3 2 1\n";
		gArray<int> profile(2);
		for (int i=1;i<=N.NumStrats(1);i++)
		{
			for (int j=1;j<=N.NumStrats(2);j++)
			{
				profile[1]=i;profile[2]=j;
				out<<N.Payoff(1,profile)<<' '<<N.Payoff(2,profile)<<' ';
			}
			out<<'\n';
		}
	}
out<<"Settings:\n";
out<<params.minLam<<'\n'<<params.maxLam<<'\n'<<params.delLam<<'\n';
out<<0<<'\n'<<1<<'\n'<<params.powLam<<'\n';

out<<"Extra:\n";
out<<1<<'\n'<<params.tol<<'\n'<<params.delp<<'\n';

out<<"DataFormat:\n";
int numcols = N.ProfileLength() + 2;
out << numcols<<' '; // Format: Lambda, ObjFunc, Prob1,0..1,n1 , Prob2,0..2,n2 , ...
for (int i = 1; i <= numcols; i++) out << i << ' ';

out<<'\n';

out<<"Data:\n";
}

void GridSolveModule::OutputResult(gOutput &out,const MixedProfile<double> P,
																	 double lam,double obj_func)
{
out << lam << ' ' << obj_func << ' ';
for (int pl=1;pl<=num_strats.Length();pl++)
	for (int st=1;st<=num_strats[pl];st++)
		out << P(pl,st) << ' ';
out << '\n';
}

double GridSolveModule::Distance(const gVector<double> &a,const gVector<double> &b) const
{
assert(a.Check(b)); // better be the same size
double dist=0;
for (int i=1;i<=a.Length();i++)	dist+=abs(a[i]-b[i]);
return dist;
}


gVector<double> GridSolveModule::UpdateFunc(const MixedProfile<double> &P,int pl,double lam)
{
gVector<double> r(num_strats[pl]);
double denom;
int strat;
denom=0.0;
for (strat=1;strat<=num_strats[pl];strat++)
{
	double p=P.Payoff(pl,pl,strat);
	tmp[strat]=exp(lam*p);
	denom+=tmp[strat];
}
for (strat=1;strat<=num_strats[pl];strat++)
	r[strat]=tmp[strat]/denom;
return r;
}


// Note: static_player just refers to the player w/ the greatest # of strats.
bool GridSolveModule::CheckEqu(MixedProfile<double> P,double lam)
{
P.SetRow(static_player,UpdateFunc(P,static_player,lam));

int pl;
double obj_func=0.0;
for (pl=1;pl<=N.NumPlayers();pl++)
	if (pl!=static_player)
	{
		P_calc.SetRow(pl,UpdateFunc(P,pl,lam));
		obj_func+=Distance(P_calc.GetRow(pl),P.GetRow(pl));
		if (obj_func>params.tol) return false;
	}

// If we got here, objective function is < tolerance -- have an EQU point
OutputResult(*params.pxifile,P,lam,obj_func); // Output it to file
if (params.trace>0) OutputResult(*params.tracefile,P,lam,obj_func);
return true;
}

GridSolveModule::GridSolveModule(const Nfg<double> &N_, const GridParams &P, const NFSupport &S_)
										:N(N_),S(S_),params(P),P_calc(N_,S_),tmp(max(S.SupportDimensions()))
{
num_strats=S.SupportDimensions();
// find the player w/ max num strats and make it static
static_player=1;
for (int i=2;i<=num_strats.Length();i++)
	if (num_strats[i]>num_strats[static_player]) static_player=i;
}

GridSolveModule::~GridSolveModule() { }

void GridSolveModule::GridSolve(void)
{
params.status<<"Grid Solve algorithm\n";
// Initialize the output file
gWatch timer;timer.Start();
OutputHeader(*params.pxifile);
if (params.trace>0) OutputHeader(*params.tracefile);
int num_steps;
if (params.powLam==0)
	num_steps=(int)((params.maxLam-params.minLam)/params.delLam);
else
	num_steps=(int)(log(params.maxLam/params.minLam)/log(params.delLam+1));
MixedProfileGrid M(N,S,params.delp);
M.SetStatic(static_player);
double lam=params.minLam;
for (int step=1;step<num_steps && !params.status.Get();step++)
{
	if (params.powLam==0)  lam=lam+params.delLam; else lam=lam*(params.delLam+1);
	do {CheckEqu(M,lam);} while (M.Inc())	;
	params.status.SetProgress((double)step/(double)num_steps);
}
// Record the time taken and close the output file
timer.Stop();
*params.pxifile<<"Simulation took "<<timer.ElapsedStr()<<'\n';
params.status<<"Simulation took "<<timer.ElapsedStr()<<'\n';


if (params.status.Get()) params.status.Reset();

}
