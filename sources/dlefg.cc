//
// FILE: dlefg.cc -- Extensive form-related dialog implementations
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "efgconst.h"

#include "efg.h"
#include "dlefgplayer.h"
#include "dlmoveadd.h"
#include "dlnodedelete.h"
#include "dlactionlabel.h"
#include "dlactionselect.h"
#include "dlactionprobs.h"
#include "dlefgreveal.h"
#include "dlefgpayoff.h"
#include "dlefgoutcome.h"
#include "dlefgsave.h"


//=========================================================================
//                  dialogEfgSelectPlayer: Member functions
//=========================================================================

dialogEfgSelectPlayer::dialogEfgSelectPlayer(const Efg &p_efg, bool p_chance, 
					     wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Player", TRUE),
    m_efg(p_efg), m_chance(p_chance)
{
  m_playerNameList = new wxListBox(this, 0, "Player");
  if (m_chance)
    m_playerNameList->Append("Chance");

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    const gText &name = m_efg.Players()[pl]->GetName();
    if (name != "")
      m_playerNameList->Append(name);
    else
      m_playerNameList->Append("Player" + ToText(pl));
  }

  m_playerNameList->SetSelection(0); 

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
				      "Help");
  helpButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogEfgSelectPlayer::~dialogEfgSelectPlayer()
{ }

void dialogEfgSelectPlayer::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgSelectPlayer::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgSelectPlayer::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

EFPlayer *dialogEfgSelectPlayer::GetPlayer(void)
{
  int playerSelected = m_playerNameList->GetSelection();
  if (m_chance) {
    if (playerSelected == 0)
      return m_efg.GetChance();
    else
      return m_efg.Players()[playerSelected];
  }
  else
    return m_efg.Players()[playerSelected + 1];
}

//=========================================================================
//                     dialogMoveAdd: Member functions
//=========================================================================

dialogMoveAdd::dialogMoveAdd(Efg &p_efg, EFPlayer *p_player,
			     Infoset *p_infoset, int p_branches,
			     wxFrame *p_frame)
  : wxDialogBox(p_frame, "Add Move", TRUE),
    m_efg(p_efg), m_branches(p_branches)
{
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player");
  m_playerItem->Append("Chance");
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append(ToText(pl) + ": " + m_efg.Players()[pl]->GetName());
  }
  m_playerItem->Append("New Player");
  m_playerItem->wxEvtHandler::SetClientData((char *) this);
  if (p_player)
    m_playerItem->SetSelection(p_player->GetNumber());
  else
    m_playerItem->SetSelection(0);

  m_infosetItem = new wxListBox(this, (wxFunction) CallbackInfoset,
				"Infoset");
  m_infosetItem->Append("New");
  if (p_player) {
    for (int iset = 1; iset <= p_player->NumInfosets(); iset++) {
      m_infosetItem->Append(ToText(iset) + ": " +
			    p_player->Infosets()[iset]->GetName());
    }
  }
  m_infosetItem->wxEvtHandler::SetClientData((char *) this);
  if (p_infoset)
    m_infosetItem->SetSelection(p_infoset->GetNumber());
  else
    m_infosetItem->SetSelection(0);

  NewLine();
 
  m_actionItem = new wxText(this, 0, "Actions");
  m_actionItem->SetValue(ToText(p_branches));
  m_actionItem->Enable(m_infosetItem->GetSelection() == 0);

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogMoveAdd::~dialogMoveAdd(void)
{ }

void dialogMoveAdd::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogMoveAdd::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

void dialogMoveAdd::OnPlayer(int p_player)
{
  EFPlayer *player = 0;
  m_playerItem->SetSelection(p_player);
  if (p_player == 0)
    player = m_efg.GetChance();
  else if (p_player <= m_efg.NumPlayers())
    player = m_efg.Players()[p_player];

  m_infosetItem->Clear();
  m_infosetItem->Append("New");
  if (player) {
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_infosetItem->Append(ToText(iset) + ": " +
			    player->Infosets()[iset]->GetName());
    }
  }
  m_infosetItem->SetSelection(0);
  m_actionItem->SetValue(ToText(2));
  m_actionItem->Enable(TRUE);
}

void dialogMoveAdd::OnInfoset(int p_infoset)
{
  m_infosetItem->SetSelection(p_infoset);
  if (p_infoset > 0) {
    int playerNumber = m_playerItem->GetSelection();
    Infoset *infoset;
    if (playerNumber == 0)
      infoset = m_efg.GetChance()->Infosets()[p_infoset];
    else
      infoset = m_efg.Players()[playerNumber]->Infosets()[p_infoset];
    m_actionItem->Enable(FALSE);
    m_actionItem->SetValue(ToText(infoset->NumActions()));
  }
  else {
    m_actionItem->Enable(TRUE);
    m_actionItem->SetValue(ToText(2));
  }
}

Bool dialogMoveAdd::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

NodeAddMode dialogMoveAdd::GetAddMode(void) const
{
  return (m_infosetItem->GetSelection() == 0) ? NodeAddNew : NodeAddIset;
}

