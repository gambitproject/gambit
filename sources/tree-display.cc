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
#include "dialog-node.h"     // for node properties dialog
#include "dialog-outcome.h"  // for outcome payoffs

//--------------------------------------------------------------------------
//                       Bitmap drawing functions
//--------------------------------------------------------------------------

static wxBitmap MakePlayerBitmap(const wxColour &p_color, 
				 const wxString &p_label)
{
  wxBitmap bitmap(100, 31);
  wxMemoryDC dc;
  dc.SelectObject(bitmap);
  dc.SetBackground(wxBrush(*wxLIGHT_GREY, wxSOLID));
  dc.Clear();
  if (p_color == *wxBLACK) {
    dc.SetPen(wxPen(*wxWHITE, 1, wxSOLID));
  }
  else {
    dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
  }
  dc.SetBrush(wxBrush(p_color, wxSOLID));
  dc.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  dc.DrawRoundedRectangle(0, 0, 100, 31, 15);

  int width, height;
  dc.GetTextExtent(p_label, &width, &height);
  dc.DrawText(p_label, 50 - width/2, 15 - height/2);
  return bitmap;
}

static wxBitmap MakeOutcomeBitmap(void)
{
  wxBitmap bitmap(32, 32);
  wxMemoryDC dc;
  dc.SelectObject(bitmap);
  dc.Clear();
  dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawCircle(16, 16, 15);
  dc.SetFont(wxFont(16, wxSWISS, wxNORMAL, wxBOLD));
  dc.SetTextForeground(wxColour(0, 192, 0));

  int width, height;
  dc.GetTextExtent(wxT("$"), &width, &height);
  dc.DrawText(wxT("$"), 16 - width/2, 16 - height/2);
  return bitmap;
}

//--------------------------------------------------------------------------
//                         class gbtTreeToolbar
//--------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtTreeToolbar, wxScrolledWindow)
  EVT_LEFT_DOWN(gbtTreeToolbar::OnLeftDown)
END_EVENT_TABLE()

gbtTreeToolbar::gbtTreeToolbar(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxScrolledWindow(p_parent, -1, wxDefaultPosition, wxSize(110, -1)), 
    gbtGameView(p_doc)
{ 
  SetBackgroundColour(*wxLIGHT_GREY);
  OnUpdate();
}

void gbtTreeToolbar::OnDraw(wxDC &p_dc)
{
  int width, height;
  p_dc.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
  p_dc.GetTextExtent(wxT("Players:"), &width, &height);
  p_dc.SetTextForeground(*wxBLACK);
  p_dc.DrawText(wxT("Players:"), 55 - width/2, 5);

  int x = 5, y = 5 + height + 10;

  m_rects = gbtBlock<wxRect>();
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    gbtGamePlayer player = m_doc->GetGame()->GetPlayer(pl);
    wxBitmap bitmap = MakePlayerBitmap(m_doc->GetPlayerColor(player->GetId()),
				       wxString(player->GetLabel().c_str(),
						*wxConvCurrent));
    p_dc.DrawBitmap(bitmap, x, y);
    m_rects.Append(wxRect(x, y, bitmap.GetWidth(), bitmap.GetHeight()));
    y += bitmap.GetHeight() + 10;
  }

  wxBitmap bitmap = MakePlayerBitmap(*wxWHITE, wxT("Add player"));
  p_dc.DrawBitmap(bitmap, x, y);
  m_rects.Append(wxRect(x, y, bitmap.GetWidth(), bitmap.GetHeight()));
}

void gbtTreeToolbar::OnUpdate(void)
{
  Refresh();
}

