//
// FILE: efgview.h -- Extensive form viewing class
//
// $Id$
//

#ifndef EFGVIEW_H
#define EFGVIEW_H

#include "wx/grid.h"

class guiEfgFrame;
class guiEfgTree;
class guiEfgInfoPanel;
class guiEfgSupports;
class guiEfgView;

class guiEfgPropertiesView : public wxGrid {
private:
  const Efg &m_efg;
  EFOutcome *m_outcome;
  Node *m_node;

  void OnChangeLabels(void);
  void SetPropertyCount(int);

public:
  guiEfgPropertiesView(guiEfgView *, const Efg &);
  virtual ~guiEfgPropertiesView() { }

  void SetOutcome(EFOutcome *);
  void SetNode(Node *);
};

class guiEfgView : public wxPanel {
private:
  guiEfgTree *m_tree;
  guiEfgInfoPanel *m_infoPanel;
  guiEfgPropertiesView *m_propertyView;
  guiEfgSupports *m_supportsView;
  guiEfgFrame *m_parent;

  FullEfg *m_efg;
  
  Node *m_copyNode;
  EFOutcome *m_copyOutcome;

  void Arrange(void);

public:
  guiEfgView(guiEfgFrame *p_parent, FullEfg *p_efg, wxWindow *);

  FullEfg *GetEfg(void) const { return m_efg; }
  guiEfgTree *TreeWindow(void) const { return m_tree; }
  guiEfgInfoPanel *InfoPanel(void) const { return m_infoPanel; }

  void ShowWindows(bool);

  void OnCopy(void);
  void OnPaste(void);
  void OnDelete(void);
  void OnProperties(void);
  
  void OnSelectedOutcome(EFOutcome *, bool);
  void OnOutcomeChanged(EFOutcome *);

  void OnSelectedNode(Node *, bool);
  void OnNodeChanged(Node *);

  const gList<EFSupport *> &Supports(void) const;
  int NumSupports(void) const;
  EFSupport *CurrentSupport(void) const;
  int CurrentSupportIndex(void) const;
  gText UniqueSupportName(void) const;

  void AddSupport(EFSupport *);
  void DeleteSupport(int p_support);
  void SetCurrentSupport(int p_support, bool = true);
  void EditCurrentSupport(const EFSupport &);

  double GetZoom(void) const;
  void SetZoom(double);

  void OnSize(wxSizeEvent &);
    
  DECLARE_EVENT_TABLE()
};

#endif // EFGVIEW_H
