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
#include "game/game.h"
#include "game/nfgciter.h"
#include "poly/quiksolv.h"
#include "nfgpoly.h"

class PolEnumParams {
public:
  int stopAfter;

  PolEnumParams(void);
};

class PolEnumModule  {
private:
  double eps;
  gbtGame m_nfg;
  PolEnumParams params;
  gbtPolySpace Space;
  gbtPolyTermOrder Lex;
  int num_vars;
  long count,nevals;
  double time;
  gbtList<gbtMixedProfile<double> > solutions;
  bool is_singular;

  bool EqZero(double x) const;
  
  // p_i_j as a gbtPolyMulti, with last prob in terms of previous probs
  gbtPolyMulti<double> Prob(int i,int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.  
  gbtPolyMulti<double> IndifferenceEquation(int i, int strat1, int strat2) const;
  gbtPolyMultiList<double>   IndifferenceEquations()                 const;
  gbtPolyMultiList<double>   LastActionProbPositiveInequalities()    const;
  gbtPolyMultiList<double>   NashOnSupportEquationsAndInequalities() const;
  gbtList<gbtVector<double> > 
               NashOnSupportSolnVectors(const gbtPolyMultiList<double> &equations,
					const gRectangle<double> &Cube,
					gbtStatus &p_status);

  int SaveSolutions(const gbtList<gbtVector<double> > &list);
public:
  PolEnumModule(const gbtGame &, const PolEnumParams &p);
  
  int PolEnum(gbtStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  PolEnumParams &Parameters(void);

  const gbtList<gbtMixedProfile<double> > &GetSolutions(void) const;
  gbtVector<double> SolVarsFromMixedProfile(const gbtMixedProfile<double> &) 
                                                                     const;

  const int PolishKnownRoot(gbtVector<double> &) const;

  gbtMixedProfile<double>  ReturnPolishedSolution(const gbtVector<double> &) const;

  bool IsSingular() const;
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

class gbtNfgNashEnumPoly {
private:
  int m_stopAfter;

public:
  gbtNfgNashEnumPoly(void);
  virtual ~gbtNfgNashEnumPoly() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  std::string GetAlgorithm(void) const { return "PolEnum[NFG]"; }
  gbtList<gbtMixedProfile<double> > Solve(const gbtGame &, gbtStatus &);
};

PolEnumModule::PolEnumModule(const gbtGame &p_game, const PolEnumParams &p)
  : m_nfg(p_game), params(p), 
    Space(p_game->StrategyProfileLength() - p_game->NumPlayers()), 
    Lex(&Space, lex), 
    num_vars(p_game->StrategyProfileLength() - p_game->NumPlayers()), 
    count(0), nevals(0), is_singular(false)
{ 
//  gEpsilon(eps,12);
}


int PolEnumModule::PolEnum(gbtStatus &p_status)
{
  gbtPolyMultiList<double> equations = NashOnSupportEquationsAndInequalities();

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
  gbtVector<double> bottoms(num_vars), tops(num_vars);
  bottoms = (double)0;
  tops = (double)1;
 
  gRectangle<double> Cube(bottoms, tops); 

  // start gbtPolyQuickSolve
  gbtList<gbtVector<double> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube,
								   p_status);

  int index = SaveSolutions(solutionlist);
  return index;	 
}

int PolEnumModule::SaveSolutions(const gbtList<gbtVector<double> > &list)
{
  gbtMixedProfile<double> profile = m_nfg->NewMixedProfile(0.0);
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=m_nfg->NumPlayers();i++) {
      sum=0;
      for(j=1;j<m_nfg->GetPlayer(i)->NumStrategies();j++) {
	profile(i,j) = (list[k][j+kk]);
	sum+=profile(i,j);
      }
      profile(i,j) = (double)1.0 - sum;
      kk+=(m_nfg->GetPlayer(i)->NumStrategies()-1);
    }
    solutions.Append(profile);
  }
  return index;
}

bool PolEnumModule::EqZero(double x) const
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

