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

#include <algorithm>
#include <map>
#include <optional>

#include "games/behavspt.h"
#include "games/stratspt.h"
#include "style.h"
#include "nashvariants.h"
#include "workspace/gameworkspace.h"

namespace Gambit::GUI {

class GameView;
class GameDocument;

class GameDocument : WorkspaceObserver {
  friend class GameView;

  std::list<GameView *> m_views;

  void AddView(GameView *p_view);
  void RemoveView(GameView *p_view);

  GameWorkspace m_gameWorkspace;
  wxString m_filename;

  NashChoice m_nashChoice;

  /// The catalog game this document was opened from, as it was when opened.
  struct CatalogOrigin {
    wxString slug, version;
    std::string game, title, description;
  };
  std::optional<CatalogOrigin> m_catalogOrigin;
  bool m_isCatalogGame{false};

  std::string WriteGame() const;
  /// Keeps the document's catalog identity in step with its game; see `SetCatalogOrigin`
  void OnWorkspaceChanged(GameModificationType p_modifications) override;

public:
  explicit GameDocument(Game p_game);
  GameDocument(const GameDocument &) = delete;
  GameDocument &operator=(const GameDocument &) = delete;
  ~GameDocument() override;

  //!
  //! @name Reading and writing savefiles
  //!
  //@{
  enum class LoadResult { Success, OpenFailed, ParseFailed, UnsupportedRepresentation };
  struct LoadOutcome {
    LoadResult result;
    std::shared_ptr<GameDocument> document; ///< non-null iff result == LoadResult::Success
  };
  /// Attempt to construct a document by reading `p_filename`, trying it first as a
  /// Gambit workspace (.gbt) and then as a bare game (.efg/.nfg) file.
  static LoadOutcome Load(const wxString &p_filename);
  //@}

  Game GetGame() const { return m_gameWorkspace.GetGame(); }
  /// The wx-free workspace behind this document
  const GameWorkspace &GetGameWorkspace() const { return m_gameWorkspace; }
  void BuildNfg() { m_gameWorkspace.BuildNfg(); }

  const wxString &GetFilename() const { return m_filename; }
  void SetFilename(const wxString &p_filename) { m_filename = p_filename; }

  /// Marks this document as the catalog game p_slug of catalog version p_version, with
  /// no filename. The first edit to the game ends this, and undoing it restores it;
  /// saving ends it for good.
  void SetCatalogOrigin(const wxString &p_slug, const wxString &p_version);
  /// Whether the document is, unchanged, the catalog game it was opened as.
  bool IsCatalogGame() const { return m_isCatalogGame; }
  /// The slug of the catalog game the document was opened as, if any, even if edited since.
  wxString GetCatalogSlug() const { return m_catalogOrigin ? m_catalogOrigin->slug : wxString(); }
  wxString GetCatalogVersion() const
  {
    return m_catalogOrigin ? m_catalogOrigin->version : wxString();
  }

  bool IsModified() const { return IsGameModified() || IsWorkspaceModified(); }
  bool IsGameModified() const { return m_gameWorkspace.IsGameModified(); }
  bool IsWorkspaceModified() const { return m_gameWorkspace.IsWorkspaceModified(); }

  const AnalysisWorkspace &GetWorkspace() const { return m_gameWorkspace.GetAnalyses(); }

  /// The last choices made in the equilibrium dialog; held for the session only.
  const NashChoice &GetNashChoice() const { return m_nashChoice; }
  void SetNashChoice(const NashChoice &p_choice) { m_nashChoice = p_choice; }

  //!
  //! @name Edits and undo
  //!
  //@{
  /// Returns whether `p_edit` can be made
  bool CanApply(const GameEdit &p_edit) const { return m_gameWorkspace.CanApply(p_edit); }
  /// Makes `p_edit` as one undoable step; see `GameWorkspace::Apply`
  void Apply(const GameEdit &p_edit) { m_gameWorkspace.Apply(p_edit); }

  bool CanUndo() const { return m_gameWorkspace.CanUndo(); }
  void Undo() { m_gameWorkspace.Undo(); }
  bool CanRedo() const { return m_gameWorkspace.CanRedo(); }
  void Redo() { m_gameWorkspace.Redo(); }
  //@}

  const TreeStyle &GetStyle() const { return m_gameWorkspace.GetStyle(); }
  /// The colour in which a player is drawn; see `TreeStyle::GetPlayerColor`
  wxColour GetPlayerColor(const GamePlayer &p_player) const
  {
    return ToWxColour(GetStyle().GetPlayerColor(p_player));
  }
  /// The colour in which the player numbered `p_player` (from 1) is drawn
  wxColour GetPlayerColor(int p_player) const
  {
    return ToWxColour(GetStyle().GetPlayerColor(p_player));
  }
  /// The font in which labels are drawn
  wxFont GetLabelFont() const { return Gambit::GUI::GetLabelFont(GetStyle()); }

  void DoAddEquilibriumOutput(const std::shared_ptr<AnalysisOutput> &p_profs)
  {
    m_gameWorkspace.AddAnalysis(p_profs);
  }
  void DoAnalysisOutputChanged() { m_gameWorkspace.AnalysisChanged(); }
  void DoSelectEquilibriumOutput(int p_index) { m_gameWorkspace.SelectAnalysis(p_index); }
  void DoSelectProfile(int p_profile) { m_gameWorkspace.SelectProfile(p_profile); }

  BehaviorSupportProfile GetEfgSupport() const { return BehaviorSupportProfile(GetGame()); }
  const StrategySupportProfile &GetNfgSupport() const { return GetWorkspace().GetNfgSupport(); }

  void DoSetDominanceStrictness(bool p_strict)
  {
    m_gameWorkspace.SetDominanceStrictness(p_strict);
  }
  bool DoNextDominanceLevel() { return m_gameWorkspace.NextDominanceLevel(); }
  void DoPreviousDominanceLevel() { m_gameWorkspace.PreviousDominanceLevel(); }
  void DoTopDominanceLevel() { m_gameWorkspace.TopDominanceLevel(); }

  /// Call to ask viewers to post any pending changes
  void PostPendingChanges();

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
};

inline std::shared_ptr<GameDocument> NewTreeDocument()
{
  const Game efg = NewTree({"Player 1", "Player 2"});
  efg->SetTitle("Untitled Extensive Game");
  return std::make_shared<GameDocument>(efg);
}

inline std::shared_ptr<GameDocument> NewTableDocument(const std::vector<int> &p_dim)
{
  const Game nfg = NewTable(p_dim);
  nfg->SetTitle("Untitled Strategic Game");
  std::map<std::string, std::string> labels;
  int pl = 1;
  for (const auto &player : nfg->GetPlayers()) {
    labels[player->GetLabel()] = "Player " + std::to_string(pl++);
  }
  nfg->RelabelPlayers(labels);
  return std::make_shared<GameDocument>(nfg);
}

class GameView : public WorkspaceObserver {
protected:
  std::shared_ptr<GameDocument> m_doc;

public:
  explicit GameView(const std::shared_ptr<GameDocument> &p_doc) : m_doc(p_doc)
  {
    m_doc->AddView(this);
  }
  GameView(const GameView &) = delete;
  GameView &operator=(const GameView &) = delete;
  ~GameView() override { m_doc->RemoveView(this); }

  void OnWorkspaceChanged(GameModificationType) override { OnUpdate(); }

  virtual void OnUpdate() = 0;

  /// Post any pending changes in the viewer to the document
  virtual void PostPendingChanges() {}

  std::shared_ptr<GameDocument> GetDocument() const { return m_doc; }
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_GAMEDOC_H
