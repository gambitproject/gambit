//
// This file is part of Gambit
// Copyright (c) 1994-2024, Litao Wei and The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfghs.h
// Compute Nash equilibria via heuristic search on game supports
// (Porter, Nudelman & Shoham, 2004)
// Implemented by Litao Wei
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

#ifndef NFGHS_H
#define NFGHS_H

#include "gambit.h"

using namespace Gambit;

class gbtNfgHs {
private:
  int m_stopAfter;
  bool m_iteratedRemoval;
  int m_removalWhenUninstantiated;
  std::string m_ordering;

#ifdef DEBUG
  std::ostream m_logfile;
#endif // DEBUG

  int minActions{0};
  int maxActions{0};
  int numPlayers{0};

  Gambit::Array<int> numActions;

  void Initialize(const Game &game);
  void Cleanup(const Game &game);

  void SolveSizeDiff(const Game &game, Gambit::List<MixedStrategyProfile<double>> &solutions,
                     int size, int diff);

  bool SolveSupportSizeProfile(const Game &game,
                               Gambit::List<MixedStrategyProfile<double>> &solutions,
                               const Gambit::Array<int> &supportSizeProfile);

  void GetSupport(const Game &game, int playerIdx, const Vector<int> &support,
                  Gambit::Array<GameStrategy> &supportBlock);

  bool UpdatePlayerSupport(const Game &game, int playerIdx, Gambit::PVector<int> &playerSupport);

  bool RecursiveBacktracking(const Game &game,
                             Gambit::List<MixedStrategyProfile<double>> &solutions,
                             Gambit::Array<Gambit::Array<GameStrategy>> &uninstantiatedSupports,
                             Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
                             int idxNextSupport2Instantiate);

  bool IteratedRemovalStrictlyDominatedStrategies(
      const Game &game, Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains);

  void GetDomainStrategies(Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
                           Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies) const;

  bool IsConditionalDominatedBy(StrategySupportProfile &dominatedGame,
                                Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies,
                                const GameStrategy &strategy, const GameStrategy &checkStrategy,
                                bool strict);

  bool IsConditionalDominated(StrategySupportProfile &dominatedGame,
                              Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies,
                              const GameStrategy &strategy, bool strict);

  bool RemoveFromDomain(Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
                        Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies, int player,
                        int removeStrategyIdx);

  bool
  FeasibilityProgram(const Game &game, Gambit::List<MixedStrategyProfile<double>> &solutions,
                     Gambit::Array<Gambit::Array<GameStrategy>> &uninstantiatedSupports) const;

public:
  gbtNfgHs(int = 1);

  virtual ~gbtNfgHs() = default;

  int StopAfter() const { return m_stopAfter; }

  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  bool IteratedRemoval() const { return m_iteratedRemoval; }

  void SetIteratedRemoval(bool p_iteratedRemoval) { m_iteratedRemoval = p_iteratedRemoval; }

  int RemovalWhenUninstantiated() const { return m_removalWhenUninstantiated; }

  void SetRemovalWhenUninstantiated(int p_removalWhenUninstantiated)
  {
    m_removalWhenUninstantiated = p_removalWhenUninstantiated;
  }

  std::string Ordering() const { return m_ordering; }

  void SetOrdering(std::string p_ordering) { m_ordering = p_ordering; }

  void Solve(const Game &);
};

#endif // NFGHS_H
