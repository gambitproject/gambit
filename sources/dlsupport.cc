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
#include "dlsupportselect.h"

const char *SOLN_SECT = "Soln-Defaults";

//=========================================================================
//                  class dialogElim: Member functions
//=========================================================================

dialogElim::dialogElim(const gArray<gText> &p_players, bool p_mixed,
		       wxWindow *p_parent /* = NULL */)
  : guiAutoDialog(p_parent, "Dominance Elimination Parameters"),
    m_mixed(p_mixed)
{
  Bool all = FALSE;
  int domType = 0, domMethod = 0, domPrecision = 0;
  wxGetResource(SOLN_SECT, "ElimDom-All", &all, gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Type", &domType, gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Method", &domMethod,
		gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "ElimDom-Precision", &domPrecision,
		gambitApp.ResourceFile());

  m_allBox = new wxCheckBox(this, 0, "Eliminate iteratively");
  m_allBox->SetValue(all);
  m_allBox->SetConstraints(new wxLayoutConstraints);
  m_allBox->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_allBox->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_allBox->GetConstraints()->width.AsIs();
  m_allBox->GetConstraints()->height.AsIs();
					
  m_compressBox = new wxCheckBox(this, 0, "Compress");
  m_compressBox->SetConstraints(new wxLayoutConstraints);
  m_compressBox->GetConstraints()->top.SameAs(m_allBox, wxTop);
  m_compressBox->GetConstraints()->left.SameAs(m_allBox, wxRight, 10);
  m_compressBox->GetConstraints()->width.AsIs();
  m_compressBox->GetConstraints()->height.AsIs();

  char *domTypeList[2] = {"Weak", "Strong"};
  m_domTypeBox = new wxRadioBox(this, NULL, "Type", 1, 1, -1, -1, 2,
				domTypeList, 1);
  if (domType == 0 || domType == 1)
    m_domTypeBox->SetSelection(domType);
  m_domTypeBox->SetConstraints(new wxLayoutConstraints);
  m_domTypeBox->GetConstraints()->top.SameAs(m_allBox, wxBottom, 10);
  m_domTypeBox->GetConstraints()->left.SameAs(m_allBox, wxLeft);
  m_domTypeBox->GetConstraints()->width.AsIs();
  m_domTypeBox->GetConstraints()->height.AsIs();
  
  char *domMethodList[2] = {"Pure", "Mixed"};
  m_domMethodBox = new wxRadioBox(this, NULL, "Method", 1, 1, -1, -1, 2,
				  domMethodList, 1);
  if (domMethod == 0 || domMethod == 1)
    m_domMethodBox->SetSelection(domMethod);
  m_domMethodBox->SetConstraints(new wxLayoutConstraints);
  m_domMethodBox->GetConstraints()->top.SameAs(m_domTypeBox, wxTop);
  m_domMethodBox->GetConstraints()->left.SameAs(m_domTypeBox, wxRight, 10);
  m_domMethodBox->GetConstraints()->width.AsIs();
  m_domMethodBox->GetConstraints()->height.AsIs();
  
  char *domPrecisionList[2] = {"Float", "Rational" };
  m_domPrecisionBox = new wxRadioBox(this, NULL, "Precision", -1, -1, -1, -1, 2,
				     domPrecisionList, 1);
  if (domPrecision == 0 || domPrecision == 1)
    m_domPrecisionBox->SetSelection(domPrecision);
  m_domPrecisionBox->SetConstraints(new wxLayoutConstraints);
  m_domPrecisionBox->GetConstraints()->top.SameAs(m_domMethodBox, wxTop);
  m_domPrecisionBox->GetConstraints()->left.SameAs(m_domMethodBox, wxRight, 10);
  m_domPrecisionBox->GetConstraints()->width.AsIs();
  m_domPrecisionBox->GetConstraints()->height.AsIs();

  if (!m_mixed) {
    m_domMethodBox->Show(FALSE);
    m_domPrecisionBox->Show(FALSE);
  }

  m_playerBox = new wxListBox(this, NULL, "Players", wxMULTIPLE);
  for (int pl = 1; pl <= p_players.Length(); pl++) {
    m_playerBox->Append(ToText(pl) + ": " + p_players[pl]);
    m_playerBox->SetSelection(pl - 1, TRUE);
  }
  m_playerBox->SetConstraints(new wxLayoutConstraints);
  m_playerBox->GetConstraints()->top.SameAs(m_domTypeBox, wxBottom, 10);
  m_playerBox->GetConstraints()->left.SameAs(m_domTypeBox, wxLeft);
  m_playerBox->GetConstraints()->right.SameAs(m_domPrecisionBox, wxRight);
  m_playerBox->GetConstraints()->height.AsIs();
  
  m_okButton->GetConstraints()->top.SameAs(m_playerBox, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(m_playerBox, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

dialogElim::~dialogElim()
{
  if (Completed() == wxOK) {
    wxWriteResource(SOLN_SECT, "ElimDom-All", 
		    m_allBox->GetValue(), gambitApp.ResourceFile());
    wxWriteResource(SOLN_SECT, "ElimDom-Type", m_domTypeBox->GetSelection(),
		    gambitApp.ResourceFile());
    wxWriteResource(SOLN_SECT, "ElimDom-Method",
		    m_domMethodBox->GetSelection(), gambitApp.ResourceFile());
    wxWriteResource(SOLN_SECT, "ElimDom-Precision",
		    m_domPrecisionBox->GetSelection(), gambitApp.ResourceFile());
  }
}

gArray<int> dialogElim::Players(void) const
{
  gBlock<int> players;
  for (int i = 1; i <= m_playerBox->Number(); i++) {
    if (m_playerBox->Selected(i-1)) {
      players.Append(i);
    }
  }
  return players;
}

//=========================================================================
//                class dialogSupportSelect: Member functions
//=========================================================================

dialogSupportSelect::dialogSupportSelect(const gList<NFSupport *> &p_supports,
					 NFSupport *p_current,
					 const gText &p_caption,
					 wxWindow *p_parent /*=0*/)
  : guiAutoDialog(p_parent, p_caption)
{
  SetLabelPosition(wxVERTICAL);
  m_supportList = new wxListBox(this, 0, "Support", wxSINGLE, 1, 1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    NFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();

    item += (" (" + ToText(support->NumStrats(1)) + ", " +
	     ToText(support->NumStrats(2)));
    if (support->Game().NumPlayers() > 2) {
      item += ", " + ToText(support->NumStrats(3));
      if (support->Game().NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_supportList->Append(item);
    if (p_supports[s] == p_current) {
      m_supportList->SetSelection(s - 1);
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

dialogSupportSelect::dialogSupportSelect(const gList<EFSupport *> &p_supports,
					 EFSupport *p_current,
					 const gText &p_caption,
					 wxWindow *p_parent /*=0*/)
  : guiAutoDialog(p_parent, p_caption)
{
  SetLabelPosition(wxVERTICAL);
  m_supportList = new wxListBox(this, 0, "Support", wxSINGLE, 1, 1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    EFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();
    m_supportList->Append(item);
    if (p_supports[s] == p_current) {
      m_supportList->SetSelection(s - 1);
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
