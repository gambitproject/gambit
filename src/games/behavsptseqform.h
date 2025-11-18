//
// Created by Stephen Pasteris on 12/11/2025.
//

#ifndef GAMBIT_BEHAVSPT_SEQ_FORM_H
#define GAMBIT_BEHAVSPT_SEQ_FORM_H

#include "gambit.h"
#include "gameseq.h"

namespace Gambit {

class BehaviorSupportProfile::SequenceContingencies {
private:
  const BehaviorSupportProfile *m_support;

public:
  SequenceContingencies(const BehaviorSupportProfile *p_support) : m_support(p_support) {}

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

  iterator begin() { return {m_support->GetSequenceForm()}; }
  iterator end() { return {m_support->GetSequenceForm(), true}; }
};

} // namespace Gambit

#endif // GAMBIT_BEHAVSPT_SEQ_FORM_H
