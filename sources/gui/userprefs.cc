//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class for global GUI user preferences.
// (Formerly gambdraw.h, plus extensions and additions for 0.97)
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/colordlg.h"

#include "wxmisc.h"

#include "gambit.h"
#include "userprefs.h"

//
// The following colors are predefined in the code as defaults, overridden
// only when the corresponding entries are present in the configuration file:
//
// Chance player: wxLIGHT_GREY
// Terminal nodes: wxBLACK
// Even-numbered players: wxGREEN
// Odd-numbered players: wxRED
//

UserPreferences::UserPreferences(void)
  : m_terminalColor(*wxBLACK), m_chanceColor(*wxLIGHT_GREY),
    m_warnSolveImperfect(true), m_warnSupportDelete(true),
    m_warnProfileInvalid(true)
{
  LoadOptions();
}

UserPreferences::~UserPreferences()
{ }

void UserPreferences::SetPlayerColor(int p_player, const wxColour &p_color)
{
  if (p_player > 0) {
    m_playerColors[(p_player - 1) % 8] = p_color;
  }
}

const wxColour &UserPreferences::GetPlayerColor(int p_player) const
{
  if (p_player <= 0) {
    return *wxBLACK;
  }
  else {
    return m_playerColors[(p_player - 1) % 8];
  }
}

void UserPreferences::LoadOptions(void)
{
  wxConfig config("Gambit");
  long red, green, blue;
  
  config.Read("/Colors/Chance-Red", &red, wxLIGHT_GREY->Red());
  config.Read("/Colors/Chance-Green", &green, wxLIGHT_GREY->Green());
  config.Read("/Colors/Chance-Blue", &blue, wxLIGHT_GREY->Blue());
  m_chanceColor.Set(red, green, blue);

  config.Read("/Colors/Terminal-Red", &red, 0L);
  config.Read("/Colors/Terminal-Green", &green, 0L);
  config.Read("/Colors/Terminal-Blue", &blue, 0L);
  m_terminalColor.Set(red, green, blue);

  for (int i = 0; i < 8; i++) {
    gText playerString = "/Colors/Player" + ToText(i);
    config.Read((char *) (playerString + "-Red"), &red, (i % 2 == 0) ? wxRED->Red() : 0L);
    config.Read((char *) (playerString + "-Green"), &green, (i % 2 == 1) ? wxGREEN->Green() : 0L);
    config.Read((char *) (playerString + "-Blue"), &blue, 0L);
    m_playerColors[i].Set(red, green, blue);
  } 

  config.Read("/Warnings/Solve-Imperfect-Recall", &m_warnSolveImperfect, true);
  config.Read("/Warnings/Support-Delete", &m_warnSupportDelete, true);
  config.Read("/Warnings/Profile-Invalid", &m_warnProfileInvalid, true);
}

void UserPreferences::SaveOptions(void) const
{
  wxConfig config("Gambit");

  config.Write("/Colors/Chance-Red", (long) m_chanceColor.Red());
  config.Write("/Colors/Chance-Green", (long) m_chanceColor.Green());
  config.Write("/Colors/Chance-Blue", (long) m_chanceColor.Blue());

  config.Write("/Colors/Terminal-Red", (long) m_terminalColor.Red());
  config.Write("/Colors/Terminal-Green", (long) m_terminalColor.Green());
  config.Write("/Colors/Terminal-Blue", (long) m_terminalColor.Blue());

  for (int i = 0; i < 8; i++) {
    gText playerString = "/Colors/Player" + ToText(i);
    config.Write((char *) (playerString + "-Red"), (long) m_playerColors[i].Red());
    config.Write((char *) (playerString + "-Green"), (long) m_playerColors[i].Green());
    config.Write((char *) (playerString + "-Blue"), (long) m_playerColors[i].Blue());
  }

  config.Write("/Warnings/Solve-Imperfect-Recall", m_warnSolveImperfect);
  config.Write("/Warnings/Support-Delete", m_warnSupportDelete);
  config.Write("/Warnings/Profile-Invalid", m_warnProfileInvalid);
}

const int idTERMINAL_COLOR = 3000;
const int idCHANCE_COLOR = 3001;
// Player buttons are numbered idCHANCE_COLOR+1..idCHANCE_COLOR+8

class ColorPanel : public wxPanel {
private:
  wxButton *m_chanceButton, *m_terminalButton, *m_playerButton[8];

public:
  ColorPanel(wxWindow *p_parent);

