//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to inspect and edit normal form strategies
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "game/nfg.h"
#include "game/nfplayer.h"
#include "game/nfstrat.h"
#include "dlnfgstrategies.h"

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

const int idCHOICE_PLAYER = 2000;
const int idLISTBOX_STRATEGIES = 2001;

BEGIN_EVENT_TABLE(dialogStrategies, wxDialog)
  EVT_CHOICE(idCHOICE_PLAYER, dialogStrategies::OnPlayerChanged)
  EVT_LISTBOX(idLISTBOX_STRATEGIES, dialogStrategies::OnStrategyChanged)
  EVT_BUTTON(wxID_OK, dialogStrategies::OnOK)
END_EVENT_TABLE()

dialogStrategies::dialogStrategies(wxWindow *p_parent, const Nfg &p_nfg)
  : wxDialog(p_parent, -1, "Strategies"), 
    m_nfg(p_nfg), m_lastStrategy(0)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Strategies for player"),
		   0, wxALL, 5);
  m_player = new wxChoice(this, idCHOICE_PLAYER);
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    NFPlayer *player = p_nfg.Players()[pl];
    m_player->Append(wxString::Format("%d: %s", pl,
				      (char *) player->GetName())); 
    m_strategyNames.Append(gArray<gText>(player->NumStrats()));
    for (int st = 1; st <= player->NumStrats(); st++) {
      m_strategyNames[pl][st] = player->Strategies()[st]->Name();
    }
  } 
  m_player->SetSelection(0);
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *strategyBoxSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Strategies"),
			 wxHORIZONTAL);
  m_strategyList = new wxListBox(this, idLISTBOX_STRATEGIES);
  for (int st = 1; st <= m_nfg.Players()[1]->NumStrats(); st++) {
    m_strategyList->Append(wxString::Format("%d: %s", st,
					    (char *) m_strategyNames[1][st]));
  }
  m_strategyList->SetSelection(0);
  strategyBoxSizer->Add(m_strategyList, 0, wxALL, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxVERTICAL);
  editSizer->Add(new wxStaticText(this, wxID_STATIC, "Strategy name"),
		 0, wxALL | wxCENTER, 5);
  m_strategyName = new wxTextCtrl(this, -1, (char *) m_strategyNames[1][1]);
  editSizer->Add(m_strategyName, 0, wxALL | wxCENTER, 5);
  strategyBoxSizer->Add(editSizer, 0, wxALL, 5);

  topSizer->Add(strategyBoxSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

void dialogStrategies::OnPlayerChanged(wxCommandEvent &)
{
  m_strategyList->Clear();
  int player = m_player->GetSelection() + 1;
  for (int st = 1; st <= m_strategyNames[player].Length(); st++) {
    m_strategyList->Append(wxString::Format("%d: %s", st,
					    (char *) m_strategyNames[player][st]));
  }
  m_strategyList->SetSelection(0);
  m_strategyName->SetValue((char *) m_strategyNames[player][1]);
  m_lastStrategy = 0;
}

void dialogStrategies::OnStrategyChanged(wxCommandEvent &)
{
  int player = m_player->GetSelection() + 1;
  m_strategyNames[player][m_lastStrategy+1] =
    m_strategyName->GetValue().c_str();
  m_strategyList->SetString(m_lastStrategy,
			    wxString::Format("%d: %s", m_lastStrategy + 1,
					     m_strategyName->GetValue().c_str()));
  m_lastStrategy = m_strategyList->GetSelection();
  m_strategyName->SetValue((char *) m_strategyNames[player][m_lastStrategy+1]);
}

void dialogStrategies::OnOK(wxCommandEvent &p_event)
{
  // Copy any edited data into the blocks
  int player = m_player->GetSelection() + 1;
  m_strategyNames[player][m_lastStrategy+1] =
    m_strategyName->GetValue().c_str();
  // Go on with usual processing
  p_event.Skip();
}
