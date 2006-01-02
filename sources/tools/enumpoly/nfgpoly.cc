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

#include <iostream>
#include <iomanip>

#include "nfgensup.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"

int g_numDecimals = 6;
bool g_verbose = false;

class PolEnumModule  {
private:
  gDouble eps;
  Gambit::GameTable NF;
  const gbtNfgSupport &support;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long count,nevals;
  double time;
  gbtList<gbtMixedProfile<double> > solutions;
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
  gbtList<gbtVector<gDouble> > 
               NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					const gRectangle<gDouble> &Cube);

  int SaveSolutions(const gbtList<gbtVector<gDouble> > &list);
public:
  PolEnumModule(const gbtNfgSupport &);
  
  int PolEnum(void);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  const gbtList<gbtMixedProfile<double> > &GetSolutions(void) const;
  gbtVector<gDouble> SolVarsFromgbtMixedProfile(const gbtMixedProfile<double> &) const;

  const int PolishKnownRoot(gbtVector<gDouble> &) const;

  gbtMixedProfile<double> ReturnPolishedSolution(const gbtVector<gDouble> &) const;

  bool IsSingular() const;
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

PolEnumModule::PolEnumModule(const gbtNfgSupport &S)
  : NF(S.GetGame()), support(S),
    Space(support.ProfileLength()-NF->NumPlayers()), 
    Lex(&Space, lex), num_vars(support.ProfileLength()-NF->NumPlayers()), 
    count(0), nevals(0), is_singular(false)
{ 
//  gEpsilon(eps,12);
}


int PolEnumModule::PolEnum(void)
{
  gPolyList<gDouble> equations = NashOnSupportEquationsAndInequalities();

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
  gbtVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
 
  gRectangle<gDouble> Cube(bottoms, tops); 

  // start QuikSolv
  gbtList<gbtVector<gDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube);

  int index = SaveSolutions(solutionlist);
  return index;	 
}

int PolEnumModule::SaveSolutions(const gbtList<gbtVector<gDouble> > &list)
{
  gbtMixedProfile<double> profile(support);
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=NF->NumPlayers();i++) {
      sum=0;
      for(j=1;j<support.NumStrats(i);j++) {
	profile(i,j) = (list[k][j+kk]).ToDouble();
	sum+=profile(i,j);
      }
      profile(i,j) = (double)1.0 - sum;
      kk+=(support.NumStrats(i)-1);
    }
    index = solutions.Append(profile);
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

