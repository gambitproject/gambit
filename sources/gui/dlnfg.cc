//
// FILE: dlnfg.cc -- Normal form-related dialog implementations
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wxmisc.h"

#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"

#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgsave.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"
#include "dlnfgeditsupport.h"

//=========================================================================
//                 class dialogNfgPayoffs: Member functions 
//=========================================================================

dialogNfgPayoffs::dialogNfgPayoffs(const Nfg &p_nfg, NFOutcome *p_outcome,
				   wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Change Payoffs", p_nfg.NumPlayers() + 1),
    m_outcome(p_outcome), m_nfg(p_nfg)
{
  SetLabel(1, "Name");
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    if (m_nfg.Players()[pl]->GetName() != "") {
      SetLabel(pl + 1, m_nfg.Players()[pl]->GetName());
    }
    else {
      SetLabel(pl + 1, (char *) ("Player " + ToText(pl)));
    }
    SetValue(pl + 1, ToText(m_nfg.Payoff(p_outcome, pl)));
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

gArray<gNumber> dialogNfgPayoffs::Payoffs(void) const
{
  gArray<gNumber> ret(m_nfg.NumPlayers());
  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = ToNumber(GetValue(pl+1));
  }
  return ret;
}

gText dialogNfgPayoffs::Name(void) const
{
  return GetValue(1);
}

//=========================================================================
//              class dialogNfgOutcomeSelect: Member functions 
//=========================================================================

dialogNfgOutcomeSelect::dialogNfgOutcomeSelect(Nfg &p_nfg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Select Outcome"), m_nfg(p_nfg)
{
  m_outcomeList = new wxListBox(this, -1);
  
  for (int outc = 1; outc <= m_nfg.NumOutcomes(); outc++) {
    NFOutcome *outcome = m_nfg.Outcomes()[outc];
    gText item = ToText(outc) + ": " + outcome->GetName();
    if (item == "")
      item = "Outcome" + ToText(outc);

    item += (" (" + ToText(m_nfg.Payoff(outcome, 1)) + ", " +
	     ToText(m_nfg.Payoff(outcome, 2)));
    if (m_nfg.NumPlayers() > 2) {
      item += ", " + ToText(m_nfg.Payoff(outcome, 3));
      if (m_nfg.NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_outcomeList->Append((char *) item);
  }

  m_outcomeList->SetSelection(0);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1, "Outcome"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_outcomeList, 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

NFOutcome *dialogNfgOutcomeSelect::GetOutcome(void)
{
  return m_nfg.Outcomes()[m_outcomeList->GetSelection() + 1];
}

//=========================================================================
//                   dialogNfgPlayers: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogNfgPlayers, guiAutoDialog)
  EVT_BUTTON(idNFPLAYERS_EDIT_BUTTON, dialogNfgPlayers::OnEdit)
END_EVENT_TABLE()

dialogNfgPlayers::dialogNfgPlayers(Nfg &p_nfg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Player Names"), m_nfg(p_nfg)
{
  m_playerNameList = new wxListBox(this, -1);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerNameList->Append((char *) (ToText(pl) + ": " + m_nfg.Players()[pl]->GetName()));
  }
  m_playerNameList->SetSelection(0);
  m_lastSelection = 0;

  wxButton *editPlayer = new wxButton(this, idNFPLAYERS_EDIT_BUTTON, "Edit...");

  m_cancelButton->Show(FALSE);

  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);

  rightSizer->Add(editPlayer, 0, wxALL, 5);

  topSizer->Add(new wxStaticText(this, -1, "Player:"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_playerNameList, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(rightSizer, 0, wxALL, 5);

  allSizer->Add(topSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
}

void dialogNfgPlayers::OnEdit(wxCommandEvent &)
{
  int selection = m_playerNameList->GetSelection();
  gText defaultName = m_nfg.Players()[selection + 1]->GetName();

  gText newName = wxGetTextFromUser("Name", "Enter Name", (char *) defaultName, this).c_str();
  if (newName != "") {
    m_nfg.Players()[selection + 1]->SetName(newName);
    m_playerNameList->SetString(selection, (char *) (ToText(selection + 1) + ": " + newName));
  }
}

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

//=========================================================================
//                     dialogNfgSave: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogNfgSave, wxDialog)
  EVT_BUTTON(idNFG_BROWSE_BUTTON, dialogNfgSave::OnBrowse)
END_EVENT_TABLE()

dialogNfgSave::dialogNfgSave(const gText &p_name,
			     const gText &p_label, int p_decimals,
			     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Save File")
{
  wxBoxSizer *filenameSizer = new wxBoxSizer(wxHORIZONTAL);
  filenameSizer->Add(new wxStaticText(this, -1, "File"), 0,
		     wxCENTER | wxALL, 5);
  m_fileName = new wxTextCtrl(this, -1, (char *) p_name);
  filenameSizer->Add(m_fileName, 1, wxEXPAND | wxALL, 5);
  wxButton *browseButton = new wxButton(this, idNFG_BROWSE_BUTTON, "Browse...");
  filenameSizer->Add(browseButton, 0, wxALL, 5);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, -1, "Label"), 0,
		  wxCENTER | wxALL, 5);  
  m_treeLabel = new wxTextCtrl(this, -1, (char *) p_label);
  labelSizer->Add(m_treeLabel, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *decimalsSizer = new wxBoxSizer(wxHORIZONTAL);
  decimalsSizer->Add(new wxStaticText(this, -1, "Decimal places"),
		     0, wxALL, 5);
  m_numDecimals = new wxSlider(this, -1, p_decimals, 0, 25,
			       wxDefaultPosition, wxDefaultSize,
			       wxSL_HORIZONTAL | wxSL_LABELS);
  decimalsSizer->Add(m_numDecimals, 1, wxEXPAND | wxALL, 5);
 
  m_helpButton->Enable(false);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(filenameSizer, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(labelSizer, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(decimalsSizer, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetAutoLayout(TRUE);
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

void dialogNfgSave::OnBrowse(wxCommandEvent &)
{
  const char *file = wxFileSelector("Save data file", 
				    gPathOnly(m_fileName->GetValue()),
				    gFileNameFromPath(m_fileName->GetValue()),
				    ".nfg", "*.nfg");

  if (file) {
    m_fileName->SetValue(file);
  }
}

gText dialogNfgSave::Filename(void) const
{ return m_fileName->GetValue().c_str(); }

gText dialogNfgSave::Label(void) const
{ return m_treeLabel->GetValue().c_str(); }
