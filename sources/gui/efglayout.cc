//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of tree layout representation
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

#include <math.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "math/gmath.h"

#include "game/efg.h"

#include "treewin.h"
#include "efgshow.h"

//-----------------------------------------------------------------------
//                       Local utility functions
//-----------------------------------------------------------------------

//
// OutcomeAsString: Returns the outcome payoffs at node 'n' as a text string
//
static wxString OutcomeAsString(const gbtEfgNode &p_node, int p_numDecimals)
{
  gbtEfgOutcome outcome = p_node.GetOutcome();
  if (!outcome.IsNull()) {
    const gArray<gNumber> &payoffs = outcome.GetPayoff();
    wxString tmp = "(";

    for (int pl = payoffs.First(); pl <= payoffs.Last(); pl++) {
      if (pl != 1) { 
	tmp += ",";
      }
      tmp += (const char *) ToText(payoffs[pl], p_numDecimals);
    }
    tmp += ")";
        
    return tmp;
  }
  else {
    return "";
  }
}

//-----------------------------------------------------------------------
//                   class NodeEntry: Member functions
//-----------------------------------------------------------------------

NodeEntry::NodeEntry(gbtEfgNode p_node)
  : m_node(p_node), m_parent(0),
    m_x(-1), m_y(-1), m_nextMember(0), m_inSupport(true),
    m_selected(false), m_cursor(false), m_cut(false),
    m_subgameRoot(false), m_subgameMarked(false), m_size(20),
    m_token(GBT_NODE_TOKEN_CIRCLE),
    m_branchStyle(GBT_BRANCH_STYLE_LINE), 
    m_branchLabel(GBT_BRANCH_LABEL_HORIZONTAL),
    m_branchLength(0),
    m_sublevel(0), m_actionProb(0)
{ }

int NodeEntry::GetChildNumber(void) const
{
  if (!m_node.GetParent().IsNull()) {
    return m_node.GetAction().GetId();
  }
  else {
    return 0;
  }
}

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
  if (!m_node.GetParent().IsNull() && m_inSupport) {
    DrawIncomingBranch(p_dc);
  }

  if (m_cut) {
    p_dc.SetPen(*wxLIGHT_GREY_PEN);
  }
  else {
    p_dc.SetPen(*wxThePenList->FindOrCreatePen(m_color, (IsSelected()) ? 4 : 2,
					       wxSOLID));
  }
  if (m_token == GBT_NODE_TOKEN_LINE) {
    p_dc.DrawLine(m_x, m_y, m_x + m_size, m_y);
    if (m_branchStyle == GBT_BRANCH_STYLE_FORKTINE) {
      // "classic" Gambit style: draw a small 'token' to separate
      // the fork from the node
      p_dc.DrawEllipse(m_x - 1, m_y - 1, 3, 3);
    }
  }
  else if (m_token == GBT_NODE_TOKEN_BOX) {
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawRectangle(m_x, m_y - m_size / 2, m_size, m_size);
  }
  else if (m_token == GBT_NODE_TOKEN_DIAMOND) {
    wxPoint points[4] = { wxPoint(m_x + m_size / 2, m_y - m_size / 2),
			  wxPoint(m_x, m_y),
			  wxPoint(m_x + m_size / 2, m_y + m_size / 2),
			  wxPoint(m_x + m_size, m_y) };
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawPolygon(4, points);
  }
  else {
    // Default: draw circles
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawEllipse(m_x, m_y - m_size / 2, m_size, m_size); 
  }

  int textWidth, textHeight;
  p_dc.SetFont(m_nodeAboveFont);
  p_dc.GetTextExtent(m_nodeAboveLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeAboveLabel,
		m_x + (m_size - textWidth) / 2, m_y - textHeight - 9);
  p_dc.SetFont(m_nodeBelowFont);
  p_dc.GetTextExtent(m_nodeBelowLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeBelowLabel,
		m_x + (m_size - textWidth) / 2, m_y + 9);
  p_dc.SetFont(m_nodeRightFont);
  p_dc.GetTextExtent(m_nodeRightLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeRightLabel,
		m_x + GetSize() + 10, m_y - textHeight/2);

  if (m_subgameRoot) {
    if (m_subgameMarked) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
    }
    else {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxLIGHT_GREY, 2, wxSOLID));
    }
    p_dc.DrawLine(m_x - m_size / 2, m_y,
		  m_x + 2 * m_size, m_y + 2 * m_size);
    p_dc.DrawLine(m_x - m_size / 2, m_y,
		  m_x + 2 * m_size, m_y - 2 * m_size);
  }
}

