//
// FILE: treewin.cc -- Implementation of TreeWindow class
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#ifdef wx_msw
#include "wx_mf.h"
#endif  // wx_msw

#include "gmisc.h"
#include "efg.h"
#include "legendc.h"
#include "treewin.h"
#include "twflash.h"
#include "efgshow.h"
#include "treedrag.h"

#include "dlmoveadd.h"
#include "infosetd.h"
#include "dlefgsave.h"
#include "dlnodedelete.h"
#include "dlactionlabel.h"
#include "dlactionselect.h"
#include "dlactionprobs.h"
#include "dlefgreveal.h"
#include "dlefgplayer.h"
#include "dlefgpayoff.h"
#include "dlefgoutcome.h"

#include "efgutils.h"
#include "glist.imp"

extern int INFOSET_SPACING;
extern int SUBGAME_LARGE_ICON_SIZE;
extern int SUBGAME_SMALL_ICON_SIZE;
extern int SUBGAME_PICK_SIZE;
extern void DrawSubgamePickIcon(wxDC &, const NodeEntry &);


#define TOP_MARGIN              40
#define DELTA                   8
#define MAX_TW                  60
#define MAX_TH                  20


wxFont   *outcome_font;
wxBrush  *white_brush;
wxCursor *scissor_cursor;


//----------------------------------------------------------------------
//                      TreeWindow: Member functions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//                  TreeWindow: Constructor and destructor
//----------------------------------------------------------------------

TreeWindow::TreeWindow(Efg &ef_, EFSupport * &disp, EfgShow *frame_) 
    : TreeRender(frame_, this, node_list, hilight_infoset, hilight_infoset1,
                 mark_node, subgame_node, draw_settings),
      ef(ef_), disp_sup(disp), frame(frame_), pframe(frame_)
{
    // Set the cursor to the root node
    m_cursor = ef.RootNode();
    // Make sure that Chance player has a name
    ef.GetChance()->SetName("Chance");
    // Add the first subgame -- root subgame
    subgame_list.Append(SubgameEntry(ef.RootNode()));
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
    mark_node = 0; old_mark_node = 0;
    // No isets are being hilighted
    hilight_infoset = 0; hilight_infoset1 = 0;
    // No zoom window or outcome dialog
    zoom_window = 0;
    outcome_font = wxTheFontList->FindOrCreateFont(9, wxSWISS, wxNORMAL, wxNORMAL);
    white_brush = wxTheBrushList->FindOrCreateBrush("WHITE", wxSOLID);
    
#ifdef wx_msw
    scissor_cursor = new wxCursor("SCISSORCUR");
#else
#include "bitmaps/scissor.xbm"
#ifndef LINUX_WXXT
    scissor_cursor = new wxCursor(scissor_bits, scissor_width, scissor_height,
                                  -1, -1, scissor_bits);
#else
    // wxxt uses an older constructor
    scissor_cursor = new wxCursor(scissor_bits, scissor_width, scissor_height,
                                  scissor_width/2, scissor_height/2);
#endif // LINUX_WXXT
#endif // wx_msw
    
    GetDC()->SetBackgroundMode(wxTRANSPARENT);
    AllowDoubleClick(TRUE);
    // Make sure the node_list gets recalculated the first time
    nodes_changed = TRUE;
    infosets_changed = TRUE;
    outcomes_changed = FALSE;
    must_recalc = FALSE;
    log = FALSE;

    // Create scrollbars
    SetScrollbars(PIXELS_PER_SCROLL, PIXELS_PER_SCROLL, 60, 60, 4, 4);
    draw_settings.set_x_steps(60);
    draw_settings.set_y_steps(60);
    
    // Create a popup menu
    MakeMenus();
    // Set up the menu enabling
    UpdateMenus();
}

TreeWindow::~TreeWindow(void)
{
  delete node_drag;
  delete iset_drag;
  delete branch_drag;
  delete outcome_drag;
  Show(FALSE);
}

void TreeWindow::MakeMenus(void)
{
  edit_menu = new wxMenu(NULL, (wxFunction)OnPopup);

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

  edit_menu->Append(efgmenuEDIT_NODE, "&Node", nodeMenu, "Edit the node");
  edit_menu->Append(efgmenuEDIT_ACTIONS, "&Actions", action_menu, 
		    "Edit actions");
  edit_menu->Append(efgmenuEDIT_INFOSET, "&Infoset", infoset_menu,
		    "Edit infosets");
  edit_menu->Append(efgmenuEDIT_OUTCOMES, "&Outcomes", outcome_menu,
		    "Edit outcomes and payoffs");
  edit_menu->Append(efgmenuEDIT_TREE, "&Tree", tree_menu,
		    "Edit the tree");

  edit_menu->SetClientData((char *)frame); // call back to parent later
}

gText TreeWindow::AsString(TypedSolnValues what, const Node *n, int br) const
{
    return frame->AsString(what, n, br);
}

//---------------------------------------------------------------------
//                  TreeWindow: Event-hook members
//---------------------------------------------------------------------

static Node *PriorSameLevel(NodeEntry *e, const gList<NodeEntry *> &node_list)
{
    int e_n = node_list.Find(e);
    assert(e_n && "Node not found");
    for (int i = e_n-1; i >= 1; i--)
        if (node_list[i]->level == e->level)
            return (Node *) node_list[i]->n;
    return 0;
}

