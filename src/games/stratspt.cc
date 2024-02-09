//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/stratspt.cc
// Implementation of strategy classes for normal forms
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

#include "gambit.h"
#include "gametable.h"

namespace Gambit {

//===========================================================================
//                          class StrategySupportProfile
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

StrategySupportProfile::StrategySupportProfile(const Game &p_nfg)
  : m_nfg(p_nfg), m_profileIndex(p_nfg->MixedProfileLength())
{
  for (int pl = 1, index = 1; pl <= p_nfg->NumPlayers(); pl++) {
    m_support.push_back(Array<GameStrategy>());
    for (int st = 1; st <= p_nfg->GetPlayer(pl)->NumStrategies(); st++, index++) {
      m_support[pl].push_back(p_nfg->GetPlayer(pl)->GetStrategy(st));
      m_profileIndex[index] = index;
    }
  }
}

//---------------------------------------------------------------------------
//                          General information
//---------------------------------------------------------------------------

Array<int> StrategySupportProfile::NumStrategies() const
{
  Array<int> a(m_support.Length());

  for (int pl = 1; pl <= a.Length(); pl++) {
    a[pl] = m_support[pl].Length();
  }
  return a;
}

int StrategySupportProfile::MixedProfileLength() const
{
  int total = 0;
  for (int pl = 1; pl <= m_nfg->NumPlayers(); total += m_support[pl++].Length())
    ;
  return total;
}

template <> MixedStrategyProfile<double> StrategySupportProfile::NewMixedStrategyProfile() const
{
  return m_nfg->NewMixedStrategyProfile(0.0, *this);
}

template <> MixedStrategyProfile<Rational> StrategySupportProfile::NewMixedStrategyProfile() const
{
  return m_nfg->NewMixedStrategyProfile(Rational(0), *this);
}

bool StrategySupportProfile::IsSubsetOf(const StrategySupportProfile &p_support) const
{
  if (m_nfg != p_support.m_nfg) {
    return false;
  }
  for (int pl = 1; pl <= m_support.Length(); pl++) {
    if (m_support[pl].Length() > p_support.m_support[pl].Length()) {
      return false;
    }
    else {
      for (int st = 1; st <= m_support[pl].Length(); st++) {
        if (!p_support.m_support[pl].Contains(m_support[pl][st])) {
          return false;
        }
      }
    }
  }
  return true;
}

namespace {

std::string EscapeQuotes(const std::string &s)
{
  std::string ret;

  for (char c : s) {
    if (c == '"') {
      ret += '\\';
    }
    ret += c;
  }

  return ret;
}

} // end anonymous namespace

void StrategySupportProfile::WriteNfgFile(std::ostream &p_file) const
{
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(m_nfg->GetTitle()) << "\" { ";

  for (auto player : m_nfg->GetPlayers()) {
    p_file << '"' << EscapeQuotes(player->GetLabel()) << "\" ";
  }

  p_file << "}\n\n{ ";

  for (auto player : m_nfg->GetPlayers()) {
    p_file << "{ ";
    for (auto strategy : GetStrategies(player)) {
      p_file << '"' << EscapeQuotes(strategy->GetLabel()) << "\" ";
    }
    p_file << "}\n";
  }

  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_nfg->GetComment()) << "\"\n\n";

  // For trees, we write the payoff version, since there need not be
  // a one-to-one correspondence between outcomes and entries, when there
  // are chance moves.
  StrategyProfileIterator iter(*this);

  for (; !iter.AtEnd(); iter++) {
    for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
      p_file << (*iter)->GetPayoff(pl) << " ";
    }
    p_file << "\n";
  }

  p_file << '\n';
}

//---------------------------------------------------------------------------
//                        Modifying the support
//---------------------------------------------------------------------------

