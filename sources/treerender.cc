//
// FILE: treerender.cc -- Implementation of class TreeRender
//
// $Id$
//

#include "wx/wx.h"
#include "wxmisc.h"
#ifdef wx_msw
#include "wx_mf.h"
#endif  // wx_msw

#include "efg.h"

#include "legendc.h"
#include "twflash.h"
#include "treewin.h"
#include "treezoom.h"
#include "efgshow.h"

int INFOSET_SPACING = 10;
int SUBGAME_LARGE_ICON_SIZE = 20;
int SUBGAME_SMALL_ICON_SIZE = 10;
int SUBGAME_PICK_SIZE = 30;

//-----------------------------------------------------------------------
//                class guiNodeList: Member functions
//-----------------------------------------------------------------------

guiNodeList::guiNodeList(FullEfg &p_efg, TreeRender *p_parent)
  : m_efg(p_efg), m_parent(p_parent)
{
  m_subgameList.Append(SubgameEntry(m_efg.RootNode()));
}

Node *guiNodeList::NodeHitTest(int p_x, int p_y, int p_nodeLength) const
{
  const int DELTA = 8;

  for (int i = 1; i <= Length(); i++) {
    NodeEntry *entry = (*this)[i];

    if (p_x > entry->x &&
	p_x < entry->x + p_nodeLength + entry->nums*INFOSET_SPACING &&
	p_y > entry->y - DELTA &&
	p_y < entry->y + DELTA) {
      return (Node *) entry->n;
    }
  }

  return 0;
}

NodeEntry *guiNodeList::GetValidParent(Node *e)
{
  NodeEntry *n = GetNodeEntry(e->GetParent());
  if (n) {
    return n;
  }
  else { 
    return GetValidParent(e->GetParent());
  }
}

NodeEntry *guiNodeList::GetValidChild(Node *e)
{
  for (int i = 1; i <= e->Game()->NumChildren(e); i++)  {
    NodeEntry *n = GetNodeEntry(e->GetChild(i));
    if (n) {
      return n;
    }
    else  {
      n = GetValidChild(e->GetChild(i));
      if (n) return n;
    }
  }
  return 0;
}

NodeEntry *guiNodeList::GetEntry(Node *p_node) const
{
  for (int i = 1; i <= Length(); i++) {
    if ((*this)[i]->n == p_node) {
      return (*this)[i];
    }
  }
  return 0;
}

Node *guiNodeList::PriorSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = Find(entry) - 1; i >= 1; i--) {
      if ((*this)[i]->level == entry->level)
	return (*this)[i]->n;
    }
  }
  return 0;
}

Node *guiNodeList::NextSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = Find(entry) + 1; i <= Length(); i++) {
      if ((*this)[i]->level == entry->level) { 
	return (*this)[i]->n;
      }
    }
  }
  return 0;
}

SubgameEntry &guiNodeList::GetSubgameEntry(Node *p_node)
{
  for (int i = 1; i <= m_subgameList.Length(); i++) {
    if (m_subgameList[i].root == p_node)  {
      return m_subgameList[i];
    }
  }

  return m_subgameList[1];  // root subgame
}

