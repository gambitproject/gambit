//
// FILE: dialogpaged.cc -- Implementation of "paged" dialog
//
// %W% %G%
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "dialogpaged.h"

const int idBACK_BUTTON = 5001;
const int idNEXT_BUTTON = 5002;

BEGIN_EVENT_TABLE(guiPagedDialog, wxDialog)
  EVT_BUTTON(idBACK_BUTTON, OnBack)
  EVT_BUTTON(idNEXT_BUTTON, OnNext)
END_EVENT_TABLE()

//========================================================================
//                     guiPagedDialog: Member functions
//========================================================================

int guiPagedDialog::s_itemsPerPage = 8;

guiPagedDialog::guiPagedDialog(wxWindow *p_parent, char *p_title,
			       int p_numValues)
  : guiAutoDialog(p_parent, p_title),
    m_pageNumber(0), m_numFields(gmin(p_numValues, s_itemsPerPage)),
    m_backButton(0), m_nextButton(0),
    m_dataValues(p_numValues)
{
  m_dataFields = new wxTextCtrl *[m_numFields];
  for (int i = 1; i <= m_numFields; i++) {
    m_dataFields[i-1] = new wxTextCtrl(this, -1);
  }

  if (p_numValues > s_itemsPerPage) {
    m_backButton = new wxButton(this, idBACK_BUTTON, "<< Back");
    m_backButton->Enable(FALSE);

    m_backButton->SetConstraints(new wxLayoutConstraints);
    m_backButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
                                               wxBottom, 10);
    m_backButton->GetConstraints()->right.SameAs(m_cancelButton, wxCentreX, 5);
    m_backButton->GetConstraints()->height.AsIs();
    m_backButton->GetConstraints()->width.AsIs();

    m_nextButton = new wxButton(this, idNEXT_BUTTON, "Next >>");
    m_nextButton->SetConstraints(new wxLayoutConstraints);
    m_nextButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
					       wxBottom, 10);
    m_nextButton->GetConstraints()->left.SameAs(m_cancelButton, wxCentreX, 5);
    m_nextButton->GetConstraints()->height.AsIs();
    m_nextButton->GetConstraints()->width.AsIs();
  }

  if (m_backButton)  {
    m_okButton->GetConstraints()->top.SameAs(m_backButton, wxBottom, 10);
  }
  else  {
    m_okButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
					     wxBottom, 10);
  }
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_cancelButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  for (int i = 1; i <= m_numFields; i++) {
    m_dataFields[i-1]->SetConstraints(new wxLayoutConstraints);
    if (i == 1) {
      m_dataFields[i-1]->GetConstraints()->top.SameAs(this, wxTop, 10);
    }
    else {
      m_dataFields[i-1]->GetConstraints()->top.SameAs(m_dataFields[i-2],
						      wxBottom, 10);
    }
    m_dataFields[i-1]->GetConstraints()->height.AsIs();
    m_dataFields[i-1]->GetConstraints()->left.SameAs(m_okButton, wxLeft);
    m_dataFields[i-1]->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  }
}

guiPagedDialog::~guiPagedDialog()
{
  delete [] m_dataFields;
}

/*
void guiPagedDialog::OnOk(void)
{
  //  guiAutoDialog::OnOk();
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < m_numFields; i++, entry++) {
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();
  }
}
*/

void guiPagedDialog::OnBack(wxCommandEvent &)
{
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < gmin((m_pageNumber + 1) * s_itemsPerPage,
		m_dataValues.Length()); i++, entry++)
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();

  m_pageNumber--;
  entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++) {
    m_dataFields[entry]->Show(FALSE);
    m_dataFields[entry]->SetValue((char *) m_dataValues[i + 1]);
    m_dataFields[entry]->SetLabel((char *) ToText(i + 1));
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_itemsPerPage - 1; entry >= 0; entry--)
    m_dataFields[entry]->Show(TRUE);
  
  m_dataFields[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void guiPagedDialog::OnNext(wxCommandEvent &)
{
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++)
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();

  m_pageNumber++;
  entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++) {
    if (i < m_dataValues.Length()) {
      m_dataFields[entry]->SetValue((char *) m_dataValues[i + 1]);
      m_dataFields[entry]->SetLabel((char *) ToText(i + 1));
    }
    else
      m_dataFields[entry]->Show(FALSE);
  }

  m_dataFields[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_itemsPerPage <=
		       m_dataValues.Length());
}

void guiPagedDialog::SetValue(int p_index, const gText &p_value)
{
  m_dataValues[p_index] = p_value;
  if ((p_index - 1) / s_itemsPerPage == m_pageNumber)
    m_dataFields[(p_index - 1) % s_itemsPerPage]->SetValue((char *) p_value);
}
