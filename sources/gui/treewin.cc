//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of window class to display extensive form tree
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/dcps.h"
#include "wx/dragimag.h"
#include "guishare/wxmisc.h"

#include "base/gmisc.h"
#include "math/math.h"
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

TreeWindow::TreeWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxScrolledWindow(p_parent),
    m_efg(*p_efgShow->Game()), m_parent(p_efgShow), m_layout(m_efg, this),
    m_zoom(1.0), m_dragImage(0), m_dragSource(0)
{
  // Make sure that Chance player has a name
  m_efg.GetChance()->SetName("Chance");

  SetBackgroundColour(*wxWHITE);
  MakeMenus();
}

TreeWindow::~TreeWindow()
{ }

void TreeWindow::MakeMenus(void)
{
  m_nodeMenu = new wxMenu;

  m_nodeMenu->Append(efgmenuEDIT_INSERT, "Insert move", "Insert a move");
  m_nodeMenu->Append(efgmenuEDIT_REVEAL, "Reveal move",
		     "Reveal the move");
  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(efgmenuEDIT_NODE, "Edit node",
		     "View and change node properties");
  m_nodeMenu->Append(efgmenuEDIT_MOVE, "Edit move",
		     "View and change move properties");
  m_nodeMenu->Append(efgmenuEDIT_GAME, "Edit game",
		     "View and change game properties");

  m_gameMenu = new wxMenu;
  m_gameMenu->Append(efgmenuEDIT_GAME, "Edit game",
		     "View and change game properties");
}

//---------------------------------------------------------------------
//                  TreeWindow: Event-hook members
//---------------------------------------------------------------------

static Node *PriorSameIset(const Node *n)
{
    Infoset *iset = n->GetInfoset();
    if (!iset) return 0;
    for (int i = 1; i <= iset->NumMembers(); i++)
        if (iset->Members()[i] == n)
            if (i > 1)
                return iset->Members()[i-1];
            else 
                return 0;
    return 0;
}

