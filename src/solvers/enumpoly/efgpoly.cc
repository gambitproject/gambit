//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/efgpoly.cc
// Enumerates all Nash equilibria of a game, via polynomial equations
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
#include "gambit.h"

using namespace Gambit;

#include "efgensup.h"
#include "sfg.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"
#include "behavextend.h"

extern int g_numDecimals;
extern bool g_verbose;

//
// A class to organize the data needed to build the polynomials
//
class ProblemData {
public:
  const BehaviorSupportProfile &support;
  Sfg SF;
  int nVars;
  gSpace *Space;
  term_order *Lex;
  Array<Array<int>> var;

  explicit ProblemData(const BehaviorSupportProfile &p_support);
  ~ProblemData();
};

ProblemData::ProblemData(const BehaviorSupportProfile &p_support)
  : support(p_support), SF(p_support),
    nVars(SF.TotalNumSequences() - SF.NumPlayerInfosets() - SF.NumPlayers()),
    Space(new gSpace(nVars)), Lex(new term_order(Space, lex)),
    var(p_support.GetGame()->NumPlayers())
{
  int tnv = 0;
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    var[pl] = Array<int>(SF.NumSequences(pl));
    var[pl][1] = 0;
    for (int seq = 2; seq <= SF.NumSequences(pl); seq++) {
      int act = SF.ActionNumber(pl, seq);
      GameInfoset infoset = SF.GetInfoset(pl, seq);
      if (act < p_support.NumActions(infoset)) {
        var[pl][seq] = ++tnv;
      }
      else {
        var[pl][seq] = 0;
      }
    }
  }
}

ProblemData::~ProblemData()
{
  delete Lex;
  delete Space;
}

//=======================================================================
//               Constructing the equilibrium conditions
//=======================================================================

// The strategy is to develop the polynomial for each agent's expected
// payoff as a function of the behavior strategies on the support,
// eliminating the last action probability for each information set.
// The system is obtained by requiring that all the partial
// derivatives vanish, and that the sum of action probabilities at
// each information set be less than one.

gPoly<double> ProbOfSequence(const ProblemData &p_data, int p, int seq)
{
  gPoly<double> equation(p_data.Space, p_data.Lex);
  Vector<int> exps(p_data.nVars);

  int isetrow = p_data.SF.InfosetRowNumber(p, seq);
  int act = p_data.SF.ActionNumber(p, seq);
  int varno = p_data.var[p][seq];
  GameInfoset infoset = p_data.SF.GetInfoset(p, seq);

  if (seq == 1) {
    exps = 0;
    exp_vect const_exp(p_data.Space, exps);
    gMono<double> const_term(1.0, const_exp);
    gPoly<double> new_term(p_data.Space, const_term, p_data.Lex);
    equation += new_term;
  }
  else if (act < p_data.support.NumActions(infoset)) {
    exps = 0;
    exps[varno] = 1;
    exp_vect const_exp(p_data.Space, exps);
    gMono<double> const_term(1.0, const_exp);
    gPoly<double> new_term(p_data.Space, const_term, p_data.Lex);
    equation += new_term;
  }
  else {
    for (int j = 1; j < seq; j++) {
      if (p_data.SF.Constraints(p)(isetrow, j) == Rational(-1)) {
        equation -= ProbOfSequence(p_data, p, j);
      }
      else if (p_data.SF.Constraints(p)(isetrow, j) == Rational(1)) {
        equation += ProbOfSequence(p_data, p, j);
      }
    }
  }
  return equation;
}

gPoly<double> GetPayoff(const ProblemData &p_data, int pl)
{
  gIndexOdometer index(p_data.SF.NumSequences());
  Rational pay;

  gPoly<double> equation(p_data.Space, p_data.Lex);
  while (index.Turn()) {
    pay = p_data.SF.Payoff(index.CurrentIndices(), pl);
    if (pay != Rational(0)) {
      gPoly<double> term(p_data.Space, (double)pay, p_data.Lex);
      int k;
      for (k = 1; k <= p_data.support.GetGame()->NumPlayers(); k++) {
        term *= ProbOfSequence(p_data, k, (index.CurrentIndices())[k]);
      }
      equation += term;
    }
  }
  return equation;
}

