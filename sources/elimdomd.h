// File: elimdomd.h -- declarations for classes dealing with inspection and
// creation of dominated strategy supports
// $Id$
#ifndef ELIMDOMD_H
#define ELIMDOMD_H
#define SUPPORT_OPEN		0
#define SUPPORT_CLOSE		1
#define SUPPORT_CHANGE	2

#define DOM_WEAK				0
#define	DOM_STRONG			1

#define	DOM_PURE				0
#define DOM_MIXED				1

#define	SOLN_SECT			"Soln-Defaults"
#define ELIMDOM_HELP		"Elimination of Dominated Strategies"

class DominanceSettings
{
protected:
	Bool use_elimdom,all;
	int	dom_type,dom_method;
	char *defaults_file;
public:
	DominanceSettings(void)
	{
	defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Nfg-ElimDom-All",&all,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-ElimDom-Type",&dom_type,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-ElimDom-Method",&dom_method,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-ElimDom-Use",&use_elimdom,defaults_file);
	}
	~DominanceSettings()
	{
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-All",all,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Type",dom_type,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Method",dom_method,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Use",use_elimdom,defaults_file);
	}
	bool UseElimDom(void) const {return use_elimdom;}
	bool FindAll(void) const {return all;}
	bool DomStrong(void) const {return dom_type==DOM_STRONG;}
	bool DomMixed(void) const {return dom_method==DOM_MIXED;}
};




class DominanceSettingsDialog: public MyDialogBox,public DominanceSettings
{
private:
char *dom_type_str,*dom_method_str;
wxStringList *dom_type_list,*dom_method_list;
public:
	DominanceSettingsDialog(wxWindow *parent):MyDialogBox(parent,"Dominance Defaults")
	{
	Add(wxMakeFormBool("ElimDom before solve",&use_elimdom));
	Add(wxMakeFormNewLine());
	Add(wxMakeFormBool("Iterative Eliminate",&all));
	Add(wxMakeFormNewLine());
	dom_type_list=new wxStringList("Weak","Strong",0);
	dom_type_str=new char[20];
	strcpy(dom_type_str,(char *)dom_type_list->Nth(dom_type)->Data());
	Add(wxMakeFormString("Dom Type",&dom_type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(dom_type_list), 0),0,wxVERTICAL));
	Add(wxMakeFormNewLine());
	dom_method_list=new wxStringList("Pure","Mixed",0);
	dom_method_str=new char[20];
	strcpy(dom_method_str,(char *)dom_method_list->Nth(dom_method)->Data());
	Add(wxMakeFormString("Dom Type",&dom_method_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(dom_method_list), 0),0,wxVERTICAL));
	Go();
	}
	~DominanceSettingsDialog()
	{
	dom_type=wxListFindString(dom_type_list,dom_type_str);
	dom_method=wxListFindString(dom_method_list,dom_method_str);
	}
};

