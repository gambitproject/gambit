//
// FILE: efgview.h -- Extensive form viewing class
//
// $Id$
//

#ifndef EFGVIEW_H
#define EFGVIEW_H

class guiEfgFrame;
class guiEfgTree;
class guiEfgInfoPanel;

class guiEfgView : public wxPanel {
private:
  guiEfgTree *m_tree;
  guiEfgInfoPanel *m_infoPanel;
  guiEfgFrame *m_parent;

  FullEfg *m_efg;
  Node *m_copyNode;
  EFOutcome *m_copyOutcome;

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
  
  double GetZoom(void) const;
  void SetZoom(double);

  void OnSize(wxSizeEvent &);
    
  DECLARE_EVENT_TABLE()
};

#endif // EFGVIEW_H
