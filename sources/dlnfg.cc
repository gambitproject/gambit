//
// FILE: dlnfg.cc -- Normal form-related dialog implementations
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"

#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgsave.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"
#include "dlnfgnewsupport.h"

//=========================================================================
//                 class dialogNfgPayoffs: Member functions 
//=========================================================================

dialogNfgPayoffs::dialogNfgPayoffs(const Nfg &p_nfg, NFOutcome *p_outcome,
				   bool p_solutions, wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Change Payoffs", p_nfg.NumPlayers()),
    m_outcome(p_outcome), m_nfg(p_nfg)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++)
    SetValue(pl, ToText(m_nfg.Payoff(p_outcome, pl)));

  m_outcomeName = new wxText(this, 0, "Outcome", "", 1, 1);
  if (p_outcome)
    m_outcomeName->SetValue(p_outcome->GetName());
  else
    m_outcomeName->SetValue("Outcome" + ToText(p_nfg.NumOutcomes() + 1));

  m_outcomeName->SetConstraints(new wxLayoutConstraints);
  m_outcomeName->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_outcomeName->GetConstraints()->left.SameAs(m_dataFields[0], wxLeft);
  m_outcomeName->GetConstraints()->right.SameAs(m_dataFields[0], wxRight);
  m_outcomeName->GetConstraints()->height.AsIs();

  m_dataFields[0]->GetConstraints()->top.SameAs(m_outcomeName, wxBottom, 10);

  Go();
}

gArray<gNumber> dialogNfgPayoffs::Payoffs(void) const
{
  gArray<gNumber> ret(m_nfg.NumPlayers());
  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = ToNumber(GetValue(pl));
  }
  return ret;
}

gText dialogNfgPayoffs::Name(void) const
{
  return m_outcomeName->GetValue();
}

//=========================================================================
//              class dialogNfgOutcomeSelect: Member functions 
//=========================================================================

dialogNfgOutcomeSelect::dialogNfgOutcomeSelect(Nfg &p_nfg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Select Outcome"), m_nfg(p_nfg)
{
  m_outcomeList = new wxListBox(this, 0, "Outcome", wxSINGLE, 1, 1);
  
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

    m_outcomeList->Append(item);
  }

  m_outcomeList->SetSelection(0);
  m_outcomeList->SetConstraints(new wxLayoutConstraints);
  m_outcomeList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_outcomeList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_outcomeList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_outcomeList->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_outcomeList, wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

NFOutcome *dialogNfgOutcomeSelect::GetOutcome(void)
{
  return m_nfg.Outcomes()[m_outcomeList->GetSelection() + 1];
}

//=========================================================================
//                   dialogNfgPlayers: Member functions
//=========================================================================

dialogNfgPlayers::dialogNfgPlayers(Nfg &p_nfg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Player Names"), m_nfg(p_nfg)
{
  m_playerNameList = new wxListBox(this, 0, "Player", wxSINGLE);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerNameList->Append(ToText(pl) + ": " + m_nfg.Players()[pl]->GetName());
  }
  m_playerNameList->SetSelection(0);
  m_lastSelection = 0;

  m_playerNameList->SetConstraints(new wxLayoutConstraints);
  m_playerNameList->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerNameList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNameList->GetConstraints()->width.AsIs();
  m_playerNameList->GetConstraints()->height.AsIs();

  wxButton *editPlayer = new wxButton(this, (wxFunction) CallbackEdit, "Edit...");
  editPlayer->SetClientData((char *) this);

  editPlayer->SetConstraints(new wxLayoutConstraints);
  editPlayer->GetConstraints()->left.SameAs(m_playerNameList, wxRight, 10);
  editPlayer->GetConstraints()->top.SameAs(m_playerNameList, wxTop);
  editPlayer->GetConstraints()->width.AsIs();
  editPlayer->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->top.SameAs(m_playerNameList, wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_helpButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->left.SameAs(this, wxCentreX, 5);
  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();

  m_cancelButton->Show(FALSE);

  Go();
}

void dialogNfgPlayers::OnEdit(void)
{
  int selection = m_playerNameList->GetSelection();
  gText defaultName = m_nfg.Players()[selection + 1]->GetName();

  char *newName = wxGetTextFromUser("Name", "Enter Name", defaultName, this);
  if (newName) {
    m_nfg.Players()[selection + 1]->SetName(newName);
    m_playerNameList->SetString(selection, ToText(selection + 1) + ": " + newName);
  }
}

//=========================================================================
//                    dialogStrategies: Member functions
//=========================================================================

dialogStrategies::dialogStrategies(Nfg &p_nfg, wxFrame *p_parent)
  : wxDialogBox(p_parent, "Strategy Information", TRUE), m_nfg(p_nfg),
    m_gameChanged(false), m_prevStrategy(0)
{
  SetLabelPosition(wxVERTICAL);
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player",
			       wxSINGLE, 11, 3, 104, 125, 0, NULL, 0);	
  m_playerItem->wxEvtHandler::SetClientData((char *) this);
  m_strategyItem = new wxListBox(this, (wxFunction) CallbackStrategy,
				 "Strategy",
				 wxSINGLE, 130, 4, 100, 125, 0, NULL, 0);
  m_strategyItem->wxEvtHandler::SetClientData((char *) this);
  m_strategyNameItem = new wxText(this, 0, "Strategy Name", "",
				  251, 12, 174, 58, 0);

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOk, "Ok",
				    160, 162, -1, -1, 0);
  okButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp, "Help",
				      220, 162, -1, -1, 0);
  helpButton->SetClientData((char *) this);

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    if (m_nfg.Players()[pl]->GetName() != "")
      m_playerItem->Append(m_nfg.Players()[pl]->GetName());
    else
      m_playerItem->Append("Player " + ToText(pl));
  }

  OnPlayer(0);
  Fit(); 
  Show(TRUE);
}

