//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/gamedoc.h
// Declaration of game document class
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

#ifndef GAMBIT_GUI_GAMEDOC_H
#define GAMBIT_GUI_GAMEDOC_H

#include "gambit.h"
#include "style.h"
#include "analysis.h"

class TiXmlNode;

namespace Gambit::GUI {

class GameView;
class GameDocument;

//!
//! This class manages the "stack" of supports obtained by eliminating
//! dominated strategies from consideration.
//!
class StrategyDominanceStack {
  GameDocument *m_doc;
  bool m_strict;
  Array<std::shared_ptr<StrategySupportProfile>> m_supports;
  size_t m_current{0};
  bool m_noFurther;

public:
  StrategyDominanceStack(GameDocument *p_doc, bool p_strict);
  ~StrategyDominanceStack() = default;

  //!
  //! Returns the number of supports in the stack
  //!
  int NumSupports() const { return m_supports.size(); }

  //!
  //! Get the i'th support in the stack
  //! (where i=1 is always the "full" support)
  //!
  const StrategySupportProfile &GetSupport(int i) const { return *m_supports[i]; }

  //!
  //! Get the current support
  //!
  const StrategySupportProfile &GetCurrent() const { return *m_supports[m_current]; }

  //!
  //! Get the level of iteration (1 = no iteration)
  //!
  int GetLevel() const { return m_current; }

  //!
  //! Sets whether elimination is strict or weak.  If this changes the
  //! internal setting, a Reset() is done
  //!
  void SetStrict(bool p_strict);

  //!
  //! Gets whether elimination is strict or weak.
  //!
  bool GetStrict() const { return m_strict; }

  //!
  //! Reset the stack by clearing out all supports
  //!
  void Reset();

  //!
  //! Go to the next level of iteration.  Returns 'true' if successful,
  //! 'false' if no effect (i.e., no further actions could be eliminated)
  //!
  bool NextLevel();

  //!
  //! Go to the previous level of iteration.  Returns 'true' if successful,
  //! 'false' if no effect (i.e., already at the full support)
  //!
  bool PreviousLevel();

  //!
  //! Go to the top level (the full support)
  //!
  void TopLevel() { m_current = 1; }

  //!
  //! Returns 'false' if it is known that no further eliminations can be done
  //!
  bool CanEliminate() const { return (m_current < m_supports.size() || !m_noFurther); }
};

//
// These are passed to GameDocument::UpdateViews() to indicate which
// types of modifications have occurred.
//
// GBT_DOC_MODIFIED_GAME: The game itself has been modified; that is, the
// physical structure (number of players, number of strategies, game tree)
// has changed.  This requires all computed data to be deleted.
//
// GBT_DOC_MODIFIED_PAYOFFS: The payoffs of the game have changed, but not
// the physical structure.  This means that strategy profiles can, in
// principle, be kept -- but they may no longer be Nash, etc.
//
// GBT_DOC_MODIFIED_LABELS: Labeling of players, strategies, etc. has
// changed.
//
// GBT_DOC_MODIFIED_WORKSPACE: Stored analysis workspace data has changed.
// This affects workbook/workspace persistence, but does not modify the
// underlying game model.
//
// GBT_DOC_MODIFIED_PRESENTATION: Settings on how information is rendered
// (for example, colors) have changed.
//
enum class GameModificationType : unsigned int {
  None = 0x00,
  GameForm = 0x01,
  GamePayoffs = 0x02,
  GameLabels = 0x04,
  Workspace = 0x08,
  Presentation = 0x10
};

inline GameModificationType operator|(GameModificationType p_left, GameModificationType p_right)
{
  return static_cast<GameModificationType>(static_cast<unsigned int>(p_left) |
                                           static_cast<unsigned int>(p_right));
}

inline GameModificationType operator&(GameModificationType p_left, GameModificationType p_right)
{
  return static_cast<GameModificationType>(static_cast<unsigned int>(p_left) &
                                           static_cast<unsigned int>(p_right));
}

inline GameModificationType &operator|=(GameModificationType &p_left, GameModificationType p_right)
{
  p_left = p_left | p_right;
  return p_left;
}

inline bool HasModification(GameModificationType p_modifications, GameModificationType p_test)
{
  return static_cast<unsigned int>(p_modifications & p_test) != 0;
}

class AnalysisWorkspace {
  GameDocument *m_doc;

