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

class gbtGameDocument {
public:
  gbtEfgGame *m_efg;
  EfgShow *m_efgShow;
  EFSupport *m_curEfgSupport;
  gList<EFSupport *> m_efgSupports;
  int m_curBehavProfile;
  gList<BehavSolution> m_behavProfiles;
  gbtEfgNode m_cursor, m_copyNode, m_cutNode;

  gbtNfgGame *m_nfg;
  NfgShow *m_nfgShow;
  gbtNfgSupport *m_curNfgSupport;
  gList<gbtNfgSupport *> m_nfgSupports;
  int m_curMixedProfile;
  gList<MixedSolution> m_mixedProfiles;

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
  const gList<BehavSolution> &AllBehavProfiles(void) const
    { return m_behavProfiles; }
  const BehavSolution &GetBehavProfile(void) const
    { return m_behavProfiles[m_curBehavProfile]; }

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
  gArray<int> GetContingency(void) const;

  // DISPLAY CONFIGURATION
  int NumDecimals(void) const { return 2; }
};

#endif  // GAMEDOC_H
