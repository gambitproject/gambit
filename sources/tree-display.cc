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
  wxPoint point = m_nodeLocations[p_node->GetId()];
  point.x += 20;

  p_dc.SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD));

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
  const wxPoint &point = m_nodeLocations[p_node->GetId()];
  wxColour color;
  if (!p_node->GetPlayer().IsNull()) {
    color = m_doc->GetPlayerColor(p_node->GetPlayer()->GetId());
  }
  else {
    color = *wxBLACK;
  }

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    p_dc.SetPen(wxPen(color, 1, wxSOLID));
    const wxPoint &child = m_nodeLocations[p_node->GetChild(i)->GetId()];
    p_dc.DrawLine(point.x, point.y, child.x, child.y);
    DrawSubtree(p_dc, p_node->GetChild(i));
  }

  p_dc.SetPen(wxPen(color, 1, wxSOLID));
  p_dc.SetBrush(wxBrush(color, wxSOLID));
  p_dc.DrawCircle(point.x, point.y, 5);

  if (!p_node->GetOutcome().IsNull()) {
    DrawOutcome(p_dc, p_node);
  }
}

void gbtTreeLayout::DrawTree(wxDC &p_dc) const
{
  DrawSubtree(p_dc, m_doc->GetGame()->GetRoot());
}

void gbtTreeLayout::LayoutSubtree(const gbtGameNode &p_node, int p_depth)
{
  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      LayoutSubtree(p_node->GetChild(i), p_depth + 1);
    }

    int yMin = m_nodeLocations[p_node->GetChild(1)->GetId()].y;
    int yMax = m_nodeLocations[p_node->GetChild(p_node->NumChildren())->GetId()].y;
    m_nodeLocations[p_node->GetId()] = wxPoint(30 + p_depth * 50, (yMin + yMax) / 2);
  }
  else {
    m_nodeLocations[p_node->GetId()] = wxPoint(30 + p_depth * 50, m_maxY);
    m_maxY += 50;
  }

  if (30 + p_depth * 50 > m_maxX) {
    m_maxX = 30 + p_depth * 50;
  }
}

void gbtTreeLayout::LayoutTree(void)
{
  m_maxX = 0;
  m_maxY = 30;    // initial margin
  m_nodeLocations = gbtArray<wxPoint>(m_doc->GetGame()->NumNodes());
  LayoutSubtree(m_doc->GetGame()->GetRoot(), 0);
  m_maxX += 50;   // right margin, to allow for outcome display
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

