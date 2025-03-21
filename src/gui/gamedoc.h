//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#ifndef GAMEDOC_H
#define GAMEDOC_H

#include "gambit.h"
#include "style.h"
#include "analysis.h"

using namespace Gambit;

class gbtGameView;
class gbtGameDocument;

//!
//! This class manages the "stack" of supports obtained by eliminating
//! dominated actions from consideration.
//!
class gbtBehavDominanceStack {
private:
  gbtGameDocument *m_doc;
  bool m_strict;
  Gambit::Array<Gambit::BehaviorSupportProfile *> m_supports;
  int m_current{0};
  bool m_noFurther;

public:
  gbtBehavDominanceStack(gbtGameDocument *p_doc, bool p_strict);
  ~gbtBehavDominanceStack();

  //!
  //! Returns the number of supports in the stack
  //!
  int NumSupports() const { return m_supports.size(); }

  //!
  //! Get the i'th support in the stack
  //! (where i=1 is always the "full" support)
  //!
  const Gambit::BehaviorSupportProfile &GetSupport(int i) const { return *m_supports[i]; }

  //!
  //! Get the current support
  //!
  const Gambit::BehaviorSupportProfile &GetCurrent() const { return *m_supports[m_current]; }

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

//!
//! This class manages the "stack" of supports obtained by eliminating
//! dominated strategies from consideration.
//!
class gbtStrategyDominanceStack {
private:
  gbtGameDocument *m_doc;
  bool m_strict;
  Gambit::Array<Gambit::StrategySupportProfile *> m_supports;
  int m_current{0};
  bool m_noFurther;

public:
  gbtStrategyDominanceStack(gbtGameDocument *p_doc, bool p_strict);
  ~gbtStrategyDominanceStack();

  //!
  //! Returns the number of supports in the stack
  //!
  int NumSupports() const { return m_supports.size(); }

  //!
  //! Get the i'th support in the stack
  //! (where i=1 is always the "full" support)
  //!
  const Gambit::StrategySupportProfile &GetSupport(int i) const { return *m_supports[i]; }

  //!
  //! Get the current support
  //!
  const Gambit::StrategySupportProfile &GetCurrent() const { return *m_supports[m_current]; }

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
// These are passed to gbtGameDocument::UpdateViews() to indicate which
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
// changed.  These have no effect on the game mathematically, so computed
// data may be kept; but, we want to track the label change for undo, etc.
//
// GBT_DOC_MODIFIED_VIEWS: Information about how the document is viewed
// (e.g., player colors) has changed.  We want to track this for undo,
// but, again, this has no effect on the game mathematically.
//
typedef enum {
  GBT_DOC_MODIFIED_NONE = 0x00,
  GBT_DOC_MODIFIED_GAME = 0x01,
  GBT_DOC_MODIFIED_PAYOFFS = 0x02,
  GBT_DOC_MODIFIED_LABELS = 0x04,
  GBT_DOC_MODIFIED_VIEWS = 0x08
} gbtGameModificationType;

class gbtGameDocument {
  friend class gbtGameView;

private:
  Gambit::Array<gbtGameView *> m_views;

  void AddView(gbtGameView *p_view) { m_views.push_back(p_view); }
  void RemoveView(gbtGameView *p_view)
  {
    m_views.erase(std::find(m_views.begin(), m_views.end(), p_view));
    if (m_views.empty()) {
      delete this;
    }
  }

  Gambit::Game m_game;
  wxString m_filename;

  gbtStyle m_style;
  Gambit::GameNode m_selectNode;
  bool m_modified;

  gbtBehavDominanceStack m_behavSupports;
  gbtStrategyDominanceStack m_stratSupports;

  Gambit::Array<gbtAnalysisOutput *> m_profiles;
  int m_currentProfileList;

  std::list<std::string> m_undoList, m_redoList;

  void UpdateViews(gbtGameModificationType p_modifications);

public:
  explicit gbtGameDocument(Gambit::Game p_game);
  ~gbtGameDocument();

  //!
  //! @name Reading and writing .gbt savefiles
  //!
  //@{
  /// Load document from the specified file (which should be a .gbt file)
  /// Returns true if successful, false if error
  bool LoadDocument(const wxString &p_filename, bool p_saveUndo = true);
  void SaveDocument(std::ostream &) const;
  //@}