int guiNodeList::FillTable(Node *n, const EFSupport &cur_sup, int level,
			   int &maxlev, int &maxy, int &miny, int &ycoord)
{
  int y1 = -1, yn=0;
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
    
  SubgameEntry &subgame_entry = GetSubgameEntry(n->GetSubgameRoot());
    
  NodeEntry *entry = new NodeEntry;
  entry->n = n;   // store the node the entry is for
  (*this) += entry;
  entry->in_sup = true;
  if (n->Game()->NumChildren(n)>0 && subgame_entry.expanded) {
    for (int i = 1; i <= n->Game()->NumChildren(n); i++) {
      bool in_sup = true;
      if (n->GetPlayer()->GetNumber())        // pn == 0 for chance nodes
	in_sup = cur_sup.Find(n->GetInfoset()->Actions()[i]);
      if (in_sup) {
	yn = FillTable(n->GetChild(i), cur_sup, level+1,
		       maxlev, maxy, miny, ycoord);
	if (y1 == -1)  y1 = yn;
      }
      else {  // not in the support.
	if (!draw_settings.RootReachable()) {
	  // show only nodes reachable from root
	  // still consider this node, but mark it as invisible
	  yn = FillTable(n->GetChild(i), cur_sup, level+1,
			 maxlev, maxy, miny, ycoord);
	  if (y1 == -1)  y1 = yn;
	  GetNodeEntry(n->GetChild(i))->in_sup = false;
	}
      }
    }
    entry->y = (y1 + yn) / 2;
  }
  else {
    entry->y = ycoord;
    ycoord += draw_settings.YSpacing();
  }
    
  entry->level = level;
  entry->has_children = n->Game()->NumChildren(n);
  // Find out what branch of the parent this node is on
  if (n == m_efg.RootNode()) {
    entry->child_number = 0;
  }
  else {
    Node *parent = n->GetParent();
    for (int i = 1; i <= parent->Game()->NumChildren(parent); i++)
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
  if (n->GetPlayer()) {
    entry->color = draw_settings.GetPlayerColor(n->GetPlayer()->GetNumber());
  }
  else {
    entry->color = draw_settings.GetPlayerColor(-1);
  }  
  
  entry->expanded = subgame_entry.expanded;
    
  maxlev = gmax(level, maxlev);
  maxy = gmax(entry->y, maxy);
  miny = gmin(entry->y, miny);
    
  return entry->y;
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
NodeEntry *guiNodeList::NextInfoset(NodeEntry *e)
{
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  
  for (int pos = Find(e)+1; pos <= Length(); pos++) {
    NodeEntry *e1 = (*this)[pos];
    // infosets are the same and the nodes are on the same level
    if (e->n->GetInfoset() == e1->n->GetInfoset()) {
      if (draw_settings.ShowInfosets() == SHOWISET_ALL) {
	return e1;
      }
      else if (e->level == e1->level) {
	return e1;
      }
    }
  }
  return 0;
}

//
// CheckInfosetEntry.  Checks how many infoset lines are to be drawn at each
// level, spaces them by setting each infoset's node's num to the previous
// infoset node+1.  Also lengthens the nodes by the amount of space taken up
// by the infoset lines.
//
void guiNodeList::CheckInfosetEntry(NodeEntry *e)
{
  int pos;
  NodeEntry *infoset_entry, *e1;
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  // Check if the infoset this entry belongs to (on this level) has already
  // been processed.  If so, make this entry->num the same as the one already
  // processed and return
  infoset_entry = NextInfoset(e);
  for (pos = 1; pos <= Length(); pos++) {
    e1 = (*this)[pos];
    // if the infosets are the same and they are on the same level and e1 has been processed
    if (e->n->GetInfoset() == e1->n->GetInfoset() && 
	e->level == e1->level && e1->num) {
      e->num = e1->num;
      if (infoset_entry) {
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
  for (pos = 1; pos <= Length(); pos++) {
    e1 = (*this)[pos];
    // Find the max num for this level
    if (e->level == e1->level) num = gmax(e1->num, num);
  }
  num++;
  e->num = num;
  e->infoset.y = infoset_entry->y;
  if (draw_settings.ShowInfosets() == SHOWISET_ALL) {
    e->infoset.x = infoset_entry->x;
  }
}

void guiNodeList::FillInfosetTable(Node *n, const EFSupport &cur_sup)
{
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  NodeEntry *entry = GetNodeEntry(n);
  if (n->Game()->NumChildren(n)>0) {
    for (int i = 1; i <= n->Game()->NumChildren(n); i++) {
      bool in_sup = true;
      if (n->GetPlayer()->GetNumber()) {
	in_sup = cur_sup.Find(n->GetInfoset()->Actions()[i]);
      }
            
      if (in_sup || !draw_settings.RootReachable()) {
	FillInfosetTable(n->GetChild(i), cur_sup);
      }
    }
  }

  if (entry) {
    CheckInfosetEntry(entry);
  }
}

void guiNodeList::UpdateTableInfosets(void)
{
  // Note that levels are numbered from 0, not 1.
  // create an array to hold max num for each level
  gArray<int> nums(0, m_maxlev+1); 
    
  for (int i = 0; i <= m_maxlev + 1; nums[i++] = 0);
  NodeEntry *e;
  // find the max e->num for each level
  for (int pos = 1; pos <= Length(); pos++) {
    e = (*this)[pos];
    nums[e->level] = gmax(e->num+1, nums[e->level]);
  }
    
  // record the max e->num for each level for each node
  for (int pos = 1; pos <= Length(); pos++) {
    e = (*this)[pos];
    e->nums = nums[e->level];
  }
    
  for (int i = 0; i <= m_maxlev; i++)  nums[i+1] += nums[i];
    
  // now add the needed length to each level
  for (int pos = 1; pos <= (*this).Length(); pos++) {
    e = (*this)[pos];
    if (e->level != 0) 
      e->x += nums[e->level-1]*INFOSET_SPACING;
  }
}

void guiNodeList::UpdateTableParents(void)
{
  for (int pos = 1; pos <= Length(); pos++) {
    NodeEntry *e = (*this)[pos];
    e->parent = (e->n == m_efg.RootNode()) ? e : GetValidParent(e->n);
    if (!GetValidChild(e->n)) e->has_children = 0;
  }
}

void guiNodeList::Layout(const EFSupport &p_support)
{
  const int TOP_MARGIN = 40;

  Flush();

  int maxlev = 0, miny = 0, maxy = 0, ycoord = TOP_MARGIN;
  FillTable(m_efg.RootNode(), p_support, 0, maxlev, maxy, miny, ycoord);
  m_maxlev = maxlev;

  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  if (draw_settings.ShowInfosets()) {
    // FIXME! This causes lines to disappear... sometimes.
    FillInfosetTable(m_efg.RootNode(), p_support);
    UpdateTableInfosets();
  }

  UpdateTableParents();

  m_maxX = ((maxlev + 1) * (draw_settings.BranchLength() + 
			    draw_settings.ForkLength() + 
			    draw_settings.NodeLength()) + 
	    draw_settings.OutcomeLength());
  m_maxY = maxy + 25;
}

//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     const wxColour &color, int thick = 0)
{
  dc.SetPen(wxPen(color, (thick) ? 8 : 2, wxSOLID));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawRectangle(wxDC &dc, int x_s, int y_s, int w, int h,
                          const wxColour &color)
{
  dc.SetPen(wxPen(color, 2, wxSOLID));
  dc.DrawRectangle(x_s, y_s, w, h);
}

inline void DrawThinLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                         const wxColour &color)
{
  dc.SetPen(wxPen(color, 1, wxSOLID));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawDashedLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
			   const wxColour &color)
{
  dc.SetPen(wxPen(color, 1, wxSHORT_DASH));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawCircle(wxDC &dc, int x, int y, int r, const wxColour &color)
{
  dc.SetPen(wxPen(color, 3, wxSOLID));
  dc.DrawEllipse(x-r, y-r, 2*r, 2*r);
}


void DrawLargeSubgameIcon(wxDC &dc, const NodeEntry &entry, int nl)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen("INDIAN RED", 2, wxSOLID));
  dc.SetBrush(*wxTheBrushList->FindOrCreateBrush("RED", wxSOLID));
  wxPoint points[3];
  int x0 = (entry.x + nl +
	    entry.nums * INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE);
  int y0 = entry.y;
  points[0].x = x0;
  points[0].y = y0;
  points[1].x = x0 + SUBGAME_LARGE_ICON_SIZE;
  points[1].y = y0 - SUBGAME_LARGE_ICON_SIZE/2;
  points[2].x = x0 + SUBGAME_LARGE_ICON_SIZE;
  points[2].y = y0 + SUBGAME_LARGE_ICON_SIZE/2;
  dc.DrawPolygon(3, points);
}

void DrawSmallSubgameIcon(wxDC &dc, const NodeEntry &entry)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen("INDIAN RED", 2, wxSOLID));
  dc.SetBrush(*wxTheBrushList->FindOrCreateBrush("RED", wxSOLID));
  wxPoint points[3];
  points[0].x = entry.x;
  points[0].y = entry.y;
  points[1].x = entry.x+SUBGAME_SMALL_ICON_SIZE;
  points[1].y = entry.y-SUBGAME_SMALL_ICON_SIZE/2;
  points[2].x = entry.x+SUBGAME_SMALL_ICON_SIZE;
  points[2].y = entry.y+SUBGAME_SMALL_ICON_SIZE/2;
  dc.DrawPolygon(3, points);
}

void DrawSubgamePickIcon(wxDC &dc, const NodeEntry &entry)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen("BLACK", 2, wxSOLID));
  dc.DrawLine(entry.x, entry.y - SUBGAME_PICK_SIZE/2,
	      entry.x, entry.y + SUBGAME_PICK_SIZE/2);
  dc.DrawLine(entry.x, entry.y - SUBGAME_PICK_SIZE/2,
	      entry.x + SUBGAME_PICK_SIZE/2, entry.y - SUBGAME_PICK_SIZE/2);
  dc.DrawLine(entry.x, entry.y + SUBGAME_PICK_SIZE/2,
	      entry.x + SUBGAME_PICK_SIZE/2, entry.y + SUBGAME_PICK_SIZE/2);
}


