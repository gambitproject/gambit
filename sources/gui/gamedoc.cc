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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "base/base.h"
#include "game/game.h"
#include "game/efgsupport.h"
#include "gamedoc.h"
#include "gambit.h"

gbtGameDocument::gbtGameDocument(gbtGame p_game, wxString p_filename)
  : m_filename(p_filename), m_modified(false),
    m_showOutcomes(false), m_showProfiles(false),
    m_showEfgSupports(false), m_showEfgNavigate(false),
    m_showNfg(false), m_showNfgSupports(false),
    m_game(p_game), 
    m_cursor(0), m_copyNode(0), m_cutNode(0),
    m_efgSupports(this),
    m_rowPlayer(1), m_colPlayer(2),
    m_contingency(p_game->NumPlayers()),
    m_nfgSupports(this),
    m_curProfile(0)
{
  for (int pl = 1; pl <= m_game->NumPlayers(); m_contingency[pl++] = 1);
}

gbtGameDocument::~gbtGameDocument()
{ }

void gbtGameDocument::SetCursor(gbtGameNode p_node)
{
  m_cursor = p_node;
  UpdateViews();
}

void gbtGameDocument::SetCopyNode(gbtGameNode p_node)
{
  m_copyNode = p_node;
  m_cutNode = 0;
  UpdateViews();
}

void gbtGameDocument::SetCutNode(gbtGameNode p_node)
{
  m_cutNode = 0;
  m_copyNode = p_node;
  UpdateViews();
}

//==========================================================================
//                 gbtGameDocument: Operations on outcomes
//==========================================================================

gbtText gbtGameDocument::UniqueOutcomeName(void) const
{
  int number = m_game->NumOutcomes() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_game->NumOutcomes(); i++) {
      if (m_game->GetOutcome(i)->GetLabel() == "Outcome" + ToText(number)) {
	break;
      }
    }

    if (i > m_game->NumOutcomes()) {
      return "Outcome" + ToText(number);
    }
    
    number++;
  }
}

//==========================================================================
//                gbtGameDocument: Operations on profiles
//==========================================================================

gbtText gbtGameDocument::UniqueBehavProfileName(void) const
{
  int number = m_behavProfiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_behavProfiles.Length(); i++) {
      if (m_behavProfiles[i].GetLabel() == "Profile" + ToText(number)) {
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
  if (p_profile.GetLabel() == "") {
    BehavSolution tmp(p_profile);
    tmp.SetLabel(UniqueBehavProfileName());
    m_behavProfiles.Append(tmp);
  }
  else {
    m_behavProfiles.Append(p_profile);
  }

  MixedSolution mixed(gbtMixedProfile<gbtNumber>(*p_profile.Profile()),
		      p_profile.GetCreator());
  m_mixedProfiles.Append(mixed);

  UpdateViews();
}

void gbtGameDocument::SetCurrentProfile(int p_index)
{
  m_curProfile = p_index;
  UpdateViews();
}

void gbtGameDocument::SetCurrentProfile(const BehavSolution &p_profile)
{
  m_behavProfiles[m_curProfile] = p_profile;
  UpdateViews();
}

void gbtGameDocument::SetCurrentProfile(const MixedSolution &p_profile)
{
  m_mixedProfiles[m_curProfile] = p_profile;
  UpdateViews();
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

  UpdateViews();
}

//==========================================================================
//                     gbtGameDocument: Labels
//==========================================================================

gbtText gbtGameDocument::GetRealizProb(const gbtGameNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].GetRealizProb(p_node),
		m_prefs.NumDecimals());
}

gbtText gbtGameDocument::GetBeliefProb(const gbtGameNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node->GetPlayer().IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].GetBelief(p_node),
		m_prefs.NumDecimals());
}

