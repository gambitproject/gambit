//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of window class to display extensive form tree
//

#ifndef TREEWIN_H
#define TREEWIN_H

#include "wx/dragimag.h"

#include "efglayout.h"
#include "guishare/wxmisc.h"

class EfgShow;

class TreeWindow : public wxScrolledWindow {
friend class EfgPrintout;
private:
  FullEfg &m_efg;
  EfgShow *m_parent;
  efgTreeLayout m_layout;
  TreeDrawSettings m_drawSettings;

  double m_zoom;
  wxMenu *m_nodeMenu, *m_gameMenu;
  Node *m_markNode;
  Node *m_cursor; 

  wxDragImage *m_dragImage;
  Node *m_dragSource;
  enum { dragCOPY, dragMOVE, dragOUTCOME } m_dragMode;
  class IsetDragger *m_infosetDragger;
  class BranchDragger *m_branchDragger;

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

  EfgShow *Parent(void) const { return m_parent; }
  
  virtual void OnDraw(wxDC &dc);

  void RefreshTree(void);
  void RefreshLayout(void);
  void RefreshLabels(void);
  
  // Used by parent EfgShow when cur_sup changes
  void SupportChanged(void);
  
  // Gives access to the parent to the private draw_settings. Used for SolnShow
  TreeDrawSettings &DrawSettings(void) { return m_drawSettings; }
  int NumDecimals(void) const { return m_drawSettings.NumDecimals(); }
  
  // Gives access to the parent to the current cursor node
  Node *Cursor(void) const { return m_cursor; }
  void SetCursorPosition(Node *p_cursor);
  Node *MarkNode(void) const { return m_markNode; }
    
  const TreeDrawSettings &DrawSettings(void) const { return m_drawSettings; }

  void UpdateMenus(void);
  
  // Check if a drag'n'drop object has been activated
  Node *GotObject(long &mx, long &my, int what);

  float GetZoom(void) const { return m_zoom; }
  void SetZoom(float p_zoom);
  void FitZoom(void);

  // Access to the numeric values from the renderer
  gText OutcomeAsString(const Node *n) const;

  DECLARE_EVENT_TABLE()
};

#endif   // TREEWIN_H

