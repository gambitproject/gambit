//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of axis format dialog
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"

#include "dlformataxis.h"
#include "guishare/valinteger.h"
#include "guishare/valnumber.h"

//=========================================================================
//                   class DisplayPanel implementation
//=========================================================================

class DisplayPanel : public wxPanel {
private:
  wxCheckBox *m_shown, *m_ticks, *m_numbers;

public:
  DisplayPanel(wxWindow *p_parent, const PxiAxisDisplayProperties &p_prop);

  PxiAxisDisplayProperties GetProperties(void) const;
};

DisplayPanel::DisplayPanel(wxWindow *p_parent,
			   const PxiAxisDisplayProperties &p_prop)
  : wxPanel(p_parent, -1)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  m_shown = new wxCheckBox(this, -1, "Show axis");
  m_shown->SetValue(p_prop.m_shown);
  sizer->Add(m_shown, 0, wxALL, 5);
  
  m_ticks = new wxCheckBox(this, -1, "Show tickmarks");
  m_ticks->SetValue(p_prop.m_ticks);
  sizer->Add(m_ticks, 0, wxALL, 5);

  m_numbers = new wxCheckBox(this, -1, "Show scale");
  m_numbers->SetValue(p_prop.m_numbers);
  sizer->Add(m_numbers, 0, wxALL, 5);

  SetAutoLayout(true);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  Layout();
}

PxiAxisDisplayProperties DisplayPanel::GetProperties(void) const
{
  PxiAxisDisplayProperties props;
  props.m_shown = m_shown->GetValue();
  props.m_ticks = m_ticks->GetValue();
  props.m_numbers = m_numbers->GetValue();
  return props;
}

//=========================================================================
//                    class ScalePanel implementation
//=========================================================================

class ScalePanel : public wxPanel {
private:
  wxTextCtrl *m_minimum, *m_maximum, *m_divisions;
  wxString m_minValue, m_maxValue, m_divisionValue;
  wxCheckBox *m_logScale;

public:
  ScalePanel(wxWindow *p_parent, const PxiAxisScaleProperties &p_prop);

  PxiAxisScaleProperties GetProperties(void) const;

  virtual bool Validate(void);
};

ScalePanel::ScalePanel(wxWindow *p_parent,
		       const PxiAxisScaleProperties &p_prop)
  : wxPanel(p_parent, -1)
{
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);


  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Minimum value:"),
		 0, wxALL, 5);
  m_minValue = p_prop.m_minimum;
  m_minimum = new wxTextCtrl(this, -1, p_prop.m_minimum,
			     wxDefaultPosition, wxDefaultSize,
			     0, gNumberValidator(&m_minValue, 0.0),
			     "minimum value");
  gridSizer->Add(m_minimum, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Maximum value:"),
		 0, wxALL, 5);
  m_maxValue = p_prop.m_maximum;
  m_maximum = new wxTextCtrl(this, -1, p_prop.m_maximum,
			     wxDefaultPosition, wxDefaultSize,
			     0, gNumberValidator(&m_maxValue, 0.01),
			     "maximum value");
  gridSizer->Add(m_maximum, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of divisions:"),
		 0, wxALL, 5);
  m_divisionValue = wxString::Format("%d", p_prop.m_divisions);
  m_divisions = new wxTextCtrl(this, -1, m_divisionValue, 
			       wxDefaultPosition, wxDefaultSize,
			       0, gIntegerValidator(&m_divisionValue, 1),
			       "number of divisions");
  gridSizer->Add(m_divisions, 0, wxALL, 5);

  m_logScale = new wxCheckBox(this, -1, "Use logarithmic scale");
  m_logScale->SetValue(p_prop.m_useLog);
  m_logScale->Enable(p_prop.m_canUseLog);
  gridSizer->Add(m_logScale, 0, wxALL, 5);

  SetAutoLayout(true);
  SetSizer(gridSizer);
  gridSizer->Fit(this);
  gridSizer->SetSizeHints(this);
  Layout();
}

