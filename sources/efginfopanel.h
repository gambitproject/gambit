//
// FILE: efginfopanel.h -- Extensive form information panel window
//
// $Id$
//

#ifndef EFGINFOPANEL_H
#define EFGINFOPANEL_H

class guiEfgView;

class guiEfgInfoPanel : public wxPanel {
private:
  Efg &m_efg;
  guiEfgView *m_parent;

public:
  guiEfgInfoPanel(guiEfgView *p_parent, wxWindow *p_window, Efg &p_efg);
  virtual ~guiEfgInfoPanel();
};

#endif  // EFGINFOPANEL_H
