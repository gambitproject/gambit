//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of game document class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "game/game.h"
#include "nash/behavsol.h"
#include "nash/mixedsol.h"
#include "prefs.h"
#include "support.h"

// Forward declarations; classes declared at bottom
class gbtGameView;
class gbtGameCommand;

class gbtGameDocument {
friend class gbtGameView;
private:
  wxString m_filename;
  bool m_modified;

  // Management of views
  gbtBlock<gbtGameView *> m_views;
  bool m_showOutcomes, m_showProfiles;
  bool m_showEfgSupports, m_showEfgNavigate;
  bool m_showNfg, m_showNfgSupports;
  
  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);

  // Extensive-form related state information
  gbtGame m_game;
  gbtGameNode m_cursor, m_copyNode, m_cutNode;
  gbtEfgSupportList m_efgSupports;

  // Normal-form related state information
  int m_rowPlayer, m_colPlayer;
  gbtArray<int> m_contingency;
  gbtNfgSupportList m_nfgSupports;

  // The list of computed profiles (refactor into own class?)
  int m_curProfile;
  gbtList<BehavSolution> m_behavProfiles;
  gbtList<MixedSolution> m_mixedProfiles;

  // Preferences for colors, layout, and so forth
  gbtPreferences m_prefs;

public:
  // Lifecycle
  gbtGameDocument(gbtGame, wxString = wxT(""));
  ~gbtGameDocument();

  // General information about game
  const wxString &GetFilename(void) const { return m_filename; }
  void SetFilename(const wxString &p_filename) { m_filename = p_filename; }

  bool IsModified(void) const { return m_modified; }
  void SetIsModified(bool p_modified) { m_modified = p_modified; }

  // Management of commands
  void Submit(gbtGameCommand *);
  void UpdateViews(gbtGameView * = 0);

  gbtGame GetGame(void) const { return m_game; }
  bool HasEfg(void) const { return m_game->IsTree(); }

  // Extensive-form related state information

  gbtPreferences &GetPreferences(void) { return m_prefs; }
  const gbtPreferences &GetPreferences(void) const { return m_prefs; }

  bool ShowNfg(void) const { return m_showNfg; }
  void SetShowNfg(bool p_show);

  bool ShowOutcomes(void) const { return m_showOutcomes; }
  void SetShowOutcomes(bool p_show);

  bool ShowProfiles(void) const { return m_showProfiles; }
  void SetShowProfiles(bool p_show);

  bool ShowNfgSupports(void) const { return m_showNfgSupports; }
  void SetShowNfgSupports(bool p_show);

  bool ShowEfgNavigate(void) const { return m_showEfgNavigate; }
  void SetShowEfgNavigate(bool p_show);

  bool ShowEfgSupports(void) const { return m_showEfgSupports; }
  void SetShowEfgSupports(bool p_show);

  // MARKED NODES
  void SetCursor(gbtGameNode m_node);
  gbtGameNode GetCursor(void) const { return m_cursor; }
  void SetCopyNode(gbtGameNode);
  gbtGameNode GetCopyNode(void) const { return m_copyNode; }
  void SetCutNode(gbtGameNode);
  gbtGameNode GetCutNode(void) const { return m_cutNode; }
  
  // OUTCOMES
  gbtText UniqueOutcomeName(void) const;
   
  // SUPPORTS
  const gbtEfgSupportList &GetEfgSupportList(void) const
    { return m_efgSupports; }
  gbtEfgSupportList &GetEfgSupportList(void) { return m_efgSupports; }

  const gbtNfgSupportList &GetNfgSupportList(void) const
    { return m_nfgSupports; }
  gbtNfgSupportList &GetNfgSupportList(void) { return m_nfgSupports; }

  // PROFILES
  gbtText UniqueBehavProfileName(void) const;
  gbtText UniqueMixedProfileName(void) const;

  void AddProfile(const BehavSolution &);
  void AddProfile(const MixedSolution &);
  const gbtList<BehavSolution> &AllBehavProfiles(void) const
    { return m_behavProfiles; }
  const BehavSolution &GetBehavProfile(void) const
    { return m_behavProfiles[m_curProfile]; }

  const gbtList<MixedSolution> &AllMixedProfiles(void) const
    { return m_mixedProfiles; }
  const MixedSolution &GetMixedProfile(void) const
    { return m_mixedProfiles[m_curProfile]; }

  bool IsProfileSelected(void) const { return (m_curProfile > 0); }
  void SetCurrentProfile(int p_index);
  void SetCurrentProfile(const BehavSolution &);
  void SetCurrentProfile(const MixedSolution &);
  void RemoveProfile(int p_index);

  // LABELS
  gbtText GetRealizProb(const gbtGameNode &) const;
  gbtText GetBeliefProb(const gbtGameNode &) const;
  gbtText GetNodeValue(const gbtGameNode &) const;
  gbtText GetInfosetProb(const gbtGameNode &) const;
  gbtText GetInfosetValue(const gbtGameNode &) const;
  gbtText GetActionValue(const gbtGameNode &, int act) const;
  gbtText GetActionProb(const gbtGameNode &, int act) const;
  gbtNumber ActionProb(const gbtGameNode &, int br) const;

  // NORMAL FORM STATE
  gbtArray<int> GetContingency(void) const;
  void SetContingency(const gbtArray<int> &);
  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }
  void SetRowPlayer(int p_player);
  void SetColPlayer(int p_player);

};

//
// Base class for windows or other objects that implement a "view"
// onto a document.  Override the OnUpdate member to update the
// view (called after any change to the document).  Views can be on
// the extensive form and/or the normal form; override IsEfgView()
// and IsNfgView(), respectively, to indicate this.
//
class gbtGameView {
protected:
  gbtGameDocument *m_doc;

public:
  gbtGameView(gbtGameDocument *p_doc);
  virtual ~gbtGameView();

  virtual void OnUpdate(gbtGameView *p_sender);
  virtual bool IsEfgView(void) const = 0;
  virtual bool IsNfgView(void) const = 0;
};

//
// Model for a command that changes the state of the game document.
// Override the Do() member to update the document; this function is
// required to ensure the state of the document remains consistent
// upon its completion.
//
class gbtGameCommand {
public:
  virtual ~gbtGameCommand() { }

  virtual void Do(gbtGameDocument *) = 0;

  // ModifiesGame(): return true if the command causes a change
  // in the game that alters the number of players, strategies, etc.
  virtual bool ModifiesGame(void) const = 0;

  // ModifiesPayoffs(): return true if the command causes a change
  // in the game that invalidates cached quantities (IsNash(), etc.)
  virtual bool ModifiesPayoffs(void) const = 0;
};

#endif  // GAMEDOC_H
