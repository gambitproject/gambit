//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/efgdisplay.cc
// Implementation of window class to display extensive form tree
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

#include <algorithm>    // for std::min

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/dnd.h>    // for drag-and-drop support
#include <wx/image.h>

#include "libgambit/libgambit.h"

#include "efgdisplay.h"
#include "menuconst.h"
#include "dlexcept.h"

//--------------------------------------------------------------------------
//                         class gbtPayoffEditor
//--------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtPayoffEditor, wxTextCtrl)
  EVT_CHAR(gbtPayoffEditor::OnChar)
END_EVENT_TABLE()

gbtPayoffEditor::gbtPayoffEditor(wxWindow *p_parent)
  : wxTextCtrl(p_parent, wxID_ANY, wxT(""), 
	       wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER)
{
  Show(false);
}

void gbtPayoffEditor::BeginEdit(gbtNodeEntry *p_entry, int p_player)
{
  m_entry = p_entry;
  m_outcome = p_entry->GetNode()->GetOutcome();
  m_player = p_player;
  SetValue(wxString(m_outcome->GetPayoff<std::string>(p_player).c_str(),
		    *wxConvCurrent));
  SetSize(wxSize(GetSize().GetWidth(), GetBestSize().GetHeight()));
  SetSelection(-1, -1);
  Show(true);
  SetFocus();
}

void gbtPayoffEditor::EndEdit(void)
{
  Show(false);
}

void gbtPayoffEditor::OnChar(wxKeyEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_TAB) {
    // We handle the event and pass it to the parent
    wxPostEvent(GetParent(), p_event);
  }
  else {
    // Default processing
    p_event.Skip();
  }
}

//--------------------------------------------------------------------------
//                       Bitmap drawing functions
//--------------------------------------------------------------------------

static wxBitmap MakeOutcomeBitmap(void)
{
  wxBitmap bitmap(24, 24);
  wxMemoryDC dc;
  dc.SelectObject(bitmap);
  dc.Clear();
  dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
  // Make a gold-colored background
  dc.SetBrush(wxBrush(wxColour(255, 215, 0), wxSOLID));
  dc.DrawCircle(12, 12, 10);
  dc.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
  dc.SetTextForeground(wxColour(0, 192, 0));

  int width, height;
  dc.GetTextExtent(wxT("u"), &width, &height);
  dc.DrawText(wxT("u"), 12 - width/2, 12 - height/2);
  return bitmap;
}

//--------------------------------------------------------------------------
//                      class gbtPlayerDropTarget
//--------------------------------------------------------------------------

class gbtPlayerDropTarget : public wxTextDropTarget {
private:
  gbtEfgDisplay *m_owner;
  gbtGameDocument *m_model;

  bool OnDropPlayer(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropCopyNode(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropMoveNode(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropInfoset(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropSetOutcome(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropMoveOutcome(Gambit::GameNode p_node, const wxString &p_text);
  bool OnDropCopyOutcome(Gambit::GameNode p_node, const wxString &p_text);

public:
  gbtPlayerDropTarget(gbtEfgDisplay *p_owner) 
    : m_owner(p_owner), m_model(p_owner->GetDocument())
  { }

  bool OnDropText(wxCoord x, wxCoord y, const wxString &p_text);
};

//
// This recurses the subtree starting at 'p_node' looking for a node
// with the ID 'p_id'. 
//
static Gambit::GameNode GetNode(Gambit::GameNode p_node, int p_id)
{
  if (p_node->GetNumber() == p_id) {
    return p_node;
  }
  else if (p_node->NumChildren() == 0) {
    return 0;
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      Gambit::GameNode node = GetNode(p_node->GetChild(i), p_id);
      if (node) return node;
    }
    return 0;
  }
}

bool gbtPlayerDropTarget::OnDropPlayer(Gambit::GameNode p_node,
				       const wxString &p_text)
{
  long pl;
  p_text.Right(p_text.Length() - 1).ToLong(&pl);
  Gambit::Game efg = m_model->GetGame();
  Gambit::GamePlayer player = ((pl == 0) ? efg->GetChance() : efg->GetPlayer(pl));
  if (p_node->NumChildren() == 0) {
    m_model->DoInsertMove(p_node, player, 2);
  }
  else if (p_node->GetPlayer() == player) {
    m_model->DoInsertAction(p_node);
  }
  else {
    m_model->DoSetPlayer(p_node, player);
  }
  return true;
}

bool gbtPlayerDropTarget::OnDropCopyNode(Gambit::GameNode p_node,
					 const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode)  return false;
  if (p_node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
    m_model->DoCopyTree(p_node, srcNode);
    return true;
  }
  return false;
}

bool gbtPlayerDropTarget::OnDropMoveNode(Gambit::GameNode p_node,
					 const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode)  return false;
  if (p_node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
    m_model->DoMoveTree(p_node, srcNode);
    return true;
  }
  return false;
}

bool gbtPlayerDropTarget::OnDropInfoset(Gambit::GameNode p_node,
					const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode)  return false;
  if (p_node->NumChildren() > 0 &&
      p_node->NumChildren() == srcNode->NumChildren()) {
    m_model->DoSetInfoset(p_node, srcNode->GetInfoset());
    return true;
  }
  else if (p_node->NumChildren() == 0 && srcNode->NumChildren() > 0) {
    m_model->DoAppendMove(p_node, srcNode->GetInfoset());
    return true;
  }
  return false;
}

bool gbtPlayerDropTarget::OnDropSetOutcome(Gambit::GameNode p_node,
					   const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || p_node == srcNode)  return false;
  m_model->DoSetOutcome(p_node, srcNode->GetOutcome());
  return true;
}

