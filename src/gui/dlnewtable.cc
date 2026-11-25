#include "dlnewtable.h"

#include <algorithm>
#include <vector>

#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include "dialogstyle.h"

namespace Gambit::GUI {

namespace {
constexpr int ID_PLAYER_COUNT = wxID_HIGHEST + 401;
constexpr int ID_FIRST_STRATEGY_COUNT = wxID_HIGHEST + 500;
} // namespace

NewTableDialog::NewTableDialog(wxWindow *parent)
  : wxDialog(parent, wxID_ANY, _("New strategic-form game"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  CreateControls();
  LayoutControls();
  RebuildStrategyControls();
}

std::vector<int> NewTableDialog::GetDimensions() const
{
  std::vector<int> dimensions;
  dimensions.reserve(m_strategyCtrls.size());

  for (const auto *ctrl : m_strategyCtrls) {
    dimensions.push_back(ctrl->GetValue());
  }

  return dimensions;
}

void NewTableDialog::CreateControls()
{
  auto *playerCountLabel = new wxStaticText(this, wxID_ANY, _("Number of players:"));
  m_playerCountCtrl = new wxSpinCtrl(this, ID_PLAYER_COUNT);
  m_playerCountCtrl->SetRange(2, 16);
  m_playerCountCtrl->SetValue(2);

  m_strategyRowsSizer = new wxBoxSizer(wxVERTICAL);

  Bind(wxEVT_SPINCTRL, &NewTableDialog::OnPlayerCountChanged, this, ID_PLAYER_COUNT);

  // Store labels locally for layout construction.
  playerCountLabel->SetName("playerCountLabel");
}

void NewTableDialog::LayoutControls()
{
  m_topSizer = new wxBoxSizer(wxVERTICAL);

  auto *formSizer = NewFormSizer(this);

  auto *playerCountLabel = FindWindowByName("playerCountLabel", this);
  formSizer->Add(playerCountLabel, 0, wxALIGN_CENTER_VERTICAL);
  formSizer->Add(m_playerCountCtrl, 0);

  auto *strategyBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Strategies per player"));
  strategyBox->Add(m_strategyRowsSizer, 0, wxEXPAND | wxALL, FromDIP(c_spaceRelated));

  m_topSizer->Add(formSizer, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceMargin));
  m_topSizer->Add(strategyBox, 0, wxEXPAND);

  SetDialogLayout(this, m_topSizer, wxOK | wxCANCEL);
}

void NewTableDialog::RebuildStrategyControls()
{
  for (auto *label : m_strategyLabels) {
    label->Destroy();
  }
  for (auto *ctrl : m_strategyCtrls) {
    ctrl->Destroy();
  }

  m_strategyLabels.clear();
  m_strategyCtrls.clear();

  m_strategyRowsSizer->Clear(false);

  const int playerCount = m_playerCountCtrl->GetValue();

  auto *grid = NewFormSizer(this);

  for (int i = 0; i < playerCount; ++i) {
    auto *label = new wxStaticText(this, wxID_ANY, wxString::Format(_("Player %d:"), i + 1));
    auto *ctrl = new wxSpinCtrl(this, ID_FIRST_STRATEGY_COUNT + i);

    ctrl->SetRange(1, 99);
    ctrl->SetValue(2);

    grid->Add(label, 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(ctrl, 0);

    m_strategyLabels.push_back(label);
    m_strategyCtrls.push_back(ctrl);
  }

  m_strategyRowsSizer->Add(grid, 0, wxEXPAND);
  GetSizer()->SetSizeHints(this);
}

void NewTableDialog::OnPlayerCountChanged(wxSpinEvent &p_event)
{
  const int oldCount = static_cast<int>(m_strategyCtrls.size());
  const int newCount = p_event.GetValue();

  std::vector<int> existingValues;
  existingValues.reserve(m_strategyCtrls.size());
  for (const auto *ctrl : m_strategyCtrls) {
    existingValues.push_back(ctrl->GetValue());
  }

  RebuildStrategyControls();

  for (int i = 0; i < std::min(oldCount, newCount); ++i) {
    m_strategyCtrls[i]->SetValue(existingValues[i]);
  }

  for (int i = oldCount; i < newCount; ++i) {
    m_strategyCtrls[i]->SetValue(2);
  }
}

} // namespace Gambit::GUI
