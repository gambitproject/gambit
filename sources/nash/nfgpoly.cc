//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of gbtPolyUni equations
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/gnullstatus.h"
#include "game/nfgensup.h"
#include "poly/quiksolv.h"
#include "nfgpoly.h"

class PolEnumParams {
public:
  int stopAfter;

  PolEnumParams(void);
};

class PolEnumModule  {
private:
  gbtDouble eps;
  gbtNfgGame m_nfg;
  const gbtNfgSupport &support;
  PolEnumParams params;
  gbtPolySpace Space;
  gbtPolyTermOrder Lex;
  int num_vars;
  long count,nevals;
  double time;
  gbtList<MixedSolution> solutions;
  bool is_singular;

  bool EqZero(gbtDouble x) const;
  
  // p_i_j as a gbtPolyMulti, with last prob in terms of previous probs
  gbtPolyMulti<gbtDouble> Prob(int i,int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.  
  gbtPolyMulti<gbtDouble> IndifferenceEquation(int i, int strat1, int strat2) const;
  gbtPolyMultiList<gbtDouble>   IndifferenceEquations()                 const;
  gbtPolyMultiList<gbtDouble>   LastActionProbPositiveInequalities()    const;
  gbtPolyMultiList<gbtDouble>   NashOnSupportEquationsAndInequalities() const;
  gbtList<gbtVector<gbtDouble> > 
               NashOnSupportSolnVectors(const gbtPolyMultiList<gbtDouble> &equations,
					const gRectangle<gbtDouble> &Cube,
					gbtStopWatch &timer,
					gbtStatus &p_status);

  int SaveSolutions(const gbtList<gbtVector<gbtDouble> > &list);
public:
  PolEnumModule(const gbtNfgSupport &, const PolEnumParams &p);
  
  int PolEnum(gbtStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  PolEnumParams &Parameters(void);

  const gbtList<MixedSolution> &GetSolutions(void) const;
  gbtVector<gbtDouble> SolVarsFromMixedProfile(const gbtMixedProfile<gbtNumber> &) 
                                                                     const;

  const int PolishKnownRoot(gbtVector<gbtDouble> &) const;

  MixedSolution ReturnPolishedSolution(const gbtVector<gbtDouble> &) const;

  bool IsSingular() const;
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

PolEnumModule::PolEnumModule(const gbtNfgSupport &S, const PolEnumParams &p)
  : m_nfg(S.GetGame()), support(S), params(p), 
    Space(support.ProfileLength()-m_nfg.NumPlayers()), 
    Lex(&Space, lex), num_vars(support.ProfileLength()-m_nfg.NumPlayers()), 
    count(0), nevals(0), is_singular(false)
{ 
//  gEpsilon(eps,12);
}


int PolEnumModule::PolEnum(gbtStatus &p_status)
{
  gbtStopWatch watch;
  gbtPolyMultiList<gbtDouble> equations = NashOnSupportEquationsAndInequalities();

  /*
  // equations for equality of strat j to strat j+1
  for( i=1;i<=m_nfg.NumPlayers();i++) 
    for(j=1;j<support.NumStrats(i);j++) 
      equations+=IndifferenceEquation(i,j,j+1);

  for( i=1;i<=m_nfg.NumPlayers();i++)
    if(support.NumStrats(i)>2) 
      equations+=Prob(i,support.NumStrats(i));
  */

  // set up the rectangle of search
  gbtVector<gbtDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gbtDouble)0;
  tops = (gbtDouble)1;
 
  gRectangle<gbtDouble> Cube(bottoms, tops); 

  // start gbtPolyQuickSolve
  gbtStopWatch timer;
  timer.Start();

  gbtList<gbtVector<gbtDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube,
								   timer,
								   p_status);

  timer.Stop();
  int index = SaveSolutions(solutionlist);
  time = watch.Elapsed();
  return index;	 
}

int PolEnumModule::SaveSolutions(const gbtList<gbtVector<gbtDouble> > &list)
{
  gbtMixedProfile<double> profile(support);
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=m_nfg.NumPlayers();i++) {
      sum=0;
      for(j=1;j<support.NumStrats(i);j++) {
	profile(i,j) = (list[k][j+kk]).ToDouble();
	sum+=profile(i,j);
      }
      profile(i,j) = (double)1.0 - sum;
      kk+=(support.NumStrats(i)-1);
    }
    index = solutions.Append(MixedSolution(profile, "PolEnum[m_nfgG]"));
    gbtNumber eps = (gbtNumber)0.0;
    gEpsilon(eps);
    solutions[index].SetEpsilon(eps);
  }
  return index;
}

