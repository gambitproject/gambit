//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form navigation window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif // WX_PRECOMP
#include "nfgnavigate.h"

BEGIN_EVENT_TABLE(NfgNavigateWindow, wxPanel)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgNavigateWindow::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgNavigateWindow::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgNavigateWindow::OnColPlayerChange)
END_EVENT_TABLE()

NfgNavigateWindow::NfgNavigateWindow(NfgShow *p_nfgShow, wxWindow *p_parent)
  : wxPanel(p_parent, -1), 
    m_parent(p_nfgShow), m_rowPlayer(1), m_colPlayer(2),
    m_support(p_nfgShow->Game())
{
  const Nfg &nfg = p_nfgShow->Game();

  wxStaticBoxSizer *playerViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "View players"),
			 wxVERTICAL);

  wxBoxSizer *rowChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  rowChoiceSizer->Add(new wxStaticText(this, -1, "Row player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_rowChoice = new wxChoice(this, idROWPLAYER_CHOICE);
  rowChoiceSizer->Add(m_rowChoice, 0, wxALL, 0);

  wxBoxSizer *colChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  colChoiceSizer->Add(new wxStaticText(this, -1, "Column player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_colChoice = new wxChoice(this, idCOLPLAYER_CHOICE);
  colChoiceSizer->Add(m_colChoice, 0, wxALL, 0);

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    nfg.Players()[pl]->GetName());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
  }

  m_rowChoice->SetSelection(0);
  m_colChoice->SetSelection(1);

  playerViewSizer->Add(rowChoiceSizer, 0, wxALL | wxEXPAND, 5);
  playerViewSizer->Add(colChoiceSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *contViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Current contingency"),
			 wxVERTICAL);

  m_playerNames = new wxStaticText *[nfg.NumPlayers()];
  m_stratProfile = new wxChoice *[nfg.NumPlayers()];
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    NFPlayer *player = nfg.Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					     player->Strategies()[st]->Name()));
    }
    m_stratProfile[pl-1]->SetSelection(0);

    wxBoxSizer *stratSizer = new wxBoxSizer(wxHORIZONTAL);
    if (player->GetName() != "") {
      m_playerNames[pl-1] = new wxStaticText(this, wxID_STATIC,
					     (char *) player->GetName());
    }
    else {
      m_playerNames[pl-1] = new wxStaticText(this, wxID_STATIC,
					     wxString::Format("Player %d",
							      pl));
    }
    stratSizer->Add(m_playerNames[pl-1], 1, wxALIGN_LEFT | wxRIGHT, 5);
    stratSizer->Add(m_stratProfile[pl-1], 0, wxALL, 0);
    contViewSizer->Add(stratSizer, 0, wxALL | wxEXPAND, 5);
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(playerViewSizer, 0, wxALL | wxEXPAND, 10);
  topSizer->Add(contViewSizer, 0, wxALL | wxEXPAND, 10);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  Show(true);
}

NfgNavigateWindow::~NfgNavigateWindow()
{
  delete [] m_playerNames;
  delete [] m_stratProfile;
}

void NfgNavigateWindow::SetProfile(const gArray<int> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stratProfile[i-1]->SetSelection(p_profile[i] - 1);
  }
}

gArray<int> NfgNavigateWindow::GetProfile(void) const
{
  gArray<int> profile(m_support.Game().NumPlayers());
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = m_stratProfile[i-1]->GetSelection() + 1;
  }
  return profile;
}

void NfgNavigateWindow::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  SetStrategy(p_rowPlayer, 1);
  SetStrategy(p_colPlayer, 1);
}

void NfgNavigateWindow::SetStrategy(int p_player, int p_strategy)
{
  m_stratProfile[p_player-1]->SetSelection(p_strategy-1);
}

void NfgNavigateWindow::SetSupport(const NFSupport &p_support)
{
  m_support = p_support;

  for (int pl = 1; pl <= m_support.Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1]->Clear();
    NFPlayer *player = m_support.Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      if (m_support.Find(player->Strategies()[st])) {
	m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					       player->Strategies()[st]->Name()));
      }
    }
    m_stratProfile[pl-1]->SetSelection(0);
  }
  SetPlayers(m_rowPlayer, m_colPlayer);
}

void NfgNavigateWindow::OnStrategyChange(wxCommandEvent &)
{
  m_parent->SetProfile(GetProfile());
}

void NfgNavigateWindow::OnRowPlayerChange(wxCommandEvent &)
{
  int oldRowPlayer = GetRowPlayer();
  int newRowPlayer = m_rowChoice->GetSelection() + 1;

  if (newRowPlayer == oldRowPlayer) {
    return;
  }

  if (newRowPlayer == m_colChoice->GetSelection() + 1) {
    m_colChoice->SetSelection(oldRowPlayer - 1);
    m_parent->SetPlayers(newRowPlayer, oldRowPlayer);
  }
  else {
    m_parent->SetPlayers(newRowPlayer, m_colChoice->GetSelection() + 1);
  }
}

void NfgNavigateWindow::OnColPlayerChange(wxCommandEvent &)
{
  int oldColPlayer = GetColPlayer();
  int newColPlayer = m_colChoice->GetSelection() + 1;

  if (newColPlayer == oldColPlayer) {
    return;
  }

  if (newColPlayer == m_rowChoice->GetSelection() + 1) {
    m_rowChoice->SetSelection(oldColPlayer - 1);
    m_parent->SetPlayers(oldColPlayer, newColPlayer);
  }
  else {
    m_parent->SetPlayers(m_rowChoice->GetSelection() + 1, newColPlayer);
  }
}

void NfgNavigateWindow::UpdateLabels(void)
{
  const Nfg &nfg = m_parent->Game();

  int rowSelection = m_rowChoice->GetSelection();
  int colSelection = m_colChoice->GetSelection();
  m_rowChoice->Clear();
  m_colChoice->Clear();
  
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    nfg.Players()[pl]->GetName());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
    if (nfg.Players()[pl]->GetName() != "") {
      m_playerNames[pl-1]->SetLabel((char *) nfg.Players()[pl]->GetName());
    }
    else {
      m_playerNames[pl-1]->SetLabel(wxString::Format("Player %d", pl));
    }
  }

  m_rowChoice->SetSelection(rowSelection);
  m_colChoice->SetSelection(colSelection);
}
