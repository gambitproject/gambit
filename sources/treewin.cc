//
// FILE: treewin.cc -- Implementation of TreeWindow class
//
// $Id$
//

#include "wx/wx.h"
#include "wx/dcps.h"
#include "wxmisc.h"

#include "gmisc.h"
#include "efg.h"
#include "legendc.h"
#include "treewin.h"
#include "twflash.h"
#include "efgshow.h"
#include "treedrag.h"
#include "legend.h"

#include "efgutils.h"
#include "glist.imp"

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


wxFont   *outcome_font;
wxCursor *scissor_cursor;


//----------------------------------------------------------------------
//                      TreeWindow: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(TreeWindow, TreeRender)
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
  : TreeRender(p_parent, this), EfgClient(p_efgShow->Game()),
    ef(*p_efgShow->Game()), m_parent(p_efgShow),
    node_list(ef, this), m_zoom(1.0)
{
  // Set the cursor to the root node
  m_cursor = ef.RootNode();
  // Make sure that Chance player has a name
  ef.GetChance()->SetName("Chance");

  // Create the flasher to flash the cursor or just a steady cursor
  MakeFlasher();
  // Create provision for drag'n dropping nodes
  node_drag = new NodeDragger(this, ef);
  // Create provision for merging isets by drag'n dropping
  iset_drag = new IsetDragger(this, ef);
  // Create provision for adding/creating braches by drag'n dropping
  branch_drag = new BranchDragger(this, ef);
  // Create provision for copying/moving outcomes by drag'n'drop
  outcome_drag = new OutcomeDragger(this, ef);
  // No node has been marked yet--mark_node is invalid
  mark_node = 0; 
  // No isets are being hilighted
  hilight_infoset = 0; hilight_infoset1 = 0;
  // No zoom window or outcome dialog
  //  zoom_window = 0;
  outcome_font = wxTheFontList->FindOrCreateFont(9, wxSWISS, wxNORMAL, wxNORMAL);

#ifdef NOT_PORTED_YET
#ifdef wx_msw
  scissor_cursor = new wxCursor("SCISSORCUR");
#else
#include "bitmaps/scissor.xbm"
  scissor_cursor = new wxCursor(scissor_bits, scissor_width, scissor_height,
				-1, -1, scissor_bits);
#endif // wx_msw
#endif   // NOT_PORTED_YET

  OnTreeChanged(true, true);

  // Create a popup menu
  MakeMenus();
}

