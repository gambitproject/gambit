//
// FILE: dlsupport.cc -- Dialogs for manipulation of supports
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "garray.h"
#include "dlelim.h"
#include "dlnfgsupport.h"

const char *SOLN_SECT = "Soln-Defaults";

//=========================================================================
//                  class dialogElim: Member functions
//=========================================================================

dialogElim::dialogElim(const gArray<gText> &p_players, bool p_mixed,
		       wxWindow *p_parent /* = NULL */)
  : wxDialogBox(p_parent, "Dominance Elimination Parameters", TRUE),
    m_mixed(p_mixed), m_compress(false), m_numPlayers(p_players.Length())
{
  wxGetResource(SOLN_SECT, "ElimDom-All", &m_all, gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Type", &m_domType, gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Method", &m_domMethod, gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Precision", &m_domPrecision,
		gambitApp.ResourceFile());

  m_allBox = new wxCheckBox(this, 0, "Eliminate iteratively");
  m_allBox->SetValue(m_all);
  m_compressBox = new wxCheckBox(this, 0, "Compress");
  NewLine();

  char *domTypeList[2] = {"Weak", "Strong"};
  m_domTypeBox = new wxRadioBox(this, NULL, "Type", -1, -1, -1, -1, 2,
				domTypeList, 1);
  if (m_domType == 0 || m_domType == 1)
    m_domTypeBox->SetSelection(m_domType);

  if (m_mixed) {
    char *domMethodList[2] = {"Pure", "Mixed"};
    m_domMethodBox = new wxRadioBox(this, NULL, "Method", -1, -1, -1, -1, 2,
				    domMethodList, 1);
    if (m_domMethod == 0 || m_domMethod == 1)
      m_domMethodBox->SetSelection(m_domMethod);

    char *domPrecisionList[2] = {"Float", "Rational" };
    m_domPrecisionBox = new wxRadioBox(this, NULL, "Precision", -1, -1, -1, -1, 2,
				       domPrecisionList, 1);
    if (m_domPrecision == 0 || m_domPrecision == 1)
      m_domPrecisionBox->SetSelection(m_domPrecision);
  }
  NewLine();

  m_playerBox = new wxListBox(this, NULL, "Players", wxMULTIPLE);
  for (int pl = 1; pl <= m_numPlayers; pl++) {
    if (p_players[pl] != "")
      m_playerBox->Append(p_players[pl]);
    else
      m_playerBox->Append("Player" + ToText(pl));
    m_playerBox->SetSelection(pl - 1, TRUE);
  }
  NewLine();
  
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char * ) this);
  (void) new wxButton(this, (wxFunction) CallbackHelp, "Help");
  Fit();
  Show(TRUE);
}

dialogElim::~dialogElim()
{
  wxWriteResource(SOLN_SECT, "ElimDom-All", m_all, gambitApp.ResourceFile());
  wxWriteResource(SOLN_SECT, "ElimDom-Type", m_domType, gambitApp.ResourceFile());
  wxWriteResource(SOLN_SECT, "ElimDom-Method", m_domMethod, gambitApp.ResourceFile());
  wxWriteResource(SOLN_SECT, "ElimDom-Precision", m_domPrecision,
		  gambitApp.ResourceFile());
}

void dialogElim::CallbackHelp(wxButton &, wxEvent &)
{
  wxHelpContents("Elimination of Dominated Strategies");
}

void dialogElim::OnOK(void)
{
  m_completed = wxOK;
  m_all = m_allBox->GetValue();
  m_compress = m_compressBox->GetValue();
  m_domType = m_domTypeBox->GetSelection();
  if (m_mixed) {
    m_domMethod = m_domMethodBox->GetSelection();
    m_domPrecision = m_domPrecisionBox->GetSelection();
  }
  int numSelections, *selections = new int[m_numPlayers];
  numSelections = m_playerBox->GetSelections(&selections);
  m_players = gArray<int>(numSelections);
  for (int i = 1; i <= numSelections; i++) {
    m_players[i] = selections[i-1] + 1;
  }	
  Show(FALSE);
}

void dialogElim::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogElim::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//               class SupportRemoveDialog: Member functions
//=========================================================================

void SupportRemoveDialog::all_func(wxCheckBox &ob,wxEvent &)
{
  SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
  for (int i=1;i<=parent->num_sups;i++)
    parent->sups_item->SetSelection(i-1,ob.GetValue());
}

void SupportRemoveDialog::ok_func(wxButton &ob,wxEvent &)
{
  SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
  parent->Show(FALSE);
  parent->completed=wxOK;
}

void SupportRemoveDialog::cancel_func(wxButton &ob,wxEvent &)
{
  SupportRemoveDialog *parent=(SupportRemoveDialog *)ob.GetClientData();
  parent->Show(FALSE);
  parent->completed=wxCANCEL;
}

SupportRemoveDialog::SupportRemoveDialog(wxWindow *parent, int num_sups_)
  : wxDialogBox(parent,"Remove Support(s)",TRUE), num_sups(num_sups_)
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

gArray<bool> SupportRemoveDialog::Selected(void)
{
  gArray<bool> selected(num_sups);
  for (int i=1;i<=num_sups;i++)
    selected[i]=(sups_item->Selected(i-1)) ? true : false;
  return selected;
}


//=========================================================================
//               class dialogNfgSupportInspect: Member functions
//=========================================================================

void dialogNfgSupportInspect::OnNewSupport(void)
{
  if (bns->MakeSupport()) {
    disp_item->Append(ToText(sups.Length()));
    disp_item->SetSize(-1,-1,-1,-1);
    cur_item->Append(ToText(sups.Length()));
    cur_item->SetSize(-1,-1,-1,-1);
  }
}