gPolyList<double> IndifferenceEquations(const ProblemData &p_data)
{
  gPolyList<double> equations(p_data.Space, p_data.Lex);

  int kk = 0;
  for (int pl = 1; pl <= p_data.SF.NumPlayers(); pl++) {
    gPoly<double> payoff = GetPayoff(p_data, pl);
    int n_vars = p_data.SF.NumSequences(pl) - p_data.SF.NumInfosets(pl) - 1;
    for (int j = 1; j <= n_vars; j++) {
      equations += payoff.PartialDerivative(kk + j);
    }
    kk += n_vars;
  }

  return equations;
}

gPolyList<double> LastActionProbPositiveInequalities(const ProblemData &p_data)
{
  gPolyList<double> equations(p_data.Space, p_data.Lex);

  for (int i = 1; i <= p_data.SF.NumPlayers(); i++) {
    for (int j = 2; j <= p_data.SF.NumSequences(i); j++) {
      int act_num = p_data.SF.ActionNumber(i, j);
      GameInfoset infoset = p_data.SF.GetInfoset(i, j);
      if (act_num == p_data.support.NumActions(infoset) && act_num > 1) {
        equations += ProbOfSequence(p_data, i, j);
      }
    }
  }

  return equations;
}

gPolyList<double> NashOnSupportEquationsAndInequalities(const ProblemData &p_data)
{
  gPolyList<double> equations(p_data.Space, p_data.Lex);

  equations += IndifferenceEquations(p_data);
  equations += LastActionProbPositiveInequalities(p_data);

  return equations;
}

//=======================================================================
//               Mapping solution vectors to sequences
//=======================================================================

double NumProbOfSequence(const ProblemData &p_data, int p, int seq, const Vector<double> &x)
{
  int isetrow = p_data.SF.InfosetRowNumber(p, seq);
  int act = p_data.SF.ActionNumber(p, seq);
  int varno = p_data.var[p][seq];
  GameInfoset infoset = p_data.SF.GetInfoset(p, seq);

  if (seq == 1) {
    return 1.0;
  }
  else if (act < p_data.support.NumActions(infoset)) {
    return x[varno];
  }
  else {
    double value = 0.0;
    for (int j = 1; j < seq; j++) {
      if (p_data.SF.Constraints(p)(isetrow, j) == Rational(-1)) {
        value -= NumProbOfSequence(p_data, p, j, x);
      }
      else if (p_data.SF.Constraints(p)(isetrow, j) == Rational(1)) {
        value += NumProbOfSequence(p_data, p, j, x);
      }
    }
    return value;
  }
}

PVector<double> SeqFormVectorFromSolFormVector(const ProblemData &p_data, const Vector<double> &v)
{
  PVector<double> x(p_data.SF.NumSequences());

  for (int i = 1; i <= p_data.support.GetGame()->NumPlayers(); i++) {
    for (int j = 1; j <= p_data.SF.NumSequences()[i]; j++) {
      x(i, j) = NumProbOfSequence(p_data, i, j, v);
    }
  }

  return x;
}

bool ExtendsToNash(const MixedBehaviorProfile<double> &bs)
{
  algExtendsToNash algorithm;
  return algorithm.ExtendsToNash(bs, BehaviorSupportProfile(bs.GetGame()),
                                 BehaviorSupportProfile(bs.GetGame()));
}

