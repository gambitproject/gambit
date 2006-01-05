//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria of a game, via polynomial equations
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

#include "efgensup.h"
#include "sfg.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"
#include "behavextend.h"

int g_numDecimals = 6;
bool g_verbose = false;

template <class T> class EfgPolEnumModule  {
private:
  T eps;
  const Gambit::Game &EF;
  const Gambit::BehavSupport &support;
  gSpace *Space;
  term_order *Lex;
  int num_vars;
  long count,nevals;
  double time;
  gbtList<Gambit::MixedBehavProfile<double> > solutions;
  const Sfg SF;
  bool is_singular;
  gbtArray<gbtArray<int> * > var;

  // The strategy is to develop the polynomial for each agent's expected
  // payoff as a function of the behavior strategies on the support,
  // eliminating the last last action probability for each information set.
  // The system is obtained by requiring that all of the partial
  // derivatives vanish, and that the sum of action probabilities at
  // each information set be less than one.
  gPoly<T>     ProbOfSequence(int pl,int seq)          const;
  gPoly<T>     Payoff(int pl)                          const;
  gPolyList<T> IndifferenceEquations()                 const;
  gPolyList<T> LastActionProbPositiveInequalities()    const;
  gPolyList<T> NashOnSupportEquationsAndInequalities() const;
  gbtList<gbtVector<gDouble> > 
               NashOnSupportSolnVectors(const gPolyList<T> &equations,
					const gRectangle<T> &Cube);

  // Pass to the sequence form variables from the solution variables
  double NumProbOfSequence(int pl,int seq, const gbtVector<gDouble> &x) const;
  gbtPVector<double> SeqFormVectorFromSolFormVector(const gbtVector<gDouble> &x)
                                                                      const;

  bool ExtendsToANFNash(const Gambit::MixedBehavProfile<double> &)             const;
  int SaveANFNashSolutions(const gbtList<gbtVector<gDouble> > &list);
  bool ExtendsToNash(const Gambit::MixedBehavProfile<double> &)                const;
  int SaveNashSolutions(const gbtList<gbtVector<gDouble> > &list);

public:
  EfgPolEnumModule(const Gambit::BehavSupport &);
  ~EfgPolEnumModule();

  int EfgPolEnum(void);

  long             NumEvals(void)   const;
  double           Time(void)       const;
  bool             IsSingular(void) const;

  const gbtList<Gambit::MixedBehavProfile<double> > &GetSolutions(void) const;

  // Passing between variables of polynomials and sequence form probs
  gbtPVector<double> SeqFormProbsFromSolVars(const gbtVector<gDouble> &) const;
  gbtVector<gDouble> SolVarsFromSeqFormProbs(const gbtPVector<double> &) const;
  gbtVector<gDouble> SolVarsFromBehavProfile(const Gambit::MixedBehavProfile<gbtNumber> &) 
                                                                     const;

  const int PolishKnownRoot(gbtVector<gDouble> &) const;

  Gambit::MixedBehavProfile<double> ReturnPolishedSolution(const gbtVector<gDouble> &) const;
};

//-------------------------------------------------------------------------
//                    EfgPolEnumModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
EfgPolEnumModule<T>::EfgPolEnumModule(const Gambit::BehavSupport &S)
  : EF(S.GetGame()), support(S), count(0), nevals(0), SF(S),
    is_singular(false), var(S.GetGame()->NumPlayers())
{ 
//  gEpsilon(eps,12);

  num_vars = SF.TotalNumSequences()-SF.NumPlayerInfosets()-SF.NumPlayers();

  Space = new gSpace(num_vars);
  Lex = new term_order(Space, lex);

  int kk=0;
  int tnv = 0;

  for(int i=1;i<=EF->NumPlayers();i++) {
    var[i] = new gbtArray<int>(SF.NumSequences(i));
    (*(var[i]))[1] = 0;
    for(int seq = 2;seq<=SF.NumSequences(i);seq++) {
      int act  = SF.ActionNumber(i,seq);
      if(act < support.NumActions(SF.GetInfoset(i,seq)))
	(*(var[i]))[seq] = ++tnv;
      else
	(*(var[i]))[seq] = 0;

    }
    kk+=(SF.NumSequences(i)-SF.NumInfosets(i)-1);
  }
  assert(tnv==num_vars);
}

template <class T>
EfgPolEnumModule<T>::~EfgPolEnumModule()
{ 
  for(int i=1;i<=EF->NumPlayers();i++)
    delete var[i];
  delete Lex;
  delete Space;
}