void dialogStrategies::OnPlayer(int p_number)
{
  m_playerItem->SetSelection(p_number);
  NFPlayer *player = m_nfg.Players()[p_number+1];
  m_strategyItem->Clear();
  for (int st = 1; st <= player->NumStrats(); st++)
    m_strategyItem->Append(ToText(st));
  OnStrategy(0);
}

void dialogStrategies::OnStrategy(int p_number)
{
  if (m_prevStrategy)
    if (strcmp(m_prevStrategy->Name(), m_strategyNameItem->GetValue()) != 0) {
      m_prevStrategy->SetName(m_strategyNameItem->GetValue());
      m_gameChanged = true;
    }

  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection()+1];
  m_strategyItem->SetSelection(p_number);
  Strategy *strategy = player->Strategies()[p_number+1];
  m_strategyNameItem->SetValue(strategy->Name());
  m_prevStrategy = strategy;
}

void dialogStrategies::OnOk(void)
{
  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection()+1];
  Strategy *strategy = player->Strategies()[m_strategyItem->GetSelection()+1];
  if (strcmp(strategy->Name(), m_strategyNameItem->GetValue()) != 0) {
    strategy->SetName(m_strategyNameItem->GetValue());
    m_gameChanged = true;
  }

  Show(FALSE);
}

Bool dialogStrategies::OnClose(void)
{
  Show(FALSE);
  return FALSE;
}

void dialogStrategies::OnHelp(void)
{wxHelpContents("");}


void dialogStrategies::CallbackPlayer(wxListBox &p_object,
				      wxCommandEvent &p_event)
{
  ((dialogStrategies *) p_object.wxEvtHandler::GetClientData())->
    OnPlayer(p_event.commandInt);
}

void dialogStrategies::CallbackStrategy(wxListBox &p_object, 
					wxCommandEvent &p_event)
{
  ((dialogStrategies *) p_object.wxEvtHandler::GetClientData())->
    OnStrategy(p_event.commandInt);
}

void dialogStrategies::CallbackOk(wxButton &p_object, wxCommandEvent &)
{
  ((dialogStrategies *) p_object.GetClientData())->OnOk();
}

void dialogStrategies::CallbackHelp(wxButton &p_object, wxCommandEvent &)
{
  ((dialogStrategies *) p_object.GetClientData())->OnHelp();
}

//=========================================================================
//                   dialogNfgNewSupport: Member functions
//=========================================================================

