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
#include "dlefgeditsupport.h"
#include "dlefgsave.h"
#include "dlefgplayers.h"


//=========================================================================
//                  dialogEfgSelectPlayer: Member functions
//=========================================================================

dialogEfgSelectPlayer::dialogEfgSelectPlayer(const Efg &p_efg, bool p_chance,
					     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Select Player"),
    m_efg(p_efg), m_chance(p_chance)
{
  m_playerNameList = new wxListBox(this, 0, "Player", wxSINGLE, 1, 1);
  if (m_chance)
    m_playerNameList->Append("Chance");

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append(ToText(pl) + ": " + m_efg.Players()[pl]->GetName());
  }
  m_playerNameList->SetSelection(0);

  m_okButton->GetConstraints()->top.SameAs(m_playerNameList, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  m_playerNameList->SetConstraints(new wxLayoutConstraints);
  m_playerNameList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_playerNameList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNameList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_playerNameList->GetConstraints()->height.AsIs();
  
  Go();
}

dialogEfgSelectPlayer::~dialogEfgSelectPlayer()
{ }

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

dialogMoveAdd::dialogMoveAdd(FullEfg &p_efg,
			     const gText &p_title, EFPlayer *p_player,
			     Infoset *p_infoset, int p_branches,
			     wxFrame *p_frame)
  : guiAutoDialog(p_frame, p_title),
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

  m_playerItem->SetConstraints(new wxLayoutConstraints);
  m_playerItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerItem->GetConstraints()->width.AsIs();
  m_playerItem->GetConstraints()->height.AsIs();

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
  m_infosetItem->SetConstraints(new wxLayoutConstraints);
  m_infosetItem->GetConstraints()->top.SameAs(m_playerItem, wxTop);
  m_infosetItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_infosetItem->GetConstraints()->width.AsIs();
  m_infosetItem->GetConstraints()->height.AsIs();

  m_actionItem = new wxText(this, 0, "Actions");
  m_actionItem->SetValue(ToText(p_branches));
  m_actionItem->Enable(m_infosetItem->GetSelection() == 0);

  m_actionItem->SetConstraints(new wxLayoutConstraints);
  m_actionItem->GetConstraints()->top.SameAs(m_playerItem, wxBottom, 10);
  m_actionItem->GetConstraints()->left.SameAs(m_playerItem, wxLeft);
  m_actionItem->GetConstraints()->width.AsIs();
  m_actionItem->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_actionItem, wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

dialogMoveAdd::~dialogMoveAdd(void)
{ }

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
  else {
    EFPlayer *player = m_efg.NewPlayer();
    player->SetName("Player" + m_efg.NumPlayers());
    return player;
  }
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

void dialogMoveAdd::OnOk(void)
{
  int actions = GetActions();
  if (actions <= 0) {
    wxMessageBox("Number of actions must be at least one", "Error");
  }
  else {
    guiAutoDialog::OnOk();
  }
}


//=========================================================================
//                   dialogNodeDelete: Member functions
//=========================================================================

dialogNodeDelete::dialogNodeDelete(Node *p_node, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Delete Node"), m_node(p_node)
{
  SetLabelPosition(wxVERTICAL);
  m_branchList = new wxListBox(this, 0, "Keep subtree at branch");
  for (int act = 1; act <= p_node->Game()->NumChildren(p_node); act++) {
    m_branchList->Append(ToText(act) + ": " +
			 p_node->GetInfoset()->Actions()[act]->GetName());
  }
  m_branchList->SetSelection(0);

  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_branchList, wxBottom, 10);
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

  m_branchList->SetConstraints(new wxLayoutConstraints);
  m_branchList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_branchList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_branchList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_branchList->GetConstraints()->height.AsIs();

  Go();
}

//=========================================================================
//                   dialogActionLabel: Member functions
//=========================================================================

dialogActionLabel::dialogActionLabel(Infoset *p_infoset, wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Label Actions", p_infoset->NumActions()),
    m_infoset(p_infoset)
{
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    SetValue(act, m_infoset->Actions()[act]->GetName());
  }

  Go();
}

//=========================================================================
//                   dialogActionSelect: Member functions
//=========================================================================

dialogActionSelect::dialogActionSelect(Infoset *p_infoset,
				       const gText &p_caption,
				       const gText &p_label,
				       wxWindow *p_parent)
  : guiAutoDialog(p_parent, p_caption), m_infoset(p_infoset)
{
  SetLabelPosition(wxVERTICAL);
  m_actionList = new wxListBox(this, 0, p_label, wxSINGLE, 1, 1);
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(ToText(act) + ": " +
			 p_infoset->Actions()[act]->GetName());
  }
  m_actionList->SetSelection(0);

  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_actionList, wxBottom, 10);
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

  m_actionList->SetConstraints(new wxLayoutConstraints);
  m_actionList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_actionList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_actionList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_actionList->GetConstraints()->height.AsIs();

  Go();
}

