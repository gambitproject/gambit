//
// FILE: efglayout.cc -- Implementation of extensive form layout
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"
#include "math/math.h"

#include "efg.h"

#include "legendc.h"
#include "twflash.h"
#include "treewin.h"
#include "efgshow.h"

int INFOSET_SPACING = 10;
int SUBGAME_LARGE_ICON_SIZE = 20;
int SUBGAME_SMALL_ICON_SIZE = 10;
int SUBGAME_PICK_SIZE = 30;
int DELTA = 8;
int MAX_TW = 60;
int MAX_TH = 20;

//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     const wxColour &color, int thick = 0)
{
  dc.SetPen(wxPen(color, (thick) ? 8 : 2, wxSOLID));
  dc.DrawLine((int) x_s, (int) y_s, (int) x_e, (int) y_e);
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
//                class efgTreeLayout: Member functions
//-----------------------------------------------------------------------

efgTreeLayout::efgTreeLayout(FullEfg &p_efg, TreeWindow *p_parent)
  : m_efg(p_efg), m_parent(p_parent)
{
  m_subgameList.Append(SubgameEntry(m_efg.RootNode()));
}

Node *efgTreeLayout::NodeHitTest(int p_x, int p_y) const
{
  const int DELTA = 8;

  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (p_x > entry->x &&
	p_x < (entry->x + m_parent->DrawSettings().NodeLength()
	       + entry->nums*INFOSET_SPACING) &&
	p_y > entry->y - DELTA &&
	p_y < entry->y + DELTA) {
      return entry->n;
    }
  }

  return 0;
}

Node *efgTreeLayout::SubgameHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->n->GetSubgameRoot() == entry->n)  {
      if (entry->expanded &&
	  p_x > entry->x &&
	  p_x < entry->x + SUBGAME_SMALL_ICON_SIZE &&
	  p_y > entry->y - SUBGAME_SMALL_ICON_SIZE/2 &&
	  p_y < entry->y + SUBGAME_SMALL_ICON_SIZE/2) {
	return entry->n;
      }
      else if (!entry->expanded && 
	       p_x > (entry->x + m_parent->DrawSettings().NodeLength() +
		      entry->nums*INFOSET_SPACING-SUBGAME_LARGE_ICON_SIZE) &&
	       p_x < (entry->x + m_parent->DrawSettings().NodeLength() + 
		      entry->nums*INFOSET_SPACING+SUBGAME_LARGE_ICON_SIZE) &&
	       p_y > entry->y-SUBGAME_LARGE_ICON_SIZE/2 &&
	       p_y < entry->y+SUBGAME_LARGE_ICON_SIZE/2) {
	return entry->n;
      }
    }
  }
  return 0;
}

