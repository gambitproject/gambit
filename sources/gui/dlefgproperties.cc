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
  wxTextCtrl *m_title, *m_comment;

public:
  panelEfgGeneral(wxWindow *p_parent, FullEfg &p_efg, const wxString &);
};

panelEfgGeneral::panelEfgGeneral(wxWindow *p_parent, FullEfg &p_efg,
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
  //  topSizer->Fit(this);
  //  topSizer->SetSizeHints(this);

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
  // topSizer->Fit(this);
  // topSizer->SetSizeHints(this);

  Layout();
  m_playerGrid->AdjustScrollbars();
}

dialogEfgProperties::dialogEfgProperties(wxWindow *p_parent, FullEfg &p_efg,
					 const wxString &p_filename)
  : wxDialog(p_parent, -1, "Extensive form properties"), m_efg(p_efg)
{
  SetAutoLayout(true);

  wxNotebook *notebook = new wxNotebook(this, -1, wxDefaultPosition,
					wxSize(300, 300));
  (void) new wxNotebookSizer(notebook);
  m_generalPanel = new panelEfgGeneral(notebook, m_efg, p_filename);
  notebook->AddPage(m_generalPanel, "General");
  m_playersPanel = new panelEfgPlayers(notebook, m_efg);
  notebook->AddPage(m_playersPanel, "Players");

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
