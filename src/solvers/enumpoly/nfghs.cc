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
#include "nfgcpoly.h"
#include "nfghs.h"

extern MixedStrategyProfile<double> ToFullSupport(const MixedStrategyProfile<double> &p_profile);

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

  Gambit::List<MixedStrategyProfile<double>> solutions;

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

  Gambit::Array<GameStrategy> strategies_list;

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
#ifdef DEBUG
    ,
    m_logfile(std::cerr)
#endif // DEBUG
{
}

void gbtNfgHs::SolveSizeDiff(const Game &game,
                             Gambit::List<MixedStrategyProfile<double>> &solutions, int size,
                             int diff)
{

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "Entering SolveSizeDiff with size = " << size << ", diff = " << diff << "\n";
#endif
  //------------------------------------

  int i, j;
  Gambit::Array<int> supportSizeProfile(numPlayers);
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
      //------------------------------------
      // Logging
#ifdef DEBUG
      m_logfile << "Exiting SolveSizeDiff1\n";
#endif
      //------------------------------------
      return;
    }
  }

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "Exiting SolveSizeDiff2\n";
#endif
  //------------------------------------
}

bool gbtNfgHs::SolveSupportSizeProfile(const Game &game,
                                       Gambit::List<MixedStrategyProfile<double>> &solutions,
                                       const Gambit::Array<int> &supportSizeProfile)
{

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "\n************\n";
  m_logfile << "* Solving support size profile: ";
  for (int i = 1; i <= numPlayers; i++) {
    m_logfile << supportSizeProfile[i] << ' ';
  }
  m_logfile << "\n************\n";
#endif
  //------------------------------------

  Gambit::Array<Gambit::Array<GameStrategy>> uninstantiatedSupports(numPlayers);
  Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> domains(numPlayers);
  Gambit::PVector<int> playerSupport(supportSizeProfile);

  for (int i = 1; i <= numPlayers; i++) {
    uninstantiatedSupports[i] = Gambit::Array<GameStrategy>();
    int m = 1;
    for (int j = 1; j <= supportSizeProfile[i]; j++) {
      playerSupport(i, j) = m;
      // m_logfile << "playerSupport: " << i << j << m << std::endl;
      m++;
    }
  }

  for (int i = 1; i <= numPlayers; i++) {
    bool success = false;
    do {
      Gambit::Array<GameStrategy> supportBlock;
      GetSupport(game, i, playerSupport.GetRow(i), supportBlock);
      domains[i].push_back(supportBlock);
      success = UpdatePlayerSupport(game, i, playerSupport);
    } while (success);
  }
  return RecursiveBacktracking(game, solutions, uninstantiatedSupports, domains, 1);
}

void gbtNfgHs::GetSupport(const Game &game, int playerIdx, const Vector<int> &support,
                          Gambit::Array<GameStrategy> &supportBlock)
{

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "* Enter GetSupport for player " << playerIdx << ", choose strategies: ";
  for (int i = 1; i <= support.Length(); i++) {
    m_logfile << support[i] << ' ';
  }
  m_logfile << "\n";
#endif
  //------------------------------------

  GamePlayer player = game->GetPlayer(playerIdx);
  for (int i = 1; i <= support.Length(); i++) {
    int strategyIdx = support[i];
    supportBlock.push_back(player->GetStrategy(strategyIdx));
  }
}

