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

class EfgShow;
class NfgShow;

// Forward declaration; class declared at bottom
class gbtGameView;

class gbtGameDocument {
friend class gbtGameView;
private:
  gBlock<gbtGameView *> m_views;

  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);
  void UpdateViews(gbtGameView *, bool, bool);

  int m_curProfile;
  gList<BehavSolution> m_behavProfiles;
  gList<MixedSolution> m_mixedProfiles;

public:
  gbtEfgGame *m_efg;
  EfgShow *m_efgShow;

  EFSupport *m_curEfgSupport;
  gList<EFSupport *> m_efgSupports;

  gbtEfgNode m_cursor, m_copyNode, m_cutNode;

  gbtNfgGame *m_nfg;
  NfgShow *m_nfgShow;
  gbtNfgSupport *m_curNfgSupport;
  gList<gbtNfgSupport *> m_nfgSupports;

  gText m_fileName;

  gbtGameDocument(gbtEfgGame);
  gbtGameDocument(gbtNfgGame);
  ~gbtGameDocument();

  gbtEfgGame GetEfg(void) const { return *m_efg; }

  // MARKED NODES
  gbtEfgNode GetCursor(void) const { return m_cursor; }
  gbtEfgNode GetCopyNode(void) const { return m_copyNode; }
  gbtEfgNode GetCutNode(void) const { return m_cutNode; }
  
  // OUTCOMES
  gText UniqueEfgOutcomeName(void) const;
 
  // SUPPORTS
  gText UniqueEfgSupportName(void) const;
  void AddSupport(EFSupport *);
  EFSupport *GetEfgSupport(void) const { return m_curEfgSupport; }
  const gList<EFSupport *> &AllEfgSupports(void) const
    { return m_efgSupports; }

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
  gbtNfgGame GetNfg(void) const { return *m_nfg; }

  gArray<int> GetContingency(void) const;

  gbtNfgSupport *GetNfgSupport(void) const { return m_curNfgSupport; }
  const gList<gbtNfgSupport *> &AllNfgSupports(void) const
    { return m_nfgSupports; }


  // DISPLAY CONFIGURATION
  int NumDecimals(void) const { return 2; }
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

#endif  // GAMEDOC_H
