// File: efsuptd.h -- Declarations of dialogs for dealing with EF
// supports.
// $Id$

class EFSupportInspectDialog:public wxDialogBox
{
private:
	BaseExtensiveShow *bes;
	const gList<EFSupport *> &sups;
	wxText *cur_dim,*disp_dim;
	wxChoice *disp_item,*cur_item;
	wxCheckBox *root_item;
	char *cur_str,*disp_str;
	wxStringList *support_list;
	Bool root_reachable;
// Low level event handlers
	static void cur_func(wxChoice &ob,wxEvent &ev)
	{((EFSupportInspectDialog *)ob.GetClientData())->OnCur(ob.GetSelection()+1);}
	static void disp_func(wxChoice &ob,wxEvent &ev)
	{((EFSupportInspectDialog *)ob.GetClientData())->OnDisp(ob.GetSelection()+1);}
	static void new_sup_func(wxButton &ob,wxEvent &ev)
	{((EFSupportInspectDialog *)ob.GetClientData())->OnNewSupport();}
	static void change_sup_func(wxButton &ob,wxEvent &ev)
	{((BaseExtensiveShow *)ob.GetClientData())->SupportInspect(SUPPORT_CHANGE);}
	static void help_func(wxButton &ob,wxEvent &ev)
	{wxHelpContents(EFG_SUPPORTS_HELP);}
	static void close_func(wxButton &ob,wxEvent &ev)
	{((BaseExtensiveShow *)ob.GetClientData())->SupportInspect(SUPPORT_CLOSE);}
// High level event handlers
	void OnNewSupport(void)
	{
	if (bes->MakeSupport())
	{
		disp_item->Append(ToString(sups.Length()));
		disp_item->SetSize(-1,-1,-1,-1);
		cur_item->Append(ToString(sups.Length()));
		cur_item->SetSize(-1,-1,-1,-1);
	}
	}
	void OnCur(int cur_sup)
	{
	cur_dim->SetValue(gpvect_to_string(sups[cur_sup]->Dimensionality(false)));
	disp_dim->SetValue(gpvect_to_string(sups[cur_sup]->Dimensionality(false)));
	disp_item->SetSelection(cur_sup-1);
	}
	void OnDisp(int disp_sup)
	{disp_dim->SetValue(gpvect_to_string(sups[disp_sup]->Dimensionality(false)));}

// Utility funcs
	static gString gpvect_to_string(const gPVector<int> &a)
	{
	int sum;
	gString tmp='(';
	for (int i=1;i<=a.Lengths().Length();i++)
	{
	  sum=0;
		for (int j=1;j<=a.Lengths()[i];j++) sum+=a(i,j);
		tmp+=ToString(sum)+((i==a.Lengths().Length()) ? ")" : ",");
	}
	return tmp;
	}
public:
	EFSupportInspectDialog(const gList<EFSupport *> &sups_,int cur_sup,
											int disp_sup,BaseExtensiveShow *bes_,wxWindow *parent=0)
		: wxDialogBox(parent,"Supports"),sups(sups_),bes(bes_)
	{
	wxForm *f=new wxForm(0);
	SetLabelPosition(wxVERTICAL);
	cur_dim=new wxText(this,0,"Current",
											gpvect_to_string(sups[cur_sup]->Dimensionality(false)),
											-1,-1,80,-1,wxREADONLY);
	disp_dim=new wxText(this,0,"Display",
											gpvect_to_string(sups[disp_sup]->Dimensionality(false)),
											-1,-1,80,-1,wxREADONLY);
	support_list=wxStringListInts(sups.Length());
	cur_str=new char[10];strcpy(cur_str,ToString(cur_sup));
	disp_str=new char[10];strcpy(disp_str,ToString(disp_sup));
	wxFormItem *cur_fitem=wxMakeFormString("",&cur_str,wxFORM_CHOICE,
					new wxList(wxMakeConstraintStrings(support_list),0));
	f->Add(cur_fitem);
	f->Add(wxMakeFormMessage("      ")); // fix the spacing... not neat but..
	wxFormItem *disp_fitem=wxMakeFormString("",&disp_str,wxFORM_CHOICE,
					new wxList(wxMakeConstraintStrings(support_list),0));
	f->Add(disp_fitem);
	f->Add(wxMakeFormNewLine());
	wxFormItem *root_fitem=wxMakeFormBool("Root reachable only",&root_reachable);
	f->Add(root_fitem);
	f->Add(wxMakeFormNewLine());
	wxFormItem *close_fitem=wxMakeFormButton("Close",(wxFunction)close_func);
	f->Add(close_fitem);
	wxFormItem *cngsup_fitem=wxMakeFormButton("Apply",(wxFunction)change_sup_func);
	f->Add(cngsup_fitem);
	wxFormItem *newsup_fitem=wxMakeFormButton("New",(wxFunction)new_sup_func);
	f->Add(newsup_fitem);
	wxFormItem *help_fitem=wxMakeFormButton("?",(wxFunction)help_func);
	f->Add(help_fitem);
	f->AssociatePanel(this);
	cur_item=(wxChoice *)cur_fitem->GetPanelItem();
	cur_item->Callback((wxFunction)cur_func);
	cur_item->SetClientData((char *)this);
	disp_item=(wxChoice *)disp_fitem->GetPanelItem();
	disp_item->SetClientData((char *)this);
	disp_item->Callback((wxFunction)disp_func);
	((wxButton *)newsup_fitem->GetPanelItem())->SetClientData((char *)this);
	((wxButton *)cngsup_fitem->GetPanelItem())->SetClientData((char *)bes);
	((wxButton *)help_fitem->GetPanelItem())->SetClientData((char *)this);
	((wxButton *)close_fitem->GetPanelItem())->SetClientData((char *)bes);
	root_item=(wxCheckBox *)root_fitem->GetPanelItem();
	Fit();
	Show(TRUE);
	}
	// Data Access members
	int 	CurSup(void) {return cur_item->GetSelection()+1;}
	int 	DispSup(void) {return disp_item->GetSelection()+1;}
	Bool	RootReachable(void) {return root_item->GetValue();}
};