bool gbtPlayerDropTarget::OnDropMoveOutcome(Gambit::GameNode p_node,
					    const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || p_node == srcNode)  return false;
  m_model->DoSetOutcome(p_node, srcNode->GetOutcome());
  m_model->DoSetOutcome(srcNode, 0);
  return true;
}

bool gbtPlayerDropTarget::OnDropCopyOutcome(Gambit::GameNode p_node,
					    const wxString &p_text)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);
  Gambit::GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || p_node == srcNode)  return false;
  m_model->DoCopyOutcome(p_node, srcNode->GetOutcome());
  return true;
}

bool gbtPlayerDropTarget::OnDropText(wxCoord p_x, wxCoord p_y,
				     const wxString &p_text)
{
  Gambit::Game efg = m_owner->GetDocument()->GetGame();

  int x, y;
#if defined( __WXMSW__)
  // The +12 here is designed to effectively make the hot spot on
  // the cursor the center of the cursor image (they're currently
  // 24 pixels wide).
  m_owner->CalcUnscrolledPosition(p_x + 12, p_y + 12, &x, &y);
#else
  // Under GTK, there is an angle in the upper left-hand corner which
  // serves to identify the hot spot.  Thus, no adjustment is used
  m_owner->CalcUnscrolledPosition(p_x, p_y, &x, &y);
#endif  // __WXMSW__ or defined(__WXMAC__)

  x = (int) ((float) x / (.01 * m_owner->GetZoom()));
  y = (int) ((float) y / (.01 * m_owner->GetZoom()));

  Gambit::GameNode node = m_owner->GetLayout().NodeHitTest(x, y);
  if (!node)  return false;

  try {
    switch ((char) p_text[0]) {
    case 'P': return OnDropPlayer(node, p_text);
    case 'C': return OnDropCopyNode(node, p_text);
    case 'M': return OnDropMoveNode(node, p_text);
    case 'I': return OnDropInfoset(node, p_text);
    case 'O': return OnDropSetOutcome(node, p_text);
    case 'o': return OnDropMoveOutcome(node, p_text);
    case 'p': return OnDropCopyOutcome(node, p_text);
    default:  return false;
    }
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(m_owner, ex.what()).ShowModal();
    return false;
  }
}

//----------------------------------------------------------------------
//                      gbtEfgDisplay: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtEfgDisplay, wxScrolledWindow)
  EVT_MOTION(gbtEfgDisplay::OnMouseMotion)
  EVT_LEFT_DOWN(gbtEfgDisplay::OnLeftClick)
  EVT_LEFT_DCLICK(gbtEfgDisplay::OnLeftDoubleClick)
  EVT_RIGHT_DOWN(gbtEfgDisplay::OnRightClick)
  EVT_CHAR(gbtEfgDisplay::OnKeyEvent)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                gbtEfgDisplay: Constructor and destructor
//----------------------------------------------------------------------

