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

class guiEfgView {
private:
  guiEfgTree *m_tree;
  guiEfgInfoPanel *m_infoPanel;
  guiEfgFrame *m_parent;

  Efg *m_efg;
  Node *m_copyNode;
  EFOutcome *m_copyOutcome;

public:
  guiEfgView(guiEfgFrame *p_parent, Efg *p_efg,
	     wxSplitterWindow *p_solutionSplitter,
	     wxSplitterWindow *p_infoSplitter);

  Efg *GetEfg(void) const { return m_efg; }
  guiEfgTree *TreeWindow(void) const { return m_tree; }
  guiEfgInfoPanel *InfoPanel(void) const { return m_infoPanel; }

  void ShowWindows(bool);

  void OnCopy(void);
  void OnPaste(void);
  void OnDelete(void);
  void OnProperties(void);
  
  double GetZoom(void) const;
  void SetZoom(double);
};

#endif // EFGVIEW_H
