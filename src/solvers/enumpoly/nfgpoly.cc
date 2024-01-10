//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgpoly.cc
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of polynomial equations
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

#include <iostream>
#include <iomanip>

#include "nfgensup.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"

extern int g_numDecimals;
extern bool g_verbose; 

class PolEnumModule  {
private:
  double eps;
  Gambit::Game NF;
  const Gambit::StrategySupportProfile &support;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long nevals;
  double time;
  Gambit::List<Gambit::MixedStrategyProfile<double> > solutions;
  bool is_singular;

  bool EqZero(double x) const;
  
  // p_i_j as a gPoly, with last prob in terms of previous probs
  gPoly<double> Prob(int i,int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.  
  gPoly<double> IndifferenceEquation(int i, int strat1, int strat2) const;
  gPolyList<double>   IndifferenceEquations()                 const;
  gPolyList<double>   LastActionProbPositiveInequalities()    const;
  gPolyList<double>   NashOnSupportEquationsAndInequalities() const;
  Gambit::List<Gambit::Vector<double> > 
               NashOnSupportSolnVectors(const gPolyList<double> &equations,
					const gRectangle<double> &Cube);

  int SaveSolutions(const Gambit::List<Gambit::Vector<double> > &list);
public:
  explicit PolEnumModule(const Gambit::StrategySupportProfile &);
  
  int PolEnum();
  
  long NumEvals() const;
  double Time() const;
  
  const Gambit::List<Gambit::MixedStrategyProfile<double> > &GetSolutions() const;
  Gambit::Vector<double> SolVarsFromMixedProfile(const Gambit::MixedStrategyProfile<double> &) const;

  int PolishKnownRoot(Gambit::Vector<double> &) const;

  Gambit::MixedStrategyProfile<double> ReturnPolishedSolution(const Gambit::Vector<double> &) const;

  bool IsSingular() const;
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

PolEnumModule::PolEnumModule(const Gambit::StrategySupportProfile &S)
  : NF(S.GetGame()), support(S),
    Space(support.MixedProfileLength()-NF->NumPlayers()), 
    Lex(&Space, lex), num_vars(support.MixedProfileLength()-NF->NumPlayers()), 
    nevals(0), is_singular(false)
{ 
//  Gambit::Epsilon(eps,12);
}


int PolEnumModule::PolEnum()
{
  gPolyList<double> equations = NashOnSupportEquationsAndInequalities();

  /*
  // equations for equality of strat j to strat j+1
  for( i=1;i<=NF->NumPlayers();i++) 
    for(j=1;j<support.NumStrats(i);j++) 
      equations+=IndifferenceEquation(i,j,j+1);

  for( i=1;i<=NF->NumPlayers();i++)
    if(support.NumStrats(i)>2) 
      equations+=Prob(i,support.NumStrats(i));
  */

  // set up the rectangle of search
  Gambit::Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = (double)0;
  tops = (double)1;
 
  gRectangle<double> Cube(bottoms, tops); 

  // start QuikSolv
  Gambit::List<Gambit::Vector<double> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube);

  int index = SaveSolutions(solutionlist);
  return index;	 
}

int PolEnumModule::SaveSolutions(const Gambit::List<Gambit::Vector<double> > &list)
{
  Gambit::MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>());
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=NF->NumPlayers();i++) {
      sum=0;
      for(j=1;j<support.NumStrategies(i);j++) {
	profile[support.GetStrategy(i,j)] = list[k][j+kk];
	sum+=profile[support.GetStrategy(i,j)];
      }
      profile[support.GetStrategy(i,j)] = (double)1.0 - sum;
      kk+=(support.NumStrategies(i)-1);
    }
    solutions.push_back(profile);
    index = solutions.size();
  }
  return index;
}

bool PolEnumModule::EqZero(double x) const
{
  if(x <= eps && x >= -eps) return true;
  return false;
}     

long PolEnumModule::NumEvals() const
{
  return nevals;
}

double PolEnumModule::Time() const
{
  return time;
}

const Gambit::List<Gambit::MixedStrategyProfile<double> > &PolEnumModule::GetSolutions() const
{
  return solutions;
}