//-----------------------------------------------------------------------
//                        TreeRender: Life cycle
//-----------------------------------------------------------------------

BEGIN_EVENT_TABLE(TreeRender, wxScrolledWindow)
  EVT_PAINT(TreeRender::OnPaint)
END_EVENT_TABLE()

TreeRender::TreeRender(wxWindow *frame, TreeWindow *parent_)
  : wxScrolledWindow(frame),
    parent(parent_), flasher(0), painting(false)
{ }

TreeRender::~TreeRender(void)
{
  if (flasher)  {
    delete flasher;
  }
}

void TreeRender::OnPaint(wxPaintEvent &)
{
  if (painting) 
    return; // prevent re-entry
  painting = true;
  wxPaintDC dc(this);
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.SetUserScale(GetZoom(), GetZoom());
  Render(dc);
  painting = false;
}

//
// RenderLabels:  Draws all the text labels for the tree according to the
// settings in draw_settings.  Currently takes care of:labels node/branch,
// outcomes.  Note: this function is getting very long, but I see no real
// reason to split it at this point...
//
void TreeRender::RenderLabels(wxDC &dc, const NodeEntry *child_entry,
                              const NodeEntry *entry)
{
  gText label = "";     // temporary to hold the label
  const Node *n = child_entry->n;
  long tw, th;
  bool hilight = false;
    
  // First take care of labeling the node on top.
  switch (DrawSettings().LabelNodeAbove()) {
  case NODE_ABOVE_NOTHING:
    label ="";  
    break;
        
  case NODE_ABOVE_LABEL:
    label = n->GetName();  
    break;
        
  case NODE_ABOVE_PLAYER:
    if (n->GetPlayer()) 
      label = n->GetPlayer()->GetName();
    else
      label = "";
    break;
        
  case NODE_ABOVE_ISETLABEL:
    if (n->GetInfoset()) 
      label = n->GetInfoset()->GetName();
    break;
        
  case NODE_ABOVE_ISETID:
    if (n->GetInfoset())
      label = ("(" + ToText(n->GetPlayer()->GetNumber()) +
	       "," + ToText(n->GetInfoset()->GetNumber()) + ")");
    break;
    
  case NODE_ABOVE_OUTCOME:
    label = OutcomeAsString(n, hilight);
    break;
        
  case NODE_ABOVE_REALIZPROB:
    label = parent->AsString(tRealizProb, n);
    break;
        
  case NODE_ABOVE_BELIEFPROB:
    label = parent->AsString(tBeliefProb, n);
    break;
        
  case NODE_ABOVE_VALUE:
    label = parent->AsString(tNodeValue, n);
    break;
        
  default:
    label = "";
    break;
  }
    
  if (label != "") {
    dc.SetFont(DrawSettings().NodeAboveFont());
    dc.GetTextExtent("0", &tw, &th);
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y - th - 9);
  }
  
  // Take care of labeling the node on the bottom.
  switch (DrawSettings().LabelNodeBelow()) { 
  case NODE_BELOW_NOTHING:
    label = "";
    break;
        
  case NODE_BELOW_LABEL:
    label = n->GetName();
    break;
        
  case NODE_BELOW_PLAYER:
    if (n->GetPlayer()) 
      label = n->GetPlayer()->GetName(); 
    else 
      label = ""; 
    break;
        
  case NODE_BELOW_ISETLABEL:
    if (n->GetInfoset()) 
      label = n->GetInfoset()->GetName();
    break;
    
  case NODE_BELOW_ISETID:
    if (n->GetInfoset())
      label = ("(" + ToText(n->GetPlayer()->GetNumber()) +
	       "," + ToText(n->GetInfoset()->GetNumber()) + ")");
    break;
        
  case NODE_BELOW_OUTCOME:
    label = OutcomeAsString(n, hilight);
    break;
        
  case NODE_BELOW_REALIZPROB:
    label = parent->AsString(tRealizProb, n);
    break;
        
  case NODE_BELOW_BELIEFPROB:
    label = parent->AsString(tBeliefProb, n);
    break;
        
  case NODE_BELOW_VALUE:
    label = parent->AsString(tNodeValue, n);
    break;
        
  default:
    label = "";
    break;
  }
    
  if (label != "") {
    dc.SetFont(DrawSettings().NodeBelowFont());
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y + 5);
  }
  
  if (child_entry->n != entry->n) {   // no branches for root
    // Now take care of branches....
    // Take care of labeling the branch on the top.
    switch (DrawSettings().LabelBranchAbove()) {
    case BRANCH_ABOVE_NOTHING:
      label = "";
      break;
            
    case BRANCH_ABOVE_LABEL:
      if (child_entry->child_number != 0)
	label = entry->n->GetInfoset()->GetActionName(child_entry->child_number);
      else
	label = "";
      break;
            
    case BRANCH_ABOVE_PLAYER:
      if (entry->n->GetPlayer()) 
	label = entry->n->GetPlayer()->GetName();
      break;
            
    case BRANCH_ABOVE_PROBS:
      label = parent->AsString(tBranchProb, entry->n, child_entry->child_number);
      break;
      
    case BRANCH_ABOVE_VALUE:
      label = parent->AsString(tBranchVal, entry->n, child_entry->child_number);
      break;
            
    default:
      label = "";
      break;
    }
    
    if (label != "") {
      dc.SetFont(DrawSettings().BranchAboveFont());
      dc.GetTextExtent("0", &tw, &th);
      gDrawText(dc, label, 
		entry->x + entry->nums * INFOSET_SPACING + 
		DrawSettings().ForkLength() + DrawSettings().NodeLength() + 3,
		child_entry->y - th - 5);
    }
        
    // Take care of labeling the branch on the bottom.
    switch (DrawSettings().LabelBranchBelow()) { 
    case BRANCH_BELOW_NOTHING:
      label = "";
      break;
            
    case BRANCH_BELOW_LABEL:
      if (child_entry->child_number != 0)
	label = entry->n->GetInfoset()->GetActionName(child_entry->child_number);
      else
	label = "";
      break;
            
    case BRANCH_BELOW_PLAYER:
      if (entry->n->GetPlayer()) 
	label = entry->n->GetPlayer()->GetName();
      break;
      
    case BRANCH_BELOW_PROBS:
      label = parent->AsString(tBranchProb, entry->n, child_entry->child_number);
      break;
            
    case BRANCH_BELOW_VALUE:
      label = parent->AsString(tBranchVal, entry->n, child_entry->child_number);
      break;
            
    default:
      label = "";
      break;
    }
        
    if (label != "") {
      dc.SetFont(DrawSettings().BranchBelowFont());
      gDrawText(dc, label,
		entry->x + entry->nums * INFOSET_SPACING + 
		DrawSettings().ForkLength() + DrawSettings().NodeLength() + 3,
		child_entry->y + 5);
    }
  }
    
  // Now take care of displaying the terminal node labels.
  hilight = false;
    
  switch (DrawSettings().LabelNodeRight()) { 
  case NODE_RIGHT_NOTHING:
    label = "";
    break;
            
  case NODE_RIGHT_OUTCOME:
    label = OutcomeAsString(n, hilight);
    break;
            
  case NODE_RIGHT_NAME:
    if (n->GetOutcome()) 
      label = n->GetOutcome()->GetName();
    break;
            
  default:
    label = "";
    break;
  }
        
  if (label != "") { 
    dc.SetFont(DrawSettings().NodeRightFont());
    gDrawText(dc, label,
	      child_entry->x + DrawSettings().NodeLength() +
	      child_entry->nums * INFOSET_SPACING + 10,
	      child_entry->y - 12);
  }
}


