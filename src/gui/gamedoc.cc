//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/gamedoc.cc
// Implementation of game document class
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

#include <sstream>
#include <fstream>

#include "gambit.h"
#include "core/tinyxml.h" // for XML parser for LoadDocument()

#include "app.h" // for wxGetApp()
#include "gamedoc.h"

namespace Gambit::GUI {

//=========================================================================
//                   class StrategyDominanceStack
//=========================================================================

StrategyDominanceStack::StrategyDominanceStack(GameDocument *p_doc, bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
{
  Reset();
}

void StrategyDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict) {
    Reset();
  }
  m_strict = p_strict;
}

void StrategyDominanceStack::Reset()
{
  m_supports.clear();
  m_supports.push_back(std::make_shared<StrategySupportProfile>(m_doc->GetGame()));
  m_current = 1;
  m_noFurther = false;
}

bool StrategyDominanceStack::NextLevel()
{
  if (m_current < m_supports.size()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  const StrategySupportProfile newSupport = m_supports[m_current]->Undominated(m_strict);

  if (newSupport != *m_supports[m_current]) {
    m_supports.push_back(std::make_shared<StrategySupportProfile>(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool StrategyDominanceStack::PreviousLevel()
{
  if (m_current > 1) {
    m_current--;
    return true;
  }
  else {
    return false;
  }
}

//=========================================================================
//                          class AnalysisWorkspace
//=========================================================================

AnalysisWorkspace::AnalysisWorkspace(GameDocument *p_doc)
  : m_doc(p_doc), m_stratSupports(p_doc, true), m_currentProfileList(0)
{
}

void AnalysisWorkspace::Clear()
{
  m_stratSupports.Reset();
  m_profiles.clear();
  m_currentProfileList = 0;
}

void AnalysisWorkspace::ResetForGameChange()
{
  m_stratSupports.Reset();

  // Even though modifications only to payoffs doesn't make the
  // computed profiles invalid for the edited game, it does mean
  // that, in general, they won't be Nash.  For now, to avoid confusion,
  // we will wipe them out.
  m_profiles.clear();
  m_currentProfileList = 0;
}

void AnalysisWorkspace::BuildNfg()
{
  m_stratSupports.Reset();
  std::for_each(m_profiles.begin(), m_profiles.end(), std::mem_fn(&AnalysisOutput::BuildNfg));
}

void AnalysisWorkspace::AddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs)
{
  m_profiles.push_back(p_profs);
  m_currentProfileList = m_profiles.size();
}

void AnalysisWorkspace::SelectEquilibriumOutput(int p_index) { m_currentProfileList = p_index; }

void AnalysisWorkspace::SelectProfile(int p_profile)
{
  m_profiles[m_currentProfileList]->SetCurrent(p_profile);
}

void AnalysisWorkspace::SetDominanceStrictness(bool p_strict)
{
  m_stratSupports.SetStrict(p_strict);
}

bool AnalysisWorkspace::GetStrategyElimStrength() const { return m_stratSupports.GetStrict(); }

bool AnalysisWorkspace::NextDominanceLevel() { return m_stratSupports.NextLevel(); }

void AnalysisWorkspace::PreviousDominanceLevel() { m_stratSupports.PreviousLevel(); }

void AnalysisWorkspace::TopDominanceLevel() { m_stratSupports.TopLevel(); }

bool AnalysisWorkspace::CanStrategyElim() const { return m_stratSupports.CanEliminate(); }

int AnalysisWorkspace::GetStrategyElimLevel() const { return m_stratSupports.GetLevel(); }

void AnalysisWorkspace::Save(std::ostream &p_file) const
{
  std::for_each(m_profiles.begin(), m_profiles.end(),
                [&p_file](std::shared_ptr<AnalysisOutput> a) { a->Save(p_file); });
}

bool AnalysisWorkspace::Load(TiXmlNode *p_game)
{
  m_stratSupports.Reset();

  m_profiles.clear();

  for (TiXmlNode *analysis = p_game->FirstChild("analysis"); analysis;
       analysis = analysis->NextSibling()) {
    const char *type = analysis->ToElement()->Attribute("type");
    // const char *rep = analysis->ToElement()->Attribute("rep");
    if (type && !strcmp(type, "list")) {
      // Read in a list of profiles
      // We need to try to guess whether the profiles are float or rational
      bool isFloat = false;
      for (TiXmlNode *profile = analysis->FirstChild("profile"); profile;
           profile = profile->NextSiblingElement()) {
        if (std::string(profile->FirstChild()->Value()).find('.') != std::string::npos ||
            std::string(profile->FirstChild()->Value()).find('e') != std::string::npos) {
          isFloat = true;
          break;
        }
      }

      if (isFloat) {
        auto plist = std::make_shared<AnalysisProfileList<double>>(m_doc, false);
        plist->Load(analysis);
        m_profiles.push_back(plist);
      }
      else {
        auto plist = std::make_shared<AnalysisProfileList<Rational>>(m_doc, false);
        plist->Load(analysis);
        m_profiles.push_back(plist);
      }
    }
  }

  m_currentProfileList = m_profiles.size();

  return true;
}

//=========================================================================
//                          class GameDocument
//=========================================================================

GameDocument::GameDocument(Game p_game)
  : m_game(p_game), m_selectNode(nullptr), m_gameModified(false), m_workspaceModified(false),
    m_workspace(this)
{
  wxGetApp().AddDocument(this);

  std::ostringstream s;
  SaveWorkspace(s);
}

GameDocument::~GameDocument() { wxGetApp().RemoveDocument(this); }

bool GameDocument::LoadWorkspace(const wxString &p_filename)
{
  TiXmlDocument doc(p_filename.mb_str());
  if (!doc.LoadFile()) {
    // Some error occurred.  Do something smart later.
    return false;
  }

  TiXmlNode *docroot = doc.FirstChild("gambit:document");

  if (!docroot) {
    // This is an "old-style" file that didn't have a proper root.
    docroot = &doc;
  }

  TiXmlNode *game = docroot->FirstChild("game");
  if (!game) {
    // There ought to be at least one game child.  If not... umm...
    return false;
  }

  TiXmlNode *efgfile = game->FirstChild("efgfile");
  if (efgfile) {
    try {
      std::istringstream s(efgfile->FirstChild()->Value());
      m_game = ReadGame(s);
    }
    catch (...) {
      return false;
    }
  }

  TiXmlNode *nfgfile = game->FirstChild("nfgfile");
  if (nfgfile) {
    try {
      std::istringstream s(nfgfile->FirstChild()->Value());
      m_game = ReadGame(s);
    }
    catch (...) {
      return false;
    }
  }

  if (!efgfile && !nfgfile) {
    // No game representation... punt!
    return false;
  }

  if (!m_workspace.Load(game)) {
    return false;
  }

  TiXmlNode *colors = docroot->FirstChild("colors");
  if (colors) {
    m_style.SetColorXML(colors);
  }
  TiXmlNode *font = docroot->FirstChild("font");
  if (font) {
    m_style.SetFontXML(font);
  }
  TiXmlNode *layout = docroot->FirstChild("autolayout");
  if (layout) {
    m_style.SetLayoutXML(layout);
  }
  TiXmlNode *labels = docroot->FirstChild("labels");
  if (labels) {
    m_style.SetLabelXML(labels);
  }
  TiXmlNode *numbers = docroot->FirstChild("numbers");
  if (numbers) {
    int numDecimals = 4;
    numbers->ToElement()->QueryIntAttribute("decimals", &numDecimals);
    m_style.SetNumDecimals(numDecimals);
  }

  return true;
}

void GameDocument::SaveWorkspace(std::ostream &p_file) const
{
  p_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

  p_file << "<gambit:document xmlns:gambit=\"http://gambit.sourceforge.net/\" version=\"0.1\">\n";

  p_file << m_style.GetColorXML();
  p_file << m_style.GetFontXML();

  if (m_game->IsTree()) {
    p_file << m_style.GetLayoutXML();
    p_file << m_style.GetLabelXML();
  }

  p_file << "<numbers decimals=\"" << m_style.NumDecimals() << "\"/>\n";

  p_file << "<game>\n";

  if (m_game->IsTree()) {
    p_file << "<efgfile>\n";
    m_game->Write(p_file, "efg");
    p_file << "</efgfile>\n";
  }
  else {
    p_file << "<nfgfile>\n";
    m_game->Write(p_file, "nfg");
    p_file << "</nfgfile>\n";
  }

  m_workspace.Save(p_file);

  p_file << "</game>\n";

  p_file << "</gambit:document>\n";
}

void GameDocument::NotifyChanged(GameModificationType p_modifications)
{
  m_gameModified |= HasModification(p_modifications, GameModificationType::GameForm |
                                                         GameModificationType::GamePayoffs |
                                                         GameModificationType::GameLabels);
  m_workspaceModified |= HasModification(p_modifications, GameModificationType::Workspace);
  if (HasModification(p_modifications,
                      GameModificationType::GameForm | GameModificationType::GamePayoffs)) {
    m_workspace.ResetForGameChange();
  }
  UpdateViews();
}

void GameDocument::UpdateViews()
{
  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::OnUpdate));
}

void GameDocument::PostPendingChanges()
{
  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::PostPendingChanges));
}