static Node *NextSameLevel(NodeEntry *e, const gList<NodeEntry *> &node_list)
{
    int e_n = node_list.Find(e);
    assert(e_n && "Node not found");
    for (int i = e_n+1; i <= node_list.Length(); i++)
        if (node_list[i]->level == e->level) 
            return (Node *) node_list[i]->n;
    return 0;
}

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
// OnChar -- handle keypress events
// Currently we support the following keys:
//     left arrow:   go to parent of current node
//     right arrow:  go to first child of current node
//     up arrow:     go to previous sibling of current node
//     down arrow:   go to next sibling of current node
// Since the addition of collapsible subgames, a node's parent may not
// be visible in the current display.  Thus, find the first predecessor
// that is visible (ROOT is always visible)
//
void TreeWindow::OnChar(wxKeyEvent& ch)
{
  // Accelerators:
  // Note that accelerators are provided for in the wxwin code but only for the
  // windows platform.  In order to make this more portable, accelerators for
  // this program are coded in the header file and processed in OnChar
  //
    
  frame->CheckAccelerators(ch);
    
  if (ch.ShiftDown() == FALSE) {
    bool c = false;   // set to true if cursor position has changed
    switch (ch.KeyCode()) {
    case WXK_LEFT:
      if (Cursor()->GetParent()) {
	SetCursorPosition((Node *) GetValidParent(Cursor())->n);
	c = true;
      }
      break;
    case WXK_RIGHT:
      if (GetValidChild(Cursor())) {
	SetCursorPosition((Node *) GetValidChild(Cursor())->n);
	c = true;
      }
      break;
    case WXK_UP: {
      Node *prior = ((!ch.ControlDown()) ? 
		     PriorSameLevel(GetNodeEntry(Cursor()), node_list) :
		     PriorSameIset(Cursor()));
      if (prior) {
	SetCursorPosition(prior);
	c = true;
      }
      break;
    }
    case WXK_DOWN: {
      Node *next = ((!ch.ControlDown()) ?
		    NextSameLevel(GetNodeEntry(Cursor()), node_list) :
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
        
    if (c) 
      ProcessCursor(); // cursor moved

    // Implement the behavior that when control+cursor key is pressed, the
    // nodes belonging to the iset are hilighted.
    if (c && ch.ControlDown()) {
      if (hilight_infoset1 != Cursor()->GetInfoset()) {
	hilight_infoset1 = Cursor()->GetInfoset();
	OnPaint();
      }
    }
    if (!ch.ControlDown() && hilight_infoset1) {
      hilight_infoset1 = 0;
      OnPaint();
    }
  }
  else
    wxCanvas::OnChar(ch);
}

//
// OnEvent -- handle mouse events
// Currently we support selecting a node by clicking on it
//
void TreeWindow::OnEvent(wxMouseEvent& ev)
{
    // Check all the draggers.  Note that they are mutually exclusive
    if (!iset_drag->Dragging() && !branch_drag->Dragging() &&
	!outcome_drag->Dragging())
      if (node_drag->OnEvent(ev, nodes_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !branch_drag->Dragging() &&
	!outcome_drag->Dragging())
      if (iset_drag->OnEvent(ev, infosets_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !iset_drag->Dragging() &&
	!outcome_drag->Dragging())
      if (branch_drag->OnEvent(ev, infosets_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !iset_drag->Dragging() &&
        !branch_drag->Dragging())
        if (outcome_drag->OnEvent(ev, outcomes_changed) != DRAG_NONE) return;
    
    // Implements the 'cutting' behavior
    if (ProcessShift(ev)) return;
    
    // Double clicking hilights iset and toggles subgames
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
}

void TreeWindow::OnPaint(void)
{
    TreeRender::OnPaint();
    if (zoom_window) 
        zoom_window->OnPaint();
    AdjustScrollbarSteps();
}

//---------------------------------------------------------------------
//                   TreeWindow: Drawing functions
//---------------------------------------------------------------------

// these global variables are convinient to use in the recursive rendering
// of course, they will cause problems if parallel rendering ever occurs 
int maxlev, maxy, miny, ycoord;

NodeEntry *TreeWindow::GetNodeEntry(const Node *n)
{
    for (int i = 1; i <= node_list.Length(); i++)
        if (node_list[i]->n == n) return node_list[i];
    return 0;
}

TreeWindow::SubgameEntry &TreeWindow::GetSubgameEntry(const Node *n)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
        if (subgame_list[i].root == n) return subgame_list[i];
    return subgame_list[1]; // root subgame
}


// Added support for EFSupports.  We can now display in two modes: either all
// the nodes reachable from the root node that are in this support, or, all of
// the nodes that are in this support, with no connections if the connections
// are not in the support.
int TreeWindow::FillTable(const Node *n, int level)
{
    int y1 = -1, yn=0;
    
    SubgameEntry &subgame_entry = GetSubgameEntry(n->GetSubgameRoot());
    
    NodeEntry *entry = new NodeEntry;
    entry->n = n;   // store the node the entry is for
    node_list += entry;
    entry->in_sup = true;
    if (n->NumChildren()>0 && subgame_entry.expanded)
    {
        for (int i = 1; i <= n->NumChildren(); i++)
        {
            bool in_sup = true;
            if (n->GetPlayer()->GetNumber())        // pn == 0 for chance nodes
                in_sup = disp_sup->Find(n->GetInfoset()->Actions()[i]);
            if (in_sup)
            {
                yn = FillTable(n->GetChild(i), level+1);
                if (y1 == -1)  y1 = yn;
            }
            else   // not in the support.
            {
                if (!draw_settings.RootReachable())  // show only nodes reachable from root
                {
                    // still consider this node, but mark it as invisible
                    yn = FillTable(n->GetChild(i), level+1);
                    if (y1 == -1)  y1 = yn;
                    GetNodeEntry(n->GetChild(i))->in_sup = false;
                }
            }
        }
        entry->y = (y1 + yn) / 2;
    }
    else
    {
        entry->y = ycoord;
        ycoord += draw_settings.YSpacing();
    }
    
    entry->level = level;
    entry->has_children = n->NumChildren();
    // Find out what branch of the parent this node is on
    if (n == ef.RootNode())
        entry->child_number = 0;
    else
    {
        Node *parent = n->GetParent();
        for (int i = 1; i <= parent->NumChildren(); i++)
            if (parent->GetChild(i) == n)
                entry->child_number = i;
    }
    
    entry->infoset.y = -1;
    entry->infoset.x = -1;
    entry->num = 0;
    entry->nums = 0;
    entry->x = level * 
        (draw_settings.NodeLength() + draw_settings.BranchLength() +
         draw_settings.ForkLength());
    if (n->GetPlayer())
        entry->color = draw_settings.GetPlayerColor(n->GetPlayer()->GetNumber());
    else
        entry->color = draw_settings.GetPlayerColor(-1);
    
    entry->expanded = subgame_entry.expanded;
    
    maxlev = gmax(level, maxlev);
    maxy = gmax(entry->y, maxy);
    miny = gmin(entry->y, miny);
    
    return entry->y;
}

//
// CheckInfosetEntry.  Checks how many infoset lines are to be drawn at each
// level, spaces them by setting each infoset's node's num to the previous
// infoset node+1.  Also lengthens the nodes by the amount of space taken up
// by the infoset lines.
//
void TreeWindow::CheckInfosetEntry(NodeEntry *e)
{
    int pos;
    NodeEntry *infoset_entry, *e1;
    // Check if the infoset this entry belongs to (on this level) has already
    // been processed.  If so, make this entry->num the same as the one already
    // processed and return
    infoset_entry = NextInfoset(e);
    for (pos = 1; pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // if the infosets are the same and they are on the same level and e1 has been processed
        if (e->n->GetInfoset() == e1->n->GetInfoset() && 
            e->level == e1->level && e1->num)
        {
            e->num = e1->num;
            if (infoset_entry)
            {
                e->infoset.y = infoset_entry->y;
                if (draw_settings.ShowInfosets() == SHOWISET_ALL)
                    e->infoset.x = infoset_entry->x;
            }
            return;
        }
    }
    
    // If we got here, this entry does not belong to any processed infoset yet.
    // Check if it belongs to ANY infoset, if not just return
    if (!infoset_entry) return;
    
    // If we got here, then this entry is new and is connected to other entries
    // find the entry on the same level with the maximum num.
    // This entry will have num = num+1.
    int num = 0;
    for (pos = 1; pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // Find the max num for this level
        if (e->level == e1->level) num = gmax(e1->num, num);
    }
    num++;
    e->num = num;
    e->infoset.y = infoset_entry->y;
    if (draw_settings.ShowInfosets() == SHOWISET_ALL) 
        e->infoset.x = infoset_entry->x;
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
NodeEntry *TreeWindow::NextInfoset(const NodeEntry * const e)
{
    NodeEntry *e1;
    
    for (int pos = node_list.Find((NodeEntry * const)e)+1;
         pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // infosets are the same and the nodes are on the same level
        if (e->n->GetInfoset() == e1->n->GetInfoset())  
            if (draw_settings.ShowInfosets() == SHOWISET_ALL)
                return e1;
            else if (e->level == e1->level) return e1;
    }
    return 0;
}

void TreeWindow::FillInfosetTable(const Node *n)
{
    NodeEntry *entry = GetNodeEntry(n);
    if (n->NumChildren()>0)
        for (int i = 1; i <= n->NumChildren(); i++)
        {
            bool in_sup = true;
            if (n->GetPlayer()->GetNumber())        // pn == 0 for chance nodes
                in_sup = disp_sup->Find(n->GetInfoset()->Actions()[i]);
            
            if (in_sup || !draw_settings.RootReachable())
                FillInfosetTable(n->GetChild(i));
        }
    if (entry) CheckInfosetEntry(entry);
}

void TreeWindow::UpdateTableInfosets(void)
{
    // Note that levels are numbered from 0, not 1.
    // create an array to hold max num for each level
    gArray<int> nums(0, maxlev+1); 
    
    for (int i = 0; i <= maxlev + 1; nums[i++] = 0);
    NodeEntry *e;
    // find the max e->num for each level
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        nums[e->level] = gmax(e->num+1, nums[e->level]);
    }
    
    // record the max e->num for each level for each node
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        e->nums = nums[e->level];
    }
    
    for (int i = 0; i <= maxlev; i++)  nums[i+1] += nums[i];
    
    // now add the needed length to each level
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        if (e->level != 0) 
            e->x += nums[e->level-1]*INFOSET_SPACING;
    }
}

NodeEntry *TreeWindow::GetValidParent(const Node *e)
{
    assert(e && "Parent not found");
    NodeEntry *n = GetNodeEntry(e->GetParent());
    if (n) 
        return n;
    else 
        return GetValidParent(e->GetParent());
}

NodeEntry *TreeWindow::GetValidChild(const Node *e)
{
    for (int i = 1; i <= e->NumChildren(); i++)
    {
        NodeEntry *n = GetNodeEntry(e->GetChild(i));
        if (n)
            return n;
        else
        {
            n = GetValidChild(e->GetChild(i));
            if (n) return n;
        }
    }
    return 0;
}

void TreeWindow::UpdateTableParents(void)
{
    NodeEntry *e;
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        e->parent = (e->n == ef.RootNode()) ? e : GetValidParent(e->n);
        if (!GetValidChild(e->n)) e->has_children = 0;
    }
}


//
// Render: The main rendering routine
//

void TreeWindow::Render(wxDC &dc)
{
    int width, height, x_start, y_start;
    if (nodes_changed || infosets_changed || must_recalc) {
      UpdateMenus();

        // Recalculate only if needed.
        // Note that node_table is preserved until the next recalculation.
        node_list.Flush();

        // If we modify the structure of the game, revert back to the full support
        // for the time being.  Otherwise, we run into weird problems.
        if (nodes_changed || infosets_changed) 
            frame->GameChanged();
        
        maxlev = miny = maxy = 0;
        ViewStart(&x_start, &y_start);
        GetClientSize(&width, &height);
        ycoord = TOP_MARGIN;
        FillTable(ef.RootNode(), 0);

        if (draw_settings.ShowInfosets())
        {
            // FIXME! This causes lines to disappear... sometimes.
            FillInfosetTable(ef.RootNode());
            UpdateTableInfosets();
        }

        UpdateTableParents();
        draw_settings.SetMaxX((maxlev + 1) * 
                              (draw_settings.BranchLength() + 
                               draw_settings.ForkLength() + 
                               draw_settings.NodeLength()) + 
                              draw_settings.OutcomeLength());
        draw_settings.SetMaxY(maxy+25);

        if (must_recalc)
        {
            must_recalc = FALSE;
            need_clear = TRUE;
        }
    }

    if (nodes_changed || infosets_changed || outcomes_changed)
    {
        frame->RemoveSolutions();
        nodes_changed = FALSE;
        infosets_changed = FALSE;
        outcomes_changed = FALSE;
        need_clear = TRUE;
    }

    char *dc_type = dc.GetClassInfo()->GetClassName();
    
    if (strcmp(dc_type, "wxCanvasDC") == 0) { // if drawing to screen
      if (Cursor()) {
	NodeEntry *entry = GetNodeEntry(Cursor());

	if (!entry) { 
	  SetCursorPosition(ef.RootNode());
	  entry = GetNodeEntry(Cursor());
	}

	UpdateCursor(entry);
      }
        
        if (need_clear)
        {
            dc.SetBrush(wxWHITE_BRUSH);
#ifdef wx_x  // a bug in wxwin/motif prevents Clear from working correctly.
            dc.DrawRectangle(0, 0, 10000, 10000);
#else
            dc.Clear();
#endif
        }
        dc.BeginDrawing();
    }
    else
    {
        flasher->SetFlashing(FALSE);
    }
    
    TreeRender::Render(dc);

    if (strcmp(dc_type, "wxCanvasDC") != 0)
        flasher->SetFlashing(TRUE); 
    else
        dc.EndDrawing();

    flasher->Flash();
}


// Adjust number of scrollbar steps if necessary.
void TreeWindow::AdjustScrollbarSteps(void)
{
    int width, height;
    int x_steps, y_steps;
    float zoom = draw_settings.Zoom();

    GetParent()->GetClientSize(&width, &height);
    // width and height are the dimensions of the visible canvas.
    height -= 50; // This compensates for a bug in GetClientSize().
    height = int(height / zoom);
    width  = int(width  / zoom);

    // x_steps and y_steps are the maximum number of scrollbar
    // steps in the x and y directions.
    if (draw_settings.MaxX() < width)
    {
        x_steps = 1;
    }
    else
    {
        //x_steps = draw_settings.MaxX() / PIXELS_PER_SCROLL + 1;
        x_steps = int((draw_settings.MaxX() * zoom) / PIXELS_PER_SCROLL) + 1;
    }

    if (draw_settings.MaxX() < height)
    {
        y_steps = 1;
    }
    else
    {
        //y_steps = draw_settings.MaxY() / PIXELS_PER_SCROLL + 1;
        y_steps = int((draw_settings.MaxY() * zoom) / PIXELS_PER_SCROLL) + 1;
    }

    if ((x_steps != draw_settings.x_steps()) ||
        (y_steps != draw_settings.y_steps()))
    {
        draw_settings.set_x_steps(x_steps);
        draw_settings.set_y_steps(y_steps);
        SetScrollbars(PIXELS_PER_SCROLL, PIXELS_PER_SCROLL, 
                    x_steps, y_steps, 4, 4);
    }
}



void TreeWindow::ProcessCursor(void)
{
    // A little scrollbar magic to insure the focus stays with the cursor.  This
    // can probably be optimized much further.  Consider using SetClippingRegion().
    // This also makes sure that the virtual canvas is large enough for the entire
    // tree.
    
    int x_start, y_start;
    int width, height;
    int x_steps, y_steps;
    int xs, xe, ys, ye;
    
    ViewStart(&x_start, &y_start);
    GetParent()->GetClientSize(&width, &height);
    height -= 50; // This compensates for a bug in GetClientSize().
    height  = int(height / draw_settings.Zoom());
    width   = int(width  / draw_settings.Zoom());
    x_steps = draw_settings.x_steps();
    y_steps = draw_settings.y_steps();
    
    // Make sure the cursor is visible.
    NodeEntry *entry = GetNodeEntry(Cursor()); 
    if (!entry) {
      SetCursorPosition(ef.RootNode());
      entry = GetNodeEntry(Cursor());
    }
    
    // Check if the cursor is in the visible x-dimension.
    // xs, xe and NodeLength are in pixels (absolute units).

    xs = entry->x;
    xe = xs + draw_settings.NodeLength();

    if (xs < (x_start * PIXELS_PER_SCROLL))
        x_start = xs / PIXELS_PER_SCROLL - 1;

    if (xe > (x_start * PIXELS_PER_SCROLL + width))
        x_start = xe / PIXELS_PER_SCROLL - (width / 2) / PIXELS_PER_SCROLL;

    if (x_start < 0) 
        x_start = 0;

    if (x_start > x_steps)
        x_start = x_steps;
    
    // Check if the cursor is in the visible y-dimension.
    ys = entry->y-10;
    ye = entry->y+10;
    if (ys < y_start*PIXELS_PER_SCROLL)
        y_start = ys/PIXELS_PER_SCROLL-1;
    if (ye > y_start*PIXELS_PER_SCROLL+height)
        y_start = ye/PIXELS_PER_SCROLL-height/PIXELS_PER_SCROLL;
    if (y_start < 0) 
        y_start = 0;
    if (y_start > y_steps)
        y_start = y_steps;
    
    // now update the flasher
    UpdateCursor(entry);
    if (zoom_window)
        zoom_window->UpdateCursor(entry);

    if ((x_start != draw_settings.get_x_scroll()) ||
        (y_start != draw_settings.get_y_scroll()))
    {
        Scroll(x_start, y_start);
        draw_settings.set_x_scroll(x_start);
        draw_settings.set_y_scroll(y_start);
    }

    frame->OnSelectedMoved(Cursor());
}


void TreeWindow::ProcessClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        // check if clicked on a node
        if(x > entry->x &&
           x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
           y > entry->y-DELTA && y < entry->y+DELTA)
             SetCursorPosition((Node *) entry->n);
    }
}

