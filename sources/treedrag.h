//
// FILE: treedrag.h -- Interface to dragging implementation classes
//
// $Id$
//

#ifndef TREEDRAG_H
#define TREEDRAG_H

typedef enum { DRAG_NODE_START = 0, DRAG_NODE_END = 1,
	       DRAG_ISET_START = 2, DRAG_ISET_END = 3,
	       DRAG_BRANCH_START = 4, DRAG_BRANCH_END = 5,
	       DRAG_OUTCOME_START = 6, DRAG_OUTCOME_END = 7 } guiDragType;

typedef enum { DRAG_NONE = 0, DRAG_START = 1, DRAG_CONTINUE = 2,
	       DRAG_STOP = 3 } guiDragState;

//
// This class enables the user to merge infosets by dragging a line from the
// first infoset's maker to the second one's.
//
class TreeWindow::IsetDragger {
private:
  FullEfg &ef;
  TreeWindow *parent;
  int drag_now;
  long x, y, ox, oy, sx, sy;    // current, previous, start positions
  Node *start_node, *end_node;
    
  void RedrawObject(void);
    
public:
  IsetDragger(TreeWindow *parent, FullEfg &ef);
  ~IsetDragger();
    
  int OnEvent(wxMouseEvent &ev);
  int Dragging(void) const { return drag_now; }
    
  Node *StartNode(void) { return start_node; }
  Node *EndNode(void) { return end_node; }
};

//
// This class allows the user to add branches to a node/infoset by dragging a
// line from the base of the branches to the position the new branch is to
// occupy.  If the node was terminal, a player dialog is opened to 
// choose a player.
//
class TreeWindow::BranchDragger {
private:
  FullEfg &ef;
  TreeWindow *parent;
  int drag_now;
  int br;
  long x, y, ox, oy, sx, sy;    // current, previous, start positions
  Node *start_node;
    
  void RedrawObject(void);
    
public:
  BranchDragger(TreeWindow *parent, FullEfg &ef);
  ~BranchDragger();
    
  int OnEvent(wxMouseEvent &ev);
  int Dragging(void) const { return drag_now; }
    
  Node *StartNode(void) { return start_node; }
  int BranchNum(void) { return br; }
};

#endif  // TREEDRAG_H


