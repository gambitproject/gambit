//
// File: grid.cc -- Implementation of the GridSolve algorithm
//
//  $Id$
//

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

NEW:  To speed up the grid searches on large games, we introduce a two step
search.  The initial search uses a rough grid (params.delp1) and a
corresponding large tolerance (params.tol1).  If an EQU is found, a new
search is started on a hypercube centered around the point found in previous
step, with a finer grid (params.delp2) and a smaller tolerance (params.tol2).
To enable this two step process, the the params.multi_grid to nonzero.  Note
that this technique is easily extended to n steps.
*/

#include <math.h>
#include "mixed.h"
#include "grid.h"
#include "nfg.h"
#include "gwatch.h"

#define MAX_GRID		1		// only two steps for now (0,1)

#ifdef max
#undef max
#endif

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
friend class MixedProfileGrid1;
private:
  double step;
  int static_player;
  gArray<double> sums;
  bool Inc1(int row);
public:
  MixedProfileGrid(const NFSupport &S, double step);
  MixedProfileGrid(const MixedProfileGrid &P);
  bool Inc(void);
  void SetStatic(int static_player);
  int  GetStatic(void) const;
};


MixedProfileGrid::MixedProfileGrid(const NFSupport &S, double step_)
  : MixedProfile<double>(S), step(step_), sums(0)
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
      do {
	for (int i=1;i<=dim-1;i++)
	  if ((*this)(row,i)<1-step+eps)
	    {(*this)(row,i)+=step;sum+=step;break;}
	  else {
	    sum-=(*this)(row,i);(*this)(row,i)=0;
	    if (i==dim-1) {(*this)(row,dim)=1;return false;}
	  }
      } while (sum>1+eps);
      (*this)(row,dim)=1-sum;
      return true;
    }
  else	{ // dim==1
    return false;
  }
}