//=========================================================================
//                   dialogActionProbs: Member functions
//=========================================================================

dialogActionProbs::dialogActionProbs(Infoset *p_infoset, wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Action Probabilities", p_infoset->NumActions()),
    m_infoset(p_infoset)
{
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    SetValue(act,
	     ToText(m_infoset->Game()->GetChanceProb(m_infoset->Actions()[act])));
  }

  Go();
}

//=========================================================================
//                    dialogEfgPayoffs: Member functions
//=========================================================================

dialogEfgPayoffs::dialogEfgPayoffs(const FullEfg &p_efg, EFOutcome *p_outcome,
				   wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Change Payoffs", p_efg.NumPlayers()),
    m_outcome(p_outcome), m_efg(p_efg)
{
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    EFPlayer *player = m_efg.Players()[pl];
    SetValue(pl, ToText(m_efg.Payoff(p_outcome, player)));
  }

  m_outcomeName = new wxText(this, 0, "Outcome", "", 1, 1);
  if (p_outcome)
    m_outcomeName->SetValue(p_outcome->GetName());
  else
    m_outcomeName->SetValue("Outcome" + ToText(p_efg.NumOutcomes() + 1));

  m_outcomeName->SetConstraints(new wxLayoutConstraints);
  m_outcomeName->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_outcomeName->GetConstraints()->left.SameAs(m_dataFields[0], wxLeft);
  m_outcomeName->GetConstraints()->right.SameAs(m_dataFields[0], wxRight);
  m_outcomeName->GetConstraints()->height.AsIs();

  m_dataFields[0]->GetConstraints()->top.SameAs(m_outcomeName, wxBottom, 10);
  m_dataFields[0]->SetFocus();
  m_dataFields[0]->SetSelection(0, strlen(m_dataFields[0]->GetValue()));

  Go();
}

gArray<gNumber> dialogEfgPayoffs::Payoffs(void) const
{
  gArray<gNumber> ret(m_efg.NumPlayers());
  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = ToNumber(GetValue(pl));
  }
  return ret;
}

gText dialogEfgPayoffs::Name(void) const
{
  return m_outcomeName->GetValue();
}

//=========================================================================
//                  dialogInfosetReveal: Member functions
//=========================================================================

dialogInfosetReveal::dialogInfosetReveal(const Efg &p_efg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Reveal to Players"), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, 0, "Players", wxMULTIPLE);

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append(ToText(pl) + ": " + 
			     m_efg.Players()[pl]->GetName());
    m_playerNameList->SetSelection(pl - 1, TRUE);
  }

  m_okButton->GetConstraints()->top.SameAs(m_playerNameList, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(m_playerNameList, wxLeft);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  m_playerNameList->SetConstraints(new wxLayoutConstraints);
  m_playerNameList->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerNameList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_playerNameList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNameList->GetConstraints()->height.AsIs();

  Go();
}

dialogInfosetReveal::~dialogInfosetReveal()
{ }

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