TreeWindow::~TreeWindow()
{
  delete node_drag;
  delete iset_drag;
  delete branch_drag;
  delete outcome_drag;
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
  // Accelerators:
  // Note that accelerators are provided for in the wxwin code but only for the
  // windows platform.  In order to make this more portable, accelerators for
  // this program are coded in the header file and processed in OnChar
    
  m_parent->CheckAccelerators(p_event);
    
  if (!p_event.ShiftDown()) {
    bool c = false;   // set to true if cursor position has changed
    switch (p_event.KeyCode()) {
    case WXK_LEFT:
      if (Cursor()->GetParent()) {
	SetCursorPosition(NodeList().GetValidParent(Cursor())->n);
	c = true;
      }
      break;
    case WXK_RIGHT:
      if (NodeList().GetValidChild(Cursor())) {
	SetCursorPosition(NodeList().GetValidChild(Cursor())->n);
	c = true;
      }
      break;
    case WXK_UP: {
      Node *prior = ((!p_event.ControlDown()) ? 
		     node_list.PriorSameLevel(Cursor()) :
		     PriorSameIset(Cursor()));
      if (prior) {
	SetCursorPosition(prior);
	c = true;
      }
      break;
    }
    case WXK_DOWN: {
      Node *next = ((!p_event.ControlDown()) ?
		    node_list.NextSameLevel(Cursor()) :
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
	Render();
      }
    }
    if (!p_event.ControlDown() && hilight_infoset1) {
      hilight_infoset1 = 0;
      Render();
    }
  }
  else {
    p_event.Skip();
  }
}

//
// OnEvent -- handle mouse events
// Currently we support selecting a node by clicking on it
//
void TreeWindow::OnEvent(wxMouseEvent& ev)
{
  // Implements the 'cutting' behavior
  if (ProcessShift(ev)) return;
    
  // Double clicking hilights iset and toggles subgames
  /*
  if (ev.LeftDClick() || (ev.LeftDown() && ev.ControlDown()))
    ProcessDClick(ev);

  // Clicking on a node will move the cursor there
  if (ev.LeftDown()) {
    const Node *old_cursor = Cursor();
    ProcessClick(ev);
    if (Cursor() != old_cursor) 
      ProcessCursor();
    SetFocus(); // click on the canvas to restore keyboard focus
  }
    
  // Right click implements a popup menu (edit), legend display
  if (ev.RightDown()) ProcessRClick(ev);
    
  // Right double click implements legend modification
  if (ev.RightDClick() || (ev.RightDown() && ev.ControlDown()))
    ProcessRDClick(ev);
  */
}

//---------------------------------------------------------------------
//                   TreeWindow: Drawing functions
//---------------------------------------------------------------------

void TreeWindow::OnTreeChanged(bool p_nodesChanged, bool p_infosetsChanged)
{
  m_needsLayout = p_nodesChanged || p_infosetsChanged;
}

void TreeWindow::ForceRecalc(void)
{
  m_needsLayout = true;
}

void TreeWindow::FitZoom(void)
{
  int width, height;
  GetClientSize(&width, &height);
    
  double zoomx = (double) width / (double) node_list.MaxX();
  double zoomy = (double) height / (double) node_list.MaxY();
    
  zoomx = gmin(zoomx, 1.0); 
  zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
  m_zoom = gmin(zoomx, zoomy);
}

void TreeWindow::Render(wxDC &dc)
{
  if (m_needsLayout) {
    node_list.Layout(*m_parent->GetSupport());
    FitZoom();
  }
  dc.SetUserScale(m_zoom, m_zoom);
  char *dc_type = dc.GetClassInfo()->GetClassName();
    
  if (strcmp(dc_type, "wxCanvasDC") == 0) { // if drawing to screen
    if (Cursor()) {
      if (!NodeList().GetNodeEntry(Cursor())) {
	SetCursorPosition(ef.RootNode());
      }
      
      UpdateCursor();
    }
    
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.Clear();

    dc.BeginDrawing();
  }
  else {
    flasher->SetFlashing(false);
  }
    
  TreeRender::Render(dc);

  if (strcmp(dc_type, "wxCanvasDC") != 0) {
    flasher->SetFlashing(true); 

  }
  else {
    dc.EndDrawing();
  }

  flasher->Flash();
}

void TreeWindow::ProcessCursor(void)
{
  NodeEntry *entry = NodeList().GetNodeEntry(Cursor()); 
  if (!entry) {
    SetCursorPosition(ef.RootNode());
    entry = NodeList().GetNodeEntry(Cursor());
  }
    
  UpdateCursor();
  m_parent->OnSelectedMoved(Cursor());
}

void TreeWindow::OnMouseMotion(wxMouseEvent &p_event)
{
  // Check all the draggers.  Note that they are mutually exclusive
  if (!iset_drag->Dragging() && !branch_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (node_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
    
  if (!node_drag->Dragging() && !branch_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (iset_drag->OnEvent(p_event) != DRAG_NONE) return;
  }

  if (!node_drag->Dragging() && !iset_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (branch_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
    
  if (!node_drag->Dragging() && !iset_drag->Dragging() &&
      !branch_drag->Dragging()) {
    if (outcome_drag->OnEvent(p_event, outcomes_changed) != DRAG_NONE) return;
  }
}    

void TreeWindow::OnLeftClick(wxMouseEvent &p_event)
{
  // Check all the draggers.  Note that they are mutually exclusive
  if (!iset_drag->Dragging() && !branch_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (node_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
    
  if (!node_drag->Dragging() && !branch_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (iset_drag->OnEvent(p_event) != DRAG_NONE) return;
  }

  if (!node_drag->Dragging() && !iset_drag->Dragging() &&
      !outcome_drag->Dragging()) {
    if (branch_drag->OnEvent(p_event) != DRAG_NONE) return;
  }
    
  if (!node_drag->Dragging() && !iset_drag->Dragging() &&
      !branch_drag->Dragging()) {
    if (outcome_drag->OnEvent(p_event, outcomes_changed) != DRAG_NONE) return;
  }
    
  int x = p_event.GetX(), y = p_event.GetY();

  Node *node = node_list.NodeHitTest(x, y, draw_settings.NodeLength());
  if (node) {
    SetCursorPosition(node);
    Render();
    ProcessCursor();
  }
}

void TreeWindow::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  long x, y;
  p_event.GetPosition(&x, &y);
  int id = -1;
  for (int i = 1; i <= node_list.Length(); i++) {
    NodeEntry *entry = node_list[i];
    // Check if double clicked on a node
    if (x > entry->x+entry->nums*INFOSET_SPACING &&
	x < entry->x+entry->nums*INFOSET_SPACING+
	draw_settings.NodeLength()-SUBGAME_LARGE_ICON_SIZE &&
	y > entry->y-DELTA && y < entry->y+DELTA) {
      SetCursorPosition((Node *) entry->n);
      if (Cursor()->GetInfoset()) // implement iset hilighting
	m_parent->HilightInfoset(Cursor()->GetPlayer()->GetNumber(),
				 Cursor()->GetInfoset()->GetNumber(), 1);
      Render();
      return;
    }
    // implement subgame toggle (different for collapsed and expanded)
    if (entry->n->GetSubgameRoot() == entry->n) {
      if (entry->expanded) {
	if (x > entry->x && x < entry->x+SUBGAME_SMALL_ICON_SIZE &&
	    y > entry->y-SUBGAME_SMALL_ICON_SIZE/2 &&
	    y < entry->y+SUBGAME_SMALL_ICON_SIZE/2) {
	  SetCursorPosition((Node *) entry->n);
	  subgame_toggle();
	  Render();
	  return;
	}
      }
      else {
	if (x > entry->x+draw_settings.NodeLength()+
	    entry->nums*INFOSET_SPACING-SUBGAME_LARGE_ICON_SIZE &&
	    x < entry->x+draw_settings.NodeLength()+
	    entry->nums*INFOSET_SPACING+SUBGAME_LARGE_ICON_SIZE &&
	    y > entry->y-SUBGAME_LARGE_ICON_SIZE/2 &&
	    y < entry->y+SUBGAME_LARGE_ICON_SIZE/2) {
	  SetCursorPosition((Node *) entry->n);
	  subgame_toggle();
	  Render();
	  return;
	}
      }
        
      // Check if clicked on a Node Above/Below
      if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW) {
	if (y > entry->y-MAX_TH && y < entry->y+DELTA)
	  id = draw_settings.LabelNodeAbove();
	if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
	  id = draw_settings.LabelNodeBelow();
      }
      if (id != -1) {
	SetCursorPosition((Node *) entry->n);
	switch (id) {
	case NODE_ABOVE_NOTHING: break;
	  //	case NODE_ABOVE_LABEL: node_label(); break;
	  //	case NODE_ABOVE_PLAYER: tree_players(); break;
	  //	case NODE_ABOVE_ISETLABEL: infoset_switch_player(); break;
	  //   case NODE_ABOVE_ISETID:  infoset_switch_player(); break;
	  //	case NODE_ABOVE_OUTCOME: EditOutcomePayoffs(); break;
	case NODE_ABOVE_REALIZPROB: break;
	case NODE_ABOVE_BELIEFPROB: break;
	case NODE_ABOVE_VALUE: break;
	}
	Render();
	return;
      }
        
      // Check if clicked on a branch Above/Below
      if (entry->parent) {  // no branches for root
	if(x > entry->x-draw_settings.BranchLength() &&
	   x < entry->x-draw_settings.BranchLength()+MAX_TW) {
	  if (y > entry->y-MAX_TH && y < entry->y+DELTA)
	    id = draw_settings.LabelBranchAbove();
	  if (y > entry->y+DELTA && y < entry->y+MAX_TH)
	    id = draw_settings.LabelBranchBelow();
	}
      }
      if (id != -1) {
	SetCursorPosition((Node *) entry->parent->n);
	switch (id) {
	case BRANCH_ABOVE_NOTHING: break;
	  //	case BRANCH_ABOVE_LABEL: action_label(); break;
	  //	case BRANCH_ABOVE_PLAYER: tree_players(); break;
	  //case BRANCH_ABOVE_PROBS: action_probs(); break;
	case BRANCH_ABOVE_VALUE: break;
	}
	Render();
	return;
      }
        
      // Check if clicked to the right of a node
      if(x > entry->x+draw_settings.NodeLength()+10 &&
	 x < entry->x+draw_settings.NodeLength()+10+
	 draw_settings.OutcomeLength()*ef.NumPlayers() &&
	 y > entry->y-DELTA && y < entry->y+DELTA) {
	id = draw_settings.LabelNodeRight();
      }
      if (id != -1) {
	SetCursorPosition((Node *) entry->n);
	switch (id) {
	case NODE_RIGHT_NOTHING:
	  break;
	case NODE_RIGHT_OUTCOME:
	case NODE_RIGHT_NAME:
	  //	  EditOutcomePayoffs();
	  break;
	}
	Render();
	return;
      }
    }
  }
}

//
// Right Clicking on a label will tell you what the label is refering to
// If the click was not on top of a text label, a popup menu (Buid) is created
//
void TreeWindow::OnRightClick(wxMouseEvent &p_event)
{
  long x, y;
  p_event.GetPosition(&x, &y);
  NodeEntry *entry;
  char *s = 0;
  for (int i = 1; i <= node_list.Length(); i++) {
    entry = node_list[i];
    // Check if clicked on a Node Above/Below
    if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW) {
      if (y > entry->y-MAX_TH && y < entry->y+DELTA)
	s = node_above_src[draw_settings.LabelNodeAbove()].l_name;
      if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
	s = node_below_src[draw_settings.LabelNodeBelow()].l_name;
    }
    // Check if clicked on a branch Above/Below
    if (entry->parent) {  // no branches for root
      if(x > entry->x-draw_settings.BranchLength() &&
	 x < entry->x-draw_settings.BranchLength()+MAX_TW) {
	if (y > entry->y-MAX_TH && y < entry->y+DELTA)
	  s = branch_above_src[draw_settings.LabelBranchAbove()].l_name;
	if (y > entry->y+DELTA && y < entry->y+MAX_TH)
	  s = branch_below_src[draw_settings.LabelBranchBelow()].l_name;
      }
    }
    // Check if clicked to the right of a node
    if (x > entry->x+draw_settings.NodeLength()+10 &&
	x < entry->x+draw_settings.NodeLength()+10+
	draw_settings.OutcomeLength()*ef.NumPlayers() &&
	y > entry->y-DELTA && y < entry->y+DELTA) {
      s = node_right_src[draw_settings.LabelNodeRight()].l_name;
    }
    if (s) {
      m_parent->SetStatusText(s);
      return;
    }
  }
  m_parent->SetStatusText("");

  // If we got here, the click was NOT on top of a text label, do the menu
  int x_start, y_start;
  ViewStart(&x_start, &y_start);
  wxClientDC dc(this);
  dc.SetUserScale(m_zoom, m_zoom);
  PopupMenu(m_editMenu, dc.LogicalToDeviceX(x-x_start*PIXELS_PER_SCROLL),
	    dc.LogicalToDeviceY(y-y_start*PIXELS_PER_SCROLL));
}

void TreeWindow::OnRightDoubleClick(wxMouseEvent &p_event)
{
  long x, y;
  p_event.GetPosition(&x, &y);
  NodeEntry *entry;
  int id = -1;
  for (int i = 1; i <= node_list.Length(); i++) {
    entry = node_list[i];
    // Check if clicked on a Node Above/Below
    if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW) {
      if (y > entry->y-MAX_TH && y < entry->y+DELTA)
	id = NODE_ABOVE_LEGEND;
      if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA) 
	id = NODE_BELOW_LEGEND;
    }
    // Check if clicked on a branch Above/Below
    if (entry->parent)  { // no branches for root
      if (x > entry->x-draw_settings.BranchLength() && 
	  x < entry->x-draw_settings.BranchLength()+MAX_TW) {
	if (y > entry->y-MAX_TH && y < entry->y+DELTA) 
	  id = BRANCH_ABOVE_LEGEND;
	if (y > entry->y+DELTA && y < entry->y+MAX_TH)
	  id = BRANCH_BELOW_LEGEND;
      }
    }
    // Check if clicked to the right of a node
    if (x > entry->x+draw_settings.NodeLength()+10 &&
	x < entry->x+draw_settings.NodeLength()+10+
	draw_settings.OutcomeLength()*ef.NumPlayers() &&
	y > entry->y-DELTA && y < entry->y+DELTA) {
      id = NODE_RIGHT_LEGEND;
    }
    if (id != -1) {
      //      draw_settings.SetLegends();
      Render();
      return;
    }
  }
}

//
// Process Shift
// In Gambit, holding down shift initiates a 'cut' function.  If the cursor
// is located over a 'cuttable' object (node, branch, iset line), it will change
// to a 'scissors.'  Pressing the left mouse button while the cursor is
// 'scissors' will cut the object under the cursor. Returns true if an actual
// cut took place.
//
bool TreeWindow::ProcessShift(wxMouseEvent &ev)
{
#ifdef NOT_PORTED_YET
    if (!ev.ShiftDown())
    {
#ifndef LINUX_WXXT
        if (wx_cursor == scissor_cursor)
            SetCursor(wxSTANDARD_CURSOR);
#else
        // wxxt doesn't have a wx_cursor field; this is a hack.
        SetCursor(wxSTANDARD_CURSOR);
#endif
        return false;
    }
    
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *iset_cut_entry = 0, *node_cut_entry = 0, *branch_cut_entry = 0,
        *outcome_cut_entry = 0;
    bool cut_cursor = false;

    for (int i = 1; i <= node_list.Length() && !iset_cut_entry; i++)
    {
        NodeEntry *entry = node_list[i];
        // Check if the cursor is on top of a infoset line
        if (entry->infoset.y != -1 && entry->n->GetInfoset())
            if (x > entry->x+entry->num*INFOSET_SPACING-2 &&
                x < entry->x+entry->num*INFOSET_SPACING+2)
                if (y > entry->y && y < entry->infoset.y)
                {
                    // next iset is below this one
                    iset_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }
                else if (y > entry->infoset.y && y < entry->y)
                {
                    // next iset is above this one
                    iset_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }

        // Check if the cursor is on top of a node
        if (x > entry->x+entry->num*INFOSET_SPACING+10 && 
            x < entry->x+draw_settings.NodeLength()+entry->num*INFOSET_SPACING &&
            y > entry->y-2 && y < entry->y+2)
        {
            node_cut_entry = entry;
            cut_cursor = true;
            break;
        }

        // Check if the cursor is on top of an outcome
        if (entry->has_children == 0 && entry->n->GetOutcome())
        {
            if (x > entry->x+entry->num*INFOSET_SPACING+10+draw_settings.NodeLength() &&
                x < entry->x+draw_settings.NodeLength()+
                entry->num*INFOSET_SPACING+draw_settings.OutcomeLength() &&
                y > entry->y-2 && y < entry->y+2)
            {
                outcome_cut_entry = entry;
                cut_cursor = true;
                break;
            }
        }
        
        // Check if the cursor is on top of a branch
        NodeEntry *parent_entry = GetNodeEntry(entry->n->GetParent());
        if (parent_entry)
        {
            if (x > parent_entry->x+draw_settings.NodeLength()+
                parent_entry->num*INFOSET_SPACING+10 &&
                x < parent_entry->x+draw_settings.NodeLength()+
                draw_settings.ForkLength()+parent_entry->num*INFOSET_SPACING)
            {
                // Good old slope/intercept method for finding a point on a line
                int y0 = parent_entry->y + 
                    (int) (x - parent_entry->x - 
                           draw_settings.NodeLength() -
                           parent_entry->nums*INFOSET_SPACING) *
                    (entry->y-parent_entry->y)/draw_settings.ForkLength();

                if (y > y0-2 && y < y0+2)
                {
                    branch_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }
            }
        }
    }

    if (ev.LeftDown() && cut_cursor)
    {
        // clicking the left mouse button will ...
        if (iset_cut_entry)  // cut an infoset
        {
            Infoset *siset = ef.SplitInfoset((Node *)iset_cut_entry->n);
            siset->SetName("Infoset"+ToText(siset->GetPlayer()->NumInfosets()));
            infosets_changed = true;
            OnPaint();
            return true;
        }

        if (node_cut_entry)  // cut a node
        {
            ef.DeleteTree((Node *)node_cut_entry->n);
            nodes_changed = true;
            OnPaint();
            return true;
        }

        if (outcome_cut_entry)
        {
            ((Node *) outcome_cut_entry->n)->SetOutcome(0);
            outcomes_changed = true;
            OnPaint();
            return true;
        }

        if (branch_cut_entry)  // cut a branch
        {
            ef.DeleteAction(branch_cut_entry->n->GetParent()->GetInfoset(),
                            LastAction(ef, (Node *)branch_cut_entry->n));
            nodes_changed = true;
            OnPaint();
            return true;
        }
    }
    
#ifndef LINUX_WXXT
    if (!cut_cursor && wx_cursor == scissor_cursor)
        SetCursor(wxSTANDARD_CURSOR);

    if (cut_cursor && wx_cursor != scissor_cursor) 
        SetCursor(scissor_cursor);
#else
    // wxxt doesn't have a wx_cursor field; this is a hack.
    if (!cut_cursor)
        SetCursor(wxSTANDARD_CURSOR);

    if (cut_cursor) 
        SetCursor(scissor_cursor);
#endif // LINUX_WXXT
#endif  // NOT_PORTED_YET
    
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

  Render();
}

//
// SupportChanged -- must be called by parent every time the cur_sup
// changes.  Note that since it is a reference, it needs not be passed here.
//
void TreeWindow::SupportChanged(void)
{
  // Check if the cursor is still valid
  NodeEntry *ne = NodeList().GetNodeEntry(Cursor());
  if (ne->child_number) {
    if (!m_parent->GetSupport()->Find(Cursor()->GetInfoset()->Actions()[ne->child_number]))
      SetCursorPosition(ef.RootNode());
  }

  ForceRecalc();
  Render();
}

void TreeWindow::SetSubgamePickNode(Node *n)
{
  if (n) {
    // save the actual cursor, and fake a cursor movement to ensure
    // that the node is visible
    Node *cur_cursor = Cursor();
    SetCursorPosition((Node *) n);
    ProcessCursor();
    NodeEntry *ne = NodeList().GetNodeEntry(n);
    wxClientDC dc(this);
    dc.SetUserScale(m_zoom, m_zoom);
    DrawSubgamePickIcon(dc, *ne);
    SetCursorPosition(cur_cursor);
  }
  subgame_node = n;
}

void TreeWindow::Render(void)
{
  wxClientDC dc(this);
  dc.SetUserScale(m_zoom, m_zoom);
  Render(dc);
}

Node *TreeWindow::GotObject(long &x, long &y, int what)
{
  for (int i = 1; i <= node_list.Length(); i++) {
    NodeEntry *entry = node_list[i];
        
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
      NodeEntry *start_entry = NodeList().GetNodeEntry(branch_drag->StartNode());
      int xs = start_entry->x+draw_settings.NodeLength()+
	draw_settings.ForkLength()+start_entry->nums*INFOSET_SPACING;
      if (x > xs && x < xs+draw_settings.BranchLength() &&
	  y < start_entry->y+(start_entry->n->Game()->NumChildren(start_entry->n)+1)*draw_settings.YSpacing() &&
	  y > start_entry->y-(start_entry->n->Game()->NumChildren(start_entry->n)+1)*draw_settings.YSpacing()) {
	// figure out at what branch # the mouse was released
	int br = 1;
	NodeEntry *child_entry, *child_entry1;
	for (int ii = 1; ii <= start_entry->n->Game()->NumChildren(start_entry->n)-1; ii++) {
	  child_entry = NodeList().GetNodeEntry(start_entry->n->GetChild(ii));
	  if (ii == 1) 
	    if (y < child_entry->y) {
	      br = 1;
	      break;
	    }
	  child_entry1 = NodeList().GetNodeEntry(start_entry->n->GetChild(ii+1));
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
  node_list.SubgameList().Flush();
  node_list.SubgameList().Append(SubgameEntry(ef.RootNode()));
  gList<Node *> subgame_roots;
  LegalSubgameRoots(ef, subgame_roots);
  ef.MarkSubgames(subgame_roots);

  for (int i = 1; i <= subgame_roots.Length(); i++) {
    if (subgame_roots[i] != ef.RootNode())
      node_list.SubgameList().Append(SubgameEntry(subgame_roots[i], true));
  }
  
  ForceRecalc();
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
  node_list.SubgameList().Append(SubgameEntry(Cursor(), true)); // collapse
  ForceRecalc();
}

void TreeWindow::SubgameUnmark(void)
{
  if (Cursor()->GetSubgameRoot() != Cursor() ||
      Cursor()->GetSubgameRoot() == ef.RootNode())
    return;
    
  ef.UnmarkSubgame(Cursor());

  for (int i = 1; i <= node_list.SubgameList().Length(); i++) {
    if (node_list.SubgameList()[i].root == Cursor())
      node_list.SubgameList().Remove(i);
  }

  ForceRecalc();
}

void TreeWindow::SubgameUnmarkAll(void)
{
  ef.UnmarkSubgames(ef.RootNode());
  node_list.SubgameList().Flush();
  node_list.SubgameList().Append(SubgameEntry(ef.RootNode()));
  ForceRecalc();
}

void TreeWindow::SubgameCollapse(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++) {
    if (node_list.SubgameList()[i].root == Cursor()) {
      node_list.SubgameList()[i].expanded = false;
      ForceRecalc();
      return;
    }
  }
}

void TreeWindow::SubgameCollapseAll(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++)
    node_list.SubgameList()[i].expanded = false;

  ForceRecalc();
}

void TreeWindow::SubgameExpand(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++) {
    if (node_list.SubgameList()[i].root == Cursor()) {
      node_list.SubgameList()[i].expanded = true;
      ForceRecalc();
      return;
    }
  }
}

void TreeWindow::SubgameExpandBranch(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++) {
    if (node_list.SubgameList()[i].root == Cursor()) {
      for (int j = 1; j <= node_list.SubgameList().Length(); j++) {
	if (ef.IsSuccessor(node_list.SubgameList()[j].root, Cursor())) {
	  node_list.SubgameList()[j].expanded = true;
	  ForceRecalc();
	}
      }

      return;
    }
  }
}

void TreeWindow::SubgameExpandAll(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++)
    node_list.SubgameList()[i].expanded = true;
  
  ForceRecalc();
}

