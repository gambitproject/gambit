//
// FILE: treewin.h -- Interface for TreeWindow class
//
// $Id$
//

#ifndef TREEWIN_H
#define TREEWIN_H

#include "treerender.h"
#include "wxmisc.h"

class TreeWindow : public TreeRender, public EfgClient {
  friend class ExtensivePrintout;

private:
  FullEfg &ef;
  EfgShow *m_parent;
  
  Node  *mark_node;                 // Used in mark/goto node operations
  const Node    *subgame_node;      // Used to mark the 'picking' subgame root
  guiNodeList node_list;     // Data for display coordinates of nodes

  Infoset *hilight_infoset;       // Hilight infoset from the solution disp
  Infoset *hilight_infoset1;      // Hilight infoset by pressing control
  wxMenu *m_editMenu;           // a popup menu, equivalent to top level edit

  class NodeDragger;              // Class to take care of tree copy/move by
  NodeDragger *node_drag;         // drag and dropping nodes.

  class IsetDragger;              // Class to take care of iset join by
  IsetDragger *iset_drag;         // drag and dropping.

  class BranchDragger;            // Class to take care of branch addition by
  BranchDragger *branch_drag;     // drag and dropping

  class OutcomeDragger;           // Class to take care of outcome copy/move
  OutcomeDragger *outcome_drag;   // by drag and dropping

  float m_zoom;
  bool m_needsLayout;

  // Private Functions
  void FitZoom(void);

  void  ProcessCursor(void);
  bool  ProcessShift(wxMouseEvent &ev);
  void MakeMenus(void);

  // Event handlers
  void OnSize(wxSizeEvent &);
  void OnMouseMotion(wxMouseEvent &);
  void OnLeftClick(wxMouseEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnLeftDoubleClick(wxMouseEvent &);
  void OnRightDoubleClick(wxMouseEvent &);
  void OnKeyEvent(wxKeyEvent &);
    
  // EfgClient members
  void OnTreeChanged(bool, bool);

protected:
  Node *m_cursor;  // Used to process cursor keys, stores current position.
  bool    outcomes_changed;
  TreeDrawSettings draw_settings;  // Stores drawing parameters

public:
  TreeWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~TreeWindow();
    
  // EVENT HANDLERS
  void OnEvent(wxMouseEvent& event);
    
  // MENU EVENT HANDLERS
  void node_set_mark(void);
  void node_goto_mark(void);
      
  void SubgameMarkAll(void);
  void SubgameMark(void);
  void SubgameUnmarkAll(void);
  void SubgameUnmark(void);
  void SubgameCollapse(void);
  void SubgameCollapseAll(void);
  void SubgameExpand(void);
  void SubgameExpandBranch(void);
  void SubgameExpandAll(void);

  void subgame_toggle(void);
    
  void prefs_display_flashing(void);

  void  output(void);
  void  print_eps(wxOutputOption fit);                 // output to postscript file
  void  print(wxOutputOption fit,bool preview=false);  // output to printer (WIN3.1 only)
  
  // copy to clipboard (WIN3.1 only)
  void  print_mf(wxOutputOption fit,bool save_mf=false);
  
  EfgShow *Parent(void) const { return m_parent; }
  
  void Render(void);
  virtual void Render(wxDC &dc);
  void HilightInfoset(int pl,int iset);

  void ForceRecalc(void);
  void OutcomeChange(void) { outcomes_changed = true; }
  
  // Used by parent EfgShow when cur_sup changes
  void SupportChanged(void);
  
  // Gives access to the parent to the private draw_settings. Used for SolnShow
  TreeDrawSettings &DrawSettings(void) { return draw_settings; }
  int NumDecimals(void) const { return draw_settings.NumDecimals(); }
  
  // Gives access to the parent to the current cursor node
  Node *Cursor(void) const { return m_cursor; }
  void SetCursorPosition(Node *p_cursor);
  Node *MarkNode(void) const { return mark_node; }
    
  Infoset *HighlightInfoset(void) const { return hilight_infoset; }
  Infoset *HighlightInfoset1(void) const { return hilight_infoset1; }

  Node *SubgameNode(void) const { return (Node *) subgame_node; }

  guiNodeList &NodeList(void) { return node_list; }
  const TreeDrawSettings &DrawSettings(void) const { return draw_settings; }

  void UpdateMenus(void);
  
  // Hilight the subgame root for the currently active subgame
  void  SetSubgamePickNode(Node *n);
  
  // Check if a drag'n'drop object has been activated
  Node *GotObject(long &mx, long &my, int what);
  float GetZoom(void) const { return m_zoom; }
  
  // Access to the numeric values from the renderer
  gText AsString(TypedSolnValues what, const Node *n, int br = 0) const;

  DECLARE_EVENT_TABLE()
};

#endif   // TREEWIN_H

