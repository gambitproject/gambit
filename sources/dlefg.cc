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
  SetAutoLayout(TRUE);
  
  m_playerNameList = new wxListBox(this, 0, "Player", wxSINGLE, 1, 1);
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
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK", 1, 1);
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel", 1, 1);
  cancelButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
				      "Help", 1, 1);
  helpButton->SetClientData((char *) this);

  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->top.SameAs(m_playerNameList, wxBottom, 10);
  okButton->GetConstraints()->right.SameAs(cancelButton, wxLeft, 10);
  okButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  okButton->GetConstraints()->height.AsIs();

  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  cancelButton->GetConstraints()->width.AsIs();
  cancelButton->GetConstraints()->height.AsIs();

  helpButton->SetConstraints(new wxLayoutConstraints);
  helpButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  helpButton->GetConstraints()->left.SameAs(cancelButton, wxRight, 10);
  helpButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  helpButton->GetConstraints()->height.AsIs();

  m_playerNameList->SetConstraints(new wxLayoutConstraints);
  m_playerNameList->GetConstraints()->left.SameAs(okButton, wxLeft);
  m_playerNameList->GetConstraints()->right.SameAs(helpButton, wxRight);
  m_playerNameList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNameList->GetConstraints()->height.AsIs();

  okButton->SetDefault();
  
  Layout();
  int width, height;
  m_playerNameList->GetSize(&width, &height);
  SetSize(-1, -1, width + 20, -1);
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

dialogMoveAdd::dialogMoveAdd(Efg &p_efg, const gText &p_title, EFPlayer *p_player,
			     Infoset *p_infoset, int p_branches,
			     wxFrame *p_frame)
  : wxDialogBox(p_frame, p_title, TRUE),
    m_efg(p_efg), m_branches(p_branches)
{
  SetAutoLayout(TRUE);

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

  NewLine();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK");
  okButton->SetClientData((char *) this);

  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
					"Help");
  helpButton->SetClientData((char *) this);

  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->right.SameAs(cancelButton, wxLeft, 10);
  okButton->GetConstraints()->top.SameAs(m_actionItem, wxBottom, 10);
  okButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  okButton->GetConstraints()->height.AsIs();

  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  cancelButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  cancelButton->GetConstraints()->width.AsIs();
  cancelButton->GetConstraints()->height.AsIs();

  helpButton->SetConstraints(new wxLayoutConstraints);
  helpButton->GetConstraints()->left.SameAs(cancelButton, wxRight, 10);
  helpButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  helpButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  helpButton->GetConstraints()->height.AsIs();

  okButton->SetDefault();
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

void dialogMoveAdd::OnHelp(void)
{
  wxHelpContents("Node Menu");
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
  SetAutoLayout(TRUE);

  SetLabelPosition(wxVERTICAL);
  m_branchList = new wxListBox(this, 0, "Keep subtree at branch");
  for (int act = 1; act <= p_node->NumChildren(); act++) {
    m_branchList->Append(ToText(act) + ": " +
			 p_node->GetInfoset()->Actions()[act]->GetName());
  }
  m_branchList->SetSelection(0);

  NewLine();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK");
  okButton->SetClientData((char *) this);

  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
					"Help");
  helpButton->SetClientData((char *) this);

  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->right.SameAs(cancelButton, wxLeft, 10);
  okButton->GetConstraints()->top.SameAs(m_branchList, wxBottom, 10);
  okButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  okButton->GetConstraints()->height.AsIs();

  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  cancelButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  cancelButton->GetConstraints()->width.AsIs();
  cancelButton->GetConstraints()->height.AsIs();

  helpButton->SetConstraints(new wxLayoutConstraints);
  helpButton->GetConstraints()->left.SameAs(cancelButton, wxRight, 10);
  helpButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  helpButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  helpButton->GetConstraints()->height.AsIs();

  m_branchList->SetConstraints(new wxLayoutConstraints);
  m_branchList->GetConstraints()->left.SameAs(okButton, wxLeft);
  m_branchList->GetConstraints()->right.SameAs(helpButton, wxRight);
  m_branchList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_branchList->GetConstraints()->height.AsIs();

  okButton->SetDefault();
  
  Layout();
  int width, height;
  m_branchList->GetSize(&width, &height);
  SetSize(-1, -1, width + 20, -1);

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

void dialogNodeDelete::OnHelp(void)
{
  wxHelpContents("Node Menu");
}

//=========================================================================
//                   dialogActionLabel: Member functions
//=========================================================================

int dialogActionLabel::s_actionsPerDialog = 8;

