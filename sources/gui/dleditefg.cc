//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of an extensive form game
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "game/efg.h"
#include "dleditefg.h"

//========================================================================
//                        class panelEfgGeneral
//========================================================================

class panelEfgGeneral : public wxPanel {
private:
  efgGame &m_efg;
  wxTextCtrl *m_title, *m_comment;

public:
  panelEfgGeneral(wxWindow *p_parent, efgGame &p_efg, const wxString &);

  wxString GetGameTitle(void) const { return m_title->GetValue(); }
  wxString GetComment(void) const { return m_comment->GetValue(); }
};

panelEfgGeneral::panelEfgGeneral(wxWindow *p_parent, efgGame &p_efg,
				 const wxString &p_filename)
  : wxPanel(p_parent, -1), m_efg(p_efg)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(this, wxID_STATIC, "Title"),
		  0, wxALL | wxCENTER, 5);
  m_title = new wxTextCtrl(this, -1, (const char *) m_efg.GetTitle());
  titleSizer->Add(m_title, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  wxBoxSizer *commentSizer = new wxBoxSizer(wxHORIZONTAL);
  commentSizer->Add(new wxStaticText(this, wxID_STATIC, "Comment"),
		    0, wxALL | wxCENTER, 5);
  m_comment = new wxTextCtrl(this, -1, (const char *) m_efg.GetComment(),
			     wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  commentSizer->Add(m_comment, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 0);
			     
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString("Filename: ") + p_filename),
		0, wxALL, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Number of players: %d",
						  m_efg.NumPlayers())),
		0, wxALL, 5);
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Constant-sum game: %s",
						  (m_efg.IsConstSum()) ?
						  "YES" : "NO")),
		0, wxALL, 5);
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Perfect recall: %s",
						  (IsPerfectRecall(m_efg)) ?
						  "YES" : "NO")),
		0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

//========================================================================
//                        class panelEfgPlayers
//========================================================================

const int idLIST_PLAYER = 1000;
const int idBUTTON_NEWPLAYER = 1001;

class panelEfgPlayers : public wxPanel {
private:
  efgGame &m_efg;
  int m_lastSelection;
  wxListBox *m_playerList;
  wxTextCtrl *m_playerName;

  void OnPlayerSelect(wxCommandEvent &);
  void OnNewPlayer(wxCommandEvent &);

public:
  panelEfgPlayers(wxWindow *p_parent, efgGame &p_efg);

  virtual bool Validate(void);

  int NumPlayers(void) const { return m_playerList->GetCount(); }
  wxString GetPlayerName(int pl) const
  { return m_playerList->GetString(pl - 1); }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgPlayers, wxPanel)
  EVT_LISTBOX(idLIST_PLAYER, panelEfgPlayers::OnPlayerSelect)
  EVT_BUTTON(idBUTTON_NEWPLAYER, panelEfgPlayers::OnNewPlayer)
END_EVENT_TABLE()

panelEfgPlayers::panelEfgPlayers(wxWindow *p_parent, efgGame &p_efg)
  : wxPanel(p_parent, -1), m_efg(p_efg), m_lastSelection(0)
{
  SetAutoLayout(true);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxVERTICAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, "Players"),
		   0, wxLEFT | wxTOP | wxRIGHT, 5);
  m_playerList = new wxListBox(this, idLIST_PLAYER);
  for (int pl = 1; pl <= p_efg.NumPlayers(); pl++) {
    m_playerList->Append((const char *) p_efg.Players()[pl]->GetName());
  }
  playerSizer->Add(m_playerList, 0, wxALL, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxVERTICAL);
  editSizer->Add(new wxStaticText(this, wxID_STATIC, "Player name"),
		 0, wxTOP | wxCENTER, 5);
  m_playerName = new wxTextCtrl(this, -1, "");
  if (p_efg.NumPlayers() > 0) {
    // should always be the case; can't be too careful though!
    m_playerList->SetSelection(0);
    m_playerName->SetValue((const char *) p_efg.Players()[1]->GetName());
  }
  editSizer->Add(m_playerName, 0, wxALL | wxCENTER, 5);

  editSizer->Add(new wxButton(this, idBUTTON_NEWPLAYER, "Add Player"),
		 0, wxALL | wxCENTER, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(playerSizer, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void panelEfgPlayers::OnPlayerSelect(wxCommandEvent &p_event)
{
  m_playerList->SetString(m_lastSelection, m_playerName->GetValue());
  m_lastSelection = p_event.GetSelection();
  m_playerName->SetValue(m_playerList->GetString(m_lastSelection));
}

void panelEfgPlayers::OnNewPlayer(wxCommandEvent &)
{
  m_playerList->SetString(m_lastSelection, m_playerName->GetValue());
  m_playerList->Append(wxString::Format("Player%d", 
					m_playerList->GetCount() + 1));
  m_lastSelection = m_playerList->GetCount() - 1;
  m_playerList->SetSelection(m_lastSelection);
  m_playerName->SetValue(m_playerList->GetStringSelection());
}

//
// Validate() is overriden to transfer any player name edits to
// the name listbox, to be sure they are captured
//
bool panelEfgPlayers::Validate(void)
{
  m_playerList->SetString(m_lastSelection, m_playerName->GetValue());
  return wxWindow::Validate();
}

//========================================================================
//                        class dialogEditEfg
//========================================================================

BEGIN_EVENT_TABLE(dialogEditEfg, wxDialog)
  EVT_BUTTON(wxID_OK, dialogEditEfg::OnOK)
END_EVENT_TABLE()

dialogEditEfg::dialogEditEfg(wxWindow *p_parent, efgGame &p_efg,
			     const wxString &p_filename)
  : wxDialog(p_parent, -1, "Extensive form properties"), m_efg(p_efg)
{
  SetAutoLayout(true);

  m_notebook = new wxNotebook(this, -1);
  wxNotebookSizer *notebookSizer = new wxNotebookSizer(m_notebook);
  m_notebook->AddPage(new panelEfgGeneral(m_notebook, m_efg, p_filename),
		      "General");
  m_notebook->AddPage(new panelEfgPlayers(m_notebook, m_efg), "Players");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebookSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void dialogEditEfg::OnOK(wxCommandEvent &)
{
  if (!m_notebook->GetPage(0)->Validate() ||
      !m_notebook->GetPage(1)->Validate()) {
    return;
  }

  EndModal(wxID_OK);
}

wxString dialogEditEfg::GetGameTitle(void) const
{
  return ((panelEfgGeneral *) m_notebook->GetPage(0))->GetGameTitle();
}

wxString dialogEditEfg::GetComment(void) const
{
  return ((panelEfgGeneral *) m_notebook->GetPage(0))->GetComment();
}

int dialogEditEfg::NumPlayers(void) const
{
  return ((panelEfgPlayers *) m_notebook->GetPage(1))->NumPlayers();
}

wxString dialogEditEfg::GetPlayerName(int pl) const
{
  return ((panelEfgPlayers *) m_notebook->GetPage(1))->GetPlayerName(pl);
}
