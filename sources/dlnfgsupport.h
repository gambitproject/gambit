//
// FILE: dlnfgsupport.h -- Declarations of dialogs for selecting supports on
//                         the normal form
//
// $Id$
//

#ifndef DLNFGSUPPORT_H
#define DLNFGSUPPORT_H

#include "nfgconst.h"
#include "nfgshow.h"

class dialogNfgSupportInspect : public wxDialogBox {
private:
  NfgShow *bns;
  gList<NFSupport *> &sups;
  int init_cur;
  wxText *cur_dim;
  wxChoice *cur_item;
  char *cur_str;
  wxStringList *support_list;

  static void CallbackCurrent(wxChoice &ob, wxEvent &)
    {((dialogNfgSupportInspect *)ob.GetClientData())->OnCur(ob.GetSelection()+1);}
  static void CallbackNew(wxButton &ob,wxEvent &)
    {((dialogNfgSupportInspect *)ob.GetClientData())->OnNewSupport();}
  static void CallbackChange(wxButton &ob,wxEvent &)
    {((dialogNfgSupportInspect *)ob.GetClientData())->OnChangeSupport();}
  static void CallbackHelp(wxButton &,wxEvent &)
    {wxHelpContents(NFG_SUPPORTS_HELP);}
  static void CallbackClose(wxButton &ob,wxEvent &)
    {((NfgShow *)ob.GetClientData())->ChangeSupport(DESTROY_DIALOG);}
  static void CallbackRemove(wxButton &ob,wxEvent &)
    {((dialogNfgSupportInspect *)ob.GetClientData())->OnRemoveSupport();}

// High level event handlers
  void OnNewSupport(void);
  void OnRemoveSupport(void);
  void OnChangeSupport(void);
  void OnCur(int cur_sup);
  void OnDisp(int disp_sup);

// Utility funcs
  static gText array_to_string(const gArray<int> &);

public:
  dialogNfgSupportInspect(gList<NFSupport *> &p_supports,
			  int p_current,
			  NfgShow *p_window, wxWindow *p_parent = 0);

  Bool OnClose(void);

  // Data Access members
  int Current(void) { return (cur_item->GetSelection() + 1); }
};

#endif  // DLNFGSUPPORT_H

