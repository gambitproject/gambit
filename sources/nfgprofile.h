//
// FILE: nfgprofile.h -- Declaration of normal form profile list
//
// $Id$
//

#ifndef NFGPROFILE_H
#define NFGPROFILE_H

#include "wx/listctrl.h"
#include "nfgshow.h"
#include "mixedfilter.h"

class NfgProfileList : public wxListCtrl, public gList<MixedSolution> {
private:
  NfgShow *m_parent;
  wxMenu *m_menu;

protected:
  MixedListFilter m_options;

  // Event handlers
  void OnSortFilter(wxCommandEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnColumnClick(wxListEvent &);

public:
  NfgProfileList(NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgProfileList();

  void UpdateValues(void);

  // Overriding gSortList<T> member functions
  virtual int Append(const MixedSolution &);  

  DECLARE_EVENT_TABLE()
};

#endif  // NFGPROFILE_H