void StrategySupportProfile::AddStrategy(const GameStrategy &p_strategy)
{
  // Get the null-pointer checking out of the way once and for all
  GameStrategyRep *strategy = p_strategy;
  Array<GameStrategy> &support = m_support[strategy->GetPlayer()->GetNumber()];

  for (int i = 1; i <= support.Length(); i++) {
    GameStrategyRep *s = support[i];
    if (s->GetNumber() == strategy->GetNumber()) {
      // Strategy already in support; no change
      return;
    }
    if (s->GetNumber() > strategy->GetNumber()) {
      // Shift all higher-id strategies by one in the profile
      m_profileIndex[strategy->GetId()] = m_profileIndex[s->GetId()];
      for (int id = s->GetId(); id <= m_profileIndex.Length(); id++) {
        if (m_profileIndex[id] >= 0) {
          m_profileIndex[id]++;
        }
      }
      // Insert here
      support.Insert(strategy, i);
      return;
    }
  }

  // If we get here, p_strategy has a higher number than anything in the
  // support for this player; append.
  GameStrategyRep *last = support[support.Last()];
  m_profileIndex[strategy->GetId()] = m_profileIndex[last->GetId()] + 1;
  for (int id = strategy->GetId() + 1; id <= m_profileIndex.Length(); id++) {
    if (m_profileIndex[id] >= 0) {
      m_profileIndex[id]++;
    }
  }
  support.push_back(strategy);
}

