//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a normal form game
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "game/nfg.h"
#include "dlnfgproperties.h"

//========================================================================
//                        class panelNfgGeneral
//========================================================================

extern bool IsConstSum(const Nfg &);

class panelNfgGeneral : public wxPanel {
private:
  Nfg &m_nfg;
  wxTextCtrl *m_title, *m_comment;

public:
  panelNfgGeneral(wxWindow *p_parent, Nfg &p_nfg, const wxString &);

  wxString GetGameTitle(void) const { return m_title->GetValue(); }
  wxString GetComment(void) const { return m_comment->GetValue(); }
};

panelNfgGeneral::panelNfgGeneral(wxWindow *p_parent, Nfg &p_nfg,
				 const wxString &p_filename)
  : wxPanel(p_parent, -1), m_nfg(p_nfg)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(this, wxID_STATIC, "Title"),
		  0, wxALL | wxCENTER, 5);
  m_title = new wxTextCtrl(this, -1, (const char *) m_nfg.GetTitle());
  titleSizer->Add(m_title, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  wxBoxSizer *commentSizer = new wxBoxSizer(wxHORIZONTAL);
  commentSizer->Add(new wxStaticText(this, wxID_STATIC, "Comment"),
		    0, wxALL | wxCENTER, 5);
  m_comment = new wxTextCtrl(this, -1, (const char *) m_nfg.GetComment(),
			     wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  commentSizer->Add(m_comment, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 0);
			     
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString("Filename: ") + p_filename),
		0, wxALL, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Number of players: %d",
						  m_nfg.NumPlayers())),
		0, wxALL, 5);
  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxString::Format("Constant-sum game: %s",
						  (IsConstSum(m_nfg)) ?
						  "YES" : "NO")),
		0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

//========================================================================
//                        class panelNfgPlayers
//========================================================================

const int idLIST_PLAYER = 1000;

class panelNfgPlayers : public wxPanel {
private:
  Nfg &m_nfg;
  int m_lastSelection;
  wxListBox *m_playerList;
  wxTextCtrl *m_playerName;

  void OnPlayerSelect(wxCommandEvent &);

public:
  panelNfgPlayers(wxWindow *p_parent, Nfg &p_nfg);

  virtual bool Validate(void);

  int NumPlayers(void) const { return m_playerList->GetCount(); }
  wxString GetPlayerName(int pl) const
  { return m_playerList->GetString(pl - 1); }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgPlayers, wxPanel)
  EVT_LISTBOX(idLIST_PLAYER, panelNfgPlayers::OnPlayerSelect)
END_EVENT_TABLE()

panelNfgPlayers::panelNfgPlayers(wxWindow *p_parent, Nfg &p_nfg)
  : wxPanel(p_parent, -1), m_nfg(p_nfg), m_lastSelection(0)
{
  SetAutoLayout(true);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxVERTICAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, "Players"),
		   0, wxLEFT | wxTOP | wxRIGHT, 5);
  m_playerList = new wxListBox(this, idLIST_PLAYER);
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    m_playerList->Append((const char *) p_nfg.Players()[pl]->GetName());
  }
  playerSizer->Add(m_playerList, 0, wxALL, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxVERTICAL);
  editSizer->Add(new wxStaticText(this, wxID_STATIC, "Player name"),
		 0, wxTOP | wxCENTER, 5);
  m_playerName = new wxTextCtrl(this, -1, "");
  if (p_nfg.NumPlayers() > 0) {
    // should always be the case; can't be too careful though!
    m_playerList->SetSelection(0);
    m_playerName->SetValue((const char *) p_nfg.Players()[1]->GetName());
  }
  editSizer->Add(m_playerName, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(playerSizer, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void panelNfgPlayers::OnPlayerSelect(wxCommandEvent &p_event)
{
  m_playerList->SetString(m_lastSelection, m_playerName->GetValue());
  m_lastSelection = p_event.GetSelection();
  m_playerName->SetValue(m_playerList->GetString(m_lastSelection));
}

//
// Validate() is overriden to transfer any player name edits to
// the name listbox, to be sure they are captured
//
bool panelNfgPlayers::Validate(void)
{
  m_playerList->SetString(m_lastSelection, m_playerName->GetValue());
  return wxWindow::Validate();
}

//========================================================================
//                      class dialogNfgProperties
//========================================================================

BEGIN_EVENT_TABLE(dialogNfgProperties, wxDialog)
  EVT_BUTTON(wxID_OK, dialogNfgProperties::OnOK)
END_EVENT_TABLE()

dialogNfgProperties::dialogNfgProperties(wxWindow *p_parent, Nfg &p_nfg,
					 const wxString &p_filename)
  : wxDialog(p_parent, -1, "Normal form properties"), m_nfg(p_nfg)
{
  SetAutoLayout(true);

  m_notebook = new wxNotebook(this, -1);
  wxNotebookSizer *notebookSizer = new wxNotebookSizer(m_notebook);
  m_notebook->AddPage(new panelNfgGeneral(m_notebook, m_nfg, p_filename),
		      "General");
  m_notebook->AddPage(new panelNfgPlayers(m_notebook, m_nfg), "Players");

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

void dialogNfgProperties::OnOK(wxCommandEvent &)
{
  if (!m_notebook->GetPage(0)->Validate() ||
      !m_notebook->GetPage(1)->Validate()) {
    return;
  }

  EndModal(wxID_OK);
}

wxString dialogNfgProperties::GetGameTitle(void) const
{
  return ((panelNfgGeneral *) m_notebook->GetPage(0))->GetGameTitle();
}

wxString dialogNfgProperties::GetComment(void) const
{
  return ((panelNfgGeneral *) m_notebook->GetPage(0))->GetComment();
}

int dialogNfgProperties::NumPlayers(void) const
{
  return ((panelNfgPlayers *) m_notebook->GetPage(1))->NumPlayers();
}

wxString dialogNfgProperties::GetPlayerName(int pl) const
{
  return ((panelNfgPlayers *) m_notebook->GetPage(1))->GetPlayerName(pl);
}