//
// Double clicking on a node will activate the iset hilight function
// Double clicking on any of the displayed labels will call up dialogs to modify
// them. This function is similar to RenderLabels
//

void TreeWindow::ProcessDClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    int id = -1;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        // Check if double clicked on a node
        if (x > entry->x+entry->nums*INFOSET_SPACING &&
            x < entry->x+entry->nums*INFOSET_SPACING+
            draw_settings.NodeLength()-SUBGAME_LARGE_ICON_SIZE &&
            y > entry->y-DELTA && y < entry->y+DELTA) {
	  SetCursorPosition((Node *) entry->n);
	  if (Cursor()->GetInfoset()) // implement iset hilighting
	    frame->HilightInfoset(Cursor()->GetPlayer()->GetNumber(),
				  Cursor()->GetInfoset()->GetNumber(), 1);
	  OnPaint();
	  return;
        }
        // implement subgame toggle (different for collapsed and expanded)
        if (entry->n->GetSubgameRoot() == entry->n)
            if (entry->expanded)
            {
                if(x > entry->x && x < entry->x+SUBGAME_SMALL_ICON_SIZE &&
                   y > entry->y-SUBGAME_SMALL_ICON_SIZE/2 &&
                   y < entry->y+SUBGAME_SMALL_ICON_SIZE/2) {
		  SetCursorPosition((Node *) entry->n);
		  subgame_toggle();
		  OnPaint();
		  return;
                }
            }
            else
            {
                if (x > entry->x+draw_settings.NodeLength()+
                    entry->nums*INFOSET_SPACING-SUBGAME_LARGE_ICON_SIZE &&
                    x < entry->x+draw_settings.NodeLength()+
                    entry->nums*INFOSET_SPACING+SUBGAME_LARGE_ICON_SIZE &&
                    y > entry->y-SUBGAME_LARGE_ICON_SIZE/2 &&
                    y < entry->y+SUBGAME_LARGE_ICON_SIZE/2)
                {
                    SetCursorPosition((Node *) entry->n);
                    subgame_toggle();
                    OnPaint();
                    return;
                }
            }
        
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                id = draw_settings.LabelNodeAbove();
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
                id = draw_settings.LabelNodeBelow();
        }
        if (id != -1)
        {
            SetCursorPosition((Node *) entry->n);
            switch (id)
            {
            case NODE_ABOVE_NOTHING: break;
            case NODE_ABOVE_LABEL: node_label(); break;
            case NODE_ABOVE_PLAYER: tree_players(); break;
            case NODE_ABOVE_ISETLABEL: infoset_switch_player(); break;
            case NODE_ABOVE_ISETID:  infoset_switch_player(); break;
            case NODE_ABOVE_OUTCOME: EditOutcomePayoffs(); break;
            case NODE_ABOVE_REALIZPROB: break;
            case NODE_ABOVE_BELIEFPROB: break;
            case NODE_ABOVE_VALUE: break;
            }
            OnPaint();
            return;
        }
        
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() &&
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                    id = draw_settings.LabelBranchAbove();
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    id = draw_settings.LabelBranchBelow();
            }
        }
        if (id != -1)
        {
            SetCursorPosition((Node *) entry->parent->n);
            switch (id)
            {
            case BRANCH_ABOVE_NOTHING: break;
            case BRANCH_ABOVE_LABEL: action_label(); break;
            case BRANCH_ABOVE_PLAYER: tree_players(); break;
            case BRANCH_ABOVE_PROBS: action_probs(); break;
            case BRANCH_ABOVE_VALUE: break;
            }
            OnPaint();
            return;
        }
        
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  id = draw_settings.LabelNodeRight();
        }
        if (id != -1)
        {
            SetCursorPosition((Node *) entry->n);
            switch (id) {
            case NODE_RIGHT_NOTHING:
	      break;
            case NODE_RIGHT_OUTCOME:
            case NODE_RIGHT_NAME:
	      EditOutcomePayoffs();
	      break;
            }
            OnPaint();
            return;
        }
    }
}

