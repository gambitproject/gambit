//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlgameprop.cc
// Dialog for viewing and editing properties of a game
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

#include <algorithm>
#include <functional>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/colourdata.h>
#ifdef __WXMAC__
#include <wx/generic/colrdlgg.h>
#else
#include <wx/colordlg.h>
#endif

#include "gambit.h"
#include "games/gametree.h"
#include "gamedoc.h"
#include "dlgameprop.h"
#include "editlabel.h"
#include "labellist.h"

namespace Gambit::GUI {

namespace {
// A neutral placeholder swatch for a player that doesn't exist yet; freely overridable via
// the row's own colour button before the dialog is committed.
const wxColour kNewPlayerColor(200, 200, 200);

// wxGenericColourDialog, not the platform's native wxColourDialog, on macOS/Cocoa only:
// its native colour panel has no real Cancel -- any interaction with it commits live,
// with no way back short of canceling this entire properties dialog. The generic dialog
// draws its own honest OK/Cancel buttons. Elsewhere, the native dialog's Cancel already
// works properly, and the generic dialog's header isn't guaranteed to be available (it
// isn't shipped with the GTK/MSW dev packages used by CI, only bundled on macOS/Homebrew).
#ifdef __WXMAC__
using PlatformColourDialog = wxGenericColourDialog;
#else
using PlatformColourDialog = wxColourDialog;
#endif
} // namespace

//========================================================================
//                        class PlayerPanel
//========================================================================

// An editable, variable-length list of the game's players (not including chance), colour-coded
// like a source-control diff (added/renamed/deleted), mirroring StrategyPanel
// (dleditstrategies.cc) with an added per-row colour swatch. The rows are held in a
// `LabelListEdit`; this panel draws them and forwards the user's actions to it.
class PlayerPanel final : public wxScrolledWindow {
public:
  struct PlayerData {
    GamePlayer player; // null for a row added in this dialog, with no prior player at all
    wxColour color;    // staged: not written to the document until the dialog's OK is committed
  };

private:
  std::shared_ptr<GameDocument> m_doc;
  LabelListEdit<PlayerData> m_list;
  // One of each per row of m_list, recreated by each Rebuild()
  std::vector<LabelTextCtrl *> m_labelCtrls;
  std::vector<wxBitmapButton *> m_colorButtons;
  wxBoxSizer *m_topSizer;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;
  // False until the constructor completes, so Rebuild()'s initial call can't notify the
  // owning dialog before it's finished being constructed.
  bool m_ready = false;
  // True from the moment a row mutation starts until its (deferred) Rebuild() finishes; see
  // StrategyPanel (dleditstrategies.cc) for why this guard is needed.
  bool m_rebuilding = false;

  // The reason `row` can't be deleted because of the game's own rules -- it has decisions (in
  // an extensive game) or more than one strategy (in a strategic game) -- or an empty string
  // if there's no such structural obstruction. Independent of the separate "at least one
  // player must remain" rule enforced where this is used.
  wxString BlockedDeleteReason(const LabelListEdit<PlayerData>::Row &row) const;
  void Rebuild();
  // See StrategyPanel::ScheduleRebuild() (dleditstrategies.cc).
  void ScheduleRebuild();
  void NotifyChanged() const
  {
    if (m_ready) {
      m_onChanged();
    }
  }

  void OnSetColor(int p_index);
  // Builds a fresh color-swatch button for a row and swaps it in for the old one, rather than
  // mutating the existing button's bitmap in place -- see PlayerPanel::OnSetColor() for why.
  void RecreateColorButton(int p_index);
  wxBitmapButton *MakeColorButton(int p_index);

public:
  PlayerPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
              const std::function<void()> &p_onChanged);

  // The rows as edited, with each label as normalized in its control
  const LabelListEdit<PlayerData> &GetEdit();

  void AddPlayer();
  // Marks an existing row deleted (or restores an already-deleted one); removes a row added
  // in this dialog outright. A no-op if this would leave no active (non-deleted) rows, or if
  // the row is structurally blocked from deletion (see BlockedDeleteReason()).
  void ToggleDeleted(int p_index);
  void MovePlayerUp(int p_index);
  void MovePlayerDown(int p_index);

