//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditmove.cc
// Dialog for viewing and editing properties of a move
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
#endif // WX_PRECOMP
#include <wx/richmsgdlg.h>
#include "wx/sheet/sheet.h"

#include "gambit.h"
#include "dleditmove.h"
#include "renratio.h"

using namespace Gambit;

class gbtActionSheet : public wxSheet {
private:
  GameInfoset m_infoset;

  // Overriding wxSheet members
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const override;

public:
  gbtActionSheet(wxWindow *p_parent, const GameInfoset &p_infoset);

  int NumActions() const { return GetNumberRows(); }

  wxString GetActionName(int p_act);
  Array<Number> GetActionProbs();
};

gbtActionSheet::gbtActionSheet(wxWindow *p_parent, const Gambit::GameInfoset &p_infoset)
  : wxSheet(p_parent, wxID_ANY), m_infoset(p_infoset)
{
  CreateGrid(p_infoset->NumActions(), (p_infoset->IsChanceInfoset()) ? 2 : 1);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);
  SetColLabelValue(0, wxT("Label"));
  if (p_infoset->IsChanceInfoset()) {
    SetColLabelValue(1, wxT("Probability"));
  }

  for (size_t act = 1; act <= p_infoset->NumActions(); act++) {
    SetCellValue(wxSheetCoords(act - 1, 0),
                 wxString(p_infoset->GetAction(act)->GetLabel().c_str(), *wxConvCurrent));
    if (p_infoset->IsChanceInfoset()) {
      SetCellValue(
          wxSheetCoords(act - 1, 1),
          wxString(static_cast<std::string>(p_infoset->GetActionProb(p_infoset->GetAction(act)))
                       .c_str(),
                   *wxConvCurrent));
    }
  }
  SetDefaultColWidth(150);
  AutoSizeRows();
  // This addresses a regression in wxWidgets 2.9.5 with using grids and
  // sheets in sizers.
  InvalidateBestSize();
}

wxString gbtActionSheet::GetActionName(int p_act)
{
  if (IsCellEditControlCreated()) {
    SaveEditControlValue();
  }
  if (IsCellEditControlShown()) {
    HideCellEditControl();
  }
  return GetCellValue(wxSheetCoords(p_act - 1, 0));
}

Array<Number> gbtActionSheet::GetActionProbs()
{
  if (IsCellEditControlCreated()) {
    SaveEditControlValue();
  }
  if (IsCellEditControlShown()) {
    HideCellEditControl();
  }
  Array<Number> probs(NumActions());
  for (int act = 1; act <= NumActions(); act++) {
    probs[act] = Number(GetCellValue(wxSheetCoords(act - 1, 1)).ToStdString());
  }
  return probs;
}

wxSheetCellAttr gbtActionSheet::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    const wxSheetCellAttr attr(GetSheetRefData()->m_defaultCornerLabelAttr);
    return attr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetReadOnly(false);
  if (p_coords.GetCol() == 1) {
    attr.SetRenderer(wxSheetCellRenderer(new gbtRationalRendererRefData()));
    attr.SetEditor(wxSheetCellEditor(new gbtRationalEditorRefData()));
  }
  else {
    attr.SetEditor(wxSheetCellEditor(new wxSheetCellTextEditorRefData()));
  }
  return attr;
}

//======================================================================
//                      class gbtEditMoveDialog
//======================================================================

wxBEGIN_EVENT_TABLE(gbtEditMoveDialog,
                    wxDialog) EVT_BUTTON(wxID_OK, gbtEditMoveDialog::OnOK) wxEND_EVENT_TABLE()

    gbtEditMoveDialog::gbtEditMoveDialog(wxWindow *p_parent, const Gambit::GameInfoset &p_infoset)
  : wxDialog(p_parent, wxID_ANY, _("Move properties"), wxDefaultPosition), m_infoset(p_infoset)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Information set label")), 0,
                  wxALL | wxALIGN_CENTER, 5);
  m_infosetName =
      new wxTextCtrl(this, wxID_ANY, wxString(p_infoset->GetLabel().c_str(), *wxConvCurrent));
  labelSizer->Add(m_infosetName, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 0);

  topSizer->Add(
      new wxStaticText(this, wxID_STATIC,
                       wxString::Format(_("Number of members: %d"), p_infoset->NumMembers())),
      0, wxALL | wxALIGN_CENTER, 5);

  auto *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Belongs to player")), 0,
                   wxALL | wxALIGN_CENTER, 5);
  m_player = new wxChoice(this, wxID_ANY);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append(_("Chance"));
    m_player->SetSelection(0);
  }
  else {
    for (const auto &player : p_infoset->GetGame()->GetPlayers()) {
      m_player->Append(wxString::Format(_T("%d: "), player->GetNumber()) +
                       wxString(player->GetLabel().c_str(), *wxConvCurrent));
    }
    m_player->SetSelection(p_infoset->GetPlayer()->GetNumber() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 0);

  auto *actionBoxSizer =
      new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Actions")), wxHORIZONTAL);
  m_actionSheet = new gbtActionSheet(this, p_infoset);
  actionBoxSizer->Add(m_actionSheet, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(actionBoxSizer, 0, wxALL | wxEXPAND, 5);

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void gbtEditMoveDialog::OnOK(wxCommandEvent &p_event)
{
  if (!m_infoset->IsChanceInfoset()) {
    p_event.Skip();
    return;
  }
  Array<Number> probs = m_actionSheet->GetActionProbs();
  Rational sum(0);
  for (size_t act = 1; act <= m_infoset->NumActions(); act++) {
    auto prob = static_cast<Rational>(probs[act]);
    if (prob < Gambit::Rational(0)) {
      wxMessageBox("Action probabilities must be nonnegative numbers.", "Error");
      return;
    }
    sum += prob;
  }
  if (sum == Gambit::Rational(1)) {
    p_event.Skip();
  }
  else {
    wxRichMessageDialog(this, "Action probabilities must sum to exactly one", "Error",
                        wxOK | wxCENTRE | wxICON_ERROR)
        .ShowModal();
  }
}

int gbtEditMoveDialog::NumActions() const { return m_actionSheet->NumActions(); }

wxString gbtEditMoveDialog::GetActionName(int p_act) const
{
  return m_actionSheet->GetActionName(p_act);
}

Array<Number> gbtEditMoveDialog::GetActionProbs() const { return m_actionSheet->GetActionProbs(); }
