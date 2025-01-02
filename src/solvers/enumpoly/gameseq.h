//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/gameseq.h
// Interface to sequence form classes
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

#ifndef GAMESEQ_H
#define GAMESEQ_H

#include "gambit.h"
#include "ndarray.h"

namespace Gambit {

struct GameSequenceRep {
public:
  GamePlayer player;
  GameAction action;
  size_t number;
  std::weak_ptr<GameSequenceRep> parent;

  explicit GameSequenceRep(const GamePlayer &p_player, const GameAction &p_action, size_t p_number,
                           std::weak_ptr<GameSequenceRep> p_parent)
    : player(p_player), action(p_action), number(p_number), parent(p_parent)
  {
  }

  GameInfoset GetInfoset(void) const { return (action) ? action->GetInfoset() : nullptr; }

  bool operator<(const GameSequenceRep &other) const
  {
    return player < other.player || (player == other.player && action < other.action);
  }
  bool operator==(const GameSequenceRep &other) const
  {
    return player == other.player && action == other.action;
  }
};

using GameSequence = std::shared_ptr<GameSequenceRep>;

class GameSequenceForm {
private:
  BehaviorSupportProfile m_support;
  std::map<GamePlayer, std::vector<GameSequence>> m_sequences;
  NDArray<Rational> m_payoffs;
  std::map<std::pair<GameInfoset, GameAction>, int> m_constraints; // (sparse) constraint matrices
  std::set<GameInfoset> m_infosets; // infosets actually reachable given support
  std::map<GameAction, GameSequence> m_correspondence;

  void BuildSequences();
  void BuildSequences(const GameNode &, std::map<GamePlayer, GameSequence> &);
  void FillTableau();
  void FillTableau(const GameNode &, const Rational &, std::map<GamePlayer, GameSequence> &);

  Array<int> ProfileToIndex(const std::map<GamePlayer, GameSequence> &p_profile) const
  {
    Array<int> index(p_profile.size());
    for (auto player : GetPlayers()) {
      index[player->GetNumber()] = p_profile.at(player)->number;
    }
    return index;
  }

  Rational &GetPayoffEntry(const std::map<GamePlayer, GameSequence> &p_profile,
                           const GamePlayer &p_player)
  {
    return m_payoffs.at(ProfileToIndex(p_profile), p_player->GetNumber());
  }

public:
  class Infosets {
  private:
    const GameSequenceForm *m_sfg;

  public:
    Infosets(const GameSequenceForm *p_sfg) : m_sfg(p_sfg) {}

    size_t size() const { return m_sfg->m_infosets.size(); }
  };

  class Sequences {
  private:
    const GameSequenceForm *m_sfg;

  public:
    class iterator {
    private:
      const GameSequenceForm *m_sfg;
      std::map<GamePlayer, std::vector<GameSequence>>::const_iterator m_currentPlayer;
      std::vector<GameSequence>::const_iterator m_currentSequence;

    public:
      iterator(const GameSequenceForm *p_sfg, bool p_end) : m_sfg(p_sfg)
      {
        if (p_end) {
          m_currentPlayer = m_sfg->m_sequences.cend();
        }
        else {
          m_currentPlayer = m_sfg->m_sequences.cbegin();
          m_currentSequence = m_currentPlayer->second.cbegin();
        }
      }

      GameSequence operator*() const { return *m_currentSequence; }
      GameSequence operator->() const { return *m_currentSequence; }

      iterator &operator++()
      {
        if (m_currentPlayer == m_sfg->m_sequences.cend()) {
          return *this;
        }
        m_currentSequence++;
        if (m_currentSequence != m_currentPlayer->second.cend()) {
          return *this;
        }
        m_currentPlayer++;
        if (m_currentPlayer != m_sfg->m_sequences.cend()) {
          m_currentSequence = m_currentPlayer->second.cbegin();
        }
        return *this;
      }

      bool operator==(const iterator &it) const
      {
        if (m_sfg != it.m_sfg || m_currentPlayer != it.m_currentPlayer) {
          return false;
        }
        if (m_currentPlayer == m_sfg->m_sequences.end()) {
          return true;
        }
        return (m_currentSequence == it.m_currentSequence);
      }
      bool operator!=(const iterator &it) const { return !(*this == it); }
    };

