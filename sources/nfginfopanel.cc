//
// FILE: nfginfopanel.cc -- Implementation of normal form information panel
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

#include "guiapp.h"
#include "nfgview.h"
#include "nfginfopanel.h"

const int idSTRATEGY_CHOICE = 1000;
const int idROWPLAYER_CHOICE = 1001;
const int idCOLPLAYER_CHOICE = 1002;
const int idPLAYER_NAME = 1003;
const int idOUTCOME_CHOICE = 1004;
const int idSUPPORT_CHOICE = 1005;

BEGIN_EVENT_TABLE(guiNfgInfoPanel, wxPanel)
  EVT_CHOICE(idSTRATEGY_CHOICE, guiNfgInfoPanel::OnStrategy)
  EVT_CHOICE(idROWPLAYER_CHOICE, guiNfgInfoPanel::OnRowPlayer)
  EVT_CHOICE(idCOLPLAYER_CHOICE, guiNfgInfoPanel::OnColPlayer)
  EVT_CHOICE(idOUTCOME_CHOICE, guiNfgInfoPanel::OnOutcome)
  EVT_CHOICE(idSUPPORT_CHOICE, guiNfgInfoPanel::OnSupport)
END_EVENT_TABLE()

guiNfgInfoPanel::guiNfgInfoPanel(guiNfgView *p_parent, Nfg &p_nfg)
  : wxPanel(p_parent, -1),
    m_nfg(p_nfg), m_parent(p_parent),
    m_strategy(p_nfg.NumPlayers()), m_playerNames(p_nfg.NumPlayers()),
    m_payoffs(p_nfg.NumPlayers())
{
  SetAutoLayout(TRUE);

  wxStaticText *supportLabel = new wxStaticText(this, -1, "Support");
  m_support = new wxChoice(this, idSUPPORT_CHOICE);

  supportLabel->SetConstraints(new wxLayoutConstraints);
  supportLabel->GetConstraints()->top.SameAs(this, wxTop, 10);
  supportLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  supportLabel->GetConstraints()->width.AsIs();
  supportLabel->GetConstraints()->height.AsIs();

  m_support->SetConstraints(new wxLayoutConstraints);
  m_support->GetConstraints()->centreY.SameAs(supportLabel, wxCentreY);
  m_support->GetConstraints()->left.SameAs(supportLabel, wxRight, 10);
  m_support->GetConstraints()->width.PercentOf(supportLabel, wxWidth, 200);
  m_support->GetConstraints()->height.AsIs();

  wxStaticText *rowPlayerLabel = new wxStaticText(this, -1, "Row player");
  m_rowPlayer = new wxChoice(this, idROWPLAYER_CHOICE);
  wxStaticText *colPlayerLabel = new wxStaticText(this, -1, "Column player");
  m_colPlayer = new wxChoice(this, idCOLPLAYER_CHOICE);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    wxString entry = (char *) (ToText(pl) + ": " +
			       m_nfg.Players()[pl]->GetName());
    m_rowPlayer->Append(entry);
    m_colPlayer->Append(entry);
  }
  m_rowPlayer->SetSelection(0);
  m_colPlayer->SetSelection(1);

  rowPlayerLabel->SetConstraints(new wxLayoutConstraints);
  rowPlayerLabel->GetConstraints()->top.SameAs(supportLabel, wxBottom, 10);
  rowPlayerLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  rowPlayerLabel->GetConstraints()->width.AsIs();
  rowPlayerLabel->GetConstraints()->height.AsIs();

  m_rowPlayer->SetConstraints(new wxLayoutConstraints);
  m_rowPlayer->GetConstraints()->centreY.SameAs(rowPlayerLabel, wxCentreY);
  m_rowPlayer->GetConstraints()->left.SameAs(colPlayerLabel, wxRight, 10);
  m_rowPlayer->GetConstraints()->width.SameAs(colPlayerLabel, wxWidth);
  m_rowPlayer->GetConstraints()->height.AsIs();

  colPlayerLabel->SetConstraints(new wxLayoutConstraints);
  colPlayerLabel->GetConstraints()->centreY.SameAs(m_colPlayer, wxCentreY);
  colPlayerLabel->GetConstraints()->left.SameAs(rowPlayerLabel, wxLeft);
  colPlayerLabel->GetConstraints()->width.AsIs();
  colPlayerLabel->GetConstraints()->height.AsIs();

  m_colPlayer->SetConstraints(new wxLayoutConstraints);
  m_colPlayer->GetConstraints()->top.SameAs(m_rowPlayer, wxBottom);
  m_colPlayer->GetConstraints()->left.SameAs(colPlayerLabel, wxRight, 10);
  m_colPlayer->GetConstraints()->width.SameAs(colPlayerLabel, wxWidth);
  m_colPlayer->GetConstraints()->height.AsIs();

  m_outcome = new wxChoice(this, idOUTCOME_CHOICE);
  m_outcome->Append("(null)");
  for (int outc = 1; outc <= m_nfg.NumOutcomes(); outc++) {
    m_outcome->Append((char *) (ToText(outc) + ": " +
				m_nfg.Outcomes()[outc]->GetName()));
  }
  m_outcome->SetConstraints(new wxLayoutConstraints);
  m_outcome->GetConstraints()->top.SameAs(m_colPlayer, wxBottom, 10);
  m_outcome->GetConstraints()->height.AsIs();
						      
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerNames[pl] = new wxTextCtrl(this, idPLAYER_NAME);
    m_playerNames[pl]->SetValue((char *) m_nfg.Players()[pl]->GetName()); 

    m_strategy[pl] = new wxChoice(this, idSTRATEGY_CHOICE);
    for (int st = 1; st <= m_nfg.NumStrats(pl); st++) {
      m_strategy[pl]->Append((char *) (ToText(st) + ": " + 
				       m_nfg.Strategies(pl)[st]->Name()));
    }

    m_payoffs[pl] = new wxTextCtrl(this, -1);

    m_playerNames[pl]->SetConstraints(new wxLayoutConstraints);
    m_strategy[pl]->SetConstraints(new wxLayoutConstraints);
    m_payoffs[pl]->SetConstraints(new wxLayoutConstraints);
    if (pl == 1) {
      m_playerNames[pl]->GetConstraints()->top.SameAs(m_outcome, wxBottom);
    }
    else {
      m_playerNames[pl]->GetConstraints()->top.SameAs(m_playerNames[pl-1],
						      wxBottom);
    }
    m_strategy[pl]->GetConstraints()->top.SameAs(m_playerNames[pl], wxTop);
    m_payoffs[pl]->GetConstraints()->top.SameAs(m_playerNames[pl], wxTop);

    m_playerNames[pl]->GetConstraints()->left.SameAs(this, wxLeft, 10);
    m_strategy[pl]->GetConstraints()->left.SameAs(m_playerNames[pl],
						  wxRight, 3);
    m_payoffs[pl]->GetConstraints()->left.SameAs(m_strategy[pl], wxRight);

    int x, y;
    m_playerNames[pl]->GetTextExtent("WWWWWWWW", &x, &y);
    m_playerNames[pl]->GetConstraints()->width.Absolute(x);
    m_strategy[pl]->GetConstraints()->width.Absolute(x);
    m_payoffs[pl]->GetConstraints()->width.Absolute(x);

    m_playerNames[pl]->GetConstraints()->height.SameAs(m_strategy[pl],
						       wxHeight);
    m_strategy[pl]->GetConstraints()->height.AsIs();
    m_payoffs[pl]->GetConstraints()->height.SameAs(m_strategy[pl], wxHeight);
  }

  m_outcome->GetConstraints()->width.SameAs(m_payoffs[1], wxWidth);
  m_outcome->GetConstraints()->left.SameAs(m_payoffs[1], wxLeft);

  Layout();
  Show(TRUE);
}