bool gbtNfgHs::UpdatePlayerSupport(const Game &game, int playerIdx,
                                   Gambit::PVector<int> &playerSupport)
{

  Vector<int> support = playerSupport.GetRow(playerIdx);

  //------------------------------------
  // Logging
  // m_logfile << "\n****\n";
  // m_logfile << "* Enter UpdatePlayerSupport for player " << playerIdx << ", current strategiess:
  // "; for ( int i = 1; i <= support.Length(); i++ ) {
  //  m_logfile << support[i] << ' ';
  //}
  // m_logfile << "\n****\n";
  //------------------------------------

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
#ifdef DEBUG
      m_logfile << "Update finished, return false.\n";
#endif
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

bool gbtNfgHs::RecursiveBacktracking(
    const Game &game, Gambit::List<MixedStrategyProfile<double>> &solutions,
    Gambit::Array<Gambit::Array<GameStrategy>> &uninstantiatedSupports,
    Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
    int idxNextSupport2Instantiate)
{

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "\n********\n";
  m_logfile << "* Begin RecursiveBacktracking to instantiate player "
            << idxNextSupport2Instantiate;
  m_logfile << "\n********\n";
#endif
  //------------------------------------

  int idx = idxNextSupport2Instantiate;
  if (idx == numPlayers + 1) {
    //------------------------------------
    // Logging
#ifdef DEBUG
    m_logfile << "\n****************\n";
    m_logfile << "* No more player. Instantiate finished. Now try to solve the restricted game:\n";
    for (int i = 1; i <= uninstantiatedSupports.Length(); i++) {
      m_logfile << "     Player " << i << ": ";
      for (int j = 1; j <= uninstantiatedSupports[i].Length(); j++) {
        m_logfile << uninstantiatedSupports[i][j]->GetNumber() << ' ';
      }
      m_logfile << "\n";
    }
#endif
    //------------------------------------

    bool success = FeasibilityProgram(game, solutions, uninstantiatedSupports);
    //------------------------------------
    // Logging
#ifdef DEBUG
    m_logfile << "* Whether solved : " << success << "\n";
    m_logfile << "****************\n";
#endif
    //------------------------------------

    if (success) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    // m_logfile << "Player " << idx << " domains length: " << domains[idx].Length() << " @@\n";
    int domainLength = domains[idx].Length();
    for (int k = 1; k <= domainLength; k++) {
      uninstantiatedSupports[idx] = domains[idx][k];
      //------------------------------------
      // Logging
#ifdef DEBUG
      m_logfile << "\nNow instantiate strategy for player " << idx << " :";
      for (int i = 1; i <= uninstantiatedSupports[idx].Length(); i++) {
        m_logfile << uninstantiatedSupports[idx][i]->GetNumber() << ' ';
      }
      m_logfile << "\n";
#endif
      //------------------------------------

      Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> newDomains(numPlayers);
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
        // update domain
        // for (int ii = idx + 1; ii <= numPlayers; ii++) {
        //  domains[ii] = newDomains[ii];
        //  }

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
    const Game &game, Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains)
{

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "\n********\n";
  m_logfile << "* Begin IteratedRemovalStrictlyDominatedStrategies:\n";
  m_logfile << "     Current domains:\n";
  for (int i = 1; i <= numPlayers; i++) {
    m_logfile << "     Player " << i << " [" << domains[i].Length() << "]: ";
    for (int j = 1; j <= domains[i].Length(); j++) {
      for (int k = 1; k <= domains[i][j].Length(); k++) {
        m_logfile << domains[i][j][k]->GetNumber();
        if (k != domains[i][j].Length()) {
          m_logfile << ' ';
        }
      }
      if (j != domains[i].Length()) {
        m_logfile << " | ";
      }
    }
    m_logfile << "\n";
  }
  m_logfile << "********\n";
#endif
  //------------------------------------

  // Gambit::Array< Gambit::Array< Gambit::Array< gbtNfgStrategy > > > domains(numPlayers);
  bool changed = false;
  Gambit::Array<Gambit::Array<GameStrategy>> domainStrategies(numPlayers);

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
        // bool dominated = IsConditionalDominated(dominatedGame, domainStrategies, stra, true);

        bool dominated = false;
        for (int aj = 1; aj <= numActions[i]; aj++) {
          if (aj != ai) {
            if (IsConditionalDominatedBy(dominatedGame, domainStrategies, stra,
                                         player->GetStrategy(aj), true)) {
              dominated = true;
#ifdef DEBUG
              m_logfile << "Strategy " << ai << " is dominated by strategy " << aj << " in player "
                        << i << "\n";
#endif
              break;
            }
          }
        }

        if (dominated) {
          bool success = RemoveFromDomain(domains, domainStrategies, i, straIdx);
#ifdef DEBUG
          m_logfile << "* Now remove strategy " << ai << " in player " << i << " : ";
          m_logfile << success << "\n";
#endif
          changed = true;
          if (!success) {
            //------------------------------------
            // Logging
#ifdef DEBUG
            m_logfile << "\n********\n";
            m_logfile << "* End IteratedRemovalStrictlyDominatedStrategies:\n";
            m_logfile << "     After IRSDS, domains exhausted!\n";
            m_logfile << "********\n";
#endif
            //------------------------------------
            return false;
          }
        }
      }
    }
  } while (changed == true);

  //------------------------------------
  // Logging
#ifdef DEBUG
  m_logfile << "\n********\n";
  m_logfile << "* End IteratedRemovalStrictlyDominatedStrategies:\n";
  m_logfile << "     After IRSDS, domains:\n";
  for (int i = 1; i <= numPlayers; i++) {
    m_logfile << "     Player " << i << " [" << domains[i].Length() << "]: ";
    for (int j = 1; j <= domains[i].Length(); j++) {
      for (int k = 1; k <= domains[i][j].Length(); k++) {
        m_logfile << domains[i][j][k]->GetNumber();
        if (k != domains[i][j].Length()) {
          m_logfile << ' ';
        }
      }
      if (j != domains[i].Length()) {
        m_logfile << " | ";
      }
    }
    m_logfile << "\n";
  }
  m_logfile << "********\n";
#endif
  //------------------------------------

  return true;
}

