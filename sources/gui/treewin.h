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
  efgGame &m_efg;
  EfgShow *m_parent;
  efgTreeLayout m_layout;
  TreeDrawSettings m_drawSettings;

  double m_zoom;
  wxMenu *m_nodeMenu, *m_gameMenu;

  wxDragImage *m_dragImage;
  Node *m_dragSource;
  enum { dragCOPY, dragMOVE, dragOUTCOME } m_dragMode;

  // Private Functions
  void MakeFlasher(void);
  void UpdateCursor(void);
  void ProcessCursor(void);
  void EnsureCursorVisible(void);

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
    
  EfgShow *Parent(void) const { return m_parent; }
  
  virtual void OnDraw(wxDC &dc);

  void RefreshTree(void);
  void RefreshLayout(void);
  void RefreshLabels(void);
  
  void SupportChanged(void);
  
  TreeDrawSettings &DrawSettings(void) { return m_drawSettings; }
  const TreeDrawSettings &DrawSettings(void) const { return m_drawSettings; }
  
  void SetCursorPosition(Node *p_cursor);
    
  void UpdateMenus(void);
  
  double GetZoom(void) const { return m_zoom; }
  void SetZoom(double p_zoom);
  void FitZoom(void);

  // Access to the numeric values from the renderer
  gText OutcomeAsString(const Node *n) const;

  DECLARE_EVENT_TABLE()
};

#endif   // TREEWIN_H

