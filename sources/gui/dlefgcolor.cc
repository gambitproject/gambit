//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to change player color
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/colordlg.h"

#include "treedraw.h"
#include "dlefgcolor.h"

//
// The following colors are predefined in the code as defaults, overridden
// only when the corresponding entries are present in the configuration file:
//
// Chance player: wxLIGHT_GREY
// Terminal nodes: wxBLACK
// Even-numbered players: wxGREEN
// Odd-numbered players: wxRED
//

const int idBUTTON_TERMINAL_COLOR = 3000;
const int idBUTTON_CHANCE_COLOR = 3001;
// Player buttons are numbered idBUTTON_CHANCE_COLOR+1..idBUTTON_CHANCE_COLOR+8

BEGIN_EVENT_TABLE(dialogEfgColor, wxDialog)
  EVT_BUTTON(idBUTTON_TERMINAL_COLOR, dialogEfgColor::OnTerminalColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR, dialogEfgColor::OnChanceColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+1, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+2, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+3, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+4, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+5, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+6, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+7, dialogEfgColor::OnPlayerColor)
  EVT_BUTTON(idBUTTON_CHANCE_COLOR+8, dialogEfgColor::OnPlayerColor)
END_EVENT_TABLE()

dialogEfgColor::dialogEfgColor(wxWindow *p_parent,
			       const TreeDrawSettings &p_settings)
  : wxDialog(p_parent, -1, "Choose colors")
{
  SetAutoLayout(true);

  wxStaticBox *colorBox = new wxStaticBox(this, -1, "Player colors");
  wxStaticBoxSizer *colorBoxSizer = new wxStaticBoxSizer(colorBox, wxVERTICAL);
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 10, 10);

  gridSizer->Add(new wxStaticText(this, -1, "Chance player"), 0,
		 wxALIGN_CENTER, 0);
  m_chanceButton = new wxButton(this, idBUTTON_CHANCE_COLOR, "");
  m_chanceButton->SetBackgroundColour(p_settings.ChanceColor());
  gridSizer->Add(m_chanceButton, 0, wxALIGN_CENTER, 0);
  
  gridSizer->Add(new wxStaticText(this, -1, "Terminal nodes"), 0,
		 wxALIGN_CENTER, 0);
  m_terminalButton = new wxButton(this, idBUTTON_TERMINAL_COLOR, "");
  m_terminalButton->SetBackgroundColour(p_settings.TerminalColor());
  gridSizer->Add(m_terminalButton, 0, wxALIGN_CENTER, 0);

  for (int pl = 1; pl <= 8; pl++) {
    gridSizer->Add(new wxStaticText(this, -1,
				    wxString::Format("Player %d", pl)),
		   0, wxALIGN_CENTER, 0);
    m_playerButton[pl-1] = new wxButton(this, idBUTTON_CHANCE_COLOR+pl, "");
    m_playerButton[pl-1]->SetBackgroundColour(p_settings.PlayerColor(pl));
    gridSizer->Add(m_playerButton[pl-1], 0, wxALIGN_CENTER, 0);
  }

  colorBoxSizer->Add(gridSizer, 0, wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(colorBoxSizer, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogEfgColor::OnTerminalColor(wxCommandEvent &)
{
  wxColourData data;
  wxColour color = m_terminalButton->GetBackgroundColour();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle("Choose Terminal Color");
  if (dialog.ShowModal() == wxID_OK) {
    m_terminalButton->SetBackgroundColour(dialog.GetColourData().GetColour());
  }
}

void dialogEfgColor::OnChanceColor(wxCommandEvent &)
{
  wxColourData data;
  wxColour color = m_chanceButton->GetBackgroundColour();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle("Choose Chance Color");
  if (dialog.ShowModal() == wxID_OK) {
    m_chanceButton->SetBackgroundColour(dialog.GetColourData().GetColour());
  }
}

void dialogEfgColor::OnPlayerColor(wxCommandEvent &p_event)
{
  int player = p_event.GetId() - idBUTTON_CHANCE_COLOR;
  wxColourData data;
  wxColour color = m_playerButton[player-1]->GetBackgroundColour();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle(wxString::Format("Choose Player %d Color", player));
  if (dialog.ShowModal() == wxID_OK) {
    m_playerButton[player-1]->SetBackgroundColour(dialog.GetColourData().GetColour());
  }
}


