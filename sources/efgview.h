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
  guiEfgInfoPanel *m_efgInfoPanel;
  guiEfgFrame *m_parent;

  Efg *m_efg;

public:
  guiEfgView(guiEfgFrame *p_parent, Efg *p_efg,
	     wxSplitterWindow *p_solutionSplitter,
	     wxSplitterWindow *p_infoSplitter);

  Efg *GetEfg(void) const { return m_efg; }
  guiEfgTree *TreeWindow(void) const { return m_tree; }

  void OnDelete(void);
  void OnProperties(void);
};

#endif // EFGVIEW_H
