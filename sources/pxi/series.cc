//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Namespace for storing, setting, and reading/writing series options
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "series.h"

namespace Series {

//=========================================================================
//                class Series::Properties implementation
//=========================================================================

Properties::Properties(void)
  : m_connect(false)
{ }

void Properties::ReadConfig(wxConfig &)
{
}

void Properties::WriteConfig(wxConfig &) const
{
}

//=========================================================================
//                 class Series::Dialog implementation
//=========================================================================

Dialog::Dialog(wxWindow *p_parent, const Properties &p_props)
  : wxDialog(p_parent, -1, "Format Series")
{
  m_connect = new wxCheckBox(this, -1, "Connect points");
  m_connect->SetValue(p_props.m_connect);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_connect, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

Properties Dialog::GetProperties(void) const
{
  Properties props;
  props.m_connect = m_connect->GetValue();
  return props;
}

}  // namespace Series