const gbtList<gbtMixedProfile<double> > &PolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gbtPolyMulti<double> PolEnumModule::Prob(int p, int strat) const
{
  gbtPolyMulti<double> equation(&Space,&Lex);
  gbtVector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(m_nfg->GetPlayer(i)->NumStrategies()-1);

  if(strat<m_nfg->GetPlayer(p)->NumStrategies()) {
    exps=0;
    exps[strat+kk]=1;
    gbtPolyExponent const_exp(&Space,exps);
    gbtMonomial<double> const_term((double)1,const_exp);
    gbtPolyMulti<double> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<m_nfg->GetPlayer(p)->NumStrategies();j++) {
      exps=0;
      exps[j+kk]=1;
      gbtPolyExponent exponent(&Space,exps);
      gbtMonomial<double> term((double)(-1),exponent);
      gbtPolyMulti<double> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    gbtPolyExponent const_exp(&Space,exps);
    gbtMonomial<double> const_term((double)1,const_exp);
    gbtPolyMulti<double> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gbtPolyMulti<double> 
PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  gbtNfgContingency profile = m_nfg->NewContingency();

  gbtNfgContIterator A(m_nfg), B(m_nfg);
  A.Freeze(m_nfg->GetPlayer(i)->GetStrategy(strat1));
  B.Freeze(m_nfg->GetPlayer(i)->GetStrategy(strat2));
  gbtPolyMulti<double> equation(&Space,&Lex);
  do {
    gbtPolyMulti<double> term(&Space,(double)1,&Lex);
    profile = A.GetProfile();
    int k;
    for(k=1;k<=m_nfg->NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k, profile->GetStrategy(m_nfg->GetPlayer(k))->GetId());
    double coeff,ap,bp;
    ap = (double) A.GetPayoff(m_nfg->GetPlayer(i));
    bp = (double) B.GetPayoff(m_nfg->GetPlayer(i));
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}


gbtPolyMultiList<double>   PolEnumModule::IndifferenceEquations()  const
{
  gbtPolyMultiList<double> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg->NumPlayers();pl++) 
    for(int j=1;j<m_nfg->GetPlayer(pl)->NumStrategies();j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gbtPolyMultiList<double> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gbtPolyMultiList<double> equations(&Space,&Lex);

  for(int pl=1;pl<=m_nfg->NumPlayers();pl++)
    if(m_nfg->GetPlayer(pl)->NumStrategies()>2) 
      equations+=Prob(pl,m_nfg->GetPlayer(pl)->NumStrategies());

  return equations;
}

gbtPolyMultiList<double> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gbtPolyMultiList<double> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


gbtList<gbtVector<double> > 
PolEnumModule::NashOnSupportSolnVectors(const gbtPolyMultiList<double> &equations,
					      const gRectangle<double> &Cube,
					      gbtStatus &p_status)
{  
  gbtPolyQuickSolve<double> quickie(equations, p_status);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647,params.stopAfter);
  }
  catch (gbtInterruptException &) { }
  catch (gbtSingularMatrixException &) {
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

int PolEnum(const gbtGame &p_game, const PolEnumParams &params,
	    gbtList<gbtMixedProfile<double> > &solutions, gbtStatus &p_status,
	    long &nevals, double &time, bool &is_singular)
{
  PolEnumModule module(p_game, params);
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

gbtMixedProfile<double>  
PolishEquilibrium(const gbtGame &p_game,
		  const gbtMixedProfile<double>  &sol, 
		  bool &is_singular)
{
  PolEnumParams params;
  PolEnumModule module(p_game, params);
  gbtVector<double> vec = module.SolVarsFromMixedProfile(sol);

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


gbtVector<double> 
PolEnumModule::SolVarsFromMixedProfile(const gbtMixedProfile<double> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) 
    numvars += m_nfg->GetPlayer(pl)->NumStrategies() - 1;

  gbtVector<double> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) 
    for (int j = 1; j < m_nfg->GetPlayer(pl)->NumStrategies(); j++) {
      count ++;
      answer[count] = (double)sol(pl,j);
    }

  return answer;
}

const int PolEnumModule::PolishKnownRoot(gbtVector<double> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gbtPolyMultiList<double> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start gbtPolyQuickSolve
    gbtNullStatus gstatus;
    gbtPolyQuickSolve<double> quickie(equations, gstatus);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (gbtInterruptException &) { }
    catch (gbtSingularMatrixException &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

gbtMixedProfile<double>  
PolEnumModule::ReturnPolishedSolution(const gbtVector<double> &root) const
{
  gbtMixedProfile<double> profile = m_nfg->NewMixedProfile(0.0);

  int j;
  int kk=0;
  for(int pl=1;pl<=m_nfg->NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<m_nfg->GetPlayer(pl)->NumStrategies();j++) {
      profile(pl,j) = (root[j+kk]);
      sum+=profile(pl,j);
    }
    profile(pl,j) = (double)1.0 - sum;
    kk+=(m_nfg->GetPlayer(pl)->NumStrategies()-1);
  }
 
  return profile;
}

//========================================================================
//                      class gbtNfgNashEnumPoly
//========================================================================

gbtNfgNashEnumPoly::gbtNfgNashEnumPoly(void)
  : m_stopAfter(0)
{ }

gbtList<gbtMixedProfile<double> > 
gbtNfgNashEnumPoly::Solve(const gbtGame &p_game, gbtStatus &p_status)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  // Temporarily disabled...
  //  gbtList<gbtNfgSupport> supports = PossibleNashSubsupports(p_support,
  //							    p_status);

  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";

  //  gbtList<gbtNfgSupport> singularSupports;
  gbtList<gbtMixedProfile<double> > solutions;

  try { 
    p_status.Get();
    p_status.SetProgress(0.0);
    long newevals = 0;
    double newtime = 0.0;
    gbtList<gbtMixedProfile<double> > newsolns;
    bool is_singular = false;
      
    PolEnumParams params;
    params.stopAfter = 0;
    PolEnum(p_game, params, newsolns, p_status,
	    newevals, newtime, is_singular);
      
    for (int j = 1; j <= newsolns.Length(); j++) {
      // FIXME: what about Nash check?
      //if (newsolns[j].IsNash()) {
      solutions += newsolns[j];
      //}
    }
    
    //if (is_singular) { 
    //  singularSupports += supports[i];
    //  }
  }
  catch (gbtInterruptException &) {
    // catch exception; return list of computed equilibria (if any)
  }

  return solutions;
}

gbtList<gbtMixedProfile<double> > gbtNashEnumPolyNfg(const gbtGame &p_game)
{
  gbtNfgNashEnumPoly algorithm;
  gbtNullStatus status;
  return algorithm.Solve(p_game, status);
}