void guiNfgInfoPanel::OnRowPlayer(wxCommandEvent &)
{
  if (m_rowPlayer->GetSelection() == m_colPlayer->GetSelection()) {
    m_colPlayer->SetSelection(m_parent->RowPlayer() - 1);
  }
  m_parent->SetPlayers(m_rowPlayer->GetSelection() + 1,
		       m_colPlayer->GetSelection() + 1);
}

void guiNfgInfoPanel::OnColPlayer(wxCommandEvent &)
{
  if (m_rowPlayer->GetSelection() == m_colPlayer->GetSelection()) {
    m_rowPlayer->SetSelection(m_parent->ColPlayer() - 1);
  }
  m_parent->SetPlayers(m_rowPlayer->GetSelection() + 1,
		       m_colPlayer->GetSelection() + 1);
}

void guiNfgInfoPanel::OnStrategy(wxCommandEvent &)
{
  gArray<int> profile(m_nfg.NumPlayers());
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    profile[pl] = m_strategy[pl]->GetSelection() + 1;
  }
  m_parent->SetProfile(profile);
}

void guiNfgInfoPanel::OnOutcome(wxCommandEvent &)
{
  if (m_outcome->GetSelection() > 0) {
    m_parent->SetOutcome(m_nfg.Outcomes()[m_outcome->GetSelection()]);
  }
  else {
    m_parent->SetOutcome(0);
  }
}

