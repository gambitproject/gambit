//
// FILE: treewin.cc -- Implementation of TreeWindow class
//
// $Id$
//

#include "wx/wx.h"
#include "wx/dcps.h"
#include "wx/dragimag.h"
#include "wxmisc.h"

#include "base/gmisc.h"
#include "math/math.h"
#include "efg.h"
#include "legendc.h"
#include "treewin.h"
#include "twflash.h"
#include "efgshow.h"
#include "treedrag.h"
#include "legend.h"

#include "efgutils.h"
#include "base/glist.imp"

extern int INFOSET_SPACING;
extern int SUBGAME_LARGE_ICON_SIZE;
extern int SUBGAME_SMALL_ICON_SIZE;
extern int SUBGAME_PICK_SIZE;
extern void DrawSubgamePickIcon(wxDC &, const NodeEntry &);


#define DELTA                   8
#define MAX_TW                  60
#define MAX_TH                  20

#define MIN_WINDOW_WIDTH   600  // at least 12 buttons
#define MIN_WINDOW_HEIGHT  300

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
    ef(*p_efgShow->Game()), m_parent(p_efgShow),
    m_layout(ef, this), m_dragImage(0), m_zoom(1.0)
{
  // Set the cursor to the root node
  m_cursor = ef.RootNode();
  // Make sure that Chance player has a name
  ef.GetChance()->SetName("Chance");

  // Create the flasher to flash the cursor or just a steady cursor
  flasher = new TreeNodeCursor(this);
  // Create provision for merging isets by drag'n dropping
  iset_drag = new IsetDragger(this, ef);
  // Create provision for adding/creating braches by drag'n dropping
  branch_drag = new BranchDragger(this, ef);
  // No node has been marked yet--mark_node is invalid
  mark_node = 0; 
  // No isets are being hilighted
  hilight_infoset = 0; hilight_infoset1 = 0;

  SetBackgroundColour(*wxWHITE);
  MakeMenus();
  RefreshLayout();
  EnsureCursorVisible();
}

TreeWindow::~TreeWindow()
{
  delete iset_drag;
  delete branch_drag;
  Show(false);
}