//
// Right Clicking on a label will tell you what the label is refering to
// If the click was not on top of a text label, a popup menu (Buid) is created
//
void TreeWindow::ProcessRClick(wxMouseEvent &ev)
{
// This #define is a hack to prevent a compiler warning:
#define  DONT_USE_LEGENDS_SRC
#include "legend.h"
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *entry;
    char *s = 0;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        entry = node_list[i];
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                s = node_above_src[draw_settings.LabelNodeAbove()].l_name;
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
                s = node_below_src[draw_settings.LabelNodeBelow()].l_name;
        }
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() &&
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                    s = branch_above_src[draw_settings.LabelBranchAbove()].l_name;
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    s = branch_below_src[draw_settings.LabelBranchBelow()].l_name;
            }
        }
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  s = node_right_src[draw_settings.LabelNodeRight()].l_name;
        }
        if (s)
        {
            frame->SetStatusText(s);
            return;
        }
    }
    frame->SetStatusText("");
    // If we got here, the click was NOT on top of a text label, do the menu
    int x_start, y_start;
    ViewStart(&x_start, &y_start);
    PopupMenu(edit_menu, GetDC()->LogicalToDeviceX(x-x_start*PIXELS_PER_SCROLL),
              GetDC()->LogicalToDeviceY(y-y_start*PIXELS_PER_SCROLL));

#undef  DONT_USE_LEGENDS_SRC
}

