//
// FILE: efsuptd.h -- Declarations of dialogs for selecting supports on
//                    the extensive form
//
// $Id$
//

#ifndef EFSUPTD_H
#define EFSUPTD_H

class EFSupportInspectDialog : public wxDialogBox {
private:
  EfgShow *es;
  gList<EFSupport *> &sups;
  int init_cur;
  wxText *cur_dim;
  wxChoice *cur_item;
  wxCheckBox *root_item;
  char *cur_str;
  wxStringList *support_list;
  Bool root_reachable;

  // Low level event handlers
  static void cur_func(wxChoice &ob,wxEvent &)
    {((EFSupportInspectDialog *)ob.GetClientData())->OnCur(ob.GetSelection()+1);}
  static void new_sup_func(wxButton &ob,wxEvent &)
    {((EFSupportInspectDialog *)ob.GetClientData())->OnNewSupport();}
  static void change_sup_func(wxButton &ob,wxEvent &)
    {((EFSupportInspectDialog *)ob.GetClientData())->OnChangeSupport();}
  static void help_func(wxButton &,wxEvent &)
    {wxHelpContents(EFG_SUPPORTS_HELP);}
  static void close_func(wxButton &ob,wxEvent &)
    {((EfgShow *)ob.GetClientData())->ChangeSupport(DESTROY_DIALOG);}
  static void remove_sup_func(wxButton &ob,wxEvent &)
    {((EFSupportInspectDialog *)ob.GetClientData())->OnRemoveSupport();}

  // High level event handlers
  void OnNewSupport(void);
  void OnRemoveSupport(void);
  void OnChangeSupport(void);
  void OnCur(int cur_sup);

// Utility funcs
  static gText gpvect_to_string(const gPVector<int> &);

public:
  EFSupportInspectDialog(gList<EFSupport *> &sups_, int cur_sup,
			 EfgShow *es_, wxWindow *parent = 0);

  Bool OnClose(void);

  // Data Access members
  int CurSup(void) { return (cur_item->GetSelection() + 1); }
  Bool RootReachable(void) { return root_item->GetValue(); }
};

#endif  // EFSUPTD_H
