//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of dialogs for extensive form window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "guishare/wxmisc.h"

#include "efgconst.h"

#include "game/efg.h"
#include "game/efstrat.h"

#include "dlefgdelete.h"
#include "dlefgreveal.h"
#include "dlefgeditsupport.h"


//=========================================================================
//                   dialogEfgDelete: Member functions
//=========================================================================

const int idRADIOBOX_DELETE_TREE = 500;

BEGIN_EVENT_TABLE(dialogEfgDelete, wxDialog)
  EVT_RADIOBOX(idRADIOBOX_DELETE_TREE, dialogEfgDelete::OnDeleteTree)
END_EVENT_TABLE()

dialogEfgDelete::dialogEfgDelete(wxWindow *p_parent, Node *p_node)
  : wxDialog(p_parent, -1, "Delete..."), m_node(p_node)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *choicesSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString deleteChoices[] = { "Entire subtree", "Selected move" };
  m_deleteTree = new wxRadioBox(this, idRADIOBOX_DELETE_TREE, "Delete",
				wxDefaultPosition, wxDefaultSize,
				2, deleteChoices, 1, wxRA_SPECIFY_COLS);
  m_deleteTree->SetSelection(0);
  choicesSizer->Add(m_deleteTree, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *actionSizer = new wxBoxSizer(wxVERTICAL);
  actionSizer->Add(new wxStaticText(this, -1, "Keep subtree after action"),
		   0, wxCENTER | wxALL, 5);
  m_branchList = new wxListBox(this, -1);
  for (int act = 1; act <= m_node->Game()->NumChildren(m_node); act++) {
    m_branchList->Append((char *) (ToText(act) + ": " + 
				   m_node->GetInfoset()->Actions()[act]->GetName()));
  }
  m_branchList->SetSelection(0);
  m_branchList->Enable(false);
  actionSizer->Add(m_branchList, 0, wxCENTER | wxALL, 5);
  choicesSizer->Add(actionSizer, 0, wxALL, 5);
  topSizer->Add(choicesSizer, 0, wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

void dialogEfgDelete::OnDeleteTree(wxCommandEvent &)
{
  m_branchList->Enable(m_deleteTree->GetSelection() == 1);
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

