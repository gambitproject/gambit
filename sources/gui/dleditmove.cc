//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a move
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "game/efg.h"
#include "valnumber.h"
#include "dleditmove.h"

//======================================================================
//                      class dialogEditMove
//======================================================================

const int idLISTBOX_ACTIONS = 1000;
const int idBUTTON_ACTION_ADDBEFORE = 1001;
const int idBUTTON_ACTION_ADDAFTER = 1002;
const int idBUTTON_ACTION_DELETE = 1003;

BEGIN_EVENT_TABLE(dialogEditMove, wxDialog)
  EVT_LISTBOX(idLISTBOX_ACTIONS, dialogEditMove::OnActionChanged)
  EVT_BUTTON(idBUTTON_ACTION_ADDBEFORE, dialogEditMove::OnAddActionBefore)
  EVT_BUTTON(idBUTTON_ACTION_ADDAFTER, dialogEditMove::OnAddActionAfter)
  EVT_BUTTON(idBUTTON_ACTION_DELETE, dialogEditMove::OnDeleteAction)
  EVT_BUTTON(wxID_OK, dialogEditMove::OnOK)
END_EVENT_TABLE()

dialogEditMove::dialogEditMove(wxWindow *p_parent, Infoset *p_infoset)
  : wxDialog(p_parent, -1, "Move properties"), m_infoset(p_infoset)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, "Information set label"),
		  0, wxALL | wxCENTER, 5);
  m_infosetName = new wxTextCtrl(this, -1,
				  (const char *) p_infoset->GetName());
  labelSizer->Add(m_infosetName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 0);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Number of members: %d",
						  p_infoset->NumMembers())),
		0, wxALL, 5);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, "Belongs to player"),
		   0, wxALL, 5);
  m_player = new wxChoice(this, -1);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append("Chance");
    m_player->SetSelection(0);
  }
  else {
    for (int pl = 1; pl <= p_infoset->Game()->NumPlayers(); pl++) {
      m_player->Append(wxString::Format("%d: %s", pl,
					(char *) 
					p_infoset->Game()->Players()[pl]->GetName()));
    } 
    m_player->SetSelection(p_infoset->GetPlayer()->GetNumber() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 0);

  wxStaticBoxSizer *actionBoxSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Actions"), wxHORIZONTAL);
 
  m_actionList = new wxListBox(this, idLISTBOX_ACTIONS);
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append((const char *)
			 (ToText(act) + ": " + 
			  p_infoset->GetAction(act)->GetName()));
    m_actionNames.Append(p_infoset->GetAction(act)->GetName());
    if (p_infoset->IsChanceInfoset()) {
      m_actionProbs.Append(p_infoset->Game()->GetChanceProbs(p_infoset)[act]);
    }
    m_actions.Append(p_infoset->GetAction(act));
  }
  m_actionList->SetSelection(0);
  m_lastSelection = 0;
  actionBoxSizer->Add(m_actionList, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *rhsSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer *editNameSizer = new wxBoxSizer(wxVERTICAL);
  editNameSizer->Add(new wxStaticText(this, wxID_STATIC, "Action name"),
		     0, wxTOP | wxCENTER, 5);
  m_actionName = new wxTextCtrl(this, -1, "");
  editNameSizer->Add(m_actionName, 0, wxALL | wxEXPAND, 5);
  m_actionName->SetValue((const char *) m_actionNames[1]);
  editSizer->Add(editNameSizer, 0, wxALL, 5);

  if (p_infoset->IsChanceInfoset()) {
    wxBoxSizer *editProbSizer = new wxBoxSizer(wxVERTICAL);
    editProbSizer->Add(new wxStaticText(this, wxID_STATIC, "Probability"),
		       0, wxTOP | wxCENTER, 5);
    m_actionProbValue = ToText(p_infoset->Game()->GetChanceProbs(p_infoset)[1]);
    m_actionProb = new wxTextCtrl(this, -1, "",
				  wxDefaultPosition, wxDefaultSize,
				  0, gNumberValidator(&m_actionProbValue,
						      0, 1),
				  "action probability");
    editProbSizer->Add(m_actionProb, 0, wxALL | wxEXPAND, 5);
    editSizer->Add(editProbSizer, 0, wxALL, 5);
  }
  rhsSizer->Add(editSizer, 0, wxALL, 5);

  wxBoxSizer *addRemoveSizer = new wxBoxSizer(wxVERTICAL);
  m_addBeforeButton = new wxButton(this, idBUTTON_ACTION_ADDBEFORE, 
				   "Add action before");
  addRemoveSizer->Add(m_addBeforeButton, 0, wxALL | wxEXPAND, 5);
  m_addAfterButton = new wxButton(this, idBUTTON_ACTION_ADDAFTER,
				  "Add action after");
  addRemoveSizer->Add(m_addAfterButton, 0, wxALL | wxEXPAND, 5);
  m_deleteButton = new wxButton(this, idBUTTON_ACTION_DELETE,
				"Delete action");
  addRemoveSizer->Add(m_deleteButton, 0, wxALL | wxEXPAND, 5);
  m_deleteButton->Enable(m_actionList->GetCount() > 1);
  rhsSizer->Add(addRemoveSizer, 0, wxALL | wxCENTER, 5);

  actionBoxSizer->Add(rhsSizer, 0, wxALL, 5);
  topSizer->Add(actionBoxSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogEditMove::OnActionChanged(wxCommandEvent &)
{ 
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().c_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = ToNumber(m_actionProb->GetValue().c_str());
  }
  Action *action = m_actions[m_lastSelection+1];
  if (action) {
    m_actionList->SetString(m_lastSelection,
			    (const char *) 
			    ((ToText(action->GetNumber()) + ": " +
			      m_actionName->GetValue().c_str())));
  }
  else {
    m_actionList->SetString(m_lastSelection, m_actionName->GetValue());
  }
  m_actionName->SetValue((const char *)
			 m_actionNames[m_actionList->GetSelection() + 1]);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProb->SetValue((const char *)
			   ToText(m_actionProbs[m_actionList->GetSelection()+1]));
  }
  m_lastSelection = m_actionList->GetSelection();
}