  // Highlights any empty or duplicate player labels among active (non-deleted) rows, and
  // returns a description of the first problem found, or an empty string if all are valid.
  wxString CheckLabels();
};

PlayerPanel::PlayerPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                         const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL),
    m_doc(p_doc), m_topSizer(new wxBoxSizer(wxVERTICAL)),
    m_defaultBg(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)), m_onChanged(p_onChanged)
{
  for (const auto &player : m_doc->GetGame()->GetPlayers()) {
    m_list.AddExisting(player->GetLabel(),
                       {.player = player, .color = m_doc->GetStyle().GetPlayerColor(player)});
  }

  SetSizer(m_topSizer);
  SetScrollRate(0, FromDIP(10));

  Rebuild();

  const wxSize bestSize = m_topSizer->CalcMin();
  SetMinSize(wxSize(FromDIP(400), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_labelCtrls.empty()) {
    m_defaultBg = m_labelCtrls.front()->GetBackgroundColour();
  }

  m_ready = true;
}

const LabelListEdit<PlayerPanel::PlayerData> &PlayerPanel::GetEdit()
{
  for (size_t i = 0; i < m_labelCtrls.size(); i++) {
    m_list.SetLabel(i, m_labelCtrls[i]->GetNormalizedValue().ToStdString(wxConvUTF8));
  }
  return m_list;
}

wxString PlayerPanel::BlockedDeleteReason(const LabelListEdit<PlayerData>::Row &row) const
{
  if (row.isNew) {
    return {}; // nothing has been committed yet -- freely removable
  }
  if (As<GameTreeRep>(m_doc->GetGame())) {
    if (row.payload.player->GetInfosets().size() > 0) {
      return _("This player has decisions in the game and cannot be deleted.");
    }
  }
  else if (row.payload.player->GetStrategies().size() != 1) {
    return _("This player has more than one strategy and cannot be deleted.");
  }
  return {};
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void PlayerPanel::Rebuild()
{
  m_rebuilding = true;
  m_topSizer->Clear(true); // destroys the previous row controls; the rows themselves are in
                           // m_list, which they don't own
  m_labelCtrls.clear();
  m_colorButtons.clear();

  auto *gridSizer = new wxFlexGridSizer(4, FromDIP(5), FromDIP(10));
  gridSizer->AddGrowableCol(2, 1);

  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Label")), 0, wxALIGN_CENTER_VERTICAL);
  gridSizer->AddSpacer(1);

  const size_t activeCount = m_list.ActiveCount();
  const wxSize buttonSize(FromDIP(28), -1);
  const int buttonGap = FromDIP(2);

  for (size_t i = 0; i < m_list.Size(); i++) {
    const auto &row = m_list.GetRow(i);

    m_colorButtons.push_back(MakeColorButton(static_cast<int>(i)));
    gridSizer->Add(m_colorButtons.back(), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(5));

    wxString number;
    number << (i + 1);
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    auto *labelCtrl = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(row.label));
    m_labelCtrls.push_back(labelCtrl);
    if (row.isDeleted) {
      wxFont strikeFont = labelCtrl->GetFont();
      strikeFont.SetStrikethrough(true);
      labelCtrl->SetFont(strikeFont);
      labelCtrl->Disable();
    }
    else {
      StyleLabelRow(labelCtrl, row.isNew, row.stableLabel, row.label);
    }
    labelCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
      if (!m_rebuilding && i < m_list.Size()) {
        m_list.SetLabel(i, m_labelCtrls[i]->GetValue().ToStdString(wxConvUTF8));
        const auto &changedRow = m_list.GetRow(i);
        StyleLabelRow(m_labelCtrls[i], changedRow.isNew, changedRow.stableLabel, changedRow.label);
        NotifyChanged();
      }
      p_event.Skip();
    });
    labelCtrl->Bind(wxEVT_KILL_FOCUS, [this, i](wxFocusEvent &p_event) {
      if (!m_rebuilding && i < m_list.Size()) {
        m_list.SetLabel(i, m_labelCtrls[i]->GetNormalizedValue().ToStdString(wxConvUTF8));
        const auto &changedRow = m_list.GetRow(i);
        StyleLabelRow(m_labelCtrls[i], changedRow.isNew, changedRow.stableLabel, changedRow.label);
        NotifyChanged();
      }
      p_event.Skip();
    });
    gridSizer->Add(labelCtrl, 1, wxEXPAND);

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!row.isDeleted) {
      auto *upButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2191), wxDefaultPosition, buttonSize);
      upButton->Enable(i > 0);
      upButton->Bind(wxEVT_BUTTON,
                     [this, i](wxCommandEvent &) { MovePlayerUp(static_cast<int>(i)); });
      buttonSizer->Add(upButton, 0, wxLEFT, buttonGap);

      auto *downButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2193), wxDefaultPosition, buttonSize);
      downButton->Enable(i + 1 < m_list.Size());
      downButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { MovePlayerDown(static_cast<int>(i)); });
      buttonSizer->Add(downButton, 0, wxLEFT, buttonGap);
    }
    else {
      // Keep the delete/restore button aligned under its counterpart in other rows, in place
      // of the Up/Down buttons a deleted row no longer has.
      buttonSizer->AddSpacer(buttonSize.GetWidth() + buttonGap);
      buttonSizer->AddSpacer(buttonSize.GetWidth() + buttonGap);
    }

    const wxString blockedReason = row.isDeleted ? wxString() : BlockedDeleteReason(row);
    const bool canDelete = row.isDeleted || (blockedReason.empty() && activeCount > 1);

    auto *deleteButton =
        new wxButton(this, wxID_ANY, row.isDeleted ? wxUniChar(0x21BA) : wxUniChar(0x2715),
                     wxDefaultPosition, buttonSize);
    if (row.isDeleted) {
      deleteButton->SetToolTip(_("Restore player"));
    }
    else if (!blockedReason.empty()) {
      deleteButton->SetToolTip(blockedReason);
    }
    else if (activeCount <= 1) {
      deleteButton->SetToolTip(_("At least one player must remain."));
    }
    else {
      deleteButton->SetToolTip(_("Delete player"));
    }
    deleteButton->Enable(canDelete);
    deleteButton->Bind(wxEVT_BUTTON,
                       [this, i](wxCommandEvent &) { ToggleDeleted(static_cast<int>(i)); });
    buttonSizer->Add(deleteButton, 0, wxLEFT, buttonGap);

    gridSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);
  }

  // A trailing row with only its button-column cell populated, so the "+" control lands
  // directly below the last row's Up/Down/Delete buttons rather than as a separate control
  // elsewhere in the dialog.
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  auto *addButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, buttonSize);
  addButton->SetToolTip(_("Add player"));
  addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { AddPlayer(); });
  auto *addSizer = new wxBoxSizer(wxHORIZONTAL);
  addSizer->Add(addButton, 0, wxLEFT, buttonGap);
  gridSizer->Add(addSizer, 0, wxALIGN_CENTER_VERTICAL);

  m_topSizer->Add(gridSizer, 1, wxALL | wxEXPAND, FromDIP(5));
  FitInside();
  Layout();
  m_rebuilding = false;
  // No NotifyChanged() call here -- see m_ready's comment. Callers that mutate m_list are
  // responsible for notifying once Rebuild() returns.
}