// Double Right Clicking on a label will let you change what is displayed
void TreeWindow::ProcessRDClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *entry;
    int id = -1;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        entry = node_list[i];
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                id = NODE_ABOVE_LEGEND;
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA) 
                id = NODE_BELOW_LEGEND;
        }
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() && 
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA) 
                    id = BRANCH_ABOVE_LEGEND;
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    id = BRANCH_BELOW_LEGEND;
            }
        }
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  id = NODE_RIGHT_LEGEND;
        }
        if (id != -1)
        {
            draw_settings.SetLegends(id);
            OnPaint();
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
            infosets_changed = TRUE;
            OnPaint();
            return true;
        }

        if (node_cut_entry)  // cut a node
        {
            ef.DeleteTree((Node *)node_cut_entry->n);
            nodes_changed = TRUE;
            OnPaint();
            return true;
        }

        if (outcome_cut_entry)
        {
            ((Node *) outcome_cut_entry->n)->SetOutcome(0);
            outcomes_changed = TRUE;
            OnPaint();
            return true;
        }

        if (branch_cut_entry)  // cut a branch
        {
            ef.DeleteAction(branch_cut_entry->n->GetParent()->GetInfoset(),
                            LastAction((Node *)branch_cut_entry->n));
            nodes_changed = TRUE;
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
    
    return false;
}

void TreeWindow::HilightInfoset(int pl, int iset)
{
    hilight_infoset = 0;

    if (pl >= 1 && pl <= ef.NumPlayers())
    {
        EFPlayer *p = ef.Players()[pl];
        if (iset >= 1 && iset <= p->NumInfosets())
            hilight_infoset = p->Infosets()[iset];
    }

    OnPaint();
}

//
// SupportChanged -- must be called by parent every time the disp_sup
// changes.  Note that since it is a reference, it needs not be passed here.
//
void TreeWindow::SupportChanged(void)
{
  must_recalc = TRUE;
  // Check if the cursor is still valid
  NodeEntry *ne = GetNodeEntry(Cursor());
  if (ne->child_number) {
    if (!disp_sup->Find(Cursor()->GetInfoset()->Actions()[ne->child_number]))
      SetCursorPosition(ef.RootNode());
  }

  OnPaint();
}

void TreeWindow::SetSubgamePickNode(const Node *n)
{
  if (n) {
    // save the actual cursor, and fake a cursor movement to ensure
    // that the node is visible
    Node *cur_cursor = Cursor();
    SetCursorPosition((Node *) n);
    ProcessCursor();
    NodeEntry *ne = GetNodeEntry(n);
    DrawSubgamePickIcon(*GetDC(), *ne);
    SetCursorPosition(cur_cursor);
  }
  subgame_node = n;
}

void TreeWindow::OnPopup(wxMenu &ob, wxCommandEvent &ev)
{
    ((EfgShow *)ob.GetClientData())->OnMenuCommand(ev.commandInt);
}