  StrategyDominanceStack m_stratSupports;

  Array<std::shared_ptr<AnalysisOutput>> m_profiles;
  int m_currentProfileList;

public:
  explicit AnalysisWorkspace(GameDocument *p_doc);

  void Clear();
  void ResetForGameChange();
  void BuildNfg();

  const AnalysisOutput &GetProfiles() const { return *m_profiles[m_currentProfileList]; }
  const AnalysisOutput &GetProfiles(int p_index) const { return *m_profiles[p_index]; }
  void AddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs);
  void SelectEquilibriumOutput(int p_index);
  int NumProfileLists() const { return m_profiles.size(); }
  int GetCurrentProfileList() const { return m_currentProfileList; }

  int GetCurrentProfile() const
  {
    return (m_profiles.size() == 0) ? 0 : GetProfiles().GetCurrent();
  }
  void SelectProfile(int p_profile);

  const StrategySupportProfile &GetNfgSupport() const { return m_stratSupports.GetCurrent(); }
  void SetDominanceStrictness(bool p_strict);
  bool GetStrategyElimStrength() const;
  bool NextDominanceLevel();
  void PreviousDominanceLevel();
  void TopDominanceLevel();
  bool CanStrategyElim() const;
  int GetStrategyElimLevel() const;

  void Save(std::ostream &) const;
  bool Load(TiXmlNode *p_game);
};

class GameDocument {
  friend class GameView;

  std::list<GameView *> m_views;

  void AddView(GameView *p_view) { m_views.push_back(p_view); }
  void RemoveView(GameView *p_view)
  {
    m_views.erase(std::find(m_views.begin(), m_views.end(), p_view));
    if (m_views.empty()) {
      delete this;
    }
  }

  Game m_game;
  wxString m_filename;

  TreeRenderConfig m_style;
  GameNode m_selectNode;
  bool m_gameModified, m_workspaceModified;

  AnalysisWorkspace m_workspace;

  void NotifyChanged(GameModificationType p_modifications);
  void UpdateViews();

public:
  explicit GameDocument(Game p_game);
  ~GameDocument();

  //!
  //! @name Reading and writing savefiles
  //!
  //@{
  /// Load workspace from the specified file (which should be a .gbt file)
  /// Returns true if successful, false if error
  bool LoadWorkspace(const wxString &p_filename);
  void SaveWorkspace(std::ostream &) const;
  //@}

  Game GetGame() const { return m_game; }
  void BuildNfg();

  const wxString &GetFilename() const { return m_filename; }
  void SetFilename(const wxString &p_filename) { m_filename = p_filename; }

  bool IsModified() const { return m_gameModified || m_workspaceModified; }
  bool IsGameModified() const { return m_gameModified; }
  bool IsWorkspaceModified() const { return m_workspaceModified; }
  void SetGameModified(bool p_modified) { m_gameModified = p_modified; }
  void SetWorkspaceModified(bool p_unsaved) { m_workspaceModified = p_unsaved; }

  const AnalysisWorkspace &GetWorkspace() const { return m_workspace; }

  const TreeRenderConfig &GetStyle() const { return m_style; }
  void SetStyle(const TreeRenderConfig &p_style);

  GameAction GetAction(int p_index) const;