bool StrategySupportProfile::RemoveStrategy(const GameStrategy &p_strategy)
{
  GameStrategyRep *strategy = p_strategy;
  Array<GameStrategy> &support = m_support[strategy->GetPlayer()->GetNumber()];

  if (support.Length() == 1) {
    return false;
  }

  for (int i = 1; i <= support.Length(); i++) {
    GameStrategyRep *s = support[i];
    if (s == strategy) {
      support.Remove(i);
      m_profileIndex[strategy->GetId()] = -1;
      // Shift strategies left in the profile
      for (int id = strategy->GetId() + 1; id <= m_profileIndex.Length(); id++) {
        if (m_profileIndex[id] >= 0) {
          m_profileIndex[id]--;
        }
      }
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
//                 Identification of dominated strategies
//---------------------------------------------------------------------------

bool StrategySupportProfile::Dominates(const GameStrategy &s, const GameStrategy &t,
                                       bool p_strict) const
{
  bool equal = true;

  for (StrategyProfileIterator iter(*this); !iter.AtEnd(); iter++) {
    Rational ap = (*iter)->GetStrategyValue(s);
    Rational bp = (*iter)->GetStrategyValue(t);
    if (p_strict && ap <= bp) {
      return false;
    }
    else if (!p_strict) {
      if (ap < bp) {
        return false;
      }
      else if (ap > bp) {
        equal = false;
      }
    }
  }

  return (p_strict || !equal);
}

bool StrategySupportProfile::IsDominated(const GameStrategy &s, bool p_strict,
                                         bool p_external) const
{
  if (p_external) {
    GamePlayer player = s->GetPlayer();
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (player->GetStrategy(st) != s && Dominates(player->GetStrategy(st), s, p_strict)) {
        return true;
      }
    }
    return false;
  }
  else {
    for (int i = 1; i <= NumStrategies(s->GetPlayer()->GetNumber()); i++) {
      if (GetStrategy(s->GetPlayer()->GetNumber(), i) != s &&
          Dominates(GetStrategy(s->GetPlayer()->GetNumber(), i), s, p_strict)) {
        return true;
      }
    }
    return false;
  }
}

bool StrategySupportProfile::Undominated(StrategySupportProfile &newS, int p_player, bool p_strict,
                                         bool p_external) const
{
  Array<GameStrategy> set((p_external) ? m_nfg->GetPlayer(p_player)->NumStrategies()
                                       : NumStrategies(p_player));

  if (p_external) {
    for (int st = 1; st <= set.Length(); st++) {
      set[st] = m_nfg->GetPlayer(p_player)->GetStrategy(st);
    }
  }
  else {
    for (int st = 1; st <= set.Length(); st++) {
      set[st] = GetStrategy(p_player, st);
    }
  }

  int min = 0, dis = set.Length() - 1;

  while (min <= dis) {
    int pp;
    for (pp = 0; pp < min && !Dominates(set[pp + 1], set[dis + 1], p_strict); pp++)
      ;
    if (pp < min) {
      dis--;
    }
    else {
      GameStrategy foo = set[dis + 1];
      set[dis + 1] = set[min + 1];
      set[min + 1] = foo;

      for (int inc = min + 1; inc <= dis;) {
        if (Dominates(set[min + 1], set[dis + 1], p_strict)) {
          // p_tracefile << GetStrategy(p_player, set[dis+1])->GetNumber() << " dominated by " <<
          // GetStrategy(p_player, set[min+1])->GetNumber() << '\n';
          dis--;
        }
        else if (Dominates(set[dis + 1], set[min + 1], p_strict)) {
          // p_tracefile << GetStrategy(p_player, set[min+1])->GetNumber() << " dominated by " <<
          // GetStrategy(p_player, set[dis+1])->GetNumber() << '\n';
          foo = set[dis + 1];
          set[dis + 1] = set[min + 1];
          set[min + 1] = foo;
          dis--;
        }
        else {
          foo = set[dis + 1];
          set[dis + 1] = set[inc + 1];
          set[inc + 1] = foo;
          inc++;
        }
      }
      min++;
    }
  }

  if (min + 1 <= set.Length()) {
    for (int i = min + 1; i <= set.Length(); i++) {
      newS.RemoveStrategy(set[i]);
    }

    return true;
  }
  else {
    return false;
  }
}

StrategySupportProfile StrategySupportProfile::Undominated(bool p_strict, bool p_external) const
{
  StrategySupportProfile newS(*this);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    Undominated(newS, pl, p_strict, p_external);
  }

  return newS;
}

StrategySupportProfile StrategySupportProfile::Undominated(bool p_strict,
                                                           const Array<int> &players) const
{
  StrategySupportProfile newS(*this);

  for (int i = 1; i <= players.Length(); i++) {
    // tracefile << "Dominated strategies for player " << pl << ":\n";
    Undominated(newS, players[i], p_strict);
  }

  return newS;
}

//---------------------------------------------------------------------------
//                Identification of overwhelmed strategies
//---------------------------------------------------------------------------

bool StrategySupportProfile::Overwhelms(const GameStrategy &s, const GameStrategy &t,
                                        bool p_strict) const
{
  StrategyProfileIterator iter(*this);
  Rational sMin = (*iter)->GetStrategyValue(s);
  Rational tMax = (*iter)->GetStrategyValue(t);

  for (; !iter.AtEnd(); iter++) {
    if ((*iter)->GetStrategyValue(s) < sMin) {
      sMin = (*iter)->GetStrategyValue(s);
    }

    if ((*iter)->GetStrategyValue(t) > tMax) {
      tMax = (*iter)->GetStrategyValue(t);
    }

    if (sMin < tMax || (sMin == tMax && p_strict)) {
      return false;
    }
  }

  return true;
}

//===========================================================================
//                     class StrategySupportProfile::iterator
//===========================================================================

bool StrategySupportProfile::iterator::GoToNext()
{
  if (strat != support.NumStrategies(pl)) {
    strat++;
    return true;
  }
  else if (pl != support.GetGame()->NumPlayers()) {
    pl++;
    strat = 1;
    return true;
  }
  else {
    return false;
  }
}

bool StrategySupportProfile::iterator::IsSubsequentTo(const GameStrategy &s) const
{
  if (pl > s->GetPlayer()->GetNumber()) {
    return true;
  }
  else if (pl < s->GetPlayer()->GetNumber()) {
    return false;
  }
  else if (strat > s->GetNumber()) {
    return true;
  }
  else {
    return false;
  }
}

} // end namespace Gambit