void TreeWindow::subgame_toggle(void)
{
  for (int i = 1; i <= node_list.SubgameList().Length(); i++) {
    if (node_list.SubgameList()[i].root == Cursor()) {
      node_list.SubgameList()[i].expanded = !node_list.SubgameList()[i].expanded; 
      ForceRecalc();
      return;
    }
  }
}


//***********************************************************************
//                      FILE-OUTPUT MENU HANDLER
//***********************************************************************
void TreeWindow::output(void)
{
  wxOutputDialogBox od;

  if (od.ShowModal() == wxID_OK) {
    switch (od.GetMedia()) {
    case wxMEDIA_PRINTER: print(od.GetOption()); break;
    case wxMEDIA_PS:print_eps(od.GetOption()); break;
    case wxMEDIA_CLIPBOARD:print_mf(od.GetOption()); break;
    case wxMEDIA_METAFILE: print_mf(od.GetOption(), true); break;
    case wxMEDIA_PREVIEW: print(od.GetOption(), true); break;
    default:
      // We'll ignore this silently
      break;
    }
  }
}

#ifdef wx_msw
#include "wx_print.h"

class ExtensivePrintout: public wxPrintout {
private:
  TreeWindow *tree;
  wxOutputOption fit;
  int num_pages;
    
public:
  ExtensivePrintout(TreeWindow *s, wxOutputOption f,
		    const char *title = "ExtensivePrintout");
  Bool OnPrintPage(int page);
  Bool HasPage(int page);
  Bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


Bool ExtensivePrintout::OnPrintPage(int)
{
  // this is funky--I am playing around w/ the
  // different zoom settings.  So the zoom setting in draw_settings does not
  // equal to the zoom setting in the printer!
  wxDC *dc = GetDC();
  if (!dc) return false;
    
  dc->SetBackgroundMode(wxTRANSPARENT);
  dc->Colour = false;
  Bool color_outcomes;
  
  if (!dc->Colour) {
    color_outcomes = tree->draw_settings.ColorCodedOutcomes();
    tree->draw_settings.SetOutcomeColor(false);
  }
    
  int win_w, win_h;
  tree->GetClientSize(&win_w, &win_h);    // that is the size of the window
  float old_zoom = tree->draw_settings.Zoom();
  // Now we have to check in case our real page size is reduced
  // (e.g. because we're drawing to a print preview memory DC)
  int pageWidth, pageHeight;
  float w, h;
  dc->GetSize(&w, &h);
  GetPageSizePixels(&pageWidth, &pageHeight);
  float pageScaleX = (float)w/pageWidth;
  float pageScaleY = (float)h/pageHeight;
  
  if (fit) { // fit to page
    int maxX = tree->node_list.MaxX(), maxY = tree->node_list.MaxY(); // size of tree
    // Figure out the 'fake' window zoom
    float zoom_x = (float)win_w/(float)maxX, zoom_y = (float)win_h/(float)maxY;
    float real_zoom = gmin(zoom_x, zoom_y);
    tree->draw_settings.SetZoom(real_zoom, true);
    // Figure out the 'real' printer zoom
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX;
    float marginY = 1*ppiPrinterY;
        
    zoom_x = (float)((pageWidth-2*marginX)/(float)maxX)*pageScaleX;
    zoom_y = (float)((pageHeight-2*marginY)/(float)maxY)*pageScaleY;
    real_zoom = gmin(zoom_x, zoom_y);
        
    dc->SetUserScale(real_zoom, real_zoom);
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
  }
  else {  // WYSIWYG
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
        
    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scaleX = (float)((float)ppiPrinterX/(float)ppiScreenX);
    float scaleY = (float)((float)ppiPrinterY/(float)ppiScreenY);
    
    // If printer pageWidth == current DC width, then this doesn't
    // change. But w might be the preview bitmap width, so scale down.
    float overallScaleX = scaleX * pageScaleX;
    float overallScaleY = scaleY * pageScaleY;
    dc->SetUserScale(overallScaleX, overallScaleY);
    
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX, marginY = 1*ppiPrinterY;
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
    // Figure out the 'fake' window zoom
    float real_width = (pageWidth-2*marginX)/scaleX;
    float real_height = (pageHeight-2*marginY)/scaleY;
    float zoom_x = win_w/real_width, zoom_y = win_h/real_height;
    float real_zoom = gmax(zoom_x, zoom_y);
    tree->draw_settings.SetZoom(real_zoom, true);
  }
    
  tree->Render(*dc);
    
  tree->draw_settings.SetZoom(old_zoom);
  if (!dc->Colour)
    tree->draw_settings.SetOutcomeColor(color_outcomes);
    
  return true;
}

Bool ExtensivePrintout::HasPage(int page)
{
  return (page <= 1);
}

ExtensivePrintout::ExtensivePrintout(TreeWindow *t, wxOutputOption f,
                                     const char *title)
  : tree(t), fit(f), wxPrintout((char *)title)
{ }

Bool ExtensivePrintout::OnBeginDocument(int startPage, int endPage)
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
    return false;
    
