//
// FILE: efgtree.h -- Declaration of tree display window
//
// $Id$
//

#ifndef EFGTREE_H
#define EFGTREE_H

class TreeObject;
class EfgDrawSettings;
class guiEfgView;

class guiEfgTree : public wxScrolledWindow {
private:
  Efg &m_efg;
  guiEfgView *m_parent;
  gBlock<TreeObject *> m_objects;
  TreeObject *m_selection;
  EfgDrawSettings *m_settings;
  int m_maxX, m_maxY, m_minY;
  int m_curX, m_curY, m_oldX, m_oldY, m_startX, m_startY;  // for dragging
  double m_zoomFactor;

  int LayoutSubtree(Node *, int, int &, int &, int &, int &);
  void LayoutTree(void);

  void OnDraw(void);
  void OnDraw(wxDC &);

  void OnLeftClick(wxMouseEvent &);
  void OnLeftUp(wxMouseEvent &);
  void OnDoubleLeftClick(wxMouseEvent &);
  void OnMouseMotion(wxMouseEvent &);

  TreeObject *HitTest(int p_x, int p_y) const;
  void ComputeScrollbars(void);

public:
  guiEfgTree(guiEfgView *p_parent, Efg &p_efg);
  virtual ~guiEfgTree();

  void OnSelectedOutcome(EFOutcome *, bool);
  void OnSelectedNode(Node *, bool);

  Node *SelectedNode(void) const;
  EFOutcome *SelectedOutcome(void) const;
  Action *SelectedAction(void) const;
  void OnTreeChanged(void);
  void OnOutcomeChanged(EFOutcome *);
  void OnNodeChanged(Node *);
  
  double GetZoom(void) const { return m_zoomFactor; }
  void SetZoom(double);

  DECLARE_EVENT_TABLE()
};

#endif // EFGTREE_H
