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
#include "wx/grid.h"
#include "wx/notebook.h"
#include "efg.h"
#include "dlefgproperties.h"

class panelEfgGeneral : public wxPanel {
private:
  FullEfg &m_efg;
  wxTextCtrl *m_title;

public:
  panelEfgGeneral(wxWindow *p_parent, FullEfg &p_efg);
};

panelEfgGeneral::panelEfgGeneral(wxWindow *p_parent, FullEfg &p_efg)
  : wxPanel(p_parent, -1), m_efg(p_efg)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(this, wxID_STATIC, "Title"),
		  0, wxALL | wxCENTER, 5);
  m_title = new wxTextCtrl(this, -1, (const char *) m_efg.GetTitle());
  titleSizer->Add(m_title, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(titleSizer, 0, wxALL, 0);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 "Filename:"),
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

class panelEfgPlayers : public wxPanel {
private:
  FullEfg &m_efg;
  wxGrid *m_playerGrid;

public:
  panelEfgPlayers(wxWindow *p_parent, FullEfg &p_efg);
};

panelEfgPlayers::panelEfgPlayers(wxWindow *p_parent, FullEfg &p_efg)
  : wxPanel(p_parent, -1), m_efg(p_efg)
{
  SetAutoLayout(true);

  m_playerGrid = new wxGrid(this, -1, wxDefaultPosition, wxSize(200, 200));
  m_playerGrid->CreateGrid(m_efg.NumPlayers(), 1);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_playerGrid, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  m_playerGrid->AdjustScrollbars();
}

dialogEfgProperties::dialogEfgProperties(wxWindow *p_parent, FullEfg &p_efg)
  : wxDialog(p_parent, -1, "Extensive form properties"), m_efg(p_efg)
{
  SetAutoLayout(true);

  wxNotebook *notebook = new wxNotebook(this, -1);
  m_generalPanel = new panelEfgGeneral(notebook, m_efg);
  notebook->AddPage(m_generalPanel, "General");
  m_playersPanel = new panelEfgPlayers(notebook, m_efg);
  notebook->AddPage(m_playersPanel, "Players");
  wxNotebookSizer *notebookSizer = new wxNotebookSizer(notebook);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebook, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}