gbtEfgDisplay::gbtEfgDisplay(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxScrolledWindow(p_parent), gbtGameView(p_doc),
    m_layout(this, p_doc), m_zoom(100)
{
  SetBackgroundColour(wxColour(250, 250, 250));

  m_payoffEditor = new gbtPayoffEditor(this);
  SetDropTarget(new gbtPlayerDropTarget(this));
  MakeMenus();

  Connect(m_payoffEditor->GetId(), wxEVT_COMMAND_TEXT_ENTER,
	  wxCommandEventHandler(gbtEfgDisplay::OnAcceptPayoffEdit));
  OnUpdate();
}

void gbtEfgDisplay::MakeMenus(void)
{
  m_nodeMenu = new wxMenu;

  m_nodeMenu->Append(wxID_UNDO, _("&Undo\tCtrl-Z"), _("Undo the last change"));
  m_nodeMenu->Append(wxID_REDO, _("&Redo\tCtrl-Y"),
		   _("Redo the last undone change"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_INSERT_MOVE, _("&Insert move"), 
		     _("Insert a move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_INSERT_ACTION, _("Insert &action"),
		     _("Insert an action at the current move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REVEAL, _("&Reveal"), 
		     _("Reveal choice at node"));
  m_nodeMenu->AppendSeparator();
  
  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_TREE, 
		     _("&Delete subtree"), 
		     _("Delete the subtree starting at the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_PARENT, 
		     _("Delete &parent"), 
		     _("Delete the node directly before the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REMOVE_OUTCOME,
		     _("Remove &outcome"),
		     _("Remove the outcome from the selected node"));
  m_nodeMenu->AppendSeparator();
  
  m_nodeMenu->Append(GBT_MENU_EDIT_NODE, _("&Node properties"),
		     _("Edit properties of the node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_MOVE, _("&Move properties"),
		     _("Edit properties of the move"));

  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_MENU_EDIT_GAME, _("&Game properties"),
		   _("Edit properties of the game"));
}

//---------------------------------------------------------------------
//                  gbtEfgDisplay: Event-hook members
//---------------------------------------------------------------------

static Gambit::GameNode PriorSameIset(const Gambit::GameNode &n)
{
  Gambit::GameInfoset iset = n->GetInfoset();
  if (!iset) return 0;
  for (int i = 1; i <= iset->NumMembers(); i++) {
    if (iset->GetMember(i) == n) {
      if (i > 1) {
	return iset->GetMember(i-1);
      } 
      else {
	return 0;
      }
    }
  }
  return 0;
}

static Gambit::GameNode NextSameIset(const Gambit::GameNode &n)
{
  Gambit::GameInfoset iset = n->GetInfoset();
  if (!iset) return 0;
  for (int i = 1; i <= iset->NumMembers(); i++) {
    if (iset->GetMember(i) == n) {
      if (i < iset->NumMembers()) {
	return iset->GetMember(i+1); 
      }
      else {
	return 0;
      }
    }
  }
  return 0;
}

//
// OnKeyEvent -- handle keypress events
// Currently we support the following keys:
//     left arrow:   go to parent of current node
//     right arrow:  go to first child of current node
//     up arrow:     go to previous sibling of current node
//     down arrow:   go to next sibling of current node
//     ALT-up:       go to previous member of information set
//     ALT-down:     go to next member of information set    
//     space:        ensure the selected node is visible
//     'R', 'r':     select the root node (and make it visible)
//     delete:       delete the subtree rooted at current node
//     backspace:    delete the parent of the current node
//     'M', 'm':     edit the move at the current node
//     'N', 'n':     edit the properties of the current node
//     escape:       cancel edit of payoff
//     tab:          accept edit of payoff, edit next payoff (if any)
//     
void gbtEfgDisplay::OnKeyEvent(wxKeyEvent &p_event)
{
  Gambit::GameNode selectNode = m_doc->GetSelectNode();

  if (p_event.GetKeyCode() == 'R' || p_event.GetKeyCode() == 'r') {
    m_doc->SetSelectNode(m_doc->GetGame()->GetRoot());
    EnsureNodeVisible(m_doc->GetSelectNode());
    return;
  }

  if (m_payoffEditor->IsEditing()) {
    if (p_event.GetKeyCode() == WXK_ESCAPE) {
      m_payoffEditor->EndEdit();
      return;
    }
    else if (p_event.GetKeyCode() == WXK_TAB) {
      m_payoffEditor->EndEdit();
      
      Gambit::GameOutcome outcome = m_payoffEditor->GetOutcome();
      int player = m_payoffEditor->GetPlayer();
      Gambit::GameNode node = m_payoffEditor->GetNodeEntry()->GetNode();
      try {
	m_doc->DoSetPayoff(outcome, player, m_payoffEditor->GetValue());
      }	
      catch (ValueException &) {
	// For the moment, we will just silently discard edits which 
	// give payoffs that are not valid numbers
	return;
      }
      catch (std::exception &ex) {
	gbtExceptionDialog(this, ex.what()).ShowModal();
	return;
      }

      // When we update views, the node entries get redone...
      // Payoff rectangles are actually set during drawing, so
      // force a refresh
      wxClientDC dc(this);
      PrepareDC(dc);
      OnDraw(dc);

      if (player < m_doc->NumPlayers()) {
	gbtNodeEntry *entry = m_layout.GetNodeEntry(node);
	wxRect rect = entry->GetPayoffExtent(player + 1);
	int xx, yy;
	CalcScrolledPosition((int) (.01 * (rect.x - 3) * m_zoom),
			     (int) (.01 * (rect.y - 3) * m_zoom), &xx, &yy);
	int width = (int) (.01 * (rect.width + 10) * m_zoom);
	int height = (int) (.01 * (rect.height + 6) * m_zoom);
	m_payoffEditor->SetSize(xx, yy, width, height);
	m_payoffEditor->BeginEdit(entry, player + 1);
      }

      return;
    }
  }

  // After this point, all events involve moving relative to selected node.
  // So if there isn't a selected node, the event doesn't apply
  if (!selectNode) {
    p_event.Skip();
    return;
  }

  switch (p_event.GetKeyCode()) {
  case 'M': case 'm': {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, 
			 GBT_MENU_EDIT_MOVE);
    wxPostEvent(this, event); 
    return;
  }
  case 'N': case 'n': {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, 
			 GBT_MENU_EDIT_NODE);
    wxPostEvent(this, event); 
    return;
  }
  case WXK_LEFT:
    if (selectNode->GetParent()) {
      m_doc->SetSelectNode(m_layout.GetValidParent(selectNode)->GetNode());
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  case WXK_RIGHT:
    if (m_layout.GetValidChild(selectNode)) {
      m_doc->SetSelectNode(m_layout.GetValidChild(selectNode)->GetNode());
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  case WXK_UP: {
    Gambit::GameNode prior = ((!p_event.AltDown()) ? 
			m_layout.PriorSameLevel(selectNode) :
			PriorSameIset(selectNode));
    if (prior) {
      m_doc->SetSelectNode(prior);
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  }
  case WXK_DOWN: {
    Gambit::GameNode next = ((!p_event.AltDown()) ?
		       m_layout.NextSameLevel(selectNode) :
		       NextSameIset(selectNode));
    if (next) {
      m_doc->SetSelectNode(next);
      EnsureNodeVisible(m_doc->GetSelectNode());
    }
    return;
  }
  case WXK_SPACE:
    EnsureNodeVisible(m_doc->GetSelectNode());
    return;
  case WXK_DELETE: {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, 
			 GBT_MENU_EDIT_DELETE_TREE);
    wxPostEvent(this, event); 
    return;
  }
  case WXK_BACK: {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			 GBT_MENU_EDIT_DELETE_PARENT);
    wxPostEvent(this, event);
    return;
  }
  default:
    // If nothing else applies, let event propagate
    p_event.Skip();
  }
}

void gbtEfgDisplay::OnAcceptPayoffEdit(wxCommandEvent &)
{
  m_payoffEditor->EndEdit();
  Gambit::GameOutcome outcome = m_payoffEditor->GetOutcome();
  int player = m_payoffEditor->GetPlayer();
  try {
    m_doc->DoSetPayoff(outcome, player, m_payoffEditor->GetValue());
  }
  catch (ValueException &) {
    // For the moment, we will just silently discard edits which 
    // give payoffs that are not valid numbers
    return;
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

//---------------------------------------------------------------------
//           gbtEfgDisplay: Implementing gbtGameView members
//---------------------------------------------------------------------

void gbtEfgDisplay::PostPendingChanges(void)
{
  // FIXME: Save edit!
  m_payoffEditor->EndEdit();
}

void gbtEfgDisplay::OnUpdate(void)
{
  // First make sure that the selected node is in fact still valid
  if (m_doc->GetSelectNode()) {
    gbtNodeEntry *entry = m_layout.GetNodeEntry(m_doc->GetSelectNode());
    if (!entry) {
      m_doc->SetSelectNode(0);
    }
  }

  // Force a rebuild on every change for now.
  RefreshTree();

  Gambit::GameNode selectNode = m_doc->GetSelectNode();

  m_nodeMenu->Enable(wxID_UNDO, m_doc->CanUndo());
  m_nodeMenu->Enable(wxID_REDO, m_doc->CanRedo());
  m_nodeMenu->Enable(GBT_MENU_EDIT_INSERT_MOVE, selectNode);
  m_nodeMenu->Enable(GBT_MENU_EDIT_INSERT_ACTION,
		     selectNode && selectNode->GetInfoset());
  m_nodeMenu->Enable(GBT_MENU_EDIT_REVEAL,
		     selectNode && selectNode->GetInfoset());
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_TREE,
		     selectNode && selectNode->NumChildren() > 0);
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_PARENT,
		     selectNode && selectNode->GetParent());
  m_nodeMenu->Enable(GBT_MENU_EDIT_REMOVE_OUTCOME,
		     selectNode && selectNode->GetOutcome());
  m_nodeMenu->Enable(GBT_MENU_EDIT_NODE, selectNode);
  m_nodeMenu->Enable(GBT_MENU_EDIT_MOVE, 
		     selectNode && selectNode->GetInfoset());
}

//---------------------------------------------------------------------
//                   gbtEfgDisplay: Drawing functions
//---------------------------------------------------------------------

void gbtEfgDisplay::RefreshTree(void)
{
  m_layout.BuildNodeList(m_doc->GetEfgSupport());
  m_layout.Layout(m_doc->GetEfgSupport());
  Refresh();
}

void gbtEfgDisplay::AdjustScrollbarSteps(void)
{
  int width, height;
  GetClientSize(&width, &height);

  int scrollX, scrollY;
  GetViewStart(&scrollX, &scrollY);

  SetScrollbars(50, 50,
		(int) (m_layout.MaxX() * (.01 * m_zoom) / 50 + 1),
		(int) (m_layout.MaxY() * (.01 * m_zoom) / 50 + 1),
		scrollX, scrollY);
}

void gbtEfgDisplay::FitZoom(void)
{
  int width, height;
  GetClientSize(&width, &height);

  double zoomx = (double) width / (double) m_layout.MaxX();
  double zoomy = (double) height / (double) m_layout.MaxY();

  zoomx = std::min(zoomx, 1.0); 
  zoomy = std::min(zoomy, 1.0);  // never zoom in (only out)
  m_zoom = int(100.0 * (std::min(zoomx, zoomy) * .9));
  AdjustScrollbarSteps();
  Refresh();
}

void gbtEfgDisplay::SetZoom(int p_zoom)
{
  m_zoom = p_zoom;
  AdjustScrollbarSteps();
  EnsureNodeVisible(m_doc->GetSelectNode());
  Refresh();
}

void gbtEfgDisplay::OnDraw(wxDC &p_dc)
{
  p_dc.SetUserScale(.01 * m_zoom, .01 * m_zoom);
#if !wxCHECK_VERSION(2,7,0)
  p_dc.BeginDrawing();
#endif
  p_dc.Clear();
  int maxX = m_layout.MaxX();
  m_layout.Render(p_dc, false);
#if !wxCHECK_VERSION(2,7,0)
  p_dc.EndDrawing();
#endif
  // When we draw, we might change the location of the right margin
  // (because of the outcome labels).  Make sure scrollbars are adjusted
  // to reflect this.
  if (maxX != m_layout.MaxX()) {
    AdjustScrollbarSteps();
  }
}

void gbtEfgDisplay::OnDraw(wxDC &p_dc, double p_zoom)
{
  // Bit of a hack: this allows us to set zoom separately in printout code
  int saveZoom = m_zoom;
  m_zoom = int(100.0 * p_zoom);

  p_dc.SetUserScale(.01 * m_zoom, .01 * m_zoom);
#if !wxCHECK_VERSION(2,7,0)
  p_dc.BeginDrawing();
#endif
  p_dc.Clear();
  int maxX = m_layout.MaxX();
  // A second hack: this is usually only called by functions for hardcopy
  // output (printouts or graphics images).  We want to suppress the
  // use of the "hints" for these.
  // FIXME: Of course, this hack implies some useful refactor is called for!
  m_layout.Render(p_dc, true);
#if !wxCHECK_VERSION(2,7,0)
  p_dc.EndDrawing();
#endif
  // When we draw, we might change the location of the right margin
  // (because of the outcome labels).  Make sure scrollbars are adjusted
  // to reflect this.
  if (maxX != m_layout.MaxX()) {
    AdjustScrollbarSteps();
  }

  m_zoom = saveZoom;
}

void gbtEfgDisplay::EnsureNodeVisible(Gambit::GameNode p_node)
{
  if (!p_node)  return;

  gbtNodeEntry *entry = m_layout.GetNodeEntry(p_node);
  int xScroll, yScroll;
  GetViewStart(&xScroll, &yScroll);
  int width, height;
  GetClientSize(&width, &height);

  int xx, yy;
  CalcScrolledPosition((int) (entry->X() * (.01 * m_zoom) - 20),
		       (int) (entry->Y() * (.01 * m_zoom)), &xx, &yy);
  if (xx < 0) {
    xScroll -= -xx / 50 + 1;
  }

  CalcScrolledPosition((int) (entry->X() * (.01 * m_zoom)), 
		       (int) (entry->Y() * (.01 * m_zoom)), &xx, &yy);
  if (xx > width) {
    xScroll += (xx - width) / 50 + 1;
  }
  if (xScroll < 0) {
    xScroll = 0;
  }
  else if (xScroll > GetScrollRange(wxHORIZONTAL)) {
    xScroll = GetScrollRange(wxHORIZONTAL);
  }

  CalcScrolledPosition((int) (entry->X() * (.01 * m_zoom)),
		       (int) (entry->Y() * (.01 * m_zoom) - 20), &xx, &yy);
  if (yy < 0) {
    yScroll -= -yy / 50 + 1;
  }
  CalcScrolledPosition((int) (entry->X() * (.01 * m_zoom)),
		       (int) (entry->Y() * (.01 * m_zoom) + 20), &xx, &yy);
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

//
// Left mouse button click:
// Without key modifiers, selects a node
// With shift key, selects whole subtree (not yet implemented)
// With control key, adds node to selection (not yet implemented)
//
void gbtEfgDisplay::OnLeftClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / (.01 * m_zoom));
  y = (int) ((float) y / (.01 * m_zoom));

  Gambit::GameNode node = m_layout.NodeHitTest(x, y);
  if (node != m_doc->GetSelectNode()) {
    m_doc->SetSelectNode(node);
  }
}

//
// Left mouse button double-click:
// Sets selection, brings up node properties dialog
//
void gbtEfgDisplay::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / (.01 * m_zoom));
  y = (int) ((float) y / (.01 * m_zoom));

  Gambit::GameNode node = m_layout.NodeHitTest(x, y);
  if (node) {
    m_doc->SetSelectNode(node);
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_NODE);
    wxPostEvent(this, event); 
    return;
  }

  node = m_layout.OutcomeHitTest(x, y);
  if (node) {
    if (!node->GetOutcome()) {
      // Create a new outcome
      m_doc->DoNewOutcome(node);
      // Payoff rectangles are actually set during drawing, so
      // force a refresh
      wxClientDC dc(this);
      PrepareDC(dc);
      OnDraw(dc);

      gbtNodeEntry *entry = m_layout.GetNodeEntry(node);
      wxRect rect = entry->GetPayoffExtent(1);
      
      int xx, yy;
      CalcScrolledPosition((int) (.01 * (rect.x - 3) * m_zoom),
			   (int) (.01 * (rect.y - 3) * m_zoom), &xx, &yy);
      int width = (int) (.01 * (rect.width + 10) * m_zoom);
      int height = (int) (.01 * (rect.height + 6) * m_zoom);
      m_payoffEditor->SetSize(xx, yy, width, height);
      m_payoffEditor->BeginEdit(entry, 1);
      return;
    }

    // Editing an existing outcome
    gbtNodeEntry *entry = m_layout.GetNodeEntry(node);
    for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
      wxRect rect = entry->GetPayoffExtent(pl);
#if wxCHECK_VERSION(2,7,0)
      if (rect.Contains(x, y)) {
#else
      if (rect.Inside(x, y)) {
#endif
	int xx, yy;
	CalcScrolledPosition((int) (.01 * (rect.x - 3) * m_zoom),
			     (int) (.01 * (rect.y - 3) * m_zoom), &xx, &yy);
	int width = (int) (.01 * (rect.width + 10) * m_zoom);
	int height = (int) (.01 * (rect.height + 6) * m_zoom);
	m_payoffEditor->SetSize(xx, yy, width, height);
	m_payoffEditor->BeginEdit(entry, pl);
	return;
      }
    }

    return;
  }

  if (m_doc->GetStyle().BranchAboveLabel() == GBT_BRANCH_LABEL_LABEL) {
    node = m_layout.BranchAboveHitTest(x, y);
    if (node) {
      m_doc->SetSelectNode(node);
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_MOVE);
      wxPostEvent(this, event);
      return;
    }
  }

  if (m_doc->GetStyle().BranchBelowLabel() == GBT_BRANCH_LABEL_LABEL) {
    node = m_layout.BranchBelowHitTest(x, y);
    if (node) {
      m_doc->SetSelectNode(node);
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_MOVE);
      wxPostEvent(this, event);
      return;
    }
  }

  
}