  Gambit::Game GetGame() const { return m_game; }
  void BuildNfg();

  const wxString &GetFilename() const { return m_filename; }
  void SetFilename(const wxString &p_filename) { m_filename = p_filename; }

  bool IsModified() const { return m_modified; }
  void SetModified(bool p_modified) { m_modified = p_modified; }

  const gbtStyle &GetStyle() const { return m_style; }
  void SetStyle(const gbtStyle &p_style);

  int NumPlayers() const { return m_game->NumPlayers(); }
  bool IsConstSum() const { return m_game->IsConstSum(); }
  bool IsTree() const { return m_game->IsTree(); }
  GameAction GetAction(int p_index) const;

  //!
  //! @name Handling of undo/redo features
  //!
  //@{
  bool CanUndo() const { return (m_undoList.size() > 1); }
  void Undo();

  bool CanRedo() const { return (m_redoList.size() > 0); }
  void Redo();
  //@}

  //!
  //! @name Handling of list of computed profiles
  //!
  //@{
  const gbtAnalysisOutput &GetProfiles() const { return *m_profiles[m_currentProfileList]; }
  const gbtAnalysisOutput &GetProfiles(int p_index) const { return *m_profiles[p_index]; }
  void AddProfileList(gbtAnalysisOutput *);
  void SetProfileList(int p_index);
  int NumProfileLists() const { return m_profiles.size(); }
  int GetCurrentProfileList() const { return m_currentProfileList; }

  int GetCurrentProfile() const
  {
    return (m_profiles.size() == 0) ? 0 : GetProfiles().GetCurrent();
  }
  void SetCurrentProfile(int p_profile);
  /*
  void AddProfiles(const Gambit::List<Gambit::MixedBehavProfile<double> > &);
  void AddProfile(const Gambit::MixedBehavProfile<double> &);
  void AddProfiles(const Gambit::List<Gambit::MixedStrategyProfile<double> > &);
  void AddProfile(const Gambit::MixedStrategyProfile<double> &);
  */
  //@}

  //!
  //! @name Handling of behavior supports
  //!
  //@{
  const Gambit::BehaviorSupportProfile &GetEfgSupport() const
  {
    return m_behavSupports.GetCurrent();
  }
  void SetBehavElimStrength(bool p_strict);
  bool NextBehavElimLevel();
  void PreviousBehavElimLevel();
  void TopBehavElimLevel();
  bool CanBehavElim() const;
  int GetBehavElimLevel() const;
  //@}

  //!
  //! @name Handling of strategy supports
  //!
  //@{
  const Gambit::StrategySupportProfile &GetNfgSupport() const
  {
    return m_stratSupports.GetCurrent();
  }
  void SetStrategyElimStrength(bool p_strict);
  bool GetStrategyElimStrength() const;
  bool NextStrategyElimLevel();
  void PreviousStrategyElimLevel();
  void TopStrategyElimLevel();
  bool CanStrategyElim() const;
  int GetStrategyElimLevel() const;
  //@}

  Gambit::GameNode GetSelectNode() const { return m_selectNode; }
  void SetSelectNode(Gambit::GameNode);

  /// Call to ask viewers to post any pending changes
  void PostPendingChanges();

  /// Operations on game model
  void DoSave(const wxString &p_filename);
  void DoExportEfg(const wxString &p_filename);
  void DoExportNfg(const wxString &p_filename);
  void DoSetTitle(const wxString &p_title, const wxString &p_comment);
  void DoNewPlayer();
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
  void DoRemoveOutcome(GameNode p_node);
  void DoCopyOutcome(GameNode p_node, GameOutcome p_outcome);
  void DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value);

  void DoAddOutput(gbtAnalysisOutput &p_list, const wxString &p_output);
};

class gbtGameView {
protected:
  gbtGameDocument *m_doc;

public:
  explicit gbtGameView(gbtGameDocument *p_doc) : m_doc(p_doc) { m_doc->AddView(this); }
  virtual ~gbtGameView() { m_doc->RemoveView(this); }

  virtual void OnUpdate() = 0;

  /// Post any pending changes in the viewer to the document
  virtual void PostPendingChanges() {}

  gbtGameDocument *GetDocument() const { return m_doc; }
};

#endif // GAMEDOC_H
