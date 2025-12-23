//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#include <wx/wxprec.h>
#include <wx/filename.h> // used to create temp files for undo/redo

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
//                          class GameDocument
//=========================================================================

GameDocument::GameDocument(Game p_game)
  : m_game(p_game), m_selectNode(nullptr), m_modified(false), m_stratSupports(this, true),
    m_currentProfileList(0)
{
  wxGetApp().AddDocument(this);

  std::ostringstream s;
  SaveDocument(s);
}

GameDocument::~GameDocument() { wxGetApp().RemoveDocument(this); }

bool GameDocument::LoadDocument(const wxString &p_filename, bool p_saveUndo)
{
  TiXmlDocument doc((const char *)p_filename.mb_str());
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

  m_stratSupports.Reset();

  m_profiles.clear();

  for (TiXmlNode *analysis = game->FirstChild("analysis"); analysis;
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
        auto plist = std::make_shared<AnalysisProfileList<double>>(this, false);
        plist->Load(analysis);
        m_profiles.push_back(plist);
      }
      else {
        auto plist = std::make_shared<AnalysisProfileList<Rational>>(this, false);
        plist->Load(analysis);
        m_profiles.push_back(plist);
      }
    }
  }

  m_currentProfileList = m_profiles.size();

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

  if (p_saveUndo) {
    std::ostringstream s;
    SaveDocument(s);
  }

  return true;
}

void GameDocument::SaveDocument(std::ostream &p_file) const
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

  std::for_each(m_profiles.begin(), m_profiles.end(),
                [&p_file](std::shared_ptr<AnalysisOutput> a) { a->Save(p_file); });

  p_file << "</game>\n";

  p_file << "</gambit:document>\n";
}

void GameDocument::UpdateViews(GameModificationType p_modifications)
{
  if (p_modifications != GBT_DOC_MODIFIED_NONE) {
    m_modified = true;
    std::ostringstream s;
    SaveDocument(s);
  }

  if (p_modifications == GBT_DOC_MODIFIED_GAME || p_modifications == GBT_DOC_MODIFIED_PAYOFFS) {
    m_stratSupports.Reset();

    // Even though modifications only to payoffs doesn't make the
    // computed profiles invalid for the edited game, it does mean
    // that, in general, they won't be Nash.  For now, to avoid confusion,
    // we will wipe them out.
    m_profiles.clear();
    m_currentProfileList = 0;
  }

  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::OnUpdate));
}

void GameDocument::PostPendingChanges()
{
  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::PostPendingChanges));
}

