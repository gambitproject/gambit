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

#include "game/efg.h"
#include "nash/behavsol.h"
#include "game/nfg.h"
#include "nash/mixedsol.h"
#include "prefs.h"

// Forward declarations; classes declared at bottom
class gbtGameView;
class gbtGameCommand;

class gbtGameDocument {
friend class gbtGameView;
private:
  wxString m_filename;
  bool m_modified;

  // Management of views
  gBlock<gbtGameView *> m_views;
  bool m_showNfg, m_showOutcomes, m_showProfiles, m_showNfgSupports;

  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);

  // Extensive-form related state information
  gbtEfgGame *m_efg;
  gbtEfgNode m_cursor, m_copyNode, m_cutNode;
  EFSupport *m_curEfgSupport;
  gList<EFSupport *> m_efgSupports;

  // Normal-form related state information
  gbtNfgGame *m_nfg;
  int m_rowPlayer, m_colPlayer;
  gArray<int> m_contingency;
  gbtNfgSupport *m_curNfgSupport;
  gList<gbtNfgSupport *> m_nfgSupports;

  // The list of computed profiles (refactor into own class?)
  int m_curProfile;
  gList<BehavSolution> m_behavProfiles;
  gList<MixedSolution> m_mixedProfiles;

  // Preferences for colors, layout, and so forth
  gbtPreferences m_prefs;

public:
  // Lifecycle
  gbtGameDocument(gbtEfgGame, wxString = "");
  gbtGameDocument(gbtNfgGame, wxString = "");
  ~gbtGameDocument();

  // General information about game
  const wxString &GetFilename(void) const { return m_filename; }
  void SetFilename(const wxString &p_filename) { m_filename = p_filename; }

  bool IsModified(void) const { return m_modified; }
  void SetIsModified(bool p_modified) { m_modified = p_modified; }

  // Management of commands
  void Submit(gbtGameCommand *);
  void UpdateViews(gbtGameView *, bool, bool);
  void OnTreeChanged(bool p_nodesChanged, bool p_infosetsChanged);

  // Extensive-form related state information
  gbtEfgGame GetEfg(void) const { return *m_efg; }
  bool HasEfg(void) const { return (m_efg != 0); }

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

  // MARKED NODES
  void SetCursor(gbtEfgNode m_node);
  gbtEfgNode GetCursor(void) const { return m_cursor; }
  void SetCopyNode(gbtEfgNode);
  gbtEfgNode GetCopyNode(void) const { return m_copyNode; }
  void SetCutNode(gbtEfgNode);
  gbtEfgNode GetCutNode(void) const { return m_cutNode; }
  
  // OUTCOMES
  gText UniqueEfgOutcomeName(void) const;
  gText UniqueNfgOutcomeName(void) const;
   
  // SUPPORTS
  gText UniqueEfgSupportName(void) const;
  gText UniqueNfgSupportName(void) const;

  const EFSupport &GetEfgSupport(void) const { return *m_curEfgSupport; }
  int GetEfgSupportIndex(void) const
    { return m_efgSupports.Find(m_curEfgSupport); }
  const gList<EFSupport *> &AllEfgSupports(void) const
    { return m_efgSupports; }
  void SetEfgSupport(int p_index);
  void AddEfgSupport(EFSupport *);
  void DeleteEfgSupport(void);
  void AddAction(gbtEfgAction);
  void RemoveAction(gbtEfgAction);

  const gbtNfgSupport &GetNfgSupport(void) const { return *m_curNfgSupport; }
  int GetNfgSupportIndex(void) const
    { return m_nfgSupports.Find(m_curNfgSupport); }
  const gList<gbtNfgSupport *> &AllNfgSupports(void) const
    { return m_nfgSupports; }
  void SetNfgSupport(int p_index);
  void AddNfgSupport(gbtNfgSupport *);
  void DeleteNfgSupport(void);
  void AddStrategy(gbtNfgStrategy);
  void RemoveStrategy(gbtNfgStrategy);

  // PROFILES
  gText UniqueBehavProfileName(void) const;
  gText UniqueMixedProfileName(void) const;

  void AddProfile(const BehavSolution &);
  void AddProfile(const MixedSolution &);
  const gList<BehavSolution> &AllBehavProfiles(void) const
    { return m_behavProfiles; }
  const BehavSolution &GetBehavProfile(void) const
    { return m_behavProfiles[m_curProfile]; }

  const gList<MixedSolution> &AllMixedProfiles(void) const
    { return m_mixedProfiles; }
  const MixedSolution &GetMixedProfile(void) const
    { return m_mixedProfiles[m_curProfile]; }

  bool IsProfileSelected(void) const { return (m_curProfile > 0); }
  void SetCurrentProfile(int p_index);
  void SetCurrentProfile(const BehavSolution &);
  void SetCurrentProfile(const MixedSolution &);
  void RemoveProfile(int p_index);

  // LABELS
  gText GetRealizProb(const gbtEfgNode &) const;
  gText GetBeliefProb(const gbtEfgNode &) const;
  gText GetNodeValue(const gbtEfgNode &) const;
  gText GetInfosetProb(const gbtEfgNode &) const;
  gText GetInfosetValue(const gbtEfgNode &) const;
  gText GetActionValue(const gbtEfgNode &, int act) const;
  gText GetActionProb(const gbtEfgNode &, int act) const;
  gNumber ActionProb(const gbtEfgNode &, int br) const;

  // NORMAL FORM STATE
  void MakeReducedNfg(void);
  gbtNfgGame GetNfg(void) const;

  gArray<int> GetContingency(void) const;
  void SetContingency(const gArray<int> &);
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
};

#endif  // GAMEDOC_H
