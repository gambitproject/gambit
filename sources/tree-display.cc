//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of view of extensive form tree
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include "tree-display.h"

//--------------------------------------------------------------------------
//                          class gbtTreeLayout
//--------------------------------------------------------------------------

void gbtTreeLayout::DrawOutcome(wxDC &p_dc, const gbtGameNode &p_node) const
{
  wxPoint point(GetX(p_node) + 20, m_ycoords[p_node->GetId()]);

  p_dc.SetFont(wxFont(9, wxSWISS, wxNORMAL, wxBOLD));

  gbtGameOutcome outcome = p_node->GetOutcome();
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    gbtGamePlayer player = m_doc->GetGame()->GetPlayer(pl);
    wxString label = ToText(outcome->GetPayoff(player)).c_str();
    int width, height;
    p_dc.GetTextExtent(label, &width, &height);
    p_dc.SetTextForeground(m_doc->GetPlayerColor(pl));
    p_dc.DrawText(label, point.x, point.y - height / 2);
    point.x += width + 10;
  }
}

void gbtTreeLayout::DrawSubtree(wxDC &p_dc, const gbtGameNode &p_node) const
{
  wxPoint point(GetX(p_node), m_ycoords[p_node->GetId()]);
  wxColour color;
  if (!p_node->GetPlayer().IsNull()) {
    color = m_doc->GetPlayerColor(p_node->GetPlayer()->GetId());
  }
  else {
    color = *wxBLACK;
  }

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    p_dc.SetPen(wxPen(color, 3, wxSOLID));
    wxPoint child(GetX(p_node->GetChild(i)),
		  m_ycoords[p_node->GetChild(i)->GetId()]);
    p_dc.DrawLine(point.x, point.y, child.x, child.y);
    DrawSubtree(p_dc, p_node->GetChild(i));

    // The "centerpoint" of the branch
    int xbar = (point.x + child.x) / 2;
    int ybar = (point.y + child.y) / 2;

    int textWidth, textHeight;
    p_dc.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
    p_dc.SetTextForeground(color);

    // Draw the label above the branch
    wxString label = wxString::Format(wxT("%s"),
				      p_node->GetInfoset()->GetAction(i)->GetLabel().c_str());
    p_dc.GetTextExtent(label, &textWidth, &textHeight);
    
    if (point.y >= child.y) {
      p_dc.DrawText(label, xbar - textWidth / 2, 
		    ybar - textHeight + 
		    textWidth / 2 * (child.y - point.y) / (child.x - point.x));
    }
    else {
      p_dc.DrawText(label, xbar - textWidth / 2, 
		    ybar - textHeight - 
		    textWidth / 2 * (child.y - point.y) / (child.x - point.x));
    }

    // Draw the label below the branch
    if (!p_node->GetInfoset()->GetPlayer()->IsChance()) {
      continue;
    }

    label = wxString::Format(wxT("%s"),
			     ToText(p_node->GetInfoset()->GetAction(i)->GetChanceProb()).c_str());
    p_dc.GetTextExtent(label, &textWidth, &textHeight);
    
    if (point.y >= child.y) {
      p_dc.DrawText(label, xbar - textWidth / 2,
		    ybar - textWidth/2 * (child.y - point.y) / (child.x - point.x));
    }
    else {
      p_dc.DrawText(label, xbar - textWidth / 2,
		    ybar + textWidth/2 * (child.y - point.y) / (child.x - point.x));
    }
  }

  if (p_node->NumChildren() > 0 && p_node->GetMemberId() > 1) {
    p_dc.SetPen(wxPen(color, 1, wxSOLID));
    gbtGameNode m = p_node->GetInfoset()->GetMember(p_node->GetMemberId() - 1);
    wxPoint member(GetX(m), m_ycoords[m->GetId()]);
    p_dc.DrawLine(point.x, point.y, member.x, member.y);
  }

  p_dc.SetPen(wxPen(color, 1, wxSOLID));
  p_dc.SetBrush(wxBrush(color, wxSOLID));
  p_dc.DrawCircle(point.x, point.y, 7);

  if (!p_node->GetOutcome().IsNull()) {
    DrawOutcome(p_dc, p_node);
  }
}

