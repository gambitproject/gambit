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
#include "dialog-node.h"   // for node properties dialog

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

  int x = m_doc->GetGame()->NumPlayers() * 20 + 10;
  int y = 5;

  p_dc.SetBrush(*wxWHITE_BRUSH);
  p_dc.DrawRectangle(x, y, 16, 15);
  p_dc.DrawText(_("+"), x + 4, y);
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
  if (pl == m_doc->GetGame()->NumPlayers() + 1) {
    m_doc->NewPlayer();
  }
  else if (pl >= 1 && pl <= m_doc->GetGame()->NumPlayers()) {
    wxTextDataObject textData(wxString::Format(wxT("P%d"), pl));
    wxDropSource source(textData, this);
    wxDragResult result = source.DoDragDrop(true);
  }
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

//
// This recurses the subtree starting at 'p_node' looking for a node
// with the ID 'p_id'. 
//
static gbtGameNode GetNode(const gbtGameNode &p_node, int p_id)
{
  if (p_node->GetId() == p_id) {
    return p_node;
  }
  else if (p_node->NumChildren() == 0) {
    return 0;
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      gbtGameNode node = GetNode(p_node->GetChild(i), p_id);
      if (!node.IsNull())  return node;
    }
    return 0;
  }
}

bool gbtPlayerDropTarget::OnDropText(wxCoord p_x, wxCoord p_y,
				     const wxString &p_text)
{
  gbtGameDocument *doc = m_owner->GetDocument();
  int x, y;
  m_owner->CalcUnscrolledPosition(p_x, p_y, &x, &y);
  x = (int) ((float) x / doc->GetTreeZoom());
  y = (int) ((float) y / doc->GetTreeZoom());

  gbtGameNode node = m_owner->GetLayout().NodeHitTest(x, y);

  if (!node.IsNull()) {
    if (p_text[0] == 'P') { 
      long pl;
      p_text.Right(p_text.Length() - 1).ToLong(&pl);
      gbtGamePlayer player = doc->GetGame()->GetPlayer(pl);
    
      if (node->NumChildren() == 0) {
	doc->NewMove(node, player);
      }
      else if (node->GetPlayer() == player) {
	doc->NewAction(node->GetInfoset());
      }
      else {
	doc->SetPlayer(node->GetInfoset(), player);
      }
      return true;
    }
    else if (p_text[0] == 'C') {
      long n;
      p_text.Right(p_text.Length() - 1).ToLong(&n);
      gbtGameNode srcNode = GetNode(doc->GetGame()->GetRoot(), n);

      if (srcNode.IsNull()) {
	return false;
      }
      
      if (node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
	doc->CopyTree(srcNode, node);
	return true;
      }
      
      return false;
    }
    else if (p_text[0] == 'M') {
      long n;
      p_text.Right(p_text.Length() - 1).ToLong(&n);
      gbtGameNode srcNode = GetNode(doc->GetGame()->GetRoot(), n);

      if (srcNode.IsNull()) {
	return false;
      }
      
      if (node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
	doc->MoveTree(srcNode, node);
	return true;
      }
      
      return false;
    }
    else if (p_text[0] == 'I') {
      long n;
      p_text.Right(p_text.Length() - 1).ToLong(&n);
      gbtGameNode srcNode = GetNode(doc->GetGame()->GetRoot(), n);

      if (srcNode.IsNull()) {
	return false;
      }
      
      if (node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
	doc->SetMove(node, srcNode->GetInfoset());
	return true;
      }
      
      return false;
    }
  }
  else {
    return false;
  }
}

//--------------------------------------------------------------------------
//                         class gbtTreeDisplay
//--------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtTreeDisplay, wxScrolledWindow)
  EVT_MOTION(gbtTreeDisplay::OnMouseMotion)
  EVT_LEFT_DCLICK(gbtTreeDisplay::OnLeftClick)
END_EVENT_TABLE()

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

void gbtTreeDisplay::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    int x, y;
    CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
    x = (int) ((float) x / m_doc->GetTreeZoom());
    y = (int) ((float) y / m_doc->GetTreeZoom());
  
    gbtGameNode node = m_layout.NodeHitTest(x, y);
    
    if (!node.IsNull()) {
      if (p_event.ControlDown()) {
	wxTextDataObject textData(wxString::Format(wxT("C%d"), node->GetId()));
	wxDropSource source(textData, this);
	wxDragResult result = source.DoDragDrop(true);
      }
      else if (p_event.ShiftDown()) {
	wxTextDataObject textData(wxString::Format(wxT("I%d"), node->GetId()));
	wxDropSource source(textData, this);
	wxDragResult result = source.DoDragDrop(true);
      }
      else {
	wxTextDataObject textData(wxString::Format(wxT("M%d"), node->GetId()));
	wxDropSource source(textData, this);
	wxDragResult result = source.DoDragDrop(true);
      }
    }
  }
}

void gbtTreeDisplay::OnLeftClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_doc->GetTreeZoom());
  y = (int) ((float) y / m_doc->GetTreeZoom());
  
  gbtGameNode node = m_layout.NodeHitTest(x, y);

  if (!node.IsNull()) {
    gbtNodeDialog dialog(this, m_doc->GetGame(), node);

    if (dialog.ShowModal() == wxID_OK) {
      if (dialog.GetNodeLabel() != node->GetLabel()) {
	m_doc->SetNodeLabel(node, dialog.GetNodeLabel());
      }
      if (node->NumChildren() > 0 &&
	  dialog.GetInfoset() != node->GetInfoset()) {
	m_doc->SetInfoset(node, dialog.GetInfoset());
      }
    }
  }
}
