//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of tree layout representation
//

#include <math.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"
#include "math/math.h"

#include "efg.h"

#include "treewin.h"
#include "efgshow.h"
#include "legend.h"

int INFOSET_SPACING = 40;
int SUBGAME_LARGE_ICON_SIZE = 20;
int SUBGAME_SMALL_ICON_SIZE = 10;
int DELTA = 8;
int MAX_TW = 60;
int MAX_TH = 20;

//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     const wxColour &color, int thick = 0)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, (thick) ? 4 : 2, wxSOLID));
  dc.DrawLine((int) x_s, (int) y_s, (int) x_e, (int) y_e);
}

inline void DrawRectangle(wxDC &dc, int x_s, int y_s, int w, int h,
                          const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 2, wxSOLID));
  dc.DrawRectangle(x_s, y_s, w, h);
}

inline void DrawThinLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                         const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxSOLID));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawDashedLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
			   const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxSHORT_DASH));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawCircle(wxDC &dc, int x, int y, int r, const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 3, wxSOLID));
  dc.DrawEllipse(x-r, y-r, 2*r, 2*r);
}


//-----------------------------------------------------------------------
//                   class NodeEntry: Member functions
//-----------------------------------------------------------------------

NodeEntry::NodeEntry(Node *p_node)
  : m_node(p_node), m_selected(false), m_cursor(false),
    m_subgameRoot(false), m_subgameMarked(false), m_size(20),
    m_token(NODE_TOKEN_CIRCLE),
    m_branchStyle(BRANCH_STYLE_LINE), m_branchLabel(BRANCH_LABEL_HORIZONTAL),
    m_branchLength(0),
    m_sublevel(0), m_actionProb(0), nums(0), in_sup(true)
{ }

void NodeEntry::SetCursor(bool p_cursor)
{
  m_cursor = p_cursor;
  if (m_cursor) {
    m_selected = true;
  }
}

//
// Draws the node token itself, as well as the incoming branch
// (if not the root node)
//
void NodeEntry::Draw(wxDC &p_dc) const
{
  if (m_node->GetParent() && in_sup) {
    DrawIncomingBranch(p_dc);
  }

  p_dc.SetPen(*wxThePenList->FindOrCreatePen(color, (IsSelected()) ? 4 : 2,
					     wxSOLID));
  if (m_token == NODE_TOKEN_LINE) {
    p_dc.DrawLine(x + m_sublevel * INFOSET_SPACING, y,
		  x + m_sublevel * INFOSET_SPACING + m_size, y);
  }
  else if (m_token == NODE_TOKEN_BOX) {
    p_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    p_dc.DrawRectangle(x + m_sublevel * INFOSET_SPACING, y - m_size / 2,
		       m_size, m_size);
  }
  else if (m_token == NODE_TOKEN_DIAMOND) {
    wxPoint points[4] = { wxPoint(x + m_size / 2, y - m_size / 2),
			  wxPoint(x, y),
			  wxPoint(x + m_size / 2, y + m_size / 2),
			  wxPoint(x + m_size, y) };
    p_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    p_dc.DrawPolygon(4, points, m_sublevel * INFOSET_SPACING);
  }
  else {
    // Default: draw circles
    p_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    p_dc.DrawEllipse(x + m_sublevel * INFOSET_SPACING, y - m_size / 2,
		     m_size, m_size); 
  }

  int textWidth, textHeight;
  p_dc.GetTextExtent(m_nodeAboveLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeAboveLabel,
		GetX() + (GetSize() - textWidth) / 2, y - textHeight - 9);
  p_dc.GetTextExtent(m_nodeBelowLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeBelowLabel,
		GetX() + (GetSize() - textWidth) / 2, y + 9);
  p_dc.GetTextExtent(m_nodeRightLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeRightLabel,
		GetX() + GetSize() + 10, y - textHeight/2);

  if (m_subgameRoot) {
    if (m_subgameMarked) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
    }
    else {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxLIGHT_GREY, 2, wxSOLID));
    }
    p_dc.DrawLine(GetX() - GetSize() / 2, y,
		  GetX() + 2 * GetSize(), y + 2 * GetSize());
    p_dc.DrawLine(GetX() - GetSize() / 2, y,
		  GetX() + 2 * GetSize(), y - 2 * GetSize());
  }

}