// Note can not delete the first support
void dialogNfgSupportInspect::OnRemoveSupport(void)
{
  int i;
  SupportRemoveDialog SRD(this,sups.Length());
  if (SRD.Completed()==wxOK)  {
    gArray<bool> selected(SRD.Selected());
    bool revert=false;
    for (i=sups.Length();i>=2;i--)
      if (selected[i])  {
	delete sups.Remove(i);
	if (i==init_cur || i==init_disp && revert==false)  {
	  wxMessageBox("Display/Current support deleted.\nReverting to full support");
	  revert=true;
	}
      }
    disp_item->Clear();cur_item->Clear();
    for (i=1;i<=sups.Length();i++)
      {disp_item->Append(ToText(i));cur_item->Append(ToText(i));}
    disp_item->SetSize(-1,-1,-1,-1);cur_item->SetSize(-1,-1,-1,-1);
    disp_item->SetSelection(0);cur_item->SetSelection(0);
    if (revert) bns->ChangeSupport(UPDATE_DIALOG);
  }
}

void dialogNfgSupportInspect::OnChangeSupport(void)
{
  bns->ChangeSupport(UPDATE_DIALOG);
  init_disp = Displayed();
  init_cur = Current();
}

void dialogNfgSupportInspect::OnCur(int cur_sup)
{
  cur_dim->SetValue(array_to_string(sups[cur_sup]->NumStrats()));
  disp_dim->SetValue(array_to_string(sups[cur_sup]->NumStrats()));
  disp_item->SetSelection(cur_sup-1);
}

void dialogNfgSupportInspect::OnDisp(int disp_sup)
{
  disp_dim->SetValue(array_to_string(sups[disp_sup]->NumStrats()));
}

gText dialogNfgSupportInspect::array_to_string(const gArray<int> &a)
{
  gText tmp='(';
  for (int i=1;i<=a.Length();i++)
    tmp+=ToText(a[i])+((i==a.Length()) ? ")" : ",");
  return tmp;
}

dialogNfgSupportInspect::dialogNfgSupportInspect(gList<NFSupport *> &p_supports,
					       int cur_sup, int disp_sup,
					       NfgShow *bns_,
					       wxWindow *parent /*=0*/)
  : wxDialogBox(parent,"Select Support"), bns(bns_), sups(p_supports)
{
  init_cur=cur_sup;init_disp=disp_sup;
  wxForm *f=new wxForm(0);
  SetLabelPosition(wxVERTICAL);
  cur_dim=new wxText(this,0,"Current",
		     array_to_string(sups[cur_sup]->NumStrats()),
		     -1,-1,80,-1,wxREADONLY);
  disp_dim=new wxText(this,0,"Display",
		      array_to_string(sups[disp_sup]->NumStrats()),
		      -1,-1,80,-1,wxREADONLY);

  support_list = new wxStringList;
  for (int i = 1; i <= p_supports.Length(); i++) {
    const gText &name = p_supports[i]->GetName();
    if (name == "") 
      support_list->Add("Support " + ToText(i));
    else
      support_list->Add(name);
  }

  cur_str = new char[100];
  if (p_supports[cur_sup]->GetName() != "")
    strncpy(cur_str, "Support " + ToText(cur_sup), 99);
  else
    strncpy(cur_str, p_supports[cur_sup]->GetName(), 99);

  disp_str = new char[100];
  if (p_supports[disp_sup]->GetName() != "")
    strncpy(cur_str, "Support " + ToText(disp_sup), 99);
  else
    strncpy(cur_str, p_supports[disp_sup]->GetName(), 99);

  wxFormItem *cur_fitem=wxMakeFormString("",&cur_str,wxFORM_CHOICE,
					 new wxList(wxMakeConstraintStrings(support_list),0));
  f->Add(cur_fitem);
  f->Add(wxMakeFormMessage("      ")); // fix the spacing... not neat but..
  wxFormItem *disp_fitem=wxMakeFormString("",&disp_str,wxFORM_CHOICE,
					  new wxList(wxMakeConstraintStrings(support_list),0));
  f->Add(disp_fitem);
  f->Add(wxMakeFormNewLine());
  wxFormItem *newsup_fitem=wxMakeFormButton("New",
					    (wxFunction) CallbackNew);
  f->Add(newsup_fitem);
  wxFormItem *rmvsup_fitem=wxMakeFormButton("Remove",
					    (wxFunction) CallbackRemove);
  f->Add(rmvsup_fitem);
  f->Add(wxMakeFormNewLine());
  wxFormItem *close_fitem=wxMakeFormButton("Close",
					   (wxFunction) CallbackClose);
  f->Add(close_fitem);
  wxFormItem *cngsup_fitem=wxMakeFormButton("Apply",
					    (wxFunction) CallbackChange);
  f->Add(cngsup_fitem);
  wxFormItem *help_fitem=wxMakeFormButton("?",(wxFunction) CallbackHelp);
  f->Add(help_fitem);
  f->AssociatePanel(this);
  cur_item=(wxChoice *)cur_fitem->GetPanelItem();
  cur_item->Callback((wxFunction) CallbackCurrent);
  cur_item->SetClientData((char *)this);
  disp_item=(wxChoice *)disp_fitem->GetPanelItem();
  disp_item->SetClientData((char *)this);
  disp_item->Callback((wxFunction) CallbackDisplayed);
  ((wxButton *)newsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)rmvsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)cngsup_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)help_fitem->GetPanelItem())->SetClientData((char *)this);
  ((wxButton *)close_fitem->GetPanelItem())->SetClientData((char *)bns);
  Fit();
  Show(TRUE);
}

Bool dialogNfgSupportInspect::OnClose(void)
{
  bns->ChangeSupport(DESTROY_DIALOG);
  return FALSE;
}