bool MixedProfileGrid::Inc(void)
{
  for (int i=1;i<=svlen.Length();i++) {
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

// ***************************************************************************
// MIXED PROFILE GRID 1
// ***************************************************************************
// This class is similar to the MixedProfileGrid.  It generates a grid of
// stepsize step in the region around the point defined by M with a radius of
// size.  It is used to generate a 'finer' grid around a potential EQU point
// in GridSolve.  In that case, size=rough_grid/2.
// In this example, * is the point of interest, step=1, rough_grid=4
// .    .    .    .
// .    .  .....  .
// .    .  .....  .
// .    .  ..*..  .
// .    .  .....  .
// .    .  .....  .
// .    .    .    .

class MixedProfileGrid1: public MixedProfile<double>
{
private:
  double step;
  MixedProfile<double> min_val,max_val;
  int static_player;
  gArray<double> sums;
  bool Inc1(int row);
public:
  MixedProfileGrid1(const MixedProfile<double> &M,double step,double size,int static_player);
  bool Inc(void);
};

MixedProfileGrid1::MixedProfileGrid1(const MixedProfile<double> &M,
				     double step_,double size,int static_player_):
  MixedProfile<double>(M),step(step_),min_val(M),max_val(M),
  static_player(static_player_)
{
  sums=gArray<double>(svlen.Length());
  for (int i=1;i<=svlen.Length();i++) {
    sums[i]=0;
    int j;
    // Precalc these to save time.  Memory is cheap.
    for (j=1;j<=svlen[i]-1;j++) { 
      if (min_val(i,j)>=size) min_val(i,j)-=size; else min_val(i,j)=0;
      (*this)(i,j)=min_val(i,j);		// start at the minimum
      sums[i]+=(*this)(i,j);
      if (max_val(i,j)<=1-size) max_val(i,j)+=size; else max_val(i,j)=1;
    }
    (*this)(i,j)=1-sums[i];  // last_player's strat = 1-Sum[previous]
  }
}

bool MixedProfileGrid1::Inc1(int row)
{
  int dim=svlen[row];
  // dim==1 is an annoying special case
  if (dim!=1) {
    double &sum=sums[row];
    do {
      for (int i=1;i<=dim-1;i++)
	if ((*this)(row,i)<max_val(row,i)-step+eps)
	  {(*this)(row,i)+=step;sum+=step;break;}
	else {
	  sum-=((*this)(row,i)-min_val(row,i));(*this)(row,i)=min_val(row,i);
	  if (i==dim-1) {(*this)(row,dim)=1-sum;return false;}
	}
    } while (sum>1+eps);
    (*this)(row,dim)=1-sum;
    return true;
  }
  // dim==1
  else	{
    return false;
  }
}

bool MixedProfileGrid1::Inc(void)
{
  for (int i=1;i<=svlen.Length();i++) {
    if (i==static_player) continue;
    if (Inc1(i))
      return true;
    else
      if (i==svlen.Length()) return false;
  }
  return false; // return from here only when static_player==svlen.Length
}



// ***************************************************************************
// GRID SOLVE
// ***************************************************************************

GridParams::GridParams(gStatus &s)
  : AlgParams(s), minLam(.01), maxLam(30), delLam(.01), delp1(.01), delp2(0.01), 
    tol1(.01),tol2(0.01), powLam(1), fullGraph(false), pxifile(&gnull)
{ }

class GridSolveModule  {
private:
  const Nfg &N;
  const NFSupport &S;
  const GridParams &params;
  gArray<int> num_strats;
  MixedProfile<double> P_calc;
  gVector<double> tmp; // scratch
  double lam;
  int static_player;
  gList<MixedSolution> solutions;

  
  gVector<double> UpdateFunc(const MixedProfile<double> &P,int pl,double lam);
  bool CheckEqu(MixedProfile<double> P,double lam,int cur_grid);
  void OutputHeader(gOutput &out);
  void OutputResult(gOutput &out,const MixedProfile<double> P,double lam,double obj_func);
protected:
  // could use norms other then the simple one
  virtual double Distance(const gVector<double> &a,const gVector<double> &b) const;
public:
  GridSolveModule(const NFSupport &, const GridParams &);
  virtual ~GridSolveModule();
  void GridSolve(void);
  gList<MixedSolution> &GetSolutions(void);
};

// Output header
void GridSolveModule::OutputHeader(gOutput &out)
{
  out<<"Dimensionality:\n";
  out<<N.NumPlayers()<<' ';
  for (int pl = 1; pl <= N.NumPlayers(); pl++) out << N.NumStrats(pl) << ' ';
  out << '\n';
  if (N.NumPlayers()==2)		// output the matrix in case of a 2x2 square game
    if (N.NumStrats(1)==N.NumStrats(2))  {
      out<<"Game:\n";
      out<<"3 2 1\n";
      gArray<int> profile(2);
      for (int i=1;i<=N.NumStrats(1);i++)  {
	for (int j=1;j<=N.NumStrats(2);j++) {
	  profile[1]=i;profile[2]=j;
	  out << N.Payoff(N.GetOutcome(profile), 1) << ' ' <<
	    N.Payoff(N.GetOutcome(profile), 2) << ' ';
	}
	out<<'\n';
      }
    }
  out<<"Settings:\n";
  out<<params.minLam<<'\n'<<params.maxLam<<'\n'<<params.delLam<<'\n';
  out<<0<<'\n'<<1<<'\n'<<params.powLam<<'\n';
  
  out<<"Extra:\n";
  out<<1<<'\n'<<params.tol1<<'\n'<<params.delp1<<'\n';
  
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
  double dist=0;
  for (int i = 1; i <= a.Length(); i++)
    dist+=abs(a[i]-b[i]);
  return dist;
}


gVector<double> GridSolveModule::UpdateFunc(const MixedProfile<double> &P,int pl,double lam)
{
  gVector<double> r(num_strats[pl]);
  double denom;
  int strat;
  denom=0.0;
  for (strat=1;strat<=num_strats[pl];strat++) {
    double p=P.Payoff(pl,pl,strat);
    tmp[strat]=exp(lam*p);
    denom+=tmp[strat];
  }
  for (strat=1;strat<=num_strats[pl];strat++)
    r[strat]=tmp[strat]/denom;
  return r;
}


// Note: static_player just refers to the player w/ the greatest # of strats.
bool GridSolveModule::CheckEqu(MixedProfile<double> P,double lam,int cur_grid)
{
  P.SetRow(static_player,UpdateFunc(P,static_player,lam));
  
  int pl, i;
  double obj_func=0.0;
  double tol=(cur_grid==0) ? params.tol1 : params.tol2;
  for (pl=1;pl<=N.NumPlayers();pl++)
    if (pl!=static_player) {
      P_calc.SetRow(pl,UpdateFunc(P,pl,lam));
      obj_func+=Distance(P_calc.GetRow(pl),P.GetRow(pl));
      if (obj_func>tol) return false;
    }
  
  // If we got here, objective function is < tolerance -- have an EQU point
  // this is it, we are done
  if (params.multi_grid==0 || cur_grid==MAX_GRID) {
    OutputResult(*params.pxifile,P,lam,obj_func); // Output it to file
    if (params.trace>0) OutputResult(*params.tracefile,P,lam,obj_func);
    if(!params.fullGraph) solutions.Flush();
    i = solutions.Append(MixedSolution(P,algorithmNfg_QREALL));
    solutions[i].SetQre(lam, obj_func);  // use Liap value instead of obj_func here?
  }
    // now redo the search on a finer grid around this point
  else {
    cur_grid++;
    MixedProfileGrid1 P1(P,params.delp2,params.delp1/2,static_player);
    do {CheckEqu(P1,lam,cur_grid);} while (P1.Inc())	;
  }
  
  return true;
}

GridSolveModule::GridSolveModule(const NFSupport &S_, const GridParams &P)
  : N(S_.Game()), S(S_), params(P), P_calc(S_), tmp(max(S.NumStrats()))
{
  num_strats=S.NumStrats();
  // find the player w/ max num strats and make it static
  static_player=1;
  for (int i=2;i<=num_strats.Length();i++)
    if (num_strats[i]>num_strats[static_player]) static_player=i;
}

GridSolveModule::~GridSolveModule() { }

void GridSolveModule::GridSolve(void)
{
  //params.status<<"Grid Solve algorithm\n";
  // Initialize the output file
  gWatch timer;timer.Start();
  OutputHeader(*params.pxifile);
  if (params.trace>0) OutputHeader(*params.tracefile);
  int num_steps;
  if (params.powLam==0)
    num_steps=(int)((params.maxLam-params.minLam)/params.delLam);
  else
    num_steps=(int)(log(params.maxLam/params.minLam)/log(params.delLam+1));
  MixedProfileGrid M(S, params.delp1);
  M.SetStatic(static_player);
  double lam=params.minLam;
  for (int step=1;step<num_steps;step++) {
    params.status.Get();
    if (params.powLam==0)  lam=lam+params.delLam; else lam=lam*(params.delLam+1);
    do {CheckEqu(M,lam,0);} while (M.Inc())	;
    params.status.SetProgress((double)step/(double)num_steps);
  }
  // Record the time taken and close the output file
  timer.Stop();
  *params.pxifile<<"Simulation took "<<timer.ElapsedStr()<<'\n';
  params.status<<"Simulation took "<<timer.ElapsedStr()<<'\n';
  
}

gList<MixedSolution> & GridSolveModule::GetSolutions(void) { return solutions;}

int GridSolve(const NFSupport &support, const GridParams &params,
              gList<MixedSolution> &solutions)
{
  GridSolveModule module(support, params);
  module.GridSolve();
  solutions = module.GetSolutions();
  return 1;
}



