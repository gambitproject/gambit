//
// FILE: mixededit.cc -- Dialog for editing mixed profiles
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "game/mixedsol.h"
#include "mixededit.h"

//-------------------------------------------------------------------------
//                class dialogMixedEditor: Member functions
//-------------------------------------------------------------------------

const int idPLAYER_LIST = 2001;
const int idPROB_GRID = 2002;

BEGIN_EVENT_TABLE(dialogMixedEditor, wxDialog)
  EVT_LISTBOX(idPLAYER_LIST, dialogMixedEditor::OnSelChanged)
  EVT_BUTTON(wxID_OK, dialogMixedEditor::OnOK)
END_EVENT_TABLE()

dialogMixedEditor::dialogMixedEditor(wxWindow *p_parent,
				     const MixedSolution &p_profile)
  : wxDialog(p_parent, -1, "Edit mixed profile"),
    m_profile(p_profile), m_selection(1)
{
  m_playerList = new wxListBox(this, idPLAYER_LIST);
  for (int pl = 1; pl <= m_profile.Game().NumPlayers(); pl++) {
    m_playerList->Append((char *) (ToText(pl) + ": " +
				   m_profile.Game().Players()[pl]->GetName()));
  }
  m_playerList->SetSelection(0);

  NFPlayer *firstPlayer = m_profile.Game().Players()[1];
  m_probGrid = new wxGrid(this, idPROB_GRID,
			  wxDefaultPosition, wxSize(200, 200));
  m_probGrid->CreateGrid(firstPlayer->NumStrats(), 1);
  m_probGrid->SetLabelValue(wxHORIZONTAL, "Probability", 0);
  for (int st = 1; st <= firstPlayer->NumStrats(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) firstPlayer->Strategies()[st]->Name(),
			      st - 1);
    m_probGrid->SetCellValue((char *) ToText(p_profile(firstPlayer->Strategies()[st])),
			     st - 1, 0);
  }
  m_probGrid->UpdateDimensions();
  m_probGrid->Refresh();

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);
  editSizer->Add(m_playerList, 0, wxALL, 5);
  editSizer->Add(m_probGrid, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

dialogMixedEditor::~dialogMixedEditor()
{ }

void dialogMixedEditor::OnSelChanged(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  NFPlayer *oldPlayer = m_profile.Game().Players()[m_selection];

  for (int st = 1; st <= oldPlayer->NumStrats(); st++) {
    m_profile.Set(oldPlayer->Strategies()[st],
		  ToNumber(m_probGrid->GetCellValue(st - 1, 0).c_str()));
  }

  NFPlayer *player = m_profile.Game().Players()[p_event.GetSelection() + 1];

  if (oldPlayer->NumStrats() > player->NumStrats()) {
    m_probGrid->DeleteRows(0,
			   oldPlayer->NumStrats() - player->NumStrats());
  }
  else if (oldPlayer->NumStrats() < player->NumStrats()) {
    m_probGrid->InsertRows(0,
			   player->NumStrats() - oldPlayer->NumStrats());
  }

  for (int st = 1; st <= player->NumStrats(); st++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) player->Strategies()[st]->Name(),
			      st - 1);
    m_probGrid->SetCellValue((char *) ToText(m_profile(player->Strategies()[st])),
			     st - 1, 0);
  }

  m_selection = p_event.GetSelection() + 1;
}

void dialogMixedEditor::OnOK(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  NFPlayer *player = m_profile.Game().Players()[m_selection];

  for (int st = 1; st <= player->NumStrats(); st++) {
    m_profile.Set(player->Strategies()[st],
		  ToNumber(m_probGrid->GetCellValue(st - 1, 0).c_str()));
  }

  p_event.Skip();
}