template <class T> 
gPoly<T> EfgPolEnumModule<T>::ProbOfSequence(int p, int seq) const
{
  gPoly<T> equation(Space,Lex);
  gbtVector<int> exps(num_vars);
  int j = 0;
  
  int isetrow = SF.InfosetRowNumber(p,seq);
  int act  = SF.ActionNumber(p,seq);
  int varno = (*(var[p]))[seq];

  if(seq==1) {
    exps=0;
    exp_vect const_exp(Space,exps);
    gMono<T> const_term((T)1,const_exp);
    gPoly<T> new_term(Space,const_term,Lex);
    equation+=new_term;
  }
  else if(act<support.NumActions(SF.GetInfoset(p,seq))) { 
    assert (varno>=exps.First());
    assert (varno<=exps.Last());
    exps=0;
    exps[varno]=1;
    exp_vect const_exp(Space,exps);
    gMono<T> const_term((T)1,const_exp);
    gPoly<T> new_term(Space,const_term,Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<seq;j++) {
      if((SF.Constraints(p))(isetrow,j)==gbtRational(-1))
	equation-=ProbOfSequence(p,j);
      if((SF.Constraints(p))(isetrow,j)==gbtRational(1))
	equation+=ProbOfSequence(p,j);
    }
  }
  return equation;
}

template <class T> gPoly<T> 
EfgPolEnumModule<T>::Payoff(int pl) const
{
  gIndexOdometer index(SF.NumSequences());
  gbtRational pay;

  gPoly<T> equation(Space,Lex);
  while (index.Turn()) {
    pay=SF.Payoff(index.CurrentIndices(),pl);
    if( pay != gbtRational(0)) {
      gPoly<T> term(Space,(T)pay,Lex);
      int k;
      for(k=1;k<=EF->NumPlayers();k++) 
	term*=ProbOfSequence(k,(index.CurrentIndices())[k]);
      equation+=term;
    }
  }
  return equation;
}

template <class T>  
gPolyList<T> EfgPolEnumModule<T>::IndifferenceEquations() const
{
  gPolyList<T> equations(Space,Lex);

  int kk = 0;
  for (int pl = 1; pl <= SF.NumPlayers(); pl++) {
    gPoly<T> payoff = Payoff(pl);
    int n_vars = SF.NumSequences(pl) - SF.NumInfosets(pl) - 1; 
    for (int j = 1; j <= n_vars; j++) 
      equations += payoff.PartialDerivative(kk+j);
    kk+=n_vars;
  }

  return equations;
}

template <class T>  
gPolyList<T> EfgPolEnumModule<T>::LastActionProbPositiveInequalities() const
{
  gPolyList<T> equations(Space,Lex);

  for (int i = 1; i <= SF.NumPlayers(); i++) 
    for (int j = 2; j <= SF.NumSequences(i); j++) {
      int act_num = SF.ActionNumber(i,j);
      if ( act_num == support.NumActions(SF.GetInfoset(i,j)) && act_num > 1 ) 
	equations += ProbOfSequence(i,j);
    }

  return equations;
}

template <class T>  
gPolyList<T> EfgPolEnumModule<T>::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<T> equations(Space,Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


template <class T> gbtList<gbtVector<gDouble> > 
EfgPolEnumModule<T>::NashOnSupportSolnVectors(const gPolyList<T> &equations,
					      const gRectangle<T> &Cube)
{
  QuikSolv<T> quickie(equations);
#ifdef UNUSED
  if(params.trace>0) {
    (*params.tracefile) << "\nThe equilibrium equations are \n" 
      << quickie.UnderlyingEquations() ;
  }  
#endif  // UNUSED

  // 2147483647 = 2^31-1 = MaxInt

  try {
    if(quickie.FindCertainNumberOfRoots(Cube,2147483647,0)) {
#ifdef UNUSED
      if(params.trace>0) {
	(*params.tracefile) << "\nThe system has the following roots in [0,1]^"
			    << num_vars << " :\n" << quickie.RootList();
      }
#endif  // UNUSED
    }
  }
  catch (Gambit::SingularMatrixException) {
    is_singular = true;
  }
  
  return quickie.RootList();
}

template <class T> int EfgPolEnumModule<T>::EfgPolEnum(void)
{
  gPolyList<T> equations = NashOnSupportEquationsAndInequalities();

  // set up the rectangle of search
  gbtVector<T> bottoms(num_vars), tops(num_vars);
  bottoms = (T)0;
  tops = (T)1;
  gRectangle<T> Cube(bottoms, tops); 

  gbtList<gbtVector<gDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube);

  int index = SaveNashSolutions(solutionlist);
  return index;	 
}

template <class T> gbtPVector<double> 
EfgPolEnumModule<T>::SeqFormVectorFromSolFormVector(const gbtVector<gDouble> &v)
                                                                      const
{
  gbtPVector<double> x(SF.NumSequences());

  for (int i = 1; i <= EF->NumPlayers(); i++) 
    for (int j = 1; j <= SF.NumSequences()[i]; j++)
      x(i,j) = NumProbOfSequence(i,j,v);
  
  return x;
}