bool PolEnumModule::EqZero(gbtDouble x) const
{
  if(x <= eps && x >= -eps) return 1;
  return 0;
}     

long PolEnumModule::NumEvals(void) const
{
  return nevals;
}

double PolEnumModule::Time(void) const
{
  return time;
}

PolEnumParams &PolEnumModule::Parameters(void)
{
  return params;
}

const gbtList<MixedSolution> &PolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gbtPolyMulti<gbtDouble> PolEnumModule::Prob(int p, int strat) const
{
  gbtPolyMulti<gbtDouble> equation(&Space,&Lex);
  gbtVector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(support.NumStrats(i)-1);

  if(strat<support.NumStrats(p)) {
    exps=0;
    exps[strat+kk]=1;
    gbtPolyExponent const_exp(&Space,exps);
    gbtMonomial<gbtDouble> const_term((gbtDouble)1,const_exp);
    gbtPolyMulti<gbtDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<support.NumStrats(p);j++) {
      exps=0;
      exps[j+kk]=1;
      gbtPolyExponent exponent(&Space,exps);
      gbtMonomial<gbtDouble> term((gbtDouble)(-1),exponent);
      gbtPolyMulti<gbtDouble> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    gbtPolyExponent const_exp(&Space,exps);
    gbtMonomial<gbtDouble> const_term((gbtDouble)1,const_exp);
    gbtPolyMulti<gbtDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gbtPolyMulti<gbtDouble> 
PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  gbtNfgContingency profile(m_nfg);

  gbtNfgContIterator A(support), B(support);
  A.Freeze(support.GetStrategy(i, strat1));
  B.Freeze(support.GetStrategy(i, strat2));
  gbtPolyMulti<gbtDouble> equation(&Space,&Lex);
  do {
    gbtPolyMulti<gbtDouble> term(&Space,(gbtDouble)1,&Lex);
    profile = A.GetProfile();
    int k;
    for(k=1;k<=m_nfg.NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.GetIndex(profile.GetStrategy(k)));
    gbtDouble coeff,ap,bp;
    ap = (double) A.GetPayoff(m_nfg.GetPlayer(i));
    bp = (double) B.GetPayoff(m_nfg.GetPlayer(i));
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}


gbtPolyMultiList<gbtDouble>   PolEnumModule::IndifferenceEquations()  const
{
  gbtPolyMultiList<gbtDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg.NumPlayers();pl++) 
    for(int j=1;j<support.NumStrats(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gbtPolyMultiList<gbtDouble> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gbtPolyMultiList<gbtDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg.NumPlayers();pl++)
    if(support.NumStrats(pl)>2) 
      equations+=Prob(pl,support.NumStrats(pl));

  return equations;
}

gbtPolyMultiList<gbtDouble> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gbtPolyMultiList<gbtDouble> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


gbtList<gbtVector<gbtDouble> > 
PolEnumModule::NashOnSupportSolnVectors(const gbtPolyMultiList<gbtDouble> &equations,
					      const gRectangle<gbtDouble> &Cube,
					      gbtStopWatch &timer,
					      gbtStatus &p_status)
{  
  gbtPolyQuickSolve<gbtDouble> quickie(equations, p_status);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647,params.stopAfter);
  }
  catch (gbtSignalBreak &) { }
  catch (gbtSquareMatrix<gbtDouble>::MatrixSingular) {
    is_singular = true;
  }

  return quickie.RootList();
}

bool PolEnumModule::IsSingular() const
{
  return is_singular;
}


//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

PolEnumParams::PolEnumParams(void)
  : stopAfter(0)
{ }