gbtText gbtGameDocument::GetNodeValue(const gbtGameNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull()) {
    return "";
  }
  gbtText tmp = "(";
  for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
    tmp += ToText(m_behavProfiles[m_curProfile].NodeValue(p_node)[pl], 
		  m_prefs.NumDecimals());
    if (pl < m_game->NumPlayers()) {
      tmp += ",";
    }
    else {
      tmp += ")";
    }
  }
  return tmp;
}

gbtText gbtGameDocument::GetInfosetProb(const gbtGameNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node->GetPlayer().IsNull()) {
    return "";
  }
  return ToText(m_behavProfiles[m_curProfile].GetInfosetProb(p_node->GetInfoset()),
		m_prefs.NumDecimals());
}

gbtText gbtGameDocument::GetInfosetValue(const gbtGameNode &p_node) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node->GetPlayer().IsNull() || p_node->GetPlayer()->IsChance()) {
    return "";
  }
  if (GetBehavProfile().GetInfosetProb(p_node->GetInfoset()) > gbtNumber(0)) {
    return ToText(GetBehavProfile().GetInfosetValue(p_node->GetInfoset()),
		  m_prefs.NumDecimals());
  }
  else {
    // this is due to a bug in the value computation
    return "";
  }
}

gbtText gbtGameDocument::GetActionProb(const gbtGameNode &p_node, int p_act) const
{
  if (!p_node->GetPlayer().IsNull() && p_node->GetPlayer()->IsChance()) {
    return ToText(p_node->GetInfoset()->GetChanceProb(p_act),
		  m_prefs.NumDecimals());
  }

  if (m_curProfile == 0 || p_node->GetPlayer().IsNull()) {
    return "";
  }

  return ToText(GetBehavProfile().GetActionProb(p_node->GetInfoset()->GetAction(p_act)),
		m_prefs.NumDecimals());
}

gbtText gbtGameDocument::GetActionValue(const gbtGameNode &p_node, int p_act) const
{
  if (m_curProfile == 0 || p_node.IsNull() ||
      p_node->GetPlayer().IsNull() || p_node->GetPlayer()->IsChance()) {
    return "";
  }

  if (GetBehavProfile().GetInfosetProb(p_node->GetInfoset()) > gbtNumber(0)) {
    return ToText(GetBehavProfile().GetActionValue(p_node->GetInfoset()->GetAction(p_act)),
		  m_prefs.NumDecimals());
  }
  else  {
    // this is due to a bug in the value computation
    return "";
  }
}

gbtNumber gbtGameDocument::ActionProb(const gbtGameNode &p_node, int p_action) const
{
  if (!p_node->GetPlayer().IsNull() && p_node->GetPlayer()->IsChance()) {
    return p_node->GetInfoset()->GetChanceProb(p_action);
  }

  if (m_curProfile && !p_node->GetInfoset().IsNull()) {
    return m_behavProfiles[m_curProfile](p_node->GetInfoset()->GetAction(p_action));
  }
  return -1;
}


//==========================================================================
//               gbtGameDocument: Operations on normal form
//==========================================================================