dialogNfgNewSupport::dialogNfgNewSupport(const Nfg &p_nfg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Define support", TRUE), m_nfg(p_nfg)
{
  SetLabelPosition(wxVERTICAL);
  m_strategyLists = new wxListBox *[m_nfg.NumPlayers()];
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_strategyLists[pl-1] = new wxListBox(this, 0, "Player " + ToText(pl),
					  wxMULTIPLE);
    for (int st = 1; st <= m_nfg.NumStrats(pl); st++) {
      m_strategyLists[pl-1]->Append(ToText(st) + ": " +
				    m_nfg.Strategies(pl)[st]->Name());
#ifndef LINUX_WXXT
      m_strategyLists[pl-1]->SetSelection(st - 1, TRUE);
#endif  // LINUX_WXXT
    }
  }

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogNfgNewSupport::~dialogNfgNewSupport()
{
  delete [] m_strategyLists;
}

void dialogNfgNewSupport::OnOK(void)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    int *selections;
    if (m_strategyLists[pl-1]->GetSelections(&selections) == 0)
      return;
  }
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgNewSupport::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgNewSupport::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

NFSupport *dialogNfgNewSupport::CreateSupport(void) const
{
  NFSupport *support = new NFSupport(m_nfg);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= m_nfg.NumStrats(pl); st++) {
      if (!m_strategyLists[pl-1]->Selected(st-1))
	support->RemoveStrategy(m_nfg.Strategies(pl)[st]);
    }
  }

  return support;
}

//=========================================================================
//                     dialogNfgSave: Member functions
//=========================================================================

dialogNfgSave::dialogNfgSave(const gText &p_name,
			     const gText &p_label, int p_decimals,
			     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Save File")
{
  m_fileName = new wxText(this, 0, "Path:", "", 1, 1);
  m_fileName->SetValue(p_name);
  m_fileName->SetConstraints(new wxLayoutConstraints);
  m_fileName->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_fileName->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_fileName->GetConstraints()->width.AsIs();
  m_fileName->GetConstraints()->height.AsIs();

  wxButton *browseButton = new wxButton(this, (wxFunction) CallbackBrowse,
					"Browse...", 1, 1);
  browseButton->SetClientData((char *) this);
  browseButton->SetConstraints(new wxLayoutConstraints);
  browseButton->GetConstraints()->top.SameAs(m_fileName, wxTop);
  browseButton->GetConstraints()->left.SameAs(m_fileName, wxRight, 10);
  browseButton->GetConstraints()->width.AsIs();
  browseButton->GetConstraints()->height.AsIs();

  m_treeLabel = new wxText(this, 0, "Description:", "", 1, 1);
  m_treeLabel->SetValue(p_label);
  m_treeLabel->SetConstraints(new wxLayoutConstraints);
  m_treeLabel->GetConstraints()->top.SameAs(m_fileName, wxBottom, 10);
  m_treeLabel->GetConstraints()->left.SameAs(m_fileName, wxLeft);
  m_treeLabel->GetConstraints()->right.SameAs(browseButton, wxRight);
  m_treeLabel->GetConstraints()->height.AsIs();

  m_numDecimals = new wxSlider(this, 0, "Decimal places:",
			       p_decimals, 0, 25, -1, 1, 1);
  m_numDecimals->SetConstraints(new wxLayoutConstraints);
  m_numDecimals->GetConstraints()->top.SameAs(m_treeLabel, wxBottom, 10);
  m_numDecimals->GetConstraints()->left.SameAs(m_treeLabel, wxLeft);
  m_numDecimals->GetConstraints()->right.SameAs(browseButton, wxRight);
  m_numDecimals->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_numDecimals, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->top.AsIs();
  m_helpButton->GetConstraints()->left.AsIs();
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->Show(FALSE);

  Go();
}

void dialogNfgSave::OnBrowse(void)
{
  char *file = wxFileSelector("Save data file", 
			      gPathOnly(m_fileName->GetValue()),
			      gFileNameFromPath(m_fileName->GetValue()),
			      ".nfg", "*.nfg");

  if (file) {
    m_fileName->SetValue(file);
  }
}


