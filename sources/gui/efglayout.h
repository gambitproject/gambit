//
// FILE: efglayout.h -- Declaration of extensive form layout
//
// $Id$
//

#ifndef EFGLAYOUT_H
#define EFGLAYOUT_H

#include "wx/wx.h"

#include "base/base.h"
#include "treedraw.h"
#include "efgconst.h"
#include "efg.h"

class NodeEntry {
public:
  typedef enum { tokenLINE = 0, tokenELLIPSE, tokenRECTANGLE } NodeType;

private:
  Node *m_node;       // the corresponding node in the game
  bool m_selected;    // true if node is selected
  bool m_cursor;      // true if node is 'cursor'
  int m_size;         // horizontal size of the node
  NodeType m_token;   // token to draw for node

  int m_level;        // depth of the node in tree
  int m_sublevel;     // # of the infoset line on this level

  wxString m_nodeAboveLabel, m_nodeBelowLabel;

public:
  int x, y;
  wxColour color;
  // pos of the next node in the infoset to connect to
  struct { int x, y; } infoset;
  int nums;           // sum of infosets previous to this level
  int has_children;   // how many children this node has
  int child_number;   // what branch # is this node from the parent
  bool in_sup;        // is this node in cur_sup
  NodeEntry *parent;
  bool expanded;      // Is this subgame root expanded or collapsed?

  NodeEntry(Node *);

  Node *GetNode(void) const { return m_node; }

  bool IsCursor(void) const { return m_cursor; }
  bool IsSelected(void) const { return m_selected; }
  void SetCursor(bool p_cursor);
  void SetSelected(bool p_selected) { m_selected = p_selected; }

  void SetSize(int p_size) { m_size = p_size; }
  int GetSize(void) const { return m_size; }

  void SetToken(NodeType p_token) { m_token = p_token; }
  NodeType GetToken(void) const { return m_token; }

  void SetLevel(int p_level) { m_level = p_level; }
  int GetLevel(void) const { return m_level; }
  void SetSublevel(int p_sublevel) { m_sublevel = p_sublevel; }
  int GetSublevel(void) const { return m_sublevel; }

  const wxString &GetNodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(const wxString &p_label)
    { m_nodeAboveLabel = p_label; }

  const wxString &GetNodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(const wxString &p_label)
    { m_nodeBelowLabel = p_label; }

  int GetX(void) const;

  void Draw(wxDC &) const;
};

class SubgameEntry {
public:
  Node *root;
  bool expanded;

  SubgameEntry(Node *r = 0, bool e = true) : root(r), expanded(e) { }

  // Need these to make a list:
  bool operator==(const SubgameEntry &s) { return (s.root == root); }
  bool operator!=(const SubgameEntry &s) { return (s.root != root); }
  friend gOutput &operator<<(gOutput &, const SubgameEntry &);
};


class TreeWindow;

class efgTreeLayout {
private:
  FullEfg &m_efg;
  TreeWindow *m_parent;
  gList<NodeEntry *> m_nodeList;
  gList<SubgameEntry> m_subgameList;
  int m_maxX, m_maxY, m_maxlev;

  NodeEntry *GetEntry(Node *) const;

  NodeEntry *NextInfoset(NodeEntry *);
  void CheckInfosetEntry(NodeEntry *);
  int FillTable(Node *, const EFSupport &, int, int &, int &, int &, int &);
  void FillInfosetTable(Node *, const EFSupport &);
  void UpdateTableInfosets(void);
  void UpdateTableParents(void);

  wxString CreateNodeAboveLabel(const NodeEntry *) const;
  wxString CreateNodeBelowLabel(const NodeEntry *) const;

  void RenderLabels(wxDC &dc, const NodeEntry *child_entry,
		    const NodeEntry *entry) const;
  void RenderSubtree(wxDC &dc) const;

public:
  efgTreeLayout(FullEfg &, TreeWindow *);
  virtual ~efgTreeLayout() { }

  Node *PriorSameLevel(Node *) const;
  Node *NextSameLevel(Node *) const;

  void Layout(const EFSupport &);

  // The following member functions are for temporary compatibility only
  NodeEntry *GetNodeEntry(Node *p_node) const
    { return GetEntry(p_node); }
  gList<SubgameEntry> &SubgameList(void) { return m_subgameList; }
  SubgameEntry &GetSubgameEntry(Node *p_node);
  NodeEntry *GetValidParent(Node *);
  NodeEntry *GetValidChild(Node *);

  int MaxX(void) const { return m_maxX; }
  int MaxY(void) const { return m_maxY; }

  Node *NodeHitTest(int, int) const;
  Node *NodeAboveHitTest(int, int) const;
  Node *NodeBelowHitTest(int, int) const;
  Node *NodeRightHitTest(int, int) const;
  Node *BranchHitTest(int, int) const;
  Node *BranchAboveHitTest(int, int) const;
  Node *BranchBelowHitTest(int, int) const;
  Node *SubgameHitTest(int, int) const;
  Node *InfosetHitTest(int, int) const;

  void Render(wxDC &) const;
};

#endif  // EFGLAYOUT_H
