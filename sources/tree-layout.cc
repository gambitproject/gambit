//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class to layout extensive form trees
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "tree-layout.h"
#include "game-document.h"

//--------------------------------------------------------------------------
//                          class gbtNodeEntry
//--------------------------------------------------------------------------

//
// This class encapsulates the data needed to represent the layout
// of a game tree.
//
class gbtNodeEntry {
private:
  gbtGameNode m_node;        // the corresponding node in the game
  gbtNodeEntry *m_parent; // parent node
  int m_x, m_y;        // Cartesian coordinates of node
  gbtNodeEntry *m_nextMember;  // entry of next information set member 
  bool m_selected;    // true if node is selected
  bool m_cursor;      // true if node is 'cursor'
  bool m_cut;         // true if node is in a 'cut' subtree
  int m_size;         // horizontal size of the node
  int m_token;        // token to draw for node
  wxColour m_color;   // color of node

  int m_branchStyle;  // lines or fork-tine
  int m_branchLabel;  // horizontal or rotated
  int m_branchLength; // length of branch (exclusive of tine, if present)

  int m_level;        // depth of the node in tree
  int m_sublevel;     // # of the infoset line on this level
  wxString m_actionProb;  // probability incoming action is taken

  wxString m_nodeAboveLabel, m_nodeBelowLabel, m_nodeRightLabel;
  wxString m_branchAboveLabel, m_branchBelowLabel;

  wxFont m_nodeAboveFont, m_nodeBelowFont, m_nodeRightFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

public:
  gbtNodeEntry(gbtGameNode p_parent);

  gbtGameNode GetNode(void) const { return m_node; }

  gbtNodeEntry *GetParent(void) const { return m_parent; }
  void SetParent(gbtNodeEntry *p_parent) { m_parent = p_parent; }

  int X(void) const { return m_x; }
  void SetX(int p_x) { m_x = p_x; }
  int Y(void) const { return m_y; }
  void SetY(int p_y) { m_y = p_y; }

  gbtNodeEntry *GetNextMember(void) const { return m_nextMember; }
  void SetNextMember(gbtNodeEntry *p_member) { m_nextMember = p_member; }

  int GetChildNumber(void) const; 

  const wxColour &GetColor(void) const { return m_color; }
  void SetColor(const wxColour &p_color) { m_color = p_color; }

  bool IsCursor(void) const { return m_cursor; }
  void SetCursor(bool p_cursor);

  bool IsSelected(void) const { return m_selected; }
  void SetSelected(bool p_selected) { m_selected = p_selected; }

  bool IsCut(void) const { return m_cut; }
  void SetCut(bool p_cut) { m_cut = p_cut; }

  int GetSize(void) const { return m_size; }
  void SetSize(int p_size) { m_size = p_size; }

  int GetToken(void) const { return m_token; }
  void SetToken(int p_token) { m_token = p_token; }

  int GetBranchStyle(void) const { return m_branchStyle; }
  void SetBranchStyle(int p_style) { m_branchStyle = p_style; }

  int GetBranchLabelStyle(void) const { return m_branchLabel; }
  void SetBranchLabelStyle(int p_style) { m_branchLabel = p_style; }

  int GetBranchLength(void) const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int GetLevel(void) const { return m_level; }
  void SetLevel(int p_level) { m_level = p_level; }

  int GetSublevel(void) const { return m_sublevel; }
  void SetSublevel(int p_sublevel) { m_sublevel = p_sublevel; }

  const wxString &GetNodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(const wxString &p_label)
    { m_nodeAboveLabel = p_label; }

  const wxString &GetNodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(const wxString &p_label)
    { m_nodeBelowLabel = p_label; }

  const wxString &GetNodeRightLabel(void) const { return m_nodeRightLabel; }
  void SetNodeRightLabel(const wxString &p_label)
    { m_nodeRightLabel = p_label; }

