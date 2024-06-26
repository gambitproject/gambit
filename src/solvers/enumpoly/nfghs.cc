//
// This file is part of Gambit
// Copyright (c) 1994-2024, Litao Wei The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfghs.cc
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

#include <iostream>
#include "gambit.h"
#include "nfghs.h"
#include "enumpoly.h"

using namespace Nash;

extern void PrintProfile(std::ostream &p_stream, const std::string &p_label,
                         const MixedStrategyProfile<double> &p_profile);

//---------------------------------------------------------------------------
//                      gbtNfgHs: member functions
//---------------------------------------------------------------------------

void gbtNfgHs::Solve(const Game &game)
{
  int i;
  int size, diff, maxsize, maxdiff;
  bool preferBalance;

  Initialize(game);

  // first, setup default, for "automatic", or for malformed ordering argument
  preferBalance = false;
  if (numPlayers == 2) {
    preferBalance = true;
  }
  // then, change if specified otherwise
  if (Ordering() == "balance") {
    preferBalance = true;
  }
  if (Ordering() == "size") {
    preferBalance = false;
  }

  List<MixedStrategyProfile<double>> solutions;

  // Iterate over possible sizes and differences of the support size profile.
  // The order of iteration is determined by preferBalance.
  // Diff is defined to be difference between the maximum and minimum size support

  maxsize = 0;
  for (i = 1; i <= numPlayers; i++) {
    maxsize += numActions[i];
  }

  maxdiff = maxActions - 1;

  size = numPlayers;
  diff = 0;
  while ((size <= maxsize) && (diff <= maxdiff)) {

    SolveSizeDiff(game, solutions, size, diff);
    if ((m_stopAfter > 0) && (solutions.Length() >= m_stopAfter)) {
      size = maxsize + 1;
      diff = maxdiff + 1;
    }

    if (preferBalance) {
      size++;
      if (size > maxsize) {
        size = numPlayers;
        diff++;
      }
    }
    else {
      diff++;
      if (diff > maxdiff) {
        diff = 0;
        size++;
      }
    }
  }

  Cleanup(game);
}

void gbtNfgHs::Initialize(const Game &p_game)
{

  // remove strict dominated strategies

  Array<GameStrategy> strategies_list;

  numPlayers = p_game->NumPlayers();
  numActions = p_game->NumStrategies();

  int i;
  minActions = numActions[1];
  maxActions = numActions[1];

  for (i = 2; i <= numPlayers; i++) {
    if (minActions > numActions[i]) {
      minActions = numActions[i];
    }
    if (maxActions < numActions[i]) {
      maxActions = numActions[i];
    }
  }
}

void gbtNfgHs::Cleanup(const Game &game) {}

gbtNfgHs::gbtNfgHs(int p_stopAfter)
  : m_iteratedRemoval(true), m_removalWhenUninstantiated(1), m_ordering("automatic"),
    m_stopAfter(p_stopAfter)
{
}

void gbtNfgHs::SolveSizeDiff(const Game &game, List<MixedStrategyProfile<double>> &solutions,
                             int size, int diff)
{
  int i, j;
  Array<int> supportSizeProfile(numPlayers);
  for (i = 1; i <= numPlayers; i++) {
    supportSizeProfile[i] = 1;
  }
  supportSizeProfile[1] = 0;
  int currdiff, currsize, currminNumActions, currmaxNumActions;
  currsize = numPlayers;
  while ((currsize <= size) && (supportSizeProfile[numPlayers] <= numActions[numPlayers])) {
    j = 1;
    supportSizeProfile[j] += 1;

    currminNumActions = supportSizeProfile[1];
    currmaxNumActions = supportSizeProfile[1];
    for (i = 2; i <= numPlayers; i++) {
      if (currminNumActions > supportSizeProfile[i]) {
        currminNumActions = supportSizeProfile[i];
      }
      if (currmaxNumActions < supportSizeProfile[i]) {
        currmaxNumActions = supportSizeProfile[i];
      }
    }
    currdiff = currmaxNumActions - currminNumActions;

    currsize = 0;
    for (i = 1; i <= numPlayers; i++) {
      currsize += supportSizeProfile[i];
    }
    while ((j != numPlayers) &&
           ((supportSizeProfile[j] == numActions[j] + 1) || (currsize > size))) {
      supportSizeProfile[j] = 1;
      j++;
      supportSizeProfile[j] = supportSizeProfile[j] + 1;

      currminNumActions = supportSizeProfile[1];
      currmaxNumActions = supportSizeProfile[1];
      for (i = 2; i <= numPlayers; i++) {
        if (currminNumActions > supportSizeProfile[i]) {
          currminNumActions = supportSizeProfile[i];
        }
        if (currmaxNumActions < supportSizeProfile[i]) {
          currmaxNumActions = supportSizeProfile[i];
        }
      }
      currdiff = currmaxNumActions - currminNumActions;

      currsize = 0;
      for (i = 1; i <= numPlayers; i++) {
        currsize += supportSizeProfile[i];
      }
    }

    if ((currdiff != diff) || (currsize != size)) {
      continue;
    }
    if (supportSizeProfile[numPlayers] <= numActions[numPlayers]) {
      SolveSupportSizeProfile(game, solutions, supportSizeProfile);
    }
    if (m_stopAfter > 0 && solutions.Length() >= m_stopAfter) {
      return;
    }
  }
}

