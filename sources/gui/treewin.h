//
// FILE: treewin.h -- Interface for TreeWindow class
//
// $Id$
//

#ifndef TREEWIN_H
#define TREEWIN_H

#include "wx/dragimag.h"

#include "efglayout.h"
#include "twflash.h"
#include "guishare/wxmisc.h"

class EfgShow;

class TreeWindow : public wxScrolledWindow {
friend class EfgPrintout;

private:
  FullEfg &ef;
  EfgShow *m_parent;
  
  Node  *mark_node;                 // Used in mark/goto node operations
  const Node    *subgame_node;      // Used to mark the 'picking' subgame root
  efgTreeLayout m_layout;

  Infoset *hilight_infoset;       // Hilight infoset from the solution disp
  Infoset *hilight_infoset1;      // Hilight infoset by pressing control
  wxMenu *m_editMenu;           // a popup menu, equivalent to top level edit

  wxDragImage *m_dragImage;
  Node *m_dragSource;
  enum { dragCOPY, dragMOVE, dragOUTCOME } m_dragMode;

  class IsetDragger;              // Class to take care of iset join by
  IsetDragger *iset_drag;         // drag and dropping.

  class BranchDragger;            // Class to take care of branch addition by
  BranchDragger *branch_drag;     // drag and dropping

  TreeNodeCursor *flasher;                // Used to flash/display the cursor

  float m_zoom;

  // Private Functions
  void MakeFlasher(void);
  void UpdateCursor(void);
  void ProcessCursor(void);
  void EnsureCursorVisible(void);

  bool ProcessShift(wxMouseEvent &ev);
  void MakeMenus(void);
  void AdjustScrollbarSteps(void);

  // Event handlers
  void OnSize(wxSizeEvent &);
  void OnMouseMotion(wxMouseEvent &);
  void OnLeftClick(wxMouseEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnLeftDoubleClick(wxMouseEvent &);
  void OnKeyEvent(wxKeyEvent &);
    
protected:
  Node *m_cursor;  // Used to process cursor keys, stores current position.
  bool    outcomes_changed;
  TreeDrawSettings draw_settings;  // Stores drawing parameters

public:
  TreeWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~TreeWindow();
    
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

  EfgShow *Parent(void) const { return m_parent; }
  
  virtual void OnDraw(wxDC &dc);
  void HilightInfoset(int pl,int iset);

  void RefreshLayout(void);
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

  const TreeDrawSettings &DrawSettings(void) const { return draw_settings; }

  void UpdateMenus(void);
  
  // Hilight the subgame root for the currently active subgame
  void  SetSubgamePickNode(Node *n);
  
  // Check if a drag'n'drop object has been activated
  Node *GotObject(long &mx, long &my, int what);

  float GetZoom(void) const { return m_zoom; }
  void SetZoom(float p_zoom);
  void FitZoom(void);

  // Access to the numeric values from the renderer
  gText AsString(TypedSolnValues what, const Node *n, int br = 0) const;
  gText OutcomeAsString(const Node *n, bool &hilight) const;

  DECLARE_EVENT_TABLE()
};

#endif   // TREEWIN_H

