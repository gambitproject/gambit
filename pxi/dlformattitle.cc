//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of title format dialog
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/fontdlg.h"
#include "wx/colordlg.h"

#include "dlformattitle.h"

const int idFONT_BUTTON = 2000;
const int idCOLOR_BUTTON = 2001;

BEGIN_EVENT_TABLE(dialogFormatTitle, wxDialog)
  EVT_BUTTON(idFONT_BUTTON, dialogFormatTitle::OnFontButton)
  EVT_BUTTON(idCOLOR_BUTTON, dialogFormatTitle::OnColorButton)
END_EVENT_TABLE()

//=========================================================================
//                class dialogFormatTitle implementation
//=========================================================================

dialogFormatTitle::dialogFormatTitle(wxWindow *p_parent,
				     const PxiTitleProperties &p_props)
  : wxDialog(p_parent, -1, "Format Title")
{
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Graph title:"),
		 0, wxALL, 5);
  m_title = new wxTextCtrl(this, -1, p_props.m_title);
  gridSizer->Add(m_title, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Font:"),
		 0, wxALL, 5);
  m_fontButton = new wxButton(this, idFONT_BUTTON, "Change...");
  m_fontButton->SetFont(p_props.m_font);
  gridSizer->Add(m_fontButton, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Color:"),
		 0, wxALL, 5);
  m_colorButton = new wxButton(this, idCOLOR_BUTTON, "");
  m_colorButton->SetBackgroundColour(p_props.m_color);
  gridSizer->Add(m_colorButton, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(gridSizer, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

PxiTitleProperties dialogFormatTitle::GetProperties(void) const
{
  PxiTitleProperties props;

  props.m_title = m_title->GetValue();
  props.m_color = m_colorButton->GetBackgroundColour();
  props.m_font = m_fontButton->GetFont();
  
  return props;
}

void dialogFormatTitle::OnFontButton(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_fontButton->GetFont());

  wxFontDialog dialog(this, &data);
  dialog.SetTitle("Choose Title Font");
  if (dialog.ShowModal() == wxID_OK) {
    m_fontButton->SetFont(dialog.GetFontData().GetChosenFont());
  }
}

void dialogFormatTitle::OnColorButton(wxCommandEvent &)
{
  wxColourData data;
  wxColour color = m_colorButton->GetBackgroundColour();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle("Choose Title Color");
  if (dialog.ShowModal() == wxID_OK) {
    m_colorButton->SetBackgroundColour(dialog.GetColourData().GetColour());
  }
}
