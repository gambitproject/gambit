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
#include <wx/dnd.h>    // for drag-and-drop support

#include "tree-display.h"

//--------------------------------------------------------------------------
//                         class gbtTreeToolbar
//--------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtTreeToolbar, wxScrolledWindow)
  EVT_LEFT_DOWN(gbtTreeToolbar::OnLeftDown)
END_EVENT_TABLE()

gbtTreeToolbar::gbtTreeToolbar(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxScrolledWindow(p_parent, -1), gbtGameView(p_doc)
{ 
  OnUpdate();
}

void gbtTreeToolbar::OnDraw(wxDC &p_dc)
{
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    int x = (pl - 1) * 20 + 10;
    int y = 5;
    p_dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
    p_dc.SetBrush(wxBrush(m_doc->GetPlayerColor(pl), wxSOLID));
    p_dc.SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD));
    p_dc.DrawRectangle(x, y, 16, 15);
    p_dc.DrawText(wxString::Format(_("%d"), pl), x + 4, y);
  }
}

void gbtTreeToolbar::OnUpdate(void)
{
  Refresh();
}

void gbtTreeToolbar::OnLeftDown(wxMouseEvent &p_event)
{
  if (p_event.GetY() < 5 || p_event.GetY() > 20) {
    p_event.Skip();
    return;
  }

  int pl = ((p_event.GetX() - 10) / 20) + 1;
  if (pl < 1 || pl > m_doc->GetGame()->NumPlayers()) {
    return;
  }

  wxTextDataObject textData(wxString::Format(_("%d"), pl));
  wxDropSource source(textData, this);
  wxDragResult result = source.DoDragDrop(true);
}

//--------------------------------------------------------------------------
//                      class gbtPlayerDropTarget
//--------------------------------------------------------------------------

class gbtPlayerDropTarget : public wxTextDropTarget {
private:
  gbtTreeDisplay *m_owner;

public:
  gbtPlayerDropTarget(gbtTreeDisplay *p_owner) { m_owner = p_owner; }

  bool OnDropText(wxCoord x, wxCoord y, const wxString &p_text);
};

bool gbtPlayerDropTarget::OnDropText(wxCoord x, wxCoord y,
				     const wxString &p_text)
{
  gbtGameNode node = m_owner->GetLayout().NodeHitTest(x, y);

  if (!node.IsNull()) {
    if (node->NumChildren() == 0) {
      long pl;
      p_text.ToLong(&pl);
      gbtGameDocument *doc = m_owner->GetDocument();
      doc->NewMove(node, doc->GetGame()->GetPlayer(pl));
      return true;
    }
    return false;
  }
  else {
    return false;
  }
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
  SetDropTarget(new gbtPlayerDropTarget(this));
  OnUpdate();
}

void gbtTreeDisplay::OnDraw(wxDC &p_dc)
{
  p_dc.SetUserScale(m_doc->GetTreeZoom(), m_doc->GetTreeZoom());
  m_layout.DrawTree(p_dc);
}

void gbtTreeDisplay::OnUpdate(void)
{
  m_layout.Layout();
  SetScrollbars(50, 50, 
		(int) (m_layout.GetMaxX() * m_doc->GetTreeZoom() / 50.0) + 1, 
		(int) (m_layout.GetMaxY() * m_doc->GetTreeZoom() / 50.0) + 1);

  Refresh();
}