  const wxString &GetBranchAboveLabel(void) const 
    { return m_branchAboveLabel; }
  void SetBranchAboveLabel(const wxString &p_label)
    { m_branchAboveLabel = p_label; }

  const wxString &GetBranchBelowLabel(void) const 
    { return m_branchBelowLabel; }
  void SetBranchBelowLabel(const wxString &p_label)
    { m_branchBelowLabel = p_label; }

  const wxFont &GetNodeAboveFont(void) const { return m_nodeAboveFont; }
  void SetNodeAboveFont(const wxFont &p_font) { m_nodeAboveFont = p_font; }

  const wxFont &GetNodeBelowFont(void) const { return m_nodeBelowFont; }
  void SetNodeBelowFont(const wxFont &p_font) { m_nodeBelowFont = p_font; }

  const wxFont &GetNodeRightFont(void) const { return m_nodeRightFont; }
  void SetNodeRightFont(const wxFont &p_font) { m_nodeRightFont = p_font; }

  const wxFont &GetBranchAboveFont(void) const { return m_branchAboveFont; }
  void SetBranchAboveFont(const wxFont &p_font) { m_branchAboveFont = p_font; }

  const wxFont &GetBranchBelowFont(void) const { return m_branchBelowFont; }
  void SetBranchBelowFont(const wxFont &p_font) { m_branchBelowFont = p_font; }

  const wxString &GetActionProb(void) const { return m_actionProb; }
  void SetActionProb(const wxString &p_prob) { m_actionProb = p_prob; }

  bool NodeHitTest(int p_x, int p_y) const;

  void Draw(wxDC &) const;
  void DrawIncomingBranch(wxDC &) const;
};

gbtNodeEntry::gbtNodeEntry(gbtGameNode p_node)
  : m_node(p_node), m_parent(0),
    m_x(-1), m_y(-1), m_nextMember(0), 
    m_selected(false), m_cursor(false), m_cut(false),
    m_size(20),
    m_token(GBT_NODE_TOKEN_CIRCLE),
    m_branchStyle(GBT_BRANCH_STYLE_LINE), 
    m_branchLabel(GBT_BRANCH_LABEL_HORIZONTAL),
    m_branchLength(0),
    m_sublevel(0), m_actionProb("")
{ }

int gbtNodeEntry::GetChildNumber(void) const
{
  if (!m_node->GetParent().IsNull()) {
    return m_node->GetPriorAction()->GetId();
  }
  else {
    return 0;
  }
}

