//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/game.cc
// Implementation of extensive form game representation
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
#include <numeric>
#include <random>

#include "games.h"
#include "writer.h"

// The references to the tree representations violate the logic
// of separating implementation types.  This will be fixed when we move
// editing operations into the game itself instead of in the member-object
// classes.
#include "gametree.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number, const std::string &p_label,
                               bool p_isNull /* = false */)
  : m_isNull(p_isNull), m_game(p_game), m_number(p_number), m_label(p_label)
{
  CheckLabel(p_label);
  for (const auto &player : m_game->m_players) {
    m_payoffs[player.get()] = Number();
  }
}

//========================================================================
//                      class GameStrategyRep
//========================================================================

GameAction GameStrategyRep::GetAction(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetPlayer().get() != m_player) {
    throw MismatchException();
  }
  try {
    return *std::next(p_infoset->GetActions().cbegin(), m_behav.at(p_infoset.get()) - 1);
  }
  catch (std::out_of_range &) {
    return nullptr;
  }
}

//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, const std::string &p_label, int p_strats)
  : m_game(p_game), m_number(p_id), m_label(p_label)
{
  for (int j = 1; j <= p_strats; j++) {
    m_strategies.push_back(std::make_shared<GameStrategyRep>(this, j, ""));
  }
}

GamePlayerRep::~GamePlayerRep()
{
  for (const auto &infoset : m_infosets) {
    infoset->Invalidate();
  }
  for (const auto &strategy : m_strategies) {
    strategy->Invalidate();
  }
  for (const auto &sequence : m_sequences) {
    sequence->Invalidate();
  }
}

void GamePlayerRep::MakeStrategy(const std::map<GameInfosetRep *, int> &behav)
{
  // Reduced strategies are labelled by their sequence number in their generation order.
  // This order is deterministic for a given game (see MakeReducedStrats)
  const std::string number = std::to_string(m_strategies.size() + 1);
  auto strategy = std::make_shared<GameStrategyRep>(this, m_strategies.size() + 1, number);
  strategy->m_behav = behav;
  m_strategies.push_back(strategy);
}

void GamePlayerRep::MakeReducedStrats(GameNodeRep *n, GameNodeRep *nn,
                                      std::map<GameInfosetRep *, int> &behav,
                                      std::map<GameNodeRep *, GameNodeRep *> &ptr,
                                      std::map<GameNodeRep *, GameNodeRep *> &whichbranch)
{
  if (!n->IsTerminal()) {
    if (n->m_infoset->m_player == this) {
      if (!behav.contains(n->m_infoset)) {
        // we haven't visited this infoset before
        for (size_t i = 1; i <= n->m_children.size(); i++) {
          GameNodeRep *m = n->m_children[i - 1].get();
          whichbranch[n] = m;
          behav[n->m_infoset] = i;
          MakeReducedStrats(m, nn, behav, ptr, whichbranch);
        }
        behav.erase(n->m_infoset);
      }
      else {
        // we have visited this infoset, take same action
        MakeReducedStrats(n->m_children[behav[n->m_infoset] - 1].get(), nn, behav, ptr,
                          whichbranch);
      }
    }
    else {
      if (nn != nullptr) {
        ptr[n] = nn->m_parent;
      }
      else {
        ptr.erase(n);
      }
      whichbranch[n] = n->m_children.front().get();
      MakeReducedStrats(n->m_children.front().get(), n->m_children.front().get(), behav, ptr,
                        whichbranch);
    }
  }
  else if (nn) {
    GameNode m;
    for (;; nn = whichbranch.at(ptr.at(nn->m_parent))) {
      m = nn->GetNextSibling();
      if (m || !ptr.contains(nn->m_parent)) {
        break;
      }
    }
    if (m) {
      GameNodeRep *mm = whichbranch.at(m->m_parent);
      whichbranch[m->m_parent] = m.get();
      MakeReducedStrats(m.get(), m.get(), behav, ptr, whichbranch);
      whichbranch[m->m_parent] = mm;
    }
    else {
      MakeStrategy(behav);
    }
  }
  else {
    MakeStrategy(behav);
  }
}

//========================================================================
//                            class GameRep
//========================================================================

GameRep::~GameRep()
{
  for (auto player : m_players) {
    player->Invalidate();
  }
  for (auto outcome : m_outcomes) {
    outcome->Invalidate();
  }
  m_nullOutcome->Invalidate();
}

void GameRep::IndexStrategies() const
{
  const size_t n = m_players.size();
  m_pureStrategies.m_radices.resize(n);
  m_pureStrategies.m_strides.resize(n);

  long stride = 1L;
  for (size_t i = 0; i < n; ++i) {
    const auto &player = m_players[i];
    m_pureStrategies.m_strides[i] = stride;
    m_pureStrategies.m_radices[i] = player->m_strategies.size();
    for (auto [st, strategy] : enumerate(player->m_strategies)) {
      strategy->m_number = st + 1;
    }
    stride *= m_pureStrategies.m_radices[i];
  }
}

