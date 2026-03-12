//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit::GUI {

class ActionSheet final : public wxSheet {
  GameInfoset m_infoset{nullptr};
  wxSheetCellAttr m_labelAttr;
  wxFont m_labelFont, m_cellFont;

  // Overriding wxSheet members
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const override;

public:
  ActionSheet(wxWindow *p_parent, const GameInfoset &p_infoset);

  int NumActions() const { return GetNumberRows(); }

  wxString GetActionName(int p_act);
  Array<Number> GetActionProbs();
};

ActionSheet::ActionSheet(wxWindow *p_parent, const GameInfoset &p_infoset)
  : wxSheet(p_parent, wxID_ANY), m_infoset(p_infoset), m_labelFont(GetFont()),
    m_cellFont(GetFont())
{
  m_labelFont.MakeBold();

  m_labelAttr = GetSheetRefData()->m_defaultRowLabelAttr;
  m_labelAttr.SetFont(m_labelFont);
  m_labelAttr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_labelAttr.SetOrientation(wxHORIZONTAL);
  m_labelAttr.SetReadOnly(true);

  CreateGrid(p_infoset->GetActions().size(), (p_infoset->IsChanceInfoset()) ? 2 : 1);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);
  SetColLabelValue(0, wxT("Label"));
  if (p_infoset->IsChanceInfoset()) {
    SetColLabelValue(1, wxT("Probability"));
  }

  for (const auto &action : p_infoset->GetActions()) {
    wxSheet::SetCellValue(wxSheetCoords(action->GetNumber() - 1, 0),
                          wxString(action->GetLabel().c_str(), *wxConvCurrent));
    if (p_infoset->IsChanceInfoset()) {
      wxSheet::SetCellValue(
          wxSheetCoords(action->GetNumber() - 1, 1),
          wxString(static_cast<std::string>(p_infoset->GetActionProb(action)).c_str(),
                   *wxConvCurrent));
    }
  }
  SetDefaultColWidth(150);
  AutoSizeRows();
  // This addresses a regression in wxWidgets 2.9.5 with using grids and
  // sheets in sizers.
  InvalidateBestSize();
}

wxString ActionSheet::GetActionName(int p_act)
{
  if (IsCellEditControlCreated()) {
    SaveEditControlValue();
  }
  if (IsCellEditControlShown()) {
    HideCellEditControl();
  }
  return GetCellValue(wxSheetCoords(p_act - 1, 0));
}

Array<Number> ActionSheet::GetActionProbs()
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

wxSheetCellAttr ActionSheet::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  if (IsLabelCell(p_coords)) {
    return m_labelAttr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(m_cellFont);
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetReadOnly(false);
  if (p_coords.GetCol() == 1) {
    attr.SetRenderer(wxSheetCellRenderer(new RationalRendererRefData()));
    attr.SetEditor(wxSheetCellEditor(new RationalEditorRefData()));
  }
  else {
    attr.SetEditor(wxSheetCellEditor(new wxSheetCellTextEditorRefData()));
  }
  return attr;
}

//======================================================================
//                      class EditMoveDialog
//======================================================================

EditMoveDialog::EditMoveDialog(wxWindow *p_parent, const GameInfoset &p_infoset)
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

  {
    wxString label;
    label << _("Number of members: ") << p_infoset->GetMembers().size();
    topSizer->Add(new wxStaticText(this, wxID_STATIC, label), 0, wxALL | wxALIGN_CENTER, 5);
  }

  auto *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(new wxStaticText(this, wxID_STATIC, _("Belongs to player")), 0,
                   wxALL | wxALIGN_CENTER, 5);
  m_player = new wxChoice(this, wxID_ANY);
  if (p_infoset->IsChanceInfoset()) {
    m_player->Append(_("Chance"));
    m_player->SetSelection(0);
    m_player->Disable();
  }
  else {
    for (const auto &player : p_infoset->GetGame()->GetPlayers()) {
      wxString label;
      label << player->GetNumber() << ": " << player->GetLabel();
      m_player->Append(label);
    }
    m_player->SetSelection(p_infoset->GetPlayer()->GetNumber() - 1);
  }
  playerSizer->Add(m_player, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(playerSizer, 0, wxALL | wxEXPAND, 0);

  auto *actionBoxSizer =
      new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Actions")), wxHORIZONTAL);
  m_actionSheet = new ActionSheet(this, p_infoset);
  actionBoxSizer->Add(m_actionSheet, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(actionBoxSizer, 0, wxALL | wxEXPAND, 5);

  if (auto *buttons = CreateSeparatedButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttons, 0, wxALL | wxEXPAND, 5);
  }
  SetSizerAndFit(topSizer);
  CenterOnParent();

  Bind(wxEVT_BUTTON, &EditMoveDialog::OnOK, this, wxID_OK);
}

void EditMoveDialog::OnOK(wxCommandEvent &p_event)
{
  if (!m_infoset->IsChanceInfoset()) {
    p_event.Skip();
    return;
  }
  try {
    ValidateDistribution(m_actionSheet->GetActionProbs());
  }
  catch (ValueException &) {
    wxRichMessageDialog(this, "Probabilities must be nonnegative numbers summing to one.", "Error",
                        wxOK | wxCENTRE | wxICON_ERROR)
        .ShowModal();
    return;
  }
  p_event.Skip();
}

int EditMoveDialog::NumActions() const { return m_actionSheet->NumActions(); }

wxString EditMoveDialog::GetActionName(int p_act) const
{
  return m_actionSheet->GetActionName(p_act);
}

Array<Number> EditMoveDialog::GetActionProbs() const { return m_actionSheet->GetActionProbs(); }
} // namespace Gambit::GUI
