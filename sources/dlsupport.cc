//
// FILE: dlsupport.cc -- Dialogs for manipulation of supports
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "garray.h"
#include "dlelim.h"

const char *SOLN_SECT = "Soln-Defaults";

//=========================================================================
//                  class dialogElim: Member functions
//=========================================================================

dialogElim::dialogElim(int p_numPlayers, wxWindow *p_parent /* = NULL */)
  : wxDialogBox(p_parent, "Dominance Elimination Parameters", TRUE),
    m_compress(false), m_numPlayers(p_numPlayers)
{
  wxGetResource(SOLN_SECT, "ElimDom-All", &m_all, "gambit.ini");
  wxGetResource(SOLN_SECT, "ElimDom-Type", &m_domType, "gambit.ini");
  wxGetResource(SOLN_SECT, "ElimDom-Method", &m_domMethod, "gambit.ini");
  wxGetResource(SOLN_SECT, "ElimDom-Precision", &m_domPrecision,
		"gambit.ini");

  m_allBox = new wxCheckBox(this, 0, "Eliminate iteratively");
  m_allBox->SetValue(m_all);
  m_compressBox = new wxCheckBox(this, 0, "Compress");
  NewLine();

  char *domTypeList[2] = {"Weak", "Strong"};
  m_domTypeBox = new wxRadioBox(this, NULL, "Type", -1, -1, -1, -1, 2,
				domTypeList, 1);
  if (m_domType == 0 || m_domType == 1)
    m_domTypeBox->SetSelection(m_domType);

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
  NewLine();

  wxStringList *playerList = wxStringListInts(m_numPlayers);
  m_playerBox = new wxListBox(this, NULL, "Players", wxMULTIPLE, -1, -1, -1, -1,
			      m_numPlayers, playerList->ListToArray());
  for (int i = 1; i <= m_numPlayers; i++)
    m_playerBox->SetSelection(i-1, TRUE);
  NewLine();
  
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char * )this);
  (void) new wxButton(this, (wxFunction) CallbackHelp, "Help");
  Fit();
  Show(TRUE);
}

dialogElim::~dialogElim()
{
  wxWriteResource(SOLN_SECT, "ElimDom-All", m_all, "gambit.ini");
  wxWriteResource(SOLN_SECT, "ElimDom-Type", m_domType, "gambit.ini");
  wxWriteResource(SOLN_SECT, "ElimDom-Method", m_domMethod, "gambit.ini");
  wxWriteResource(SOLN_SECT, "ElimDom-Precision", m_domPrecision,
		  "gambit.ini");
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
  m_domMethod = m_domMethodBox->GetSelection();
  m_domPrecision = m_domPrecisionBox->GetSelection();
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

