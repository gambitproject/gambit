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
  const std::shared_ptr<GameSequenceForm> m_sfg;

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

  Sequences(const std::shared_ptr<GameSequenceForm> p_sfg) : m_sfg(p_sfg) {}

  size_t size() const
  {
    return std::accumulate(
        m_sfg->m_sequences.cbegin(), m_sfg->m_sequences.cend(), 0,
        [](int acc, const std::pair<GamePlayer, std::vector<GameSequence>> &seq) {
          return acc + seq.second.size();
        });
  }

  iterator begin() const { return {m_sfg, false}; }
  iterator end() const { return {m_sfg, true}; }
};

class BehaviorSupportProfile::PlayerSequences {
private:
  std::shared_ptr<GameSequenceForm> m_sfg;
  GamePlayer m_player;

public:
  PlayerSequences(const std::shared_ptr<GameSequenceForm> p_sfg, const GamePlayer &p_player)
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

} // namespace Gambit

#endif // GAMBIT_BEHAVSPT_SEQ_FORM_H