//
// RenderSubtree: Render branches and labels
//
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.  Since the region clipping implemented by wxwin seems
// to be less than optimal, I add rudimentary clipping of my own.
// The offset is used to simulate scrollbars in the
// zoom window.  It might be used for the main window if scrollbars prove to
// be a limitation.
//
void TreeRender::RenderSubtree(wxDC &dc)
{
  // x-start, x-end, y-start, y-end: coordinates for drawing branches 
  int xs, xe, ys, ye;
  NodeEntry entry, child_entry;
  // Determine the visible region on screen to implement clipping
  int x_start, y_start, width, height;

  wxPoint deviceOrigin = dc.GetDeviceOrigin();
  
  ViewStart(&x_start, &y_start);
  GetClientSize(&width, &height);
  
  // go through the list of nodes, plotting them
  for (int pos = 1; pos <= NodeList().Length(); pos++) {
    child_entry = *NodeList()[pos];    // must make a copy to use Translate
    entry = *child_entry.parent;
        
    float zoom = GetZoom();

    // Check if this node/labels are visible
    if (!(child_entry.x+deviceOrigin.x < x_start*PIXELS_PER_SCROLL  ||
	  entry.x+deviceOrigin.x > x_start*PIXELS_PER_SCROLL+width/zoom ||
	  (entry.y+deviceOrigin.y > y_start*PIXELS_PER_SCROLL+height/zoom &&
	   child_entry.y+deviceOrigin.y > y_start*PIXELS_PER_SCROLL+height/zoom) ||
	  (entry.y+deviceOrigin.y < y_start*PIXELS_PER_SCROLL && 
	   child_entry.y+deviceOrigin.y < y_start*PIXELS_PER_SCROLL)) ||
	(entry.infoset.y+deviceOrigin.y < y_start*PIXELS_PER_SCROLL+height/zoom)) {

      // draw the labels
      RenderLabels(dc, &child_entry, &entry);

      // Draw a triangle to show subgame roots
      if (entry.n->GetSubgameRoot() == entry.n) {
	if (entry.expanded) 
	  DrawSmallSubgameIcon(dc, entry);
      }

      // Only draw the node line once for all children.
      if (child_entry.child_number == 1) {
	// draw the 'node' line
	bool hilight = 
	  (HighlightInfoset()  && (entry.n->GetInfoset() == HighlightInfoset())) ||
	  (HighlightInfoset1() && (entry.n->GetInfoset() == HighlightInfoset1()));
	::DrawLine(dc, entry.x, entry.y,
		   entry.x + DrawSettings().NodeLength() + 
		   entry.nums * INFOSET_SPACING, entry.y, 
		   entry.color, hilight ? 1 : 0);
	
	// show the infoset lines, if required by draw settings
	::DrawCircle(dc, entry.x + entry.num * INFOSET_SPACING, entry.y, 
		     3, entry.color);
      }
      
      if (child_entry.n == SubgameNode())
	DrawSubgamePickIcon(dc, child_entry);
    
      // draw the 'branches'
      if (child_entry.n->GetParent() && child_entry.in_sup) {
	// no branches for root node
	xs = entry.x+DrawSettings().NodeLength()+entry.nums*INFOSET_SPACING;
	ys = entry.y;
	xe = xs+DrawSettings().ForkLength();
	ye = child_entry.y;
	::DrawLine(dc, xs, ys, xe, ye, entry.color);

	// Draw the highlight... y = a + bx = ys + (ye-ys) / (xe-xs) * x
	double prob = parent->Parent()->ActionProb(entry.n,
						   child_entry.child_number);
	if (prob > 0) {
	  ::DrawLine(dc, xs, ys, (xs + DrawSettings().ForkLength() * prob), 
		     (ys + (ye - ys) * prob), *wxBLACK);
	}
	
	xs = xe;
	ys = ye;
	xe = child_entry.x;
	ye = ys;
	::DrawLine(dc, xs, ye, xe, ye, entry.color);
      }
      else {
	xe = entry.x;
	ye = entry.y;
      }
      
      // Take care of terminal nodes
      // (either real terminal or collapsed subgames)
      if (!child_entry.has_children) { 
	::DrawLine(dc, xe, ye, 
		   xe + DrawSettings().NodeLength() + 
		   child_entry.nums * INFOSET_SPACING, 
		   ye, DrawSettings().GetPlayerColor(-1));
      
	// Collapsed subgame: subgame icon is drawn at this terminal node.
	if ((child_entry.n->GetSubgameRoot() == child_entry.n) && 
	    !child_entry.expanded)
	  DrawLargeSubgameIcon(dc, child_entry, DrawSettings().NodeLength());
      
	// Marked Node: a circle is drawn at this terminal node
	if (child_entry.n == MarkNode()) {
	  ::DrawCircle(dc, 
		       xe + child_entry.nums * INFOSET_SPACING +
		       DrawSettings().NodeLength(), ye, 
		       4, DrawSettings().CursorColor());
	}
      }

      // Draw a circle to show the marked node
      if ((entry.n == MarkNode()) && 
	  (child_entry.child_number == entry.n->Game()->NumChildren(entry.n))) {
	::DrawCircle(dc, entry.x + entry.nums * INFOSET_SPACING + 
		     DrawSettings().NodeLength(), entry.y, 
		     4, DrawSettings().CursorColor());
      }
    }

    if (child_entry.child_number == 1) {
      if (DrawSettings().ShowInfosets()) {
	if (entry.infoset.y != -1) {
	  ::DrawThinLine(dc, 
			 entry.x + entry.num * INFOSET_SPACING, 
			 entry.y, 
			 entry.x + entry.num * INFOSET_SPACING, 
			 entry.infoset.y, 
			 entry.color);
	}
	  
	if (entry.infoset.x != -1) {
	  // Draw a little arrow in the direction of the iset.
	  if (entry.infoset.x > entry.x) { // iset is to the right
	    ::DrawLine(dc, 
		       entry.x+entry.num*INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x+(entry.num+1)*INFOSET_SPACING, 
		       entry.infoset.y, entry.color);
	  }
	  else {  // iset is to the left
	    ::DrawLine(dc, 
		       entry.x + entry.num * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x + (entry.num - 1) * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.color);
	  }
	}
      }
    }
  }
}

