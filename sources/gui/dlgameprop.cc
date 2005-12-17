//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a game
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "gamedoc.h"
#include "dlgameprop.h"

//========================================================================
//                   class gbtGamePropertiesDialog
//========================================================================

gbtGamePropertiesDialog::gbtGamePropertiesDialog(wxWindow *p_parent,
						 gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, _("Game properties"), wxDefaultPosition), 
    m_doc(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Title")),
		  0, wxALL | wxALIGN_CENTER, 5);
  if (m_doc->GetEfg()) {
    m_title = new wxTextCtrl(this, -1, 
			     wxString(m_doc->GetEfg()->GetTitle().c_str(),
				      *wxConvCurrent),
			     wxDefaultPosition, wxSize(400, -1));
  }
  else {
    m_title = new wxTextCtrl(this, -1, 
			     wxString(m_doc->GetNfg()->GetTitle().c_str(),
				      *wxConvCurrent),
			     wxDefaultPosition, wxSize(400, -1));
  }
  titleSizer->Add(m_title, 1, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  wxBoxSizer *commentSizer = new wxBoxSizer(wxHORIZONTAL);
  commentSizer->Add(new wxStaticText(this, wxID_STATIC, _("Comment")),
		    0, wxALL | wxALIGN_CENTER, 5);
  if (m_doc->GetEfg()) {
    m_comment = new wxTextCtrl(this, -1, 
			       wxString(m_doc->GetEfg()->GetComment().c_str(),
					*wxConvCurrent),
			       wxDefaultPosition, wxSize(400, -1),
			       wxTE_MULTILINE);
  }
  else {
    m_comment = new wxTextCtrl(this, -1, 
			       wxString(m_doc->GetNfg()->GetComment().c_str(),
					*wxConvCurrent),
			       wxDefaultPosition, wxSize(400, -1),
			       wxTE_MULTILINE);
  }

  commentSizer->Add(m_comment, 1, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 0);
	
  wxStaticBoxSizer *boxSizer = 
    new wxStaticBoxSizer(wxVERTICAL, this, _("Information about this game"));

  boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString(_("Filename: ")) + 
				 m_doc->GetFilename()),
		0, wxALL, 5);

  if (m_doc->GetEfg()) {
    gbtEfgGame efg = m_doc->GetEfg();
    boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				   wxString::Format(_("Number of players: %d"),
						    efg->NumPlayers())),
		  0, wxALL, 5);
    if (efg->IsConstSum()) {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC, 
				     _("This is a constant-sum game")),
		    0, wxALL, 5);
    }
    else {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC, 
				     _("This is not a constant-sum game")),
		    0, wxALL, 5);
    }
    if (efg->IsPerfectRecall()) {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				     _("This is a game of perfect recall")),
		    0, wxALL, 5);
    }
    else {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				     _("This is not a game of perfect recall")),
		    0, wxALL, 5);
    }
  }
  else {
    gbtNfgGame nfg = m_doc->GetNfg();

    boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				   wxString::Format(_("Number of players: %d"),
						    nfg->NumPlayers())),
		  0, wxALL, 5);
    if (nfg->IsConstSum()) {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				     wxString(_("This is a constant-sum game"))),
		    0, wxALL, 5);
      
    }
    else {
      boxSizer->Add(new wxStaticText(this, wxID_STATIC,
				     wxString(_("This is not a constant-sum game"))),
		    0, wxALL, 5);
    }
  }

  topSizer->Add(boxSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

