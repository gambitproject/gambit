//
// FILE: dlefg.cc -- Extensive form-related dialog implementations
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/sizer.h"

#include "guishare/wxmisc.h"

#include "efgconst.h"

#include "efg.h"
#include "efstrat.h"

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
#include "dlefgplayers.h"
#include "dlinfosets.h"
#include "dlsubgames.h"

//=========================================================================
//                  dialogEfgSelectPlayer: Member functions
//=========================================================================

dialogEfgSelectPlayer::dialogEfgSelectPlayer(const Efg::Game &p_efg, bool p_chance,
					     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Select Player"),
    m_efg(p_efg), m_chance(p_chance)
{
  m_playerNameList = new wxListBox(this, -1);
  if (m_chance)
    m_playerNameList->Append("Chance");

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append((char *) (ToText(pl) + ": " + m_efg.Players()[pl]->GetName()));
  }
  m_playerNameList->SetSelection(0);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_playerNameList, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
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

BEGIN_EVENT_TABLE(dialogMoveAdd, guiAutoDialog)
  EVT_LISTBOX(idMOVE_PLAYER_LISTBOX, dialogMoveAdd::OnPlayer)
  EVT_LISTBOX(idMOVE_INFOSET_LISTBOX, dialogMoveAdd::OnInfoset)
END_EVENT_TABLE()

dialogMoveAdd::dialogMoveAdd(wxWindow *p_parent, FullEfg &p_efg,
			     const gText &p_title, EFPlayer *p_player,
			     Infoset *p_infoset, int p_branches)
  : guiAutoDialog(p_parent, p_title),
    m_efg(p_efg), m_branches(p_branches)
{
  m_playerItem = new wxListBox(this, idMOVE_PLAYER_LISTBOX);
  m_playerItem->Append("Chance");
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + m_efg.Players()[pl]->GetName()));
  }
  m_playerItem->Append("New Player");
  if (p_player)
    m_playerItem->SetSelection(p_player->GetNumber());
  else
    m_playerItem->SetSelection(0);

  m_infosetItem = new wxListBox(this, idMOVE_INFOSET_LISTBOX);
  m_infosetItem->Append("New");
  if (p_player) {
    for (int iset = 1; iset <= p_player->NumInfosets(); iset++) {
      m_infosetItem->Append((char *) (ToText(iset) + ": " +
			    p_player->Infosets()[iset]->GetName()));
    }
  }
  if (p_infoset)
    m_infosetItem->SetSelection(p_infoset->GetNumber());
  else
    m_infosetItem->SetSelection(0);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxVERTICAL);
  playerSizer->Add(new wxStaticText(this, -1, "Player"),0, wxALL, 5);
  playerSizer->Add(m_playerItem, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *infosetSizer = new wxBoxSizer(wxVERTICAL);
  infosetSizer->Add(new wxStaticText(this, -1, "Infoset"),
		    0, wxALL, 5);
  infosetSizer->Add(m_infosetItem, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *playerInfosetSizer = new wxBoxSizer(wxHORIZONTAL);
  playerInfosetSizer->Add(playerSizer, 1, wxALL, 5);
  playerInfosetSizer->Add(infosetSizer, 1, wxALL, 5);

  wxBoxSizer *actionSizer = new wxBoxSizer(wxHORIZONTAL);
  actionSizer->Add(new wxStaticText(this, -1, "Number of actions"),
		   0, wxALL, 5);
  m_actionItem = new wxTextCtrl(this, -1, "Actions");
  m_actionItem->SetValue((char *) ToText(p_branches));
  m_actionItem->Enable(m_infosetItem->GetSelection() == 0);
  actionSizer->Add(m_actionItem, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(playerInfosetSizer, 0, wxCENTRE | wxALL, 5);
  topSizer->Add(actionSizer, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

void dialogMoveAdd::OnPlayer(wxCommandEvent &)
{
  int playerNumber = m_playerItem->GetSelection(); 

  EFPlayer *player = 0;
  if (playerNumber == 0)
    player = m_efg.GetChance();
  else if (playerNumber <= m_efg.NumPlayers())
    player = m_efg.Players()[playerNumber];

  m_infosetItem->Clear();
  m_infosetItem->Append("New");
  if (player) {
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_infosetItem->Append((char *) (ToText(iset) + ": " +
			    player->Infosets()[iset]->GetName()));
    }
  }
  m_infosetItem->SetSelection(0);
  m_actionItem->SetValue("2");
  m_actionItem->Enable(true);
}

void dialogMoveAdd::OnInfoset(wxCommandEvent &)
{
  int infosetNumber = m_infosetItem->GetSelection();

  if (infosetNumber > 0) {
    int playerNumber = m_playerItem->GetSelection();
    Infoset *infoset;
    if (playerNumber == 0)
      infoset = m_efg.GetChance()->Infosets()[infosetNumber];
    else
      infoset = m_efg.Players()[playerNumber]->Infosets()[infosetNumber];
    m_actionItem->Enable(false);
    m_actionItem->SetValue((char *) ToText(infoset->NumActions()));
  }
  else {
    m_actionItem->Enable(true);
    m_actionItem->SetValue("2");
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
    player->SetName("Player " + ToText(m_efg.NumPlayers()));
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
    return (int) ToDouble(m_actionItem->GetValue().c_str());
}

//=========================================================================
//                   dialogNodeDelete: Member functions
//=========================================================================


dialogNodeDelete::dialogNodeDelete(Node *p_node, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Delete Node"), m_node(p_node)
{
  m_branchList = new wxListBox(this, -1);
  for (int  act = 1; act <= m_node->Game()->NumChildren(m_node); act++) {
    m_branchList->Append((char *) (ToText(act) + ": " + 
				   m_node->GetInfoset()->Actions()[act]->GetName()));
  }
  m_branchList->SetSelection(0);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1, "Keep subtree at branch"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_branchList, 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetAutoLayout(TRUE);
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
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

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
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
  m_actionList = new wxListBox(this, -1);

  const gArray<Action *> &acts(p_infoset->Actions());
  for (int act = 1; act <= acts.Length(); act++) 
    m_actionList->Append((char *) (ToText(act) + ": " + acts[act]->GetName()));
  m_actionList->SetSelection(0);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1, (char *)p_label), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_actionList, 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

//=========================================================================
//                   dialogActionProbs: Member functions
//=========================================================================

dialogActionProbs::dialogActionProbs(Infoset *p_infoset, wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Action Probabilities", p_infoset->NumActions()),
    m_infoset(p_infoset)
{

  const gArray<Action *> &acts(m_infoset->Actions());
  for (int act = 1; act <= acts.Length(); act++) 
    SetValue(act, ToText(m_infoset->Game()->GetChanceProb(acts[act])));

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

gNumber dialogActionProbs::GetActionProb(int p_action) const
{ return ToNumber(GetValue(p_action)); }

//=========================================================================
//                    dialogEfgPayoffs: Member functions
//=========================================================================

dialogEfgPayoffs::dialogEfgPayoffs(const FullEfg &p_efg, 
				   const Efg::Outcome &p_outcome,
				   wxWindow *p_parent)
  : guiPagedDialog(p_parent, "Change Payoffs", p_efg.NumPlayers()),
    m_outcome(p_outcome), m_efg(p_efg)
{
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    EFPlayer *player = m_efg.Players()[pl];
    SetValue(pl, ToText(m_efg.Payoff(p_outcome, player)));
  }

  m_outcomeName = new wxTextCtrl(this, -1);
  if (!p_outcome.IsNull()) {
    m_outcomeName->SetValue((char *) m_efg.GetOutcomeName(p_outcome));
  }
  else {
    m_outcomeName->SetValue((char *) ("Outcome" + ToText(p_efg.NumOutcomes() + 1)));
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_outcomeName, 0, wxALL, 5);
  topSizer->Add(m_grid, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
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
  return m_outcomeName->GetValue().c_str();
}

//=========================================================================
//                  dialogInfosetReveal: Member functions
//=========================================================================

dialogInfosetReveal::dialogInfosetReveal(const Efg::Game &p_efg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Reveal to Players"), m_efg(p_efg)
{
#ifdef __WXGTK__
  // the wxGTK multiple-selection listbox is flaky (2.1.11)
  m_playerNameList = new wxListBox(this, -1, wxDefaultPosition,
				   wxDefaultSize, 0, 0, wxLB_EXTENDED);
#else
  m_playerNameList = new wxListBox(this, -1, wxDefaultPosition,
				   wxDefaultSize, 0, 0, wxLB_MULTIPLE);
#endif // __WXGTK__

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append((char *) (ToText(pl) + ": " + 
			     m_efg.Players()[pl]->GetName()));
    m_playerNameList->SetSelection(pl - 1, TRUE);
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1, "Players:"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_playerNameList, 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
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

BEGIN_EVENT_TABLE(dialogInfosets, guiAutoDialog)
  EVT_LISTBOX(idINFOSET_PLAYER_LISTBOX, dialogInfosets::OnPlayer)
  EVT_LISTBOX(idINFOSET_INFOSET_LISTBOX, dialogInfosets::OnInfoset)
  EVT_BUTTON(idINFOSET_EDIT_BUTTON, dialogInfosets::OnEdit)
  EVT_BUTTON(idINFOSET_NEW_BUTTON, dialogInfosets::OnNew)
  EVT_BUTTON(idINFOSET_REMOVE_BUTTON, dialogInfosets::OnRemove)
END_EVENT_TABLE()

dialogInfosets::dialogInfosets(FullEfg &p_efg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Infoset Information"), m_efg(p_efg),
    m_gameChanged(false), m_prevInfoset(0)
{
  m_playerItem = new wxListBox(this, idINFOSET_PLAYER_LISTBOX);
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + m_efg.Players()[pl]->GetName()));
  }
  m_playerItem->SetSelection(0);
  m_infosetItem = new wxListBox(this, idINFOSET_INFOSET_LISTBOX);
  m_actionsItem = new wxTextCtrl(this, -1, "",
				 wxDefaultPosition, wxDefaultSize,
				 wxTE_READONLY);
  m_membersItem = new wxTextCtrl(this, -1, "",
				 wxDefaultPosition, wxDefaultSize,
				 wxTE_READONLY);
  m_editButton = new wxButton(this, idINFOSET_EDIT_BUTTON, "Edit...");

  wxButton *newButton = new wxButton(this, idINFOSET_NEW_BUTTON, "New");

  m_removeButton = new wxButton(this, idINFOSET_REMOVE_BUTTON, "Remove");

  m_cancelButton->Show(FALSE);

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *midSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  leftSizer->Add(new wxStaticText(this, -1, "Player:"), 0, wxCENTRE | wxALL, 5);
  leftSizer->Add(m_playerItem, 0, wxCENTRE | wxALL, 5);

  midSizer->Add(new wxStaticText(this, -1, "Infoset:"), 0, wxCENTRE | wxALL, 5);
  midSizer->Add(m_infosetItem, 0, wxCENTRE | wxALL, 5);

  rightSizer->Add(new wxStaticText(this, -1, "Actions:"), 0, wxCENTRE | wxALL, 5);
  rightSizer->Add(m_actionsItem, 0, wxCENTRE | wxALL, 5);
  rightSizer->Add(new wxStaticText(this, -1, "Members:"), 0, wxCENTRE | wxALL, 5);
  rightSizer->Add(m_membersItem, 0, wxCENTRE | wxALL, 5);

  topSizer->Add(leftSizer, 0, wxALL, 5);
  topSizer->Add(midSizer, 0, wxALL, 5);
  topSizer->Add(rightSizer, 0, wxALL, 5);

  botSizer->Add(m_editButton, 0, wxALL, 5);
  botSizer->Add(newButton, 0, wxALL, 5);
  botSizer->Add(m_removeButton, 0, wxALL, 5);

  allSizer->Add(topSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(botSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
  wxCommandEvent event;
  OnPlayer(event);
}

void dialogInfosets::OnPlayer(wxCommandEvent &)
{
  int p_number = m_playerItem->GetSelection();
  EFPlayer *player = m_efg.Players()[p_number+1];
  m_infosetItem->Clear();
  if (player->NumInfosets() > 0)  {
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      m_infosetItem->Append((char *) (ToText(iset) + ": " + player->Infosets()[iset]->GetName()));
    m_infosetItem->SetSelection(0);
    m_infosetItem->Enable(TRUE);
    m_editButton->Enable(TRUE);
    wxCommandEvent event;
    OnInfoset(event);
  }
  else {
    m_infosetItem->Append("(None)");
    m_infosetItem->Enable(FALSE);
    m_editButton->Enable(FALSE);
    m_removeButton->Enable(FALSE);
  }
}

void dialogInfosets::OnInfoset(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];

  m_actionsItem->SetValue((char *) ToText(infoset->NumActions()));
  m_membersItem->SetValue((char *) ToText(infoset->NumMembers()));

  m_removeButton->Enable(infoset->NumMembers() == 0);
}

void dialogInfosets::OnEdit(wxCommandEvent &)
{
  int selection = m_infosetItem->GetSelection();
  gText defaultName = (m_efg.Players()[m_playerItem->GetSelection()+1]->
                       Infosets()[selection+1])->GetName();

  gText newName = wxGetTextFromUser("Name", "Enter Name", (char *) defaultName, this).c_str();
  if (newName != "") {
	 m_efg.Players()[m_playerItem->GetSelection()+1]->
      Infosets()[selection+1]->SetName(newName);
	 m_infosetItem->SetString(selection, (char *) (ToText(selection + 1) + ": " + newName));
  }
}

void dialogInfosets::OnNew(wxCommandEvent &)
{
  static int s_numActions = 2;

  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];

  gText prompt = "Number of actions at new information set for ";
  if (player->GetName() != "")
    prompt += player->GetName();
  else
    prompt += "Player " + ToText(player->GetNumber());

  gText reply = wxGetTextFromUser((char *) prompt, "New Infoset",
				  (char *) ToText(s_numActions)).c_str();
  int numActions = atoi(reply);

  if (numActions > 0) {
    s_numActions = numActions;
    m_efg.CreateInfoset(player, numActions);
    m_gameChanged = true;
    if (player->NumInfosets() == 1)  {
      m_infosetItem->Clear();
    }
    m_infosetItem->Append((char *) (ToText(player->NumInfosets()) + ": "));
    m_infosetItem->SetSelection(player->NumInfosets() - 1);
    wxCommandEvent event;
    OnInfoset(event);
    m_removeButton->Enable(TRUE);
    m_editButton->Enable(TRUE);
    m_infosetItem->Enable(TRUE);
  }
}

void dialogInfosets::OnRemove(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection()+1];
  m_efg.DeleteEmptyInfoset(infoset);
  m_gameChanged = true;
  wxCommandEvent event;
  OnPlayer(event);
}

//=========================================================================
//                   dialogEfgPlayers: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogEfgPlayers, guiAutoDialog)
  EVT_BUTTON(idEFPLAYERS_NEW_BUTTON, dialogEfgPlayers::OnNew)
  EVT_BUTTON(idEFPLAYERS_EDIT_BUTTON, dialogEfgPlayers::OnEdit)
END_EVENT_TABLE()

dialogEfgPlayers::dialogEfgPlayers(FullEfg &p_efg, wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Player Names"), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, -1);
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append((char *) (ToText(pl) + ": " + m_efg.Players()[pl]->GetName()));
  }
  m_playerNameList->SetSelection(0);
  m_lastSelection = 0;

  wxButton *editPlayer = new wxButton(this, idEFPLAYERS_EDIT_BUTTON, "Edit...");

  wxButton *newPlayer = new wxButton(this, idEFPLAYERS_NEW_BUTTON, "New player...");

  m_cancelButton->Show(FALSE);

  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);

  rightSizer->Add(editPlayer, 0, wxALL, 5);
  rightSizer->Add(newPlayer, 0, wxALL, 5);

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

void dialogEfgPlayers::OnEdit(wxCommandEvent &)
{
  int selection = m_playerNameList->GetSelection();
  gText defaultName = m_efg.Players()[selection + 1]->GetName();

  gText newName = wxGetTextFromUser("Name", "Enter Name", (char *) defaultName, this).c_str();
  if (newName != "") {
    m_efg.Players()[selection + 1]->SetName(newName);
    m_playerNameList->SetString(selection, (char *) (ToText(selection + 1) + ": " + newName));
  }
}

void dialogEfgPlayers::OnNew(wxCommandEvent &)
{
  gText defaultName = "Player " + ToText(m_efg.NumPlayers() + 1);
  gText newName = wxGetTextFromUser("New Player's name", "Enter Name",
                                    (char *) defaultName, this).c_str();
  if (newName != "") {
    EFPlayer *newPlayer = m_efg.NewPlayer();
    newPlayer->SetName(newName);
    m_playerNameList->Append((char *) (ToText(m_efg.NumPlayers()) + ": " + newName));
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
  m_outcomeList = new wxListBox(this, -1);
  
  for (int outc = 1; outc <= m_efg.NumOutcomes(); outc++) {
    Efg::Outcome outcome = m_efg.GetOutcome(outc);
    gText item = ToText(outc) + ": " + m_efg.GetOutcomeName(outcome);
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

Efg::Outcome dialogEfgOutcomeSelect::GetOutcome(void)
{
  return m_efg.GetOutcome(m_outcomeList->GetSelection() + 1);
}

//=========================================================================
//                   dialogEfgEditSupport: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogEfgEditSupport, guiAutoDialog)
  EVT_LISTBOX(idEFSUPPORT_PLAYER_LISTBOX, dialogEfgEditSupport::OnPlayer)
  EVT_LISTBOX(idEFSUPPORT_INFOSET_LISTBOX, dialogEfgEditSupport::OnInfoset)
  EVT_LISTBOX(idEFSUPPORT_ACTION_LISTBOX, dialogEfgEditSupport::OnAction)
END_EVENT_TABLE()

dialogEfgEditSupport::dialogEfgEditSupport(const EFSupport &p_support,
					   wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Edit support"),
    m_efg(p_support.GetGame()), m_support(p_support)
{
  m_nameItem = new wxTextCtrl(this, -1);
  m_nameItem->SetValue((char *) p_support.GetName());

  m_playerItem = new wxListBox(this, idEFSUPPORT_PLAYER_LISTBOX);
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + 
			 m_efg.Players()[pl]->GetName()));
  }
  m_playerItem->SetSelection(0);

  m_infosetItem = new wxListBox(this, idEFSUPPORT_INFOSET_LISTBOX);

#ifdef __WXGTK__
  // the wxGTK multiple-selection listbox is flaky (2.1.11)
  m_actionItem = new wxListBox(this, idEFSUPPORT_ACTION_LISTBOX,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0, wxLB_EXTENDED);
#else
  m_actionItem = new wxListBox(this, idEFSUPPORT_ACTION_LISTBOX,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0, wxLB_MULTIPLE);
#endif // __WXGTK__

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *midSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);

  leftSizer->Add(new wxStaticText(this, -1, "Player"), 0,
		 wxCENTRE | wxALL, 5);
  leftSizer->Add(m_playerItem, 0, wxCENTRE | wxALL, 5);

  midSizer->Add(new wxStaticText(this, -1, "Infoset"), 0,
		wxCENTRE | wxALL, 5);
  midSizer->Add(m_infosetItem, 0, wxCENTRE | wxALL, 5);

  rightSizer->Add(new wxStaticText(this, -1, "Actions"), 0,
		  wxCENTRE | wxALL, 5);
  rightSizer->Add(m_actionItem, 0, wxCENTRE | wxALL, 5);

  topSizer->Add(new wxStaticText(this, -1, "Name"), 0, wxCENTRE | wxALL, 5);
  topSizer->Add(m_nameItem, 1, wxEXPAND | wxALL, 5);

  botSizer->Add(leftSizer, 0, wxALL, 5);
  botSizer->Add(midSizer, 0, wxALL, 5);
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

void dialogEfgEditSupport::OnPlayer(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];

  m_infosetItem->Clear();
  m_actionItem->Clear();
  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
    m_infosetItem->Append((char *) (ToText(iset) + ": " + player->Infosets()[iset]->GetName()));
  }
  m_infosetItem->SetSelection(0);
  m_infosetItem->Enable(true);

  if (player->NumInfosets() > 0) {
    Infoset *infoset = player->Infosets()[1];
    for (int act = 1; act <= infoset->NumActions(); act++) {
      m_actionItem->Append((char *) (ToText(act) + ": " +
				     infoset->Actions()[act]->GetName()));
    }

    for (int act = 1; act <= infoset->NumActions(); act++) {
      if (m_support.Find(infoset->Actions()[act])) {
	m_actionItem->SetSelection(act - 1, true);
      }
    }
  }

  m_actionItem->Enable(true);
}

void dialogEfgEditSupport::OnInfoset(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  m_actionItem->Clear();
  for (int act = 1; act <= infoset->NumActions(); act++) {
    m_actionItem->Append((char *) (ToText(act) + ": " + infoset->Actions()[act]->GetName()));
    if (m_support.Find(infoset->Actions()[act])) {
      m_actionItem->SetSelection(act - 1, true);
    }
  }
}

void dialogEfgEditSupport::OnAction(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  for (int act = 0; act < m_actionItem->Number(); act++) {
    Action *action = infoset->Actions()[act+1];
    if (m_actionItem->Selected(act)) {
      m_support.AddAction(action);
    }
    else {
      m_support.RemoveAction(action);
    }
  }
}

gText dialogEfgEditSupport::Name(void) const
{ return m_nameItem->GetValue().c_str(); }

//=====================================================================
//                  dialogSubgames: Member functions
//=====================================================================

dialogSubgames::dialogSubgames(wxWindow *p_parent, FullEfg &p_efg)
  : guiAutoDialog(p_parent, "Subgames"),
    m_efg(p_efg)
{
  m_subgameList = new wxListCtrl(this, -1, wxDefaultPosition,
				 wxSize(200, 300),
				 wxLC_REPORT | wxLC_SINGLE_SEL);
  m_subgameList->InsertColumn(0, "Subgame Root");
  m_subgameList->InsertColumn(1, "Marked");

  gList<Node *> subgameRoots;
  LegalSubgameRoots(p_efg, subgameRoots);

  for (int i = 1; i <= subgameRoots.Length(); i++) {
    m_subgameList->InsertItem(i - 1, (char *) subgameRoots[i]->GetName());
    if (subgameRoots[i]->GetSubgameRoot() == subgameRoots[i]) {
      m_subgameList->SetItem(i - 1, 1, "True");
    }
    else {
      m_subgameList->SetItem(i - 1, 1, "False");
    }
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_subgameList, 1, wxCENTRE | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