  wxColour GetTerminalColor(void) const
  { return m_terminalButton->GetBackgroundColour(); }
  wxColour GetChanceColor(void) const
  { return m_chanceButton->GetBackgroundColour(); }
  wxColour GetPlayerColor(int p_player) const
  { return m_playerButton[p_player-1]->GetBackgroundColour(); }

  void SetTerminalColor(const wxColour &p_color) const
  { m_terminalButton->SetBackgroundColour(p_color); }
  void SetChanceColor(const wxColour &p_color) const
  { m_chanceButton->SetBackgroundColour(p_color); }
  void SetPlayerColor(int p_player, const wxColour &p_color) const
  { m_playerButton[p_player-1]->SetBackgroundColour(p_color); }
};

ColorPanel::ColorPanel(wxWindow *p_parent)
  : wxPanel(p_parent, -1)
{
  SetAutoLayout(true);

  wxStaticBox *colorBox = new wxStaticBox(this, -1, "Player colors");
  wxStaticBoxSizer *colorBoxSizer = new wxStaticBoxSizer(colorBox, wxVERTICAL);
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 10, 10);

  gridSizer->Add(new wxStaticText(this, -1, "Chance player"), 0,
		 wxALIGN_CENTER, 0);
  m_chanceButton = new wxButton(this, idCHANCE_COLOR, "");
  m_chanceButton->SetBackgroundColour(*wxLIGHT_GREY);
  gridSizer->Add(m_chanceButton, 0, wxALIGN_CENTER, 0);
  
  gridSizer->Add(new wxStaticText(this, -1, "Terminal nodes"), 0,
		 wxALIGN_CENTER, 0);
  m_terminalButton = new wxButton(this, idTERMINAL_COLOR, "");
  m_terminalButton->SetBackgroundColour(*wxBLACK);
  gridSizer->Add(m_terminalButton, 0, wxALIGN_CENTER, 0);

  for (int pl = 1; pl <= 8; pl++) {
    gridSizer->Add(new wxStaticText(this, -1,
				    wxString::Format("Player %d", pl)),
		   0, wxALIGN_CENTER, 0);
    m_playerButton[pl-1] = new wxButton(this, idCHANCE_COLOR+pl, "");
    m_playerButton[pl-1]->SetBackgroundColour(*wxRED);
    gridSizer->Add(m_playerButton[pl-1], 0, wxALIGN_CENTER, 0);
  }

  colorBoxSizer->Add(gridSizer, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(colorBoxSizer, 0, wxALL, 5);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class WarningPanel : public wxPanel {
private:
  wxCheckBox *m_solveImperfect, *m_makeImperfect;
  wxCheckBox *m_deletingSupports, *m_invalidatingProfiles;

public:
  WarningPanel(wxWindow *p_parent);

  void SetWarnOnSolveImperfect(bool p_warn)
  { m_solveImperfect->SetValue(p_warn); }
  void SetWarnOnSupportDelete(bool p_warn)
  { m_deletingSupports->SetValue(p_warn); }
  void SetWarnOnInvalidateProfiles(bool p_warn)
  { m_invalidatingProfiles->SetValue(p_warn); }

  bool GetWarnOnSolveImperfect(void) const
  { return m_solveImperfect->GetValue(); }
  bool GetWarnOnSupportDelete(void) const
  { return m_deletingSupports->GetValue(); }
  bool GetWarnOnInvalidateProfiles(void) const
  { return m_invalidatingProfiles->GetValue(); }
};

WarningPanel::WarningPanel(wxWindow *p_parent)
  : wxPanel(p_parent, -1)
{
  SetAutoLayout(true);

  wxStaticBox *warningBox = new wxStaticBox(this, -1, "Warn me when");
  wxStaticBoxSizer *warningBoxSizer = new wxStaticBoxSizer(warningBox,
							   wxVERTICAL);

  m_solveImperfect = new wxCheckBox(this, -1,
				    "Solving a game of imperfect recall");
  warningBoxSizer->Add(m_solveImperfect, 0, wxALL, 5);

  m_makeImperfect = new wxCheckBox(this, -1,
				   "Making a game of imperfect recall");
  m_makeImperfect->Enable(false);
  warningBoxSizer->Add(m_makeImperfect, 0, wxALL, 5);

  m_deletingSupports = new wxCheckBox(this, -1,
				      "Changing the game requires all "
				      "supports to be deleted");
  warningBoxSizer->Add(m_deletingSupports, 0, wxALL, 5);

  m_invalidatingProfiles = new wxCheckBox(this, -1,
					  "Changing the game may invalidate "
					  "equilibrium properties of profiles");
  warningBoxSizer->Add(m_invalidatingProfiles, 0, wxALL, 5);
  
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(warningBoxSizer, 0, wxALL, 5);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class dialogPreferences : public wxDialog {
private:
  ColorPanel *m_colorPanel;
  WarningPanel *m_warningPanel;

  // Event handlers
  void OnTerminalColor(wxCommandEvent &);
  void OnChanceColor(wxCommandEvent &);
  void OnPlayerColor(wxCommandEvent &);

public:
  dialogPreferences(wxWindow *p_parent);
  virtual ~dialogPreferences() { }

  ColorPanel *GetColorPanel(void) const { return m_colorPanel; }
  WarningPanel *GetWarningPanel(void) const { return m_warningPanel; }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogPreferences, wxDialog)
  EVT_BUTTON(idTERMINAL_COLOR, dialogPreferences::OnTerminalColor)
  EVT_BUTTON(idCHANCE_COLOR, dialogPreferences::OnChanceColor)
  EVT_BUTTON(idCHANCE_COLOR+1, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+2, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+3, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+4, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+5, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+6, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+7, dialogPreferences::OnPlayerColor)
  EVT_BUTTON(idCHANCE_COLOR+8, dialogPreferences::OnPlayerColor)
END_EVENT_TABLE()

dialogPreferences::dialogPreferences(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "Configure Preferences")
{
  wxNotebook *notebook = new wxNotebook(this, -1);

  m_colorPanel = new ColorPanel(notebook);
  notebook->AddPage(m_colorPanel, "Colors");

  m_warningPanel = new WarningPanel(notebook);
  notebook->AddPage(m_warningPanel, "Warnings");

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebook, 0, wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void dialogPreferences::OnTerminalColor(wxCommandEvent &)
{
  wxColourData data;
  wxColour color = m_colorPanel->GetTerminalColor();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle("Choose Terminal Color");
  if (dialog.ShowModal() == wxID_OK) {
    m_colorPanel->SetTerminalColor(dialog.GetColourData().GetColour());
  }
}

void dialogPreferences::OnChanceColor(wxCommandEvent &)
{
  wxColourData data;
  wxColour color = m_colorPanel->GetChanceColor();
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle("Choose Chance Color");
  if (dialog.ShowModal() == wxID_OK) {
    m_colorPanel->SetChanceColor(dialog.GetColourData().GetColour());
  }
}

void dialogPreferences::OnPlayerColor(wxCommandEvent &p_event)
{
  int player = p_event.GetId() - idCHANCE_COLOR;
  wxColourData data;
  wxColour color = m_colorPanel->GetPlayerColor(player);
  data.SetColour(color);

  wxColourDialog dialog(this, &data);
  dialog.SetTitle(wxString::Format("Choose Player %d Color", player));
  if (dialog.ShowModal() == wxID_OK) {
    m_colorPanel->SetPlayerColor(player, dialog.GetColourData().GetColour());
  }
}

void UserPreferences::EditOptions(wxWindow *p_parent)
{
  dialogPreferences dialog(p_parent);
  dialog.GetColorPanel()->SetTerminalColor(m_terminalColor);
  dialog.GetColorPanel()->SetChanceColor(m_chanceColor);
  for (int pl = 1; pl <= 8; pl++) {
    dialog.GetColorPanel()->SetPlayerColor(pl, m_playerColors[pl-1]);
  }

  dialog.GetWarningPanel()->SetWarnOnSolveImperfect(m_warnSolveImperfect);
  dialog.GetWarningPanel()->SetWarnOnSupportDelete(m_warnSupportDelete);
  dialog.GetWarningPanel()->SetWarnOnInvalidateProfiles(m_warnProfileInvalid);

  if (dialog.ShowModal() == wxID_OK) {
    m_terminalColor = dialog.GetColorPanel()->GetTerminalColor();
    m_chanceColor = dialog.GetColorPanel()->GetChanceColor();
    for (int pl = 1; pl <= 8; pl++) {
      m_playerColors[pl-1] = dialog.GetColorPanel()->GetPlayerColor(pl);
    }

    m_warnSolveImperfect = dialog.GetWarningPanel()->GetWarnOnSolveImperfect();
    m_warnSupportDelete = dialog.GetWarningPanel()->GetWarnOnSupportDelete();
    m_warnProfileInvalid = dialog.GetWarningPanel()->GetWarnOnInvalidateProfiles();
    SaveOptions();
  }

}


