//
// FILE: dlnfg.cc -- Normal form-related dialog implementations
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"

#include "game/nfg.h"
#include "game/nfplayer.h"
#include "game/nfstrat.h"

#include "dlstrategies.h"
#include "dlnfgeditsupport.h"

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogStrategies, guiAutoDialog)
  EVT_LISTBOX(idSTRATEGY_PLAYER_LISTBOX, dialogStrategies::OnPlayer)
  EVT_LISTBOX(idSTRATEGY_STRATEGY_LISTBOX, dialogStrategies::OnStrategy)
END_EVENT_TABLE()

dialogStrategies::dialogStrategies(Nfg &p_nfg, wxFrame *p_parent)
  : guiAutoDialog(p_parent, "Strategy Information"), 
    m_nfg(p_nfg), m_gameChanged(false), m_prevStrategy(0)
{
  m_playerItem = new wxListBox(this, idSTRATEGY_PLAYER_LISTBOX);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + 
			 m_nfg.Players()[pl]->GetName()));
  }
  m_strategyItem = new wxListBox(this, idSTRATEGY_STRATEGY_LISTBOX);

  m_strategyNameItem = new wxTextCtrl(this, -1,"",
				      wxDefaultPosition, wxDefaultSize,
				      wxTE_READONLY);

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *midSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);

  leftSizer->Add(new wxStaticText(this, -1, "Player:"), 0, wxCENTRE | wxALL, 5);
  leftSizer->Add(m_playerItem, 0, wxCENTRE | wxALL, 5);

  midSizer->Add(new wxStaticText(this, -1, "Strategies:"), 0, wxCENTRE | wxALL, 5);
  midSizer->Add(m_strategyItem, 0, wxCENTRE | wxALL, 5);

  rightSizer->Add(new wxStaticText(this, -1, "StrategyName:"), 0, wxCENTRE | wxALL, 5);
  rightSizer->Add(m_strategyNameItem, 0, wxCENTRE | wxALL, 5);

  topSizer->Add(leftSizer, 0, wxALL, 5);
  topSizer->Add(midSizer, 0, wxALL, 5);
  topSizer->Add(rightSizer, 0, wxALL, 5);

  allSizer->Add(topSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();

  wxCommandEvent event;
  OnPlayer(event);
}

void dialogStrategies::OnPlayer(wxCommandEvent &)
{
  int p_number = m_playerItem->GetSelection();
  NFPlayer *player = m_nfg.Players()[p_number+1];
  m_strategyItem->Clear();
  for (int st = 1; st <= player->NumStrats(); st++)
    m_strategyItem->Append((char *) ToText(st));

  wxCommandEvent event;
  OnStrategy(event);
}

void dialogStrategies::OnStrategy(wxCommandEvent &)
{
  int p_number = m_strategyItem->GetSelection();
  if (m_prevStrategy)
    if (strcmp(m_prevStrategy->Name(), m_strategyNameItem->GetValue()) != 0) {
      m_prevStrategy->SetName(m_strategyNameItem->GetValue().c_str());
      m_gameChanged = true;
    }

  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection()+1];
  m_strategyItem->SetSelection(p_number);
  Strategy *strategy = player->Strategies()[p_number+1];
  m_strategyNameItem->SetValue((char *) strategy->Name());
  m_prevStrategy = strategy;
}

void dialogStrategies::OnOk(void)
{
  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection()+1];
  Strategy *strategy = player->Strategies()[m_strategyItem->GetSelection()+1];
  if (strcmp(strategy->Name(), m_strategyNameItem->GetValue()) != 0) {
    strategy->SetName(m_strategyNameItem->GetValue().c_str());
    m_gameChanged = true;
  }

  Show(FALSE);
}

//=========================================================================
//                   dialogNfgEditSupport: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogNfgEditSupport, guiAutoDialog)
  EVT_LISTBOX(idNFSUPPORT_PLAYER_LISTBOX, dialogNfgEditSupport::OnPlayer)
  EVT_LISTBOX(idNFSUPPORT_STRATEGY_LISTBOX, dialogNfgEditSupport::OnStrategy)
END_EVENT_TABLE()

dialogNfgEditSupport::dialogNfgEditSupport(const NFSupport &p_support,
					   wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Edit support"),
    m_nfg(p_support.Game()), m_support(p_support)
{
  m_nameItem = new wxTextCtrl(this, -1);
  m_nameItem->SetValue((char *) p_support.GetName());

  m_playerItem = new wxListBox(this, idNFSUPPORT_PLAYER_LISTBOX);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + 
			 m_nfg.Players()[pl]->GetName()));
  }
  m_playerItem->SetSelection(0);

#ifdef __WXGTK__
  // the wxGTK multiple-selection listbox is flaky (2.1.11)
  m_strategyItem = new wxListBox(this, idNFSUPPORT_STRATEGY_LISTBOX,
				 wxDefaultPosition, wxDefaultSize,
				 0, 0, wxLB_EXTENDED);
#else
  m_strategyItem = new wxListBox(this, idNFSUPPORT_STRATEGY_LISTBOX,
				 wxDefaultPosition, wxDefaultSize,
				 0, 0, wxLB_MULTIPLE);
#endif // __WXGTK__

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);

  leftSizer->Add(new wxStaticText(this, -1, "Player"), 0, wxCENTRE | wxALL, 5);
  leftSizer->Add(m_playerItem, 0, wxCENTRE | wxALL, 5);

  rightSizer->Add(new wxStaticText(this, -1, "Strategy"),
		  0, wxCENTRE | wxALL, 5);
  rightSizer->Add(m_strategyItem, 0, wxCENTRE | wxALL, 5);

  topSizer->Add(new wxStaticText(this, -1, "Name"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_nameItem, 1, wxEXPAND | wxALL, 5);

  botSizer->Add(leftSizer, 0, wxALL, 5);
  botSizer->Add(rightSizer, 0, wxALL, 5);

  allSizer->Add(topSizer, 1, wxEXPAND | wxALL, 5);
  allSizer->Add(botSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();

  wxCommandEvent event;
  OnPlayer(event);
}

void dialogNfgEditSupport::OnPlayer(wxCommandEvent &)
{
  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection() + 1];
  m_strategyItem->Clear();
  for (int st = 1; st <= player->NumStrats(); st++) {
    m_strategyItem->Append((char *) (ToText(st) + ": " +
			   player->Strategies()[st]->Name()));
    if (m_support.Find(player->Strategies()[st])) {
      m_strategyItem->SetSelection(st - 1, true);
    }
  }
}

void dialogNfgEditSupport::OnStrategy(wxCommandEvent &)
{
  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection() + 1];
  for (int st = 0; st < m_strategyItem->Number(); st++) {
    Strategy *strategy = player->Strategies()[st+1];
    if (m_strategyItem->Selected(st)) {
      m_support.AddStrategy(strategy);
    }
    else {
      m_support.RemoveStrategy(strategy);
    }
  }
}

gText dialogNfgEditSupport::Name(void) const
{ return m_nameItem->GetValue().c_str(); }

