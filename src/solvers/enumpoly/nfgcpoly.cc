//
// This file is part of Gambit
// Copyright (c) 1994-2024, Litao Wei and The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgcpoly.cc
// Compute Nash equilibria via heuristic search on game supports
// (Porter, Nudelman & Shoham, 2004)
// Implemented by Litao Wei
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

#include "nfgcpoly.h"

//-------------------------------------------------------------------------
//              HeuristicPolEnumModule: Member functions
//-------------------------------------------------------------------------

HeuristicPolEnumModule::HeuristicPolEnumModule(const StrategySupportProfile &S, int p_stopAfter)
  : m_stopAfter(p_stopAfter), NF(S.GetGame()), support(S),
    Space(support.MixedProfileLength() - NF->NumPlayers()), Lex(&Space, lex),
    num_vars(support.MixedProfileLength() - NF->NumPlayers()), nevals(0), is_singular(false)
{
  //  gEpsilon(eps,12);
}

int HeuristicPolEnumModule::PolEnum()
{
  gPolyList<double> equations = NashOnSupportEquationsAndInequalities();

  /*
  // equations for equality of strat j to strat j+1
  for( i=1;i<=NF.NumPlayers();i++)
    for(j=1;j<support.NumStrats(i);j++)
      equations+=IndifferenceEquation(i,j,j+1);

  for( i=1;i<=NF.NumPlayers();i++)
    if(support.NumStrats(i)>2)
      equations+=Prob(i,support.NumStrats(i));
  */

  // set up the rectangle of search
  Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = (double)0;
  tops = (double)1;

  gRectangle<double> Cube(bottoms, tops);

  // start QuikSolv
  Gambit::List<Vector<double>> solutionlist = NashOnSupportSolnVectors(equations, Cube);

  int index = SaveSolutions(solutionlist);
  return index;
}

int HeuristicPolEnumModule::SaveSolutions(const Gambit::List<Vector<double>> &list)
{
  MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>());
  int i, j, k, kk, index = 0;
  double sum;

  for (k = 1; k <= list.Length(); k++) {
    kk = 0;
    for (i = 1; i <= NF->NumPlayers(); i++) {
      sum = 0;
      for (j = 1; j < support.NumStrategies(i); j++) {
        profile[support.GetStrategy(i, j)] = list[k][j + kk];
        sum += profile[support.GetStrategy(i, j)];
      }
      profile[support.GetStrategy(i, j)] = (double)1.0 - sum;
      kk += (support.NumStrategies(i) - 1);
    }
    solutions.push_back(profile);
    index = solutions.size();
  }
  return index;
}

bool HeuristicPolEnumModule::EqZero(double x) const
{
  if (x <= eps && x >= -eps) {
    return true;
  }
  return false;
}

long HeuristicPolEnumModule::NumEvals() const { return nevals; }

double HeuristicPolEnumModule::Time() const { return time; }

const Gambit::List<MixedStrategyProfile<double>> &HeuristicPolEnumModule::GetSolutions() const
{
  return solutions;
}

gPoly<double> HeuristicPolEnumModule::Prob(int p, int strat) const
{
  gPoly<double> equation(&Space, &Lex);
  Vector<int> exps(num_vars);
  int i, j, kk = 0;

  for (i = 1; i < p; i++) {
    kk += (support.NumStrategies(i) - 1);
  }

  if (strat < support.NumStrategies(p)) {
    exps = 0;
    exps[strat + kk] = 1;
    exp_vect const_exp(&Space, exps);
    gMono<double> const_term((double)1, const_exp);
    gPoly<double> new_term(&Space, const_term, &Lex);
    equation += new_term;
  }
  else {
    for (j = 1; j < support.NumStrategies(p); j++) {
      exps = 0;
      exps[j + kk] = 1;
      exp_vect exponent(&Space, exps);
      gMono<double> term((double)(-1), exponent);
      gPoly<double> new_term(&Space, term, &Lex);
      equation += new_term;
    }
    exps = 0;
    exp_vect const_exp(&Space, exps);
    gMono<double> const_term((double)1, const_exp);
    gPoly<double> new_term(&Space, const_term, &Lex);
    equation += new_term;
  }
  return equation;
}

