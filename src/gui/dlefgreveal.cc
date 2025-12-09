//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlefgreveal.cc
// Dialog for revealing actions to players
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

#include "gambit.h"
#include "dlefgreveal.h"

namespace {

using namespace Gambit;
using namespace Gambit::GUI;

class RevealMoveDialog final : public wxDialog {
  struct PlayerEntry {
    GamePlayer player;
    wxCheckBox *checkbox;
  };
  GameDocument *m_doc{nullptr};
  std::vector<PlayerEntry> m_entries;

  void OnCheckbox(wxCommandEvent &) { UpdateButtonState(); }
  void UpdateButtonState();

public:
  RevealMoveDialog(wxWindow *p_parent, GameDocument *p_doc);
  std::vector<GamePlayer> GetPlayers() const;
};

RevealMoveDialog::RevealMoveDialog(wxWindow *p_parent, GameDocument *p_doc)
  : wxDialog(p_parent, wxID_ANY, _("Reveal this move to players"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *groupLabel = new wxStaticText(this, wxID_ANY, _("Reveal this move to players"));
  auto f = groupLabel->GetFont();
  f.SetWeight(wxFONTWEIGHT_BOLD);
  groupLabel->SetFont(f);
  topSizer->Add(groupLabel, wxSizerFlags().Border(wxLEFT | wxTOP | wxRIGHT, 10));

  auto *playerBox = new wxBoxSizer(wxVERTICAL);
  playerBox->AddSpacer(3);

  const auto &players = m_doc->GetGame()->GetPlayers();
  m_entries.reserve(players.size());

  for (const auto &player : players) {
    wxString label;
    if (!player->GetLabel().empty()) {
      label = wxString::FromUTF8(player->GetLabel());
    }
    else {
      label = wxString::Format("Player %u", player->GetNumber());
    }
    auto *cb = new wxCheckBox(this, wxID_ANY, label);
    cb->SetValue(true);
    cb->SetForegroundColour(m_doc->GetStyle().GetPlayerColor(player));
    cb->Bind(wxEVT_CHECKBOX, &RevealMoveDialog::OnCheckbox, this);
    m_entries.push_back({player, cb});
    playerBox->Add(cb, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT | wxTOP, 4));
  }

  topSizer->Add(playerBox, wxSizerFlags(1).Expand().Border(wxALL, 5));

  auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
  buttonSizer->Realize();
  topSizer->Add(buttonSizer, wxSizerFlags().Right().Border(wxALL, 10));

  SetSizerAndFit(topSizer);
  CenterOnParent();
  UpdateButtonState();
}

void RevealMoveDialog::UpdateButtonState()
{
  const bool anyChecked =
      std::any_of(m_entries.begin(), m_entries.end(),
                  [](const PlayerEntry &entry) { return entry.checkbox->IsChecked(); });
  FindWindow(wxID_OK)->Enable(anyChecked);
}

std::vector<GamePlayer> RevealMoveDialog::GetPlayers() const
{
  std::vector<GamePlayer> result;
  result.reserve(m_entries.size());
  for (const auto &[player, checkbox] : m_entries) {
    if (checkbox->GetValue()) {
      result.push_back(player);
    }
  }
  return result;
}
} // anonymous namespace

namespace Gambit::GUI {

std::optional<std::vector<GamePlayer>> RevealMove(wxWindow *p_parent, GameDocument *p_doc)
{
  RevealMoveDialog dialog(p_parent, p_doc);
  if (dialog.ShowModal() == wxID_OK) {
    return dialog.GetPlayers();
  }
  return std::nullopt;
}

} // namespace Gambit::GUI
