//
// FILE: dialoginfosets.cc -- Information set editing dialog
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

#include "efg.h"
#include "dialoginfosets.h"

const int idPLAYER_ITEM = 6001;
const int idPLAYER_NAME = 6002;
const int idNEWPLAYER_BUTTON = 6003;
const int idINFOSET_ITEM = 6011;
const int idINFOSET_NAME = 6012;

BEGIN_EVENT_TABLE(dialogInfosets, guiAutoDialog)
  EVT_LISTBOX(idPLAYER_ITEM, OnSelectPlayer)
  EVT_TEXT(idPLAYER_NAME, OnEditPlayer)
  EVT_BUTTON(idNEWPLAYER_BUTTON, OnNewPlayer)
  EVT_LISTBOX(idINFOSET_ITEM, OnSelectInfoset)
  EVT_TEXT(idINFOSET_NAME, OnEditInfoset)
END_EVENT_TABLE()

//=========================================================================
//                    dialogInfosets: Member functions
//=========================================================================

dialogInfosets::dialogInfosets(wxWindow *p_parent, FullEfg &p_efg)
  : guiAutoDialog(p_parent, "Infoset Information"), m_efg(p_efg),
    m_gameChanged(false)
{
  m_playerItem = new wxListBox(this, idPLAYER_ITEM);
  m_playerItem->SetConstraints(new wxLayoutConstraints());
  m_playerItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerItem->GetConstraints()->width.AsIs();
  m_playerItem->GetConstraints()->height.AsIs();

  m_playerName = new wxTextCtrl(this, idPLAYER_NAME);
  m_playerName->SetConstraints(new wxLayoutConstraints);
  m_playerName->GetConstraints()->top.SameAs(m_playerItem, wxBottom, 10);
  m_playerName->GetConstraints()->left.SameAs(m_playerItem, wxLeft);
  m_playerName->GetConstraints()->width.SameAs(m_playerItem, wxWidth);
  m_playerName->GetConstraints()->height.AsIs();

  m_infosetItem = new wxListBox(this, idINFOSET_ITEM);
  m_infosetItem->SetConstraints(new wxLayoutConstraints());
  m_infosetItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_infosetItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_infosetItem->GetConstraints()->width.AsIs();
  m_infosetItem->GetConstraints()->height.AsIs();

  m_infosetName = new wxTextCtrl(this, idINFOSET_NAME);
  m_infosetName->SetConstraints(new wxLayoutConstraints);
  m_infosetName->GetConstraints()->top.SameAs(m_infosetItem, wxBottom, 10);
  m_infosetName->GetConstraints()->left.SameAs(m_infosetItem, wxLeft);
  m_infosetName->GetConstraints()->width.SameAs(m_infosetItem, wxWidth);
  m_infosetName->GetConstraints()->height.AsIs();

  wxButton *newButton = new wxButton(this, idNEWPLAYER_BUTTON, "New Player");
  m_removeButton = new wxButton(this, -1, "Remove");

  newButton->SetConstraints(new wxLayoutConstraints);
  newButton->GetConstraints()->top.SameAs(m_playerName, wxBottom, 10);
  newButton->GetConstraints()->right.SameAs(m_removeButton, wxLeft, 10);
  newButton->GetConstraints()->height.AsIs();
  newButton->GetConstraints()->width.SameAs(m_removeButton, wxWidth);

  m_removeButton->SetConstraints(new wxLayoutConstraints);
  m_removeButton->GetConstraints()->centreY.SameAs(newButton, wxCentreY);
  m_removeButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_removeButton->GetConstraints()->height.AsIs();
  m_removeButton->GetConstraints()->width.AsIs();

  m_okButton->GetConstraints()->centreY.SameAs(m_removeButton, wxCentreY);
  m_okButton->GetConstraints()->left.SameAs(m_removeButton, wxRight, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_removeButton, wxWidth);

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_removeButton, wxWidth);

  m_cancelButton->Show(false);
  m_cancelButton->GetConstraints()->top.SameAs(this, wxTop);
  m_cancelButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " +
				   m_efg.Players()[pl]->GetName()));
  }
  m_playerItem->SetSelection(0);

  wxCommandEvent fakeEvent;
  OnSelectPlayer(fakeEvent);
  AutoSize();
}

void dialogInfosets::OnSelectPlayer(wxCommandEvent &p_event)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  m_playerName->SetValue((char *) player->GetName());
  
  m_infosetItem->Clear();
  wxMessageBox("ut");
  if (player->NumInfosets() > 0)  {
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      m_infosetItem->Append((char *) (ToText(iset) + ": " +
				      player->Infosets()[iset]->GetName()));
    m_infosetItem->SetSelection(0);
    m_infosetItem->Enable(true);
    OnSelectInfoset(p_event);
  }
  else {
    m_infosetItem->Append("(None)");
    m_infosetItem->Enable(false);
    m_infosetName->SetValue("");
    m_infosetName->Enable(false);
    m_removeButton->Enable(false);
  }
}

void dialogInfosets::OnEditPlayer(wxCommandEvent &)
{
  int selection = m_playerItem->GetSelection();
  m_playerItem->SetString(selection, (char *) (ToText(selection + 1) + ": " +
					       m_playerName->GetValue().c_str()));
  m_efg.Players()[selection+1]->SetName(m_playerName->GetValue().c_str());
}

void dialogInfosets::OnNewPlayer(wxCommandEvent &)
{
  int playerNumber = m_efg.NumPlayers() + 1;
  gText newName = "Player " + ToText(playerNumber);
  EFPlayer *player = m_efg.NewPlayer();
  player->SetName(newName);
  m_playerItem->Append((char *) (ToText(playerNumber) + ": " + newName));
  m_playerItem->SetSelection(playerNumber - 1);

  wxCommandEvent fakeEvent;
  OnSelectPlayer(fakeEvent);
}

void dialogInfosets::OnSelectInfoset(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  m_infosetName->SetValue((char *) infoset->GetName());

  m_removeButton->Enable(infoset->NumMembers() == 0);
}

void dialogInfosets::OnEditInfoset(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  m_infosetItem->SetString(m_infosetItem->GetSelection(),
			   (char *) (ToText(m_infosetItem->GetSelection() + 1)
				     + ": " +
				     m_infosetName->GetValue().c_str()));
  infoset->SetName(m_infosetName->GetValue().c_str());
}

void dialogInfosets::OnEdit(wxCommandEvent &)
{
  /*
  int selection = m_infosetItem->GetSelection();
  gText defaultName = (m_efg.Players()[m_playerItem->GetSelection()+1]->
                       Infosets()[selection+1])->GetName();

  m_efg.Players()[m_playerItem->GetSelection()+1]->
    Infosets()[selection+1]->SetName(newName);
	 m_infosetItem->SetString(selection, ToText(selection + 1) + ": " + newName);
  }
  */
}

void dialogInfosets::NewInfoset(void)
{
  /*
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
    m_removeButton->Enable(true);
    m_editButton->Enable(true);
    m_infosetItem->Enable(true);
  }
  */
}

void dialogInfosets::RemoveInfoset(void)
{
  /*
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection()+1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection()+1];
  m_efg.DeleteEmptyInfoset(infoset);
  m_gameChanged = true;
  */
}
