//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of window class to display extensive form tree
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/dcps.h"
#include "wx/dragimag.h"

#include "base/gmisc.h"
#include "math/gmath.h"
#include "game/efg.h"
#include "treewin.h"
#include "efgshow.h"

#include "game/efgutils.h"
#include "base/glist.imp"


//----------------------------------------------------------------------
//                      TreeWindow: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(TreeWindow, wxScrolledWindow)
  EVT_SIZE(TreeWindow::OnSize)
  EVT_MOTION(TreeWindow::OnMouseMotion)
  EVT_LEFT_DOWN(TreeWindow::OnLeftClick)
  EVT_LEFT_DCLICK(TreeWindow::OnLeftDoubleClick)
  EVT_LEFT_UP(TreeWindow::OnMouseMotion)
  EVT_RIGHT_DOWN(TreeWindow::OnRightClick)
  EVT_CHAR(TreeWindow::OnKeyEvent)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                  TreeWindow: Constructor and destructor
//----------------------------------------------------------------------

TreeWindow::TreeWindow(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxScrolledWindow(p_parent), gbtGameView(p_doc),
    m_layout(p_doc),
    m_zoom(1.0), m_dragImage(0), m_dragSource(0)
{
  SetBackgroundColour(*wxWHITE);
  MakeMenus();
}

TreeWindow::~TreeWindow()
{ }

void TreeWindow::MakeMenus(void)
{
  m_nodeMenu = new wxMenu;

  m_nodeMenu->Append(wxID_CUT, "Cut subtree", "Cut subtree to clipboard");
  m_nodeMenu->Append(wxID_COPY, "Copy subtree", "Copy subtree to clipboard");
  m_nodeMenu->Append(wxID_PASTE, "Paste subtree",
		     "Paste subtree from clipboard");
  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_INSERT, "Insert move", "Insert a move");
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_REVEAL, "Reveal move",
		     "Reveal this move");
  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_TOGGLE_SUBGAME, "Mark subgame",
		     "Mark or unmark this subgame");
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_MARK_SUBGAME_TREE, "Mark subgame tree",
		     "Mark all subgames in this tree");
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_UNMARK_SUBGAME_TREE, "Unmark subgame tree",
		     "Unmark all subgames in this tree");
  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_NODE, "Edit node",
		     "View and change node properties");
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_MOVE, "Edit move",
		     "View and change move properties");
  m_nodeMenu->Append(GBT_EFG_MENU_EDIT_GAME, "Edit game",
		     "View and change game properties");

  m_gameMenu = new wxMenu;
  m_gameMenu->Append(GBT_EFG_MENU_EDIT_GAME, "Edit game",
		     "View and change game properties");
}

//---------------------------------------------------------------------
//                  TreeWindow: Event-hook members
//---------------------------------------------------------------------

static gbtEfgNode PriorSameIset(const gbtEfgNode &n)
{
  gbtEfgInfoset iset = n.GetInfoset();
  if (iset.IsNull()) return 0;
  for (int i = 1; i <= iset.NumMembers(); i++)
    if (iset.GetMember(i) == n)
      if (i > 1)
	return iset.GetMember(i-1);
      else 
	return 0;
  return 0;
}

static gbtEfgNode NextSameIset(const gbtEfgNode &n)
{
  gbtEfgInfoset iset = n.GetInfoset();
  if (iset.IsNull()) return 0;
  for (int i = 1; i <= iset.NumMembers(); i++)
    if (iset.GetMember(i) == n)
      if (i < iset.NumMembers()) 
	return iset.GetMember(i+1); 
      else
	return 0;
  return 0;
}