void GameRep::CheckPlayerLabel(const std::string &p_label,
                               const std::set<const GamePlayerRep *> &p_ignore) const
{
  if (p_label.empty()) {
    throw ValueException("Player label must not be empty");
  }
  CheckLabel(p_label);
  if (dynamic_cast<const GameTreeRep *>(this) && p_label == GetChance()->GetLabel()) {
    throw ValueException("Player label must not be the reserved chance player label");
  }
  for (const auto &player : m_players) {
    if (p_ignore.count(player.get()) == 0 && player->GetLabel() == p_label) {
      throw ValueException("Player label must be unique within the game");
    }
  }
}

//------------------------------------------------------------------------
//                     GameRep: Writing data files
//------------------------------------------------------------------------

///
/// Write the game to a savefile in .nfg payoff format.
///
/// This implements writing a game to a .nfg savefile.  As it uses
/// only publicly-accessible operations, it is in principle valid
/// for any game.  It writes the payoff format, as it is not required
/// that a game representation implements an outcome for each possible
/// pure strategy profile.  For example, in extensive games with chance
/// moves, the outcome from a pure strategy profile is not deterministic,
/// but rather a probability distribution over outcomes.
///
void GameRep::WriteNfgFile(std::ostream &p_file) const
{
  auto players = GetPlayers();
  p_file << "NFG 1 R " << std::quoted(GetTitle()) << ' '
         << FormatList(players, [](const GamePlayer &p) { return QuoteString(p->GetLabel()); })
         << std::endl
         << std::endl;
  p_file << "{ ";
  for (auto player : players) {
    p_file << FormatList(player->GetStrategies(), [](const GameStrategy &s) {
      return QuoteString(s->GetLabel());
    }) << std::endl;
  }
  p_file << "}" << std::endl;
  p_file << std::quoted(GetDescription()) << std::endl << std::endl;

  for (auto iter : StrategyContingencies(
           StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this())))) {
    p_file << FormatList(
                  players,
                  [&iter](const GamePlayer &p) {
                    return lexical_cast<std::string>(iter->GetPayoff(p));
                  },
                  false, false)
           << std::endl;
  };
}

//------------------------------------------------------------------------
//                           GameRep: Players
//------------------------------------------------------------------------

void GameRep::RelabelPlayers(const std::map<std::string, std::string> &p_labels)
{
  // Resolve each key to exactly one (personal) player of the game.
  std::map<GamePlayerRep *, std::string> assignment;
  std::set<const GamePlayerRep *> relabeled;
  for (const auto &[old_label, new_label] : p_labels) {
    GamePlayerRep *match = nullptr;
    for (const auto &player : m_players) {
      if (player->GetLabel() == old_label) {
        if (match) {
          throw ValueException("Player label '" + old_label + "' is ambiguous in this game");
        }
        match = player.get();
      }
    }
    if (!match) {
      if (dynamic_cast<GameTreeRep *>(this) && old_label == GetChance()->GetLabel()) {
        throw ValueException("The chance player's label cannot be changed");
      }
      throw ValueException("No player with label '" + old_label + "' in this game");
    }
    assignment[match] = new_label;
    relabeled.insert(match);
  }
  // Replacement labels must be legal, unique against untouched players, and pairwise distinct
  std::set<std::string> targets;
  for (const auto &[player, new_label] : assignment) {
    CheckPlayerLabel(new_label, relabeled);
    if (!targets.insert(new_label).second) {
      throw ValueException("Player label '" + new_label +
                           "' would be duplicated by the relabelling");
    }
  }
  for (const auto &[player, new_label] : assignment) {
    player->m_label = new_label;
  }
}

//------------------------------------------------------------------------
//                           GameRep: Outcomes
//------------------------------------------------------------------------

void GameRep::RelabelOutcomes(const std::map<std::string, std::string> &p_labels)
{
  // Resolve each key to exactly one outcome of the game.
  std::map<GameOutcomeRep *, std::string> assignment;
  std::set<const GameOutcomeRep *> relabeled;
  for (const auto &[old_label, new_label] : p_labels) {
    GameOutcomeRep *match = nullptr;
    for (const auto &outcome : m_outcomes) {
      if (outcome->GetLabel() == old_label) {
        if (match) {
          throw ValueException("Outcome label '" + old_label + "' is ambiguous in this game");
        }
        match = outcome.get();
      }
    }
    if (!match) {
      throw ValueException("No outcome with label '" + old_label + "' in this game");
    }
    assignment[match] = new_label;
    relabeled.insert(match);
  }
  // Replacement labels must be legal, unique against untouched outcomes, and pairwise distinct
  std::set<std::string> targets;
  for (const auto &[outcome, new_label] : assignment) {
    CheckOutcomeLabel(new_label, relabeled);
    if (!targets.insert(new_label).second) {
      throw ValueException("Outcome label '" + new_label +
                           "' would be duplicated by the relabelling");
    }
  }
  for (const auto &[outcome, new_label] : assignment) {
    outcome->m_label = new_label;
  }
}

} // end namespace Gambit