void guiNfgInfoPanel::OnSupport(wxCommandEvent &)
{
  m_parent->SetCurrentSupport(m_support->GetSelection() + 1);
}

void guiNfgInfoPanel::SetProfile(const gArray<int> &p_profile)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_strategy[pl]->SetSelection(p_profile[pl] - 1);
  }

  NFOutcome *outcome = m_nfg.GetOutcome(p_profile);
  if (outcome) {
    m_outcome->SetSelection(outcome->GetNumber());
    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
      m_payoffs[pl]->SetValue((char *) ToText(m_nfg.Payoff(outcome, pl)));
      m_payoffs[pl]->Enable(TRUE);
    }
  }
  else {
    m_outcome->SetSelection(0);
    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
      m_payoffs[pl]->SetValue("0");
      m_payoffs[pl]->Enable(FALSE);
    }
  }    
}

void guiNfgInfoPanel::SetLabels(void)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerNames[pl]->SetValue((char *) m_nfg.Players()[pl]->GetName());
  }
}

void guiNfgInfoPanel::SetOutcome(NFOutcome *p_outcome)
{
  if (p_outcome) {
    m_outcome->SetSelection(p_outcome->GetNumber());
    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
      m_payoffs[pl]->SetValue((char *) ToText(m_nfg.Payoff(p_outcome, pl)));
      m_payoffs[pl]->Enable(TRUE);
    }
  }
  else {
    m_outcome->SetSelection(0);
    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
      m_payoffs[pl]->SetValue("0");
      m_payoffs[pl]->Enable(FALSE);
    }
  }
}

void guiNfgInfoPanel::SetSupport(NFSupport *p_support)
{
  int index = m_parent->Supports().Find(p_support);
  m_support->SetSelection(index - 1);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_strategy[pl]->Clear();
    for (int st = 1; st <= p_support->NumStrats(pl); st++) {
      m_strategy[pl]->Append((char *) (ToText(st) + ": " + 
				       p_support->Strategies(pl)[st]->Name()));
    }
    m_strategy[pl]->SetSelection(0);
  }
}

void guiNfgInfoPanel::UpdateSupports(const gList<NFSupport *> &p_supports)
{
  m_support->Clear();

  for (int i = 1; i <= p_supports.Length(); i++) {
    m_support->Append((char *) (ToText(i) + ": " +
				p_supports[i]->GetName()));
  }
}

gText guiNfgInfoPanel::GetPlayerName(int pl) const
{
  return m_playerNames[pl]->GetValue().c_str();
}

gNumber guiNfgInfoPanel::GetPayoff(int pl) const
{
  return ToNumber(m_payoffs[pl]->GetValue().c_str());
}