void GameDocument::BuildNfg()
{
  if (m_game->IsTree()) {
    m_stratSupports.Reset();
    std::for_each(m_profiles.begin(), m_profiles.end(), std::mem_fn(&AnalysisOutput::BuildNfg));
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
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void GameDocument::SetCurrentProfile(int p_profile)
{
  m_profiles[m_currentProfileList]->SetCurrent(p_profile);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void GameDocument::AddProfileList(std::shared_ptr<AnalysisOutput> p_profs)
{
  m_profiles.push_back(p_profs);
  m_currentProfileList = m_profiles.size();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void GameDocument::SetProfileList(int p_index)
{
  m_currentProfileList = p_index;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void GameDocument::SetStrategyElimStrength(bool p_strict)
{
  m_stratSupports.SetStrict(p_strict);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool GameDocument::GetStrategyElimStrength() const { return m_stratSupports.GetStrict(); }

bool GameDocument::NextStrategyElimLevel()
{
  const bool ret = m_stratSupports.NextLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  return ret;
}

void GameDocument::PreviousStrategyElimLevel()
{
  m_stratSupports.PreviousLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void GameDocument::TopStrategyElimLevel()
{
  m_stratSupports.TopLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool GameDocument::CanStrategyElim() const { return m_stratSupports.CanEliminate(); }

int GameDocument::GetStrategyElimLevel() const { return m_stratSupports.GetLevel(); }

void GameDocument::SetSelectNode(GameNode p_node)
{
  m_selectNode = p_node;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

//======================================================================
// Commands for model part of MVC architecture start here.
//======================================================================

void GameDocument::DoSave(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  SaveDocument(file);
  m_filename = p_filename;
  SetModified(false);
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void GameDocument::DoExportEfg(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  m_game->Write(file, "efg");
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void GameDocument::DoExportNfg(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  BuildNfg();
  m_game->Write(file, "nfg");
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void GameDocument::DoSetTitle(const wxString &p_title, const wxString &p_comment)
{
  m_game->SetTitle(static_cast<const char *>(p_title.mb_str()));
  m_game->SetComment(static_cast<const char *>(p_comment.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoNewPlayer()
{
  const GamePlayer player = m_game->NewPlayer();
  player->SetLabel("Player " + lexical_cast<std::string>(player->GetNumber()));
  if (!m_game->IsTree()) {
    player->GetStrategy(1)->SetLabel("1");
  }
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoSetPlayerLabel(GamePlayer p_player, const wxString &p_label)
{
  p_player->SetLabel(p_label.ToStdString());
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoNewStrategy(GamePlayer p_player)
{
  m_game->NewStrategy(p_player, std::to_string(p_player->GetStrategies().size() + 1));
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoDeleteStrategy(GameStrategy p_strategy)
{
  m_game->DeleteStrategy(p_strategy);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoSetStrategyLabel(GameStrategy p_strategy, const wxString &p_label)
{
  p_strategy->SetLabel(p_label.ToStdString());
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoSetInfosetLabel(GameInfoset p_infoset, const wxString &p_label)
{
  p_infoset->SetLabel(p_label.ToStdString());
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoSetActionLabel(GameAction p_action, const wxString &p_label)
{
  p_action->SetLabel(p_label.ToStdString());
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoSetActionProbs(GameInfoset p_infoset, const Array<Number> &p_probs)
{
  m_game->SetChanceProbs(p_infoset, p_probs);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoSetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  m_game->SetInfoset(p_node, p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoLeaveInfoset(GameNode p_node)
{
  m_game->LeaveInfoset(p_node);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoRevealAction(GameInfoset p_infoset, GamePlayer p_player)
{
  m_game->Reveal(p_infoset, p_player);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoInsertAction(GameNode p_node)
{
  if (!p_node || !p_node->GetInfoset()) {
    return;
  }
  const GameAction action = m_game->InsertAction(p_node->GetInfoset());
  action->SetLabel(std::to_string(action->GetNumber()));
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoSetNodeLabel(GameNode p_node, const wxString &p_label)
{
  p_node->SetLabel(p_label.ToStdString());
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void GameDocument::DoAppendMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->AppendMove(p_node, p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoInsertMove(GameNode p_node, GamePlayer p_player, unsigned int p_actions)
{
  m_game->InsertMove(p_node, p_player, p_actions, true);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoInsertMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->InsertMove(p_node, p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoCopyTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->CopyTree(p_destNode, p_srcNode);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoMoveTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->MoveTree(p_destNode, p_srcNode);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoDeleteParent(GameNode p_node)
{
  if (!p_node || !p_node->GetParent()) {
    return;
  }
  m_game->DeleteParent(p_node);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoDeleteTree(GameNode p_node)
{
  m_game->DeleteTree(p_node);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void GameDocument::DoSetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_infoset->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    m_game->SetPlayer(p_infoset, p_player);
    UpdateViews(GBT_DOC_MODIFIED_GAME);
  }
}

void GameDocument::DoSetPlayer(GameNode p_node, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_node->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    m_game->SetPlayer(p_node->GetInfoset(), p_player);
    UpdateViews(GBT_DOC_MODIFIED_GAME);
  }
}

void GameDocument::DoNewOutcome(GameNode p_node)
{
  m_game->SetOutcome(p_node, m_game->NewOutcome());
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoNewOutcome(const PureStrategyProfile &p_profile)
{
  p_profile->SetOutcome(m_game->NewOutcome());
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoSetOutcome(GameNode p_node, GameOutcome p_outcome)
{
  m_game->SetOutcome(p_node, p_outcome);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoRemoveOutcome(GameNode p_node)
{
  if (!p_node || !p_node->GetOutcome()) {
    return;
  }
  m_game->SetOutcome(p_node, nullptr);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoCopyOutcome(GameNode p_node, GameOutcome p_outcome)
{
  const GameOutcome outcome = m_game->NewOutcome();
  outcome->SetLabel("Outcome" + lexical_cast<std::string>(outcome->GetNumber()));
  for (const auto &player : m_game->GetPlayers()) {
    outcome->SetPayoff(player, p_outcome->GetPayoff<Number>(player));
  }
  m_game->SetOutcome(p_node, outcome);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value)
{
  p_outcome->SetPayoff(m_game->GetPlayer(p_player), Number(p_value.ToStdString()));
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void GameDocument::DoAddOutput(AnalysisOutput &p_list, const wxString &p_output)
{
  p_list.AddOutput(p_output);
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

} // namespace Gambit::GUI
