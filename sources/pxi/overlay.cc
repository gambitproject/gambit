//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Namespace for storing, setting, and reading/writing overlay options
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/spinctrl.h"

#include "overlay.h"

namespace Overlay {

//=========================================================================
//               class Overlay::Properties implementation
//=========================================================================

Properties::Properties(void)
  : m_token(tokenNUMBER), m_lines(false), m_tokenSize(2),
    m_font(10, wxSWISS, wxNORMAL, wxBOLD)
{ }

void Properties::ReadConfig(wxConfig &)
{

}

void Properties::WriteConfig(wxConfig &) const
{

}

//=========================================================================
//              class Overlay::PatternsPanel implementation
//=========================================================================

class PatternsPanel : public wxPanel {
private:
  wxRadioBox *m_token;
  wxCheckBox *m_connect;
  wxSpinCtrl *m_tokenSize;

public:
  PatternsPanel(wxWindow *p_parent, const Properties &);

  void GetProperties(Properties &) const;
};

PatternsPanel::PatternsPanel(wxWindow *p_parent, const Properties &p_props)
  : wxPanel(p_parent, -1)
{
  const wxString radioChoices[8] = {
    "Number", "x", "+", "circle", "del", "triangle", "square", "star"
  };

  m_token = new wxRadioBox(this, -1, "Token", wxDefaultPosition,
			   wxDefaultSize, 8, radioChoices,
			   1, wxRA_SPECIFY_COLS);
  m_token->SetSelection(p_props.m_token);

  wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
  m_connect = new wxCheckBox(this, -1, "Connect points");
  m_connect->SetValue(p_props.m_lines);
  vertSizer->Add(m_connect, 0, wxALL, 5);

  wxBoxSizer *sizeSizer = new wxBoxSizer(wxHORIZONTAL);
  sizeSizer->Add(new wxStaticText(this, wxID_STATIC, "Token size"),
		 0, wxALL, 5);
  m_tokenSize = new wxSpinCtrl(this, -1, 
			       wxString::Format("%d", p_props.m_tokenSize),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 8, p_props.m_tokenSize);
  sizeSizer->Add(m_tokenSize, 0, wxALL, 5);
  vertSizer->Add(sizeSizer, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(m_token, 0, wxALL, 5);
  topSizer->Add(vertSizer, 0, wxALL, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void PatternsPanel::GetProperties(Properties &p_props) const
{
  p_props.m_token = (TokenType) m_token->GetSelection();
  p_props.m_lines = m_connect->GetValue();
  p_props.m_tokenSize = m_tokenSize->GetValue();
}

//=========================================================================
//                 class Overlay::Dialog implementation
//=========================================================================

Dialog::Dialog(wxWindow *p_parent, const Properties &p_props)
  : wxDialog(p_parent, -1, "Format Overlay")
{
  m_notebook = new wxNotebook(this, -1, wxDefaultPosition, wxSize(300, 300));
  
  m_patternsPanel = new PatternsPanel(m_notebook, p_props);
  m_notebook->AddPage(m_patternsPanel, "Patterns");

  m_fontPanel = new FontPanel(m_notebook, p_props.m_font);
  m_notebook->AddPage(m_fontPanel, "Font");

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_notebook, 0, wxALL, 5);
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

  m_patternsPanel->GetProperties(props);
  props.m_font = m_fontPanel->GetFont();
  
  return props;
}


}  // namespace Overlay
