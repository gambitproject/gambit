//
// FILE: efgview.h -- Extensive form viewing class
//
// $Id$
//

#ifndef EFGVIEW_H
#define EFGVIEW_H

#include "wx/proplist.h"

class guiEfgFrame;
class guiEfgTree;
class guiEfgInfoPanel;
class guiEfgView;

class guiEfgOutcomeProperties : public wxPropertyListPanel {
private:
  guiEfgView *m_parent;
  EFOutcome *m_outcome;

public:
  guiEfgOutcomeProperties(guiEfgView *, EFOutcome *);
  virtual ~guiEfgOutcomeProperties() { }
};

class guiEfgNodeProperties : public wxPropertyListPanel {
private:
  guiEfgView *m_parent;
  Node *m_node;

public:
  guiEfgNodeProperties(guiEfgView *, Node *);
  virtual ~guiEfgNodeProperties() { }
};

class guiEfgView : public wxPanel {
private:
  guiEfgTree *m_tree;
  guiEfgInfoPanel *m_infoPanel;
  guiEfgOutcomeProperties *m_outcomeProps;
  guiEfgNodeProperties *m_nodeProps;
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

  double GetZoom(void) const;
  void SetZoom(double);

  void OnSize(wxSizeEvent &);
    
  DECLARE_EVENT_TABLE()
};

#endif // EFGVIEW_H
