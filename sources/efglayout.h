//
// FILE: efglayout.h -- Declaration of extensive form layout
//
// $id$
//

#ifndef EFGLAYOUT_H
#define EFGLAYOUT_H

#include "wx/wx.h"

#include "garray.h"
#include "glist.h"
#include "treedraw.h"
#include "efgconst.h"
#include "efg.h"

class NodeEntry {
public:
  int x, y, level;
  wxColour color;
  // pos of the next node in the infoset to connect to
  struct { int x, y; } infoset;
  int num;            // # of the infoset line on this level
  int nums;           // sum of infosets previous to this level
  int has_children;   // how many children this node has
  int child_number;   // what branch # is this node from the parent
  bool in_sup;        // is this node in cur_sup
  Node *n;
  NodeEntry *parent;
  bool expanded;      // Is this subgame root expanded or collapsed?
    
  NodeEntry(void) { }
  NodeEntry(const NodeEntry &e)
    : x(e.x), y(e.y), level(e.level), color(e.color),
      infoset(e.infoset),num(e.num),
      nums(e.nums),has_children(e.has_children),
      child_number(e.child_number),in_sup(e.in_sup),
      n(e.n),expanded(e.expanded) { }
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