int PolEnum(const gbtNfgSupport &support, const PolEnumParams &params,
	    gbtList<MixedSolution> &solutions, gbtStatus &p_status,
	    long &nevals, double &time, bool &is_singular)
{
  PolEnumModule module(support, params);
  module.PolEnum(p_status);
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  if (module.IsSingular()) 
    is_singular = true;
  else 
    is_singular = false;
  return 1;
}


//---------------------------------------------------------------------------
//                        Polish Equilibrum for Nfg
//---------------------------------------------------------------------------

MixedSolution PolishEquilibrium(const gbtNfgSupport &support, 
				const MixedSolution &sol, 
				bool &is_singular)
{
  PolEnumParams params;
  PolEnumModule module(support, params);
  gbtVector<gbtDouble> vec = module.SolVarsFromMixedProfile(*(sol.Profile()));

  /* //DEBUG
  gbtPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  gbtMixedProfile<gbtNumber> newsol = module.SequenceForm().ToMixed(xx);

  gout << "sol.Profile = " << *(sol.Profile()) << "\n";
  gout << "vec  = " << vec << "\n";
  gout << "xx   = " << xx << "\n";
  gout << "newsol   = " << newsol << "\n";

    exit(0);
  if ( newsol != *(sol.Profile()) ) {
    gout << "Failure of reversibility in PolishEquilibrium.\n";
    exit(0);
  }
  */

  //DEBUG
  //  gout << "Prior to Polishing vec is " << vec << ".\n";

  module.PolishKnownRoot(vec);

  //DEBUG
  //  gout << "After Polishing vec is " << vec << ".\n";

  return module.ReturnPolishedSolution(vec);
}


gbtVector<gbtDouble> 
PolEnumModule::SolVarsFromMixedProfile(const gbtMixedProfile<gbtNumber> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) 
    numvars += support.NumStrats(pl) - 1;

  gbtVector<gbtDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrats(pl); j++) {
      count ++;
      answer[count] = (gbtDouble)sol(pl,j);
    }

  return answer;
}

const int PolEnumModule::PolishKnownRoot(gbtVector<gbtDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    
    gbtStopWatch watch;
    
    // equations for equality of strat j to strat j+1
    gbtPolyMultiList<gbtDouble> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start gbtPolyQuickSolve
    gbtNullStatus gstatus;
    gbtPolyQuickSolve<gbtDouble> quickie(equations, gstatus);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (gbtSignalBreak &) { }
    catch (gbtSquareMatrix<gbtDouble>::MatrixSingular &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

MixedSolution 
PolEnumModule::ReturnPolishedSolution(const gbtVector<gbtDouble> &root) const
{
  gbtMixedProfile<double> profile(support);

  int j;
  int kk=0;
  for(int pl=1;pl<=m_nfg.NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<support.NumStrats(pl);j++) {
      profile(pl,j) = (root[j+kk]).ToDouble();
      sum+=profile(pl,j);
    }
    profile(pl,j) = (double)1.0 - sum;
    kk+=(support.NumStrats(pl)-1);
  }
       
  MixedSolution sol(profile, "Polish[NFG]");
  return sol;
}

//========================================================================
//                      class gbtNfgNashEnumPoly
//========================================================================

gbtNfgNashEnumPoly::gbtNfgNashEnumPoly(void)
  : m_stopAfter(0)
{ }

gbtList<MixedSolution> gbtNfgNashEnumPoly::Solve(const gbtNfgSupport &p_support,
					       gbtStatus &p_status)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  gbtList<const gbtNfgSupport> supports = PossibleNashSubsupports(p_support,
								p_status);

  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";

  gbtList<const gbtNfgSupport> singularSupports;
  gbtList<MixedSolution> solutions;

  for (int i = 1; (i <= supports.Length() &&
		   (m_stopAfter == 0 || m_stopAfter > solutions.Length())); 
       i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) supports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gbtList<MixedSolution> newsolns;
    bool is_singular = false;
    
    PolEnumParams params;
    params.stopAfter = 0;
    PolEnum(supports[i], params, newsolns, p_status,
	    newevals, newtime, is_singular);

    for (int j = 1; j <= newsolns.Length(); j++) {
      if (newsolns[j].IsNash()) {
	solutions += newsolns[j];
      }
    }

    if (is_singular) { 
      singularSupports += supports[i];
    }
  }

  return solutions;
}