    Sequences(const GameSequenceForm *p_sfg) : m_sfg(p_sfg) {}

    size_t size() const
    {
      return std::accumulate(
          m_sfg->m_sequences.cbegin(), m_sfg->m_sequences.cend(), 0,
          [](int acc, const std::pair<GamePlayer, std::vector<GameSequence>> &seq) {
            return acc + seq.second.size();
          });
    }

    iterator begin() const { return iterator(m_sfg, false); }
    iterator end() const { return iterator(m_sfg, true); }
  };

  class PlayerSequences {
  private:
    const GameSequenceForm *m_sfg;
    GamePlayer m_player;

  public:
    PlayerSequences(const GameSequenceForm *p_sfg, const GamePlayer &p_player)
      : m_sfg(p_sfg), m_player(p_player)
    {
    }

    size_t size() const { return m_sfg->m_sequences.at(m_player).size(); }

    std::vector<GameSequence>::const_iterator begin() const
    {
      return m_sfg->m_sequences.at(m_player).begin();
    }
    std::vector<GameSequence>::const_iterator end() const
    {
      return m_sfg->m_sequences.at(m_player).end();
    }
  };

  class Contingencies {
  private:
    const GameSequenceForm *m_sfg;

  public:
    Contingencies(const GameSequenceForm *p_sfg) : m_sfg(p_sfg) {}

    class iterator {
    private:
      const GameSequenceForm *m_sfg;
      bool m_end{false};
      std::map<GamePlayer, size_t> m_indices;

    public:
      using iterator_category = std::input_iterator_tag;

      iterator(const GameSequenceForm *p_sfg, bool p_end = false) : m_sfg(p_sfg), m_end(p_end)
      {
        for (auto [player, sequences] : m_sfg->m_sequences) {
          m_indices[player] = 0;
        }
      }

      std::map<GamePlayer, GameSequence> operator*() const
      {
        std::map<GamePlayer, GameSequence> ret;
        for (auto [player, index] : m_indices) {
          ret[player] = m_sfg->m_sequences.at(player)[index];
        }
        return ret;
      }

      std::map<GamePlayer, GameSequence> operator->() const
      {
        std::map<GamePlayer, GameSequence> ret;
        for (auto [player, index] : m_indices) {
          ret[player] = m_sfg->m_sequences.at(player)[index];
        }
        return ret;
      }

      iterator &operator++()
      {
        for (auto [player, index] : m_indices) {
          if (index < m_sfg->m_sequences.at(player).size() - 1) {
            m_indices[player]++;
            return *this;
          }
          m_indices[player] = 0;
        }
        m_end = true;
        return *this;
      }

      bool operator==(const iterator &it) const
      {
        return (m_end == it.m_end && m_sfg == it.m_sfg && m_indices == it.m_indices);
      }
      bool operator!=(const iterator &it) const { return !(*this == it); }
    };

    iterator begin() { return iterator(m_sfg); }
    iterator end() { return iterator(m_sfg, true); }
  };

  explicit GameSequenceForm(const BehaviorSupportProfile &p_support) : m_support(p_support)
  {
    BuildSequences();
    FillTableau();
  }

  ~GameSequenceForm() = default;

  const BehaviorSupportProfile &GetSupport() const { return m_support; }

  Sequences GetSequences() const { return Sequences(this); }

  PlayerSequences GetSequences(const GamePlayer &p_player) const
  {
    return PlayerSequences(this, p_player);
  }

  Contingencies GetContingencies() const { return Contingencies(this); }

  Array<GamePlayer> GetPlayers() const { return m_support.GetGame()->GetPlayers(); }

  Infosets GetInfosets() const { return Infosets(this); }

  const Rational &GetPayoff(const std::map<GamePlayer, GameSequence> &p_profile,
                            const GamePlayer &p_player) const
  {
    return m_payoffs.at(ProfileToIndex(p_profile), p_player->GetNumber());
  }

  int GetConstraintEntry(const GameInfoset &p_infoset, const GameAction &p_action) const
  {
    try {
      return m_constraints.at({p_infoset, p_action});
    }
    catch (std::out_of_range &) {
      return 0;
    }
  }

  MixedBehaviorProfile<double>
  ToMixedBehaviorProfile(const std::map<GameSequence, double> &) const;
};

} // end namespace Gambit

#endif // GAMESEQ_H
