//
// FILE: elimdomd.h -- declarations for classes dealing with inspection and
// creation of dominated strategy supports
//
// $Id$
//

#ifndef ELIMDOMD_H
#define ELIMDOMD_H

#define DOM_WEAK				0
#define	DOM_STRONG			1

#define	DOM_PURE				0
#define DOM_MIXED				1

#define	SOLN_SECT			"Soln-Defaults"
#define ELIMDOM_HELP		"Elimination of Dominated Strategies"

class DominanceSettings {
protected:
  Bool use_elimdom,all;
  int	dom_type,dom_method;
  char *defaults_file;

public:
  DominanceSettings(void);
  ~DominanceSettings();

  bool UseElimDom(void) const { return use_elimdom; }
  bool FindAll(void) const { return all; }
  bool DomStrong(void) const { return (dom_type == DOM_STRONG); }
  bool DomMixed(void) const { return (dom_method == DOM_MIXED); }
};


class DominanceSettingsDialog : public MyDialogBox, public DominanceSettings {
private:
  char *dom_type_str, *dom_method_str;
  wxStringList *dom_type_list, *dom_method_list;

public:
  DominanceSettingsDialog(wxWindow *parent);
  ~DominanceSettingsDialog();
};


class ElimDomParamsDialog : public DominanceSettings {
  // Can not use MyDialogBox due to wxMULTIPLE
private:
  wxDialogBox *d;
  wxListBox 	*player_box;
  wxCheckBox	*all_box,*compress_box;
  wxRadioBox	*dom_type_box,*dom_method_box;
  bool 				compress;
  int					completed,num_players;
  gArray<int>	players;
	
  // now come the private functions
  void OnEvent(int result);
  static void ok_button_func(wxButton &ob,wxEvent &)
    {((ElimDomParamsDialog *)ob.GetClientData())->OnEvent(wxOK);}
  static void cancel_button_func(wxButton &ob,wxEvent &)
    {((ElimDomParamsDialog *)ob.GetClientData())->OnEvent(wxCANCEL);}
  static void help_button_func(wxButton &,wxEvent &)
    {wxHelpContents(ELIMDOM_HELP);}
  
public:
  ElimDomParamsDialog(int numplayers,wxWindow *parent=NULL);

  // Data access functions
  const gArray<int> &Players(void) const { return players; }
  bool Compress(void) const { return compress; }
  int  Completed(void) const { return completed; }
};


class SupportRemoveDialog: public wxDialogBox {
private:
  int num_sups;
  wxListBox *sups_item;
  int completed;
	
  static void all_func(wxCheckBox &ob,wxEvent &);
  static void ok_func(wxButton &ob,wxEvent &);
  static void cancel_func(wxButton &ob,wxEvent &);

public:
  SupportRemoveDialog(wxWindow *parent,int num_sups_);

  gArray<bool> Selected(void);
  int Completed(void) { return completed; }
};

#endif   // ELIMDOM_D