Node *efgTreeLayout::NodeAboveHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (p_x > entry->x + entry->nums * INFOSET_SPACING + 3 &&
	p_x < entry->x + MAX_TW &&
	p_y > entry->y - MAX_TH && 
	p_y < entry->y + DELTA) {
      return entry->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::NodeBelowHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (p_x > entry->x + entry->nums * INFOSET_SPACING + 3 &&
	p_x < entry->x + MAX_TW &&
	p_y > entry->y + DELTA && 
	p_y < entry->y + MAX_TH + DELTA)  {
      return entry->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::NodeRightHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (p_x > entry->x + m_parent->DrawSettings().NodeLength() + 10 &&
	p_x < (entry->x + m_parent->DrawSettings().NodeLength() + 10 +
	       m_parent->DrawSettings().OutcomeLength() * 3) &&
	p_y > entry->y - DELTA &&
	p_y < entry->y + DELTA) {
      return entry->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::BranchHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    NodeEntry *parent_entry = GetNodeEntry(entry->n->GetParent());

    if (parent_entry) {
      if (p_x > (parent_entry->x + m_parent->DrawSettings().NodeLength() + 
		 parent_entry->num * INFOSET_SPACING + 10) &&
	  p_x < (parent_entry->x + m_parent->DrawSettings().NodeLength() +
		 m_parent->DrawSettings().ForkLength() +
		 parent_entry->num * INFOSET_SPACING)) {
	// Good old slope/intercept method for finding a point on a line
	int y0 = (parent_entry->y + 
		  (int) (p_x - parent_entry->x - 
			 m_parent->DrawSettings().NodeLength() -
			 parent_entry->nums * INFOSET_SPACING) *
		  (entry->y - parent_entry->y) / 
		  m_parent->DrawSettings().ForkLength());

	if (p_y > y0-2 && p_y < y0+2) {
	  return entry->n;
	}
      }
    }
  }
  return 0;
}



Node *efgTreeLayout::BranchAboveHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->parent &&
	p_x > entry->x - m_parent->DrawSettings().BranchLength() &&
	p_x < entry->x - m_parent->DrawSettings().BranchLength() + MAX_TW &&
	p_y > entry->y - MAX_TH &&
	p_y < entry->y + DELTA) {
      return entry->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::BranchBelowHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->parent &&
	p_x > entry->x - m_parent->DrawSettings().BranchLength() &&
	p_x < entry->x - m_parent->DrawSettings().BranchLength() + MAX_TW &&
	p_y > entry->y + DELTA && 
	p_y < entry->y + MAX_TH) {
      return entry->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    if (entry->infoset.y != -1 && entry->n->GetInfoset()) {
      if (p_x > entry->x + entry->num * INFOSET_SPACING - 2 &&
	  p_x < entry->x + entry->num * INFOSET_SPACING + 2) {
	if (p_y > entry->y && p_y < entry->infoset.y) {
	  // next iset is below this one
	  return entry->n;
	}
	else if (p_y > entry->infoset.y && p_y < entry->y) {
	  // next iset is above this one
	  return entry->n;
	}
      }
    }
  }
  return 0;
}


NodeEntry *efgTreeLayout::GetValidParent(Node *e)
{
  NodeEntry *n = GetNodeEntry(e->GetParent());
  if (n) {
    return n;
  }
  else { 
    return GetValidParent(e->GetParent());
  }
}

NodeEntry *efgTreeLayout::GetValidChild(Node *e)
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

NodeEntry *efgTreeLayout::GetEntry(Node *p_node) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->n == p_node) {
      return m_nodeList[i];
    }
  }
  return 0;
}

Node *efgTreeLayout::PriorSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) - 1; i >= 1; i--) {
      if (m_nodeList[i]->level == entry->level)
	return m_nodeList[i]->n;
    }
  }
  return 0;
}

Node *efgTreeLayout::NextSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) + 1; i <= m_nodeList.Length(); i++) {
      if (m_nodeList[i]->level == entry->level) { 
	return m_nodeList[i]->n;
      }
    }
  }
  return 0;
}

SubgameEntry &efgTreeLayout::GetSubgameEntry(Node *p_node)
{
  for (int i = 1; i <= m_subgameList.Length(); i++) {
    if (m_subgameList[i].root == p_node)  {
      return m_subgameList[i];
    }
  }

  return m_subgameList[1];  // root subgame
}