dialogInfosets::dialogInfosets(FullEfg &p_efg, wxFrame *p_parent)
  : guiAutoDialog(p_parent, "Infoset Information"), m_efg(p_efg),
    m_gameChanged(false), m_prevInfoset(0)
{
  SetLabelPosition(wxVERTICAL);
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player");
  m_playerItem->wxEvtHandler::SetClientData((char *) this);
  m_playerItem->SetConstraints(new wxLayoutConstraints());
  m_playerItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerItem->GetConstraints()->width.AsIs();
  m_playerItem->GetConstraints()->height.AsIs();

  m_infosetItem = new wxListBox(this, 0, "Infoset");
  m_infosetItem->wxEvtHandler::SetClientData((char *) this);
  m_infosetItem->SetConstraints(new wxLayoutConstraints());
  m_infosetItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_infosetItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_infosetItem->GetConstraints()->width.AsIs();
  m_infosetItem->GetConstraints()->height.AsIs();

  m_actionsItem = new wxText(this, 0, "Actions", "", 1, 1, -1, -1,
			     wxTE_READONLY);
  m_actionsItem->SetConstraints(new wxLayoutConstraints());
  m_actionsItem->GetConstraints()->top.SameAs(m_infosetItem, wxTop);
  m_actionsItem->GetConstraints()->left.SameAs(m_infosetItem, wxRight, 10);
  m_actionsItem->GetConstraints()->height.AsIs();
  m_actionsItem->GetConstraints()->width.PercentOf(m_playerItem, wxWidth, 30);

  m_membersItem = new wxText(this, 0, "Members", "", 1, 1, -1, -1,
			     wxTE_READONLY);
  m_membersItem->SetConstraints(new wxLayoutConstraints());
  m_membersItem->GetConstraints()->top.SameAs(m_actionsItem, wxBottom, 10);
  m_membersItem->GetConstraints()->left.SameAs(m_actionsItem, wxLeft);
  m_membersItem->GetConstraints()->height.AsIs();
  m_membersItem->GetConstraints()->width.SameAs(m_actionsItem, wxWidth);

  m_editButton = new wxButton(this, (wxFunction) CallbackEdit, "Edit...");
  m_editButton->SetClientData((char *) this);

  wxButton *newButton = new wxButton(this, (wxFunction) CallbackRemove, "New");
  newButton->SetClientData((char *) this);

  m_removeButton = new wxButton(this, (wxFunction) CallbackRemove,
				"Remove");
  m_removeButton->SetClientData((char *) this);

  m_editButton->SetConstraints(new wxLayoutConstraints);
  m_editButton->GetConstraints()->top.SameAs(newButton, wxTop);
  m_editButton->GetConstraints()->right.SameAs(newButton, wxLeft, 10);
  m_editButton->GetConstraints()->height.AsIs();
  m_editButton->GetConstraints()->width.AsIs();

  newButton->SetConstraints(new wxLayoutConstraints);
  newButton->GetConstraints()->top.SameAs(m_infosetItem, wxBottom, 10);
  newButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  newButton->GetConstraints()->height.AsIs();
  newButton->GetConstraints()->width.SameAs(m_editButton, wxWidth);

  m_removeButton->SetConstraints(new wxLayoutConstraints);
  m_removeButton->GetConstraints()->centreY.SameAs(newButton, wxCentreY);
  m_removeButton->GetConstraints()->left.SameAs(newButton, wxRight, 10);
  m_removeButton->GetConstraints()->height.AsIs();
  m_removeButton->GetConstraints()->width.SameAs(m_editButton, wxWidth);

  m_okButton->GetConstraints()->top.SameAs(newButton, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_editButton, wxWidth);

  m_helpButton->GetConstraints()->top.SameAs(newButton, wxBottom, 10);
  m_helpButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_editButton, wxWidth);

  m_cancelButton->Show(FALSE);
  m_cancelButton->GetConstraints()->top.SameAs(this, wxTop);
  m_cancelButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append(ToText(pl) + ": " + m_efg.Players()[pl]->GetName());
  }

  OnPlayer(0);
  Go();
}

