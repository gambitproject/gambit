//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for editing properties of behavior profiles
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
#include "nash/behavsol.h"
#include "dleditbehav.h"

//-------------------------------------------------------------------------
//                 class dialogEditBehav: Member functions
//-------------------------------------------------------------------------

const int idINFOSET_TREE = 2001;
const int idPROB_GRID = 2002;

class ProbGrid : public wxGrid {
private:
  // Event handlers
  void OnKeyDown(wxKeyEvent &);

public:
  ProbGrid(wxWindow *p_parent)
    : wxGrid(p_parent, idPROB_GRID, wxPoint(5, 5), wxSize(200, 200)) { }
  virtual ~ProbGrid() { }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ProbGrid, wxGrid)
  EVT_KEY_DOWN(ProbGrid::OnKeyDown)
END_EVENT_TABLE()

void ProbGrid::OnKeyDown(wxKeyEvent &p_event)
{
  // This is here only to keep the program from crashing under MSW
  if (!IsCellEditControlEnabled()) {
    EnableCellEditControl();
    ShowCellEditControl();
  }
}


BEGIN_EVENT_TABLE(dialogEditBehav, wxDialog)
  EVT_TREE_ITEM_COLLAPSING(idINFOSET_TREE, dialogEditBehav::OnItemCollapsing)
  EVT_TREE_SEL_CHANGING(idINFOSET_TREE, dialogEditBehav::OnSelChanging)
  EVT_TREE_SEL_CHANGED(idINFOSET_TREE, dialogEditBehav::OnSelChanged)
  EVT_BUTTON(wxID_OK, dialogEditBehav::OnOK)
END_EVENT_TABLE()

dialogEditBehav::dialogEditBehav(wxWindow *p_parent,
				 const BehavSolution &p_profile)
  : wxDialog(p_parent, -1, "Behavior profile properties"),
    m_profile(p_profile), m_lastInfoset(0), m_map((Infoset *) 0)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *nameSizer = new wxBoxSizer(wxHORIZONTAL);
  nameSizer->Add(new wxStaticText(this, wxID_STATIC, "Profile name"),
		 0, wxALL, 5);
  m_profileName = new wxTextCtrl(this, -1, (char *) p_profile.GetName());
  nameSizer->Add(m_profileName, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);
  m_infosetTree = new wxTreeCtrl(this, idINFOSET_TREE,
				 wxPoint(5, 5), wxSize(200, 200));
  m_infosetTree->AddRoot((char *) p_profile.GetName());

  m_lastInfoset = p_profile.GetGame().Infosets()[1];
  wxTreeItemId firstID;

  for (int pl = 1; pl <= p_profile.GetGame().NumPlayers(); pl++) {
    EFPlayer *player = p_profile.GetGame().Players()[pl];
    wxTreeItemId id;
    if (player->GetName() != "") {
      id = m_infosetTree->AppendItem(m_infosetTree->GetRootItem(),
				     (char *) player->GetName());
    }
    else {
      id = m_infosetTree->AppendItem(m_infosetTree->GetRootItem(),
				     wxString::Format("Player %d", pl));
    }
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      wxTreeItemId isetID;
      if (infoset->GetName() != "") {
	isetID = m_infosetTree->AppendItem(id,
					   (char *) infoset->GetName());
      }
      else {
	isetID = m_infosetTree->AppendItem(id, wxString::Format("(%d,%d)",
								pl, iset));
      }
      m_map.Define(isetID, player->Infosets()[iset]);
      if (player->Infosets()[iset] == m_lastInfoset) {
	firstID = isetID;
      }
    }
    m_infosetTree->Expand(id);
  }
  m_infosetTree->Expand(m_infosetTree->GetRootItem());
  editSizer->Add(m_infosetTree, 0, wxALL, 5);

  m_probGrid = new ProbGrid(this);
  m_probGrid->CreateGrid(m_lastInfoset->NumActions(), 1);
  m_probGrid->SetLabelValue(wxHORIZONTAL, "Probability", 0);
  m_probGrid->SetDefaultCellAlignment(wxRIGHT, wxCENTER);
  for (int act = 1; act <= m_lastInfoset->NumActions(); act++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) m_lastInfoset->Actions()[act]->GetName(),
			      act - 1);
    m_probGrid->SetCellValue((char *) ToText(p_profile(m_lastInfoset->Actions()[act])),
			     act - 1, 0);
  }
  m_probGrid->UpdateDimensions();
  m_probGrid->Refresh();
  editSizer->Add(m_probGrid, 0, wxALL, 5);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  // We wait and do this until the end, since this emits a
  // selection event, and the grid must be set up properly before
  // that happens
  m_infosetTree->SelectItem(firstID);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

dialogEditBehav::~dialogEditBehav()
{ }

void dialogEditBehav::OnItemCollapsing(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_infosetTree->GetRootItem()) {
    p_event.Veto();
  }
}

void dialogEditBehav::OnSelChanging(wxTreeEvent &p_event)
{
  if (!m_map.IsDefined(p_event.GetItem())) {
    p_event.Veto();
  }
}

void dialogEditBehav::OnSelChanged(wxTreeEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  if (m_lastInfoset) {
    for (int act = 1; act <= m_lastInfoset->NumActions(); act++) {
      m_profile.Set(m_lastInfoset->Actions()[act],
		    ToNumber(m_probGrid->GetCellValue(act - 1, 0).c_str()));
    }
  }

  m_lastInfoset = m_map.Lookup(p_event.GetItem());

  if (m_lastInfoset) {
    if (m_lastInfoset->NumActions() < m_probGrid->GetRows()) {
      m_probGrid->DeleteRows(0,
			     m_probGrid->GetRows() - m_lastInfoset->NumActions());
    }
    else if (m_lastInfoset->NumActions() > m_probGrid->GetRows()) {
      m_probGrid->InsertRows(0,
			     m_lastInfoset->NumActions() - m_probGrid->GetRows());
    }

    for (int act = 1; act <= m_lastInfoset->NumActions(); act++) {
      m_probGrid->SetLabelValue(wxVERTICAL,
				(char *) m_lastInfoset->Actions()[act]->GetName(),
				act - 1);
      m_probGrid->SetCellValue((char *) ToText(m_profile(m_lastInfoset->Actions()[act])),
			       act - 1, 0);
    }
  }
  else {
    m_probGrid->DeleteRows(m_probGrid->GetRows());
  }
}

void dialogEditBehav::OnOK(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  Infoset *infoset = m_map.Lookup(m_infosetTree->GetSelection());

  if (!infoset) {
    infoset = m_profile.GetGame().Infosets()[1];
  }

  for (int act = 1; act <= infoset->NumActions(); act++) {
    m_profile.Set(infoset->Actions()[act],
		  ToNumber(m_probGrid->GetCellValue(act - 1, 0).c_str()));
  }

  p_event.Skip();
}

const BehavSolution &dialogEditBehav::GetProfile(void) const
{
  m_profile.SetName(m_profileName->GetValue().c_str());
  return m_profile;
}

#include "base/gmap.imp"

gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Infoset *>;
template class gOrdMap<wxTreeItemId, Infoset *>;