template <class T> bool 
EfgPolEnumModule<T>::ExtendsToANFNash(const Gambit::MixedBehavProfile<double> &bs) const 
{
  algExtendsToAgentNash algorithm;
  return algorithm.ExtendsToAgentNash(bs, 
				      Gambit::BehavSupport(bs.GetGame()),
				      Gambit::BehavSupport(bs.GetGame()));
}

template <class T> int 
EfgPolEnumModule<T>::SaveANFNashSolutions(const gbtList<gbtVector<gDouble> > &list)
{
  int index=0;
  for (int k = 1; k <= list.Length(); k++) {
    gbtPVector<double> y = SeqFormVectorFromSolFormVector(list[k]);

    Gambit::MixedBehavProfile<double> sol(SF.ToBehav(y));

    if (ExtendsToNash(sol)) { 
      index = solutions.Append(sol);
    }
  }
  return index;
}

template <class T> bool 
EfgPolEnumModule<T>::ExtendsToNash(const Gambit::MixedBehavProfile<double> &bs) const 
{
  algExtendsToNash algorithm;
  return algorithm.ExtendsToNash(bs, 
				 Gambit::BehavSupport(bs.GetGame()),
				 Gambit::BehavSupport(bs.GetGame()));
}

template <class T> int 
EfgPolEnumModule<T>::SaveNashSolutions(const gbtList<gbtVector<gDouble> > &list)
{
  int index=0;
  for (int k = 1; k <= list.Length(); k++) {
    gbtPVector<double> y = SeqFormVectorFromSolFormVector(list[k]);

    Gambit::MixedBehavProfile<double> sol(SF.ToBehav(y));

    if(ExtendsToNash(sol)) { 
      index = solutions.Append(sol);
    }
  }
  return index;
}


template <class T> long EfgPolEnumModule<T>::NumEvals(void) const
{
  return nevals;
}

template <class T> double EfgPolEnumModule<T>::Time(void) const
{
  return time;
}

template <class T> bool EfgPolEnumModule<T>::IsSingular(void) const
{
  return is_singular;
}

template <class T>
const gbtList<Gambit::MixedBehavProfile<double> > &EfgPolEnumModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> double EfgPolEnumModule<T>::
NumProbOfSequence(int p,int seq, const gbtVector<gDouble> &x) const
{
  int j = 0;
  double value=0;

  int isetrow = SF.InfosetRowNumber(p,seq);
  int act  = SF.ActionNumber(p,seq);
  int varno = (*(var[p]))[seq];

  if(seq==1)
    return (double)1;
  else if(act<support.NumActions(SF.GetInfoset(p,seq)))
    return x[varno].ToDouble();
  else {    
    for(j=1;j<seq;j++) {
      if((SF.Constraints(p))(isetrow,j)==gbtRational(-1))
	value-=NumProbOfSequence(p,j,x);
      if((SF.Constraints(p))(isetrow,j)==gbtRational(1))
	value+=NumProbOfSequence(p,j,x);
    }
    return value;
  }
}

template <class T> gbtPVector<double> 
EfgPolEnumModule<T>::SeqFormProbsFromSolVars(const gbtVector<gDouble> &v) const
{
  gbtPVector<double> x(SF.NumSequences());

  for(int pl=1;pl<=EF->NumPlayers();pl++) 
    for(int seq=1;seq<=SF.NumSequences()[pl];seq++)
      x(pl,seq) = NumProbOfSequence(pl,seq,v);

  return x;
}

template <class T> gbtVector<gDouble> 
EfgPolEnumModule<T>::SolVarsFromBehavProfile(const Gambit::MixedBehavProfile<gbtNumber> &sol)
  const
{
  int numvars(0);

  for (int pl = 1; pl <= EF->NumPlayers(); pl++) {
    Gambit::GamePlayer player = EF->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      const Gambit::GameInfoset *infoset = player->GetInfoset(iset);
      if ( support.MayReach(infoset) )
	numvars += support.Actions(infoset).Length() - 1;
    }
  }

  gbtVector<gDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= EF->NumPlayers(); pl++) {
    Gambit::GamePlayer player = EF->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      const Gambit::GameInfoset *infoset = player->GetInfoset(iset);
      if ( support.MayReach(infoset) ) {
	const gbtArray<Gambit::GameAction *> acts = support.Actions(infoset);
	for (int act = 1; act <= acts.Length() - 1; act++) {
	  count++;
	  answer[count] = (gDouble)sol.GetActionProb(acts[act]);
	}
      }
    }
  }

  return answer;
}