void gbtNodeEntry::SetCursor(bool p_cursor)
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
void gbtNodeEntry::Draw(wxDC &p_dc) const
{
  if (!m_node->GetParent().IsNull()) {
    DrawIncomingBranch(p_dc);
  }

  if (m_cut) {
    p_dc.SetPen(*wxLIGHT_GREY_PEN);
  }
  else {
    p_dc.SetPen(*wxThePenList->FindOrCreatePen(m_color, (IsSelected()) ? 6 : 3,
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
}


void gbtNodeEntry::DrawIncomingBranch(wxDC &p_dc) const
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
					       4, wxSOLID)); 
  }
  if (m_branchStyle == GBT_BRANCH_STYLE_LINE) {
    p_dc.DrawLine(xStart, yStart, xEnd, yEnd);

    /*
    // Draw in the highlight indicating action probabilities
    if (m_actionProb != "") {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 4, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart +
		    (int) ((double) (xEnd - xStart) * (double) m_actionProb),
		    yStart +
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }
    */

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
    /*
    if (m_actionProb >= gNumber(0)) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart + 
		    (int) ((double) m_branchLength * (double) m_actionProb),
		    yStart + 
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }
    */

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

bool gbtNodeEntry::NodeHitTest(int p_x, int p_y) const
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

//--------------------------------------------------------------------------
//                          class gbtTreeLayout
//--------------------------------------------------------------------------

gbtTreeLayout::gbtTreeLayout(gbtGameDocument *p_doc)
  : m_doc(p_doc), m_infosetSpacing(40),
    c_leftMargin(20), c_topMargin(40)
{ }

gbtGameNode gbtTreeLayout::NodeHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->NodeHitTest(p_x, p_y)) {
      return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

gbtGameNode gbtTreeLayout::BranchHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    gbtNodeEntry *entry = m_nodeList[i];
    gbtNodeEntry *parent_entry = GetEntry(entry->GetNode()->GetParent());

    if (parent_entry) {
      if (p_x > (parent_entry->X() + m_doc->GetTreeOptions().GetNodeSize() + 
		 parent_entry->GetSublevel() * m_infosetSpacing + 10) &&
	  p_x < (parent_entry->X() + m_doc->GetTreeOptions().GetNodeSize() +
		 m_doc->GetTreeOptions().GetBranchLength() +
		 parent_entry->GetSublevel() * m_infosetSpacing)) {
	// Good old slope/intercept method for finding a point on a line
	int y0 = (parent_entry->Y() + 
		  (int) (p_x - parent_entry->X() - 
			 m_doc->GetTreeOptions().GetNodeSize()) *
		  (entry->Y() - parent_entry->Y()) / 
		  m_doc->GetTreeOptions().GetBranchLength());

	if (p_y > y0-2 && p_y < y0+2) {
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}


gbtGameNode gbtTreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    gbtNodeEntry *entry = m_nodeList[i];
    if (entry->GetNextMember() && 
	!entry->GetNode()->GetInfoset().IsNull()) {
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

wxString gbtTreeLayout::CreateNodeLabel(const gbtNodeEntry *p_entry,
					int p_type) const
{
  gbtGameNode n = p_entry->GetNode();
    
  switch (p_type) {
  case GBT_LABEL_NODE_NONE:
    return _T("");
  case GBT_LABEL_NODE_LABEL:
    return wxString::Format(_T("%s"), n->GetLabel().c_str());
  case GBT_LABEL_NODE_PLAYER:
    if (!n->GetPlayer().IsNull()) {
      return wxString::Format(_T("%s"), n->GetPlayer()->GetLabel().c_str());
    }
    else {
      return _T("");
    }
  case GBT_LABEL_NODE_INFOSET:
    if (!n->GetInfoset().IsNull()) {
      return wxString::Format(_T("%s"), n->GetInfoset()->GetLabel().c_str());
    }
    else {
      return _T("");
    }
  case GBT_LABEL_NODE_INFOSETID:
    if (!n->GetInfoset().IsNull()) {
      return wxString::Format(_T("%d,%d"),
			      n->GetPlayer()->GetId(),
			      n->GetInfoset()->GetId());
    }
    else {
      return _T("");
    }
  default:
    return _T("");
  }
}    

wxString gbtTreeLayout::CreateOutcomeLabel(const gbtNodeEntry *p_entry,
					   int p_type) const
{    
  gbtGameNode node = p_entry->GetNode();

  switch (p_type) {
  case GBT_LABEL_OUTCOME_PAYOFFS:
    /*
    return wxString::Format(_T("%s"),
			    (const char *) m_parent->OutcomeAsString(node));
    */
    return _T("");
  case GBT_LABEL_OUTCOME_LABEL:
    if (!node->GetOutcome().IsNull()) {
      return wxString::Format(_T("%s"), 
			      node->GetOutcome()->GetLabel().c_str());
    }
    else {
      return _T("");
    }
  default:
    return _T("");
  }
}

wxString gbtTreeLayout::CreateBranchLabel(const gbtNodeEntry *p_entry,
					  int p_type) const
{
  gbtGameNode parent = p_entry->GetParent()->GetNode();

  switch (p_type) {
  case GBT_LABEL_BRANCH_NONE:
    return _T("");
  case GBT_LABEL_BRANCH_LABEL:
    return wxString::Format(_T("%s"), 
			    parent->GetInfoset()->GetAction(p_entry->GetChildNumber())->GetLabel().c_str());
  case GBT_LABEL_BRANCH_PROB:
    /*
    return wxString::Format(_T("%s"), 
			    m_parent->Parent()->GetActionProb(parent,
							    p_entry->GetChildNumber()));
    */
    return _T("");
  default:
    return _T("");
  }
}

gbtNodeEntry *gbtTreeLayout::GetEntry(gbtGameNode p_node) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->GetNode() == p_node) {
      return m_nodeList[i];
    }
  }
  return 0;
}

gbtGameNode gbtTreeLayout::PriorSameLevel(gbtGameNode p_node) const
{
  gbtNodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) - 1; i >= 1; i--) {
      if (m_nodeList[i]->GetLevel() == entry->GetLevel())
	return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

gbtGameNode gbtTreeLayout::NextSameLevel(gbtGameNode p_node) const
{
  gbtNodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) + 1; i <= m_nodeList.Length(); i++) {
      if (m_nodeList[i]->GetLevel() == entry->GetLevel()) { 
	return m_nodeList[i]->GetNode();
      }
    }
  }
  return 0;
}

