//
// Created by Stephen Pasteris on 12/11/2025.
//

#ifndef GAMBIT_BEHAVSPT_SEQ_FORM_H
#define GAMBIT_BEHAVSPT_SEQ_FORM_H

#include "gambit.h"
#include "gameseq.h"

namespace Gambit {

class BehaviorSupportProfile::Sequences {
private:
  const BehaviorSupportProfile *m_support;

public:
  class iterator {
  private:
    const std::shared_ptr<GameSequenceForm> m_sfg;
    std::map<GamePlayer, std::vector<GameSequence>>::const_iterator m_currentPlayer;
    std::vector<GameSequence>::const_iterator m_currentSequence;

  public:
    iterator(const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end) : m_sfg(p_sfg)
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

  Sequences(const BehaviorSupportProfile *p_support) : m_support(p_support) {}

  size_t size() const
  {
    return std::accumulate(
        m_support->GetSequenceForm()->m_sequences.cbegin(),
        m_support->GetSequenceForm()->m_sequences.cend(), 0,
        [](int acc, const std::pair<GamePlayer, std::vector<GameSequence>> &seq) {
          return acc + seq.second.size();
        });
  }

  iterator begin() const { return {m_support->GetSequenceForm(), false}; }
  iterator end() const { return {m_support->GetSequenceForm(), true}; }
};

class BehaviorSupportProfile::Contingencies {
private:
  const std::shared_ptr<GameSequenceForm> m_sfg;

public:
  Contingencies(const std::shared_ptr<GameSequenceForm> p_sfg) : m_sfg(p_sfg) {}

  class iterator {
  private:
    const std::shared_ptr<GameSequenceForm> m_sfg;
    bool m_end{false};
    std::map<GamePlayer, size_t> m_indices;

  public:
    using iterator_category = std::input_iterator_tag;

    iterator(const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end = false)
      : m_sfg(p_sfg), m_end(p_end)
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

  iterator begin() { return {m_sfg}; }
  iterator end() { return {m_sfg, true}; }
};

} // namespace Gambit

#endif // GAMBIT_BEHAVSPT_SEQ_FORM_H
