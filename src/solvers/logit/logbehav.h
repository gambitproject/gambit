//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/logit/logbehav.h
// Behavior strategy profile where action probabilities are represented using
// logarithms.
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

#ifndef GAMBIT_SOLVERS_LOGIT_LOGBEHAV_H
#define GAMBIT_SOLVERS_LOGIT_LOGBEHAV_H

using namespace Gambit;

///
/// This is a modified behavior profile class used in the tracing procedure.
/// It assumes that all probabilities are positive; therefore, the log
/// of the probability is well-defined.  Because of this, the realization
/// probability for any node can be computed by taking the sum of the log
/// probabilities of the actions leading up to it, and then exponentiating
/// the sum.
///
/// This means that beliefs can be computed accurately even for information
/// sets for which the information set realization probability is going
/// to zero.  In computing beliefs, for each information set, we find the
/// member reached with maximum probability.  Observe that it must be that
/// this node's realization probability divided by the information set's
/// realization probability must be bounded away from zero.  Beliefs are
/// then computed by subtracting the other nodes' log-realization probability
/// from this "leading" node's log-realization probability, and then
/// exponentiating the result.
///
/// This procedure is significant for this application because it is
/// necessary to accurately compute beliefs for information sets whose
/// realization probabilities are going to zero, so as to be able to
/// get a good approximation to the limiting sequential equilibrium.
///
template <class T> class LogBehavProfile {
protected:
  Game m_game;
  Vector<T> m_probs, m_logProbs;
  std::map<GameAction, int> m_profileIndex;

  // structures for storing cached data
  mutable bool m_cacheValid{false};
  mutable std::map<GameNode, T> m_logRealizProbs;
  mutable std::map<GameNode, T> m_beliefs;
  mutable std::map<GameNode, std::map<GamePlayer, T>> m_nodeValues;
  mutable std::map<GameAction, T> m_actionValues;

  /// A player action taken on the path from the root to the node currently being visited
  struct PathAction {
    int index{0};       // position of the action in the profile
    GamePlayer player;  // the player taking the action
    T belief{0};        // belief at the node where the action is taken
    T logRealizProb{0}; // log realization probability of the node the action leads to
  };

  /// @name Auxiliary functions for computation of interesting values
  //@{
  void ComputeSolutionDataPass2(const GameNode &node) const;
  void ComputeSolutionDataPass1(const GameNode &node) const;
  void ComputeSolutionData() const;
  void DiffActionValuesPass(const GameNode &p_node, std::vector<PathAction> &p_path,
                            Matrix<T> &p_derivs,
                            std::map<GameInfoset, std::map<int, T>> &p_shares) const;
  //@}

public:
  /// @name Lifecycle
  //@{
  explicit LogBehavProfile(const Game &);
  ~LogBehavProfile() = default;

  LogBehavProfile<T> &operator=(const LogBehavProfile<T> &) = delete;

  //@}

  /// @name Operator overloading
  //@{
  bool operator==(const LogBehavProfile<T> &) const;

  void SetProb(const GameAction &p_action, const T &p_value)
  {
    m_probs[m_profileIndex.at(p_action)] = p_value;
    m_logProbs[m_profileIndex.at(p_action)] = log(p_value);
  }

  const T &GetProb(const GameAction &p_action) const
  {
    return m_probs[m_profileIndex.at(p_action)];
  }

  const T &GetLogProb(const GameAction &p_action) const
  {
    return m_logProbs[m_profileIndex.at(p_action)];
  }

  void SetLogProb(int a, const T &p_value)
  {
    Invalidate();
    m_logProbs[a] = p_value;
    m_probs[a] = exp(p_value);
  }
  //@}

  /// @name Initialization, validation
  //@{
  /// Force recomputation of stored quantities
  void Invalidate() const { m_cacheValid = false; }
  //@}

  /// @name General data access
  //@{
  size_t BehaviorProfileLength() const { return m_probs.size(); }
  //@}

  /// @name Computation of interesting quantities
  //@{
  T GetActionProb(const GameAction &act) const;
  T GetLogActionProb(const GameAction &) const;
  const T &GetPayoff(const GameAction &act) const;

  /// @brief Compute the derivatives of all action values with respect to the log
  ///        probabilities of all actions
  /// @param[out] p_derivs Square matrix indexed by profile position; entry (a, b) is the
  ///             derivative of the value of action a with respect to the log probability
  ///             of action b.  Entries where a and b are at the same information set are zero.
  void DiffActionValues(Matrix<T> &p_derivs) const;

  //@}
};

#endif // GAMBIT_SOLVERS_LOGIT_LOGBEHAV_H