void PlayerPanel::ScheduleRebuild()
{
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void PlayerPanel::AddPlayer()
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  m_list.AddNew("Player ", {.player = nullptr, .color = kNewPlayerColor});
  ScheduleRebuild();
}

void PlayerPanel::ToggleDeleted(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  const auto &row = m_list.GetRow(p_index);
  if (!row.isDeleted && !BlockedDeleteReason(row).empty()) {
    return; // structurally blocked; the disabled button shouldn't have fired regardless
  }
  if (m_list.ToggleDeleted(p_index)) {
    ScheduleRebuild();
  }
}

void PlayerPanel::MovePlayerUp(int p_index)
{
  if (!m_rebuilding && m_list.MoveUp(p_index)) {
    ScheduleRebuild();
  }
}

void PlayerPanel::MovePlayerDown(int p_index)
{
  if (!m_rebuilding && m_list.MoveDown(p_index)) {
    ScheduleRebuild();
  }
}

wxString PlayerPanel::CheckLabels()
{
  return ValidateLabels(m_list.Validate(), m_labelCtrls, m_defaultBg,
                        _("Player labels cannot be empty."), _("Player labels must be unique."));
}

wxBitmapButton *PlayerPanel::MakeColorButton(int p_index)
{
  const auto &row = m_list.GetRow(p_index);
  auto *button = new wxBitmapButton(this, wxID_ANY, MakeColorSwatch(row.payload.color),
                                    wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  button->SetToolTip(_("Change the color for this player"));
  button->Bind(wxEVT_BUTTON, [this, p_index](wxCommandEvent &) { OnSetColor(p_index); });
  return button;
}

void PlayerPanel::OnSetColor(int p_index)
{
  auto &row = m_list.GetRow(p_index);

  wxColourData data;
  data.SetColour(row.payload.color);
  PlatformColourDialog dialog(this, &data);
  dialog.SetTitle(wxString::Format(_("Choose color for %s"), wxString::FromUTF8(row.label)));

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  // Staged only -- not written to the document until the dialog's OK is committed (by
  // GameFrame::OnEditGame), so canceling the dialog leaves colors untouched.
  row.payload.color = dialog.GetColourData().GetColour();

  // Recreated (rather than mutated in place via SetBitmap()) because SetBitmap() on an
  // existing wxBitmapButton visibly shifted its rendered content on this platform. Deferred
  // via CallAfter(): this destroys the very button whose click invoked this handler, which is
  // still on the call stack -- see StrategyPanel::ScheduleRebuild() in dleditstrategies.cc for
  // the same hazard.
  CallAfter([this, p_index]() { RecreateColorButton(p_index); });
}

void PlayerPanel::RecreateColorButton(int p_index)
{
  wxBitmapButton *oldButton = m_colorButtons.at(p_index);
  wxBitmapButton *newButton = MakeColorButton(p_index);

  // The button lives in a gridSizer nested inside m_topSizer, not directly in m_topSizer
  // itself, so the search needs to recurse into child sizers to find it.
  m_topSizer->Replace(oldButton, newButton, true);
  oldButton->Destroy();
  m_colorButtons.at(p_index) = newButton;

  Layout();
}

//========================================================================

GamePropertiesDialog::GamePropertiesDialog(wxWindow *p_parent,
                                           const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_parent, wxID_ANY, _("Game properties"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  const int S = FromDIP(5);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *notebook = new wxNotebook(this, wxID_ANY);

  auto *generalPanel = new wxPanel(notebook);
  auto *generalSizer = new wxBoxSizer(wxVERTICAL);

  auto *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Title")), 0,
                  wxALL | wxALIGN_CENTER, S);
  m_title =
      new wxTextCtrl(generalPanel, wxID_ANY, wxString::FromUTF8(m_doc->GetGame()->GetTitle()),
                     wxDefaultPosition, wxSize(FromDIP(400), -1));
  titleSizer->Add(m_title, 1, wxALL | wxALIGN_CENTER, S);
  generalSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  auto *commentSizer = new wxBoxSizer(wxHORIZONTAL);
  commentSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Comment")), 0,
                    wxALL | wxALIGN_CENTER, S);
  m_comment = new wxTextCtrl(generalPanel, wxID_ANY,
                             wxString::FromUTF8(m_doc->GetGame()->GetDescription()),
                             wxDefaultPosition, wxSize(FromDIP(400), -1), wxTE_MULTILINE);
  commentSizer->Add(m_comment, 1, wxALL | wxALIGN_CENTER, S);
  generalSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 0);

  auto *boxSizer =
      new wxStaticBoxSizer(wxVERTICAL, generalPanel, _("Information about this game"));

  boxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC,
                                 wxString(_("Filename: ")) + m_doc->GetFilename()),
                0, wxALL, S);

  const Game game = m_doc->GetGame();
  if (game->IsConstSum()) {
    boxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("This is a constant-sum game")), 0,
                  wxALL, S);
  }
  else {
    boxSizer->Add(
        new wxStaticText(generalPanel, wxID_STATIC, _("This is not a constant-sum game")), 0,
        wxALL, S);
  }

  if (As<GameTreeRep>(game)) {
    if (game->IsPerfectRecall()) {
      boxSizer->Add(
          new wxStaticText(generalPanel, wxID_STATIC, _("This is a game of perfect recall")), 0,
          wxALL, S);
    }
    else {
      boxSizer->Add(
          new wxStaticText(generalPanel, wxID_STATIC, _("This is not a game of perfect recall")),
          0, wxALL, S);
    }
  }

  generalSizer->Add(boxSizer, 0, wxALL | wxEXPAND, S);
  generalPanel->SetSizer(generalSizer);
  notebook->AddPage(generalPanel, _("General"));

  auto *playersPanel = new wxPanel(notebook);
  auto *playersSizer = new wxBoxSizer(wxVERTICAL);

  if (As<GameTreeRep>(game)) {
    m_chanceColor = m_doc->GetStyle().ChanceColor();
    auto *chanceSizer = new wxBoxSizer(wxHORIZONTAL);
    m_chanceColorButton = MakeChanceColorButton(playersPanel);
    chanceSizer->Add(m_chanceColorButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));
    chanceSizer->Add(new wxStaticText(playersPanel, wxID_STATIC,
                                      wxString::FromUTF8(game->GetChance()->GetLabel())),
                     0, wxALIGN_CENTER_VERTICAL);
    playersSizer->Add(chanceSizer, 0, wxALL, FromDIP(5));
  }

  m_playerPanel = new PlayerPanel(playersPanel, m_doc, [this]() { UpdateValidation(); });
  playersSizer->Add(m_playerPanel, 1, wxALL | wxEXPAND, FromDIP(5));

  playersPanel->SetSizer(playersSizer);
  notebook->AddPage(playersPanel, _("Players"));

  topSizer->Add(notebook, 1, wxALL | wxEXPAND, S);

  m_errorText = new wxStaticText(this, wxID_STATIC, wxEmptyString);
  m_errorText->SetForegroundColour(*wxRED);
  topSizer->Add(m_errorText, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, S);

  if (auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, S);
  }

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  SetSize(GetBestSize());
  SetMinSize(GetSize());
  CenterOnParent();

  UpdateValidation();
}

