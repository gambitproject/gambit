//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to tree layout representation
//

#ifndef EFGLAYOUT_H
#define EFGLAYOUT_H

#include "base/base.h"
#include "treedraw.h"
#include "efgconst.h"
#include "efg.h"

class NodeEntry {
private:
  Node *m_node;       // the corresponding node in the game
  bool m_selected;    // true if node is selected
  bool m_cursor;      // true if node is 'cursor'
  bool m_subgameRoot, m_subgameMarked;
  int m_size;         // horizontal size of the node
  int m_token;        // token to draw for node

  int m_level;        // depth of the node in tree
  int m_sublevel;     // # of the infoset line on this level

  wxString m_nodeAboveLabel, m_nodeBelowLabel, m_nodeRightLabel;
  wxString m_branchAboveLabel, m_branchBelowLabel;

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
  void SetCursor(bool p_cursor);

  bool IsSelected(void) const { return m_selected; }
  void SetSelected(bool p_selected) { m_selected = p_selected; }

  bool IsSubgameRoot(void) const { return m_subgameRoot; }
  void SetSubgameRoot(bool p_root) { m_subgameRoot = p_root; }
  
  bool IsSubgameMarked(void) const { return m_subgameMarked; }
  void SetSubgameMarked(bool p_marked) { m_subgameMarked = p_marked; }

  void SetSize(int p_size) { m_size = p_size; }
  int GetSize(void) const { return m_size; }

  void SetToken(int p_token) { m_token = p_token; }
  int GetToken(void) const { return m_token; }

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

  int GetX(void) const;

  bool NodeHitTest(int p_x, int p_y) const;

  void Draw(wxDC &) const;
  void DrawIncomingBranch(wxDC &) const;
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

  const int c_leftMargin, c_topMargin;

  NodeEntry *GetEntry(Node *) const;

  NodeEntry *NextInfoset(NodeEntry *);
  void CheckInfosetEntry(NodeEntry *);
  int FillTable(Node *, const EFSupport &, int, int &, int &, int &, int &);
  void FillInfosetTable(Node *, const EFSupport &);
  void UpdateTableInfosets(void);
  void UpdateTableParents(void);

  wxString CreateNodeAboveLabel(const NodeEntry *) const;
  wxString CreateNodeBelowLabel(const NodeEntry *) const;
  wxString CreateNodeRightLabel(const NodeEntry *) const;
  wxString CreateBranchAboveLabel(const NodeEntry *) const;
  wxString CreateBranchBelowLabel(const NodeEntry *) const;

  void RenderSubtree(wxDC &dc) const;

public:
  efgTreeLayout(FullEfg &, TreeWindow *);
  virtual ~efgTreeLayout() { }

  Node *PriorSameLevel(Node *) const;
  Node *NextSameLevel(Node *) const;

  void Layout(const EFSupport &);
  void GenerateLabels(void);

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