void dialogInfosets::OnPlayer(int p_number)
{
  m_playerItem->SetSelection(p_number);
  EFPlayer *player = m_efg.Players()[p_number+1];
  m_infosetItem->Clear();
  if (player->NumInfosets() > 0)  {
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      m_infosetItem->Append(ToText(iset) + ": " + player->Infosets()[iset]->GetName());
    m_infosetItem->SetSelection(0);
    m_infosetItem->Enable(TRUE);
    m_editButton->Enable(TRUE);
    OnInfoset();
  }
  else {
    m_infosetItem->Append("(None)");
    m_infosetItem->Enable(FALSE);
    m_editButton->Enable(FALSE);
    m_removeButton->Enable(FALSE);
  }
}

void dialogInfosets::OnInfoset(void)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];

  m_actionsItem->SetValue(ToText(infoset->NumActions()));
  m_membersItem->SetValue(ToText(infoset->NumMembers()));

  m_removeButton->Enable(infoset->NumMembers() == 0);
}

void dialogInfosets::OnEdit(void)
{
  int selection = m_infosetItem->GetSelection();
  gText defaultName = (m_efg.Players()[m_playerItem->GetSelection()+1]->
                       Infosets()[selection+1])->GetName();

  char *newName = wxGetTextFromUser("Name", "Enter Name", defaultName, this);
  if (newName) {
	 m_efg.Players()[m_playerItem->GetSelection()+1]->
      Infosets()[selection+1]->SetName(newName);
	 m_infosetItem->SetString(selection, ToText(selection + 1) + ": " + newName);
  }
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
    if (player->NumInfosets() == 1)  {
      m_infosetItem->Clear();
    }
    m_infosetItem->Append(ToText(player->NumInfosets()) + ": ");
    m_infosetItem->SetSelection(player->NumInfosets() - 1);
    m_removeButton->Enable(TRUE);
    m_editButton->Enable(TRUE);
    m_infosetItem->Enable(TRUE);
  }
}

void dialogInfosets::RemoveInfoset(void)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection()+1];
  m_efg.DeleteEmptyInfoset(infoset);
  m_gameChanged = true;
}

void dialogInfosets::CallbackPlayer(wxListBox &p_object,
				    wxCommandEvent &p_event)
{
  ((dialogInfosets *) p_object.wxEvtHandler::GetClientData())->
    OnPlayer(p_event.commandInt);
}

void dialogInfosets::CallbackInfoset(wxListBox &p_object,
				    wxCommandEvent &)
{
  ((dialogInfosets *) p_object.wxEvtHandler::GetClientData())->
    OnInfoset();
}

void dialogInfosets::CallbackEdit(wxButton &p_object,
				     wxCommandEvent &)
{
  ((dialogInfosets *) p_object.wxEvtHandler::GetClientData())->
    OnEdit();
}

void dialogInfosets::CallbackNew(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->NewInfoset();
}

void dialogInfosets::CallbackRemove(wxButton &p_object, wxCommandEvent &)
{
  ((dialogInfosets *) p_object.GetClientData())->RemoveInfoset();
}

//=========================================================================
//                   dialogEfgPlayers: Member functions
//=========================================================================