const gbtList<gbtMixedProfile<double> > &PolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gPoly<gDouble> PolEnumModule::Prob(int p, int strat) const
{
  gPoly<gDouble> equation(&Space,&Lex);
  gbtVector<int> exps(num_vars);
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
  gbtStrategyProfile profile(NF);

  gbtNfgContingencyIterator A(support, i, strat1);
  gbtNfgContingencyIterator B(support, i, strat2);

  gPoly<gDouble> equation(&Space,&Lex);
  do {
    gPoly<gDouble> term(&Space,(gDouble)1,&Lex);
    profile = A.GetProfile();
    int k;
    for(k=1;k<=NF->NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.GetIndex(profile.GetStrategy(k)));
    gDouble coeff,ap,bp;
    ap = (double) A.GetPayoff(i);
    bp = (double) B.GetPayoff(i);
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

  for(int pl=1;pl<=NF->NumPlayers();pl++) 
    for(int j=1;j<support.NumStrats(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gPolyList<gDouble> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=NF->NumPlayers();pl++)
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


gbtList<gbtVector<gDouble> > 
PolEnumModule::NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					const gRectangle<gDouble> &Cube)
{  
  QuikSolv<gDouble> quickie(equations);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647,0);
  }
  catch (Gambit::SingularMatrixException) {
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

int PolEnum(const gbtNfgSupport &support,
	    gbtList<gbtMixedProfile<double> > &solutions, 
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

gbtMixedProfile<double> PolishEquilibrium(const gbtNfgSupport &support, 
				       const gbtMixedProfile<double> &sol, 
				       bool &is_singular)
{
  PolEnumModule module(support);
  gbtVector<gDouble> vec = module.SolVarsFromgbtMixedProfile(sol);

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


gbtVector<gDouble> 
PolEnumModule::SolVarsFromgbtMixedProfile(const gbtMixedProfile<double> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    numvars += support.NumStrats(pl) - 1;

  gbtVector<gDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrats(pl); j++) {
      count ++;
      answer[count] = (gDouble)sol(pl,j);
    }

  return answer;
}

const int PolEnumModule::PolishKnownRoot(gbtVector<gDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gPolyList<gDouble> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    QuikSolv<gDouble> quickie(equations);
    
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

gbtMixedProfile<double>
PolEnumModule::ReturnPolishedSolution(const gbtVector<gDouble> &root) const
{
  gbtMixedProfile<double> profile(support);

  int j;
  int kk=0;
  for(int pl=1;pl<=NF->NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<support.NumStrats(pl);j++) {
      profile(pl,j) = (root[j+kk]).ToDouble();
      sum+=profile(pl,j);
    }
    profile(pl,j) = (double)1.0 - sum;
    kk+=(support.NumStrats(pl)-1);
  }
       
  return profile;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtMixedProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

gbtMixedProfile<double> ToFullSupport(const gbtMixedProfile<double> &p_profile)
{
  Gambit::GameTable nfg = p_profile.GetGame();
  const gbtNfgSupport &support = p_profile.GetSupport();

  gbtMixedProfile<double> fullProfile(nfg);
  for (int i = 1; i <= fullProfile.Length(); fullProfile[i++] = 0.0);

  int index = 1;
  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    Gambit::TablePlayer player = nfg->GetPlayer(pl);
    for (int st = 1; st <= nfg->NumStrats(pl); st++) {
      if (support.Contains(player->GetStrategy(st))) {
	fullProfile(pl, st) = p_profile[index++];
      }
    }
  }

  return fullProfile;
}

void PrintSupport(std::ostream &p_stream, 
		  const std::string &p_label, const gbtNfgSupport &p_support)
{
  p_stream << p_label;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    Gambit::TablePlayer player = p_support.GetGame()->GetPlayer(pl);

    p_stream << ",";
    for (int st = 1; st <= player->NumStrats(); st++) {
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

void Solve(const Gambit::GameTable &p_nfg)
{
  gbtList<gbtNfgSupport> supports = PossibleNashSubsupports(p_nfg);

  try {
    for (int i = 1; i <= supports.Length(); i++) {
      long newevals = 0;
      double newtime = 0.0;
      gbtList<gbtMixedProfile<double> > newsolns;
      bool is_singular = false;
    
      if (g_verbose) {
	PrintSupport(std::cout, "candidate", supports[i]);
      }

      PolEnum(supports[i], newsolns, newevals, newtime, is_singular);
      
      for (int j = 1; j <= newsolns.Length(); j++) {
	gbtMixedProfile<double> fullProfile = ToFullSupport(newsolns[j]);
	if (fullProfile.GetLiapValue() < 1.0e-6) {
	  PrintProfile(std::cout, "NE", fullProfile);
	}
      }

      if (is_singular && g_verbose) {
	PrintSupport(std::cout, "singular", supports[i]);
      }
    }
  }
  catch (...) {
    return;
  }
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving polynomial systems\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts strategic game on standard input.\n";
  std::cerr << "With no options, reports all Nash equilibria found.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibrium probabilities with DECIMALS digits\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v               verbose mode (shows supports investigated)\n";
  std::cerr << "                   (default is only to show equilibria)\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false;

  int c;
  while ((c = getopt(argc, argv, "d:hqv")) != -1) {
    switch (c) {
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case 'v':
      g_verbose = true;
      break;
    case '?':
      if (isprint(optopt)) {
	std::cerr << argv[0] << ": Unknown option `-" << ((char) optopt) << "'.\n";
      }
      else {
	std::cerr << argv[0] << ": Unknown option character `\\x" << optopt << "`.\n";
      }
      return 1;
    default:
      abort();
    }
  }

  if (!quiet) {
    PrintBanner(std::cerr);
  }

  Gambit::GameTable nfg;

  try {
    nfg = Gambit::ReadNfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  Solve(nfg);
  
  return 0;
}






