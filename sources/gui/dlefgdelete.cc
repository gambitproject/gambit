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

dialogEfgDelete::dialogEfgDelete(wxWindow *p_parent, gbtEfgNode p_node)
  : wxDialog(p_parent, -1, _("Delete..."), wxDefaultPosition)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  if (!p_node->GetParent().IsNull()) {
    wxString deleteChoices[] = { _("Entire subtree"), _("Parent move") };
    m_deleteTree = new wxRadioBox(this, -1, _("Delete"),
				  wxDefaultPosition, wxDefaultSize,
				  2, deleteChoices, 1, 
				  wxRA_SPECIFY_COLS);
    m_deleteTree->SetSelection(0);
    topSizer->Add(m_deleteTree, 0, wxALL | wxCENTER, 5);
  }
  else {
    wxString deleteChoices[] = { _("Entire subtree") };
    m_deleteTree = new wxRadioBox(this, -1, _("Delete"),
				  wxDefaultPosition, wxDefaultSize,
				  1, deleteChoices, 1,
				  wxRA_SPECIFY_COLS);
    m_deleteTree->SetSelection(0);
    topSizer->Add(m_deleteTree, 0, wxALL | wxCENTER, 5);
  }

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, _("Help")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
  CenterOnParent();
}


