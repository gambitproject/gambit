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

dialogEditMove::dialogEditMove(wxWindow *p_parent, gbtEfgInfoset p_infoset)
  : wxDialog(p_parent, -1, _("Move properties"), wxDefaultPosition), 
    m_infoset(p_infoset)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC,
				   _("Information set label")),
		  0, wxALL | wxCENTER, 5);
  m_infosetName = new wxTextCtrl(this, -1,
				 wxString::Format(wxT("%s"),
						  (const char *) p_infoset->GetLabel()));
  labelSizer->Add(m_infosetName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 0);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format(_("Number of members: %d"),
						  p_infoset->NumMembers())),
		0, wxALL, 5);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Belongs to player")),
		   0, wxALL, 5);
  m_player = new wxChoice(this, -1);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append(_("Chance"));
    m_player->SetSelection(0);
  }
  else {
    for (int pl = 1; pl <= p_infoset->GetGame()->NumPlayers(); pl++) {
      m_player->Append(wxString::Format(wxT("%d: %s"), pl,
					(char *) 
					p_infoset->GetGame()->GetPlayer(pl)->GetLabel()));
    } 
    m_player->SetSelection(p_infoset->GetPlayer()->GetId() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 0);

  wxStaticBoxSizer *actionBoxSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Actions")),
			 wxHORIZONTAL);
 
  m_actionList = new wxListBox(this, idLISTBOX_ACTIONS,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0, wxLB_SINGLE);
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    m_actionList->Append(wxString::Format(wxT("%s"),
					  (const char *)
					  (ToText(act) + ": " + 
					   p_infoset->GetAction(act)->GetLabel())));
    m_actionNames.Append(p_infoset->GetAction(act)->GetLabel());
    if (p_infoset->IsChanceInfoset()) {
      m_actionProbs.Append(p_infoset->GetChanceProb(act));
    }
    m_actions.Append(p_infoset->GetAction(act));
  }
  m_actionList->SetSelection(0);
  m_lastSelection = 0;
  actionBoxSizer->Add(m_actionList, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *rhsSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer *editNameSizer = new wxBoxSizer(wxVERTICAL);
  editNameSizer->Add(new wxStaticText(this, wxID_STATIC, _("Action name")),
		     0, wxTOP | wxCENTER, 5);
  m_actionName = new wxTextCtrl(this, -1, wxT(""));
  editNameSizer->Add(m_actionName, 0, wxALL | wxEXPAND, 5);
  m_actionName->SetValue(wxString::Format(wxT("%s"),
					  (const char *) m_actionNames[1]));
  editSizer->Add(editNameSizer, 0, wxALL, 5);

  if (p_infoset->IsChanceInfoset()) {
    wxBoxSizer *editProbSizer = new wxBoxSizer(wxVERTICAL);
    editProbSizer->Add(new wxStaticText(this, wxID_STATIC, _("Probability")),
		       0, wxTOP | wxCENTER, 5);
    m_actionProbValue = wxString::Format(wxT("%s"),
					 (char *) ToText(p_infoset->GetChanceProb(1)));
    m_actionProb = new wxTextCtrl(this, -1, wxT(""),
				  wxDefaultPosition, wxDefaultSize,
				  0, gbtNumberValidator(&m_actionProbValue,
						      0, 1),
				  wxT("action probability"));
    editProbSizer->Add(m_actionProb, 0, wxALL | wxEXPAND, 5);
    editSizer->Add(editProbSizer, 0, wxALL, 5);
  }
  rhsSizer->Add(editSizer, 0, wxALL, 5);

  wxBoxSizer *addRemoveSizer = new wxBoxSizer(wxVERTICAL);
  m_addBeforeButton = new wxButton(this, idBUTTON_ACTION_ADDBEFORE, 
				   _("Add action before"));
  addRemoveSizer->Add(m_addBeforeButton, 0, wxALL | wxEXPAND, 5);
  m_addAfterButton = new wxButton(this, idBUTTON_ACTION_ADDAFTER,
				  _("Add action after"));
  addRemoveSizer->Add(m_addAfterButton, 0, wxALL | wxEXPAND, 5);
  m_deleteButton = new wxButton(this, idBUTTON_ACTION_DELETE,
				_("Delete action"));
  addRemoveSizer->Add(m_deleteButton, 0, wxALL | wxEXPAND, 5);
  m_deleteButton->Enable(m_actionList->GetCount() > 1);
  rhsSizer->Add(addRemoveSizer, 0, wxALL | wxCENTER, 5);

  actionBoxSizer->Add(rhsSizer, 0, wxALL, 5);
  topSizer->Add(actionBoxSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, _("Help")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void dialogEditMove::OnActionChanged(wxCommandEvent &)
{ 
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().mb_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = ToNumber(gbtText(m_actionProb->GetValue().mb_str()));
  }
  gbtEfgAction action = m_actions[m_lastSelection+1];
  if (!action.IsNull()) {
    m_actionList->SetString(m_lastSelection,
			    wxString::Format(wxT("%s"),
					     (const char *) 
					     ((ToText(action->GetId()) + ": " +
					       gbtText(m_actionName->GetValue().mb_str())))));
  }
  else {
    m_actionList->SetString(m_lastSelection, m_actionName->GetValue());
  }
  m_actionName->SetValue(wxString::Format(wxT("%s"),
					  (const char *)
					  m_actionNames[m_actionList->GetSelection() + 1]));
  if (m_infoset->IsChanceInfoset()) {
    m_actionProb->SetValue(wxString::Format(wxT("%s"),
					    (const char *)
					    ToText(m_actionProbs[m_actionList->GetSelection()+1])));
  }
  m_lastSelection = m_actionList->GetSelection();
}

void dialogEditMove::OnAddActionBefore(wxCommandEvent &)
{
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().mb_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(gbtText(m_actionProb->GetValue().mb_str()));
  }
  gbtEfgAction action = m_actions[m_lastSelection+1];
  if (!action.IsNull()) {
    m_actionList->SetString(m_lastSelection,
			    wxString::Format(wxT("%s"),
					     (const char *) ((ToText(action->GetId()) + ": " +
							      gbtText(m_actionName->GetValue().mb_str())))));
  }
  else {
    m_actionList->SetString(m_lastSelection, m_actionName->GetValue());
  }
  
  m_actionNames.Insert("NewAction", m_lastSelection + 1);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs.Insert(0, m_lastSelection + 1);
  }
  m_actions.Insert(0, m_lastSelection + 1);
  wxArrayString labels;
  labels.Add(_("NewAction"));
  m_actionList->InsertItems(labels, m_lastSelection);
  // Even though this is a single-choice listbox, need to explicitly
  // deselect old item -- at least under GTK with wx2.3.3
  m_actionList->SetSelection(m_lastSelection + 1, false);
  m_actionList->SetSelection(m_lastSelection);
  m_actionName->SetValue(wxString::Format(wxT("%s"),
					  (const char *) m_actionNames[m_lastSelection + 1]));
  if (m_infoset->IsChanceInfoset()) {
    m_actionProb->SetValue(wxString::Format(wxT("%s"),
					    (const char *) 
					    ToText(m_actionProbs[m_lastSelection + 1])));
  }
  m_deleteButton->Enable(true);
}