PxiAxisScaleProperties ScalePanel::GetProperties(void) const
{
  PxiAxisScaleProperties props;

  props.m_minimum = m_minimum->GetValue();
  props.m_maximum = m_maximum->GetValue();
  props.m_divisions = atoi(m_divisions->GetValue());
  props.m_useLog = m_logScale->GetValue();

  return props;
}

bool ScalePanel::Validate(void)
{
  if (!wxWindow::Validate()) {
    return false;
  }

  if (GetProperties().GetMaximum() <= GetProperties().GetMinimum()) {
    wxMessageBox("The maximum value must be greater than the minimum value.",
		 "Error", wxOK | wxICON_EXCLAMATION, this);
    m_minimum->SetFocus();
    return false;
  }
  return true;
}

//=========================================================================
//                    class FontPanel implementation
//=========================================================================

class FontPanel : public wxPanel {
private:
  wxFont m_font;

public:
  FontPanel(wxWindow *p_parent, const wxFont &p_font);

  wxFont GetFont(void) const;
};

FontPanel::FontPanel(wxWindow *p_parent, const wxFont &p_font)
  : wxPanel(p_parent, -1), m_font(p_font)
{
  
}

wxFont FontPanel::GetFont(void) const
{
  return wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
}

//=========================================================================
//                    class ColorPanel implementation
//=========================================================================

class ColorPanel : public wxPanel {
private:

public:
  ColorPanel(wxWindow *p_parent);

  wxColour GetColor(void) const { return *wxBLUE; }
};

ColorPanel::ColorPanel(wxWindow *p_parent)
  : wxPanel(p_parent, -1)
{

}

//=========================================================================
//                 class dialogFormatAxis implementation
//=========================================================================

const int idFORMAT_NOTEBOOK = 2000;

BEGIN_EVENT_TABLE(dialogFormatAxis, wxDialog)
  EVT_BUTTON(wxID_OK, dialogFormatAxis::OnOK)
  EVT_NOTEBOOK_PAGE_CHANGING(idFORMAT_NOTEBOOK,
			     dialogFormatAxis::OnNotebookPageChanging)
END_EVENT_TABLE()

dialogFormatAxis::dialogFormatAxis(wxWindow *p_parent,
				   const PxiAxisProperties &p_props)
  : wxDialog(p_parent, -1, "Format Axis")
{
  m_notebook = new wxNotebook(this, idFORMAT_NOTEBOOK,
			      wxDefaultPosition, wxSize(300, 300));
  m_displayPanel = new DisplayPanel(m_notebook, p_props.m_display);
  m_notebook->AddPage(m_displayPanel, "Display");
  m_scalePanel = new ScalePanel(m_notebook, p_props.m_scale);
  m_notebook->AddPage(m_scalePanel, "Scale");
  m_fontPanel = new FontPanel(m_notebook, p_props.m_font);
  m_notebook->AddPage(m_fontPanel, "Font");
  m_colorPanel = new ColorPanel(m_notebook);
  m_notebook->AddPage(m_colorPanel, "Color");

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_notebook, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

PxiAxisProperties dialogFormatAxis::GetProperties(void) const
{
  PxiAxisProperties props;
  props.m_display = m_displayPanel->GetProperties();
  props.m_scale = m_scalePanel->GetProperties();
  props.m_color = m_colorPanel->GetColor();
  props.m_font = m_fontPanel->GetFont();
  return props;
}

void dialogFormatAxis::OnOK(wxCommandEvent &)
{
  if (!m_scalePanel->Validate()) {
    m_notebook->SetSelection(1);
    return;
  }

  EndModal(wxID_OK);
}

void dialogFormatAxis::OnNotebookPageChanging(wxNotebookEvent &p_event)
{
  if (m_notebook->GetSelection() == 1) {
    if (!m_scalePanel->Validate()) {
      p_event.Veto();
    }
  }
}

