// File: efsuptd.h -- Declarations of dialogs for dealing with EF
// supports.
// $Id$
#ifndef EFSUPTD_H
#define EFSUPTD_H
#include "elimdomd.h"
class EFSupportInspectDialog:public wxDialogBox
{
private:
	EfgShow *es;
	gList<EFSupport *> &sups;
	int init_disp,init_cur;
	wxText *cur_dim,*disp_dim;
	wxChoice *disp_item,*cur_item;
	wxCheckBox *root_item;
	char *cur_str,*disp_str;
	wxStringList *support_list;
	Bool root_reachable;
// Low level event handlers
	static void cur_func(wxChoice &ob,wxEvent &)
	{((EFSupportInspectDialog *)ob.GetClientData())->OnCur(ob.GetSelection()+1);}
	static void disp_func(wxChoice &ob,wxEvent &)
	{((EFSupportInspectDialog *)ob.GetClientData())->OnDisp(ob.GetSelection()+1);}
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
	void OnNewSupport(void)
	{
	if (es->MakeSupport())
	{
		disp_item->Append(ToString(sups.Length()));
		disp_item->SetSize(-1,-1,-1,-1);
		cur_item->Append(ToString(sups.Length()));
		cur_item->SetSize(-1,-1,-1,-1);
	}
	}
	void OnRemoveSupport(void);
	void OnChangeSupport(void)
	{
	es->ChangeSupport(UPDATE_DIALOG);init_disp=DispSup();init_cur=CurSup();
	}
	void OnCur(int cur_sup)
	{
	cur_dim->SetValue(gpvect_to_string(sups[cur_sup]->NumActions()));
	disp_dim->SetValue(gpvect_to_string(sups[cur_sup]->NumActions()));
	disp_item->SetSelection(cur_sup-1);
	}
	void OnDisp(int disp_sup)
	{disp_dim->SetValue(gpvect_to_string(sups[disp_sup]->NumActions()));}
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
	EFSupportInspectDialog(gList<EFSupport *> &sups_,int cur_sup,
											int disp_sup,EfgShow *es_,wxWindow *parent=0)
		: wxDialogBox(parent,"Supports"),es(es_),sups(sups_)
	{
  init_disp=disp_sup;init_cur=cur_sup;
	wxForm *f=new wxForm(0);
	SetLabelPosition(wxVERTICAL);
	cur_dim=new wxText(this,0,"Current",
											gpvect_to_string(sups[cur_sup]->NumActions()),
											-1,-1,80,-1,wxREADONLY);
	disp_dim=new wxText(this,0,"Display",
											gpvect_to_string(sups[disp_sup]->NumActions()),
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
	wxFormItem *newsup_fitem=wxMakeFormButton("New",(wxFunction)new_sup_func);
	f->Add(newsup_fitem);
	wxFormItem *rmvsup_fitem=wxMakeFormButton("Remove",(wxFunction)remove_sup_func);
	f->Add(rmvsup_fitem);
	f->Add(wxMakeFormNewLine());
	wxFormItem *close_fitem=wxMakeFormButton("Close",(wxFunction)close_func);
	f->Add(close_fitem);
	wxFormItem *cngsup_fitem=wxMakeFormButton("Apply",(wxFunction)change_sup_func);
	f->Add(cngsup_fitem);
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
	((wxButton *)rmvsup_fitem->GetPanelItem())->SetClientData((char *)this);
	((wxButton *)cngsup_fitem->GetPanelItem())->SetClientData((char *)this);
	((wxButton *)help_fitem->GetPanelItem())->SetClientData((char *)this);
	((wxButton *)close_fitem->GetPanelItem())->SetClientData((char *)es);
	root_item=(wxCheckBox *)root_fitem->GetPanelItem();
	Fit();
	Show(TRUE);
	}
	// Data Access members
	int 	CurSup(void) {return cur_item->GetSelection()+1;}
	int 	DispSup(void) {return disp_item->GetSelection()+1;}
	Bool	RootReachable(void) {return root_item->GetValue();}
};


// Note can not delete the first support
void EFSupportInspectDialog::OnRemoveSupport(void)
{
SupportRemoveDialog SRD(this,sups.Length());
if (SRD.Completed()==wxOK)
{
gArray<bool> selected(SRD.Selected());
bool revert=false;
int i;
for (i=sups.Length();i>=2;i--)
	if (selected[i])
	{
		delete sups.Remove(i);
		if (i==init_cur || i==init_disp && revert==false)
		{
			wxMessageBox("Display/Current support deleted.\nReverting to full support");
			revert=true;
		}
	}
disp_item->Clear();cur_item->Clear();
for (i=1;i<=sups.Length();i++)
	{disp_item->Append(ToString(i));cur_item->Append(ToString(i));}
disp_item->SetSize(-1,-1,-1,-1);cur_item->SetSize(-1,-1,-1,-1);
disp_item->SetSelection(0);cur_item->SetSelection(0);
if (revert) es->ChangeSupport(UPDATE_DIALOG);
}
}
#endif