void dialogEditMove::OnAddActionBefore(wxCommandEvent &)
{
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().c_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(m_actionProb->GetValue().c_str());
  }
  m_actionList->SetString(m_lastSelection,
			  (const char *) ((ToText(m_lastSelection+1) + ": " +
					   m_actionName->GetValue().c_str())));
  
  m_actionNames.Insert("NewAction", m_actionList->GetSelection() + 1);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs.Insert(0, m_actionList->GetSelection() + 1);
  }
  m_actions.Insert(0, m_actionList->GetSelection() + 1);
  wxArrayString labels;
  labels.Add("NewAction");
  m_actionList->InsertItems(labels, m_actionList->GetSelection());
  m_actionList->SetSelection(m_lastSelection);
  m_actionName->SetValue((const char *)
			 m_actionNames[m_actionList->GetSelection() + 1]);
  m_deleteButton->Enable(true);
}

void dialogEditMove::OnAddActionAfter(wxCommandEvent &)
{
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().c_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(m_actionProb->GetValue().c_str());
  }
  m_actionList->SetString(m_lastSelection,
			  (const char *) ((ToText(m_lastSelection+1) + ": " +
					   m_actionName->GetValue().c_str())));
  
  m_actionNames.Insert("NewAction", m_actionList->GetSelection() + 2);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs.Insert(0, m_actionList->GetSelection() + 2);
  }
  m_actions.Insert(0, m_actionList->GetSelection() + 2);
  wxArrayString labels;
  labels.Add("NewAction");
  m_actionList->InsertItems(labels, m_actionList->GetSelection() + 1);
  m_lastSelection++;
  m_actionList->SetSelection(m_lastSelection);
  m_actionName->SetValue((const char *)
			 m_actionNames[m_actionList->GetSelection() + 1]);
  m_deleteButton->Enable(true);
}

void dialogEditMove::OnDeleteAction(wxCommandEvent &)
{
  m_actionNames.Remove(m_actionList->GetSelection() + 1);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs.Remove(m_actionList->GetSelection() + 1);
  }
  m_actions.Remove(m_actionList->GetSelection() + 1);
  m_actionList->Delete(m_actionList->GetSelection());
  if (m_lastSelection >= m_actionList->GetCount()) {
    m_lastSelection = m_actionList->GetCount() - 1;
  }
  m_actionList->SetSelection(m_lastSelection);
  m_actionName->SetValue((const char *)
			 m_actionNames[m_actionList->GetSelection() + 1]);
  m_deleteButton->Enable(m_actionList->GetCount() > 1);
}

void dialogEditMove::OnOK(wxCommandEvent &p_event)
{
  // Copy any edited data into the blocks
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().c_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(m_actionProb->GetValue().c_str());
  }
  // Go on with usual processing
  p_event.Skip();
}





