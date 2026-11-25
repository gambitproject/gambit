//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/gameworkspace.h
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

#ifndef GAMBIT_GUI_WORKSPACE_GAMEWORKSPACE_H
#define GAMBIT_GUI_WORKSPACE_GAMEWORKSPACE_H

#include <iosfwd>
#include <vector>

#include "games/workspace.h"
#include "analysis.h"
#include "gameedit.h"

namespace Gambit::GUI {

/// Receives notice of each change to a workspace
class WorkspaceObserver {
public:
  virtual ~WorkspaceObserver() = default;

  /// Called after the workspace has changed in the ways given
  virtual void OnWorkspaceChanged(GameModificationType p_modifications) = 0;
};

/// A game being worked on: the game, how it is drawn, its analyses, its undo history and whether
/// it has unsaved changes.
///
/// Every edit to the game or its style goes through `Apply`, which makes it as one undoable step;
/// an edit to the structure or payoffs of the game discards the analyses. Undo is a single linear
/// history of these edits, and restores the analyses there were before each. Adding results and
/// moving among them are not undoable.
class GameWorkspace {
public:
  explicit GameWorkspace(Game p_game);
  GameWorkspace(const GameWorkspace &) = delete;
  GameWorkspace &operator=(const GameWorkspace &) = delete;
  ~GameWorkspace() = default;

  Game GetGame() const { return m_game; }
  const TreeStyle &GetStyle() const { return m_style; }

  const AnalysisWorkspace &GetAnalyses() const { return m_analyses; }

  void AddObserver(WorkspaceObserver *p_observer);
  void RemoveObserver(WorkspaceObserver *p_observer);

  /// @name Edits
  //@{
  /// Returns whether `p_edit` can be made
  bool CanApply(const GameEdit &p_edit) const;
  /// Makes `p_edit` as one undoable step. Throws `std::invalid_argument` if the edit cannot be
  /// made. If the change fails part way, the workspace is restored to its state before it and the
  /// exception is rethrown.
  void Apply(const GameEdit &p_edit);
  /// Reports a change made outside `Apply`, which is not undoable
  void NotifyChanged(GameModificationType p_modifications);
  //@}

  /// @name Analyses
  //@{
  /// Adds a list of results, which becomes the selected one
  void AddAnalysis(const std::shared_ptr<AnalysisOutput> &p_output);
  /// Reports that results have been added to a list
  void AnalysisChanged() { NotifyChanged(GameModificationType::Workspace); }
  /// Restarts the elimination of dominated strategies, for showing the strategic form
  void BuildNfg();
  //@}

  /// @name Navigation among the analyses, which is not undoable
  //@{
  void SelectAnalysis(int p_index);
  void SelectProfile(int p_profile);
  void SetDominanceStrictness(bool p_strict);
  /// Eliminates the next level of dominated strategies; returns false if none could be
  bool NextDominanceLevel();
  void PreviousDominanceLevel();
  void TopDominanceLevel();
  //@}

  /// @name Undo and redo
  //@{
  bool CanUndo() const { return !m_undoList.empty(); }
  /// Restores the state before the most recent undoable step
  void Undo();
  bool CanRedo() const { return !m_redoList.empty(); }
  /// Makes again the most recently undone step
  void Redo();
  /// Discards the undo history
  void ClearHistory();
  //@}

  /// @name Unsaved changes
  //@{
  bool IsGameModified() const { return m_gameModified; }
  bool IsWorkspaceModified() const { return m_workspaceModified; }
  /// Records that the game, and the rest of the workspace if `p_withWorkspace`, has been saved
  void MarkSaved(bool p_withWorkspace);
  //@}

  /// Writes the workspace in the workspace file format
  void Save(std::ostream &p_stream) const;
  /// Takes the style and analyses from a saved workspace file of this game
  void Load(const WorkspaceFile &p_file);

private:
  Game m_game;
  TreeStyle m_style;
  AnalysisWorkspace m_analyses;
  std::vector<WorkspaceObserver *> m_observers;
  std::vector<std::string> m_undoList, m_redoList;
  bool m_gameModified{false}, m_workspaceModified{false};

  std::string Snapshot() const;
  void Restore(const std::string &p_snapshot);
  void Notify(GameModificationType p_modifications);
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_WORKSPACE_GAMEWORKSPACE_H