  return true;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void ExtensivePrintout::GetPageInfo(int *minPage, int *maxPage,
                                    int *selPageFrom, int *selPageTo)
{
  num_pages = 1;
  *minPage = 0;
  *maxPage = num_pages;
  *selPageFrom = 0;
  *selPageTo = 0;
}
#endif

//***********************************************************************
//                      TREE-PRINT MENU HANDLER
//***********************************************************************

#ifdef wx_msw
void TreeWindow::print(wxOutputOption fit, bool preview)
{
  if (!preview) {
    wxPrinter printer;
    ExtensivePrintout printout(this, fit);
    printer.Print(pframe, &printout, true);
  }
  else {
    wxPrintPreview *preview = new wxPrintPreview(new ExtensivePrintout(this, fit), new ExtensivePrintout(this, fit));
    wxPreviewFrame *ppframe = new wxPreviewFrame(preview, pframe, "Print Preview", 100, 100, 600, 650);
    ppframe->Centre(wxBOTH);
    ppframe->Initialize();
    ppframe->Show(true);
  }
}
#else
void TreeWindow::print(wxOutputOption /*fit*/, bool preview)
{
  if (!preview)
    wxMessageBox("Printing not supported under X");
  else
    wxMessageBox("Print Preview is not supported under X");
}
#endif


//***********************************************************************
//                      TREE-PRINT EPS MENU HANDLER
//***********************************************************************

void TreeWindow::print_eps(wxOutputOption fit)
{
#ifdef NOT_PORTED_YET
  wxPostScriptDC dc(NULL, true);
  if (dc.Ok()) {
    float old_zoom = 1.0;

    if (fit == wxFITTOPAGE) {
      old_zoom = draw_settings.Zoom();
      int w, h;
      GetSize(&w, &h);
      draw_settings.SetZoom(gmin((float)w/(float)node_list.MaxX(),
				 (float)h/(float)node_list.MaxY()), true);
    }

    Bool color_outcomes = draw_settings.ColorCodedOutcomes();
    draw_settings.SetOutcomeColor(false);
    dc.StartDoc("Gambit printout");
    dc.StartPage();
    Render(dc);
    dc.EndPage();
    dc.EndDoc();
    if (fit == wxFITTOPAGE)
      draw_settings.SetZoom(old_zoom);
    draw_settings.SetOutcomeColor(color_outcomes);
  }
#endif  // NOT_PORTED_YET
}

//***********************************************************************
//                      TREE-PRINT MF MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print_mf(wxOutputOption fit, bool save_mf)
{
#ifdef NOT_PORTED_YET
  char *metafile_name = 0;

  if (save_mf)
    metafile_name = copystring(wxFileSelector("Save Metafile", 0, 0, ".wmf", "*.wmf"));

  wxMetaFileDC dc_mf(metafile_name);

  if (dc_mf.Ok()) {
    dc_mf.SetBackgroundMode(wxTRANSPARENT);
    float old_zoom;

    if (fit == wxFITTOPAGE) {
      old_zoom = draw_settings.Zoom();
      int w, h;
      GetSize(&w, &h);
      draw_settings.SetZoom(gmin((float)w/(float)node_list.MaxX(),
				 (float)h/(float)node_list.MaxY()), true);
    }

    if (!save_mf) 
      dc_mf.SetUserScale(2.0, 2.0);

    Render(dc_mf);

    if (fit == wxFITTOPAGE) 
      draw_settings.SetZoom(old_zoom);
        
    wxMetaFile *mf = dc_mf.Close();
    
    if (mf) {
      Bool success = mf->SetClipboard((int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
      if (!success) 
	wxMessageBox("Copy Failed", "Error", wxOK | wxCENTRE, pframe);
      delete mf;
    }

    if (save_mf)
      wxMakeMetaFilePlaceable(
	  metafile_name, 0, 0, (int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
  }
#endif  // NOT_PORTED_YET
}
#else
void TreeWindow::print_mf(wxOutputOption /*fit*/, bool /*save_mf*/)
{
  wxMessageBox("Metafiles are not supported under X");
}
#endif

//-----------------------------------------------------------------------
//                     DISPLAY MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void TreeWindow::OnSize(wxSizeEvent &p_event)
{
  if (node_list.MaxX() == 0 || node_list.MaxY() == 0) {
    Render();
  }

  // This extra check because wxMSW seems to generate OnSize events
  // rather liberally (e.g., a size of (0,0) for minimizing the window)
  if (p_event.GetSize().GetWidth() == 0 ||
      p_event.GetSize().GetHeight() == 0) {
    return;
  }

  double zoomx = ((double) p_event.GetSize().GetWidth() /
		  (double) node_list.MaxX());
  double zoomy = ((double) p_event.GetSize().GetHeight() /
		  (double) node_list.MaxY());
    
  zoomx = gmin(zoomx, 1.0);
  zoomy = gmin(zoomy, 1.0);
  m_zoom = gmin(zoomx, zoomy); 
  Render();
}

void TreeWindow::prefs_display_flashing(void)
{
  if (!draw_settings.FlashingCursor()) {
    draw_settings.SetFlashingCursor(true);
    delete (TreeNodeCursor *) flasher;
    flasher = new TreeNodeFlasher(this);
  }
  else {
    draw_settings.SetFlashingCursor(false);
    delete (TreeNodeFlasher *) flasher;
    flasher = new TreeNodeCursor(this);
  }

  ProcessCursor();
  ForceRecalc();
}


template class gList<NodeEntry *>;
template class gList<SubgameEntry>;
gOutput &operator<<(gOutput &p_stream, const SubgameEntry &)
{ return p_stream; }