int gbtTreeLayout::LayoutSubtree(gbtGameNode p_node, 
				 int &p_maxy, int &p_miny, int &p_ycoord)
{
  int y1 = -1, yn = 0;
    
  gbtNodeEntry *entry = m_nodeList[p_node->GetId()];
  entry->SetNextMember(0);
  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      yn = LayoutSubtree(p_node->GetChild(i), 
			 p_maxy, p_miny, p_ycoord);
      if (y1 == -1) {
	y1 = yn;
      }
    }
    entry->SetY((y1 + yn) / 2);
  }
  else {
    entry->SetY(p_ycoord);
    p_ycoord += m_doc->GetTreeOptions().GetTerminalSpacing();
  }
    
  if (m_doc->GetTreeOptions().GetBranchStyle() == GBT_BRANCH_STYLE_LINE) {
    entry->SetX(c_leftMargin + 
		entry->GetLevel() * (m_doc->GetTreeOptions().GetNodeSize() +
				     m_doc->GetTreeOptions().GetBranchLength()));
  }
  else {
    entry->SetX(c_leftMargin + 
		entry->GetLevel() * (m_doc->GetTreeOptions().GetNodeSize() +
				     m_doc->GetTreeOptions().GetBranchLength() +
				     m_doc->GetTreeOptions().GetTineLength()));
  }

  if (!p_node->GetPlayer().IsNull() && p_node->GetPlayer()->IsChance()) {
    entry->SetColor(m_doc->GetPlayerColor(0));
    entry->SetToken(m_doc->GetTreeOptions().GetChanceToken());
  }
  else if (!p_node->GetPlayer().IsNull()) {
    entry->SetColor(m_doc->GetPlayerColor(p_node->GetPlayer()->GetId()));
    entry->SetToken(m_doc->GetTreeOptions().GetPlayerToken(p_node->GetPlayer()->GetId()));
  }
  else {
    entry->SetColor(*wxBLACK);
    entry->SetToken(m_doc->GetTreeOptions().GetTerminalToken());
  }  
  
  entry->SetSize(m_doc->GetTreeOptions().GetNodeSize());
  entry->SetBranchStyle(m_doc->GetTreeOptions().GetBranchStyle());
  if (m_doc->GetTreeOptions().GetBranchStyle() == GBT_BRANCH_STYLE_LINE) {
    entry->SetBranchLabelStyle(m_doc->GetTreeOptions().GetBranchLabelStyle());
  }
  entry->SetBranchLength(m_doc->GetTreeOptions().GetBranchLength());

  p_maxy = gmax(entry->Y(), p_maxy);
  p_miny = gmin(entry->Y(), p_miny);
    
  return entry->Y();
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
gbtNodeEntry *gbtTreeLayout::NextInfoset(gbtNodeEntry *e)
{
  for (int pos = m_nodeList.Find(e) + 1; pos <= m_nodeList.Length(); pos++) {
    gbtNodeEntry *e1 = m_nodeList[pos];
    // infosets are the same and the nodes are on the same level
    if (e->GetNode()->GetInfoset() == e1->GetNode()->GetInfoset()) {
      if (m_doc->GetTreeOptions().GetInfosetStyle() == GBT_INFOSET_CONNECT_ALL) {
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
void gbtTreeLayout::CheckInfosetEntry(gbtNodeEntry *e)
{
  int pos;
  gbtNodeEntry *infoset_entry, *e1;
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

void gbtTreeLayout::FillInfosetTable(gbtGameNode n)
{
  gbtNodeEntry *entry = GetEntry(n);
  for (int i = 1; i <= n->NumChildren(); i++) {
    FillInfosetTable(n->GetChild(i));
  }

  if (entry) {
    CheckInfosetEntry(entry);
  }
}

void gbtTreeLayout::UpdateTableInfosets(void)
{
  // Note that levels are numbered from 0, not 1.
  // create an array to hold max num for each level
  gbtArray<int> nums(0, m_maxLevel + 1); 
    
  for (int i = 0; i <= m_maxLevel + 1; nums[i++] = 0);
  // find the max e->num for each level
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    gbtNodeEntry *entry = m_nodeList[pos];
    nums[entry->GetLevel()] = gmax(entry->GetSublevel() + 1,
				   nums[entry->GetLevel()]);
  }
    
  for (int i = 0; i <= m_maxLevel; i++) {
    nums[i+1] += nums[i];
  }
    
  // now add the needed length to each level, and set maxX accordingly
  m_maxX = 0;
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    gbtNodeEntry *entry = m_nodeList[pos];
    if (entry->GetLevel() != 0) {
      entry->SetX(entry->X() + 
		  (nums[entry->GetLevel()-1] +
		   entry->GetSublevel()) * m_infosetSpacing);
    }
    m_maxX = gmax(m_maxX, entry->X());
  }
}

void gbtTreeLayout::UpdateTableParents(void)
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    gbtNodeEntry *e = m_nodeList[pos];
    e->SetParent((e->GetNode() == m_doc->GetGame()->GetRoot()) ? 
		 e : GetEntry(e->GetNode()->GetParent()));
  }
}

void gbtTreeLayout::Layout(void)
{
  // Kinda kludgey; probably should query draw settings whenever needed.
  m_infosetSpacing = 
    (m_doc->GetTreeOptions().GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) ? 10 : 40;

  if (m_nodeList.Length() != m_doc->GetGame()->NumNodes()) {
    // A rebuild is in order; force it
    BuildNodeList();
  }

  int miny = 0, maxy = 0, ycoord = c_topMargin;
  LayoutSubtree(m_doc->GetGame()->GetRoot(), maxy, miny, ycoord);

  if (m_doc->GetTreeOptions().GetInfosetStyle() != GBT_INFOSET_CONNECT_NONE) {
    // FIXME! This causes lines to disappear... sometimes.
    FillInfosetTable(m_doc->GetGame()->GetRoot());
    UpdateTableInfosets();
  }

  UpdateTableParents();
  GenerateLabels();

  const int OUTCOME_LENGTH = 60;

  m_maxX += m_doc->GetTreeOptions().GetNodeSize() + OUTCOME_LENGTH;
  m_maxY = maxy + 25;
}

void gbtTreeLayout::BuildNodeList(gbtGameNode p_node, int p_level)
{
  gbtNodeEntry *entry = new gbtNodeEntry(p_node);
  m_nodeList += entry;
  entry->SetLevel(p_level);
  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      BuildNodeList(p_node->GetChild(i), p_level + 1);
    }
  }
  m_maxLevel = gmax(p_level, m_maxLevel);
}