void TreeRender::UpdateCursor(void)
{
  NodeEntry *entry = NodeList().GetNodeEntry(Cursor());

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

  flasher->Flash();
}

void TreeRender::Render(wxDC &dc)
{ 
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();
  RenderSubtree(dc);
}

void TreeRender::MakeFlasher(void)
{
  flasher = ((DrawSettings().FlashingCursor()) ? new TreeNodeFlasher(this) :
	     new TreeNodeCursor(this));
}

gText TreeRender::OutcomeAsString(const Node *n, bool &/*hilight*/) const
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



//----------------------------------------------------------------------
//                   TreeZoomWindow: Member functions
//----------------------------------------------------------------------

TreeZoomWindow::TreeZoomWindow(wxFrame *frame, TreeWindow *parent)
  : TreeRender(frame, parent),
    m_parent(parent), m_zoom(1.0)
{
  MakeFlasher();
  UpdateCursor();
  Show(false);
}

//
// This Render function takes into account the current position of the cursor.
// Calculates ox and oy so that the cursor is located in the middle of the
// window.
//
void TreeZoomWindow::Render(wxDC &dc)
{
  int width, height;
  GetClientSize(&width, &height);
  width = (int)(width/m_zoom);
  height = (int)(height/m_zoom);

  // coordinates of the middle of the cursor
  int xm = (xs+xe)/2, ym = (ys+ye)/2;
  ox = width/2-xm;
  oy = height/2-ym;
  dc.SetDeviceOrigin(0, 0); // should not be necessary, but its a bug
  Clear();
  dc.SetDeviceOrigin(ox * m_zoom, oy * m_zoom);

  TreeRender::Render(dc);
  flasher->Flash(dc);
}