void gbtNfgHs::GetDomainStrategies(
    Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
    Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies) const
{

  for (int i = 1; i <= numPlayers; i++) {
    domainStrategies[i] = Gambit::Array<GameStrategy>();
    for (int j = 1; j <= domains[i].Length(); j++) {
      for (int k = 1; k <= domains[i][j].Length(); k++) {
        if (domainStrategies[i].Find(domains[i][j][k]) == 0) { // no found
          domainStrategies[i].push_back(domains[i][j][k]);
        }
      }
    }
  }
}

bool gbtNfgHs::IsConditionalDominatedBy(
    StrategySupportProfile &dominatedGame,
    Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies, const GameStrategy &strategy,
    const GameStrategy &checkStrategy, bool strict)
{

  //------------------------------------
  // Logging
  // m_logfile << "\n********\n";
  // m_logfile << "* Enter IsConditionalDominates:\n ";
  // m_logfile << "\n********\n";
  //------------------------------------

  GamePlayer player = strategy->GetPlayer();
  int playerIdx = player->GetNumber();

  int strategyIdx = strategy->GetNumber();
  int checkStrategyIdx = checkStrategy->GetNumber();
  GamePlayer dominatedPlayer = dominatedGame.GetGame()->GetPlayer(playerIdx);
  GameStrategy newStrategy = dominatedPlayer->GetStrategy(strategyIdx);
  GameStrategy newCheckStrategy = dominatedPlayer->GetStrategy(checkStrategyIdx);

  // return newCheckStrategy->Dominates(newStrategy, strict);
  return dominatedGame.Dominates(newCheckStrategy, newStrategy, strict);
}

bool gbtNfgHs::IsConditionalDominated(StrategySupportProfile &dominatedGame,
                                      Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies,
                                      const GameStrategy &strategy, bool strict)
{

  //------------------------------------
  // Logging
  // m_logfile << "\n********\n";
  // m_logfile << "* Enter IsConditionalDominated:\n ";
  // m_logfile << "\n********\n";
  //------------------------------------

  GamePlayer player = strategy->GetPlayer();
  int playerIdx = player->GetNumber();

  int strategyIdx = strategy->GetNumber();
  GamePlayer dominatedPlayer = dominatedGame.GetGame()->GetPlayer(playerIdx);
  GameStrategy newStrategy = dominatedPlayer->GetStrategy(strategyIdx);

  // return newStrategy->IsDominated(strict);
  return dominatedGame.IsDominated(newStrategy, strict);
}

bool gbtNfgHs::RemoveFromDomain(Gambit::Array<Gambit::Array<Gambit::Array<GameStrategy>>> &domains,
                                Gambit::Array<Gambit::Array<GameStrategy>> &domainStrategies,
                                int player, int removeStrategyIdx)
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
  if (domains[player].Length() != 0) {
    return true;
  }
  else {
    return false;
  }
}

bool gbtNfgHs::FeasibilityProgram(
    const Game &game, Gambit::List<MixedStrategyProfile<double>> &solutions,
    Gambit::Array<Gambit::Array<GameStrategy>> &uninstantiatedSupports) const
{
  StrategySupportProfile restrictedGame(game);
  for (int pl = 1; pl <= numPlayers; pl++) {
    for (int st = 1; st <= game->GetPlayer(pl)->NumStrategies(); st++) {
      if (!uninstantiatedSupports[pl].Contains(game->GetPlayer(pl)->GetStrategy(st))) {
        restrictedGame.RemoveStrategy(game->GetPlayer(pl)->GetStrategy(st));
      }
    }
  }

  Gambit::List<MixedStrategyProfile<double>> newSolutions;
  HeuristicPolEnumModule module(restrictedGame, (m_stopAfter == 1) ? 1 : 0);
  module.PolEnum();
  newSolutions = module.GetSolutions();

  bool gotSolutions = false;
  for (int k = 1; k <= newSolutions.Length(); k++) {
    MixedStrategyProfile<double> solutionToTest = ToFullSupport(newSolutions[k]);
    if (solutionToTest.GetLiapValue() < .01) {
      gotSolutions = true;
      PrintProfile(std::cout, "NE", solutionToTest);
      solutions.push_back(solutionToTest);
      if ((m_stopAfter > 0) && (solutions.Length() >= m_stopAfter)) {
        return true;
      }
    }
  }
  if (gotSolutions) {
    return true;
  }
  else {
    return false;
  }
}