int efgTreeLayout::FillTable(Node *n, const EFSupport &cur_sup, int level,
			   int &maxlev, int &maxy, int &miny, int &ycoord)
{
  int y1 = -1, yn=0;
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
    
  SubgameEntry &subgame_entry = GetSubgameEntry(n->GetSubgameRoot());
    
  NodeEntry *entry = new NodeEntry;
  entry->n = n;   // store the node the entry is for
  m_nodeList += entry;
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
  if (n->GetPlayer() && n->GetPlayer()->IsChance()) {
    entry->color = wxGetApp().GetPreferences().GetChanceColor();
  }
  else if (n->GetPlayer()) {
    entry->color = wxGetApp().GetPreferences().GetPlayerColor(n->GetPlayer()->GetNumber());
  }
  else {
    entry->color = wxGetApp().GetPreferences().GetTerminalColor();
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
NodeEntry *efgTreeLayout::NextInfoset(NodeEntry *e)
{
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  
  for (int pos = m_nodeList.Find(e) + 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e1 = m_nodeList[pos];
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
void efgTreeLayout::CheckInfosetEntry(NodeEntry *e)
{
  int pos;
  NodeEntry *infoset_entry, *e1;
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  // Check if the infoset this entry belongs to (on this level) has already
  // been processed.  If so, make this entry->num the same as the one already
  // processed and return
  infoset_entry = NextInfoset(e);
  for (pos = 1; pos <= m_nodeList.Length(); pos++) {
    e1 = m_nodeList[pos];
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
  for (pos = 1; pos <= m_nodeList.Length(); pos++) {
    e1 = m_nodeList[pos];
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

void efgTreeLayout::FillInfosetTable(Node *n, const EFSupport &cur_sup)
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

void efgTreeLayout::UpdateTableInfosets(void)
{
  // Note that levels are numbered from 0, not 1.
  // create an array to hold max num for each level
  gArray<int> nums(0, m_maxlev+1); 
    
  for (int i = 0; i <= m_maxlev + 1; nums[i++] = 0);
  NodeEntry *e;
  // find the max e->num for each level
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    e = m_nodeList[pos];
    nums[e->level] = gmax(e->num+1, nums[e->level]);
  }
    
  // record the max e->num for each level for each node
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    e = m_nodeList[pos];
    e->nums = nums[e->level];
  }
    
  for (int i = 0; i <= m_maxlev; i++)  nums[i+1] += nums[i];
    
  // now add the needed length to each level, and set maxX
  m_maxX = 0;
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    e = m_nodeList[pos];
    if (e->level != 0) {
      e->x += nums[e->level-1]*INFOSET_SPACING;
    }
    m_maxX = gmax(m_maxX, e->x);
  }
}

void efgTreeLayout::UpdateTableParents(void)
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e = m_nodeList[pos];
    e->parent = (e->n == m_efg.RootNode()) ? e : GetValidParent(e->n);
    if (!GetValidChild(e->n)) e->has_children = 0;
  }
}

void efgTreeLayout::Layout(const EFSupport &p_support)
{
  const int TOP_MARGIN = 40;

  while (m_nodeList.Length() > 0) {
    delete m_nodeList.Remove(1);
  }

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

  m_maxX += draw_settings.NodeLength() + draw_settings.OutcomeLength();
  m_maxY = maxy + 25;
}

//
// RenderLabels:  Draws all the text labels for the tree according to the
// settings in draw_settings.  Currently takes care of:labels node/branch,
// outcomes.  Note: this function is getting very long, but I see no real
// reason to split it at this point...
//
void efgTreeLayout::RenderLabels(wxDC &dc, const NodeEntry *child_entry,
			       const NodeEntry *entry) const
{
  gText label = "";     // temporary to hold the label
  const Node *n = child_entry->n;
  long tw, th;
  bool hilight = false;
    
  // First take care of labeling the node on top.
  switch (m_parent->DrawSettings().LabelNodeAbove()) {
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
    label = m_parent->OutcomeAsString(n, hilight);
    break;
        
  case NODE_ABOVE_REALIZPROB:
    label = m_parent->Parent()->GetRealizProb(n);
    break;
        
  case NODE_ABOVE_BELIEFPROB:
    label = m_parent->Parent()->GetBeliefProb(n);
    break;
        
  case NODE_ABOVE_VALUE:
    label = m_parent->Parent()->GetNodeValue(n);
    break;
        
  default:
    label = "";
    break;
  }
    
  if (label != "") {
    dc.SetFont(m_parent->DrawSettings().NodeAboveFont());
    dc.GetTextExtent("0", &tw, &th);
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y - th - 9);
  }
  
  // Take care of labeling the node on the bottom.
  switch (m_parent->DrawSettings().LabelNodeBelow()) { 
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
    label = m_parent->OutcomeAsString(n, hilight);
    break;
        
  case NODE_BELOW_REALIZPROB:
    label = m_parent->Parent()->GetRealizProb(n);
    break;
        
  case NODE_BELOW_BELIEFPROB:
    label = m_parent->Parent()->GetBeliefProb(n);
    break;
        
  case NODE_BELOW_VALUE:
    label = m_parent->Parent()->GetNodeValue(n);
    break;
        
  default:
    label = "";
    break;
  }
    
  if (label != "") {
    dc.SetFont(m_parent->DrawSettings().NodeBelowFont());
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y + 5);
  }
  
  if (child_entry->n != entry->n) {   // no branches for root
    // Now take care of branches....
    // Take care of labeling the branch on the top.
    switch (m_parent->DrawSettings().LabelBranchAbove()) {
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
      label = m_parent->Parent()->GetActionProb(entry->n, child_entry->child_number);
      break;
      
    case BRANCH_ABOVE_VALUE:
      label = m_parent->Parent()->GetActionValue(entry->n, child_entry->child_number);
      break;
            
    default:
      label = "";
      break;
    }
    
    if (label != "") {
      dc.SetFont(m_parent->DrawSettings().BranchAboveFont());
      dc.GetTextExtent("0", &tw, &th);
      gDrawText(dc, label, 
		entry->x + entry->nums * INFOSET_SPACING + 
		m_parent->DrawSettings().ForkLength() +
		m_parent->DrawSettings().NodeLength() + 3,
		child_entry->y - th - 5);
    }
        
    // Take care of labeling the branch on the bottom.
    switch (m_parent->DrawSettings().LabelBranchBelow()) { 
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
      label = m_parent->Parent()->GetActionProb(entry->n, child_entry->child_number);
      break;
            
    case BRANCH_BELOW_VALUE:
      label = m_parent->Parent()->GetActionValue(entry->n, child_entry->child_number);
      break;
            
    default:
      label = "";
      break;
    }
        
    if (label != "") {
      dc.SetFont(m_parent->DrawSettings().BranchBelowFont());
      gDrawText(dc, label,
		entry->x + entry->nums * INFOSET_SPACING + 
		m_parent->DrawSettings().ForkLength() +
		m_parent->DrawSettings().NodeLength() + 3,
		child_entry->y + 5);
    }
  }
    
  // Now take care of displaying the terminal node labels.
  hilight = false;
    
  switch (m_parent->DrawSettings().LabelNodeRight()) { 
  case NODE_RIGHT_NOTHING:
    label = "";
    break;
            
  case NODE_RIGHT_OUTCOME:
    label = m_parent->OutcomeAsString(n, hilight);
    break;
            
  case NODE_RIGHT_NAME:
    if (!n->Game()->GetOutcome(n).IsNull()) {
      label = m_efg.GetOutcomeName(n->Game()->GetOutcome(n));
    }
    break;
            
  default:
    label = "";
    break;
  }
        
  if (label != "") { 
    dc.SetFont(m_parent->DrawSettings().NodeRightFont());
    gDrawText(dc, label,
	      child_entry->x + m_parent->DrawSettings().NodeLength() +
	      child_entry->nums * INFOSET_SPACING + 10,
	      child_entry->y - 12);
  }
}


