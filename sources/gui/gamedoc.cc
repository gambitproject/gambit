//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of game document class
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

#include "base/base.h"
#include "gamedoc.h"
#include "game/efg.h"
#include "game/efstrat.h"
#include "game/nfg.h"

// FIXME: It would be nice not to have these dependencies...
// once all data manipulation is moved into this class, they should
// be removable.
#include "wx/wx.h"
#include "efgshow.h"
#include "nfgshow.h"

gbtGameDocument::gbtGameDocument(gbtEfgGame p_efg)
  : m_curProfile(0),
    m_efg(new gbtEfgGame(p_efg)), m_efgShow(0),
    m_curEfgSupport(0),
    m_cursor(0), m_copyNode(0), m_cutNode(0),
    m_nfg(0), m_nfgShow(0),
    m_curNfgSupport(0) 
{ }

gbtGameDocument::gbtGameDocument(gbtNfgGame p_nfg)
  : m_curProfile(0),
    m_efg(0), m_efgShow(0),
    m_curEfgSupport(0), 
    m_cursor(0), m_copyNode(0), m_cutNode(0),
    m_nfg(new gbtNfgGame(p_nfg)), m_nfgShow(0),
    m_curNfgSupport(0) 
{ }

gbtGameDocument::~gbtGameDocument()
{
  if (m_efg) {
    for (int i = 1; i <= m_efgSupports.Length(); delete m_efgSupports[i++]);
    delete m_efg;
  }
  if (m_nfg) {
    delete m_nfg;
  }
}

void gbtGameDocument::OnTreeChanged(bool p_nodesChanged,
				    bool p_infosetsChanged)
{
  if (p_infosetsChanged) {
    while (m_efgSupports.Length()) { 
      delete m_efgSupports.Remove(1);
    }

    m_curEfgSupport = new EFSupport(*m_efg);
    m_efgSupports.Append(m_curEfgSupport);
    m_curEfgSupport->SetName("Full Support");
  }

  if (p_infosetsChanged || p_nodesChanged) {
    // It would be nice to relax this, but be conservative for now
    m_copyNode = 0;
    m_cutNode = 0;
  }
}


//==========================================================================
//                 gbtGameDocument: Operations on outcomes
//==========================================================================

gText gbtGameDocument::UniqueEfgOutcomeName(void) const
{
  int number = m_efg->NumOutcomes() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_efg->NumOutcomes(); i++) {
      if (m_efg->GetOutcome(i).GetLabel() == "Outcome" + ToText(number)) {
	break;
      }
    }

    if (i > m_efg->NumOutcomes()) {
      return "Outcome" + ToText(number);
    }
    
    number++;
  }
}

//==========================================================================
//                 gbtGameDocument: Operations on supports
//==========================================================================

gText gbtGameDocument::UniqueEfgSupportName(void) const
{
  int number = m_efgSupports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_efgSupports.Length(); i++) {
      if (m_efgSupports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_efgSupports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

void gbtGameDocument::AddSupport(EFSupport *p_support)
{
  m_efgSupports.Append(p_support);
}


//==========================================================================
//                gbtGameDocument: Operations on profiles
//==========================================================================

gText gbtGameDocument::UniqueBehavProfileName(void) const
{
  int number = m_behavProfiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_behavProfiles.Length(); i++) {
      if (m_behavProfiles[i].GetName() == "Profile" + ToText(number)) {
	break;
      }
    }

    if (i > m_behavProfiles.Length())
      return "Profile" + ToText(number);
    
    number++;
  }
}

void gbtGameDocument::AddProfile(const BehavSolution &p_profile)
{
  if (p_profile.GetName() == "") {
    BehavSolution tmp(p_profile);
    tmp.SetName(UniqueBehavProfileName());
    m_behavProfiles.Append(tmp);
  }
  else {
    m_behavProfiles.Append(p_profile);
  }

  if (m_nfg) {
    MixedSolution mixed(MixedProfile<gNumber>(*p_profile.Profile()),
			p_profile.Creator());
    m_mixedProfiles.Append(mixed);
    m_nfgShow->OnProfilesEdited();
  }

  UpdateViews(0, true, true);
}

void gbtGameDocument::SetCurrentProfile(int p_index)
{
  m_curProfile = p_index;
  UpdateViews(0, true, true);
}

void gbtGameDocument::SetCurrentProfile(const BehavSolution &p_profile)
{
  m_behavProfiles[m_curProfile] = p_profile;
  UpdateViews(0, true, true);
}

void gbtGameDocument::SetCurrentProfile(const MixedSolution &p_profile)
{
  m_mixedProfiles[m_curProfile] = p_profile;
  UpdateViews(0, true, true);
}

void gbtGameDocument::RemoveProfile(int p_index)
{
  if (m_behavProfiles.Length() >= p_index) {
    m_behavProfiles.Remove(p_index);
  }
  if (m_mixedProfiles.Length() >= p_index) {
    m_mixedProfiles.Remove(p_index);
  }

  if (m_curProfile == p_index) {
    m_curProfile = 0;
  }

  UpdateViews(0, true, true);
}

//==========================================================================
//                     gbtGameDocument: Labels
//==========================================================================

gText gbtGameDocument::GetRealizProb(const gbtEfgNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].RealizProb(p_node),
		NumDecimals());
}

gText gbtGameDocument::GetBeliefProb(const gbtEfgNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node.GetPlayer().IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].BeliefProb(p_node),
		NumDecimals());
}

