//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of game document/view classes
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

#include "gamedoc.h"

//========================================================================
//                       class gbtGameDocument
//========================================================================

gbtGameDocument::gbtGameDocument(efgGame *p_efg)
  : m_efg(p_efg), m_nfg(0),
    m_currentProfile(0), m_currentEfgSupport(0), m_currentNfgSupport(0),
    m_cursor(0), m_cutNode(0), m_copyNode(0),
    m_numDecimals(2)
{ }

gbtGameDocument::gbtGameDocument(Nfg *p_nfg)
  : m_efg(0), m_nfg(p_nfg),
    m_currentProfile(0), m_currentEfgSupport(0), m_currentNfgSupport(0),
    m_cursor(0), m_cutNode(0), m_copyNode(0),
    m_numDecimals(2)
{ }

void gbtGameDocument::AddView(gbtGameView *p_view)
{
  m_views.Append(p_view);
}

void gbtGameDocument::RemoveView(gbtGameView *p_view)
{
  if (m_views.Find(p_view)) { 
    m_views.Remove(m_views.Find(p_view));
  }
}

void gbtGameDocument::UpdateAllViews(gbtGameView *p_sender /* = 0 */)
{
  for (int i = 1; i <= m_views.Length(); i++) {
    if (m_views[i] != p_sender) {
      m_views[i]->OnUpdate(p_sender);
    }
  }
}

void gbtGameDocument::Submit(gbtGameCommand *p_command)
{
  p_command->Do();
  delete p_command;
}

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

    if (i > m_efgSupports.Length()) {
      return "Support" + ToText(number);
    }
    
    number++;
  }
}

void gbtGameDocument::FlushEfgSupports(void)
{
  while (m_efgSupports.Length()) {
    delete m_efgSupports.Remove(1);
  }

  m_currentEfgSupport = new EFSupport(*m_efg);
  m_efgSupports.Append(m_currentEfgSupport);
  m_currentEfgSupport->SetName("Full Support");
}

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

void gbtGameDocument::SelectNode(Node *p_node)
{
  m_cursor = p_node;
  UpdateAllViews();
}

gText gbtGameDocument::GetRealizProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node) {
    return "";
  }
  return ToText(m_behavProfiles[m_currentProfile].RealizProb(p_node),
		m_numDecimals);
}

gText gbtGameDocument::GetBeliefProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }
  return ToText(m_behavProfiles[m_currentProfile].BeliefProb(p_node),
		m_numDecimals);
}

gText gbtGameDocument::GetNodeValue(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node) {
    return "";
  }
  gText tmp = "(";
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    tmp += ToText(m_behavProfiles[m_currentProfile].NodeValue(p_node)[pl], 
		  m_numDecimals);
    if (pl < m_efg->NumPlayers()) {
      tmp += ",";
    }
    else {
      tmp += ")";
    }
  }
  return tmp;
}

gText gbtGameDocument::GetInfosetProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }
  return ToText(m_behavProfiles[m_currentProfile].IsetProb(p_node->GetInfoset()),
		m_numDecimals);
}

gText gbtGameDocument::GetInfosetValue(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }
  if (CurrentBehav().IsetProb(p_node->GetInfoset()) > gNumber(0)) {
    return ToText(CurrentBehav().IsetValue(p_node->GetInfoset()),
		  m_numDecimals);
  }
  else {
    // this is due to a bug in the value computation
    return "";
  }
}

gText gbtGameDocument::GetActionProb(const Node *p_node, int p_act) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return ToText(m_efg->GetChanceProb(p_node->GetInfoset(), p_act),
		  m_numDecimals);
  }

  if (m_currentProfile == 0 || !p_node->GetPlayer()) {
    return "";
  }

  return ToText(CurrentBehav().ActionProb(p_node->GetInfoset()->Actions()[p_act]),
		m_numDecimals);
}

gText gbtGameDocument::GetActionValue(const Node *p_node, int p_act) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }

  if (CurrentBehav().IsetProb(p_node->GetInfoset()) > gNumber(0)) {
    return ToText(CurrentBehav().ActionValue(p_node->GetInfoset()->Actions()[p_act]),
		  m_numDecimals);
  }
  else  {
    // this is due to a bug in the value computation
    return "";
  }
}

gNumber gbtGameDocument::ActionProb(const Node *p_node, int p_action) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return m_efg->GetChanceProb(p_node->GetInfoset(), p_action);
  }

  if (m_currentProfile && p_node->GetInfoset()) {
    return m_behavProfiles[m_currentProfile](p_node->GetInfoset()->Actions()[p_action]);
  }
  return -1;
}

//========================================================================
//                        class gbtGameView
//========================================================================

gbtGameView::gbtGameView(gbtGameDocument *p_game)
  : m_game(p_game)
{
  m_game->AddView(this);
}

gbtGameView::~gbtGameView()
{
  m_game->RemoveView(this);
}

void gbtGameView::OnUpdate(gbtGameView *)
{ }

#include "base/glist.imp"
template class gList<gbtGameView *>;