void dialogEditMove::OnAddActionAfter(wxCommandEvent &)
{
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().mb_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(gbtText(m_actionProb->GetValue().mb_str()));
  }
  gbtEfgAction action = m_actions[m_lastSelection+1];
  if (!action.IsNull()) {
    m_actionList->SetString(m_lastSelection,
			    wxString::Format(wxT("%s"),
					     (const char *) ((ToText(action->GetId()) + ": " +
							      gbtText(m_actionName->GetValue().mb_str())))));
  }
  else {
    m_actionList->SetString(m_lastSelection, m_actionName->GetValue());
  }
  
  m_actionNames.Insert("NewAction", m_lastSelection + 2);
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs.Insert(0, m_lastSelection + 2);
  }
  m_actions.Insert(0, m_lastSelection + 2);
  wxArrayString labels;
  labels.Add(_("NewAction"));
  m_actionList->InsertItems(labels, m_lastSelection + 1);
  // Even though this is a single-choice listbox, need to explicitly
  // deselect old item -- at least under GTK with wx2.3.3
  m_actionList->SetSelection(m_lastSelection, false);
  m_actionList->SetSelection(++m_lastSelection);
  m_actionName->SetValue(wxString::Format(wxT("%s"),
					  (const char *)
					  m_actionNames[m_lastSelection + 1]));
  if (m_infoset->IsChanceInfoset()) {
    m_actionProb->SetValue(wxString::Format(wxT("%s"),
					    (const char *) 
					    ToText(m_actionProbs[m_lastSelection + 1])));
  }
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
  m_actionName->SetValue(wxString::Format(wxT("%s"),
					  (const char *)
					  m_actionNames[m_actionList->GetSelection() + 1]));
  if (m_infoset->IsChanceInfoset()) {
    m_actionProb->SetValue(wxString::Format(wxT("%s"),
					    (const char *) 
					    ToText(m_actionProbs[m_lastSelection + 1])));
  }
  m_deleteButton->Enable(m_actionList->GetCount() > 1);
}