gText gbtGameDocument::GetNodeValue(const gbtEfgNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull()) {
    return "";
  }
  gText tmp = "(";
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    tmp += ToText(m_behavProfiles[m_curProfile].NodeValue(p_node)[pl], 
		  NumDecimals());
    if (pl < m_efg->NumPlayers()) {
      tmp += ",";
    }
    else {
      tmp += ")";
    }
  }
  return tmp;
}

gText gbtGameDocument::GetInfosetProb(const gbtEfgNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node.GetPlayer().IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].IsetProb(p_node.GetInfoset()),
		NumDecimals());
}

gText gbtGameDocument::GetInfosetValue(const gbtEfgNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node.GetPlayer().IsNull() || p_node.GetPlayer().IsChance()) {
    return "";
  }
  if (GetBehavProfile().IsetProb(p_node.GetInfoset()) > gNumber(0)) {
    return ToText(GetBehavProfile().IsetValue(p_node.GetInfoset()),
		  NumDecimals());
  }
  else {
    // this is due to a bug in the value computation
    return "";
  }
}

gText gbtGameDocument::GetActionProb(const gbtEfgNode &p_node, int p_act) const
{
  if (!p_node.GetPlayer().IsNull() && p_node.GetPlayer().IsChance()) {
    return ToText(p_node.GetInfoset().GetChanceProb(p_act),
		  NumDecimals());
  }

  if (m_curProfile == 0 || p_node.GetPlayer().IsNull()) {
    return "";
  }

  return ToText(GetBehavProfile().ActionProb(p_node.GetInfoset().GetAction(p_act)),
		NumDecimals());
}

gText gbtGameDocument::GetActionValue(const gbtEfgNode &p_node, int p_act) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node.GetPlayer().IsNull() || p_node.GetPlayer().IsChance()) {
    return "";
  }

  if (GetBehavProfile().IsetProb(p_node.GetInfoset()) > gNumber(0)) {
    return ToText(GetBehavProfile().ActionValue(p_node.GetInfoset().GetAction(p_act)),
		  NumDecimals());
  }
  else  {
    // this is due to a bug in the value computation
    return "";
  }
}

gNumber gbtGameDocument::ActionProb(const gbtEfgNode &p_node, int p_action) const
{
  if (!p_node.GetPlayer().IsNull() && p_node.GetPlayer().IsChance()) {
    return p_node.GetInfoset().GetChanceProb(p_action);
  }

  if (m_curProfile && !p_node.GetInfoset().IsNull()) {
    return m_behavProfiles[m_curProfile](p_node.GetInfoset().GetAction(p_action));
  }
  return -1;
}


//==========================================================================
//               gbtGameDocument: Operations on normal form
//==========================================================================

void gbtGameDocument::MakeReducedNfg(void)
{
  m_nfg = new gbtNfgGame(::MakeReducedNfg(*m_curEfgSupport));
  m_nfgShow = new NfgShow(this, 0);
  m_nfgShow->SetFilename("");

  m_mixedProfiles.Flush();
  for (int i = 1; i <= m_behavProfiles.Length(); i++) {
    BehavProfile<gNumber> profile(*m_behavProfiles[i].Profile());
    MixedProfile<gNumber> mixed(profile);
    AddProfile(MixedSolution(mixed, m_behavProfiles[i].Creator()));
  }
  if (m_behavProfiles.Length() > 0) {
    m_nfgShow->OnChangeProfile();
  }
}

gText gbtGameDocument::UniqueMixedProfileName(void) const
{
  int number = m_mixedProfiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_mixedProfiles.Length(); i++) {
      if (m_mixedProfiles[i].GetName() == "Profile" + ToText(number)) {
	break;
      }
    }

    if (i > m_mixedProfiles.Length()) {
      return "Profile" + ToText(number);
    }
    
    number++;
  }
}

void gbtGameDocument::AddProfile(const MixedSolution &p_profile)
{
  if (p_profile.GetName() == "") {
    MixedSolution tmp(p_profile);
    tmp.SetName(UniqueMixedProfileName());
    m_mixedProfiles.Append(tmp);
  }
  else {
    m_mixedProfiles.Append(p_profile);
  }

  if (m_efg) {
    m_behavProfiles.Append(BehavProfile<gNumber>(*p_profile.Profile()));
  }

  UpdateViews(0, true, true);
}

gArray<int> gbtGameDocument::GetContingency(void) const
{
  return m_nfgShow->GetContingency();
}


//==========================================================================
//                 gbtGameDocument: Management of views
//==========================================================================

void gbtGameDocument::AddView(gbtGameView *p_view)
{
  m_views.Append(p_view);
  UpdateViews(0, true, true);
}

void gbtGameDocument::RemoveView(gbtGameView *p_view)
{
  m_views.Remove(m_views.Find(p_view));
}

void gbtGameDocument::UpdateViews(gbtGameView *p_sender,
				  bool p_efgViews, bool p_nfgViews)
{
  for (int i = 1; i <= m_views.Length(); i++) {
    if ((m_views[i]->IsEfgView() && p_efgViews) ||
	(m_views[i]->IsNfgView() && p_nfgViews)) {
      m_views[i]->OnUpdate(p_sender);
    }
  }

}

//==========================================================================
//                 class gbtGameView: Member functions
//==========================================================================

gbtGameView::gbtGameView(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{
  
}

gbtGameView::~gbtGameView()
{ }

void gbtGameView::OnUpdate(gbtGameView *)
{ }



#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<gbtGameView *>;
template class gBlock<gbtGameView *>;