//
// RenderSubtree: Render branches and labels
//
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.
//
// There was some clipping code in here, but it didn't correctly
// deal with drawing information sets while scrolling.  So, the code
// has temporarily been removed.  It remains to be seen whether
// performance will require a more sophisticated solution to the
// problem.  (TLT 5/2001)
//
// The offset is used to simulate scrollbars in the
// zoom window.  It might be used for the main window if scrollbars prove to
// be a limitation.
//
void efgTreeLayout::RenderSubtree(wxDC &dc) const
{
  // x-start, x-end, y-start, y-end: coordinates for drawing branches 
  int xs, xe, ys, ye;
  NodeEntry entry, child_entry;
  // Determine the visible region on screen to implement clipping
  int x_start, y_start, width, height;

  m_parent->ViewStart(&x_start, &y_start);
  m_parent->GetClientSize(&width, &height);
  
  // go through the list of nodes, plotting them
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    child_entry = *m_nodeList[pos];    // must make a copy to use Translate
    entry = *child_entry.parent;
        
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
	(m_parent->HighlightInfoset()  && (entry.n->GetInfoset() == m_parent->HighlightInfoset())) ||
	(m_parent->HighlightInfoset1() && (entry.n->GetInfoset() == m_parent->HighlightInfoset1()));
      ::DrawLine(dc, entry.x, entry.y,
		 entry.x + m_parent->DrawSettings().NodeLength() + 
		 entry.nums * INFOSET_SPACING, entry.y, 
		 entry.color, hilight ? 1 : 0);
      
      // show the infoset lines, if required by draw settings
      ::DrawCircle(dc, entry.x + entry.num * INFOSET_SPACING, entry.y, 
		   3, entry.color);
    }
      
    if (child_entry.n == m_parent->SubgameNode())
      DrawSubgamePickIcon(dc, child_entry);
    
    // draw the 'branches'
    if (child_entry.n->GetParent() && child_entry.in_sup) {
      // no branches for root node
      xs = entry.x+m_parent->DrawSettings().NodeLength()+entry.nums*INFOSET_SPACING;
      ys = entry.y;
      xe = xs+m_parent->DrawSettings().ForkLength();
      ye = child_entry.y;
      ::DrawLine(dc, xs, ys, xe, ye, entry.color);

      // Draw the highlight... y = a + bx = ys + (ye-ys) / (xe-xs) * x
      double prob = m_parent->Parent()->ActionProb(entry.n,  child_entry.child_number);
      if (prob > 0) {
	::DrawLine(dc, xs, ys, (xs + m_parent->DrawSettings().ForkLength() * prob), 
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
		 xe + m_parent->DrawSettings().NodeLength() + 
		 child_entry.nums * INFOSET_SPACING, 
		 ye, wxGetApp().GetPreferences().GetTerminalColor());
      
      // Collapsed subgame: subgame icon is drawn at this terminal node.
      if ((child_entry.n->GetSubgameRoot() == child_entry.n) && 
	  !child_entry.expanded)
	DrawLargeSubgameIcon(dc, child_entry, m_parent->DrawSettings().NodeLength());
      
      // Marked Node: a circle is drawn at this terminal node
      if (child_entry.n == m_parent->MarkNode()) {
	::DrawCircle(dc, 
		     xe + child_entry.nums * INFOSET_SPACING +
		     m_parent->DrawSettings().NodeLength(), ye, 
		     4, m_parent->DrawSettings().CursorColor());
      }
    }

    // Draw a circle to show the marked node
    if ((entry.n == m_parent->MarkNode()) && 
	(child_entry.child_number == entry.n->Game()->NumChildren(entry.n))) {
      ::DrawCircle(dc, entry.x + entry.nums * INFOSET_SPACING + 
		   m_parent->DrawSettings().NodeLength(), entry.y, 
		   4, m_parent->DrawSettings().CursorColor());
    }

    if (child_entry.child_number == 1) {
      if (m_parent->DrawSettings().ShowInfosets()) {
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

void efgTreeLayout::Render(wxDC &p_dc) const
{ 
  RenderSubtree(p_dc);
}

#ifdef ZOOM_WINDOW

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

efgTreeLayout &TreeZoomWindow::NodeList(void)
{ return parent->NodeList(); }

const TreeDrawSettings &TreeZoomWindow::DrawSettings(void) const
{ return parent->DrawSettings(); }

Node *TreeZoomWindow::Cursor(void) const
{ return parent->Cursor(); }

#endif  // ZOOM_WINDOW
