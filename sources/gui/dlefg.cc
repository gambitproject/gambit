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