void TreeZoomWindow::UpdateCursor(void)
{
  TreeRender::UpdateCursor();
  flasher->GetFlashNode(xs, ys, xe, ye);
  wxClientDC dc(this);
  dc.SetUserScale(m_zoom, m_zoom);
  Render(dc);
}

void TreeZoomWindow::SetZoom(float p_zoom)
{
  m_zoom = p_zoom;
}

float TreeZoomWindow::GetZoom(void) const
{
  return m_zoom;
}

int TreeZoomWindow::NumDecimals(void) const
{ return parent->NumDecimals(); }

Infoset *TreeZoomWindow::HighlightInfoset(void) const
{ return parent->HighlightInfoset(); }

Infoset *TreeZoomWindow::HighlightInfoset1(void) const
{ return parent->HighlightInfoset1(); }

Node *TreeZoomWindow::MarkNode(void) const
{ return parent->MarkNode(); }

Node *TreeZoomWindow::SubgameNode(void) const
{ return parent->SubgameNode(); }

guiNodeList &TreeZoomWindow::NodeList(void)
{ return parent->NodeList(); }

const TreeDrawSettings &TreeZoomWindow::DrawSettings(void) const
{ return parent->DrawSettings(); }

Node *TreeZoomWindow::Cursor(void) const
{ return parent->Cursor(); }
