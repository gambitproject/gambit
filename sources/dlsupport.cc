//
// FILE: dlsupport.cc -- Dialogs for manipulation of supports
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "garray.h"
#include "gambit.h"
#include "nfgshow.h"
#include "dlelim.h"
#include "dlnfgsupportview.h"

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
//               class dialogNfgSupportView: Member functions
//=========================================================================

dialogNfgSupportView::dialogNfgSupportView(const gList<NFSupport *> &p_supports,
					   NFSupport *p_current,
					   wxWindow *p_parent /*=0*/)
  : guiAutoDialog(p_parent, "Select Support")
{
  m_supportList = new wxListBox(this, 0, "Support", wxSINGLE, 1, 1);
  for (int support = 1; support <= p_supports.Length(); support++) {
    m_supportList->Append(ToText(support) + ": " + 
			  p_supports[support]->GetName());
    if (p_supports[support] == p_current) {
      m_supportList->SetSelection(support - 1);
    }
  }

  m_okButton->GetConstraints()->top.SameAs(m_supportList, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  m_supportList->SetConstraints(new wxLayoutConstraints);
  m_supportList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_supportList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_supportList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_supportList->GetConstraints()->height.AsIs();
  
  Go();
}