gbtText gbtGameDocument::UniqueMixedProfileName(void) const
{
  int number = m_mixedProfiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_mixedProfiles.Length(); i++) {
      if (m_mixedProfiles[i].GetLabel() == "Profile" + ToText(number)) {
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
  if (p_profile.GetLabel() == "") {
    MixedSolution tmp(p_profile);
    tmp.SetLabel(UniqueMixedProfileName());
    m_mixedProfiles.Append(tmp);
  }
  else {
    m_mixedProfiles.Append(p_profile);
  }

  if (HasEfg()) {
    m_behavProfiles.Append(gbtBehavProfile<gbtNumber>(*p_profile.Profile()));
  }

  UpdateViews();
}

gbtArray<int> gbtGameDocument::GetContingency(void) const
{
  return m_contingency;
}

void gbtGameDocument::SetContingency(const gbtArray<int> &p_contingency)
{
  m_contingency = p_contingency;
  UpdateViews();
}

void gbtGameDocument::SetRowPlayer(int p_player)
{
  if (m_colPlayer == p_player) {
    m_colPlayer = m_rowPlayer;
  }
  m_rowPlayer = p_player;
  UpdateViews();
}

void gbtGameDocument::SetColPlayer(int p_player)
{
  if (m_rowPlayer == p_player) {
    m_rowPlayer = m_colPlayer;
  }
  m_colPlayer = p_player;
  UpdateViews();
}

//==========================================================================
//                 gbtGameDocument: Management of views
//==========================================================================

void gbtGameDocument::AddView(gbtGameView *p_view)
{
  m_views.Append(p_view);
}

void gbtGameDocument::RemoveView(gbtGameView *p_view)
{
  m_views.Remove(m_views.Find(p_view));
}

void gbtGameDocument::UpdateViews(gbtGameView *p_sender /* =0 */)
{
  for (int i = 1; i <= m_views.Length(); i++) {
    m_views[i]->OnUpdate(p_sender);
  }
}

void gbtGameDocument::SetShowNfg(bool p_show)
{
  m_showNfg = p_show;
  UpdateViews();
}

void gbtGameDocument::SetShowOutcomes(bool p_show)
{
  m_showOutcomes = p_show;
  UpdateViews();
}

void gbtGameDocument::SetShowProfiles(bool p_show)
{
  m_showProfiles = p_show;
  UpdateViews();
}

void gbtGameDocument::SetShowNfgSupports(bool p_show)
{
  m_showNfgSupports = p_show;
  UpdateViews();
}

void gbtGameDocument::SetShowEfgNavigate(bool p_show)
{
  m_showEfgNavigate = p_show;
  UpdateViews();
}

void gbtGameDocument::SetShowEfgSupports(bool p_show)
{
  m_showEfgSupports = p_show;
  UpdateViews();
}

void gbtGameDocument::Submit(gbtGameCommand *p_command)
{
  try {
    p_command->Do(this);
  }
  catch (...) {
    guiExceptionDialog("", wxGetApp().GetTopWindow());
  }

  if (p_command->ModifiesGame()) {
    if (HasEfg()) {
      m_efgSupports.Flush();
      
      // Reset the list of computed profiles
      while (m_behavProfiles.Length()) {
	m_behavProfiles.Remove(1);
      }
    }

    m_nfgSupports.Flush();

    // Reset the list of computed profiles
    while (m_mixedProfiles.Length()) {
      m_mixedProfiles.Remove(1);
    }

    m_curProfile = 0;

    // Make sure the contingency points to a non-bogus profile
    m_contingency = gbtArray<int>(GetGame()->NumPlayers());
    for (int pl = 1; pl <= GetGame()->NumPlayers(); m_contingency[pl++] = 1);

    m_modified = true;
  }
  
  if (p_command->ModifiesPayoffs()) {
    for (int i = 1; i <= m_behavProfiles.Length(); i++) {
      m_behavProfiles[i].Invalidate();
    }
    for (int i = 1; i <= m_mixedProfiles.Length(); i++) {
      m_mixedProfiles[i].Invalidate();
    }
    m_modified = true;
  }
  
  UpdateViews();

  // Someday, we might save the command for undo/redo; for now, 
  // just delete it.
  delete p_command;
}

//==========================================================================
//                 class gbtGameView: Member functions
//==========================================================================

gbtGameView::gbtGameView(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{
  m_doc->AddView(this);
}

gbtGameView::~gbtGameView()
{
  m_doc->RemoveView(this);
}

void gbtGameView::OnUpdate(gbtGameView *)
{ }



#include "base/garray.imp"
#include "base/gblock.imp"

template class gbtArray<gbtGameView *>;
template class gbtBlock<gbtGameView *>;

#include "base/glist.imp"
template class gbtList<gbtEfgSupport *>;
template class gbtList<gbtNfgSupport *>;