void gbtTreeLayout::BuildNodeList(void)
{
  while (m_nodeList.Length() > 0) {
    delete m_nodeList.Remove(1);
  }

  m_maxLevel = 0;
  BuildNodeList(m_doc->GetGame()->GetRoot(), 0);
}


void gbtTreeLayout::GenerateLabels(void)
{
  const gbtTreeLayoutOptions &options = m_doc->GetTreeOptions();

  for (int i = 1; i <= m_nodeList.Length(); i++) {
    gbtNodeEntry *entry = m_nodeList[i];
    entry->SetNodeAboveLabel(CreateNodeLabel(entry, 
					     options.GetNodeAboveLabel()));
    entry->SetNodeAboveFont(options.GetTreeLabelFont());
    entry->SetNodeBelowLabel(CreateNodeLabel(entry,
					     options.GetNodeBelowLabel()));
    entry->SetNodeBelowFont(options.GetTreeLabelFont());
    entry->SetNodeRightLabel(CreateOutcomeLabel(entry,
						options.GetOutcomeLabel()));
    entry->SetNodeRightFont(options.GetTreeLabelFont());
    if (entry->GetChildNumber() > 0) {
      entry->SetBranchAboveLabel(CreateBranchLabel(entry,
						   options.GetBranchAboveLabel()));
      entry->SetBranchAboveFont(options.GetTreeLabelFont());
      entry->SetBranchBelowLabel(CreateBranchLabel(entry,
						   options.GetBranchBelowLabel()));
      entry->SetBranchBelowFont(options.GetTreeLabelFont());
      /*
      entry->SetActionProb(m_parent->Parent()->ActionProb(entry->GetNode()->GetParent(),
							  entry->GetChildNumber()));
      */
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
void gbtTreeLayout::RenderSubtree(wxDC &p_dc) const
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    gbtNodeEntry *entry = m_nodeList[pos];  
    gbtNodeEntry *parentEntry = entry->GetParent();
        
    if (entry->GetChildNumber() == 1) {
      parentEntry->Draw(p_dc);

      if (m_doc->GetTreeOptions().GetInfosetStyle() != GBT_INFOSET_CONNECT_NONE &&
	  parentEntry->GetNextMember()) {
	int nextX = parentEntry->GetNextMember()->X();
	int nextY = parentEntry->GetNextMember()->Y();

	if ((m_doc->GetTreeOptions().GetInfosetStyle() != GBT_INFOSET_CONNECT_SAMELEVEL) ||
	    parentEntry->X() == nextX) {
#ifdef __WXGTK__
	  // A problem with using styled pens and user scaling on wxGTK
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxSOLID));
#else
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxDOT));
#endif   // __WXGTK__
	  p_dc.DrawLine(parentEntry->X(), parentEntry->Y(), 
			parentEntry->X(), nextY);
	  if (m_doc->GetTreeOptions().GetInfosetJoin() == GBT_INFOSET_JOIN_CIRCLES) {
	    p_dc.DrawLine(parentEntry->X() + parentEntry->GetSize(), 
			  parentEntry->Y(),
			  parentEntry->X() + parentEntry->GetSize(), 
			  nextY);
	  }

	  if (parentEntry->GetNextMember()->X() != parentEntry->X()) {
	    // Draw a little arrow in the direction of the iset.
	    int startX, endX; 
	    if (m_doc->GetTreeOptions().GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) {
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

    if (entry->GetNode()->NumChildren() == 0) {
      entry->Draw(p_dc);
    }

  }
}

void gbtTreeLayout::DrawTree(wxDC &p_dc) const
{ 
  RenderSubtree(p_dc);
}

void gbtTreeLayout::SetCutNode(gbtGameNode p_node, bool p_cut)
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (p_node->IsPredecessorOf(m_nodeList[i]->GetNode())) {
      m_nodeList[i]->SetCut(p_cut);
    }
  }
}