void GamePropertiesDialog::UpdateValidation()
{
  const wxString message = m_playerPanel->CheckLabels();
  m_errorText->SetLabel(message);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(message.empty());
  }
}

GamePropertiesEdit GamePropertiesDialog::GetEdit() const
{
  const auto &players = m_playerPanel->GetEdit();
  GamePropertiesEdit edit{.title = m_title->GetValue().ToStdString(wxConvUTF8),
                          .description = m_comment->GetValue().ToStdString(wxConvUTF8),
                          .stableLabels = players.GetStableLabels(),
                          .labels = players.GetLabels(),
                          .playerColors = {},
                          .chanceColor = m_chanceColor};
  for (size_t i = 0; i < players.Size(); i++) {
    if (!players.GetRow(i).isDeleted) {
      edit.playerColors.push_back(players.GetRow(i).payload.color);
    }
  }
  return edit;
}

wxBitmapButton *GamePropertiesDialog::MakeChanceColorButton(wxWindow *p_parent)
{
  auto *button = new wxBitmapButton(p_parent, wxID_ANY, MakeColorSwatch(m_chanceColor),
                                    wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  button->SetToolTip(_("Change the color for chance"));
  button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { OnSetChanceColor(); });
  return button;
}

void GamePropertiesDialog::OnSetChanceColor()
{
  wxColourData data;
  data.SetColour(m_chanceColor);
  PlatformColourDialog dialog(this, &data);
  dialog.SetTitle(_("Choose color for chance"));

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  m_chanceColor = dialog.GetColourData().GetColour();
  CallAfter([this]() { RecreateChanceColorButton(); });
}

void GamePropertiesDialog::RecreateChanceColorButton()
{
  wxWindow *parent = m_chanceColorButton->GetParent();
  wxBitmapButton *oldButton = m_chanceColorButton;
  wxBitmapButton *newButton = MakeChanceColorButton(parent);

  parent->GetSizer()->Replace(oldButton, newButton, true);
  oldButton->Destroy();
  m_chanceColorButton = newButton;

  parent->Layout();
}

} // namespace Gambit::GUI