gPoly<double> PolEnumModule::Prob(int p, int strat) const
{
  gPoly<double> equation(&Space,&Lex);
  Gambit::Vector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(support.NumStrategies(i)-1);

  if(strat<support.NumStrategies(p)) {
    exps=0;
    exps[strat+kk]=1;
    exp_vect const_exp(&Space,exps);
    gMono<double> const_term((double)1,const_exp);
    gPoly<double> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<support.NumStrategies(p);j++) {
      exps=0;
      exps[j+kk]=1;
      exp_vect exponent(&Space,exps);
      gMono<double> term((double)(-1),exponent);
      gPoly<double> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    exp_vect const_exp(&Space,exps);
    gMono<double> const_term((double)1,const_exp);
    gPoly<double> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gPoly<double> 
PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  gPoly<double> equation(&Space,&Lex);

  for (Gambit::StrategyProfileIterator A(support, i, strat1), B(support, i, strat2);
       !A.AtEnd(); A++, B++) {
    gPoly<double> term(&Space, (double) 1, &Lex);
    for (int k = 1; k <= NF->NumPlayers(); k++) {
      if (i != k) {
        term *= Prob(k, support.GetStrategies(NF->GetPlayer(k)).Find((*A)->GetStrategy(k)));
      }
    }
    double coeff, ap, bp;
    ap = (*A)->GetPayoff(i);
    bp = (*B)->GetPayoff(i);
    coeff = ap - bp;
    term *= coeff;
    equation += term;
  }
  return equation;
}


gPolyList<double>   PolEnumModule::IndifferenceEquations()  const
{
  gPolyList<double> equations(&Space,&Lex);

  for(int pl=1;pl<=NF->NumPlayers();pl++) 
    for(int j=1;j<support.NumStrategies(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gPolyList<double> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<double> equations(&Space,&Lex);

  for(int pl=1;pl<=NF->NumPlayers();pl++)
    if(support.NumStrategies(pl)>2) 
      equations+=Prob(pl,support.NumStrategies(pl));

  return equations;
}

gPolyList<double> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<double> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


Gambit::List<Gambit::Vector<double> > 
PolEnumModule::NashOnSupportSolnVectors(const gPolyList<double> &equations,
					const gRectangle<double> &Cube)
{  
  QuikSolv<double> quickie(equations);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647,0);
  }
  catch (const Gambit::SingularMatrixException &) {
    is_singular = true;
  }
  catch (const Gambit::AssertionException &e) {
    std::cerr << "Assertion warning: " << e.what() << std::endl;
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

int PolEnum(const Gambit::StrategySupportProfile &support,
	    Gambit::List<Gambit::MixedStrategyProfile<double> > &solutions, 
	    long &nevals, double &time, bool &is_singular)
{
  PolEnumModule module(support);
  module.PolEnum();
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

#ifdef UNUSED
static
Gambit::MixedStrategyProfile<double> PolishEquilibrium(const Gambit::StrategySupportProfile &support,
				       const Gambit::MixedStrategyProfile<double> &sol, 
				       bool &is_singular)
{
  PolEnumModule module(support);
  Gambit::Vector<double> vec = module.SolVarsFromMixedProfile(sol);

  /* //DEBUG
  Gambit::PVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  Gambit::MixedStrategyProfile<gbtNumber> newsol = module.SequenceForm().ToMixed(xx);

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
#endif  // UNUSED

Gambit::Vector<double> 
PolEnumModule::SolVarsFromMixedProfile(const Gambit::MixedStrategyProfile<double> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    numvars += support.NumStrategies(pl) - 1;

  Gambit::Vector<double> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrategies(pl); j++) {
      count ++;
      answer[count] = (double)sol[support.GetStrategy(pl,j)];
    }

  return answer;
}

int PolEnumModule::PolishKnownRoot(Gambit::Vector<double> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gPolyList<double> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    QuikSolv<double> quickie(equations);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (Gambit::SingularMatrixException &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

Gambit::MixedStrategyProfile<double>
PolEnumModule::ReturnPolishedSolution(const Gambit::Vector<double> &root) const
{
  Gambit::MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>());

  int j;
  int kk=0;
  for(int pl=1;pl<=NF->NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<support.NumStrategies(pl);j++) {
      profile[support.GetStrategy(pl,j)] = root[j+kk];
      sum+=profile[support.GetStrategy(pl,j)];
    }
    profile[support.GetStrategy(pl,j)] = (double)1.0 - sum;
    kk+=(support.NumStrategies(pl)-1);
  }
       
  return profile;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

Gambit::MixedStrategyProfile<double> ToFullSupport(const Gambit::MixedStrategyProfile<double> &p_profile)
{
  Gambit::Game nfg = p_profile.GetGame();
  const Gambit::StrategySupportProfile &support = p_profile.GetSupport();

  Gambit::MixedStrategyProfile<double> fullProfile(nfg->NewMixedStrategyProfile(0.0));
  for (int i = 1; i <= fullProfile.MixedProfileLength(); fullProfile[i++] = 0.0);

  int index = 1;
  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    Gambit::GamePlayer player = nfg->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (support.Contains(player->GetStrategy(st))) {
	fullProfile[player->GetStrategy(st)] = p_profile[index++];
      }
    }
  }

  return fullProfile;
}

void PrintSupport(std::ostream &p_stream, 
		  const std::string &p_label, const Gambit::StrategySupportProfile &p_support)
{
  p_stream << p_label;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = p_support.GetGame()->GetPlayer(pl);

    p_stream << ",";
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (p_support.Contains(player->GetStrategy(st))) {
	p_stream << "1";
      }
      else {
	p_stream << "0";
      }
    }
  }
  p_stream << std::endl;
}

void EnumPolySolveStrategic(const Gambit::Game &p_nfg)
{
  Gambit::List<Gambit::StrategySupportProfile> supports = PossibleNashSubsupports(p_nfg);

  for (int i = 1; i <= supports.Length(); i++) {
    long newevals = 0;
    double newtime = 0.0;
    Gambit::List<Gambit::MixedStrategyProfile<double> > newsolns;
    bool is_singular = false;
    
    if (g_verbose) {
      PrintSupport(std::cout, "candidate", supports[i]);
    }

    PolEnum(supports[i], newsolns, newevals, newtime, is_singular);
      
    for (int j = 1; j <= newsolns.Length(); j++) {
      Gambit::MixedStrategyProfile<double> fullProfile = ToFullSupport(newsolns[j]);
      if (fullProfile.GetLiapValue() < 1.0e-6) {
	PrintProfile(std::cout, "NE", fullProfile);
      }
    }

    if (is_singular && g_verbose) {
      PrintSupport(std::cout, "singular", supports[i]);
    }
  }
}






