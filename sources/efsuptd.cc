//
// FILE: efsuptd.cc -- Implementation of dialogs for selecting supports
//                     on the extensive form
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "garray.h"

#include "efgshow.h"
#include "efsuptd.h"
#include "dlelim.h"

void EFSupportInspectDialog::OnNewSupport(void)
{
  if (es->MakeSupport()) {
    cur_item->Append(ToText(sups.Length()));
    cur_item->SetSize(-1,-1,-1,-1);
  }
}

// Note can not delete the first support
void EFSupportInspectDialog::OnRemoveSupport(void)
{
  SupportRemoveDialog SRD(this,sups.Length());
  if (SRD.Completed()==wxOK) {
    gArray<bool> selected(SRD.Selected());
    bool revert=false;
    int i;
    for (i=sups.Length();i>=2;i--)
      if (selected[i]) {
	delete sups.Remove(i);
	if (i==init_cur && revert==false) {
	  wxMessageBox("Display/Current support deleted.\nReverting to full support");
	  revert=true;
	}
      }
    cur_item->Clear();
    for (i=1;i<=sups.Length();i++) {
      cur_item->Append(ToText(i));
    }
    cur_item->SetSize(-1,-1,-1,-1);
    cur_item->SetSelection(0);
    if (revert) 
      es->ChangeSupport(UPDATE_DIALOG);
  }
}

void EFSupportInspectDialog::OnChangeSupport(void)
{
  es->ChangeSupport(UPDATE_DIALOG);
  init_cur=CurSup();
}

void EFSupportInspectDialog::OnCur(int cur_sup)
{
  cur_dim->SetValue(gpvect_to_string(sups[cur_sup]->NumActions()));
}

gText EFSupportInspectDialog::gpvect_to_string(const gPVector<int> &a)
{
  int sum;
  gText tmp='(';
  for (int i=1;i<=a.Lengths().Length();i++) {
    sum=0;
    for (int j=1;j<=a.Lengths()[i];j++) sum+=a(i,j);
    tmp+=ToText(sum)+((i==a.Lengths().Length()) ? ")" : ",");
  }
  return tmp;
}

EFSupportInspectDialog::EFSupportInspectDialog(gList<EFSupport *> &sups_,
					       int cur_sup,
					       EfgShow *es_,
					       wxWindow *parent /*=0*/)
  : wxDialogBox(parent,"Select Support"),es(es_),sups(sups_)
{
  init_cur=cur_sup;
  wxForm *f=new wxForm(0);
  SetLabelPosition(wxVERTICAL);
  cur_dim=new wxText(this,0,"Current",
		     gpvect_to_string(sups[cur_sup]->NumActions()),
		     -1,-1,80,-1,wxREADONLY);
  support_list=wxStringListInts(sups.Length());
  cur_str=new char[10];strcpy(cur_str,ToText(cur_sup));
  wxFormItem *cur_fitem=wxMakeFormString("",&cur_str,wxFORM_CHOICE,
					 new wxList(wxMakeConstraintStrings(support_list),0));
  f->Add(cur_fitem);
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
  ((wxButton *)newsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)rmvsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)cngsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)help_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)close_fitem->GetPanelItem())->SetClientData((char *)es);
  root_item=(wxCheckBox *)root_fitem->GetPanelItem();
  Fit();
  Show(TRUE);
}

Bool EFSupportInspectDialog::OnClose(void)
{
  es->ChangeSupport(DESTROY_DIALOG);
  return FALSE;
}