dialogActionLabel::dialogActionLabel(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Label Actions", TRUE), m_infoset(p_infoset),
    m_pageNumber(0), m_actionNames(p_infoset->NumActions())
{
  for (int act = 1; act <= m_infoset->NumActions(); act++)
    m_actionNames[act] = m_infoset->Actions()[act]->GetName();

  m_actionLabels = new wxText *[gmin(m_infoset->NumActions(),
				     s_actionsPerDialog)];
  for (int act = 1; act <= gmin(m_infoset->NumActions(), s_actionsPerDialog);
       act++) {
    m_actionLabels[act-1] = new wxText(this, 0, ToText(act) + "  ", "",
				       -1, -1, -1, -1, wxFIXED_LENGTH);
    m_actionLabels[act-1]->SetValue(m_infoset->Actions()[act]->GetName());
    NewLine();
  }

  if (m_infoset->NumActions() > s_actionsPerDialog) {
    m_backButton = new wxButton(this, (wxFunction) CallbackBack,
					"<< Back");
    m_backButton->SetClientData((char *) this);
    m_backButton->Enable(FALSE);
    m_nextButton = new wxButton(this, (wxFunction) CallbackNext,
					"Next >>");
    m_nextButton->SetClientData((char *) this);
  }

  m_okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  m_okButton->SetClientData((char *) this);
  m_cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  m_cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogActionLabel::OnOK(void)
{
  m_completed = wxOK;
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < gmin((m_pageNumber + 1) * s_actionsPerDialog,
		  m_infoset->NumActions()); act++, entry++)
    m_actionNames[act + 1] = m_actionLabels[entry]->GetValue();
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

void dialogActionLabel::OnBack(void)
{
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < gmin((m_pageNumber + 1) * s_actionsPerDialog,
		  m_infoset->NumActions()); act++, entry++)
    m_actionNames[act + 1] = m_actionLabels[entry]->GetValue();

  m_pageNumber--;
  entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++) {
    m_actionLabels[entry]->Show(FALSE);
    m_actionLabels[entry]->SetValue(m_actionNames[act + 1]);
    m_actionLabels[entry]->SetLabel(ToText(act + 1));
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_actionsPerDialog - 1; entry >= 0; entry--)
    m_actionLabels[entry]->Show(TRUE);
  
  m_actionLabels[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void dialogActionLabel::OnNext(void)
{
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++)
    m_actionNames[act + 1] = m_actionLabels[entry]->GetValue();

  m_pageNumber++;
  entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++) {
    if (act < m_infoset->NumActions()) {
      m_actionLabels[entry]->SetValue(m_actionNames[act + 1]);
      m_actionLabels[entry]->SetLabel(ToText(act + 1));
    }
    else
      m_actionLabels[entry]->Show(FALSE);
  }

  m_actionLabels[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_actionsPerDialog <=
		       m_infoset->NumActions());
}

//=========================================================================
//                   dialogActionSelect: Member functions
//=========================================================================

dialogActionSelect::dialogActionSelect(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select action", TRUE), m_infoset(p_infoset)
{
  SetAutoLayout(TRUE);

  SetLabelPosition(wxVERTICAL);
  m_actionList = new wxListBox(this, 0, "Actions");
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(ToText(act) + ": " +
			 p_infoset->Actions()[act]->GetName());
  }
  m_actionList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK");
  okButton->SetClientData((char *) this);

  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
					"Help");
  helpButton->SetClientData((char *) this);

  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->right.SameAs(cancelButton, wxLeft, 10);
  okButton->GetConstraints()->top.SameAs(m_actionList, wxBottom, 10);
  okButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  okButton->GetConstraints()->height.AsIs();

  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  cancelButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  cancelButton->GetConstraints()->width.AsIs();
  cancelButton->GetConstraints()->height.AsIs();

  helpButton->SetConstraints(new wxLayoutConstraints);
  helpButton->GetConstraints()->left.SameAs(cancelButton, wxRight, 10);
  helpButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  helpButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  helpButton->GetConstraints()->height.AsIs();

  m_actionList->SetConstraints(new wxLayoutConstraints);
  m_actionList->GetConstraints()->left.SameAs(okButton, wxLeft);
  m_actionList->GetConstraints()->right.SameAs(helpButton, wxRight);
  m_actionList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_actionList->GetConstraints()->height.AsIs();

  okButton->SetDefault();
  
  Layout();
  int width, height;
  m_actionList->GetSize(&width, &height);
  SetSize(-1, -1, width + 20, -1);

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

void dialogActionSelect::OnHelp(void)
{
  wxHelpContents("Action Menu");
}

//=========================================================================
//                   dialogActionProbs: Member functions
//=========================================================================