void dialogEditMove::OnOK(wxCommandEvent &p_event)
{
  // Copy any edited data into the blocks
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().mb_str();
  if (m_infoset->IsChanceInfoset()) {
    m_actionProbs[m_lastSelection+1] = 
      ToNumber(gbtText(m_actionProb->GetValue().mb_str()));
  }
  // Go on with usual processing
  p_event.Skip();
}

//========================================================================
//                     Command class for editing move
//========================================================================

class gbtCmdEditMove : public gbtGameCommand {
private:
  gbtEfgInfoset m_infoset;
  gbtText m_infosetLabel;
  int m_infosetPlayer;
  gbtBlock<gbtEfgAction> m_actions;
  gbtBlock<gbtText> m_actionLabels;
  gbtBlock<gbtNumber> m_actionProbs;

public:
  gbtCmdEditMove(gbtEfgInfoset p_infoset, const gbtText &p_infosetLabel,
		 int p_infosetPlayer,
		 const gbtBlock<gbtEfgAction> &p_actions,
		 const gbtBlock<gbtText> &p_actionLabels,
		 const gbtBlock<gbtNumber> &p_actionProbs)
    : m_infoset(p_infoset), m_infosetLabel(p_infosetLabel), 
      m_infosetPlayer(p_infosetPlayer),
      m_actions(p_actions), m_actionLabels(p_actionLabels),
      m_actionProbs(p_actionProbs)  { }
  virtual ~gbtCmdEditMove() { }

  void Do(gbtGameDocument *); 

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdEditMove::Do(gbtGameDocument *p_doc)
{
  m_infoset->SetLabel(m_infosetLabel);
    
  if (!m_infoset->IsChanceInfoset() && 
      m_infosetPlayer != m_infoset->GetPlayer()->GetId()) {
    m_infoset->SetPlayer(p_doc->GetEfg()->GetPlayer(m_infosetPlayer));
  }

  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    if (!m_actions.Find(m_infoset->GetAction(act))) {
      m_infoset->GetAction(act)->DeleteAction();
      act--;
    }
  }

  int insertAt = 1;
  for (int act = 1; act <= m_actions.Length(); act++) {
    gbtEfgAction action = m_actions[act];
    if (!action.IsNull()) {
      action->SetLabel(m_actionLabels[act]);
      if (m_infoset->IsChanceInfoset()) {
	p_doc->GetEfg()->SetChanceProb(m_infoset, action->GetId(),
				      m_actionProbs[act]);
      }
      insertAt = m_actions[act]->GetId() + 1;
    }
    else if (insertAt > m_infoset->NumActions()) {
      gbtEfgAction newAction = p_doc->GetEfg()->InsertAction(m_infoset);
      insertAt++;
      newAction->SetLabel(m_actionLabels[act]);
      if (m_infoset->IsChanceInfoset()) {
	p_doc->GetEfg()->SetChanceProb(m_infoset, newAction->GetId(), 
				      m_actionProbs[act]);
      }
    }
    else {
      gbtEfgAction newAction =
	p_doc->GetEfg()->InsertAction(m_infoset,
				     m_infoset->GetAction(insertAt++));
      newAction->SetLabel(m_actionLabels[act]);
      if (m_infoset->IsChanceInfoset()) {
	p_doc->GetEfg()->SetChanceProb(m_infoset, newAction->GetId(), 
				      m_actionProbs[act]);
      }
    }
  }
}


gbtGameCommand *dialogEditMove::GetCommand(void) const
{
  return new gbtCmdEditMove(m_infoset,
			    (const char *) m_infosetName->GetValue().mb_str(),
			    m_player->GetSelection() + 1,
			    m_actions, m_actionNames, m_actionProbs);
}