//
// OnKeyEvent -- handle keypress events
// Currently we support the following keys:
//     left arrow:   go to parent of current node
//     right arrow:  go to first child of current node
//     up arrow:     go to previous sibling of current node
//     down arrow:   go to next sibling of current node
// Since the addition of collapsible subgames, a node's parent may not
// be visible in the current display.  Thus, find the first predecessor
// that is visible (ROOT is always visible)
//
void TreeWindow::OnKeyEvent(wxKeyEvent &p_event)
{
  if (!m_doc->GetCursor().IsNull() && !p_event.ShiftDown()) {
    bool c = false;   // set to true if cursor position has changed
    switch (p_event.KeyCode()) {
    case WXK_LEFT:
      if (!m_doc->GetCursor().GetParent().IsNull()) {
	m_doc->SetCursor(m_layout.GetValidParent(m_doc->GetCursor())->GetNode());
	c = true;
      }
      break;
    case WXK_RIGHT:
      if (m_layout.GetValidChild(m_doc->GetCursor())) {
	m_doc->SetCursor(m_layout.GetValidChild(m_doc->GetCursor())->GetNode());
	c = true;
      }
      break;
    case WXK_UP: {
      gbtEfgNode prior = ((!p_event.ControlDown()) ? 
			  m_layout.PriorSameLevel(m_doc->GetCursor()) :
			  PriorSameIset(m_doc->GetCursor()));
      if (!prior.IsNull()) {
	m_doc->SetCursor(prior);
	c = true;
      }
      break;
    }
    case WXK_DOWN: {
      gbtEfgNode next = ((!p_event.ControlDown()) ?
			 m_layout.NextSameLevel(m_doc->GetCursor()) :
			 NextSameIset(m_doc->GetCursor()));
      if (!next.IsNull()) {
	m_doc->SetCursor(next);
	c = true;
      }
      break;
    }
    case WXK_SPACE:
      // Force a scroll to be sure selected node is visible
      c = true;
      break;
    }
        
    if (c) { 
      ProcessCursor(); // cursor moved
    }
  }
  else {
    p_event.Skip();
  }
}

//---------------------------------------------------------------------
//                   TreeWindow: Drawing functions
//---------------------------------------------------------------------

void TreeWindow::OnUpdate(gbtGameView *)
{
  m_layout.BuildNodeList(*m_doc->GetEfgSupport());
  m_layout.Layout(*m_doc->GetEfgSupport());
  if (!m_doc->GetCutNode().IsNull()) {
    m_layout.SetCutNode(m_doc->GetCutNode());
  }
  AdjustScrollbarSteps();

  gbtEfgNode cursor = m_doc->GetCursor();

  m_nodeMenu->Enable(wxID_COPY, !cursor.IsNull());
  m_nodeMenu->Enable(wxID_CUT, !cursor.IsNull());
  m_nodeMenu->Enable(wxID_PASTE, (!m_doc->GetCopyNode().IsNull() || 
				  !m_doc->GetCutNode().IsNull()));
  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_INSERT, !cursor.IsNull());
  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_REVEAL,
		     (!cursor.IsNull() && !cursor.GetInfoset().IsNull()));
  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_MOVE, 
		     (!cursor.IsNull() && !cursor.GetInfoset().IsNull()));

  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_TOGGLE_SUBGAME,
		     (!cursor.IsNull() && 
		      m_doc->GetEfg().IsLegalSubgame(cursor) &&
		      !cursor.GetParent().IsNull()));
  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_MARK_SUBGAME_TREE,
		     (!cursor.IsNull() && 
		      m_doc->GetEfg().IsLegalSubgame(cursor)));
  m_nodeMenu->Enable(GBT_EFG_MENU_EDIT_UNMARK_SUBGAME_TREE,
		     (!cursor.IsNull() && 
		      m_doc->GetEfg().IsLegalSubgame(cursor)));
  m_nodeMenu->SetLabel(GBT_EFG_MENU_EDIT_TOGGLE_SUBGAME,
		       (!cursor.IsNull() && !cursor.GetParent().IsNull() &&
			m_doc->GetEfg().IsLegalSubgame(cursor) &&
			cursor.GetSubgameRoot() == cursor) ?
		       "Unmark subgame" : "Mark subgame");
  Refresh();
}

void TreeWindow::AdjustScrollbarSteps(void)
{
  int width, height;
  GetClientSize(&width, &height);

  int scrollX, scrollY;
  GetViewStart(&scrollX, &scrollY);

  const int OUTCOME_LENGTH = 60;

  SetScrollbars(50, 50,
		(int) ((m_layout.MaxX() + m_doc->GetPreferences().NodeSize() + 
			OUTCOME_LENGTH) * m_zoom / 50 + 1),
		(int) (m_layout.MaxY() * m_zoom / 50 + 1),
		scrollX, scrollY);
}

void TreeWindow::FitZoom(void)
{
  int width, height;
  GetClientSize(&width, &height);

  double zoomx = (double) width / (double) m_layout.MaxX();
  double zoomy = (double) height / (double) m_layout.MaxY();

  zoomx = gmin(zoomx, 1.0); 
  zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
  m_zoom = gmin(zoomx, zoomy) * .9;
}