void NodeEntry::DrawIncomingBranch(wxDC &p_dc) const
{
  int xStart = m_parent->m_x + m_parent->m_size;
  int xEnd = m_x;
  int yStart = m_parent->m_y;
  int yEnd = m_y;

  if (m_parent->m_cut) {
    p_dc.SetPen(*wxLIGHT_GREY_PEN);
  }
  else {
    p_dc.SetPen(*wxThePenList->FindOrCreatePen(m_parent->m_color,
					       2, wxSOLID)); 
  }
  if (m_branchStyle == GBT_BRANCH_STYLE_LINE) {
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

    if (m_branchLabel == GBT_BRANCH_LABEL_HORIZONTAL) {
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

    if (m_branchLabel == GBT_BRANCH_LABEL_HORIZONTAL) {
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

bool NodeEntry::NodeHitTest(int p_x, int p_y) const
{
  if (p_x < m_x || p_x >= m_x + m_size) {
    return false;
  }

  if (m_token == GBT_NODE_TOKEN_LINE) {
    const int DELTA = 8;  // a fudge factor for "almost" hitting the node
    return (p_y >= m_y - DELTA && p_y <= m_y + DELTA);
  }
  else {
    return (p_y >= m_y - m_size / 2 && p_y <= m_y + m_size / 2);
  }
}

//-----------------------------------------------------------------------
//                class efgTreeLayout: Member functions
//-----------------------------------------------------------------------

efgTreeLayout::efgTreeLayout(gbtGameDocument *p_doc)
  : m_doc(p_doc), 
    m_infosetSpacing(40), c_leftMargin(20), c_topMargin(40)
{ }

gbtEfgNode efgTreeLayout::NodeHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->NodeHitTest(p_x, p_y)) {
      return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

gbtEfgNode efgTreeLayout::BranchHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    NodeEntry *parent_entry = GetNodeEntry(entry->GetNode().GetParent());

    if (parent_entry) {
      if (p_x > (parent_entry->X() + m_doc->GetPreferences().NodeSize() + 
		 parent_entry->GetSublevel() * m_infosetSpacing + 10) &&
	  p_x < (parent_entry->X() + m_doc->GetPreferences().NodeSize() +
		 m_doc->GetPreferences().BranchLength() +
		 parent_entry->GetSublevel() * m_infosetSpacing)) {
	// Good old slope/intercept method for finding a point on a line
	int y0 = (parent_entry->Y() + 
		  (int) (p_x - parent_entry->X() - 
			 m_doc->GetPreferences().NodeSize()) *
		  (entry->Y() - parent_entry->Y()) / 
		  m_doc->GetPreferences().BranchLength());

	if (p_y > y0-2 && p_y < y0+2) {
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}


gbtEfgNode efgTreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    if (entry->GetNextMember() && !entry->GetNode().GetInfoset().IsNull()) {
      if (p_x > entry->X() + entry->GetSublevel() * m_infosetSpacing - 2 &&
	  p_x < entry->X() + entry->GetSublevel() * m_infosetSpacing + 2) {
	if (p_y > entry->Y() && p_y < entry->GetNextMember()->Y()) {
	  // next iset is below this one
	  return entry->GetNode();
	}
	else if (p_y > entry->GetNextMember()->Y() && p_y < entry->Y()) {
	  // next iset is above this one
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}

wxString efgTreeLayout::CreateNodeLabel(const NodeEntry *p_entry,
					int p_which) const
{
  gbtEfgNode n = p_entry->GetNode();
    
  switch (p_which) {
  case GBT_NODE_LABEL_NOTHING:
    return "";
  case GBT_NODE_LABEL_LABEL:
    return (const char *) n.GetLabel();
  case GBT_NODE_LABEL_PLAYER:
    return ((const char *) 
	    (!(n.GetPlayer().IsNull()) ? 
	     n.GetPlayer().GetLabel() : gText("")));
  case GBT_NODE_LABEL_ISETLABEL:
    return ((const char *)
	    (!(n.GetInfoset().IsNull()) ? n.GetInfoset().GetLabel() : gText("")));
  case GBT_NODE_LABEL_ISETID:
    return ((const char *)
	    ((!n.GetInfoset().IsNull()) ?
	     ("(" + ToText(n.GetPlayer().GetId()) +
	      "," + ToText(n.GetInfoset().GetId()) + ")") : gText("")));
  case GBT_NODE_LABEL_OUTCOME:
    return OutcomeAsString(n, m_doc->GetPreferences().NumDecimals());
  case GBT_NODE_LABEL_REALIZPROB:
    return (const char *) m_doc->GetRealizProb(n);
  case GBT_NODE_LABEL_BELIEFPROB:
    return (const char *) m_doc->GetBeliefProb(n);
  case GBT_NODE_LABEL_VALUE:
    return (const char *) m_doc->GetNodeValue(n);
  default:
    return "";
  }
}    

wxString efgTreeLayout::CreateOutcomeLabel(const NodeEntry *p_entry) const
{    
  gbtEfgNode node = p_entry->GetNode();

  switch (m_doc->GetPreferences().OutcomeLabel()) { 
  case GBT_OUTCOME_LABEL_PAYOFFS:
    return OutcomeAsString(node, m_doc->GetPreferences().NumDecimals());
  case GBT_OUTCOME_LABEL_LABEL:
    return (const char *) node.GetOutcome().GetLabel();
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateBranchLabel(const NodeEntry *p_entry,
					  int p_which) const
{
  gbtEfgNode parent = p_entry->GetParent()->GetNode();

  switch (p_which) {
  case GBT_BRANCH_LABEL_NOTHING:
    return "";
  case GBT_BRANCH_LABEL_LABEL:
    return (const char *) parent.GetInfoset().GetAction(p_entry->GetChildNumber()).GetLabel();
  case GBT_BRANCH_LABEL_PROBS:
    return (const char *) m_doc->GetActionProb(parent,
					       p_entry->GetChildNumber());
  case GBT_BRANCH_LABEL_VALUE:
    return (const char *) m_doc->GetActionValue(parent,
						p_entry->GetChildNumber());
  default:
    return "";
  }
}

NodeEntry *efgTreeLayout::GetValidParent(const gbtEfgNode &e)
{
  NodeEntry *n = GetNodeEntry(e.GetParent());
  if (n) {
    return n;
  }
  else { 
    return GetValidParent(e.GetParent());
  }
}

NodeEntry *efgTreeLayout::GetValidChild(const gbtEfgNode &e)
{
  for (int i = 1; i <= e.NumChildren(); i++)  {
    NodeEntry *n = GetNodeEntry(e.GetChild(i));
    if (n) {
      return n;
    }
    else  {
      n = GetValidChild(e.GetChild(i));
      if (n) return n;
    }
  }
  return 0;
}

NodeEntry *efgTreeLayout::GetEntry(const gbtEfgNode &p_node) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->GetNode() == p_node) {
      return m_nodeList[i];
    }
  }
  return 0;
}

gbtEfgNode efgTreeLayout::PriorSameLevel(const gbtEfgNode &p_node) const
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

gbtEfgNode efgTreeLayout::NextSameLevel(const gbtEfgNode &p_node) const
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

int efgTreeLayout::LayoutSubtree(const gbtEfgNode &p_node, 
				 const EFSupport &p_support,
				 int &p_maxy, int &p_miny, int &p_ycoord)
{
  int y1 = -1, yn = 0;
  const gbtPreferences &prefs = m_doc->GetPreferences();
    
  NodeEntry *entry = m_nodeList[p_node.GetId()];
  entry->SetNextMember(0);
  if (p_node.NumChildren() > 0) {
    for (int i = 1; i <= p_node.NumChildren(); i++) {
      yn = LayoutSubtree(p_node.GetChild(i), p_support,
			 p_maxy, p_miny, p_ycoord);
      if (y1 == -1) {
	y1 = yn;
      }

      if (!p_node.GetPlayer().IsChance() &&
	  !p_support.Contains(p_node.GetInfoset().GetAction(i))) {
	m_nodeList[p_node.GetChild(i).GetId()]->SetInSupport(false);
      }
    }
    entry->SetY((y1 + yn) / 2);
  }
  else {
    entry->SetY(p_ycoord);
    p_ycoord += prefs.TerminalSpacing();
  }
    
  if (prefs.BranchStyle() == GBT_BRANCH_STYLE_LINE) {
    entry->SetX(c_leftMargin + entry->GetLevel() * (prefs.NodeSize() +
						    prefs.BranchLength()));
  }
  else {
    entry->SetX(c_leftMargin + entry->GetLevel() * (prefs.NodeSize() +
						    prefs.BranchLength() +
						    prefs.TineLength()));
  }

  if (!p_node.GetPlayer().IsNull() && p_node.GetPlayer().IsChance()) {
    entry->SetColor(prefs.ChanceColor());
    entry->SetToken(prefs.ChanceToken());
  }
  else if (!p_node.GetPlayer().IsNull()) {
    entry->SetColor(prefs.PlayerColor(p_node.GetPlayer().GetId()));
    entry->SetToken(prefs.PlayerToken());
  }
  else {
    entry->SetColor(prefs.TerminalColor());
    entry->SetToken(prefs.TerminalToken());
  }  
  
  entry->SetSize(prefs.NodeSize());
  entry->SetBranchStyle(prefs.BranchStyle());
  if (prefs.BranchStyle() == GBT_BRANCH_STYLE_LINE) {
    entry->SetBranchLabelStyle(prefs.BranchLabels());
  }
  entry->SetBranchLength(prefs.BranchLength());

  if (prefs.SubgameStyle() == GBT_SUBGAME_ARC &&
      p_node.GetGame().IsLegalSubgame(p_node)) {
    entry->SetSubgameRoot(true);
    entry->SetSubgameMarked(p_node.GetSubgameRoot() == p_node);
  }
  p_maxy = gmax(entry->Y(), p_maxy);
  p_miny = gmin(entry->Y(), p_miny);
    
  return entry->Y();
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
NodeEntry *efgTreeLayout::NextInfoset(NodeEntry *e)
{
  const gbtPreferences &prefs = m_doc->GetPreferences();
  
  for (int pos = m_nodeList.Find(e) + 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e1 = m_nodeList[pos];
    // infosets are the same and the nodes are on the same level
    if (e->GetNode().GetInfoset() == e1->GetNode().GetInfoset()) {
      if (prefs.InfosetConnect() == GBT_INFOSET_CONNECT_ALL) {
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
  // Check if the infoset this entry belongs to (on this level) has already
  // been processed.  If so, make this entry->num the same as the one already
  // processed and return
  infoset_entry = NextInfoset(e);
  for (pos = 1; pos <= m_nodeList.Length(); pos++) {
    e1 = m_nodeList[pos];
    // if the infosets are the same and they are on the same level and e1 has been processed
    if (e->GetNode().GetInfoset() == e1->GetNode().GetInfoset() && 
	e->GetLevel() == e1->GetLevel() && e1->GetSublevel() > 0) {
      e->SetSublevel(e1->GetSublevel());
      if (infoset_entry) {
	e->SetNextMember(infoset_entry);
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
  e->SetNextMember(infoset_entry);
}

void efgTreeLayout::FillInfosetTable(const gbtEfgNode &n,
				     const EFSupport &cur_sup)
{
  const gbtPreferences &prefs = m_doc->GetPreferences();
  NodeEntry *entry = GetNodeEntry(n);
  if (n.NumChildren() > 0) {
    for (int i = 1; i <= n.NumChildren(); i++) {
      bool in_sup = true;
      if (n.GetPlayer().GetId()) {
	in_sup = cur_sup.Contains(n.GetInfoset().GetAction(i));
      }
            
      if (in_sup || !prefs.RootReachable()) {
	FillInfosetTable(n.GetChild(i), cur_sup);
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
  gArray<int> nums(0, m_maxLevel + 1); 
    
  for (int i = 0; i <= m_maxLevel + 1; nums[i++] = 0);
  // find the max e->num for each level
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];
    nums[entry->GetLevel()] = gmax(entry->GetSublevel() + 1,
				   nums[entry->GetLevel()]);
  }
    
  for (int i = 0; i <= m_maxLevel; i++) {
    nums[i+1] += nums[i];
  }
    
  // now add the needed length to each level, and set maxX accordingly
  m_maxX = 0;
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];
    if (entry->GetLevel() != 0) {
      entry->SetX(entry->X() + 
		  (nums[entry->GetLevel()-1] +
		   entry->GetSublevel()) * m_infosetSpacing);
    }
    m_maxX = gmax(m_maxX, entry->X());
  }
}

void efgTreeLayout::UpdateTableParents(void)
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e = m_nodeList[pos];
    e->SetParent((e->GetNode() == m_doc->m_efg->RootNode()) ? 
		 e : GetValidParent(e->GetNode()));
  }
}

void efgTreeLayout::Layout(const EFSupport &p_support)
{
  // Kinda kludgey; probably should query draw prefs whenever needed.
  m_infosetSpacing = 
    (m_doc->GetPreferences().InfosetJoin() == GBT_INFOSET_JOIN_LINES) ? 10 : 40;

  if (m_nodeList.Length() != NumNodes(*m_doc->m_efg)) {
    // A rebuild is in order; force it
    BuildNodeList(p_support);
  }

  int miny = 0, maxy = 0, ycoord = c_topMargin;
  LayoutSubtree(m_doc->m_efg->RootNode(), p_support, maxy, miny, ycoord);

  const gbtPreferences &prefs = m_doc->GetPreferences();
  if (prefs.InfosetConnect() != GBT_INFOSET_CONNECT_NONE) {
    // FIXME! This causes lines to disappear... sometimes.
    FillInfosetTable(m_doc->m_efg->RootNode(), p_support);
    UpdateTableInfosets();
  }

  UpdateTableParents();
  GenerateLabels();

  const int OUTCOME_LENGTH = 60;

  m_maxX += prefs.NodeSize() + OUTCOME_LENGTH;
  m_maxY = maxy + 25;
}

void efgTreeLayout::BuildNodeList(const gbtEfgNode &p_node, 
				  const EFSupport &p_support,
				  int p_level)
{
  NodeEntry *entry = new NodeEntry(p_node);
  m_nodeList += entry;
  entry->SetLevel(p_level);
  if (p_node.NumChildren() > 0) {
    for (int i = 1; i <= p_node.NumChildren(); i++) {
      BuildNodeList(p_node.GetChild(i), p_support, p_level + 1);
    }
  }
  m_maxLevel = gmax(p_level, m_maxLevel);
}

void efgTreeLayout::BuildNodeList(const EFSupport &p_support)
{
  while (m_nodeList.Length() > 0) {
    delete m_nodeList.Remove(1);
  }

  m_maxLevel = 0;
  BuildNodeList(m_doc->m_efg->RootNode(), p_support, 0);
}


void efgTreeLayout::GenerateLabels(void)
{
  const gbtPreferences &prefs = m_doc->GetPreferences();
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    entry->SetNodeAboveLabel(CreateNodeLabel(entry, prefs.NodeAboveLabel()));
    entry->SetNodeAboveFont(prefs.NodeAboveFont());
    entry->SetNodeBelowLabel(CreateNodeLabel(entry, prefs.NodeBelowLabel()));
    entry->SetNodeBelowFont(prefs.NodeBelowFont());
    entry->SetNodeRightLabel(CreateOutcomeLabel(entry));
    entry->SetNodeRightFont(prefs.NodeRightFont());
    if (entry->GetChildNumber() > 0) {
      entry->SetBranchAboveLabel(CreateBranchLabel(entry,
						   prefs.BranchAboveLabel()));
      entry->SetBranchAboveFont(prefs.BranchAboveFont());
      entry->SetBranchBelowLabel(CreateBranchLabel(entry,
						   prefs.BranchBelowLabel()));
      entry->SetBranchBelowFont(prefs.BranchBelowFont());
      entry->SetActionProb(m_doc->ActionProb(entry->GetNode().GetParent(),
					     entry->GetChildNumber()));
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
void efgTreeLayout::RenderSubtree(wxDC &p_dc) const
{
  const gbtPreferences &prefs = m_doc->GetPreferences();

  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];  
    NodeEntry *parentEntry = entry->GetParent();
        
    if (entry->GetChildNumber() == 1) {
      parentEntry->Draw(p_dc);

      if (m_doc->GetPreferences().InfosetConnect() != GBT_INFOSET_CONNECT_NONE &&
	  parentEntry->GetNextMember()) {
	int nextX = parentEntry->GetNextMember()->X();
	int nextY = parentEntry->GetNextMember()->Y();

	if ((m_doc->GetPreferences().InfosetConnect() !=
	     GBT_INFOSET_CONNECT_SAMELEVEL) ||
	    parentEntry->X() == nextX) {
#ifdef __WXGTK__
	  // A problem with using styled pens and user scaling on wxGTK
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxSOLID));
#else
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxDOT));
#endif   // __WXGTK__
	  p_dc.DrawLine(parentEntry->X(), parentEntry->Y(), 
			parentEntry->X(), nextY);
	  if (prefs.InfosetJoin() == GBT_INFOSET_JOIN_CIRCLES) {
	    p_dc.DrawLine(parentEntry->X() + parentEntry->GetSize(), 
			  parentEntry->Y(),
			  parentEntry->X() + parentEntry->GetSize(), 
			  nextY);
	  }

	  if (parentEntry->GetNextMember()->X() != parentEntry->X()) {
	    // Draw a little arrow in the direction of the iset.
	    int startX, endX; 
	    if (prefs.InfosetJoin() == GBT_INFOSET_JOIN_LINES) {
	      startX = parentEntry->X();
	      endX = (startX + m_infosetSpacing * 
		      ((parentEntry->GetNextMember()->X() > 
			parentEntry->X()) ? 1 : -1));
	    }
	    else {
	      if (parentEntry->GetNextMember()->X() < parentEntry->X()) {
		// information set is continued to the left
		startX = parentEntry->X() + parentEntry->GetSize();
		endX = parentEntry->X() - m_infosetSpacing;
	      }
	      else {
		// information set is continued to the right
		startX = parentEntry->X();
		endX = (parentEntry->X() + parentEntry->GetSize() + 
			m_infosetSpacing);
	      }
	    }
	    p_dc.DrawLine(startX, nextY, endX, nextY);
	    if (startX > endX) {
	      p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2,
			    nextY + m_infosetSpacing / 2);
	      p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2,
			    nextY - m_infosetSpacing / 2);
	    }
	    else {
	      p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2,
			    nextY + m_infosetSpacing / 2);
	      p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2,
			    nextY - m_infosetSpacing / 2);
	    }
	  }
	}
      }
    }

    if (entry->GetNode().NumChildren() == 0) {
      entry->Draw(p_dc);
    }

  }
}

void efgTreeLayout::Render(wxDC &p_dc) const
{ 
  RenderSubtree(p_dc);
}

void efgTreeLayout::SetCutNode(const gbtEfgNode &p_node)
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    m_nodeList[i]->SetCut(p_node.IsPredecessor(m_nodeList[i]->GetNode()));
  }
}