Node *TreeWindow::GotObject(float &x, float &y, int what)
{
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        
        if (what == DRAG_NODE_START) // check if clicked a non terminal node
            if (entry->n->NumChildren() != 0)
                if(x > entry->x+entry->nums*INFOSET_SPACING &&
                   x < entry->x+draw_settings.NodeLength()+
                   entry->nums*INFOSET_SPACING-10 &&
                   y > entry->y-DELTA && y < entry->y+DELTA) 
                    return (Node *)entry->n;
        
        if (what == DRAG_NODE_END) // check if clicked on a terminal node
            if (entry->n->NumChildren() == 0)
                if(x > entry->x+entry->nums*INFOSET_SPACING &&
                   x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
                   y > entry->y-DELTA && y < entry->y+DELTA)
                    return (Node *)entry->n;
        
        if (what == DRAG_OUTCOME_START) // check if clicked on a terminal node
            if (entry->n->NumChildren() == 0 && entry->n->GetOutcome())
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
            if (entry->n->NumChildren() != 0)
                if(x > entry->x+entry->num*INFOSET_SPACING-4 && 
                   x < entry->x+entry->num*INFOSET_SPACING+4 &&
                   y > entry->y-4 && y < entry->y+4)
                {
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
                y > entry->y-4 && y < entry->y+4)
            {
                x = entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING;
                y = entry->y;
                return (Node *)entry->n;
            }
        
        if (what == DRAG_BRANCH_END)  // check if released in a valid position
        {
            NodeEntry *start_entry = GetNodeEntry(branch_drag->StartNode());
            int xs = start_entry->x+draw_settings.NodeLength()+
                draw_settings.ForkLength()+start_entry->nums*INFOSET_SPACING;
            if (x > xs && x < xs+draw_settings.BranchLength() &&
                y < start_entry->y+(start_entry->n->NumChildren()+1)*draw_settings.YSpacing() &&
                y > start_entry->y-(start_entry->n->NumChildren()+1)*draw_settings.YSpacing())
            {
                // figure out at what branch # the mouse was released
                int br = 1;
                NodeEntry *child_entry, *child_entry1;
                for (int ii = 1; ii <= start_entry->n->NumChildren()-1; ii++)
                {
                    child_entry = GetNodeEntry(start_entry->n->GetChild(ii));
                    if (ii == 1) 
                        if (y < child_entry->y)
                        {
                            br = 1;
                            break;
                        }
                    child_entry1 = GetNodeEntry(start_entry->n->GetChild(ii+1));
                    if (y > child_entry->y && y < child_entry1->y)
                    {
                        br = ii+1;
                        break;
                    }
                    if (ii == start_entry->n->NumChildren()-1 && y > child_entry1->y)
                    {
                        br = start_entry->n->NumChildren()+1;
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

//***********************************************************************
//                      FILE-SAVE MENU HANDLER
//***********************************************************************

Bool TreeWindow::file_save(void)
{
  static int s_nDecimals = 6;
  dialogEfgSave dialog(frame->Filename(), ef.GetTitle(), s_nDecimals, this);

  if (dialog.Completed() == wxOK) {
    if (wxFileExists(dialog.Filename())) {
      if (wxMessageBox("File " + dialog.Filename() + " exists.  Overwrite?",
		       "Confirm", wxOK | wxCANCEL) != wxOK) {
	return FALSE;
      }
    }

    ef.SetTitle(dialog.Label());

    Efg *efg = 0;
    try {
      gFileOutput file(dialog.Filename());
      efg = CompressEfg(ef, *frame->GetSupport(0));
      efg->WriteEfgFile(file, s_nDecimals);
      delete efg;
      frame->SetFileName(dialog.Filename());
    }
    catch (gFileOutput::OpenFailed &) {
      wxMessageBox("Could not open " + dialog.Filename() + " for writing.",
		   "Error", wxOK);
      if (efg)  delete efg;
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageBox("Write error occurred in saving " + dialog.Filename(),
		   "Error", wxOK);
      if (efg)  delete efg;
    }
    catch (Efg::Exception &) {
      wxMessageBox("Internal exception in extensive form", "Error", wxOK);
      if (efg)  delete efg;
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}

void TreeWindow::SetCursorPosition(Node *p_cursor)
{
  m_cursor = p_cursor;
  UpdateMenus();
}

void TreeWindow::UpdateMenus(void)
{
  edit_menu->Enable(efgmenuEDIT_NODE_ADD,
		    (m_cursor->NumChildren() > 0) ? FALSE : TRUE);
  edit_menu->Enable(efgmenuEDIT_NODE_DELETE,
		    (m_cursor->NumChildren() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_MERGE,
		    (mark_node && mark_node->GetInfoset() &&
		     m_cursor->GetInfoset() &&
		     mark_node->GetSubgameRoot() == m_cursor->GetSubgameRoot() &&
		     mark_node->GetPlayer() == m_cursor->GetPlayer()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_BREAK,
		    (m_cursor->GetInfoset()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_SPLIT,
		    (m_cursor->GetInfoset()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_JOIN,
		    (mark_node && mark_node->GetInfoset() &&
		     m_cursor->GetInfoset() &&
		     mark_node->GetSubgameRoot() == m_cursor->GetSubgameRoot()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_LABEL,
		    (m_cursor->GetInfoset()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_PLAYER,
		    (m_cursor->GetInfoset()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_INFOSET_REVEAL,
		    (m_cursor->GetInfoset()) ? TRUE : FALSE);

  edit_menu->Enable(efgmenuEDIT_ACTION_LABEL,
		    (m_cursor->GetInfoset() &&
		     m_cursor->GetInfoset()->NumActions() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_ACTION_INSERT,
		    (m_cursor->NumChildren() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_ACTION_APPEND,
		    (m_cursor->NumChildren() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_ACTION_DELETE,
		    (m_cursor->NumChildren() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_ACTION_PROBS,
		    (m_cursor->GetInfoset() &&
		     m_cursor->GetPlayer()->IsChance()) ? TRUE : FALSE);

  edit_menu->Enable(efgmenuEDIT_TREE_DELETE,
		    (m_cursor->NumChildren() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_TREE_COPY,
		    (mark_node &&
		     m_cursor->GetSubgameRoot() == mark_node->GetSubgameRoot()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_TREE_MOVE,
		    (mark_node &&
		     m_cursor->GetSubgameRoot() == mark_node->GetSubgameRoot()) ? TRUE : FALSE);

  edit_menu->Enable(efgmenuEDIT_OUTCOMES_ATTACH,
		    (ef.NumOutcomes() > 0) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_OUTCOMES_DETACH,
		    (m_cursor->GetOutcome()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_OUTCOMES_LABEL,
		    (m_cursor->GetOutcome()) ? TRUE : FALSE);
  edit_menu->Enable(efgmenuEDIT_OUTCOMES_DELETE,
		    (ef.NumOutcomes() > 0) ? TRUE : FALSE);
  frame->UpdateMenus(m_cursor, mark_node);
}

// This function used to be in the ef and is used frequently
EFPlayer *EfgGetPlayer(const Efg &ef, const gText &name)
{
    for (int i = 1; i <= ef.NumPlayers(); i++)
    {
        if (ef.Players()[i]->GetName() == name) 
            return ef.Players()[i];
    }

    if (ef.GetChance()->GetName() == name) 
        return ef.GetChance();

    return 0;
}

//-----------------------------------------------------------------------
//                    NODE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//-----------------------
// Edit->Node->Add Move
//-----------------------

void TreeWindow::node_add(void)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &ef)  {
    player = 0;
    infoset = 0;
    last_ef = &ef;
  }
    
  dialogMoveAdd dialog(ef, player, infoset, branches, pframe);

  if (dialog.Completed() == wxOK)  {
    NodeAddMode mode = dialog.GetAddMode();
    player = dialog.GetPlayer();
    infoset = dialog.GetInfoset();
    branches = dialog.GetActions();

    try {
      if (mode == NodeAddNew) 
	ef.AppendNode(Cursor(), player, branches);
      else
	ef.AppendNode(Cursor(), infoset);

    nodes_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}

//---------------------------
// Edit->Node->Insert Move
//---------------------------

void TreeWindow::node_insert(void)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &ef)  {
    player = 0;
    infoset = 0;
    last_ef = &ef;
  }
    
  dialogMoveAdd dialog(ef, player, infoset, branches, pframe);

  if (dialog.Completed() == wxOK)  {
    NodeAddMode mode = dialog.GetAddMode();
    player = dialog.GetPlayer();
    infoset = dialog.GetInfoset();
    branches = dialog.GetActions();

    try {
      if (mode == NodeAddNew) 
	ef.InsertNode(Cursor(), player, branches);
      else
	ef.InsertNode(Cursor(), infoset);

      nodes_changed = true;
      SetCursorPosition(Cursor()->GetParent());
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }

}

//----------------------
// Edit->Node->Delete
//----------------------

void TreeWindow::node_delete(void)
{
  try {
    dialogNodeDelete dialog(Cursor(), this);

    if (dialog.Completed() == wxOK) {
      Node *keep = dialog.KeepNode();
      nodes_changed = TRUE;
      SetCursorPosition(ef.DeleteNode(Cursor(), keep));
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//---------------------
// Edit->Node->Label
//---------------------

void TreeWindow::node_label(void)
{
  Node *node = Cursor();
  char *label = wxGetTextFromUser("New label for node", "Label Node",
				  node->GetName());

  if (label)
    node->SetName(label);
}

//------------------------
// Edit->Node->Set Mark
//------------------------

void TreeWindow::node_set_mark(void)
{
  old_mark_node = mark_node;
  if (mark_node != Cursor())
    mark_node = Cursor();
  else
    mark_node = 0;                                   
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

//---------------------------
// Edit->Outcomes->Payoffs
//---------------------------

void TreeWindow::EditOutcomePayoffs(void)
{
  dialogEfgPayoffs dialog(ef, Cursor()->GetOutcome(), false, pframe);

  if (dialog.Completed() == wxOK) {
    EFOutcome *outc = Cursor()->GetOutcome();
    gArray<gNumber> payoffs(dialog.Payoffs());

    if (!outc) {
      outc = ef.NewOutcome();
      Cursor()->SetOutcome(outc);
    }

    for (int pl = 1; pl <= ef.NumPlayers(); pl++)
      ef.SetPayoff(outc, pl, payoffs[pl]);
    outc->SetName(dialog.Name());

    outcomes_changed = true;
  }
}

//--------------------------
// Edit->Outcomes->Attach
//--------------------------

void TreeWindow::EditOutcomeAttach(void)
{
  dialogEfgOutcomeSelect dialog(ef, pframe);
  
  if (dialog.Completed() == wxOK) {
    Cursor()->SetOutcome(dialog.GetOutcome());
    outcomes_changed = true;
    OnPaint();
  }
}

//--------------------------
// Edit->Outcomes->Detach
//--------------------------

void TreeWindow::EditOutcomeDetach(void)
{
  Cursor()->SetOutcome(0);
  outcomes_changed = true;
  OnPaint();
}

//--------------------------
// Edit->Outcomes->Label
//--------------------------

void TreeWindow::EditOutcomeLabel(void)
{
  char *label = wxGetTextFromUser("New outcome label", "Label outcome",
				  Cursor()->GetOutcome()->GetName());

  if (label) {
    Cursor()->GetOutcome()->SetName(label);
    outcomes_changed = true;
  }
}

//-----------------------
// Edit->Outcomes->New
//-----------------------

void TreeWindow::EditOutcomeNew(void)
{
  dialogEfgPayoffs dialog(ef, 0, false, pframe);

  if (dialog.Completed() == wxOK) {
    EFOutcome *outc = ef.NewOutcome();
    gArray<gNumber> payoffs(dialog.Payoffs());

    for (int pl = 1; pl <= ef.NumPlayers(); pl++)
      ef.SetPayoff(outc, pl, payoffs[pl]);
    outc->SetName(dialog.Name());

    outcomes_changed = true;
  }
}

//--------------------------
// Edit->Outcomes->Delete
//--------------------------

void TreeWindow::EditOutcomeDelete(void)
{
  dialogEfgOutcomeSelect dialog(ef, pframe);
  
  if (dialog.Completed() == wxOK) {
    ef.DeleteOutcome(dialog.GetOutcome());
    outcomes_changed = true;
    OnPaint();
  }
}  


//-----------------------------------------------------------------------
//                   INFOSET MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//------------------------
// Edit->Infoset->Merge
//------------------------

void TreeWindow::infoset_merge(void)
{
  try {
    ef.MergeInfoset(mark_node->GetInfoset(), Cursor()->GetInfoset());
    infosets_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//------------------------
// Edit->Infoset->Break
//------------------------

void TreeWindow::infoset_break(void)
{
  try {
    ef.LeaveInfoset(Cursor());
    infosets_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//------------------------
// Edit->Infoset->Split
//------------------------

void TreeWindow::infoset_split(void)
{
  try {
    ef.SplitInfoset(Cursor());
    infosets_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//------------------------
// Edit->Infoset->Join
//------------------------

void TreeWindow::infoset_join(void)
{
  try {
    ef.JoinInfoset(mark_node->GetInfoset(), Cursor());
    infosets_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//------------------------
// Edit->Infoset->Label
//------------------------

void TreeWindow::infoset_label(void)
{
  Infoset *infoset = Cursor()->GetInfoset();
  char *label = wxGetTextFromUser("New label for information set ",
				  "Label Infoset", infoset->GetName());
  if (label)
    infoset->SetName(label);
}

//-------------------------
// Edit->Infoset->Player
//-------------------------

void TreeWindow::infoset_switch_player(void)
{
  try {
    dialogEfgSelectPlayer dialog(ef, this);
        
    if (dialog.Completed() == wxOK) {
      if (dialog.GetPlayer() != Cursor()->GetInfoset()->GetPlayer()) {
	ef.SwitchPlayer(Cursor()->GetInfoset(), dialog.GetPlayer());
	infosets_changed = TRUE;
      }
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//-------------------------
// Edit->Infoset->Reveal
//-------------------------

void TreeWindow::infoset_reveal(void)
{
  dialogInfosetReveal dialog(ef, this);

  if (dialog.Completed() == wxOK) {
    try {
      ef.Reveal(Cursor()->GetInfoset(), dialog.GetPlayers());
      infosets_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}



//-----------------------------------------------------------------------
//                    ACTION MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//------------------------
// Edit->Action->Label
//------------------------

void TreeWindow::action_label(void)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionLabel dialog(infoset, this);
  
  if (dialog.Completed() == wxOK) {
    try {
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->Actions()[act]->SetName(dialog.GetActionLabel(act));
      }
      infosets_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}

//------------------------
// Edit->Action->Insert
//------------------------

void TreeWindow::action_insert(void)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionSelect dialog(infoset, pframe);

  if (dialog.Completed() == wxOK) {
    try {
      ef.InsertAction(infoset, dialog.GetAction());
      nodes_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}

//------------------------
// Edit->Action->Append
//------------------------

void TreeWindow::action_append(void)
{
  try {
    ef.InsertAction(Cursor()->GetInfoset());
    nodes_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//------------------------
// Edit->Action->Delete
//------------------------

void TreeWindow::action_delete(void)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionSelect dialog(infoset, pframe);

  if (dialog.Completed() == wxOK) {
    try {
      ef.DeleteAction(infoset, dialog.GetAction());
      nodes_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}

//-------------------------------
// Edit->Action->Probabilities
//-------------------------------

void TreeWindow::action_probs(void)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionProbs dialog(infoset, pframe);

  if (dialog.Completed() == wxOK) {
    try {
      for (int act = 1; act <= infoset->NumActions(); act++) {
	ef.SetChanceProb(infoset, act, dialog.GetActionProb(act));
      }
      infosets_changed = true;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}


//-----------------------------------------------------------------------
//                     TREE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//---------------------
// Edit->Tree->Label
//---------------------

void TreeWindow::tree_label(void)
{
  char *label = wxGetTextFromUser("Label of game", "Label Game",
				  ef.GetTitle());
  if (label)
    ef.SetTitle(label);
}

//-----------------------
// Edit->Tree->Delete
//-----------------------

void TreeWindow::tree_delete(void)
{
  try {
    if (MyMessageBox("Are you sure?", "Delete Tree",
		     EFG_TREE_HELP, pframe).Completed() == wxOK) {
      ef.DeleteTree(Cursor());
      nodes_changed = true;
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//---------------------
// Edit->Tree->Copy
//---------------------

void TreeWindow::tree_copy(void)
{
  try {
    ef.CopyTree(mark_node, Cursor());
    nodes_changed = true; 
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//--------------------
// Edit->Tree->Move
//--------------------

void TreeWindow::tree_move(void)
{
  try {
    ef.MoveTree(mark_node, Cursor());
    nodes_changed = true;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//-----------------------
// Edit->Tree->Players
//-----------------------

#define PLAYERSD_INST // instantiate the players display dialog
#include "playersd.h"

void TreeWindow::tree_players(void)
{
  PlayerNamesDialog player_names(ef, pframe);
}

//------------------------
// Edit->Tree->Infosets
//------------------------

void TreeWindow::tree_infosets(void)
{
  InfosetDialog ID(ef, pframe);

  if (ID.GameChanged()) 
    infosets_changed = true;
}

//-----------------------------------------------------------------------
//                     SUBGAME MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//----------------------
// Subgames->Mark All
//----------------------

void TreeWindow::SubgameMarkAll(void)
{
  subgame_list.Flush();
  subgame_list.Append(SubgameEntry(ef.RootNode()));
  gList<Node *> subgame_roots;
  LegalSubgameRoots(ef, subgame_roots);
  ef.MarkSubgames(subgame_roots);

  for (int i = 1; i <= subgame_roots.Length(); i++) {
    if (subgame_roots[i] != ef.RootNode())
      subgame_list.Append(SubgameEntry(subgame_roots[i], true));
  }
  
  must_recalc = true;
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

  ef.DefineSubgame(Cursor());
  subgame_list.Append(SubgameEntry(Cursor(), true)); // collapse
  must_recalc = true;
}

void TreeWindow::SubgameUnmark(void)
{
  if (Cursor()->GetSubgameRoot() != Cursor() ||
      Cursor()->GetSubgameRoot() == ef.RootNode())
    return;
    
  ef.RemoveSubgame(Cursor());

  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor())
      subgame_list.Remove(i);
  }

  must_recalc = true;
}

void TreeWindow::SubgameUnmarkAll(void)
{
  ef.UnmarkSubgames(ef.RootNode());
  subgame_list.Flush();
  subgame_list.Append(SubgameEntry(ef.RootNode()));
  must_recalc = true;
}

void TreeWindow::SubgameCollapse(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = false;
      must_recalc = true;
      return;
    }
  }
}

void TreeWindow::SubgameCollapseAll(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++)
    subgame_list[i].expanded = false;

  must_recalc = true;
}

void TreeWindow::SubgameExpand(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = true;
      must_recalc = true;
      return;
    }
  }
}

void TreeWindow::SubgameExpandBranch(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      for (int j = 1; j <= subgame_list.Length(); j++) {
	if (ef.IsSuccessor(subgame_list[j].root, Cursor())) {
	  subgame_list[j].expanded = true;
	  must_recalc = true;
	}
      }

      return;
    }
  }
}

void TreeWindow::SubgameExpandAll(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++)
    subgame_list[i].expanded = true;
  
  must_recalc = true;
}

void TreeWindow::subgame_toggle(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = !subgame_list[i].expanded; 
      must_recalc = true;
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

  if (od.Completed() == wxOK) {
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
  if (!dc) return FALSE;
    
  dc->SetBackgroundMode(wxTRANSPARENT);
  dc->Colour = FALSE;
  Bool color_outcomes;
  
  if (!dc->Colour) {
    color_outcomes = tree->draw_settings.ColorCodedOutcomes();
    tree->draw_settings.SetOutcomeColor(FALSE);
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
    int maxX = tree->draw_settings.MaxX(), maxY = tree->draw_settings.MaxY(); // size of tree
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
    
  return TRUE;
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
    return FALSE;
    
  return TRUE;
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
    printer.Print(pframe, &printout, TRUE);
  }
  else {
    wxPrintPreview *preview = new wxPrintPreview(new ExtensivePrintout(this, fit), new ExtensivePrintout(this, fit));
    wxPreviewFrame *ppframe = new wxPreviewFrame(preview, pframe, "Print Preview", 100, 100, 600, 650);
    ppframe->Centre(wxBOTH);
    ppframe->Initialize();
    ppframe->Show(TRUE);
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
  wxPostScriptDC dc(NULL, TRUE);
  if (dc.Ok()) {
    float old_zoom = 1.0;

    if (fit == wxFITTOPAGE) {
      old_zoom = draw_settings.Zoom();
      int w, h;
      GetSize(&w, &h);
      draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
				 (float)h/(float)draw_settings.MaxY()), true);
    }

    Bool color_outcomes = draw_settings.ColorCodedOutcomes();
    draw_settings.SetOutcomeColor(FALSE);
    dc.StartDoc("Gambit printout");
    dc.StartPage();
    Render(dc);
    dc.EndPage();
    dc.EndDoc();
    if (fit == wxFITTOPAGE)
      draw_settings.SetZoom(old_zoom);
    draw_settings.SetOutcomeColor(color_outcomes);
  }
}

//***********************************************************************
//                      TREE-PRINT MF MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print_mf(wxOutputOption fit, bool save_mf)
{
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
      draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
				 (float)h/(float)draw_settings.MaxY()), true);
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

//***********************************************************************
//                      DISPLAY-ZOOM WINDOW MENU HANDLER
//***********************************************************************
void TreeWindow::display_zoom_win(void)
{
  if (!zoom_window) {
    zoom_window = new TreeZoomWindow(pframe, this, node_list, 
				     (const Infoset *&) hilight_infoset,
				     (const Infoset *&) hilight_infoset1, 
				     (const Node *&) mark_node, 
				     (const Node *&) subgame_node, 
				     draw_settings, GetNodeEntry(Cursor()));
  }
}

void TreeWindow::delete_zoom_win(void)
{
  // We don't delete this because this is called from zoom_window's
  // parent frame... would be bad :)
  // The virtual destructors should result in the window being deleted
  // by the system upon closing, so there ought to be no memory leakage
  zoom_window = 0;
}

//***********************************************************************
//                      DISPLAY-ZOOM MENU HANDLER
//***********************************************************************

void TreeWindow::display_set_zoom(float z)
{
    draw_settings.SetZoom(z);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    must_recalc = TRUE;
}

float TreeWindow::display_get_zoom(void)
{
    return draw_settings.Zoom();
}

#define MAX_WINDOW_WIDTH   750  // assuming an 800x600 display to be safe
#define MAX_WINDOW_HEIGHT  550
#define MIN_WINDOW_WIDTH   600  // at least 12 buttons
#define MIN_WINDOW_HEIGHT  300

void TreeWindow::display_zoom_fit(void)
{
    int width, height;
    Render(*GetDC());
    width = gmin(draw_settings.MaxX(), MAX_WINDOW_WIDTH);
    height = gmin(draw_settings.MaxY(), MAX_WINDOW_HEIGHT);
    
    double zoomx = (double)width/(double)draw_settings.MaxX();
    double zoomy = (double)height/(double)draw_settings.MaxY();
    
    zoomx = gmin(zoomx, 1.0); zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
    double zoom = gmin(zoomx, zoomy); // same zoom for vertical and horiz
    
    width = (int)(zoom*(double)draw_settings.MaxX());
    height = (int)(zoom*(double)draw_settings.MaxY());
    
    width = gmax(width, MIN_WINDOW_WIDTH);
    height = gmax(height, MIN_WINDOW_HEIGHT);
    
    draw_settings.SetZoom(zoom);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    pframe->SetClientSize(width, height+50); // +50 to account for the toolbar
    must_recalc = TRUE;
    ProcessCursor();
    OnPaint();
}


//***********************************************************************
//                      DISPLAY LEGENGS HANDLER
//***********************************************************************
// Controls what gets displayed above/below [node, branch]
void TreeWindow::display_legends(void)
{
    draw_settings.SetLegends();
}

//***********************************************************************
//                      DISPLAY OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts

void TreeWindow::display_options(void)
{
    draw_settings.SetOptions();
    // Must take care of flashing/nonflashing cursor here since draw_settings cannot
    if (draw_settings.FlashingCursor() == TRUE && flasher->Type() == myCursor)
    {
        delete (TreeNodeCursor *)flasher;
        flasher = new TreeNodeFlasher(GetDC());
    }
    
    if (draw_settings.FlashingCursor() == FALSE && flasher->Type() == myFlasher)
    {
        delete (TreeNodeFlasher *)flasher;
        flasher = new TreeNodeCursor(GetDC());
    }

    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY COLORS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_colors(void)
{
    gArray<gText> names(ef.NumPlayers());
    for (int i = 1; i <= names.Length(); i++)
        names[i] = (ef.Players()[i])->GetName();
    draw_settings.PlayerColorDialog(names);
    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY SAVE OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_save_options(Bool def)
{
    if (def)
        draw_settings.SaveOptions();
    else
    {
        char *s = wxFileSelector("Save Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.SaveOptions(copystring(s)); 
            delete [] s;
        }
    }
}

//***********************************************************************
//                      DISPLAY LOAD OPTIONS HANDLER
//***********************************************************************

// Controls the size of the various tree parts
void TreeWindow::display_load_options(Bool def)
{
    if (def)
        draw_settings.LoadOptions(0);
    else
    {
        char *s = wxFileSelector("Load Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.LoadOptions(copystring(s));
            delete [] s;
        }
    }
    
    must_recalc = TRUE;
}


template class gList<NODEENTRY *>;
template class gList<TreeWindow::SUBGAMEENTRY>;
gOutput &operator<<(gOutput &o, const TreeWindow::SUBGAMEENTRY &)
{
    return o;
}

