//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a game
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
#include "wx/notebook.h"
#include "wx/grid.h"

#include "dleditgame.h"

const int GBT_BUTTON_ADD_PLAYER = 1001;

//========================================================================
//                        class gbtDialogEditGame
//========================================================================

BEGIN_EVENT_TABLE(gbtDialogEditGame, wxDialog)
  EVT_BUTTON(wxID_OK, gbtDialogEditGame::OnOK)
  EVT_BUTTON(GBT_BUTTON_ADD_PLAYER, gbtDialogEditGame::OnAddPlayer)
END_EVENT_TABLE()

gbtDialogEditGame::gbtDialogEditGame(wxWindow *p_parent,
				     gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, _("Game properties"), wxDefaultPosition), 
    m_doc(p_doc)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Title")),
		  0, wxALL | wxCENTER, 5);
  if (m_doc->HasEfg()) {
    m_title = new wxTextCtrl(this, -1, 
			     wxString::Format(wxT("%s"),
					      (const char *) m_doc->GetEfg()->GetLabel()));
  }
  else {
    m_title = new wxTextCtrl(this, -1, 
			     wxString::Format(wxT("%s"),
					      (const char *) m_doc->GetNfg()->GetLabel()));
  }

  titleSizer->Add(m_title, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  wxBoxSizer *centralSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *propBox = new wxStaticBox(this, wxID_STATIC, _("Properties"));
  wxStaticBoxSizer *propSizer = new wxStaticBoxSizer(propBox, wxVERTICAL);
  propSizer->Add(new wxStaticText(this, wxID_STATIC,
				  wxString::Format(_("Representation: %s"),
						   (m_doc->HasEfg()) ?
						   _("Extensive form") :
						   _("Normal form"))),
		 0, wxALL, 5);
  propSizer->Add(new wxStaticText(this, wxID_STATIC,
				  wxString(_("Filename: ")) +
				  m_doc->GetFilename()),
		 0, wxALL, 5);
  if (m_doc->HasEfg()) {
    propSizer->Add(new wxStaticText(this, wxID_STATIC,
				    wxString::Format(_("Number of players: %d"),
						     m_doc->GetEfg()->NumPlayers())),
		   0, wxALL, 5);
    propSizer->Add(new wxStaticText(this, wxID_STATIC,
				    wxString::Format(_("Constant-sum game: %s"),
						     (m_doc->GetEfg()->IsConstSum()) ?
						     _("YES") : _("NO"))),
		   0, wxALL, 5);
    propSizer->Add(new wxStaticText(this, wxID_STATIC,
				    wxString::Format(_("Perfect recall: %s"),
						     (m_doc->GetEfg()->IsPerfectRecall()) ?
						     _("YES") : _("NO"))),
		   0, wxALL, 5);
  }
  else {
    propSizer->Add(new wxStaticText(this, wxID_STATIC,
				    wxString::Format(_("Number of players: %d"),
						     m_doc->GetNfg()->NumPlayers())),
		   0, wxALL, 5);
    propSizer->Add(new wxStaticText(this, wxID_STATIC,
				    wxString::Format(_("Constant-sum game: %s"),
						     (m_doc->GetNfg()->IsConstSum()) ?
						     _("YES") : _("NO"))),
		   0, wxALL, 5);
  }
  centralSizer->Add(propSizer, 1, wxALL | wxEXPAND, 5);

  wxStaticBox *playerGridBox = new wxStaticBox(this, wxID_STATIC, _("Players"));
  wxStaticBoxSizer *playerGridSizer = new wxStaticBoxSizer(playerGridBox,
							   wxVERTICAL);
  m_players = new wxGrid(this, -1, wxDefaultPosition, wxSize(200, 150));
  if (m_doc->HasEfg()) {
    m_players->CreateGrid(m_doc->GetEfg()->NumPlayers(), 1);
  }
  else {
    m_players->CreateGrid(m_doc->GetNfg()->NumPlayers(), 1);
  }
  m_players->DisableDragRowSize();
  m_players->DisableDragColSize();
  m_players->EnableGridLines(false);
  m_players->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_players->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_players->SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  m_players->SetLabelFont(m_doc->GetPreferences().GetLabelFont());
  m_players->SetLabelValue(wxHORIZONTAL, _("Name"), 0);

  if (m_doc->HasEfg()) {
    for (int pl = 1; pl <= m_doc->GetEfg()->NumPlayers(); pl++) {
      m_players->SetCellValue(pl - 1, 0, 
			      wxString::Format(wxT("%s"),
					       (char *) m_doc->GetEfg()->GetPlayer(pl)->GetLabel()));
      if ((pl - 1) % 2 == 0) {
	m_players->SetCellBackgroundColour(pl - 1, 0, wxColour(200, 200, 200));
      }
      else {
	m_players->SetCellBackgroundColour(pl - 1, 0, wxColour(225, 225, 225));
      }
    }
  }
  else {
    for (int pl = 1; pl <= m_doc->GetNfg()->NumPlayers(); pl++) {
      m_players->SetCellValue(pl - 1, 0, 
			      wxString::Format(wxT("%s"),
					       (char *) m_doc->GetNfg()->GetPlayer(pl)->GetLabel()));
      if ((pl - 1) % 2 == 0) {
	m_players->SetCellBackgroundColour(pl - 1, 0, wxColour(200, 200, 200));
      }
      else {
	m_players->SetCellBackgroundColour(pl - 1, 0, wxColour(225, 225, 225));
      }
    }
  }

  playerGridSizer->Add(m_players, 1, wxALL | wxCENTER | wxEXPAND, 5);

  if (m_doc->HasEfg()) {
    playerGridSizer->Add(new wxButton(this, GBT_BUTTON_ADD_PLAYER,
				      _("Add Player")),
			 0, wxALL | wxCENTER, 5);
  }

  centralSizer->Add(playerGridSizer, 0, wxALL, 5);
  topSizer->Add(centralSizer, 0, wxALL, 0);
  
  wxStaticBox *commentBox = new wxStaticBox(this, wxID_STATIC, _("Comments"));
  wxStaticBoxSizer *commentSizer = new wxStaticBoxSizer(commentBox,
							wxHORIZONTAL);
  if (m_doc->HasEfg()) {
    m_comment = new wxTextCtrl(this, -1, 
			       wxString::Format(wxT("%s"),
						(const char *) m_doc->GetEfg()->GetComment()),
			       wxDefaultPosition, wxSize(100, 100),
			       wxTE_MULTILINE);
  }
  else {
    m_comment = new wxTextCtrl(this, -1, 
			       wxString::Format(wxT("%s"),
						(const char *) m_doc->GetNfg()->GetComment()),
			       wxDefaultPosition, wxSize(100, 100),
			       wxTE_MULTILINE);
  }
  commentSizer->Add(m_comment, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

void gbtDialogEditGame::OnAddPlayer(wxCommandEvent &)
{
  m_players->AppendRows();
  int row = m_players->GetNumberRows() - 1;
  if (row % 2 == 0) {
    m_players->SetCellBackgroundColour(row, 0, wxColour(200, 200, 200));
  }
  else {
    m_players->SetCellBackgroundColour(row, 0, wxColour(225, 225, 225));
  }
  m_players->AdjustScrollbars();
}

void gbtDialogEditGame::OnOK(wxCommandEvent &p_event)
{
  // Make sure to save any active edit in the players grid
  if (m_players->IsCellEditControlEnabled()) {
    m_players->SaveEditControlValue();
    m_players->HideCellEditControl();
  }

  // Continue usual processing
  p_event.Skip();
}


//========================================================================
//                     Command class for editing game
//========================================================================


class gbtCmdEditGame : public gbtGameCommand {
private:
  gbtText m_title, m_comment;
  gbtArray<gbtText> m_players;

public:
  gbtCmdEditGame(const gbtText &p_title, const gbtText &p_comment,
		 const gbtArray<gbtText> &p_players)
    : m_title(p_title), m_comment(p_comment), m_players(p_players) { }
  virtual ~gbtCmdEditGame() { }

  void Do(gbtGameDocument *); 

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdEditGame::Do(gbtGameDocument *p_doc)
{
  if (p_doc->HasEfg()) {
    gbtEfgGame efg = p_doc->GetEfg();
    efg->SetLabel(m_title);
    efg->SetComment(m_comment);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      if (pl > efg->NumPlayers()) {
	efg->NewPlayer();
      }
      efg->GetPlayer(pl)->SetLabel(m_players[pl]);
    }
  }
  else {
    gbtNfgGame nfg = p_doc->GetNfg();
    nfg->SetLabel(m_title);
    nfg->SetComment(m_comment);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      nfg->GetPlayer(pl)->SetLabel(m_players[pl]);
    }
  }
}

gbtGameCommand *gbtDialogEditGame::GetCommand(void) const
{
  gbtArray<gbtText> players(m_players->GetNumberRows());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = m_players->GetCellValue(pl - 1, 0).mb_str();
  }

  return new gbtCmdEditGame(gbtText(m_title->GetValue().mb_str()),
			    gbtText(m_comment->GetValue().mb_str()),
			    players);
}