bool gbtNfgHs::SolveSupportSizeProfile(const Game &game,
                                       List<MixedStrategyProfile<double>> &solutions,
                                       const Array<int> &supportSizeProfile)
{
  Array<Array<GameStrategy>> uninstantiatedSupports(numPlayers);
  Array<Array<Array<GameStrategy>>> domains(numPlayers);
  PVector<int> playerSupport(supportSizeProfile);

  for (int i = 1; i <= numPlayers; i++) {
    uninstantiatedSupports[i] = Array<GameStrategy>();
    int m = 1;
    for (int j = 1; j <= supportSizeProfile[i]; j++) {
      playerSupport(i, j) = m;
      m++;
    }
  }

  for (int i = 1; i <= numPlayers; i++) {
    bool success = false;
    do {
      Array<GameStrategy> supportBlock;
      GetSupport(game, i, playerSupport.GetRow(i), supportBlock);
      domains[i].push_back(supportBlock);
      success = UpdatePlayerSupport(game, i, playerSupport);
    } while (success);
  }
  return RecursiveBacktracking(game, solutions, uninstantiatedSupports, domains, 1);
}

void gbtNfgHs::GetSupport(const Game &game, int playerIdx, const Vector<int> &support,
                          Array<GameStrategy> &supportBlock)
{
  GamePlayer player = game->GetPlayer(playerIdx);
  for (int i = 1; i <= support.Length(); i++) {
    int strategyIdx = support[i];
    supportBlock.push_back(player->GetStrategy(strategyIdx));
  }
}

bool gbtNfgHs::UpdatePlayerSupport(const Game &game, int playerIdx, PVector<int> &playerSupport)
{
  Vector<int> support = playerSupport.GetRow(playerIdx);
  int lastBit = support.Length();
  while (true) {
    playerSupport(playerIdx, lastBit) += 1;
    int idx = lastBit;
    while (idx > 1) {
      if (playerSupport(playerIdx, idx) == (numActions[playerIdx] + 1)) {
        playerSupport(playerIdx, idx - 1) += 1;
        if (playerSupport(playerIdx, idx - 1) < numActions[playerIdx]) {
          playerSupport(playerIdx, idx) = playerSupport(playerIdx, idx - 1) + 1;
        }
        else {
          playerSupport(playerIdx, idx) = numActions[playerIdx];
        }
      }
      idx--;
    }
    if (playerSupport(playerIdx, 1) == (numActions[playerIdx] + 1)) {
      return false;
    }

    int maxIdx = support.Length();
    for (int i = maxIdx - 1; i >= 1; i--) {
      if (playerSupport(playerIdx, i) >= playerSupport(playerIdx, maxIdx)) {
        maxIdx = i;
      }
    }
    if (maxIdx != lastBit) {
      continue;
    }
    else {
      break;
    }
  }
  return true;
}

bool gbtNfgHs::RecursiveBacktracking(const Game &game,
                                     List<MixedStrategyProfile<double>> &solutions,
                                     Array<Array<GameStrategy>> &uninstantiatedSupports,
                                     Array<Array<Array<GameStrategy>>> &domains,
                                     int idxNextSupport2Instantiate)
{
  int idx = idxNextSupport2Instantiate;
  if (idx == numPlayers + 1) {
    return FeasibilityProgram(game, solutions, uninstantiatedSupports);
  }
  else {
    // m_logfile << "Player " << idx << " domains length: " << domains[idx].Length() << " @@\n";
    int domainLength = domains[idx].Length();
    for (int k = 1; k <= domainLength; k++) {
      uninstantiatedSupports[idx] = domains[idx][k];
      Array<Array<Array<GameStrategy>>> newDomains(numPlayers);
      for (int ii = 1; ii <= idx; ii++) {
        newDomains[ii].push_back(uninstantiatedSupports[ii]);
      }
      for (int ii = idx + 1; ii <= numPlayers; ii++) {
        newDomains[ii] = domains[ii];
      }
      bool success = true;
      if (IteratedRemoval()) {
        success = IteratedRemovalStrictlyDominatedStrategies(game, newDomains);
      }
      if (success) {
        if (RecursiveBacktracking(game, solutions, uninstantiatedSupports, newDomains, idx + 1)) {
          if ((m_stopAfter > 0) && (solutions.Length() >= m_stopAfter)) {
            return true;
          }
        }
      }
    }

    return false;
  }
}

