//
// FILE: nfsuptd.h -- Declarations of dialogs for selecting supports on
//                    the normal form
//
// $Id$
//

#ifndef NFSUPTD_H
#define NFSUPTD_H

class NFSupportInspectDialog : public wxDialogBox {
private:
  NfgShow *bns;
  gList<NFSupport *> &sups;
  int init_disp,init_cur;
  wxText *cur_dim,*disp_dim;
  wxChoice *disp_item,*cur_item;
  char *cur_str,*disp_str;
  wxStringList *support_list;

// Low level event handlers
  static void cur_func(wxChoice &ob,wxEvent &)
    {((NFSupportInspectDialog *)ob.GetClientData())->OnCur(ob.GetSelection()+1);}
  static void disp_func(wxChoice &ob,wxEvent &)
    {((NFSupportInspectDialog *)ob.GetClientData())->OnDisp(ob.GetSelection()+1);}
  static void new_sup_func(wxButton &ob,wxEvent &)
    {((NFSupportInspectDialog *)ob.GetClientData())->OnNewSupport();}
  static void change_sup_func(wxButton &ob,wxEvent &)
    {((NFSupportInspectDialog *)ob.GetClientData())->OnChangeSupport();}
  static void help_func(wxButton &,wxEvent &)
    {wxHelpContents(NFG_SUPPORTS_HELP);}
  static void close_func(wxButton &ob,wxEvent &)
    {((NfgShow *)ob.GetClientData())->ChangeSupport(DESTROY_DIALOG);}
  static void remove_sup_func(wxButton &ob,wxEvent &)
    {((NFSupportInspectDialog *)ob.GetClientData())->OnRemoveSupport();}

// High level event handlers
  void OnNewSupport(void);
  void OnRemoveSupport(void);
  void OnChangeSupport(void);
  void OnCur(int cur_sup);
  void OnDisp(int disp_sup);

// Utility funcs
  static gText array_to_string(const gArray<int> &);

public:
  NFSupportInspectDialog(gList<NFSupport *> &sups_, int cur_sup, int disp_sup,
			 NfgShow *bns_, wxWindow *parent = 0);

  Bool OnClose(void);

  // Data Access members
  int CurSup(void) { return (cur_item->GetSelection() + 1); }
  int DispSup(void) { return (disp_item->GetSelection() + 1); }
};

#endif  // NFSUPTD_H
