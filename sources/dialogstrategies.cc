//
// FILE: dialogstrategies.cc -- Implementation of strategy edit dialog
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

#include "nfg.h"
#include "dialogauto.h"
#include "dialogstrategies.h"

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

const int idSTRATEGIES_PLAYERLIST = 1001;
const int idSTRATEGIES_PLAYERNAME = 1002;
const int idSTRATEGIES_STRATEGYLIST = 1003;
const int idSTRATEGIES_STRATEGYNAME = 1004;

BEGIN_EVENT_TABLE(dialogStrategies, wxDialog)
  EVT_LISTBOX(idSTRATEGIES_PLAYERLIST, dialogStrategies::OnSelectPlayer)
  EVT_TEXT(idSTRATEGIES_PLAYERNAME, dialogStrategies::OnEditPlayer)
  EVT_LISTBOX(idSTRATEGIES_STRATEGYLIST, dialogStrategies::OnSelectStrategy)
  EVT_TEXT(idSTRATEGIES_STRATEGYNAME, dialogStrategies::OnEditStrategy)
END_EVENT_TABLE()

dialogStrategies::dialogStrategies(wxWindow *p_parent, Nfg &p_nfg)
  : guiAutoDialog(p_parent, "Strategy Information"), 
    m_nfg(p_nfg)
{
  m_players = new wxListBox(this, idSTRATEGIES_PLAYERLIST); 
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_players->Append((char *) (ToText(pl) + ": " + 
				m_nfg.Players()[pl]->GetName()));
  }

  m_players->SetConstraints(new wxLayoutConstraints);
  m_players->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_players->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_players->GetConstraints()->width.AsIs();
  m_players->GetConstraints()->height.AsIs();
  m_players->SetSelection(0);

  m_playerName = new wxTextCtrl(this, idSTRATEGIES_PLAYERNAME);
  m_playerName->SetValue((char *) m_nfg.Players()[1]->GetName());
  m_playerName->SetConstraints(new wxLayoutConstraints);
  m_playerName->GetConstraints()->top.SameAs(m_players, wxBottom, 10);
  m_playerName->GetConstraints()->left.SameAs(m_players, wxLeft);
  m_playerName->GetConstraints()->width.SameAs(m_players, wxWidth);
  m_playerName->GetConstraints()->height.AsIs();

  m_strategies = new wxListBox(this, idSTRATEGIES_STRATEGYLIST);
  for (int st = 1; st <= m_nfg.NumStrats(1); st++) {
    m_strategies->Append((char *) (ToText(st) + ": " +
				   m_nfg.Strategies(1)[st]->Name()));
  }
  m_strategies->SetConstraints(new wxLayoutConstraints);
  m_strategies->GetConstraints()->top.SameAs(m_players, wxTop);
  m_strategies->GetConstraints()->left.SameAs(m_players, wxRight, 10);
  m_strategies->GetConstraints()->width.AsIs();
  m_strategies->GetConstraints()->height.AsIs();
  m_strategies->SetSelection(0);

  m_strategyName = new wxTextCtrl(this, idSTRATEGIES_STRATEGYNAME);
  m_strategyName->SetValue((char *) m_nfg.Strategies(1)[1]->Name());
  m_strategyName->SetConstraints(new wxLayoutConstraints);
  m_strategyName->GetConstraints()->top.SameAs(m_strategies, wxBottom, 10);
  m_strategyName->GetConstraints()->left.SameAs(m_strategies, wxLeft);
  m_strategyName->GetConstraints()->width.SameAs(m_strategies, wxWidth);
  m_strategyName->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_playerName, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_helpButton, wxWidth);

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.AsIs();

  m_cancelButton->Show(FALSE);
  m_cancelButton->GetConstraints()->top.SameAs(this, wxTop);
  m_cancelButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  AutoSize();
}

void dialogStrategies::OnSelectPlayer(void)
{
  int selection = m_players->GetSelection();
  m_playerName->SetValue((char *) m_nfg.Players()[selection + 1]->GetName());

  m_strategies->Clear();
  for (int st = 1; st <= m_nfg.NumStrats(selection + 1); st++) {
    m_strategies->Append((char *) (ToText(st) + ": " +
				   m_nfg.Strategies(selection+1)[st]->Name()));
  }
  m_strategies->SetSelection(0);
  m_strategyName->SetValue((char *) m_nfg.Strategies(selection+1)[1]->Name());
}

void dialogStrategies::OnEditPlayer(void)
{
  int selection = m_players->GetSelection();
  m_players->SetString(selection, (char *) (ToText(selection + 1) + ": " +
					       m_playerName->GetValue().c_str()));
  m_nfg.Players()[selection+1]->SetName(m_playerName->GetValue().c_str());
}

void dialogStrategies::OnSelectStrategy(void)
{
  int player = m_players->GetSelection() + 1;
  int strategy = m_strategies->GetSelection() + 1;
  m_strategyName->SetValue((char *) m_nfg.Strategies(player)[strategy]->Name());
}

void dialogStrategies::OnEditStrategy(void)
{
  int player = m_players->GetSelection() + 1;
  int strategy = m_strategies->GetSelection() + 1;
  m_strategies->SetString(strategy-1, (char *) (ToText(strategy) + ": " +
						m_strategyName->GetValue().c_str()));
  m_nfg.Strategies(player)[strategy]->SetName(m_strategyName->GetValue().c_str());
}


