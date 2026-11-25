//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/gameworkspace.cc
// A game being worked on, with its undo history
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

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "games/gametree.h"
#include "gameworkspace.h"

namespace Gambit::GUI {

using enum GameModificationType;

GameWorkspace::GameWorkspace(Game p_game) : m_game(std::move(p_game)), m_analyses(this) {}

void GameWorkspace::AddObserver(WorkspaceObserver *p_observer)
{
  m_observers.push_back(p_observer);
}

void GameWorkspace::RemoveObserver(WorkspaceObserver *p_observer)
{
  std::erase(m_observers, p_observer);
}

bool GameWorkspace::CanApply(const GameEdit &p_edit) const
{
  return !CheckEdit(m_game, p_edit).has_value();
}

void GameWorkspace::Apply(const GameEdit &p_edit)
{
  if (const auto reason = CheckEdit(m_game, p_edit)) {
    throw std::invalid_argument(*reason);
  }
  std::string before = Snapshot();
  GameModificationType modifications;
  try {
    modifications = PerformEdit(m_game, m_style, p_edit);
  }
  catch (...) {
    Restore(before);
    Notify(All);
    throw;
  }
  if (modifications == None) {
    return;
  }
  m_undoList.push_back(std::move(before));
  m_redoList.clear();
  if (HasModification(modifications, GameForm | GamePayoffs)) {
    m_analyses.ResetForGameChange();
  }
  NotifyChanged(modifications);
}

void GameWorkspace::NotifyChanged(GameModificationType p_modifications)
{
  m_gameModified |= HasModification(p_modifications, GameForm | GamePayoffs | GameLabels);
  m_workspaceModified |= HasModification(p_modifications, Workspace | Presentation);
  Notify(p_modifications);
}

void GameWorkspace::Notify(GameModificationType p_modifications)
{
  // An observer may remove itself when notified
  const auto observers = m_observers;
  for (auto *observer : observers) {
    observer->OnWorkspaceChanged(p_modifications);
  }
}

void GameWorkspace::Undo()
{
  if (!CanUndo()) {
    throw std::logic_error("There is no step to undo");
  }
  m_redoList.push_back(Snapshot());
  Restore(m_undoList.back());
  m_undoList.pop_back();
  m_gameModified = m_workspaceModified = true;
  Notify(All);
}

void GameWorkspace::Redo()
{
  if (!CanRedo()) {
    throw std::logic_error("There is no step to redo");
  }
  m_undoList.push_back(Snapshot());
  Restore(m_redoList.back());
  m_redoList.pop_back();
  m_gameModified = m_workspaceModified = true;
  Notify(All);
}

void GameWorkspace::ClearHistory()
{
  m_undoList.clear();
  m_redoList.clear();
}

void GameWorkspace::MarkSaved(bool p_withWorkspace)
{
  m_gameModified = false;
  if (p_withWorkspace) {
    m_workspaceModified = false;
  }
}

void GameWorkspace::Save(std::ostream &p_stream) const
{
  WorkspaceFile workspace;
  m_style.Save(workspace);
  workspace.analyses = m_analyses.Save();
  workspace.game_format = As<GameTreeRep>(m_game) ? "efg" : "nfg";
  if (!As<GameTreeRep>(m_game)) {
    workspace.layout.reset();
    workspace.labels.reset();
  }
  std::ostringstream game_text;
  m_game->Write(game_text, workspace.game_format);
  workspace.game = game_text.str();
  if (!workspace.game.empty() && workspace.game.back() == '\n') {
    workspace.game.pop_back();
  }
  WriteWorkspace(p_stream, workspace);
}

std::string GameWorkspace::Snapshot() const
{
  std::ostringstream stream;
  Save(stream);
  return stream.str();
}

void GameWorkspace::Restore(const std::string &p_snapshot)
{
  std::istringstream input(p_snapshot);
  const WorkspaceFile workspace = ReadWorkspace(input);
  std::istringstream game_text(workspace.game);
  m_game = ReadGame(game_text);
  Load(workspace);
}

void GameWorkspace::Load(const WorkspaceFile &p_file)
{
  m_style = TreeStyle();
  m_style.Load(p_file);
  m_analyses.Load(p_file.analyses);
}

void GameWorkspace::AddAnalysis(const std::shared_ptr<AnalysisOutput> &p_output)
{
  m_analyses.AddEquilibriumOutput(p_output);
  NotifyChanged(Workspace);
}

void GameWorkspace::BuildNfg()
{
  if (As<GameTreeRep>(m_game)) {
    m_analyses.BuildNfg();
  }
}

void GameWorkspace::SelectAnalysis(int p_index)
{
  m_analyses.SelectEquilibriumOutput(p_index);
  NotifyChanged(None);
}

void GameWorkspace::SelectProfile(int p_profile)
{
  m_analyses.SelectProfile(p_profile);
  NotifyChanged(None);
}

void GameWorkspace::SetDominanceStrictness(bool p_strict)
{
  m_analyses.SetDominanceStrictness(p_strict);
  NotifyChanged(None);
}

bool GameWorkspace::NextDominanceLevel()
{
  const bool eliminated = m_analyses.NextDominanceLevel();
  NotifyChanged(None);
  return eliminated;
}

void GameWorkspace::PreviousDominanceLevel()
{
  m_analyses.PreviousDominanceLevel();
  NotifyChanged(None);
}

void GameWorkspace::TopDominanceLevel()
{
  m_analyses.TopDominanceLevel();
  NotifyChanged(None);
}

} // namespace Gambit::GUI