void TreeWindow::SetZoom(double p_zoom)
{
  m_zoom = p_zoom;
  AdjustScrollbarSteps();
  EnsureCursorVisible();
  Refresh();
}

void TreeWindow::OnDraw(wxDC &dc)
{
  if (!m_doc->GetCursor().IsNull()) {
    if (!m_layout.GetNodeEntry(m_doc->GetCursor())) {
      m_doc->SetCursor(m_doc->GetEfg().RootNode());
    }
    
    UpdateCursor();
  }
    
  dc.SetUserScale(m_zoom, m_zoom);
  dc.BeginDrawing();
  dc.Clear();
  m_layout.Render(dc);
  dc.EndDrawing();
}

void TreeWindow::OnDraw(wxDC &p_dc, double p_zoom)
{
  // Bit of a hack: this allows us to set zoom separately in printout code
  double saveZoom = m_zoom;
  m_zoom = p_zoom;
  OnDraw(p_dc);
  m_zoom = saveZoom;
}

void TreeWindow::EnsureCursorVisible(void)
{
  if (m_doc->GetCursor().IsNull()) {
    return;
  }

  NodeEntry *entry = m_layout.GetNodeEntry(m_doc->GetCursor()); 
  int xScroll, yScroll;
  GetViewStart(&xScroll, &yScroll);
  int width, height;
  GetClientSize(&width, &height);

  int xx, yy;
  CalcScrolledPosition((int) (entry->X() * m_zoom - 20),
		       (int) (entry->Y() * m_zoom), &xx, &yy);
  if (xx < 0) {
    xScroll -= -xx / 50 + 1;
  }
  const int OUTCOME_LENGTH = 60;
  CalcScrolledPosition((int) (entry->X() * m_zoom + 
			      m_doc->GetPreferences().NodeSize() +
			      OUTCOME_LENGTH),
		       (int) (entry->Y() * m_zoom), &xx, &yy);
  if (xx > width) {
    xScroll += (xx - width) / 50 + 1;
  }
  if (xScroll < 0) {
    xScroll = 0;
  }
  else if (xScroll > GetScrollRange(wxHORIZONTAL)) {
    xScroll = GetScrollRange(wxHORIZONTAL);
  }

  CalcScrolledPosition((int) (entry->X() * m_zoom),
		       (int) (entry->Y() * m_zoom - 20), &xx, &yy);
  if (yy < 0) {
    yScroll -= -yy / 50 + 1;
  }
  CalcScrolledPosition((int) (entry->X() * m_zoom),
		       (int) (entry->Y() * m_zoom + 20), &xx, &yy);
  if (yy > height) {
    yScroll += (yy - height) / 50 + 1;
  }
  if (yScroll < 0) {
    yScroll = 0;
  }
  else if (yScroll > GetScrollRange(wxVERTICAL)) {
    yScroll = GetScrollRange(wxVERTICAL);
  } 

  Scroll(xScroll, yScroll);
}

void TreeWindow::ProcessCursor(void)
{
  if (!m_doc->GetCursor().IsNull()) {
    NodeEntry *entry = m_layout.GetNodeEntry(m_doc->GetCursor()); 
    if (!entry) {
      m_doc->SetCursor(m_doc->GetEfg().RootNode());
      entry = m_layout.GetNodeEntry(m_doc->GetCursor());
    }
    
    UpdateCursor();
    EnsureCursorVisible();
  }
  Refresh();
}

void TreeWindow::UpdateCursor(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(m_doc->GetCursor());

  if (entry) {
    entry->SetCursor(true);
  }
}

//#include "bitmaps/copy.xpm"
//#include "bitmaps/move.xpm"

