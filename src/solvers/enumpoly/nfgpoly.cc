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

#include "enumpoly.h"
#include "solvers/nashsupport/nashsupport.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"

using namespace Gambit;

class PolEnumModule {
private:
  double eps{0.0};
  Game NF;
  const StrategySupportProfile &support;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long nevals{0};
  double time{0.0};
  List<MixedStrategyProfile<double>> solutions;
  bool is_singular{false};

  bool EqZero(double x) const;

  // p_i_j as a gPoly, with last prob in terms of previous probs
  gPoly<double> Prob(int i, int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.
  gPoly<double> IndifferenceEquation(int i, int strat1, int strat2) const;
  gPolyList<double> IndifferenceEquations() const;
  gPolyList<double> LastActionProbPositiveInequalities() const;
  gPolyList<double> NashOnSupportEquationsAndInequalities() const;
  List<Vector<double>> NashOnSupportSolnVectors(const gPolyList<double> &equations,
                                                const gRectangle<double> &Cube);

  int SaveSolutions(const List<Vector<double>> &list);

public:
  explicit PolEnumModule(const StrategySupportProfile &);

  int PolEnum();

  long NumEvals() const { return nevals; }
  double Time() const { return time; }

  const List<MixedStrategyProfile<double>> &GetSolutions() const { return solutions; }

  int PolishKnownRoot(Vector<double> &) const;

  MixedStrategyProfile<double> ReturnPolishedSolution(const Vector<double> &) const;

  bool IsSingular() const { return is_singular; }
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

PolEnumModule::PolEnumModule(const StrategySupportProfile &S)
  : NF(S.GetGame()), support(S), Space(support.MixedProfileLength() - NF->NumPlayers()),
    Lex(&Space, lex), num_vars(support.MixedProfileLength() - NF->NumPlayers())
{
}

int PolEnumModule::PolEnum()
{
  gPolyList<double> equations = NashOnSupportEquationsAndInequalities();

  Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = 0;
  tops = 1;

  gRectangle<double> Cube(bottoms, tops);

  // start QuikSolv
  List<Vector<double>> solutionlist = NashOnSupportSolnVectors(equations, Cube);

  int index = SaveSolutions(solutionlist);
  return index;
}

int PolEnumModule::SaveSolutions(const List<Vector<double>> &list)
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

bool PolEnumModule::EqZero(double x) const
{
  if (x <= eps && x >= -eps) {
    return true;
  }
  return false;
}

gPoly<double> PolEnumModule::Prob(int p, int strat) const
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

gPoly<double> PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
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

gPolyList<double> PolEnumModule::IndifferenceEquations() const
{
  gPolyList<double> equations(&Space, &Lex);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    for (int j = 1; j < support.NumStrategies(pl); j++) {
      equations += IndifferenceEquation(pl, j, j + 1);
    }
  }

  return equations;
}

gPolyList<double> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<double> equations(&Space, &Lex);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    if (support.NumStrategies(pl) > 2) {
      equations += Prob(pl, support.NumStrategies(pl));
    }
  }

  return equations;
}

gPolyList<double> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<double> equations(&Space, &Lex);

  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}

List<Vector<double>> PolEnumModule::NashOnSupportSolnVectors(const gPolyList<double> &equations,
                                                             const gRectangle<double> &Cube)
{
  QuikSolv<double> quickie(equations);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube, 2147483647, 0);
  }
  catch (const SingularMatrixException &) {
    is_singular = true;
  }
  catch (const AssertionException &e) {
    // std::cerr << "Assertion warning: " << e.what() << std::endl;
    is_singular = true;
  }

  return quickie.RootList();
}

int PolEnum(const StrategySupportProfile &support, List<MixedStrategyProfile<double>> &solutions,
            long &nevals, double &time, bool &is_singular)
{
  PolEnumModule module(support);
  module.PolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  is_singular = module.IsSingular();
  return 1;
}

int PolEnumModule::PolishKnownRoot(Vector<double> &point) const
{
  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gPolyList<double> equations(&Space, &Lex);
    equations += IndifferenceEquations();
    QuikSolv<double> quickie(equations);
    try {
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (SingularMatrixException &) {
      return 0;
    }
  }
  return 1;
}

MixedStrategyProfile<double>
PolEnumModule::ReturnPolishedSolution(const Vector<double> &root) const
{
  MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>());

  int kk = 0;
  for (int pl = 1; pl <= NF->NumPlayers(); pl++) {
    double sum = 0;
    int j;
    for (j = 1; j < support.NumStrategies(pl); j++) {
      profile[support.GetStrategy(pl, j)] = root[j + kk];
      sum += profile[support.GetStrategy(pl, j)];
    }
    profile[support.GetStrategy(pl, j)] = 1.0 - sum;
    kk += (support.NumStrategies(pl) - 1);
  }

  return profile;
}

MixedStrategyProfile<double> ToFullSupport(const MixedStrategyProfile<double> &p_profile)
{
  Game nfg = p_profile.GetGame();
  const StrategySupportProfile &support = p_profile.GetSupport();

  MixedStrategyProfile<double> fullProfile(nfg->NewMixedStrategyProfile(0.0));
  for (int i = 1; i <= fullProfile.MixedProfileLength(); fullProfile[i++] = 0.0)
    ;

  int index = 1;
  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    GamePlayer player = nfg->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (support.Contains(player->GetStrategy(st))) {
        fullProfile[player->GetStrategy(st)] = p_profile[index++];
      }
    }
  }

  return fullProfile;
}

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double>>
EnumPolyStrategySolve(const Game &p_game,
                      EnumPolyMixedStrategyObserverFunctionType p_onEquilibrium,
                      EnumPolyStrategySupportObserverFunctionType p_onSupport)
{
  List<MixedStrategyProfile<double>> ret;
  auto possible_supports = PossibleNashStrategySupports(p_game);

  for (auto support : possible_supports->m_supports) {
    long newevals = 0;
    double newtime = 0.0;
    List<MixedStrategyProfile<double>> newsolns;
    bool is_singular = false;

    p_onSupport("candidate", support);
    PolEnum(support, newsolns, newevals, newtime, is_singular);

    for (int j = 1; j <= newsolns.Length(); j++) {
      MixedStrategyProfile<double> fullProfile = ToFullSupport(newsolns[j]);
      if (fullProfile.GetLiapValue() < 1.0e-6) {
        p_onEquilibrium(fullProfile);
        ret.push_back(fullProfile);
      }
    }

    if (is_singular) {
      p_onSupport("singular", support);
    }
  }
  return ret;
}

} // namespace Nash
} // namespace Gambit
