//
// FILE: nfgpanel.cc -- Implementation of normal form navigation panel
//
// $Id$
//

#include "wx/wx.h"
#include "nfgshow.h"
#include "nfgpanel.h"

BEGIN_EVENT_TABLE(NfgPanel, wxPanel)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgPanel::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgPanel::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgPanel::OnColPlayerChange)
END_EVENT_TABLE()

NfgPanel::NfgPanel(NfgShow *p_parent, wxWindow *p_window)
  : wxPanel(p_window, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_parent)
{
  SetAutoLayout(true);
  m_rowChoice = new wxChoice(this, idROWPLAYER_CHOICE);
  m_colChoice = new wxChoice(this, idCOLPLAYER_CHOICE);

  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    m_parent->Game().Players()[pl]->GetName());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
  }

  m_rowChoice->SetSelection(0);
  m_colChoice->SetSelection(1);

  wxStaticText *rowLabel = new wxStaticText(this, -1, "Row player");
  wxStaticText *columnLabel = new wxStaticText(this, -1, "Column player");

  rowLabel->SetConstraints(new wxLayoutConstraints);
  rowLabel->GetConstraints()->centreY.SameAs(m_rowChoice, wxCentreY);
  rowLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  rowLabel->GetConstraints()->width.SameAs(columnLabel, wxWidth);
  rowLabel->GetConstraints()->height.AsIs();
  
  m_rowChoice->SetConstraints(new wxLayoutConstraints);
  m_rowChoice->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_rowChoice->GetConstraints()->left.SameAs(rowLabel, wxRight, 10);
  m_rowChoice->GetConstraints()->width.AsIs();
  m_rowChoice->GetConstraints()->height.AsIs();

  columnLabel->SetConstraints(new wxLayoutConstraints);
  columnLabel->GetConstraints()->centreY.SameAs(m_colChoice, wxCentreY);
  columnLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  columnLabel->GetConstraints()->width.AsIs();
  columnLabel->GetConstraints()->height.AsIs();

  m_colChoice->SetConstraints(new wxLayoutConstraints);
  m_colChoice->GetConstraints()->top.SameAs(m_rowChoice, wxBottom);
  m_colChoice->GetConstraints()->left.SameAs(columnLabel, wxRight, 10);
  m_colChoice->GetConstraints()->width.AsIs();
  m_colChoice->GetConstraints()->height.AsIs();

  m_stratProfile = new wxChoice *[m_parent->Game().NumPlayers()];
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    NFPlayer *player = m_parent->Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					     player->Strategies()[st]->Name()));
    }
    m_stratProfile[pl-1]->SetSelection(0);

    wxStaticText *profileLabel = new wxStaticText(this, -1,
						  (char *) ("Player " + ToText(pl)));

    profileLabel->SetConstraints(new wxLayoutConstraints);
    profileLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
    profileLabel->GetConstraints()->centreY.SameAs(m_stratProfile[pl-1],
						   wxCentreY);
    profileLabel->GetConstraints()->width.SameAs(columnLabel, wxWidth);
    profileLabel->GetConstraints()->height.AsIs();

    m_stratProfile[pl-1]->SetConstraints(new wxLayoutConstraints);
    m_stratProfile[pl-1]->GetConstraints()->left.SameAs(m_rowChoice, wxLeft);
    if (pl > 1) {
      m_stratProfile[pl-1]->GetConstraints()->top.SameAs(m_stratProfile[pl-2],
							 wxBottom);
    }
    else {
      m_stratProfile[pl-1]->GetConstraints()->top.SameAs(m_colChoice,
							 wxBottom, 10);
    }
    m_stratProfile[pl-1]->GetConstraints()->width.AsIs();
    m_stratProfile[pl-1]->GetConstraints()->height.AsIs();
  }

  Layout();
  Show(true);
}

void NfgPanel::SetProfile(const gArray<int> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stratProfile[i-1]->SetSelection(p_profile[i] - 1);
  }
}

gArray<int> NfgPanel::GetProfile(void) const
{
  gArray<int> profile(m_parent->Game().NumPlayers());
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = m_stratProfile[i-1]->GetSelection() + 1;
  }
  return profile;
}

void NfgPanel::SetStrategy(int p_player, int p_strategy)
{
  m_stratProfile[p_player-1]->SetSelection(p_strategy-1);
}

void NfgPanel::OnStrategyChange(wxCommandEvent &)
{
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_parent->SetStrategy(pl, m_stratProfile[pl-1]->GetSelection() + 1);
  }
}

void NfgPanel::OnRowPlayerChange(wxCommandEvent &)
{
  int oldRowPlayer = m_parent->GetRowPlayer();
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

void NfgPanel::OnColPlayerChange(wxCommandEvent &)
{
  int oldColPlayer = m_parent->GetColPlayer();
  int newColPlayer = m_colChoice->GetSelection() + 1;

  if (newColPlayer == oldColPlayer) {
    return;
  }

  if (newColPlayer == m_rowChoice->GetSelection() + 1) {
    m_rowChoice->SetSelection(oldColPlayer - 1);
    m_parent->SetPlayers(oldColPlayer, newColPlayer);
  }
  else {
    m_parent->SetPlayers(m_colChoice->GetSelection() + 1, newColPlayer);
  }
}

void NfgPanel::SetSupport(const NFSupport &p_support)
{
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1]->Clear();
    NFPlayer *player = m_parent->Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      if (p_support.Find(player->Strategies()[st])) {
	m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					       player->Strategies()[st]->Name()));
      }
    }
    m_stratProfile[pl-1]->SetSelection(0);
  }
}