List<MixedBehaviorProfile<double>> SolveSupport(const BehaviorSupportProfile &p_support,
                                                bool &p_isSingular)
{
  ProblemData data(p_support);
  gPolyList<double> equations = NashOnSupportEquationsAndInequalities(data);

  // set up the rectangle of search
  Vector<double> bottoms(data.nVars), tops(data.nVars);
  bottoms = (double)0;
  tops = (double)1;
  gRectangle<double> Cube(bottoms, tops);

  QuikSolv<double> quickie(equations);
#ifdef UNUSED
  if (params.trace > 0) {
    (*params.tracefile) << "\nThe equilibrium equations are \n" << quickie.UnderlyingEquations();
  }
#endif // UNUSED

  // 2147483647 = 2^31-1 = MaxInt

  try {
    if (quickie.FindCertainNumberOfRoots(Cube, 2147483647, 0)) {
#ifdef UNUSED
      if (params.trace > 0) {
        (*params.tracefile) << "\nThe system has the following roots in [0,1]^" << num_vars
                            << " :\n"
                            << quickie.RootList();
      }
#endif // UNUSED
    }
  }
  catch (const Gambit::SingularMatrixException &) {
    p_isSingular = true;
  }
  catch (const Gambit::AssertionException &e) {
    std::cerr << "Assertion warning: " << e.what() << std::endl;
    p_isSingular = true;
  }

  List<Vector<double>> solutionlist = quickie.RootList();

  List<MixedBehaviorProfile<double>> solutions;
  for (int k = 1; k <= solutionlist.Length(); k++) {
    PVector<double> y = SeqFormVectorFromSolFormVector(data, solutionlist[k]);
    MixedBehaviorProfile<double> sol(data.SF.ToBehav(y));
    if (ExtendsToNash(sol)) {
      solutions.push_back(sol);
    }
  }

  return solutions;
}

PVector<double> SeqFormProbsFromSolVars(const ProblemData &p_data, const Vector<double> &v)
{
  PVector<double> x(p_data.SF.NumSequences());

  for (int pl = 1; pl <= p_data.support.GetGame()->NumPlayers(); pl++) {
    for (int seq = 1; seq <= p_data.SF.NumSequences()[pl]; seq++) {
      x(pl, seq) = NumProbOfSequence(p_data, pl, seq, v);
    }
  }

  return x;
}

void PrintProfile(std::ostream &p_stream, const std::string &p_label,
                  const MixedBehaviorProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.BehaviorProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

MixedBehaviorProfile<double> ToFullSupport(const MixedBehaviorProfile<double> &p_profile)
{
  Game efg = p_profile.GetGame();
  const BehaviorSupportProfile &support = p_profile.GetSupport();

  MixedBehaviorProfile<double> fullProfile(efg);
  fullProfile = 0.0;

  int index = 1;
  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    GamePlayer player = efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
        if (support.Contains(infoset->GetAction(act))) {
          fullProfile[infoset->GetAction(act)] = p_profile[index++];
        }
      }
    }
  }

  return fullProfile;
}

void PrintSupport(std::ostream &p_stream, const std::string &p_label,
                  const BehaviorSupportProfile &p_support)
{
  p_stream << p_label;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_support.GetGame()->GetPlayer(pl);

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      p_stream << ",";

      for (int act = 1; act <= infoset->NumActions(); act++) {
        if (p_support.Contains(infoset->GetAction(act))) {
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

void EnumPolySolveExtensive(const Game &p_game)
{
  List<BehaviorSupportProfile> supports(PossibleNashSubsupports(BehaviorSupportProfile(p_game)));

  for (int i = 1; i <= supports.Length(); i++) {
    if (g_verbose) {
      PrintSupport(std::cout, "candidate", supports[i]);
    }

    bool isSingular = false;
    List<MixedBehaviorProfile<double>> newsolns = SolveSupport(supports[i], isSingular);

    for (int j = 1; j <= newsolns.Length(); j++) {
      MixedBehaviorProfile<double> fullProfile = ToFullSupport(newsolns[j]);
      if (fullProfile.GetLiapValue() < 1.0e-6) {
        PrintProfile(std::cout, "NE", fullProfile);
      }
    }

    if (isSingular && g_verbose) {
      PrintSupport(std::cout, "singular", supports[i]);
    }
  }
}