class ElimDomParamsDialog:public DominanceSettings // Can not use MyDialogBox due to wxMULTIPLE
{
private:
	wxDialogBox *d;
	wxListBox 	*player_box;
	wxCheckBox	*all_box,*compress_box;
	wxRadioBox	*dom_type_box,*dom_method_box;
	bool 				compress;
	int					completed,num_players;
	gArray<int>	players;
	// now come the private functions
	void OnEvent(int result)
	{
	completed=result;
	if (completed==wxOK)
	{
		all=all_box->GetValue();compress=compress_box->GetValue();
		dom_type=dom_type_box->GetSelection();
		dom_method=dom_method_box->GetSelection();
		int num_selections,*selections=new int[num_players];
		num_selections=player_box->GetSelections(&selections);
		players=gArray<int>(num_selections);
		for (int i=1;i<=num_selections;i++) players[i]=selections[i-1]+1;
	}
	d->Show(FALSE);delete d;
	}
	static void ok_button_func(wxButton &ob,wxEvent &)
	{((ElimDomParamsDialog *)ob.GetClientData())->OnEvent(wxOK);}
	static void cancel_button_func(wxButton &ob,wxEvent &)
	{((ElimDomParamsDialog *)ob.GetClientData())->OnEvent(wxCANCEL);}
	static void help_button_func(wxButton &,wxEvent &)
	{wxHelpContents(ELIMDOM_HELP);}

public:
	ElimDomParamsDialog(int numplayers,wxWindow *parent=NULL)
	{
	num_players=numplayers;
	compress=FALSE;
	d=new wxDialogBox(parent,"ElimDom Parameters",TRUE);
	all_box=new wxCheckBox(d,0,"Iterative Eliminate");
	all_box->SetValue(all);
	compress_box=new wxCheckBox(d,0,"Compress");
	d->NewLine();
	char *dom_type_list[2]={"Weak","Strong"};
	dom_type_box=new wxRadioBox(d,NULL,"Dom type",-1,-1,-1,-1,2,dom_type_list,1);
	dom_type_box->SetSelection(dom_type);
	char *dom_method_list[2]={"Pure Strat","Mixed Strat"};
	dom_method_box=new wxRadioBox(d,NULL,"Dom method",-1,-1,-1,-1,2,dom_method_list,1);
	dom_method_box->SetSelection(dom_method);
	d->NewLine();
	wxStringList *player_list=wxStringListInts(num_players);
	player_box=new wxListBox(d,NULL,"Players",wxMULTIPLE,-1,-1,-1,-1,num_players,
		player_list->ListToArray());
	for (int i=1;i<=num_players;i++) player_box->SetSelection(i-1,TRUE);
	d->NewLine();

	wxButton *ok_button=new wxButton(d,(wxFunction)ok_button_func,"Ok");
	ok_button->SetClientData((char *)this);
	wxButton *cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
	cancel_button->SetClientData((char *)this);
	new wxButton(d,(wxFunction)help_button_func,"Help");
	d->Fit();
	d->Show(TRUE);
	}
	// Data access functions
	gArray<int> Players(void) const {return players;}
	bool Compress(void) const {return compress;}
	int  Completed(void) const {return completed;}
};

class SupportRemoveDialog: public wxDialogBox
{
private:
	int num_sups;
	wxListBox *sups_item;
	int completed;
	static void all_func(wxCheckBox &ob,wxEvent &)
	{
	SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
	for (int i=1;i<=parent->num_sups;i++)
		parent->sups_item->SetSelection(i-1,ob.GetValue());
	}
	static void ok_func(wxButton &ob,wxEvent &)
	{
	SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
	parent->Show(FALSE);parent->completed=wxOK;
	}
	static void cancel_func(wxButton &ob,wxEvent &)
	{
	SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
	parent->Show(FALSE);parent->completed=wxCANCEL;
	}
public:
	SupportRemoveDialog(wxWindow *parent,int num_sups_):
		wxDialogBox(parent,"Remove Support(s)",TRUE),num_sups(num_sups_)
	{
	wxStringList *support_list=wxStringListInts(num_sups);
	sups_item=new wxListBox(this,0,"Supports",wxMULTIPLE,-1,-1,-1,-1,num_sups,support_list->ListToArray(),wxLB_MULTIPLE	);
	wxCheckBox *all_item=new wxCheckBox(this,(wxFunction)all_func,"All");
	all_item->SetClientData((char *)this);
	NewLine();
	wxButton *ok_button=new wxButton(this,(wxFunction)ok_func,"Ok");
	wxButton *cancel_button=new wxButton(this,(wxFunction)cancel_func,"Cancel");
	ok_button->SetClientData((char *)this);
	cancel_button->SetClientData((char *)this);
	Fit();
	Show(TRUE);
	}
	gArray<bool> Selected(void)
	{
	gArray<bool> selected(num_sups);
	for (int i=1;i<=num_sups;i++)
		selected[i]=(sups_item->Selected(i-1)) ? true : false;
	return selected;
	}
	int Completed(void)
  {return completed;}
};

#endif