EFPlayer *dialogMoveAdd::GetPlayer(void) const
{
  int playerNumber = m_playerItem->GetSelection();

  if (playerNumber == 0)
    return m_efg.GetChance();
  else if (playerNumber <= m_efg.NumPlayers())
    return m_efg.Players()[playerNumber];
  else
    return 0;
}

Infoset *dialogMoveAdd::GetInfoset(void) const
{
  EFPlayer *player = GetPlayer();
  int infosetNumber = m_infosetItem->GetSelection();

  if (player && infosetNumber > 0)
    return player->Infosets()[infosetNumber];
  else
    return 0;
}

int dialogMoveAdd::GetActions(void) const
{
  Infoset *infoset = GetInfoset();
  if (infoset)
    return infoset->NumActions();
  else
    return (int) ToDouble(m_actionItem->GetValue());
}


//=========================================================================
//                   dialogNodeDelete: Member functions
//=========================================================================

dialogNodeDelete::dialogNodeDelete(Node *p_node, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Delete Node", TRUE), m_node(p_node)
{
  SetLabelPosition(wxVERTICAL);
  m_branchList = new wxListBox(this, 0, "Keep subtree at branch");
  for (int act = 1; act <= p_node->NumChildren(); act++) {
    m_branchList->Append(ToText(act) + ": " +
			 p_node->GetInfoset()->Actions()[act]->GetName());
  }
  m_branchList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogNodeDelete::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNodeDelete::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNodeDelete::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//                   dialogActionLabel: Member functions
//=========================================================================

dialogActionLabel::dialogActionLabel(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Label Actions", TRUE), m_infoset(p_infoset),
    m_lastSelection(0), m_actionNames(p_infoset->NumActions())
{
  SetLabelPosition(wxVERTICAL);
  m_actionList = new wxListBox(this, (wxFunction) CallbackAction, "Actions");
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(ToText(act) + ": " +
			 p_infoset->Actions()[act]->GetName());
    m_actionNames[act] = p_infoset->Actions()[act]->GetName();
  }
  m_actionList->wxEvtHandler::SetClientData((char *) this);
  m_actionList->SetSelection(0);

  m_actionName = new wxText(this, (wxFunction) 0, "Name");
  m_actionName->SetValue(p_infoset->Actions()[1]->GetName());
  
  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogActionLabel::OnOK(void)
{
  m_completed = wxOK;
  m_actionNames[m_actionList->GetSelection() + 1] = m_actionName->GetValue();
  Show(FALSE);
}

void dialogActionLabel::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogActionLabel::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogActionLabel::OnAction(int p_action)
{
  if (p_action == m_lastSelection)  return;
  m_actionList->SetSelection(p_action);
  m_actionNames[m_lastSelection + 1] = m_actionName->GetValue();
  m_actionName->SetValue(m_actionNames[p_action + 1]);
  m_actionList->SetString(m_lastSelection,
			  ToText(m_lastSelection + 1) + ": " +
			  m_actionNames[m_lastSelection + 1]);
  m_lastSelection = p_action;
}

//=========================================================================
//                   dialogActionSelect: Member functions
//=========================================================================

dialogActionSelect::dialogActionSelect(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select actions", TRUE), m_infoset(p_infoset)
{
  SetLabelPosition(wxVERTICAL);
  m_actionList = new wxListBox(this, 0, "Actions");
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(ToText(act) + ": " +
			 p_infoset->Actions()[act]->GetName());
  }
  m_actionList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogActionSelect::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogActionSelect::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogActionSelect::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//                   dialogActionProbs: Member functions
//=========================================================================

dialogActionProbs::dialogActionProbs(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Label Actions", TRUE), m_infoset(p_infoset),
    m_lastSelection(0), m_actionProbs(p_infoset->NumActions())
{
  SetLabelPosition(wxVERTICAL);
  m_actionList = new wxListBox(this, (wxFunction) CallbackAction, "Actions");
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(ToText(act) + ": " +
			 p_infoset->Actions()[act]->GetName());
    m_actionProbs[act] = p_infoset->Game()->GetChanceProb(p_infoset, act);
  }
  m_actionList->wxEvtHandler::SetClientData((char *) this);
  m_actionList->SetSelection(0);

  m_actionProb = new wxText(this, 0, "Probability");
  m_actionProb->SetValue(ToText(m_actionProbs[1]));
  
  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogActionProbs::OnOK(void)
{
  m_completed = wxOK;
  m_actionProbs[m_actionList->GetSelection() + 1] = ToNumber(m_actionProb->GetValue());
  Show(FALSE);
}

void dialogActionProbs::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogActionProbs::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogActionProbs::OnAction(int p_action)
{
  if (p_action == m_lastSelection)  return;
  m_actionList->SetSelection(p_action);
  m_actionProbs[m_lastSelection + 1] = ToNumber(m_actionProb->GetValue());
  m_actionProb->SetValue(ToText(m_actionProbs[p_action + 1]));
  m_lastSelection = p_action;
}

//=========================================================================
//                    dialogEfgPayoffs: Member functions
//=========================================================================

dialogEfgPayoffs::dialogEfgPayoffs(const Efg &p_efg, EFOutcome *p_outcome,
				   bool p_solutions, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Change Payoffs", TRUE),
    m_outcome(p_outcome), m_efg(p_efg), m_payoffs(p_efg.NumPlayers())
{
  (void) new wxMessage(this, "Change payoffs for outcome:");
  NewLine();

  m_outcomeName = new wxText(this, 0, "Outcome");
  if (p_outcome)
    m_outcomeName->SetValue(p_outcome->GetName());
  else
    m_outcomeName->SetValue("Outcome" + ToText(p_efg.NumOutcomes() + 1));
  NewLine();

  if (p_solutions) {
    (void) new wxMessage(this, "Pressing OK will delete computed solutions");
    NewLine();
  }

  m_outcomePayoffs = new wxText *[m_efg.NumPlayers()];

  const int ENTRIES_PER_ROW = 3;

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_outcomePayoffs[pl - 1] = new wxText(this, 0, "");
    m_outcomePayoffs[pl - 1]->SetValue(ToText(m_efg.Payoff(p_outcome, pl)));
    if (pl % ENTRIES_PER_ROW == 0)
      NewLine();
  }

  m_outcomePayoffs[0]->SetFocus();
#ifndef LINUX_WXXT
  m_outcomePayoffs[0]->SetSelection(0, strlen(m_outcomePayoffs[0]->GetValue()));
#endif

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogEfgPayoffs::OnOK(void)
{
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++)
    FromText(m_outcomePayoffs[pl - 1]->GetValue(), m_payoffs[pl]);
  m_name = m_outcomeName->GetValue();

  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgPayoffs::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgPayoffs::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//                  dialogInfosetReveal: Member functions
//=========================================================================

dialogInfosetReveal::dialogInfosetReveal(const Efg &p_efg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Reveal to Players", TRUE), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, 0, "Players", wxMULTIPLE);

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    const gText &name = m_efg.Players()[pl]->GetName();
    if (name != "")
      m_playerNameList->Append(name);
    else
      m_playerNameList->Append("Player" + ToText(pl));
    m_playerNameList->SetSelection(pl - 1, TRUE);
  }

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogInfosetReveal::~dialogInfosetReveal()
{ }

void dialogInfosetReveal::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogInfosetReveal::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogInfosetReveal::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

gArray<EFPlayer *> dialogInfosetReveal::GetPlayers(void) const
{
  gBlock<EFPlayer *> players;

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    if (m_playerNameList->Selected(pl - 1))
      players.Append(m_efg.Players()[pl]);
  }

  return players;
}

//=========================================================================
//                dialogEfgOutcomeSelect: Member functions
//=========================================================================

dialogEfgOutcomeSelect::dialogEfgOutcomeSelect(Efg &p_efg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Outcome", TRUE), m_efg(p_efg)
{
  m_outcomeList = new wxListBox(this, 0, "Outcome");
  
  for (int outc = 1; outc <= m_efg.NumOutcomes(); outc++) {
    EFOutcome *outcome = m_efg.Outcomes()[outc];
    gText item = outcome->GetName();
    if (item == "")
      item = "Outcome" + ToText(outc);

    item += (" (" + ToText(m_efg.Payoff(outcome, 1)) + ", " +
	     ToText(m_efg.Payoff(outcome, 2)));
    if (m_efg.NumPlayers() > 2) {
      item += ", " + ToText(m_efg.Payoff(outcome, 3));
      if (m_efg.NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_outcomeList->Append(item);
  }

  m_outcomeList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  
  Fit();
  Show(TRUE);
}

void dialogEfgOutcomeSelect::OnOK(void)
{
  m_outcomeSelected = m_outcomeList->GetSelection();
  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgOutcomeSelect::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgOutcomeSelect::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

EFOutcome *dialogEfgOutcomeSelect::GetOutcome(void)
{
  return m_efg.Outcomes()[m_outcomeSelected + 1];
}

//=========================================================================
//                     dialogEfgSave: Member functions
//=========================================================================

dialogEfgSave::dialogEfgSave(const gText &p_name,
			     const gText &p_label, int p_decimals,
			     wxWindow *p_parent)
  : wxDialogBox(p_parent, "Save File", TRUE)
{
  m_fileName = new wxText(this, 0, "Path:");
  m_fileName->SetValue(p_name);

  wxButton *browseButton = new wxButton(this, (wxFunction) CallbackBrowse,
					"Browse...");
  browseButton->SetClientData((char *) this);
  NewLine();

  m_treeLabel = new wxText(this, 0, "Description:", p_label, -1, -1, 300);
  m_treeLabel->SetValue(p_label);
  NewLine();

  m_numDecimals = new wxSlider(this, 0, "Decimal places:",
			       p_decimals, 0, 25, 100);
  NewLine();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogEfgSave::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgSave::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgSave::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogEfgSave::OnBrowse(void)
{
  char *file = wxFileSelector("Save data file", 
			      gPathOnly(m_fileName->GetValue()),
			      gFileNameFromPath(m_fileName->GetValue()),
			      ".efg", "*.efg");

  if (file) {
    m_fileName->SetValue(file);
  }
}