int dialogActionProbs::s_actionsPerDialog = 8;

dialogActionProbs::dialogActionProbs(Infoset *p_infoset, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Action Probabilities", TRUE), m_infoset(p_infoset),
    m_pageNumber(0), m_actionProbs(p_infoset->NumActions())
{
  for (int act = 1; act <= m_infoset->NumActions(); act++)
    m_actionProbs[act] = 
      m_infoset->Game()->GetChanceProb(m_infoset->Actions()[act]);

  m_probItems = new wxNumberItem *[gmin(m_infoset->NumActions(),
					s_actionsPerDialog)];
  for (int act = 1; act <= gmin(m_infoset->NumActions(), s_actionsPerDialog);
       act++) {
    m_probItems[act-1] = new wxNumberItem(this, ToText(act) + "  ", 
					  ToText(m_actionProbs[act]));
    NewLine();
  }

  if (m_infoset->NumActions() > s_actionsPerDialog) {
    m_backButton = new wxButton(this, (wxFunction) CallbackBack,
					"<< Back");
    m_backButton->SetClientData((char *) this);
    m_backButton->Enable(FALSE);
    m_nextButton = new wxButton(this, (wxFunction) CallbackNext,
					"Next >>");
    m_nextButton->SetClientData((char *) this);
  }

  m_okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  m_okButton->SetClientData((char *) this);
  m_cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  m_cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogActionProbs::OnOK(void)
{
  m_completed = wxOK;
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < gmin((m_pageNumber + 1) * s_actionsPerDialog,
		  m_infoset->NumActions()); act++, entry++)
    m_actionProbs[act + 1] = m_probItems[entry]->GetNumber();
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

void dialogActionProbs::OnBack(void)
{
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < gmin((m_pageNumber + 1) * s_actionsPerDialog,
		  m_infoset->NumActions()); act++, entry++)
    m_actionProbs[act + 1] = m_probItems[entry]->GetNumber();

  m_pageNumber--;
  entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++) {
    m_probItems[entry]->Show(FALSE);
    m_probItems[entry]->SetNumber(m_actionProbs[act + 1]);
    m_probItems[entry]->SetValue(ToText(m_actionProbs[act + 1]));
    m_probItems[entry]->SetLabel(ToText(act + 1));
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_actionsPerDialog - 1; entry >= 0; entry--)
    m_probItems[entry]->Show(TRUE);
  
  m_probItems[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void dialogActionProbs::OnNext(void)
{
  int entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++)
    m_actionProbs[act + 1] = m_probItems[entry]->GetNumber();

  m_pageNumber++;
  entry = 0;
  for (int act = m_pageNumber * s_actionsPerDialog;
       act < (m_pageNumber + 1) * s_actionsPerDialog; act++, entry++) {
    if (act < m_infoset->NumActions()) {
      m_probItems[entry]->SetNumber(m_actionProbs[act + 1]);
      m_probItems[entry]->SetValue(ToText(m_actionProbs[act + 1]));
      m_probItems[entry]->SetLabel(ToText(act + 1));
    }
    else
      m_probItems[entry]->Show(FALSE);
  }

  m_probItems[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_actionsPerDialog <=
		       m_infoset->NumActions());
}

//=========================================================================
//                    dialogEfgPayoffs: Member functions
//=========================================================================

int dialogEfgPayoffs::s_payoffsPerDialog = 8;

dialogEfgPayoffs::dialogEfgPayoffs(const Efg &p_efg, EFOutcome *p_outcome,
				   bool p_solutions, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Change Payoffs", TRUE),
    m_outcome(p_outcome), m_efg(p_efg), m_pageNumber(0),
    m_payoffs(p_efg.NumPlayers())
{
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++)
    m_payoffs[pl] = m_efg.Payoff(p_outcome, pl);

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

  m_outcomePayoffs = new wxNumberItem *[gmin(m_efg.NumPlayers(),
					     s_payoffsPerDialog)];

  for (int pl = 1; pl <= gmin(m_efg.NumPlayers(),
			      s_payoffsPerDialog); pl++) {
    m_outcomePayoffs[pl-1] = new wxNumberItem(this,
					      ToText(pl) + "  ",
					      ToText(m_payoffs[pl]));
    NewLine();
  }

  m_outcomePayoffs[0]->SetFocus();
#ifndef LINUX_WXXT
  m_outcomePayoffs[0]->SetSelection(0, strlen(m_outcomePayoffs[0]->GetValue()));
#endif

  NewLine();
  if (m_efg.NumPlayers() > s_payoffsPerDialog) {
    m_backButton = new wxButton(this, (wxFunction) CallbackBack,
					"<< Back");
    m_backButton->SetClientData((char *) this);
    m_backButton->Enable(FALSE);
    m_nextButton = new wxButton(this, (wxFunction) CallbackNext,
					"Next >>");
    m_nextButton->SetClientData((char *) this);
  }

  m_okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  m_okButton->SetClientData((char *) this);
  m_okButton->SetDefault();
  m_cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  m_cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogEfgPayoffs::OnOK(void)
{
  m_completed = wxOK;
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < gmin((m_pageNumber + 1) * s_payoffsPerDialog,
		 m_efg.NumPlayers()); pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();
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

void dialogEfgPayoffs::OnBack(void)
{
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < gmin((m_pageNumber + 1) * s_payoffsPerDialog,
		 m_efg.NumPlayers()); pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();

  m_pageNumber--;
  entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++) {
    m_outcomePayoffs[entry]->Show(FALSE);
    m_outcomePayoffs[entry]->SetNumber(m_payoffs[pl + 1]);
    m_outcomePayoffs[entry]->SetValue(ToText(m_payoffs[pl + 1]));
    m_outcomePayoffs[entry]->SetLabel(ToText(pl + 1) + "  ");
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_payoffsPerDialog - 1; entry >= 0; entry--)
    m_outcomePayoffs[entry]->Show(TRUE);
  
  m_outcomePayoffs[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void dialogEfgPayoffs::OnNext(void)
{
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();

  m_pageNumber++;
  entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++) {
    if (pl < m_efg.NumPlayers()) {
      m_outcomePayoffs[entry]->SetNumber(m_payoffs[pl + 1]);
      m_outcomePayoffs[entry]->SetValue(ToText(m_payoffs[pl + 1]));
      m_outcomePayoffs[entry]->SetLabel(ToText(pl + 1) + "  ");
    }
    else
      m_outcomePayoffs[entry]->Show(FALSE);
  }

  m_outcomePayoffs[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_payoffsPerDialog <=
		       m_efg.NumPlayers());
}

//=========================================================================
//                  dialogInfosetReveal: Member functions
//=========================================================================

dialogInfosetReveal::dialogInfosetReveal(const Efg &p_efg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Reveal to Players", TRUE), m_efg(p_efg)
{
  SetAutoLayout(TRUE);
  
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
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK");
  okButton->SetClientData((char *) this);

  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
					"Help");
  helpButton->SetClientData((char *) this);

  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->top.SameAs(m_playerNameList, wxBottom, 10);
  okButton->GetConstraints()->right.SameAs(cancelButton, wxLeft, 10);
  okButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  okButton->GetConstraints()->height.AsIs();

  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  cancelButton->GetConstraints()->width.AsIs();
  cancelButton->GetConstraints()->height.AsIs();

  helpButton->SetConstraints(new wxLayoutConstraints);
  helpButton->GetConstraints()->centreY.SameAs(okButton, wxCentreY);
  helpButton->GetConstraints()->left.SameAs(cancelButton, wxRight, 10);
  helpButton->GetConstraints()->width.SameAs(cancelButton, wxWidth);
  helpButton->GetConstraints()->height.AsIs();

  m_playerNameList->SetConstraints(new wxLayoutConstraints);
  m_playerNameList->GetConstraints()->left.SameAs(okButton, wxLeft);
  m_playerNameList->GetConstraints()->right.SameAs(helpButton, wxRight);
  m_playerNameList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNameList->GetConstraints()->height.AsIs();

  okButton->SetDefault();
  
  Layout();
  int width, height;
  m_playerNameList->GetSize(&width, &height);
  SetSize(-1, -1, width + 20, -1);

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
}  \

void dialogInfosetReveal::OnHelp(void)
{
  wxHelpContents("Infoset Menu");
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
//                    dialogInfosets: Member functions
//=========================================================================

#include "dlinfosets.h"

dialogInfosets::dialogInfosets(Efg &p_efg, wxFrame *p_parent)
  : wxDialogBox(p_parent, "Infoset Information", TRUE), m_efg(p_efg),
    m_gameChanged(false), m_prevInfoset(0)
{
  SetLabelPosition(wxVERTICAL);
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player",
			       wxSINGLE, 11, 3, 104, 125, 0, NULL, 0);
  m_playerItem->wxEvtHandler::SetClientData((char *) this);
  m_infosetItem = new wxListBox(this, (wxFunction) CallbackInfoset, "Infoset",
				wxSINGLE, 130, 4, 100, 125, 0, NULL, 0);
  m_infosetItem->wxEvtHandler::SetClientData((char *) this);
  m_infosetNameItem = new wxText(this, 0, "Infoset Name", "",
				 251, 12, 174, 58, 0);
  m_actionsItem = new wxText(this, 0, "Actions", "", 330, 76, 80, 54,
			     wxTE_READONLY);
  m_membersItem = new wxText(this, 0, "Members", "", 253, 77, 51, 53,
			     wxTE_READONLY);

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOk, "Ok",
				    100, 162, -1, -1, 0);
  okButton->SetClientData((char *) this);
  wxButton *newButton = new wxButton(this, (wxFunction) CallbackNew, "New",
				     150, 162, -1, -1, 0);
  newButton->SetClientData((char *) this);
  m_removeButton = new wxButton(this, (wxFunction) CallbackRemove,
				"Remove", 210, 162, -1, -1, 0);
  m_removeButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp, "Help",
				      290, 162, -1, -1, 0);
  helpButton->SetClientData((char *) this);

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    if (m_efg.Players()[pl]->GetName() != "")
      m_playerItem->Append(m_efg.Players()[pl]->GetName());
    else
      m_playerItem->Append("Player " + ToText(pl));
  }

  OnPlayer(0);
  Fit(); 
  Show(TRUE);
}