gPoly<double> HeuristicPolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  gPoly<double> equation(&Space, &Lex);

  for (StrategyProfileIterator A(support, i, strat1), B(support, i, strat2); !A.AtEnd();
       A++, B++) {
    gPoly<double> term(&Space, (double)1, &Lex);
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

gPolyList<double> HeuristicPolEnumModule::IndifferenceEquations() const
{
  gPolyList<double> equations(&Space, &Lex);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    for (int j = 1; j < support.NumStrategies(pl); j++) {
      equations += IndifferenceEquation(pl, j, j + 1);
    }
  }

  return equations;
}

gPolyList<double> HeuristicPolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<double> equations(&Space, &Lex);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    if (support.NumStrategies(pl) > 2) {
      equations += Prob(pl, support.NumStrategies(pl));
    }
  }

  return equations;
}

gPolyList<double> HeuristicPolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<double> equations(&Space, &Lex);

  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}

Gambit::List<Vector<double>>
HeuristicPolEnumModule::NashOnSupportSolnVectors(const gPolyList<double> &equations,
                                                 const gRectangle<double> &Cube)
{
  QuikSolv<double> quickie(equations);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube, 2147483647, m_stopAfter);
  }
  catch (SingularMatrixException &) {
    is_singular = true;
  }

  return quickie.RootList();
}

bool HeuristicPolEnumModule::IsSingular() const { return is_singular; }

//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

int PolEnum(const StrategySupportProfile &support, int p_stopAfter,
            Gambit::List<MixedStrategyProfile<double>> &solutions, long &nevals, double &time,
            bool &is_singular)
{
  HeuristicPolEnumModule module(support, p_stopAfter);
  module.PolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  if (module.IsSingular()) {
    is_singular = true;
  }
  else {
    is_singular = false;
  }
  return 1;
}

//---------------------------------------------------------------------------
//                        Polish Equilibrum for Nfg
//---------------------------------------------------------------------------

#ifdef UNUSED
static MixedStrategyProfile<double> PolishEquilibrium(const StrategySupportProfile &support,
                                                      const MixedStrategyProfile<double> &sol,
                                                      bool &is_singular)
{
  HeuristicPolEnumModule module(support, 0);
  Vector<double> vec = module.SolVarsFromMixedStrategyProfile(sol);

  /* //DEBUG
  gbtPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  MixedStrategyProfile<gbtNumber> newsol = module.SequenceForm().ToMixed(xx);

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

  // DEBUG
  //   gout << "Prior to Polishing vec is " << vec << ".\n";

  module.PolishKnownRoot(vec);

  // DEBUG
  //   gout << "After Polishing vec is " << vec << ".\n";

  return module.ReturnPolishedSolution(vec);
}

#endif // UNUSED

Vector<double> HeuristicPolEnumModule::SolVarsFromMixedStrategyProfile(
    const MixedStrategyProfile<double> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    numvars += support.NumStrategies(pl) - 1;
  }

  Vector<double> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    for (int j = 1; j < support.NumStrategies(pl); j++) {
      count++;
      answer[count] = (double)sol[support.GetStrategy(pl, j)];
    }
  }

  return answer;
}

int HeuristicPolEnumModule::PolishKnownRoot(Vector<double> &point) const
{
  // DEBUG
  //   gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gPolyList<double> equations(&Space, &Lex);
    equations += IndifferenceEquations();

    // DEBUG
    //     gout << "We are about to construct quickie with Dmnsn() = "
    //   << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";

    // start QuikSolv
    QuikSolv<double> quickie(equations);

    // DEBUG
    //     gout << "We constructed quickie.\n";

    try {
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (SingularMatrixException &) {
      return 0;
    }

    // DEBUG
    //     gout << "After Polishing point = " << point << ".\n";
  }

  return 1;
}

MixedStrategyProfile<double>
HeuristicPolEnumModule::ReturnPolishedSolution(const Vector<double> &root) const
{
  MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>());

  int j;
  int kk = 0;
  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    double sum = 0;
    for (j = 1; j < support.NumStrategies(pl); j++) {
      profile[support.GetStrategy(pl, j)] = root[j + kk];
      sum += profile[support.GetStrategy(pl, j)];
    }
    profile[support.GetStrategy(pl, j)] = (double)1.0 - sum;
    kk += (support.NumStrategies(pl) - 1);
  }

  return profile;
}
