//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for deleting moves/subtrees from extensive form
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
#include "dlefgdelete.h"

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
  for (int act = 1; act <= m_node->NumChildren(); act++) {
    m_branchList->Append((char *) (ToText(act) + ": " + 
				   m_node->GetInfoset().GetAction(act).GetLabel()));
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
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
  CenterOnParent();
}

void dialogEfgDelete::OnDeleteTree(wxCommandEvent &)
{
  m_branchList->Enable(m_deleteTree->GetSelection() == 1);
}