void dialogInfosets::OnPlayer(int p_number)
{
  if (m_efg.NumPlayers() == 0) {
    if (m_playerItem->Number() == 0)
      m_playerItem->Append("None");
    return;
  }

  m_playerItem->SetSelection(p_number);
  EFPlayer *player = m_efg.Players()[p_number+1];
  m_infosetItem->Clear();
  for (int iset = 1; iset <= player->NumInfosets(); iset++)
    m_infosetItem->Append(ToText(iset));
  OnInfoset(0);
}

void dialogInfosets::OnInfoset(int p_number)
{
  if (m_prevInfoset)
    if (strcmp(m_prevInfoset->GetName(),m_infosetNameItem->GetValue())!=0) {
      m_prevInfoset->SetName(m_infosetNameItem->GetValue());
      m_gameChanged = true;
    }

  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  if (player->NumInfosets() == 0) { // can have players w/ no infosets
    if (m_infosetItem->Number() == 0)
      m_infosetItem->Append("None");
    return;
  }

  m_infosetItem->SetSelection(p_number);
  Infoset *infoset = player->Infosets()[p_number+1];
  m_infosetNameItem->SetValue(infoset->GetName());
  m_actionsItem->SetValue(ToText(infoset->NumActions()));
  m_membersItem->SetValue(ToText(infoset->NumMembers()));
  m_prevInfoset = infoset;
  m_removeButton->Enable(infoset->NumMembers() == 0);
}