void GameDocument::BuildNfg()
{
  if (m_game->IsTree()) {
    m_workspace.BuildNfg();
  }
}

GameAction GameDocument::GetAction(int p_index) const
{
  int index = 1;
  for (auto player : m_game->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        if (index++ == p_index) {
          return action;
        }
      }
    }
  }
  throw std::out_of_range("Action index out of range");
}

void GameDocument::SetStyle(const TreeRenderConfig &p_style)
{
  m_style = p_style;
  NotifyChanged(GameModificationType::Presentation);
}

void GameDocument::DoSelectProfile(int p_profile)
{
  m_workspace.SelectProfile(p_profile);
  UpdateViews();
}

void GameDocument::DoAddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs)
{
  m_workspace.AddEquilibriumOutput(p_profs);
  NotifyChanged(GameModificationType::Workspace);
}

void GameDocument::DoAddOutput(AnalysisOutput &p_list, const wxString &p_output)
{
  p_list.AddOutput(p_output);
  NotifyChanged(GameModificationType::Workspace);
}

void GameDocument::DoSelectEquilibriumOutput(int p_index)
{
  m_workspace.SelectEquilibriumOutput(p_index);
  UpdateViews();
}

void GameDocument::DoSetDominanceStrictness(bool p_strict)
{
  m_workspace.SetDominanceStrictness(p_strict);
  UpdateViews();
}