void gbtTreeLayout::DrawTree(wxDC &p_dc) const
{
  DrawSubtree(p_dc, m_doc->GetGame()->GetRoot());
}

int gbtTreeLayout::GetX(const gbtGameNode &p_node) const
{
  return (m_levels[m_nodeLevels[p_node->GetId()]] + 
	  15 * m_nodeSublevels[p_node->GetId()]);
}

void gbtTreeLayout::LayoutSubtree(const gbtGameNode &p_node, int p_depth,
				  gbtBlock<int> &p_sublevels)
{
  if (p_depth > p_sublevels.Length()) {
    p_sublevels.Append(0);
  }

  m_nodeLevels[p_node->GetId()] = p_depth;

  if (p_node->NumChildren() == 0 || p_node->GetInfoset()->NumMembers() == 1) {
    m_nodeSublevels[p_node->GetId()] = 0;
  }
  else if (p_node->GetMemberId() == 1) {
    m_nodeSublevels[p_node->GetId()] = ++p_sublevels[p_depth];
  }
  else {
    m_nodeSublevels[p_node->GetId()] = 0;
    for (int i = 1; i < p_node->GetMemberId(); i++) {
      int id = p_node->GetInfoset()->GetMember(i)->GetId();
      if (m_nodeLevels[id] == p_depth) {
	m_nodeSublevels[p_node->GetId()] = m_nodeSublevels[id];
	break;
      }
    }
    if (m_nodeSublevels[p_node->GetId()] == 0) {
      m_nodeSublevels[p_node->GetId()] = ++ p_sublevels[p_depth];
    }
  }

  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      LayoutSubtree(p_node->GetChild(i), p_depth + 1, p_sublevels);
    }

    int yMin = m_ycoords[p_node->GetChild(1)->GetId()];
    int yMax = m_ycoords[p_node->GetChild(p_node->NumChildren())->GetId()];
    m_ycoords[p_node->GetId()] = (yMin + yMax) / 2;
  }
  else {
    m_ycoords[p_node->GetId()] = m_maxY;
    m_maxY += 50;
  }
}

void gbtTreeLayout::LayoutTree(void)
{
  m_maxX = 0;
  m_maxY = 30;    // initial margin
  m_nodeLevels = gbtArray<int>(m_doc->GetGame()->NumNodes());
  m_nodeSublevels = gbtArray<int>(m_doc->GetGame()->NumNodes());
  m_ycoords = gbtArray<int>(m_doc->GetGame()->NumNodes());
  gbtBlock<int> sublevels;
  LayoutSubtree(m_doc->GetGame()->GetRoot(), 0, sublevels);
  m_levels = gbtArray<int>(0, sublevels.Length());
  m_levels[0] = 30;
  m_levels[1] = 50;
  for (int i = 2; i <= m_levels.Last(); i++) {
    m_levels[i] = m_levels[i - 1] + 50 + 15 * sublevels[i - 1];
  } 
  m_maxX += m_levels[m_levels.Last()] + 50;   // right margin, to allow for outcome display
}

//--------------------------------------------------------------------------
//                         class gbtTreeDisplay
//--------------------------------------------------------------------------

gbtTreeDisplay::gbtTreeDisplay(wxWindow *p_parent, 
			       gbtGameDocument *p_doc)
  : wxScrolledWindow(p_parent, -1), gbtGameView(p_doc),
    m_layout(p_doc)
{
  SetBackgroundColour(*wxWHITE);
  OnUpdate();
}

void gbtTreeDisplay::OnDraw(wxDC &p_dc)
{
  p_dc.SetUserScale(m_doc->GetTreeZoom(), m_doc->GetTreeZoom());
  m_layout.DrawTree(p_dc);
}

void gbtTreeDisplay::OnUpdate(void)
{
  m_layout.LayoutTree();
  SetScrollbars(50, 50, 
		(int) (m_layout.GetMaxX() * m_doc->GetTreeZoom() / 50.0) + 1, 
		(int) (m_layout.GetMaxY() * m_doc->GetTreeZoom() / 50.0) + 1);

  Refresh();
}