void TreeWindow::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    if (!m_dragImage) {
      int x, y;
      CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
      x = (int) ((float) x / m_zoom);
      y = (int) ((float) y / m_zoom);

      gbtEfgNode node = m_layout.NodeHitTest(x, y);
    
      if (!node.IsNull() && node.NumChildren() > 0) {
	m_dragSource = node;
	if (p_event.ControlDown()) {
	  m_dragImage = new wxDragImage("Copy subtree",
					wxCursor(wxCURSOR_HAND));
	  m_dragMode = dragCOPY;
	}
	else {
	  m_dragImage = new wxDragImage("Move subtree",
					wxCursor(wxCURSOR_HAND));
	  m_dragMode = dragMOVE;
	}

	m_dragImage->BeginDrag(wxPoint(0, 0), this);
	m_dragImage->Show();
	m_dragImage->Move(p_event.GetPosition());
	return;
      }
    }
    else {
      m_dragImage->Move(p_event.GetPosition());
    }
  }
  else if (!p_event.LeftIsDown() && m_dragImage) {
    m_dragImage->Hide();
    m_dragImage->EndDrag();
    delete m_dragImage;
    m_dragImage = 0;

    int x, y;
    CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
    x = (int) ((float) x / m_zoom);
    y = (int) ((float) y / m_zoom);

    gbtEfgNode node = m_layout.NodeHitTest(x, y);
    if (!node.IsNull() && node.NumChildren() == 0) {
      try {
	if (m_dragMode == dragCOPY) {
	  m_doc->Submit(new gbtCmdCopyTree(m_dragSource, node));
	}
	else if (m_dragMode == dragMOVE) {
	  m_doc->Submit(new gbtCmdMoveTree(m_dragSource, node));
	}
	else if (m_dragMode == dragOUTCOME) { 
	  m_doc->Submit(new gbtCmdSetOutcome(node,
					     m_dragSource.GetOutcome()));
	}
      }
      catch (gException &ex) {
	guiExceptionDialog(ex.Description(), this);
      }
    }
  }
}    

//
// Left mouse button click:
// Without key modifiers, selects a node
// With shift key, selects whole subtree (not yet implemented)
// With control key, adds node to selection (not yet implemented)
//
void TreeWindow::OnLeftClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  gbtEfgNode node = m_layout.NodeHitTest(x, y);
  m_doc->SetCursor(node);
  Refresh();
  ProcessCursor();
}

//
// Left mouse button double-click:
// Sets selection, brings up node properties dialog
//
void TreeWindow::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  gbtEfgNode node = m_layout.NodeHitTest(x, y);
  if (!node.IsNull()) {
    m_doc->SetCursor(node);
    Refresh();
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			 GBT_EFG_MENU_EDIT_NODE);
    GetParent()->AddPendingEvent(event);
  }
}

//
// Right mouse-button click:
// Set selection, display context-sensitive popup menu
//
void TreeWindow::OnRightClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  wxClientDC dc(this);
  PrepareDC(dc);
  dc.SetUserScale(m_zoom, m_zoom);

  gbtEfgNode node = m_layout.NodeHitTest(x, y);
  if (!node.IsNull()) {
    m_doc->SetCursor(node);
    Refresh();
    PopupMenu(m_nodeMenu, p_event.GetX(), p_event.GetY());
  }
  else {
    // If right-click doesn't hit anything, display generic game menu
    m_doc->SetCursor(0);
    Refresh();
    PopupMenu(m_gameMenu, p_event.GetX(), p_event.GetY());
  }
}

//-----------------------------------------------------------------------
//                     DISPLAY MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void TreeWindow::OnSize(wxSizeEvent &p_event)
{
  if (m_layout.MaxX() == 0 || m_layout.MaxY() == 0) {
    m_layout.Layout(*m_doc->GetEfgSupport());
  }

  // This extra check because wxMSW seems to generate OnSize events
  // rather liberally (e.g., a size of (0,0) for minimizing the window)
  if (p_event.GetSize().GetWidth() == 0 ||
      p_event.GetSize().GetHeight() == 0) {
    return;
  }

  /*
  double zoomx = ((double) p_event.GetSize().GetWidth() /
		  (double) m_layout.MaxX());
  double zoomy = ((double) p_event.GetSize().GetHeight() /
		  (double) m_layout.MaxY());
    
  zoomx = gmin(zoomx, 1.0);
  zoomy = gmin(zoomy, 1.0);
  m_zoom = gmin(zoomx, zoomy); 
  */
  AdjustScrollbarSteps();

  Refresh();
}


//=======================================================================
//                class gbtCmdMoveTree: Implementation
//=======================================================================

void gbtCmdMoveTree::Do(gbtGameDocument *p_doc)  
{
  p_doc->GetEfg().MoveTree(m_src, m_dest);
}

//=======================================================================
//                class gbtCmdCopyTree: Implementation
//=======================================================================

void gbtCmdCopyTree::Do(gbtGameDocument *p_doc)  
{
  p_doc->GetEfg().CopyTree(m_src, m_dest);
}

//=======================================================================
//               class gbtCmdSetOutcome: Implementation
//=======================================================================

void gbtCmdSetOutcome::Do(gbtGameDocument *p_doc)  
{
  m_node.SetOutcome(m_outcome);
}

template class gList<NodeEntry *>;