bool GameDocument::DoNextDominanceLevel()
{
  const bool ret = m_workspace.NextDominanceLevel();
  UpdateViews();
  return ret;
}

void GameDocument::DoPreviousDominanceLevel()
{
  m_workspace.PreviousDominanceLevel();
  UpdateViews();
}

void GameDocument::DoTopDominanceLevel()
{
  m_workspace.TopDominanceLevel();
  UpdateViews();
}

void GameDocument::SetSelectNode(GameNode p_node)
{
  m_selectNode = p_node;
  UpdateViews();
}

//======================================================================
// Commands for model part of MVC architecture start here.
//======================================================================

void GameDocument::DoSave(const wxString &p_filename, GameSaveFormat p_format)
{
  std::ofstream file(p_filename.mb_str());
  if (!file) {
    throw std::runtime_error(std::string("Unable to open file for writing: ") +
                             static_cast<const char *>(p_filename.mb_str()));
  }
  switch (p_format) {
  case GameSaveFormat::Workspace:
    SaveWorkspace(file);
    m_filename = p_filename;
    m_gameModified = false;
    m_workspaceModified = false;
    break;

  case GameSaveFormat::Efg:
    m_game->Write(file, "efg");
    m_gameModified = false;
    break;

  case GameSaveFormat::Nfg:
    BuildNfg();
    m_game->Write(file, "nfg");
    m_gameModified = false;
    break;
  }
  UpdateViews();
}