void gbtTreeToolbar::OnLeftDown(wxMouseEvent &p_event)
{
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    if (m_rects[pl].Inside(p_event.GetPosition())) {
      wxBitmap bitmap = MakePlayerBitmap(m_doc->GetPlayerColor(pl),
					 wxString(m_doc->GetGame()->GetPlayer(pl)->GetLabel().c_str(),
						  *wxConvCurrent));

      wxIcon icon;
      icon.CopyFromBitmap(bitmap);
      
      wxTextDataObject textData(wxString::Format(wxT("P%d"), pl));
      wxDropSource source(textData, this, icon, icon, icon);
      wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);
      return;
    }
  }

  if (m_rects[m_doc->GetGame()->NumPlayers()+1].Inside(p_event.GetPosition())) {
    m_doc->NewPlayer();
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
    switch (p_text[0]) {
    case 'P': {
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
    case 'C': {
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
    case 'M': {
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
    case 'I': {
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
    case 'O': {
      long n;
      p_text.Right(p_text.Length() - 1).ToLong(&n);
      gbtGameNode srcNode = GetNode(doc->GetGame()->GetRoot(), n);
      
      if (srcNode.IsNull() || node == srcNode) {
	return false;
      }
      
      doc->SetOutcome(node, srcNode->GetOutcome());
      return true;
    }
    case 'o': {
      long n;
      p_text.Right(p_text.Length() - 1).ToLong(&n);
      gbtGameNode srcNode = GetNode(doc->GetGame()->GetRoot(), n);
      
      if (srcNode.IsNull() || node == srcNode) {
	return false;
      }
      
      doc->MoveOutcome(node, srcNode);
      return true;
    }
    }
  } 

  return false;
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
    
    if (!node.IsNull() && node->NumChildren() > 0) {
      gbtGamePlayer player = node->GetPlayer();
      wxString label;
      if (p_event.ShiftDown()) {
	label = wxT("i");
      }
      else {
	label = wxString(player->GetLabel().c_str(), *wxConvCurrent);
      }

      wxBitmap bitmap = MakePlayerBitmap(m_doc->GetPlayerColor(player->GetId()),
					 label);

      wxIcon icon;
      icon.CopyFromBitmap(bitmap);

      if (p_event.ControlDown()) {
	wxTextDataObject textData(wxString::Format(wxT("C%d"), node->GetId()));
	wxDropSource source(textData, this, icon, icon, icon);
	wxDragResult result = source.DoDragDrop(true);
      }
      else if (p_event.ShiftDown()) {
	wxTextDataObject textData(wxString::Format(wxT("I%d"), node->GetId()));
	wxDropSource source(textData, this, icon, icon, icon);
	wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);
      }
      else {
	wxTextDataObject textData(wxString::Format(wxT("M%d"), node->GetId()));
	wxDropSource source(textData, this, icon, icon, icon);
	wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);
      }
      return;
    }

    node = m_layout.OutcomeHitTest(x, y);
    
    if (!node.IsNull() && !node->GetOutcome().IsNull()) {
      wxBitmap bitmap = MakeOutcomeBitmap();
      wxIcon icon;
      icon.CopyFromBitmap(bitmap);

      if (p_event.ControlDown()) {
	wxTextDataObject textData(wxString::Format(wxT("O%d"), node->GetId()));
	wxDropSource source(textData, this, icon, icon, icon);
	wxDragResult result = source.DoDragDrop(true);
      }
      else {
	wxTextDataObject textData(wxString::Format(wxT("o%d"), node->GetId()));
	wxDropSource source(textData, this, icon, icon, icon);
	wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);
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

      if (dialog.GetOutcome() > 0) {
	if (node->GetOutcome().IsNull() || 
	    node->GetOutcome()->GetId() != dialog.GetOutcome()) {
	  m_doc->SetOutcome(node,
			    m_doc->GetGame()->GetOutcome(dialog.GetOutcome()));
	}
      }
      else {
	if (!node->GetOutcome().IsNull()) {
	  m_doc->SetOutcome(node, 0);
	}
      }
    }
    return;
  }

  node = m_layout.OutcomeHitTest(x, y);
  if (!node.IsNull()) {
    gbtOutcomeDialog dialog(this, m_doc, m_doc->GetGame(), node->GetOutcome());

    if (dialog.ShowModal() == wxID_OK) {
      gbtGameOutcome outcome = node->GetOutcome();
      if (outcome.IsNull()) {
	outcome = m_doc->NewOutcome();
	node->SetOutcome(outcome);
      }
      for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
	m_doc->SetPayoff(outcome, m_doc->GetGame()->GetPlayer(pl),
			 dialog.GetPayoff(pl));
      }
    }
    return;
  }
}

//--------------------------------------------------------------------------
//                         class gbtTreePanel
//--------------------------------------------------------------------------

gbtTreePanel::gbtTreePanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1)
{
  m_display = new gbtTreeDisplay(this, p_doc);
  m_toolbar = new gbtTreeToolbar(this, p_doc);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(m_toolbar, 0, wxEXPAND, 0);
  sizer->Add(m_display, 1, wxEXPAND, 0);
  SetSizer(sizer);
  Layout();
}
