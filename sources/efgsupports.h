//
// FILE: efgsupports.h -- Extensive form supports display
//
// $Id$
//

#ifndef EFGSUPPORTS_H
#define EFGSUPPORTS_H

#include "wx/listctrl.h"

class guiEfgSupports : public wxListCtrl {
private:
  Efg &m_efg;
  guiEfgView *m_parent;
  gList<EFSupport *> m_supports;
  int m_currentSupport;

  void OnSelected(wxListEvent &);

public:
  guiEfgSupports(guiEfgView *m_parent, Efg &p_efg);
  virtual ~guiEfgSupports();

  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  int NumSupports(void) const { return m_supports.Length(); }
  EFSupport *CurrentSupport(void) const
    { return m_supports[m_currentSupport]; }
  int CurrentSupportIndex(void) const { return m_currentSupport; }
  gText UniqueSupportName(void) const;

  void AddSupport(EFSupport *);
  void DeleteSupport(int p_support);
  void SetCurrentSupport(int p_support);
  void EditCurrentSupport(const EFSupport &);

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSUPPORTS_H
