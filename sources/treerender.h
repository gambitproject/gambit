//
// FILE: treerender.h -- Declaration of abstract tree-rendering class
//
// $Id$
//

#ifndef TREERENDER_H
#define TREERENDER_H

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


class TreeRender;

class guiNodeList : public gList<NodeEntry *> {
private:
  FullEfg &m_efg;
  TreeRender *m_parent;
  gList<SubgameEntry> m_subgameList;
  int m_maxX, m_maxY, m_maxlev;

  NodeEntry *GetEntry(Node *) const;

  NodeEntry *NextInfoset(NodeEntry *);
  void CheckInfosetEntry(NodeEntry *);
  int FillTable(Node *, const EFSupport &, int, int &, int &, int &, int &);
  void FillInfosetTable(Node *, const EFSupport &);
  void UpdateTableInfosets(void);
  void UpdateTableParents(void);

public:
  guiNodeList(FullEfg &, TreeRender *);
  virtual ~guiNodeList() { }

  Node *NodeHitTest(int p_x, int p_y, int p_forkLength) const;

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
};

class EfgShow;
class TreeWindow;
class TreeNodeCursor;


//
// This class can render an extensive form tree using a pre-calculated
// (in TreeWindow) list of NodeEntry.  It is used for rendering
// functions for the main TreeWindow display and for the optional
// 'unity zoom' zoom window.  Note that it does not have any data
// members of its own, but just references to those of its parent. This
// way we do not duplicate the data and only need one assignment regardless
// of the number of renderers.
//

class TreeRender : public wxScrolledWindow {
 private:
  // Private Functions
  void RenderLabels(wxDC &dc, const NodeEntry *child_entry,
		    const NodeEntry *entry);
  void RenderSubtree(wxDC &dc);
  gText OutcomeAsString(const Node *n, bool &hilight) const;

  // Event handlers
  void OnPaint(wxPaintEvent &);
  
protected:
  TreeWindow *parent;
  TreeNodeCursor *flasher;                // Used to flash/display the cursor
  bool painting;                          // Used to prevent re-entry.

  // Hilight infoset from the solution display
  virtual Infoset *HighlightInfoset(void) const = 0;
  // Hilight infoset by pressing control
  virtual Infoset *HighlightInfoset1(void) const = 0;

  // Mark node
  virtual Node *MarkNode(void) const = 0;
  // Subgame node
  virtual Node *SubgameNode(void) const = 0;
    
  virtual guiNodeList &NodeList(void) = 0;

public:
  TreeRender(wxWindow *frame, TreeWindow *parent);
  virtual ~TreeRender(void);
    
  // Windows event handlers
  virtual void Render(wxDC &dc);
    
  // Call this every time the cursor moves.
  virtual void UpdateCursor(void);

  virtual Node *Cursor(void) const = 0;
    
  // This must be here since we do not have draw_settings at constructor time
  void MakeFlasher(void);

  virtual int NumDecimals(void) const = 0;
  virtual float GetZoom(void) const = 0;

  virtual const TreeDrawSettings &DrawSettings(void) const = 0;

  DECLARE_EVENT_TABLE()
};

#endif  // TREERENDER_H
