//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of polynomial equations
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
  gDouble eps;
  gbtNfgGame m_nfg;
  const gbtNfgSupport &support;
  PolEnumParams params;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long count,nevals;
  double time;
  gList<MixedSolution> solutions;
  bool is_singular;

  bool EqZero(gDouble x) const;
  
  // p_i_j as a gPoly, with last prob in terms of previous probs
  gPoly<gDouble> Prob(int i,int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.  
  gPoly<gDouble> IndifferenceEquation(int i, int strat1, int strat2) const;
  gPolyList<gDouble>   IndifferenceEquations()                 const;
  gPolyList<gDouble>   LastActionProbPositiveInequalities()    const;
  gPolyList<gDouble>   NashOnSupportEquationsAndInequalities() const;
  gList<gVector<gDouble> > 
               NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					const gRectangle<gDouble> &Cube,
					gWatch &timer,
					gStatus &p_status);

  int SaveSolutions(const gList<gVector<gDouble> > &list);
public:
  PolEnumModule(const gbtNfgSupport &, const PolEnumParams &p);
  
  int PolEnum(gStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  PolEnumParams &Parameters(void);

  const gList<MixedSolution> &GetSolutions(void) const;
  gVector<gDouble> SolVarsFromMixedProfile(const MixedProfile<gNumber> &) 
                                                                     const;

  const int PolishKnownRoot(gVector<gDouble> &) const;

  MixedSolution ReturnPolishedSolution(const gVector<gDouble> &) const;

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


int PolEnumModule::PolEnum(gStatus &p_status)
{
  gWatch watch;
  gPolyList<gDouble> equations = NashOnSupportEquationsAndInequalities();

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
  gVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
 
  gRectangle<gDouble> Cube(bottoms, tops); 

  // start QuikSolv
  gWatch timer;
  timer.Start();

  gList<gVector<gDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube,
								   timer,
								   p_status);

  timer.Stop();
  int index = SaveSolutions(solutionlist);
  time = watch.Elapsed();
  return index;	 
}

int PolEnumModule::SaveSolutions(const gList<gVector<gDouble> > &list)
{
  MixedProfile<double> profile(support);
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
    gNumber eps = (gNumber)0.0;
    gEpsilon(eps);
    solutions[index].SetEpsilon(eps);
  }
  return index;
}

bool PolEnumModule::EqZero(gDouble x) const
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

const gList<MixedSolution> &PolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gPoly<gDouble> PolEnumModule::Prob(int p, int strat) const
{
  gPoly<gDouble> equation(&Space,&Lex);
  gVector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(support.NumStrats(i)-1);

  if(strat<support.NumStrats(p)) {
    exps=0;
    exps[strat+kk]=1;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<support.NumStrats(p);j++) {
      exps=0;
      exps[j+kk]=1;
      exp_vect exponent(&Space,exps);
      gMono<gDouble> term((gDouble)(-1),exponent);
      gPoly<gDouble> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gPoly<gDouble> 
PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  StrategyProfile profile(m_nfg);

  NfgContIter A(support), B(support);
  A.Freeze(i);
  A.Set(i, strat1);
  B.Freeze(i);
  B.Set(i, strat2);
  gPoly<gDouble> equation(&Space,&Lex);
  do {
    gPoly<gDouble> term(&Space,(gDouble)1,&Lex);
    profile = A.Profile();
    int k;
    for(k=1;k<=m_nfg.NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.GetIndex(profile[k]));
    gDouble coeff,ap,bp;
    ap = (double) A.GetOutcome().GetPayoff(m_nfg.GetPlayer(i));
    bp = (double) B.GetOutcome().GetPayoff(m_nfg.GetPlayer(i));
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}


gPolyList<gDouble>   PolEnumModule::IndifferenceEquations()  const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg.NumPlayers();pl++) 
    for(int j=1;j<support.NumStrats(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gPolyList<gDouble> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg.NumPlayers();pl++)
    if(support.NumStrats(pl)>2) 
      equations+=Prob(pl,support.NumStrats(pl));

  return equations;
}

gPolyList<gDouble> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


gList<gVector<gDouble> > 
PolEnumModule::NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					      const gRectangle<gDouble> &Cube,
					      gWatch &timer,
					      gStatus &p_status)
{  
  QuikSolv<gDouble> quickie(equations, p_status);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647,params.stopAfter);
  }
  catch (gSignalBreak &) { }
  catch (gSquareMatrix<gDouble>::MatrixSingular) {
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
	    gList<MixedSolution> &solutions, gStatus &p_status,
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
  gVector<gDouble> vec = module.SolVarsFromMixedProfile(*(sol.Profile()));

  /* //DEBUG
  gPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  MixedProfile<gNumber> newsol = module.SequenceForm().ToMixed(xx);

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


gVector<gDouble> 
PolEnumModule::SolVarsFromMixedProfile(const MixedProfile<gNumber> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) 
    numvars += support.NumStrats(pl) - 1;

  gVector<gDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrats(pl); j++) {
      count ++;
      answer[count] = (gDouble)sol(pl,j);
    }

  return answer;
}

const int PolEnumModule::PolishKnownRoot(gVector<gDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    
    gWatch watch;
    
    // equations for equality of strat j to strat j+1
    gPolyList<gDouble> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    gNullStatus gstatus;
    QuikSolv<gDouble> quickie(equations, gstatus);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (gSignalBreak &) { }
    catch (gSquareMatrix<gDouble>::MatrixSingular &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

MixedSolution 
PolEnumModule::ReturnPolishedSolution(const gVector<gDouble> &root) const
{
  MixedProfile<double> profile(support);

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

gList<MixedSolution> gbtNfgNashEnumPoly::Solve(const gbtNfgSupport &p_support,
					       gStatus &p_status)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  gList<const gbtNfgSupport> supports = PossibleNashSubsupports(p_support,
								p_status);

  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";

  gList<const gbtNfgSupport> singularSupports;
  gList<MixedSolution> solutions;

  for (int i = 1; (i <= supports.Length() &&
		   (m_stopAfter == 0 || m_stopAfter > solutions.Length())); 
       i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) supports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gList<MixedSolution> newsolns;
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