#include "bitmaps/tree.xpm"
#include "bitmaps/move.xpm"

void gbtEfgDisplay::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    int x, y;
    CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
    x = (int) ((float) x / (.01 * GetZoom()));
    y = (int) ((float) y / (.01 * GetZoom()));
  
    Gambit::GameNode node = m_layout.NodeHitTest(x, y);
    
    if (node && node->NumChildren() > 0) {
      Gambit::GamePlayer player = node->GetPlayer();
      wxString label;
      if (p_event.ShiftDown()) {
	label = wxT("i");
      }
      else {
	label = wxString(player->GetLabel().c_str(), *wxConvCurrent);
      }

      if (p_event.ControlDown()) {
	// Copy subtree
	wxBitmap bitmap(tree_xpm);
#if defined( __WXMSW__) or defined(__WXMAC__)
	wxImage image = bitmap.ConvertToImage();
#else
	wxIcon image;
	image.CopyFromBitmap(bitmap);
#endif // _WXMSW__

	wxTextDataObject textData(wxString::Format(wxT("C%d"), node->GetNumber()));
	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(true);
      }
      else if (p_event.ShiftDown()) {
	// Copy move (information set)
	// This should be the pawn icon!
	wxBitmap bitmap(move_xpm);
#if defined( __WXMSW__) or defined(__WXMAC__)
	wxImage image = bitmap.ConvertToImage();
#else
	wxIcon image;
	image.CopyFromBitmap(bitmap);
#endif // _WXMSW__

	wxTextDataObject textData(wxString::Format(wxT("I%d"), node->GetNumber()));

	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(wxDrag_DefaultMove);
      }
      else {
	// Move subtree
	wxBitmap bitmap(tree_xpm);
#if defined( __WXMSW__) or defined(__WXMAC__)
	wxImage image = bitmap.ConvertToImage();
#else
	wxIcon image;
	image.CopyFromBitmap(bitmap);
#endif // _WXMSW__
	
	wxTextDataObject textData(wxString::Format(wxT("M%d"), node->GetNumber()));

	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(wxDrag_DefaultMove);
      }
      return;
    }

    node = m_layout.OutcomeHitTest(x, y);
    
    if (node && node->GetOutcome()) {
      wxBitmap bitmap = MakeOutcomeBitmap();
#if defined( __WXMSW__) or defined(__WXMAC__)
      wxImage image = bitmap.ConvertToImage();
#else
      wxIcon image;
      image.CopyFromBitmap(bitmap);
#endif // _WXMSW__

      if (p_event.ControlDown()) {
	wxTextDataObject textData(wxString::Format(wxT("O%d"), node->GetNumber()));
	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(true);
      }
      else if (p_event.ShiftDown()) {
	wxTextDataObject textData(wxString::Format(wxT("p%d"), node->GetNumber()));
	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(true);
      }
      else {
	wxTextDataObject textData(wxString::Format(wxT("o%d"), node->GetNumber()));
	wxDropSource source(textData, this, image, image, image);
	/*wxDragResult result =*/ source.DoDragDrop(wxDrag_DefaultMove);
      }
    }
  }
}

//
// Right mouse-button click:
// Set selection, display context-sensitive popup menu
//
void gbtEfgDisplay::OnRightClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / (.01 * m_zoom));
  y = (int) ((float) y / (.01 * m_zoom));

  Gambit::GameNode node = m_layout.NodeHitTest(x, y);
  if (node != m_doc->GetSelectNode()) {
    m_doc->SetSelectNode(node);
  }
  PopupMenu(m_nodeMenu);
}