  void DoAddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs);
  void DoSelectEquilibriumOutput(int p_index);
  void DoSelectProfile(int p_profile);

  BehaviorSupportProfile GetEfgSupport() const { return BehaviorSupportProfile(m_game); }
  const StrategySupportProfile &GetNfgSupport() const { return m_workspace.GetNfgSupport(); }

  void DoSetDominanceStrictness(bool p_strict);
  bool DoNextDominanceLevel();
  void DoPreviousDominanceLevel();
  void DoTopDominanceLevel();

  GameNode GetSelectNode() const { return m_selectNode; }
  void SetSelectNode(GameNode);

  /// Call to ask viewers to post any pending changes
  void PostPendingChanges();

  /// Operations on game model
  enum class GameSaveFormat { Efg, Nfg, Workspace };
  GameSaveFormat GetCurrentSaveFormat() const
  {
    if (m_filename.EndsWith(".efg")) {
      return GameSaveFormat::Efg;
    }
    if (m_filename.EndsWith(".nfg")) {
      return GameSaveFormat::Nfg;
    }
    return GameSaveFormat::Workspace;
  }
  void DoSave(const wxString &p_filename, GameSaveFormat p_format);
  void DoSetTitle(const wxString &p_title, const wxString &p_comment);
  GamePlayer DoNewPlayer();
  void DoSetPlayerLabel(GamePlayer p_player, const wxString &p_label);
  void DoNewStrategy(GamePlayer p_player);
  void DoDeleteStrategy(GameStrategy p_strategy);
  void DoSetStrategyLabel(GameStrategy p_strategy, const wxString &p_label);
  void DoSetInfosetLabel(GameInfoset p_infoset, const wxString &p_label);
  void DoSetActionLabel(GameAction p_action, const wxString &p_label);
  void DoSetActionProbs(GameInfoset p_infoset, const Array<Number> &p_probs);
  void DoSetInfoset(GameNode p_node, GameInfoset p_infoset);
  void DoLeaveInfoset(GameNode p_node);
  void DoRevealAction(GameInfoset p_infoset, GamePlayer p_player);
  void DoInsertAction(GameNode p_node);
  void DoSetNodeLabel(GameNode p_node, const wxString &p_label);
  void DoAppendMove(GameNode p_node, GameInfoset p_infoset);
  void DoInsertMove(GameNode p_node, GamePlayer p_player, unsigned int p_actions);
  void DoInsertMove(GameNode p_node, GameInfoset p_infoset);
  void DoCopyTree(GameNode p_destNode, GameNode p_srcNode);
  void DoMoveTree(GameNode p_destNode, GameNode p_srcNode);
  void DoDeleteParent(GameNode p_node);
  void DoDeleteTree(GameNode p_node);
  void DoSetPlayer(GameInfoset p_infoset, GamePlayer p_player);
  void DoSetPlayer(GameNode p_node, GamePlayer p_player);
  void DoNewOutcome(GameNode p_node);
  void DoNewOutcome(const PureStrategyProfile &p_profile);
  void DoSetOutcome(GameNode p_node, GameOutcome p_outcome);
  void DoSetOutcomeData(const GameNode &p_node, const wxString &p_label,
                        const std::vector<wxString> &p_payoffs);
  void DoRemoveOutcome(GameNode p_node);
  void DoCopyOutcome(GameNode p_node, GameOutcome p_outcome);
  void DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value);

  void DoAnalysisOutputChanged();
};

inline GameDocument *NewTreeDocument()
{
  const Game efg = NewTree();
  efg->SetTitle("Untitled Extensive Game");
  efg->NewPlayer("Player 1");
  efg->NewPlayer("Player 2");
  return new GameDocument(efg);
}

inline GameDocument *NewTableDocument(const std::vector<int> &p_dim)
{
  const Game nfg = NewTable(p_dim);
  nfg->SetTitle("Untitled Strategic Game");
  int pl = 1;
  for (auto player : nfg->GetPlayers()) {
    player->SetLabel("Player " + std::to_string(pl++));
  }
  return new GameDocument(nfg);
}

class GameView {
protected:
  GameDocument *m_doc;

public:
  explicit GameView(GameDocument *p_doc) : m_doc(p_doc) { m_doc->AddView(this); }
  virtual ~GameView() { m_doc->RemoveView(this); }

  virtual void OnUpdate() = 0;

  /// Post any pending changes in the viewer to the document
  virtual void PostPendingChanges() {}

  GameDocument *GetDocument() const { return m_doc; }
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_GAMEDOC_H