dialogEfgPlayers::dialogEfgPlayers(FullEfg &p_efg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Player Names"), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, 0, "Player", wxSINGLE);
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append(ToText(pl) + ": " + m_efg.Players()[pl]->GetName());
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

  wxButton *newPlayer = new wxButton(this, (wxFunction) CallbackNew, "New player...");
  newPlayer->SetClientData((char *) this);

  editPlayer->SetConstraints(new wxLayoutConstraints);
  editPlayer->GetConstraints()->left.SameAs(m_playerNameList, wxRight, 10);
  editPlayer->GetConstraints()->top.SameAs(m_playerNameList, wxTop);
  editPlayer->GetConstraints()->width.SameAs(newPlayer, wxWidth);
  editPlayer->GetConstraints()->height.AsIs();

  newPlayer->SetConstraints(new wxLayoutConstraints);
  newPlayer->GetConstraints()->left.SameAs(editPlayer, wxLeft);
  newPlayer->GetConstraints()->top.SameAs(editPlayer, wxBottom, 10);
  newPlayer->GetConstraints()->width.AsIs();
  newPlayer->GetConstraints()->height.AsIs();

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

void dialogEfgPlayers::OnEdit(void)
{
  int selection = m_playerNameList->GetSelection();
  gText defaultName = m_efg.Players()[selection + 1]->GetName();

  char *newName = wxGetTextFromUser("Name", "Enter Name", defaultName, this);
  if (newName) {
	m_efg.Players()[selection + 1]->SetName(newName);
	m_playerNameList->SetString(selection, ToText(selection + 1) + ": " + newName);
  }
}

void dialogEfgPlayers::OnNew(void)
{
  gText defaultName = "Player " + ToText(m_efg.NumPlayers() + 1);
  char *newName = wxGetTextFromUser("New Player's name", "Enter Name",
                                    defaultName, this);
  if (newName) {
    EFPlayer *newPlayer = m_efg.NewPlayer();
    newPlayer->SetName(newName);
    m_playerNameList->Append(ToText(m_efg.NumPlayers()) + ": " + newName);
    m_playerNameList->SetSelection(m_efg.NumPlayers() - 1);
  }
}

//=========================================================================
//                dialogEfgOutcomeSelect: Member functions
//=========================================================================

dialogEfgOutcomeSelect::dialogEfgOutcomeSelect(FullEfg &p_efg,
					       wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Select Outcome"), m_efg(p_efg)
{
  m_outcomeList = new wxListBox(this, 0, "Outcome", wxSINGLE, 1, 1);
  
  for (int outc = 1; outc <= m_efg.NumOutcomes(); outc++) {
    EFOutcome *outcome = m_efg.Outcomes()[outc];
    gText item = ToText(outc) + ": " + outcome->GetName();
    if (item == "")
      item = "Outcome" + ToText(outc);

    item += (" (" + ToText(m_efg.Payoff(outcome, m_efg.Players()[1])) + ", " +
	     ToText(m_efg.Payoff(outcome, m_efg.Players()[2])));
    if (m_efg.NumPlayers() > 2) {
      item += ", " + ToText(m_efg.Payoff(outcome, m_efg.Players()[3]));
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

EFOutcome *dialogEfgOutcomeSelect::GetOutcome(void)
{
  return m_efg.Outcomes()[m_outcomeList->GetSelection() + 1];
}

//=========================================================================
//                   dialogEfgEditSupport: Member functions
//=========================================================================

dialogEfgEditSupport::dialogEfgEditSupport(const EFSupport &p_support,
					   wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Edit support"),
    m_efg(p_support.Game()), m_support(p_support)
{
  m_nameItem = new wxText(this, 0, "Name");
  m_nameItem->SetValue(p_support.GetName());
  m_nameItem->SetConstraints(new wxLayoutConstraints);
  m_nameItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_nameItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_nameItem->GetConstraints()->width.AsIs();
  m_nameItem->GetConstraints()->height.AsIs();

  SetLabelPosition(wxVERTICAL);
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player");
  m_playerItem->wxEvtHandler::SetClientData((char *) this);
  m_playerItem->SetConstraints(new wxLayoutConstraints);
  m_playerItem->GetConstraints()->top.SameAs(m_nameItem, wxBottom, 10);
  m_playerItem->GetConstraints()->left.SameAs(m_nameItem, wxLeft);
  m_playerItem->GetConstraints()->width.AsIs();
  m_playerItem->GetConstraints()->height.AsIs();

  m_infosetItem = new wxListBox(this, (wxFunction) CallbackInfoset, "Infoset");
  m_infosetItem->wxEvtHandler::SetClientData((char *) this);
  m_infosetItem->SetConstraints(new wxLayoutConstraints);
  m_infosetItem->GetConstraints()->top.SameAs(m_playerItem, wxTop);
  m_infosetItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_infosetItem->GetConstraints()->width.AsIs();
  m_infosetItem->GetConstraints()->height.AsIs();

  m_actionItem = new wxListBox(this, (wxFunction) CallbackAction,
			       "Action", wxMULTIPLE);
  m_actionItem->wxEvtHandler::SetClientData((char *) this);
  m_actionItem->SetConstraints(new wxLayoutConstraints);
  m_actionItem->GetConstraints()->top.SameAs(m_infosetItem, wxTop);
  m_actionItem->GetConstraints()->left.SameAs(m_infosetItem, wxRight, 10);
  m_actionItem->GetConstraints()->width.AsIs();
  m_actionItem->GetConstraints()->height.AsIs();

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append(ToText(pl) + ": " + 
			 m_efg.Players()[pl]->GetName());
#ifndef LINUX_WXXT
    m_playerItem->SetSelection(pl - 1, TRUE);
#endif  // LINUX_WXXT
  }

  m_okButton->GetConstraints()->top.SameAs(m_playerItem, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  OnPlayer(0);
  Go();
}

void dialogEfgEditSupport::OnPlayer(int p_number)
{
  m_playerItem->SetSelection(p_number);
  EFPlayer *player = m_efg.Players()[p_number+1];
  m_infosetItem->Clear();
  m_actionItem->Clear();
  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
    m_infosetItem->Append(ToText(iset) + ": " +
			  player->Infosets()[iset]->GetName());
  }
  m_infosetItem->SetSelection(0);
  for (int act = 1; act <= player->Infosets()[1]->NumActions(); act++) {
    m_actionItem->Append(ToText(act) + ": " +
			 player->Infosets()[1]->Actions()[act]->GetName());
    if (m_support.Find(player->Infosets()[1]->Actions()[act])) {
      m_actionItem->SetSelection(act - 1, TRUE);
    }
  }
}

void dialogEfgEditSupport::OnInfoset(int p_number)
{
  m_infosetItem->SetSelection(p_number);
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[p_number + 1];
  m_actionItem->Clear();
  for (int act = 1; act <= infoset->NumActions(); act++) {
    m_actionItem->Append(ToText(act) + ": " + infoset->Actions()[act]->GetName());
    if (m_support.Find(infoset->Actions()[act])) {
      m_actionItem->SetSelection(act - 1, TRUE);
    }
  }
}

void dialogEfgEditSupport::OnAction(int /*p_action*/)
{
  int player = m_playerItem->GetSelection() + 1;
  int infoset = m_infosetItem->GetSelection() + 1;
  for (int act = 0; act < m_actionItem->Number(); act++) {
    Action *action = m_efg.Players()[player]->Infosets()[infoset]->Actions()[act+1];
    if (m_actionItem->Selected(act)) {
      m_support.AddAction(action);
    }
    else {
      m_support.RemoveAction(action);
    }
  }
}

void dialogEfgEditSupport::CallbackPlayer(wxListBox &p_object,
					  wxCommandEvent &p_event)
{
  ((dialogEfgEditSupport *) p_object.wxEvtHandler::GetClientData())->
    OnPlayer(p_event.commandInt);
}

void dialogEfgEditSupport::CallbackInfoset(wxListBox &p_object,
					   wxCommandEvent &p_event)
{
  ((dialogEfgEditSupport *) p_object.wxEvtHandler::GetClientData())->
    OnInfoset(p_event.commandInt);
}

void dialogEfgEditSupport::CallbackAction(wxListBox &p_object, 
					    wxCommandEvent &p_event)
{
  ((dialogEfgEditSupport *) p_object.wxEvtHandler::GetClientData())->
    OnAction(p_event.commandInt);
}

//=========================================================================
//                     dialogEfgSave: Member functions
//=========================================================================

dialogEfgSave::dialogEfgSave(const gText &p_name,
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

void dialogEfgSave::OnBrowse(void)
{
#ifdef wx_motif
  char *file = wxGetTextFromUser("Save data file", "Save data file",
				 m_fileName->GetValue());
#else
  char *file = wxFileSelector("Save data file", 
			      gPathOnly(m_fileName->GetValue()),
			      gFileNameFromPath(m_fileName->GetValue()),
			      ".efg", "*.efg");
#endif  // wx_motif
  if (file) {
    m_fileName->SetValue(file);
  }
}