bool gbtNfgHs::IteratedRemovalStrictlyDominatedStrategies(
    const Game &game, Array<Array<Array<GameStrategy>>> &domains)
{
  bool changed = false;
  Array<Array<GameStrategy>> domainStrategies(numPlayers);

  GetDomainStrategies(domains, domainStrategies);

  do {
    changed = false;
    for (int i = 1; i <= numPlayers; i++) {
      GamePlayer player = game->GetPlayer(i);

      // construct restrict game
      StrategySupportProfile dominatedGame(game);

      for (int pl = 1; pl <= numPlayers; pl++) {
        if (pl != i) {
          for (int st = 1; st <= game->GetPlayer(pl)->NumStrategies(); st++) {
            if (!domainStrategies[pl].Contains(game->GetPlayer(pl)->GetStrategy(st))) {
              dominatedGame.RemoveStrategy(game->GetPlayer(pl)->GetStrategy(st));
            }
          }
        }
      }
      // construct end

      for (int ai = 1; ai <= numActions[i]; ai++) {

        GameStrategy stra = player->GetStrategy(ai);
        int straIdx = domainStrategies[i].Find(stra);
        if (straIdx == 0) {
          continue;
        }
        bool dominated = false;
        for (int aj = 1; aj <= numActions[i]; aj++) {
          if (aj != ai) {
            if (IsConditionalDominatedBy(dominatedGame, domainStrategies, stra,
                                         player->GetStrategy(aj), true)) {
              dominated = true;
              break;
            }
          }
        }

        if (dominated) {
          bool success = RemoveFromDomain(domains, domainStrategies, i, straIdx);
          changed = true;
          if (!success) {
            return false;
          }
        }
      }
    }
  } while (changed);

  return true;
}

void gbtNfgHs::GetDomainStrategies(Array<Array<Array<GameStrategy>>> &domains,
                                   Array<Array<GameStrategy>> &domainStrategies) const
{

  for (int i = 1; i <= numPlayers; i++) {
    domainStrategies[i] = Array<GameStrategy>();
    for (int j = 1; j <= domains[i].Length(); j++) {
      for (int k = 1; k <= domains[i][j].Length(); k++) {
        if (domainStrategies[i].Find(domains[i][j][k]) == 0) { // no found
          domainStrategies[i].push_back(domains[i][j][k]);
        }
      }
    }
  }
}

bool gbtNfgHs::IsConditionalDominatedBy(StrategySupportProfile &dominatedGame,
                                        Array<Array<GameStrategy>> &domainStrategies,
                                        const GameStrategy &strategy,
                                        const GameStrategy &checkStrategy, bool strict)
{
  GamePlayer player = strategy->GetPlayer();
  int playerIdx = player->GetNumber();

  int strategyIdx = strategy->GetNumber();
  int checkStrategyIdx = checkStrategy->GetNumber();
  GamePlayer dominatedPlayer = dominatedGame.GetGame()->GetPlayer(playerIdx);
  GameStrategy newStrategy = dominatedPlayer->GetStrategy(strategyIdx);
  GameStrategy newCheckStrategy = dominatedPlayer->GetStrategy(checkStrategyIdx);

  return dominatedGame.Dominates(newCheckStrategy, newStrategy, strict);
}

bool gbtNfgHs::RemoveFromDomain(Array<Array<Array<GameStrategy>>> &domains,
                                Array<Array<GameStrategy>> &domainStrategies, int player,
                                int removeStrategyIdx)
{

  GameStrategy removeStrategy = domainStrategies[player][removeStrategyIdx];
  for (int j = 1; j <= domains[player].Length(); j++) {
    int idx = domains[player][j].Find(removeStrategy);
    if (idx > 0) {
      domains[player].Remove(j);
      j--;
    }
  }
  GetDomainStrategies(domains, domainStrategies);
  return (domains[player].Length() != 0);
}

bool gbtNfgHs::FeasibilityProgram(const Game &game, List<MixedStrategyProfile<double>> &solutions,
                                  Array<Array<GameStrategy>> &uninstantiatedSupports) const
{
  StrategySupportProfile restrictedGame(game);
  for (int pl = 1; pl <= numPlayers; pl++) {
    for (int st = 1; st <= game->GetPlayer(pl)->NumStrategies(); st++) {
      if (!uninstantiatedSupports[pl].Contains(game->GetPlayer(pl)->GetStrategy(st))) {
        restrictedGame.RemoveStrategy(game->GetPlayer(pl)->GetStrategy(st));
      }
    }
  }

  bool is_singular;
  auto newSolutions =
      EnumPolyStrategySupportSolve(restrictedGame, is_singular, (m_stopAfter == 1) ? 1 : 0);

  bool gotSolutions = false;
  for (auto soln : newSolutions) {
    MixedStrategyProfile<double> solutionToTest = soln.ToFullSupport();
    if (solutionToTest.GetLiapValue() < .01) {
      gotSolutions = true;
      PrintProfile(std::cout, "NE", solutionToTest);
      solutions.push_back(solutionToTest);
      if ((m_stopAfter > 0) && (solutions.Length() >= m_stopAfter)) {
        return true;
      }
    }
  }
  return gotSolutions;
}