void GameDocument::DoSetTitle(const wxString &p_title, const wxString &p_comment)
{
  m_game->SetTitle(static_cast<const char *>(p_title.mb_str()));
  m_game->SetDescription(static_cast<const char *>(p_comment.mb_str()));
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoNewPlayer()
{
  const GamePlayer player =
      m_game->NewPlayer("Player " + lexical_cast<std::string>(m_game->NumPlayers() + 1));
  if (!m_game->IsTree()) {
    player->GetStrategy(1)->SetLabel("1");
  }
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetPlayerLabel(GamePlayer p_player, const wxString &p_label)
{
  p_player->SetLabel(p_label.ToStdString());
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoNewStrategy(GamePlayer p_player)
{
  m_game->NewStrategy(p_player, std::to_string(p_player->GetStrategies().size() + 1));
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoDeleteStrategy(GameStrategy p_strategy)
{
  m_game->DeleteStrategy(p_strategy);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetStrategyLabel(GameStrategy p_strategy, const wxString &p_label)
{
  p_strategy->SetLabel(p_label.ToStdString());
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoSetInfosetLabel(GameInfoset p_infoset, const wxString &p_label)
{
  p_infoset->SetLabel(p_label.ToStdString());
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoSetActionLabel(GameAction p_action, const wxString &p_label)
{
  p_action->SetLabel(p_label.ToStdString());
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoSetActionProbs(GameInfoset p_infoset, const Array<Number> &p_probs)
{
  m_game->SetChanceProbs(p_infoset, p_probs);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  m_game->SetInfoset(p_node, p_infoset);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoLeaveInfoset(GameNode p_node)
{
  m_game->LeaveInfoset(p_node);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoRevealAction(GameInfoset p_infoset, GamePlayer p_player)
{
  m_game->Reveal(p_infoset, p_player);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoInsertAction(GameNode p_node)
{
  if (!p_node || !p_node->GetInfoset()) {
    return;
  }
  const GameAction action = m_game->InsertAction(p_node->GetInfoset());
  action->SetLabel(std::to_string(action->GetNumber()));
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetNodeLabel(GameNode p_node, const wxString &p_label)
{
  p_node->SetLabel(p_label.ToStdString());
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoAppendMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->AppendMove(p_node, p_infoset);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoInsertMove(GameNode p_node, GamePlayer p_player, unsigned int p_actions)
{
  m_game->InsertMove(p_node, p_player, p_actions, true);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoInsertMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->InsertMove(p_node, p_infoset);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoCopyTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->CopyTree(p_destNode, p_srcNode);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoMoveTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->MoveTree(p_destNode, p_srcNode);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoDeleteParent(GameNode p_node)
{
  if (!p_node || !p_node->GetParent()) {
    return;
  }
  m_game->DeleteParent(p_node);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoDeleteTree(GameNode p_node)
{
  m_game->DeleteTree(p_node);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_infoset->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    m_game->SetPlayer(p_infoset, p_player);
    NotifyChanged(GameModificationType::GameForm);
  }
}

void GameDocument::DoSetPlayer(GameNode p_node, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_node->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    m_game->SetPlayer(p_node->GetInfoset(), p_player);
    NotifyChanged(GameModificationType::GameForm);
  }
}

void GameDocument::DoNewOutcome(GameNode p_node)
{
  m_game->SetOutcome(p_node, m_game->NewOutcome());
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoNewOutcome(const PureStrategyProfile &p_profile)
{
  p_profile->SetOutcome(m_game->NewOutcome());
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetOutcome(GameNode p_node, GameOutcome p_outcome)
{
  m_game->SetOutcome(p_node, p_outcome);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetOutcomeData(const GameNode &p_node, const wxString &p_label,
                                    const std::vector<wxString> &p_payoffs)
{
  if (!p_node) {
    return;
  }

  if (p_payoffs.size() != GetGame()->NumPlayers()) {
    throw std::invalid_argument("Incorrect number of payoff values");
  }

  std::vector<Rational> parsedPayoffs;
  parsedPayoffs.reserve(p_payoffs.size());

  for (const auto &value : p_payoffs) {
    parsedPayoffs.push_back(lexical_cast<Rational>(value.ToStdString()));
  }

  const std::string label = p_label.ToStdString();
  GameOutcome outcome = p_node->GetOutcome();

  bool changed = !outcome;

  if (outcome) {
    changed = outcome->GetLabel() != label;

    if (!changed) {
      for (size_t player = 1; player <= GetGame()->NumPlayers(); ++player) {
        if (outcome->GetPayoff<Rational>(GetGame()->GetPlayer(player)) !=
            parsedPayoffs[player - 1]) {
          changed = true;
          break;
        }
      }
    }
  }

  if (!changed) {
    return;
  }

  if (!outcome) {
    outcome = GetGame()->NewOutcome();
    GetGame()->SetOutcome(p_node, outcome);
  }

  outcome->SetLabel(label);

  for (size_t player = 1; player <= GetGame()->NumPlayers(); ++player) {
    outcome->SetPayoff(GetGame()->GetPlayer(player), Number(p_payoffs[player - 1].ToStdString()));
  }

  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoRemoveOutcome(GameNode p_node)
{
  if (!p_node || !p_node->GetOutcome()) {
    return;
  }
  m_game->SetOutcome(p_node, nullptr);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoCopyOutcome(GameNode p_node, GameOutcome p_outcome)
{
  const GameOutcome outcome = m_game->NewOutcome();
  outcome->SetLabel("Outcome" + lexical_cast<std::string>(outcome->GetNumber()));
  for (const auto &player : m_game->GetPlayers()) {
    outcome->SetPayoff(player, p_outcome->GetPayoff<Number>(player));
  }
  m_game->SetOutcome(p_node, outcome);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value)
{
  p_outcome->SetPayoff(m_game->GetPlayer(p_player), Number(p_value.ToStdString()));
  NotifyChanged(GameModificationType::GamePayoffs);
}

} // namespace Gambit::GUI
