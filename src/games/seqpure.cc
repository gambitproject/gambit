//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/seqpure.cc
// Implementation of pure sequence profile
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

#include <stack>

#include "gambit.h"

namespace Gambit {

PureSequenceProfile::PureSequenceProfile(const Game &p_efg) : m_efg(p_efg) {}

namespace {

/// One frame of the explicit-stack traversal used by WalkRealizedNodes().
/// Tracks the node under consideration, the probability of reaching it
/// (given chance's actual probabilities and each player's moves as
/// prescribed by the profile being walked), and how far each player has
/// progressed along the chain of sequences from the empty sequence to the
/// one designated for them.
struct SequenceWalkFrame {
  GameNode node;
  Rational prob;
  std::map<GamePlayer, size_t> progress;
};

/// Walks the game tree, following the moves prescribed by p_profile for
/// each player (chance moves freely over all its actions) and pruning any
/// branch inconsistent with it.  Invokes p_visit(node, prob) at every node
/// reached at which p_profile is exactly realised so far, i.e. every
/// player's progress has caught up to the sequence designated for them.
/// This is iterative (an explicit stack), not recursive, so it is not
/// limited by the game tree's depth.
template <class F> void WalkRealizedNodes(const PureSequenceProfile &p_profile, F &&p_visit)
{
  const Game &efg = p_profile.GetGame();
  std::map<GamePlayer, std::vector<GameSequence>> chains;
  std::map<GamePlayer, size_t> initialProgress;
  for (auto player : efg->GetPlayers()) {
    std::vector<GameSequence> chain;
    for (GameSequence seq = p_profile.GetSequence(player); seq; seq = seq->GetParent()) {
      chain.push_back(seq);
    }
    std::reverse(chain.begin(), chain.end()); // chain.front() is the empty sequence
    chains[player] = chain;
    initialProgress[player] = 0;
  }

  std::stack<SequenceWalkFrame> frames;
  frames.push({efg->GetRoot(), Rational(1), initialProgress});

  while (!frames.empty()) {
    const SequenceWalkFrame frame = std::move(frames.top());
    frames.pop();
    const GameNode &n = frame.node;

    const bool matches = std::all_of(chains.begin(), chains.end(), [&](const auto &entry) {
      return frame.progress.at(entry.first) + 1 == entry.second.size();
    });
    if (matches) {
      p_visit(n, frame.prob);
    }

    if (!n->GetInfoset()) {
      continue;
    }
    if (n->GetPlayer()->IsChance()) {
      for (const auto &[action, child] : n->GetActions()) {
        frames.push({child,
                     frame.prob * static_cast<Rational>(n->GetInfoset()->GetActionProb(action)),
                     frame.progress});
      }
      continue;
    }

    const auto &chain = chains.at(n->GetPlayer());
    const size_t index = frame.progress.at(n->GetPlayer());
    if (index + 1 >= chain.size()) {
      // This player has already realised their designated sequence; any
      // further move of theirs here is inconsistent with this profile.
      continue;
    }
    const GameSequence &next = chain[index + 1];
    if (next->GetInfoset() != n->GetInfoset()) {
      // This is not the information set at which this player's next
      // designated move occurs; this branch cannot realise the profile.
      continue;
    }
    auto progress = frame.progress;
    progress[n->GetPlayer()] = index + 1;
    frames.push({n->GetChild(next->GetAction()->GetLabel()), frame.prob, std::move(progress)});
  }
}

/// One frame of the explicit-stack traversal used by GetPayoff().  As well
/// as the reach probability and each player's progress (see
/// SequenceWalkFrame), tracks the cumulative payoff to the player of
/// interest from every outcome encountered from the root up to and
/// including this node -- matching Gambit's general convention that an
/// outcome may be attached to a non-terminal node, in which case it is
/// received in addition to whatever happens later in the game.
struct PayoffWalkFrame {
  GameNode node;
  Rational prob;
  Rational cumulative;
  std::map<GamePlayer, size_t> progress;
};

} // end anonymous namespace

Rational PureSequenceProfile::GetPayoff(const GamePlayer &p_player) const
{
  std::map<GamePlayer, std::vector<GameSequence>> chains;
  std::map<GamePlayer, size_t> initialProgress;
  for (auto player : m_efg->GetPlayers()) {
    std::vector<GameSequence> chain;
    for (GameSequence seq = GetSequence(player); seq; seq = seq->GetParent()) {
      chain.push_back(seq);
    }
    std::reverse(chain.begin(), chain.end()); // chain.front() is the empty sequence
    chains[player] = chain;
    initialProgress[player] = 0;
  }

  Rational payoff(0);
  std::stack<PayoffWalkFrame> frames;
  frames.push({m_efg->GetRoot(), Rational(1), Rational(0), initialProgress});

  while (!frames.empty()) {
    PayoffWalkFrame frame = std::move(frames.top());
    frames.pop();
    const GameNode &n = frame.node;

    if (n->GetOutcome()) {
      frame.cumulative += n->GetOutcome()->GetPayoff<Rational>(p_player);
    }

    if (n->IsTerminal()) {
      // Credit the cumulative payoff along this path only if every
      // player's progress has caught up to the sequence designated for
      // them -- i.e. this path is one on which the profile is exactly
      // realised, not merely a prefix of it.
      const bool matches = std::all_of(chains.begin(), chains.end(), [&](const auto &entry) {
        return frame.progress.at(entry.first) + 1 == entry.second.size();
      });
      if (matches) {
        payoff += frame.prob * frame.cumulative;
      }
      continue;
    }
    if (n->GetPlayer()->IsChance()) {
      for (const auto &[action, child] : n->GetActions()) {
        frames.push({child,
                     frame.prob * static_cast<Rational>(n->GetInfoset()->GetActionProb(action)),
                     frame.cumulative, frame.progress});
      }
      continue;
    }

    const auto &chain = chains.at(n->GetPlayer());
    const size_t index = frame.progress.at(n->GetPlayer());
    if (index + 1 >= chain.size()) {
      // This player has already realised their designated sequence; any
      // further move of theirs here is inconsistent with this profile.
      continue;
    }
    const GameSequence &next = chain[index + 1];
    if (next->GetInfoset() != n->GetInfoset()) {
      // This is not the information set at which this player's next
      // designated move occurs; this branch cannot realise the profile.
      continue;
    }
    auto progress = frame.progress;
    progress[n->GetPlayer()] = index + 1;
    frames.push({n->GetChild(next->GetAction()->GetLabel()), frame.prob, frame.cumulative,
                 std::move(progress)});
  }
  return payoff;
}

Rational PureSequenceProfile::GetRealizationProbability() const
{
  Rational total(0);
  WalkRealizedNodes(*this, [&](const GameNode &n, const Rational &prob) {
    if (n->IsTerminal()) {
      total += prob;
    }
  });
  return total;
}

SequenceContingencies::iterator::iterator(const Game &p_efg,
                                          const std::shared_ptr<SequenceMap> p_sequences,
                                          bool p_end)
  : m_efg(p_efg), m_sequences(p_sequences), m_end(p_end)
{
  for (auto [player, sequences] : *m_sequences) {
    m_indices[player] = 0;
  }
}

PureSequenceProfile SequenceContingencies::iterator::operator*() const
{
  PureSequenceProfile ret(m_efg);
  for (auto [player, index] : m_indices) {
    ret.SetSequence(m_sequences->at(player)[index]);
  }
  return ret;
}

PureSequenceProfile SequenceContingencies::iterator::operator->() const
{
  PureSequenceProfile ret(m_efg);
  for (auto [player, index] : m_indices) {
    ret.SetSequence(m_sequences->at(player)[index]);
  }
  return ret;
}

SequenceContingencies::iterator &SequenceContingencies::iterator::operator++()
{
  for (auto [player, index] : m_indices) {
    if (index < m_sequences->at(player).size() - 1) {
      m_indices[player]++;
      return *this;
    }
    m_indices[player] = 0;
  }
  m_end = true;
  return *this;
}

SequenceContingencies GameRep::GetSequenceContingencies() const
{
  auto sequences = std::make_shared<SequenceMap>();
  for (auto player : GetPlayers()) {
    for (auto sequence : player->GetSequences()) {
      (*sequences)[player].push_back(sequence);
    }
  }
  return {std::const_pointer_cast<GameRep>(shared_from_this()), sequences};
}

} // end namespace Gambit