static Node *NextSameIset(const Node *n)
{
    Infoset *iset = n->GetInfoset();
    if (!iset) return 0;
    for (int i = 1; i <= iset->NumMembers(); i++)
        if (iset->Members()[i] == n)
            if (i < iset->NumMembers()) 
                return iset->Members()[i+1]; 
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
  if (m_parent->Cursor() && !p_event.ShiftDown()) {
    bool c = false;   // set to true if cursor position has changed
    switch (p_event.KeyCode()) {
    case WXK_LEFT:
      if (m_parent->Cursor()->GetParent()) {
	m_parent->SetCursor(m_layout.GetValidParent(m_parent->Cursor())->GetNode());
	c = true;
      }
      break;
    case WXK_RIGHT:
      if (m_layout.GetValidChild(m_parent->Cursor())) {
	m_parent->SetCursor(m_layout.GetValidChild(m_parent->Cursor())->GetNode());
	c = true;
      }
      break;
    case WXK_UP: {
      Node *prior = ((!p_event.ControlDown()) ? 
		     m_layout.PriorSameLevel(m_parent->Cursor()) :
		     PriorSameIset(m_parent->Cursor()));
      if (prior) {
	m_parent->SetCursor(prior);
	c = true;
      }
      break;
    }
    case WXK_DOWN: {
      Node *next = ((!p_event.ControlDown()) ?
		    m_layout.NextSameLevel(m_parent->Cursor()) :
		    NextSameIset(m_parent->Cursor()));
      if (next) {
	m_parent->SetCursor(next);
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

void TreeWindow::RefreshTree(void)
{
  m_layout.BuildNodeList(*m_parent->GetSupport());
  m_layout.Layout(*m_parent->GetSupport());
  AdjustScrollbarSteps();
}

void TreeWindow::RefreshLayout(void)
{
  m_layout.Layout(*m_parent->GetSupport());
  AdjustScrollbarSteps();
}

void TreeWindow::RefreshLabels(void)
{
  m_layout.GenerateLabels();
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
		(int) ((m_layout.MaxX() + m_drawSettings.NodeSize() + 
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
  dc.SetUserScale(m_zoom, m_zoom);
    
  if (m_parent->Cursor()) {
    if (!m_layout.GetNodeEntry(m_parent->Cursor())) {
      m_parent->SetCursor(m_efg.RootNode());
    }
    
    UpdateCursor();
  }
    
  dc.BeginDrawing();
  dc.Clear();
  m_layout.Render(dc);
  dc.EndDrawing();
}

void TreeWindow::EnsureCursorVisible(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(m_parent->Cursor()); 
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
  CalcScrolledPosition((int) (entry->X() * m_zoom + m_drawSettings.NodeSize() +
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
  if (m_parent->Cursor()) {
    NodeEntry *entry = m_layout.GetNodeEntry(m_parent->Cursor()); 
    if (!entry) {
      m_parent->SetCursor(m_efg.RootNode());
      entry = m_layout.GetNodeEntry(m_parent->Cursor());
    }
    
    UpdateCursor();
    EnsureCursorVisible();
  }
  Refresh();
}

void TreeWindow::UpdateCursor(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(m_parent->Cursor());

  if (entry) {
    entry->SetCursor(true);
  }
}

gText TreeWindow::OutcomeAsString(const Node *n) const
{
  Efg::Outcome outcome = n->Game()->GetOutcome(n);
  if (!outcome.IsNull()) {
    const gArray<gNumber> &v = n->Game()->Payoff(outcome);
    gText tmp = "(";

    for (int i = v.First(); i <= v.Last(); i++) {
      if (i != 1) 
	tmp += ",";
      /*      
      if (DrawSettings().ColorCodedOutcomes())
	tmp += ("\\C{"+ToText(DrawSettings().GetPlayerColor(i))+"}");
      */
      tmp += ToText(v[i], DrawSettings().NumDecimals());
    }
    /*
    if (DrawSettings().ColorCodedOutcomes()) 
      tmp += ("\\C{"+ToText(WX_COLOR_LIST_LENGTH-1)+"}");
    */
    tmp += ")";
        
    return tmp;
  }
  else
    return "";
}

#include "bitmaps/copy.xpm"
#include "bitmaps/move.xpm"

void TreeWindow::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    if (!m_dragImage) {
      int x, y;
      CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
      x = (int) ((float) x / m_zoom);
      y = (int) ((float) y / m_zoom);

      Node *node = m_layout.NodeHitTest(x, y);
    
      if (node && m_efg.NumChildren(node) > 0) {
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

    Node *node = m_layout.NodeHitTest(x, y);
    if (node && node->NumChildren() == 0) {
      try {
	if (m_dragMode == dragCOPY) {
	  m_efg.CopyTree(m_dragSource, node);
	  m_parent->OnTreeChanged(true, false);
	}
	else if (m_dragMode == dragMOVE) {
	  m_efg.MoveTree(m_dragSource, node);
	  m_parent->OnTreeChanged(true, false);
	  RefreshTree();
	}
	else if (m_dragMode == dragOUTCOME) { 
	  node->Game()->SetOutcome(node,
				   m_dragSource->Game()->GetOutcome(m_dragSource));
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

  Node *node = m_layout.NodeHitTest(x, y);
  m_parent->SetCursor(node);
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

  Node *node = m_layout.NodeHitTest(x, y);
  if (node) {
    m_parent->SetCursor(node);
    Refresh();
    wxCommandEvent event;
    m_parent->OnEditNode(event);
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

  Node *node = m_layout.NodeHitTest(x, y);
  if (node) {
    m_parent->SetCursor(node);
    Refresh();
    PopupMenu(m_nodeMenu, p_event.GetX(), p_event.GetY());
  }
  else {
    // If right-click doesn't hit anything, display generic game menu
    m_parent->SetCursor(0);
    Refresh();
    PopupMenu(m_gameMenu, p_event.GetX(), p_event.GetY());
  }
}

void TreeWindow::SupportChanged(void)
{
  if (!m_layout.GetNodeEntry(m_parent->Cursor())) {
    m_parent->SetCursor(0);
  }
  RefreshLayout();
  Refresh();
}

void TreeWindow::SetCursorPosition(Node *p_cursor)
{
  if (m_parent->Cursor()) {
    m_layout.GetNodeEntry(m_parent->Cursor())->SetCursor(false);
    m_layout.GetNodeEntry(m_parent->Cursor())->SetSelected(false);
  }
}

void TreeWindow::UpdateMenus(void)
{
  m_nodeMenu->Enable(efgmenuEDIT_INSERT, (m_parent->Cursor()) ? true : false);
  m_nodeMenu->Enable(efgmenuEDIT_REVEAL,
		     (m_parent->Cursor() && m_parent->Cursor()->GetInfoset()));
  m_nodeMenu->Enable(efgmenuEDIT_MOVE, 
		     (m_parent->Cursor() && m_parent->Cursor()->GetInfoset()));
}

//-----------------------------------------------------------------------
//                     SUBGAME MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//----------------------
// Subgames->Mark All
//----------------------

void TreeWindow::SubgameMarkAll(void)
{
  gList<Node *> subgame_roots;
  LegalSubgameRoots(m_efg, subgame_roots);
  m_efg.MarkSubgames(subgame_roots);
  RefreshLayout();
}

void TreeWindow::SubgameMark(void)
{
  if (m_parent->Cursor()->GetSubgameRoot() == m_parent->Cursor()) {
    // ignore silently
    return;
  }

  if (!m_efg.IsLegalSubgame(m_parent->Cursor())) {
    wxMessageBox("This node is not a root of a valid subgame"); 
    return;
  }

  m_efg.MarkSubgame(m_parent->Cursor());
  RefreshLayout();
}

void TreeWindow::SubgameUnmark(void)
{
  if (m_parent->Cursor()->GetSubgameRoot() != m_parent->Cursor() ||
      m_parent->Cursor()->GetSubgameRoot() == m_efg.RootNode())
    return;
    
  m_efg.UnmarkSubgame(m_parent->Cursor());
  RefreshLayout();
}

void TreeWindow::SubgameUnmarkAll(void)
{
  m_efg.UnmarkSubgames(m_efg.RootNode());
  RefreshLayout();
}

//-----------------------------------------------------------------------
//                     DISPLAY MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void TreeWindow::OnSize(wxSizeEvent &p_event)
{
  if (m_layout.MaxX() == 0 || m_layout.MaxY() == 0) {
    m_layout.Layout(*m_parent->GetSupport());
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

template class gList<NodeEntry *>;

