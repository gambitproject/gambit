//
// Created by Stephen Pasteris on 12/11/2025.
//

#ifndef GAMBIT_BEHAVSPT_SEQ_FORM_H
#define GAMBIT_BEHAVSPT_SEQ_FORM_H

#include "gambit.h"
#include "gameseq.h"

namespace Gambit {

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