void TreeWindow::MakeMenus(void)
{
  m_editMenu = new wxMenu;

  wxMenu *nodeMenu  = new wxMenu;
  nodeMenu->Append(efgmenuEDIT_NODE_ADD, "&Add Move", "Add a move");
  nodeMenu->Append(efgmenuEDIT_NODE_DELETE, "&Delete Move", "Remove move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_INSERT, "&Insert Move", "Insert move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_LABEL,     "&Label",     "Label cursor node");
  nodeMenu->AppendSeparator();
  nodeMenu->Append(efgmenuEDIT_NODE_SET_MARK,  "Set &Mark",  "Mark cursor node");
  nodeMenu->Append(efgmenuEDIT_NODE_GOTO_MARK, "Go&to Mark", "Goto marked node");

  wxMenu *action_menu = new wxMenu;
  action_menu->Append(efgmenuEDIT_ACTION_DELETE, "&Delete", "Delete an action from cursor information set");
  action_menu->Append(efgmenuEDIT_ACTION_INSERT, "&Insert", "Insert an action in the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_APPEND, "&Append", "Append an action to the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_LABEL, "&Label", "Label the actions of the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_PROBS, "&Probabilities", "Set chance probabilities for the cursor's information set");

  wxMenu *infoset_menu = new wxMenu;
  infoset_menu->Append(efgmenuEDIT_INFOSET_MERGE,  "&Merge",  "Merge cursor iset w/ marked");
  infoset_menu->Append(efgmenuEDIT_INFOSET_BREAK,  "&Break",  "Make cursor a new iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_SPLIT,  "&Split",  "Split iset at cursor");
  infoset_menu->Append(efgmenuEDIT_INFOSET_JOIN,   "&Join",   "Join cursor to marked iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_LABEL,  "&Label",  "Label cursor iset & actions");
  infoset_menu->Append(efgmenuEDIT_INFOSET_PLAYER, "&Player", "Change player of cursor iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_REVEAL, "&Reveal", "Reveal infoset to players");

  wxMenu *outcome_menu = new wxMenu;
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_NEW, "&New",
		       "Create a new outcome");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_DELETE, "Dele&te",
		       "Delete an outcome");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_ATTACH, "&Attach",
		       "Attach an outcome to the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_DETACH, "&Detach",
		       "Detach the outcome from the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_LABEL, "&Label",
		       "Label the outcome at the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_PAYOFFS, "&Payoffs",
		       "Set the payoffs for the outcome at the cursor");

  wxMenu *tree_menu = new wxMenu;
  tree_menu->Append(efgmenuEDIT_TREE_COPY, "&Copy",
		    "Copy tree from marked node");
  tree_menu->Append(efgmenuEDIT_TREE_MOVE, "&Move",
		    "Move tree from marked node");
  tree_menu->Append(efgmenuEDIT_TREE_DELETE, "&Delete",
		    "Delete recursively from cursor");
  tree_menu->Append(efgmenuEDIT_TREE_LABEL, "&Label",
		    "Set the game label");
  tree_menu->Append(efgmenuEDIT_TREE_PLAYERS, "&Players",
		    "Edit/View players");
  tree_menu->Append(efgmenuEDIT_TREE_INFOSETS, "&Infosets",
		    "Edit/View infosets");

  m_editMenu->Append(efgmenuEDIT_NODE, "&Node", nodeMenu, "Edit the node");
  m_editMenu->Append(efgmenuEDIT_ACTIONS, "&Actions", action_menu, 
		    "Edit actions");
  m_editMenu->Append(efgmenuEDIT_INFOSET, "&Infoset", infoset_menu,
		    "Edit infosets");
  m_editMenu->Append(efgmenuEDIT_OUTCOMES, "&Outcomes", outcome_menu,
		    "Edit outcomes and payoffs");
  m_editMenu->Append(efgmenuEDIT_TREE, "&Tree", tree_menu,
		    "Edit the tree");
}

gText TreeWindow::AsString(TypedSolnValues what, const Node *n, int br) const
{
  return m_parent->AsString(what, n, br);
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
  if (!p_event.ShiftDown()) {
    bool c = false;   // set to true if cursor position has changed
    switch (p_event.KeyCode()) {
    case WXK_LEFT:
      if (Cursor()->GetParent()) {
	SetCursorPosition(m_layout.GetValidParent(Cursor())->n);
	c = true;
      }
      break;
    case WXK_RIGHT:
      if (m_layout.GetValidChild(Cursor())) {
	SetCursorPosition(m_layout.GetValidChild(Cursor())->n);
	c = true;
      }
      break;
    case WXK_UP: {
      Node *prior = ((!p_event.ControlDown()) ? 
		     m_layout.PriorSameLevel(Cursor()) :
		     PriorSameIset(Cursor()));
      if (prior) {
	SetCursorPosition(prior);
	c = true;
      }
      break;
    }
    case WXK_DOWN: {
      Node *next = ((!p_event.ControlDown()) ?
		    m_layout.NextSameLevel(Cursor()) :
		    NextSameIset(Cursor()));
      if (next) {
	SetCursorPosition(next);
	c = true;
      }
      break;
    }
    case WXK_SPACE:
      c = true;
      break;
    }
        
    if (c) { 
      ProcessCursor(); // cursor moved
    }

    // Implement the behavior that when control+cursor key is pressed, the
    // nodes belonging to the iset are hilighted.
    if (c && p_event.ControlDown()) {
      if (hilight_infoset1 != Cursor()->GetInfoset()) {
	hilight_infoset1 = Cursor()->GetInfoset();
	Refresh();
      }
    }
    if (!p_event.ControlDown() && hilight_infoset1) {
      hilight_infoset1 = 0;
      Refresh();
    }
  }
  else {
    p_event.Skip();
  }
}

//---------------------------------------------------------------------
//                   TreeWindow: Drawing functions
//---------------------------------------------------------------------

void TreeWindow::RefreshLayout(void)
{
  m_layout.Layout(*m_parent->GetSupport());
  AdjustScrollbarSteps();
}

void TreeWindow::AdjustScrollbarSteps(void)
{
  int width, height;
  GetClientSize(&width, &height);

  if (width < m_layout.MaxX() * m_zoom || height < m_layout.MaxY() * m_zoom) {
    int scrollX, scrollY;
    GetViewStart(&scrollX, &scrollY);

    SetScrollbars(50, 50,
		  m_layout.MaxX() * m_zoom / 50 + 1,
		  m_layout.MaxY() * m_zoom / 50 + 1,
		  scrollX, scrollY);
  }
}

void TreeWindow::FitZoom(void)
{
  int width, height;
  GetClientSize(&width, &height);
    
  double zoomx = (double) width / (double) m_layout.MaxX();
  double zoomy = (double) height / (double) m_layout.MaxY();
    
  zoomx = gmin(zoomx, 1.0); 
  zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
  m_zoom = gmin(zoomx, zoomy);
}

void TreeWindow::SetZoom(float p_zoom)
{
  m_zoom = p_zoom;
  AdjustScrollbarSteps();
  Refresh();
}

void TreeWindow::OnDraw(wxDC &dc)
{
  dc.SetUserScale(m_zoom, m_zoom);
    
  if (Cursor()) {
    if (!m_layout.GetNodeEntry(Cursor())) {
      SetCursorPosition(ef.RootNode());
    }
    
    UpdateCursor();
  }
    
  dc.BeginDrawing();
  dc.Clear();
  m_layout.Render(dc);
  flasher->Flash(dc);
  dc.EndDrawing();
}

void TreeWindow::EnsureCursorVisible(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(Cursor()); 
  int xScroll, yScroll;
  GetViewStart(&xScroll, &yScroll);
  int width, height;
  GetClientSize(&width, &height);
  width = (int) (width / m_zoom);
  height = (int) (height / m_zoom);
  int xSteps = (int) (m_layout.MaxX() * m_zoom / 50) + 1;
  int ySteps = (int) (m_layout.MaxY() * m_zoom / 50) + 1;

  if (entry->x < xScroll * 50) {
    xScroll = entry->x / 50 - 1;
  }
  if (entry->x + draw_settings.NodeLength() > xScroll * 50 + width) {
    xScroll = ((entry->x + draw_settings.NodeLength()) / 50 +
	       (width / 2) / 50);
  }
  if (xScroll < 0) {
    xScroll = 0;
  }
  if (xScroll > xSteps) {
    xScroll = xSteps;
  }

  if (entry->y - 10 < yScroll * 50) {
    yScroll = (entry->y - 10) / 50 - 1;
  }
  if (entry->y + 10 > yScroll * 50 + height) {
    yScroll = (entry->y + 10) / 50 - height / 50;
  }
  if (yScroll < 0) {
    yScroll = 0;
  }
  if (yScroll > ySteps) {
    yScroll = ySteps;
  }

  Scroll(xScroll, yScroll);
}

void TreeWindow::ProcessCursor(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(Cursor()); 
  if (!entry) {
    SetCursorPosition(ef.RootNode());
    entry = m_layout.GetNodeEntry(Cursor());
  }
    
  UpdateCursor();
  EnsureCursorVisible();
  m_parent->OnSelectedMoved(Cursor());
}

void TreeWindow::UpdateCursor(void)
{
  NodeEntry *entry = m_layout.GetNodeEntry(Cursor());

  if (!entry) {
    return;
  }

  if (entry->n->GetSubgameRoot() == entry->n && !entry->expanded) {
    flasher->SetFlashNode(entry->x + DrawSettings().NodeLength() +
			  entry->nums*INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE,
			  entry->y,
			  entry->x + DrawSettings().NodeLength() +
			  entry->nums*INFOSET_SPACING,
			  entry->y, subgameCursor);
  }
  else {
    flasher->SetFlashNode(entry->x + entry->nums*INFOSET_SPACING, entry->y,
			  entry->x + DrawSettings().NodeLength() + 
			  entry->nums*INFOSET_SPACING - 8,
			  entry->y, nodeCursor);
  }

  wxClientDC dc(this);
  PrepareDC(dc);
  dc.SetUserScale(m_zoom, m_zoom);
  flasher->Flash(dc);
}

gText TreeWindow::OutcomeAsString(const Node *n, bool &/*hilight*/) const
{
  if (n->GetOutcome()) {
    EFOutcome *tv = n->GetOutcome();
    const gArray<gNumber> &v = n->Game()->Payoff(tv);
    gText tmp = "(";

    for (int i = v.First(); i <= v.Last(); i++) {
      if (i != 1) 
	tmp += ",";
      /*      
      if (DrawSettings().ColorCodedOutcomes())
	tmp += ("\\C{"+ToText(DrawSettings().GetPlayerColor(i))+"}");
      */
      tmp += ToText(v[i], NumDecimals());
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

void TreeWindow::OnMouseMotion(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    if (!m_dragImage) {
      int x, y;
      CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
      x = (int) ((float) x / m_zoom);
      y = (int) ((float) y / m_zoom);

      Node *node = m_layout.NodeHitTest(x, y);
    
      if (node && ef.NumChildren(node) > 0) {
	wxPoint hotSpot(p_event.GetPosition().x, p_event.GetPosition().y);
#ifdef __WXMSW__
	if (p_event.ControlDown()) {
	  m_dragImage = new wxDragImage(wxBitmap("COPY_BITMAP"),
					wxCursor(wxCURSOR_HAND),
					hotSpot);
	  m_dragMode = dragCOPY;
	}
	else {
	  m_dragImage = new wxDragImage(wxBitmap("MOVE_BITMAP"),
					wxCursor(wxCURSOR_HAND),
					hotSpot);
	  m_dragMode = dragMOVE;
	}
#else
#include "bitmaps/copy.xpm"
#include "bitmaps/move.xpm"
	if (p_event.ControlDown()) {
	  m_dragImage = new wxDragImage(wxBitmap(copy_xpm),
					wxCursor(wxCURSOR_HAND), hotSpot);
	  m_dragMode = dragCOPY;
	}
	else {
	  m_dragImage = new wxDragImage(wxBitmap(move_xpm),
					wxCursor(wxCURSOR_HAND), hotSpot);
	  m_dragMode = dragMOVE;
	}
#endif  // __WXMSW__
	m_dragImage->BeginDrag(wxPoint(0, 0), this);
	m_dragImage->Move(p_event.GetPosition());
	m_dragImage->Show();
	m_dragSource = node;
	return;
      }

      node = m_layout.NodeRightHitTest(x, y);
      if (node && node->GetOutcome()) {
	wxPoint hotSpot(p_event.GetPosition().x, p_event.GetPosition().y);
#ifdef __WXMSW__
	m_dragImage = new wxDragImage(wxBitmap("PAYOFF_BITMAP"),
				      wxCursor(wxCURSOR_HAND), hotSpot);
#else
#include "bitmaps/payoff.xpm"
	m_dragImage = new wxDragImage(wxBitmap(payoff_xpm),
				      wxCursor(wxCURSOR_HAND), hotSpot);
#endif  // __WXMSW__
	m_dragMode = dragOUTCOME;
	m_dragImage->BeginDrag(wxPoint(0, 0), this);
	m_dragImage->Move(p_event.GetPosition());
	m_dragImage->Show();
	m_dragSource = node;
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
    if (node) {
      try {
	if (m_dragMode == dragCOPY) {
	  ef.CopyTree(m_dragSource, node);
	}
	else if (m_dragMode == dragMOVE) {
	  ef.MoveTree(m_dragSource, node);
	}
	else if (m_dragMode == dragOUTCOME) { 
	  node->SetOutcome(m_dragSource->GetOutcome());
	}
      }
      catch (gException &ex) {
	guiExceptionDialog(ex.Description(), this);
      }
      Refresh();
    }
  }

  // Check all the draggers.  Note that they are mutually exclusive
  if (!branch_drag->Dragging()) {
    if (iset_drag->OnEvent(p_event) != DRAG_NONE) return;
  }

  if (!iset_drag->Dragging()) {
    if (branch_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
}    

void TreeWindow::OnLeftClick(wxMouseEvent &p_event)
{
  if (ProcessShift(p_event))  {
    return;
  }   
 
  // Check all the draggers.  Note that they are mutually exclusive
  if (!branch_drag->Dragging()) {
    if (iset_drag->OnEvent(p_event) != DRAG_NONE) return;
  }

  if (!iset_drag->Dragging()) {
    if (branch_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
    
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  Node *node = m_layout.NodeHitTest(x, y);
  if (node) {
    SetCursorPosition(node);
    Refresh();
    ProcessCursor();
  }
}

void TreeWindow::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  Node *node = m_layout.NodeHitTest(x, y);
  if (node) {
    SetCursorPosition(node);
    if (Cursor()->GetInfoset()) {
      m_parent->HilightInfoset(Cursor()->GetPlayer()->GetNumber(),
			       Cursor()->GetInfoset()->GetNumber(), 1);
    }
    Refresh();
    return;
  }

  node = m_layout.SubgameHitTest(x, y);
  if (node) {
    SetCursorPosition(node);
    for (int i = 1; i <= m_layout.SubgameList().Length(); i++) {
      if (m_layout.SubgameList()[i].root == Cursor()) {
	m_layout.SubgameList()[i].expanded = !m_layout.SubgameList()[i].expanded; 
	RefreshLayout();
      }
    }
    Refresh();
    return;
  }

  node = m_layout.NodeAboveHitTest(x, y);
  if (node) {
    // Action for clicking on node label -- currently undefined
    return;
  }

  node = m_layout.NodeBelowHitTest(x, y);
  if (node) {
    // Action for clicking on node label -- currently undefined
    return;
  }

  node = m_layout.NodeRightHitTest(x, y);
  if (node) {
    SetCursorPosition(node);
    // Action for clicking on right label -- currently undefined
  }

  node = m_layout.BranchAboveHitTest(x, y);
  if (node) {
    SetCursorPosition(node->GetParent());
    // Action for clicking on branch label -- currently undefined
    return;
  }

  node = m_layout.BranchBelowHitTest(x, y);
  if (node) {
    SetCursorPosition(node->GetParent());
    // Action for clicking on branch label -- currently undefined
    return;
  }
}

void TreeWindow::OnRightClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = (int) ((float) x / m_zoom);
  y = (int) ((float) y / m_zoom);

  int x_start, y_start;
  ViewStart(&x_start, &y_start);
  wxClientDC dc(this);
  PrepareDC(dc);
  dc.SetUserScale(m_zoom, m_zoom);
  PopupMenu(m_editMenu, dc.LogicalToDeviceX(x-x_start*PIXELS_PER_SCROLL),
	    dc.LogicalToDeviceY(y-y_start*PIXELS_PER_SCROLL));
}

bool TreeWindow::ProcessShift(wxMouseEvent &ev)
{
  if (!ev.ShiftDown()) {
    return false;
  }

  long x, y;
  ev.GetPosition(&x, &y);

  Node *node = m_layout.NodeHitTest(x, y);
  if (node) {
    ef.DeleteTree(node);
    ef.DeleteEmptyInfosets();
    Refresh();
    return true;
  }

  node = m_layout.NodeRightHitTest(x, y);
  if (node && node->GetOutcome()) {
    node->SetOutcome(0);
    outcomes_changed = true;
    Refresh();
    return true;
  }

  node = m_layout.BranchHitTest(x, y);
  if (node) {
    ef.DeleteAction(node->GetParent()->GetInfoset(),
		    LastAction(ef, node));
    Refresh();
    return true;
  }

  node = m_layout.InfosetHitTest(x, y);
  if (node) {
    Infoset *iset = ef.SplitInfoset(node);
    iset->SetName("Infoset" + ToText(iset->GetPlayer()->NumInfosets()));
    Refresh();
    return true;
  }

  return false;
}

void TreeWindow::HilightInfoset(int pl, int iset)
{
  hilight_infoset = 0;

  if (pl >= 1 && pl <= ef.NumPlayers()) {
    EFPlayer *p = ef.Players()[pl];
    if (iset >= 1 && iset <= p->NumInfosets())
      hilight_infoset = p->Infosets()[iset];
  }

  Refresh();
}

//
// SupportChanged -- must be called by parent every time the cur_sup
// changes.  Note that since it is a reference, it needs not be passed here.
//
void TreeWindow::SupportChanged(void)
{
  // Check if the cursor is still valid
  NodeEntry *ne = m_layout.GetNodeEntry(Cursor());
  if (ne->child_number) {
    if (!m_parent->GetSupport()->Find(Cursor()->GetInfoset()->Actions()[ne->child_number]))
      SetCursorPosition(ef.RootNode());
  }

  RefreshLayout();
  Refresh();
}

void TreeWindow::SetSubgamePickNode(Node *n)
{
  if (n) {
    // save the actual cursor, and fake a cursor movement to ensure
    // that the node is visible
    Node *cur_cursor = Cursor();
    SetCursorPosition((Node *) n);
    ProcessCursor();
    NodeEntry *ne = m_layout.GetNodeEntry(n);
    wxClientDC dc(this);
    PrepareDC(dc);
    dc.SetUserScale(m_zoom, m_zoom);
    DrawSubgamePickIcon(dc, *ne);
    SetCursorPosition(cur_cursor);
  }
  subgame_node = n;
}

Node *TreeWindow::GotObject(long &x, long &y, int what)
{
#ifdef NOT_PORTED_YET
  for (int i = 1; i <= m_layout.Length(); i++) {
    NodeEntry *entry = m_layout[i];
        
    if (what == DRAG_NODE_START) // check if clicked a non terminal node
      if (entry->n->Game()->NumChildren(entry->n) != 0)
	if(x > entry->x+entry->nums*INFOSET_SPACING &&
	   x < entry->x+draw_settings.NodeLength()+
	   entry->nums*INFOSET_SPACING-10 &&
	   y > entry->y-DELTA && y < entry->y+DELTA) 
	  return (Node *)entry->n;
        
    if (what == DRAG_NODE_END) // check if clicked on a terminal node
      if (entry->n->Game()->NumChildren(entry->n) == 0)
	if(x > entry->x+entry->nums*INFOSET_SPACING &&
	   x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
	   y > entry->y-DELTA && y < entry->y+DELTA)
	  return (Node *)entry->n;
    
    if (what == DRAG_OUTCOME_START) // check if clicked on a terminal node
      if (entry->n->Game()->NumChildren(entry->n) == 0 && entry->n->GetOutcome())
	if(x > entry->x+entry->nums*INFOSET_SPACING+draw_settings.NodeLength() &&
	   x < entry->x+draw_settings.NodeLength()+
	   entry->nums*INFOSET_SPACING + draw_settings.OutcomeLength()&&
	   y > entry->y-DELTA && y < entry->y+DELTA)
	  return (Node *)entry->n;
    
    if (what == DRAG_OUTCOME_END) // check if clicked on any valid node
      if(x > entry->x+entry->nums*INFOSET_SPACING && 
	 x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
	 y > entry->y-DELTA && y < entry->y+DELTA)
	return (Node *)entry->n;
    
    if (what == DRAG_ISET_START || what == DRAG_ISET_END)
      // check if clicked on a non terminal node
      if (entry->n->Game()->NumChildren(entry->n) != 0)
	if(x > entry->x+entry->num*INFOSET_SPACING-4 && 
	   x < entry->x+entry->num*INFOSET_SPACING+4 &&
	   y > entry->y-4 && y < entry->y+4) {
	  x = entry->x+entry->num*INFOSET_SPACING;
	  y = entry->y;
	  return (Node *)entry->n;
	}
    
    if (what == DRAG_BRANCH_START)
      // check if clicked on the very end of a node
      if (x > entry->x+draw_settings.NodeLength()+
	  entry->nums*INFOSET_SPACING-4 &&
	  x < entry->x+draw_settings.NodeLength()+
	  entry->nums*INFOSET_SPACING+4 &&
	  y > entry->y-4 && y < entry->y+4) {
	x = entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING;
	y = entry->y;
	return (Node *)entry->n;
      }
        
    if (what == DRAG_BRANCH_END) {
      // check if released in a valid position
      NodeEntry *start_entry = m_layout.GetNodeEntry(branch_drag->StartNode());
      int xs = start_entry->x+draw_settings.NodeLength()+
	draw_settings.ForkLength()+start_entry->nums*INFOSET_SPACING;
      if (x > xs && x < xs+draw_settings.BranchLength() &&
	  y < start_entry->y+(start_entry->n->Game()->NumChildren(start_entry->n)+1)*draw_settings.YSpacing() &&
	  y > start_entry->y-(start_entry->n->Game()->NumChildren(start_entry->n)+1)*draw_settings.YSpacing()) {
	// figure out at what branch # the mouse was released
	int br = 1;
	NodeEntry *child_entry, *child_entry1;
	for (int ii = 1; ii <= start_entry->n->Game()->NumChildren(start_entry->n)-1; ii++) {
	  child_entry = m_layout.GetNodeEntry(start_entry->n->GetChild(ii));
	  if (ii == 1) 
	    if (y < child_entry->y) {
	      br = 1;
	      break;
	    }
	  child_entry1 = m_layout.GetNodeEntry(start_entry->n->GetChild(ii+1));
	  if (y > child_entry->y && y < child_entry1->y) {
	    br = ii+1;
	    break;
	  }
	  if (ii == start_entry->n->Game()->NumChildren(start_entry->n)-1 && y > child_entry1->y) {
	    br = start_entry->n->Game()->NumChildren(start_entry->n)+1;
	    break;
	  }
	}
	x = br;
	return (Node *)start_entry->n;
      }
      else
	return 0;
    }
  }
#endif // NOT_PORTED_YET
  return 0;
}

void TreeWindow::SetCursorPosition(Node *p_cursor)
{
  m_cursor = p_cursor;
  m_parent->UpdateMenus();
}

void TreeWindow::UpdateMenus(void)
{
  m_editMenu->Enable(efgmenuEDIT_NODE_ADD,
		   (ef.NumChildren(m_cursor) > 0) ? false : true);
  m_editMenu->Enable(efgmenuEDIT_NODE_DELETE,
		   (ef.NumChildren(m_cursor) > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_MERGE,
		     (mark_node && mark_node->GetInfoset() &&
		      m_cursor->GetInfoset() &&
		      mark_node->GetSubgameRoot() == m_cursor->GetSubgameRoot() &&
		     mark_node->GetPlayer() == m_cursor->GetPlayer()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_BREAK,
		     (m_cursor->GetInfoset()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_SPLIT,
		     (m_cursor->GetInfoset()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_JOIN,
		     (mark_node && mark_node->GetInfoset() &&
		      m_cursor->GetInfoset() &&
		      mark_node->GetSubgameRoot() == m_cursor->GetSubgameRoot()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_LABEL,
		    (m_cursor->GetInfoset()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_PLAYER,
		    (m_cursor->GetInfoset()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_INFOSET_REVEAL,
		    (m_cursor->GetInfoset()) ? true : false);

  m_editMenu->Enable(efgmenuEDIT_ACTION_LABEL,
		    (m_cursor->GetInfoset() &&
		     m_cursor->GetInfoset()->NumActions() > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_ACTION_INSERT,
		    (ef.NumChildren(m_cursor) > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_ACTION_APPEND,
		    (ef.NumChildren(m_cursor) > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_ACTION_DELETE,
		    (ef.NumChildren(m_cursor) > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_ACTION_PROBS,
		    (m_cursor->GetInfoset() &&
		     m_cursor->GetPlayer()->IsChance()) ? true : false);

  m_editMenu->Enable(efgmenuEDIT_TREE_DELETE,
		    (ef.NumChildren(m_cursor) > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_TREE_COPY,
		    (mark_node &&
		     m_cursor->GetSubgameRoot() == mark_node->GetSubgameRoot()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_TREE_MOVE,
		    (mark_node &&
		     m_cursor->GetSubgameRoot() == mark_node->GetSubgameRoot()) ? true : false);

  m_editMenu->Enable(efgmenuEDIT_OUTCOMES_ATTACH,
		    (ef.NumOutcomes() > 0) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_OUTCOMES_DETACH,
		    (m_cursor->GetOutcome()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_OUTCOMES_LABEL,
		    (m_cursor->GetOutcome()) ? true : false);
  m_editMenu->Enable(efgmenuEDIT_OUTCOMES_DELETE,
		    (ef.NumOutcomes() > 0) ? true : false);
}

//-----------------------------------------------------------------------
//                    NODE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//------------------------
// Edit->Node->Set Mark
//------------------------

void TreeWindow::node_set_mark(void)
{
  if (mark_node != Cursor())
    mark_node = Cursor();
  else
    mark_node = 0;
  m_parent->UpdateMenus();
}

//-------------------------
// Edit->Node->Goto Mark
//-------------------------

void TreeWindow::node_goto_mark(void)
{
  if (mark_node) {
    SetCursorPosition(mark_node);
    ProcessCursor();
  }
}

//-----------------------------------------------------------------------
//                     SUBGAME MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//----------------------
// Subgames->Mark All
//----------------------

void TreeWindow::SubgameMarkAll(void)
{
  m_layout.SubgameList().Flush();
  m_layout.SubgameList().Append(SubgameEntry(ef.RootNode()));
  gList<Node *> subgame_roots;
  LegalSubgameRoots(ef, subgame_roots);
  ef.MarkSubgames(subgame_roots);

  for (int i = 1; i <= subgame_roots.Length(); i++) {
    if (subgame_roots[i] != ef.RootNode())
      m_layout.SubgameList().Append(SubgameEntry(subgame_roots[i], true));
  }
  
  RefreshLayout();
}

void TreeWindow::SubgameMark(void)
{
  if (Cursor()->GetSubgameRoot() == Cursor()) {
    // ignore silently
    return;
  }

  if (!ef.IsLegalSubgame(Cursor())) {
    wxMessageBox("This node is not a root of a valid subgame"); 
    return;
  }

  ef.MarkSubgame(Cursor());
  m_layout.SubgameList().Append(SubgameEntry(Cursor(), true)); // collapse
  RefreshLayout();
}

void TreeWindow::SubgameUnmark(void)
{
  if (Cursor()->GetSubgameRoot() != Cursor() ||
      Cursor()->GetSubgameRoot() == ef.RootNode())
    return;
    
  ef.UnmarkSubgame(Cursor());

  for (int i = 1; i <= m_layout.SubgameList().Length(); i++) {
    if (m_layout.SubgameList()[i].root == Cursor())
      m_layout.SubgameList().Remove(i);
  }

  RefreshLayout();
}

void TreeWindow::SubgameUnmarkAll(void)
{
  ef.UnmarkSubgames(ef.RootNode());
  m_layout.SubgameList().Flush();
  m_layout.SubgameList().Append(SubgameEntry(ef.RootNode()));
  RefreshLayout();
}

void TreeWindow::SubgameCollapse(void)
{
  for (int i = 1; i <= m_layout.SubgameList().Length(); i++) {
    if (m_layout.SubgameList()[i].root == Cursor()) {
      m_layout.SubgameList()[i].expanded = false;
      RefreshLayout();
      return;
    }
  }
}

void TreeWindow::SubgameCollapseAll(void)
{
  for (int i = 1; i <= m_layout.SubgameList().Length(); i++)
    m_layout.SubgameList()[i].expanded = false;

  RefreshLayout();
}

void TreeWindow::SubgameExpand(void)
{
  for (int i = 1; i <= m_layout.SubgameList().Length(); i++) {
    if (m_layout.SubgameList()[i].root == Cursor()) {
      m_layout.SubgameList()[i].expanded = true;
      RefreshLayout();
      return;
    }
  }
}

void TreeWindow::SubgameExpandBranch(void)
{
  for (int i = 1; i <= m_layout.SubgameList().Length(); i++) {
    if (m_layout.SubgameList()[i].root == Cursor()) {
      for (int j = 1; j <= m_layout.SubgameList().Length(); j++) {
	if (ef.IsSuccessor(m_layout.SubgameList()[j].root, Cursor())) {
	  m_layout.SubgameList()[j].expanded = true;
	  RefreshLayout();
	}
      }

      return;
    }
  }
}

void TreeWindow::SubgameExpandAll(void)
{
  for (int i = 1; i <= m_layout.SubgameList().Length(); i++)
    m_layout.SubgameList()[i].expanded = true;
  
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
template class gList<SubgameEntry>;
gOutput &operator<<(gOutput &p_stream, const SubgameEntry &)
{ return p_stream; }