void NodeEntry::DrawIncomingBranch(wxDC &p_dc) const
{
  int xStart = parent->GetX() + parent->GetSize();
  int xEnd = GetX();
  int yStart = parent->y;
  int yEnd = y;

  p_dc.SetPen(*wxThePenList->FindOrCreatePen(parent->color, 2, wxSOLID)); 
  if (m_branchStyle == BRANCH_STYLE_LINE) {
    p_dc.DrawLine(xStart, yStart, xEnd, yEnd);

    // Draw in the highlight indicating action probabilities
    if (m_actionProb >= gNumber(0)) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart +
		    (int) ((double) (xEnd - xStart) * (double) m_actionProb),
		    yStart +
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_branchAboveFont);
    p_dc.GetTextExtent(m_branchAboveLabel, &textWidth, &textHeight);

    // The angle of the branch
    double theta = -atan((double) (yEnd - yStart) / (double) (xEnd - xStart));
    // The "centerpoint" of the branch
    int xbar = (xStart + xEnd) / 2;
    int ybar = (yStart + yEnd) / 2;

    if (m_branchLabel == BRANCH_LABEL_HORIZONTAL) {
      if (yStart >= yEnd) {
	p_dc.DrawText(m_branchAboveLabel, xbar - textWidth / 2, 
		      ybar - textHeight + 
		      textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
      }
      else {
	p_dc.DrawText(m_branchAboveLabel, xbar - textWidth / 2, 
		      ybar - textHeight - 
		      textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(m_branchAboveLabel,
			   (int) ((double) xbar -
				  (double) textHeight * sin(theta) -
				  (double) textWidth * cos(theta) / 2.0),
			   (int) ((double) ybar - 
				  (double) textHeight * cos(theta) +
				  (double) textWidth * sin(theta) / 2.0),
			   theta * 180.0 / 3.14159);
    }

    p_dc.SetFont(m_branchBelowFont);
    p_dc.GetTextExtent(m_branchBelowLabel, &textWidth, &textHeight);

    if (m_branchLabel == BRANCH_LABEL_HORIZONTAL) {
      if (yStart >= yEnd) {
	p_dc.DrawText(m_branchBelowLabel, xbar - textWidth / 2,
		      ybar - textWidth/2 * (yEnd - yStart) / (xEnd - xStart));
      }
      else {
	p_dc.DrawText(m_branchBelowLabel, xbar - textWidth / 2,
		      ybar + textWidth/2 * (yEnd - yStart) / (xEnd - xStart));
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(m_branchBelowLabel,
			   (int) ((double) xbar -
				  (double) textWidth * cos(theta) / 2.0),
			   (int) ((double) ybar +
				  (double) textWidth * sin(theta) / 2.0),
			   theta * 180.0 / 3.14159);
    }
  }
  else {
    // Old style fork-tine 
    p_dc.DrawLine(xStart, yStart, xStart + m_branchLength, yEnd);
    p_dc.DrawLine(xStart + m_branchLength, yEnd, xEnd, yEnd);
    
    // Draw in the highlight indicating action probabilities
    if (m_actionProb >= gNumber(0)) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart + 
		    (int) ((double) m_branchLength * (double) m_actionProb),
		    yStart + 
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_branchAboveFont);
    p_dc.GetTextExtent(m_branchAboveLabel, &textWidth, &textHeight);
    p_dc.DrawText(m_branchAboveLabel,
		  xStart + m_branchLength + 3, yEnd - textHeight - 3);
    
    p_dc.SetFont(m_branchBelowFont);
    p_dc.GetTextExtent(m_branchBelowLabel, &textWidth, &textHeight);
    p_dc.DrawText(m_branchBelowLabel,
		  xStart + m_branchLength + 3, yEnd + 3);
  }
}

int NodeEntry::GetX(void) const
{
  return (x + m_sublevel * INFOSET_SPACING);
}

bool NodeEntry::NodeHitTest(int p_x, int p_y) const
{
  if (p_x < GetX() || p_x >= GetX() + m_size) {
    return false;
  }

  if (m_token == NODE_TOKEN_LINE) {
    const int DELTA = 8;  // a fudge factor for "almost" hitting the node
    return (p_y >= y - DELTA && p_y <= y + DELTA);
  }
  else {
    return (p_y >= y - m_size / 2 && p_y <= y + m_size / 2);
  }
}

//-----------------------------------------------------------------------
//                class efgTreeLayout: Member functions
//-----------------------------------------------------------------------

efgTreeLayout::efgTreeLayout(FullEfg &p_efg, TreeWindow *p_parent)
  : m_efg(p_efg), m_parent(p_parent),
    c_leftMargin(20), c_topMargin(40)
{ }

Node *efgTreeLayout::NodeHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->NodeHitTest(p_x, p_y)) {
      return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::SubgameHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->GetNode()->GetSubgameRoot() == entry->GetNode())  {
      if (p_x > entry->x &&
	  p_x < entry->x + SUBGAME_SMALL_ICON_SIZE &&
	  p_y > entry->y - SUBGAME_SMALL_ICON_SIZE/2 &&
	  p_y < entry->y + SUBGAME_SMALL_ICON_SIZE/2) {
	return entry->GetNode();
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
      return entry->GetNode();
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
      return entry->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::NodeRightHitTest(int p_x, int p_y) const
{
  // This is a fudge factor for hitting the outcome; this should be
  // improved in an improved hit-test implementation
  const int OUTCOME_LENGTH = 60;

  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (p_x > entry->x + m_parent->DrawSettings().NodeSize() + 10 &&
	p_x < (entry->x + m_parent->DrawSettings().NodeSize() + 10 +
	       OUTCOME_LENGTH * 3) &&
	p_y > entry->y - DELTA &&
	p_y < entry->y + DELTA) {
      return entry->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::BranchHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    NodeEntry *parent_entry = GetNodeEntry(entry->GetNode()->GetParent());

    if (parent_entry) {
      if (p_x > (parent_entry->x + m_parent->DrawSettings().NodeSize() + 
		 parent_entry->GetSublevel() * INFOSET_SPACING + 10) &&
	  p_x < (parent_entry->x + m_parent->DrawSettings().NodeSize() +
		 m_parent->DrawSettings().BranchLength() +
		 parent_entry->GetSublevel() * INFOSET_SPACING)) {
	// Good old slope/intercept method for finding a point on a line
	int y0 = (parent_entry->y + 
		  (int) (p_x - parent_entry->x - 
			 m_parent->DrawSettings().NodeSize() -
			 parent_entry->nums * INFOSET_SPACING) *
		  (entry->y - parent_entry->y) / 
		  m_parent->DrawSettings().BranchLength());

	if (p_y > y0-2 && p_y < y0+2) {
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}



Node *efgTreeLayout::BranchAboveHitTest(int p_x, int p_y) const
{
#ifdef UNUSED
  // Needs to be updated for new tree style
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->parent &&
	p_x > entry->x - m_parent->DrawSettings().BranchLength() &&
	p_x < entry->x - m_parent->DrawSettings().BranchLength() + MAX_TW &&
	p_y > entry->y - MAX_TH &&
	p_y < entry->y + DELTA) {
      return entry->GetNode();
    }
  }
#endif
  return 0;
}

Node *efgTreeLayout::BranchBelowHitTest(int p_x, int p_y) const
{
#ifdef UNUSED
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];

    if (entry->parent &&
	p_x > entry->x - m_parent->DrawSettings().BranchLength() &&
	p_x < entry->x - m_parent->DrawSettings().BranchLength() + MAX_TW &&
	p_y > entry->y + DELTA && 
	p_y < entry->y + MAX_TH) {
      return entry->GetNode();
    }
  }
#endif
  return 0;
}

Node *efgTreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    if (entry->infoset.y != -1 && entry->GetNode()->GetInfoset()) {
      if (p_x > entry->x + entry->GetSublevel() * INFOSET_SPACING - 2 &&
	  p_x < entry->x + entry->GetSublevel() * INFOSET_SPACING + 2) {
	if (p_y > entry->y && p_y < entry->infoset.y) {
	  // next iset is below this one
	  return entry->GetNode();
	}
	else if (p_y > entry->infoset.y && p_y < entry->y) {
	  // next iset is above this one
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}

wxString efgTreeLayout::CreateNodeAboveLabel(const NodeEntry *p_entry) const
{
  const Node *n = p_entry->GetNode();
    
  switch (m_parent->DrawSettings().NodeAboveLabel()) {
  case NODE_ABOVE_NOTHING:
    return "";
  case NODE_ABOVE_LABEL:
    return (const char *) n->GetName();
  case NODE_ABOVE_PLAYER:
    return ((const char *) 
	    ((n->GetPlayer()) ? n->GetPlayer()->GetName() : gText("")));
  case NODE_ABOVE_ISETLABEL:
    return ((const char *)
	    ((n->GetInfoset()) ? n->GetInfoset()->GetName() : gText("")));
  case NODE_ABOVE_ISETID:
    return ((const char *)
	    ((n->GetInfoset()) ?
	     ("(" + ToText(n->GetPlayer()->GetNumber()) +
	      "," + ToText(n->GetInfoset()->GetNumber()) + ")") : gText("")));
  case NODE_ABOVE_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(n);
  case NODE_ABOVE_REALIZPROB:
    return (const char *) m_parent->Parent()->GetRealizProb(n);
  case NODE_ABOVE_BELIEFPROB:
    return (const char *) m_parent->Parent()->GetBeliefProb(n);
  case NODE_ABOVE_VALUE:
    return (const char *) m_parent->Parent()->GetNodeValue(n);
  default:
    return "";
  }
}    

wxString efgTreeLayout::CreateNodeBelowLabel(const NodeEntry *p_entry) const
{
  const Node *n = p_entry->GetNode();

  switch (m_parent->DrawSettings().NodeBelowLabel()) { 
  case NODE_BELOW_NOTHING:
    return "";
  case NODE_BELOW_LABEL:
    return (const char *) n->GetName();
  case NODE_BELOW_PLAYER:
    return ((const char *)
	    ((n->GetPlayer()) ? n->GetPlayer()->GetName() : gText("")));
  case NODE_BELOW_ISETLABEL:
    return ((const char *)
	    ((n->GetInfoset()) ? n->GetInfoset()->GetName() : gText("")));
  case NODE_BELOW_ISETID:
    return ((const char *)
	    ((n->GetInfoset()) ?
	     ("(" + ToText(n->GetPlayer()->GetNumber()) +
	      "," + ToText(n->GetInfoset()->GetNumber()) + ")") : gText("")));
  case NODE_BELOW_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(n);
  case NODE_BELOW_REALIZPROB:
    return (const char *) m_parent->Parent()->GetRealizProb(n);
  case NODE_BELOW_BELIEFPROB:
    return (const char *) m_parent->Parent()->GetBeliefProb(n);
  case NODE_BELOW_VALUE:
    return (const char *) m_parent->Parent()->GetNodeValue(n);
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateNodeRightLabel(const NodeEntry *p_entry) const
{    
  const Node *node = p_entry->GetNode();

  switch (m_parent->DrawSettings().NodeRightLabel()) { 
  case NODE_RIGHT_NOTHING:
    return "";
  case NODE_RIGHT_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(node);
  case NODE_RIGHT_NAME:
    if (!node->Game()->GetOutcome(node).IsNull()) {
      return (const char *) m_efg.GetOutcomeName(node->Game()->GetOutcome(node));
    }
    else {
      return "";
    }
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateBranchAboveLabel(const NodeEntry *p_entry) const
{
  const Node *parent = p_entry->parent->GetNode();

  switch (m_parent->DrawSettings().BranchAboveLabel()) {
  case BRANCH_ABOVE_NOTHING:
    return "";
  case BRANCH_ABOVE_LABEL:
    return (const char *) parent->GetInfoset()->GetActionName(p_entry->child_number);
  case BRANCH_ABOVE_PROBS:
    return (const char *) m_parent->Parent()->GetActionProb(parent,
							    p_entry->child_number);
  case BRANCH_ABOVE_VALUE:
    return (const char *) m_parent->Parent()->GetActionValue(parent,
							     p_entry->child_number);
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateBranchBelowLabel(const NodeEntry *p_entry) const
{
  const Node *parent = p_entry->parent->GetNode();

  switch (m_parent->DrawSettings().BranchBelowLabel()) {
  case BRANCH_BELOW_NOTHING:
    return "";
  case BRANCH_BELOW_LABEL:
    return (const char *) parent->GetInfoset()->GetActionName(p_entry->child_number);
  case BRANCH_BELOW_PROBS:
    return (const char *) m_parent->Parent()->GetActionProb(parent,
							    p_entry->child_number);
  case BRANCH_BELOW_VALUE:
    return (const char *) m_parent->Parent()->GetActionValue(parent,
							     p_entry->child_number);
  default:
    return "";
  }
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
    if (m_nodeList[i]->GetNode() == p_node) {
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
      if (m_nodeList[i]->GetLevel() == entry->GetLevel())
	return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::NextSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) + 1; i <= m_nodeList.Length(); i++) {
      if (m_nodeList[i]->GetLevel() == entry->GetLevel()) { 
	return m_nodeList[i]->GetNode();
      }
    }
  }
  return 0;
}

int efgTreeLayout::FillTable(Node *n, const EFSupport &cur_sup, int level,
			   int &maxlev, int &maxy, int &miny, int &ycoord)
{
  int y1 = -1, yn=0;
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
    
  NodeEntry *entry = new NodeEntry(n);
  if (n == m_efg.RootNode()) {
    entry->child_number = 0;
  }
  else {
    Node *parent = n->GetParent();
    for (int i = 1; i <= parent->Game()->NumChildren(parent); i++) {
      if (parent->GetChild(i) == n) {
	entry->child_number = i;
	break;
      }
    }
  }
  m_nodeList += entry;
  if (n->Game()->NumChildren(n) > 0) {
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
    ycoord += draw_settings.TerminalSpacing();
  }
    
  entry->SetLevel(level);
  entry->has_children = n->Game()->NumChildren(n);
    
  entry->infoset.y = -1;
  entry->infoset.x = -1;
  if (draw_settings.BranchStyle() == BRANCH_STYLE_LINE) {
    entry->x = c_leftMargin + level * (draw_settings.NodeSize() +
				       draw_settings.BranchLength());
  }
  else {
    entry->x = c_leftMargin + level * (draw_settings.NodeSize() +
				       draw_settings.BranchLength() +
				       draw_settings.TineLength());
  }
  if (n->GetPlayer() && n->GetPlayer()->IsChance()) {
    entry->color = wxGetApp().GetPreferences().GetChanceColor();
    entry->SetToken(draw_settings.ChanceToken());
  }
  else if (n->GetPlayer()) {
    entry->color = wxGetApp().GetPreferences().GetPlayerColor(n->GetPlayer()->GetNumber());
    entry->SetToken(draw_settings.PlayerToken());
  }
  else {
    entry->color = wxGetApp().GetPreferences().GetTerminalColor();
    entry->SetToken(draw_settings.TerminalToken());
  }  
  
  entry->SetSize(draw_settings.NodeSize());
  entry->SetBranchStyle(draw_settings.BranchStyle());
  if (draw_settings.BranchStyle() == BRANCH_STYLE_LINE) {
    entry->SetBranchLabelStyle(draw_settings.BranchLabels());
  }
  entry->SetBranchLength(draw_settings.BranchLength());

  if (draw_settings.SubgameStyle() == SUBGAME_ARC &&
      n->Game()->IsLegalSubgame(n)) {
    entry->SetSubgameRoot(true);
    entry->SetSubgameMarked(n->GetSubgameRoot() == n);
  }
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
    if (e->GetNode()->GetInfoset() == e1->GetNode()->GetInfoset()) {
      if (draw_settings.InfosetConnect() == SHOWISET_ALL) {
	return e1;
      }
      else if (e->GetLevel() == e1->GetLevel()) {
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
    if (e->GetNode()->GetInfoset() == e1->GetNode()->GetInfoset() && 
	e->GetLevel() == e1->GetLevel() && e1->GetSublevel() > 0) {
      e->SetSublevel(e1->GetSublevel());
      if (infoset_entry) {
	e->infoset.y = infoset_entry->y;
	if (draw_settings.InfosetConnect() == SHOWISET_ALL)
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
    if (e->GetLevel() == e1->GetLevel())  {
      num = gmax(e1->GetSublevel(), num);
    }
  }
  num++;
  e->SetSublevel(num);
  e->infoset.y = infoset_entry->y;
  if (draw_settings.InfosetConnect() == SHOWISET_ALL) {
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
    nums[e->GetLevel()] = gmax(e->GetSublevel()+1, nums[e->GetLevel()]);
  }
    
  // record the max e->num for each level for each node
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    e = m_nodeList[pos];
    e->nums = nums[e->GetLevel()];
  }
    
  for (int i = 0; i <= m_maxlev; i++)  nums[i+1] += nums[i];
    
  // now add the needed length to each level, and set maxX
  m_maxX = 0;
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    e = m_nodeList[pos];
    if (e->GetLevel() != 0) {
      e->x += nums[e->GetLevel()-1]*INFOSET_SPACING;
    }
    m_maxX = gmax(m_maxX, e->x);
  }
}

void efgTreeLayout::UpdateTableParents(void)
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e = m_nodeList[pos];
    e->parent = (e->GetNode() == m_efg.RootNode()) ? e : GetValidParent(e->GetNode());
    if (!GetValidChild(e->GetNode())) e->has_children = 0;
  }
}

void efgTreeLayout::Layout(const EFSupport &p_support)
{
  while (m_nodeList.Length() > 0) {
    delete m_nodeList.Remove(1);
  }

  int maxlev = 0, miny = 0, maxy = 0, ycoord = c_topMargin;
  FillTable(m_efg.RootNode(), p_support, 0, maxlev, maxy, miny, ycoord);
  m_maxlev = maxlev;

  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  if (draw_settings.InfosetConnect()) {
    // FIXME! This causes lines to disappear... sometimes.
    FillInfosetTable(m_efg.RootNode(), p_support);
    UpdateTableInfosets();
  }

  UpdateTableParents();
  GenerateLabels();

  const int OUTCOME_LENGTH = 60;

  m_maxX += draw_settings.NodeSize() + OUTCOME_LENGTH;
  m_maxY = maxy + 25;
}

void efgTreeLayout::GenerateLabels(void)
{
  const TreeDrawSettings &settings = m_parent->DrawSettings();
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    entry->SetNodeAboveLabel(CreateNodeAboveLabel(entry));
    entry->SetNodeBelowLabel(CreateNodeBelowLabel(entry));
    entry->SetNodeRightLabel(CreateNodeRightLabel(entry));
    if (entry->child_number > 0) {
      entry->SetBranchAboveLabel(CreateBranchAboveLabel(entry));
      entry->SetBranchAboveFont(settings.BranchAboveFont());
      entry->SetBranchBelowLabel(CreateBranchBelowLabel(entry));
      entry->SetBranchBelowFont(settings.BranchBelowFont());
      entry->SetActionProb(m_parent->Parent()->ActionProb(entry->GetNode()->GetParent(),
							  entry->child_number));
    }
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
void efgTreeLayout::RenderSubtree(wxDC &p_dc) const
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];  
    NodeEntry *parentEntry = entry->parent;
        
    if (entry->child_number == 1) {
      parentEntry->Draw(p_dc);

      if (m_parent->DrawSettings().InfosetConnect()) {
	if (parentEntry->infoset.y != -1) {
#ifdef __WXGTK__
	  // A problem with using styled pens and user scaling on wxGTK
	  p_dc.SetPen(wxPen(parentEntry->color, 1, wxSOLID));
#else
	  p_dc.SetPen(wxPen(parentEntry->color, 1, wxDOT));
#endif   // __WXGTK__
	  p_dc.DrawLine(parentEntry->GetX(), parentEntry->y, 
			parentEntry->GetX(), parentEntry->infoset.y); 
	  p_dc.DrawLine(parentEntry->GetX() + parentEntry->GetSize(), 
			parentEntry->y,
			parentEntry->GetX() + parentEntry->GetSize(), 
			parentEntry->infoset.y);
	}

#ifdef UNUSED	  
	if (entry.infoset.x != -1) {
	  // Draw a little arrow in the direction of the iset.
	  if (entry.infoset.x > entry.x) { // iset is to the right
	    ::DrawLine(dc, 
		       entry.x + entry.GetSublevel() * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x+(entry.GetSublevel()+1)*INFOSET_SPACING, 
		       entry.infoset.y, entry.color);
	  }
	  else {  // iset is to the left
	    ::DrawLine(dc, 
		       entry.x + entry.GetSublevel() * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x + (entry.GetSublevel() - 1) * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.color);
	  }
	}
#endif  // UNUSED
      }
    }

    if (!entry->has_children) { 
      entry->Draw(p_dc);
    }

  }
}

void efgTreeLayout::Render(wxDC &p_dc) const
{ 
  RenderSubtree(p_dc);
}