void dialogInfosets::NewInfoset(void)
{
  static int s_numActions = 2;

  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];

  gText prompt = "Number of actions at new information set for ";
  if (player->GetName() != "")
    prompt += player->GetName();
  else
    prompt += "Player " + ToText(player->GetNumber());

  gText reply = wxGetTextFromUser(prompt, "New Infoset",
				  ToText(s_numActions));
  int numActions = atoi(reply);

  if (numActions > 0) {
    s_numActions = numActions;
    m_efg.CreateInfoset(player, numActions);
    m_gameChanged = true;
    m_infosetItem->Append(ToText(player->NumInfosets()));
  }
}

void dialogInfosets::RemoveInfoset(void)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection()+1];
  m_efg.DeleteEmptyInfoset(infoset);
  m_gameChanged = true;
}

void dialogInfosets::OnOk(void)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection()+1];
  if (strcmp(infoset->GetName(),m_infosetNameItem->GetValue())!=0) {
    infoset->SetName(m_infosetNameItem->GetValue());
    m_gameChanged = true;
  }

  Show(FALSE);
}

Bool dialogInfosets::OnClose(void)
{
  Show(FALSE);
  return FALSE;
}

void dialogInfosets::OnHelp(void)
{wxHelpContents("");}


void dialogInfosets::CallbackPlayer(wxListBox &p_object,
				    wxCommandEvent &p_event)
{
  ((dialogInfosets *) p_object.wxEvtHandler::GetClientData())->
    OnPlayer(p_event.commandInt);
}

void dialogInfosets::CallbackInfoset(wxListBox &p_object, 
				     wxCommandEvent &p_event)
{
  ((dialogInfosets *) p_object.wxEvtHandler::GetClientData())->
    OnInfoset(p_event.commandInt);
}

void dialogInfosets::CallbackNew(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->NewInfoset();
}

void dialogInfosets::CallbackRemove(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->RemoveInfoset();
}

void dialogInfosets::CallbackOk(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->OnOk();
}

void dialogInfosets::CallbackHelp(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->OnHelp();
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