template <class T> gbtVector<gDouble> 
EfgPolEnumModule<T>::SolVarsFromSeqFormProbs(const gbtPVector<double> &x) const
{
  

  // Old version that doesn't work 
  int numvars = 0;
  for(int pl=1;pl<=EF->NumPlayers();pl++) 
    for(int seq=2;seq<=SF.NumSequences()[pl];seq++) {
      int act  = SF.ActionNumber(pl,seq);
      if(act<support.NumActions(SF.GetInfoset(pl,seq))) 
	numvars++;
    }
  gbtVector<gDouble> v(numvars);
  //  gbtVector<gDouble> v(SF.NumIndepVars());

  int count = 0;
  for(int pl=1;pl<=EF->NumPlayers();pl++) 
    for(int seq=2;seq<=SF.NumSequences()[pl];seq++) {
      int act  = SF.ActionNumber(pl,seq);
      if(act<support.NumActions(SF.GetInfoset(pl,seq))) {
	count++;
	int varno = (*(var[pl]))[seq];
	v[count] = (gDouble)x[varno];
      }
    }

  return v;
}

template <class T> 
const int EfgPolEnumModule<T>::PolishKnownRoot(gbtVector<gDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    int i,j;
    
    
    gPolyList<T> equations(Space,Lex);
    
    // equations for equality of strat j to strat j+1
    
    int kk=0;
    for( i=1;i<=SF.NumPlayers();i++) {
      int n_vars=SF.NumSequences(i)-SF.NumInfosets(i)-1; 
      for(j=1;j<=n_vars;j++) 
	equations+=(Payoff(i)).PartialDerivative(kk+j);
      kk+=n_vars;
    }

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    QuikSolv<T> quickie(equations);
    
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

template <class T> Gambit::MixedBehavProfile<double> 
EfgPolEnumModule<T>::ReturnPolishedSolution(const gbtVector<gDouble> &root) const
{
  gbtPVector<double> x(SF.NumSequences());

  for(int i=1;i<=EF->NumPlayers();i++) 
    for(int j=1;j<=SF.NumSequences()[i];j++) 
      x(i,j) = NumProbOfSequence(i,j,root);

  Gambit::MixedBehavProfile<double> sol(SF.ToBehav(x));
  return sol;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedBehavProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

Gambit::MixedBehavProfile<double> ToFullSupport(const Gambit::MixedBehavProfile<double> &p_profile)
{
  Gambit::Game efg = p_profile.GetGame();
  const Gambit::BehavSupport &support = p_profile.GetSupport();

  Gambit::MixedBehavProfile<double> fullProfile(efg);
  for (int i = 1; i <= fullProfile.Length(); fullProfile[i++] = 0.0);

  int index = 1;
  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    Gambit::GamePlayer player = efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Gambit::GameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (support.Find(infoset->GetAction(act))) {
	  fullProfile(pl, iset, act) = p_profile[index++];
	}
      }
    }
  }

  return fullProfile;
}

int EfgPolEnum(const Gambit::BehavSupport &support, 
	       gbtList<Gambit::MixedBehavProfile<double> > &solutions,
	       long &nevals, double &time, bool &is_singular)
{
  EfgPolEnumModule<gDouble> module(support);
  module.EfgPolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  is_singular = module.IsSingular();

  return 1;
}

void PrintSupport(std::ostream &p_stream,
		  const std::string &p_label, const Gambit::BehavSupport &p_support)
{
  p_stream << p_label;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = p_support.GetGame()->GetPlayer(pl);

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Gambit::GameInfoset infoset = player->GetInfoset(iset);

      p_stream << ",";

      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (p_support.Find(infoset->GetAction(act))) {
	  p_stream << "1";
	}
	else {
	  p_stream << "0";
	}
      }
    }
  }
  p_stream << std::endl;
}

void Solve(const Gambit::Game &p_game)
{
  gbtList<Gambit::BehavSupport> supports = PossibleNashSubsupports(p_game);

  try {
    for (int i = 1; i <= supports.Length(); i++) {
      long newevals = 0;
      double newtime = 0.0;
      gbtList<Gambit::MixedBehavProfile<double> > newsolns;
      bool is_singular = false;

      if (g_verbose) {
	PrintSupport(std::cout, "candidate", supports[i]);
      }
      
      EfgPolEnum(supports[i], newsolns, newevals, newtime, is_singular);
      for (int j = 1; j <= newsolns.Length(); j++) {
	Gambit::MixedBehavProfile<double> fullProfile = ToFullSupport(newsolns[j]);
	if (fullProfile.GetLiapValueOnDefined() < 1.0e-6) {
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
  std::cerr << "Accepts extensive game on standard input.\n";
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

  Gambit::Game efg;

  try {
    efg = Gambit::ReadGame(std::cin);
  }
  catch (...) {
    return 1;
  }

  Solve(efg);

  return 0;
}
