//
// FILE: dialogefgsave.cc -- Dialog for saving .efg files
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "efg.h"
#include "dialogauto.h"
#include "dialogefgsave.h"

const int idBROWSE_BUTTON = 1001;

BEGIN_EVENT_TABLE(dialogEfgSave, guiAutoDialog)
  EVT_BUTTON(idBROWSE_BUTTON, OnBrowse)
END_EVENT_TABLE()

//=========================================================================
//                     dialogEfgSave: Member functions
//=========================================================================

dialogEfgSave::dialogEfgSave(wxWindow *p_parent, const gText &p_name,
			     const gText &p_label, int p_decimals)
  : guiAutoDialog(p_parent, "Save File")
{
  m_fileName = new wxTextCtrl(this, -1);
  m_fileName->SetValue((char *) p_name);
  m_fileName->SetConstraints(new wxLayoutConstraints);
  m_fileName->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_fileName->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_fileName->GetConstraints()->width.AsIs();
  m_fileName->GetConstraints()->height.AsIs();

  wxButton *browseButton = new wxButton(this, idBROWSE_BUTTON, "Browse...");
  browseButton->SetConstraints(new wxLayoutConstraints);
  browseButton->GetConstraints()->top.SameAs(m_fileName, wxTop);
  browseButton->GetConstraints()->left.SameAs(m_fileName, wxRight, 10);
  browseButton->GetConstraints()->width.AsIs();
  browseButton->GetConstraints()->height.AsIs();

  m_treeLabel = new wxTextCtrl(this, -1);
  m_treeLabel->SetValue((char *) p_label);
  m_treeLabel->SetConstraints(new wxLayoutConstraints);
  m_treeLabel->GetConstraints()->top.SameAs(m_fileName, wxBottom, 10);
  m_treeLabel->GetConstraints()->left.SameAs(m_fileName, wxLeft);
  m_treeLabel->GetConstraints()->right.SameAs(browseButton, wxRight);
  m_treeLabel->GetConstraints()->height.AsIs();

  m_numDecimals = new wxSlider(this, -1, p_decimals, 0, 25);
  m_numDecimals->SetConstraints(new wxLayoutConstraints);
  m_numDecimals->GetConstraints()->top.SameAs(m_treeLabel, wxBottom, 10);
  m_numDecimals->GetConstraints()->left.SameAs(m_treeLabel, wxLeft);
  m_numDecimals->GetConstraints()->right.SameAs(browseButton, wxRight);
  m_numDecimals->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_numDecimals, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->top.AsIs();
  m_helpButton->GetConstraints()->left.AsIs();
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->Show(FALSE);

  AutoSize();
}

void dialogEfgSave::OnBrowse(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Save extensive form...");
  dialog.SetWildcard("*.efg");
  if (dialog.ShowModal() == wxID_OK) {
    m_fileName->SetValue(dialog.GetPath());
  }
}

gText dialogEfgSave::Filename(void) const
{ return m_fileName->GetValue().c_str(); }

gText dialogEfgSave::Label(void) const 
{ return m_treeLabel->GetValue().c_str(); }
