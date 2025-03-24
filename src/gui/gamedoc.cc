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

//=========================================================================
//                       class gbtBehavDominanceStack
//=========================================================================

gbtBehavDominanceStack::gbtBehavDominanceStack(gbtGameDocument *p_doc, bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
{
  Reset();
}

gbtBehavDominanceStack::~gbtBehavDominanceStack()
{
  for (int i = 1; i <= m_supports.size(); delete m_supports[i++])
    ;
}

void gbtBehavDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict) {
    Reset();
  }
  m_strict = p_strict;
}

void gbtBehavDominanceStack::Reset()
{
  for (int i = 1; i <= m_supports.size(); delete m_supports[i++])
    ;
  m_supports = Gambit::Array<Gambit::BehaviorSupportProfile *>();
  if (m_doc->IsTree()) {
    m_supports.push_back(new Gambit::BehaviorSupportProfile(m_doc->GetGame()));
    m_current = 1;
  }
  m_noFurther = false;
}

bool gbtBehavDominanceStack::NextLevel()
{
  if (m_current < m_supports.size()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  Gambit::BehaviorSupportProfile newSupport = m_supports[m_current]->Undominated(m_strict);

  if (newSupport != *m_supports[m_current]) {
    m_supports.push_back(new Gambit::BehaviorSupportProfile(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool gbtBehavDominanceStack::PreviousLevel()
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
//                   class gbtStrategyDominanceStack
//=========================================================================

gbtStrategyDominanceStack::gbtStrategyDominanceStack(gbtGameDocument *p_doc, bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
{
  Reset();
}

gbtStrategyDominanceStack::~gbtStrategyDominanceStack()
{
  for (int i = 1; i <= m_supports.size(); delete m_supports[i++])
    ;
}

void gbtStrategyDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict) {
    Reset();
  }
  m_strict = p_strict;
}

void gbtStrategyDominanceStack::Reset()
{
  for (int i = 1; i <= m_supports.size(); delete m_supports[i++])
    ;
  m_supports = Gambit::Array<Gambit::StrategySupportProfile *>();
  m_supports.push_back(new Gambit::StrategySupportProfile(m_doc->GetGame()));
  m_current = 1;
  m_noFurther = false;
}

bool gbtStrategyDominanceStack::NextLevel()
{
  if (m_current < m_supports.size()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  Gambit::StrategySupportProfile newSupport = m_supports[m_current]->Undominated(m_strict);

  if (newSupport != *m_supports[m_current]) {
    m_supports.push_back(new Gambit::StrategySupportProfile(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool gbtStrategyDominanceStack::PreviousLevel()
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
//                          class gbtGameDocument
//=========================================================================

gbtGameDocument::gbtGameDocument(Gambit::Game p_game)
  : m_game(p_game), m_selectNode(nullptr), m_modified(false), m_behavSupports(this, true),
    m_stratSupports(this, true), m_currentProfileList(0)
{
  wxGetApp().AddDocument(this);

  std::ostringstream s;
  SaveDocument(s);
  m_undoList.push_back(s.str());
}

gbtGameDocument::~gbtGameDocument() { wxGetApp().RemoveDocument(this); }

bool gbtGameDocument::LoadDocument(const wxString &p_filename, bool p_saveUndo)
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
      m_game = Gambit::ReadGame(s);
    }
    catch (...) {
      return false;
    }
  }

  TiXmlNode *nfgfile = game->FirstChild("nfgfile");
  if (nfgfile) {
    try {
      std::istringstream s(nfgfile->FirstChild()->Value());
      m_game = Gambit::ReadGame(s);
    }
    catch (...) {
      return false;
    }
  }

  if (!efgfile && !nfgfile) {
    // No game representation... punt!
    return false;
  }

  m_behavSupports.Reset();
  m_stratSupports.Reset();

  m_profiles = Gambit::Array<gbtAnalysisOutput *>();

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
        auto *plist = new gbtAnalysisProfileList<double>(this, false);
        plist->Load(analysis);
        m_profiles.push_back(plist);
      }
      else {
        auto *plist = new gbtAnalysisProfileList<Rational>(this, false);
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
    m_undoList.push_back(s.str());
  }

  return true;
}

void gbtGameDocument::SaveDocument(std::ostream &p_file) const
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

  for (int i = 1; i <= m_profiles.size(); i++) {
    m_profiles[i]->Save(p_file);
  }

  p_file << "</game>\n";

  p_file << "</gambit:document>\n";
}

void gbtGameDocument::UpdateViews(gbtGameModificationType p_modifications)
{
  if (p_modifications != GBT_DOC_MODIFIED_NONE) {
    m_modified = true;
    m_redoList = std::list<std::string>();

    std::ostringstream s;
    SaveDocument(s);
    m_undoList.push_back(s.str());
  }

  if (p_modifications == GBT_DOC_MODIFIED_GAME || p_modifications == GBT_DOC_MODIFIED_PAYOFFS) {
    m_behavSupports.Reset();
    m_stratSupports.Reset();

    // Even though modifications only to payoffs doesn't make the
    // computed profiles invalid for the edited game, it does mean
    // that, in general, they won't be Nash.  For now, to avoid confusion,
    // we will wipe them out.
    while (!m_profiles.empty()) {
      delete m_profiles.back();
      m_profiles.pop_back();
    }
    m_currentProfileList = 0;
  }

  for (int i = 1; i <= m_views.size(); m_views[i++]->OnUpdate())
    ;
}

void gbtGameDocument::PostPendingChanges()
{
  for (int i = 1; i <= m_views.size(); m_views[i++]->PostPendingChanges())
    ;
}

void gbtGameDocument::BuildNfg()
{
  if (m_game->IsTree()) {
    m_stratSupports.Reset();
    for (int i = 1; i <= m_profiles.size(); m_profiles[i++]->BuildNfg())
      ;
  }
}

GameAction gbtGameDocument::GetAction(int p_index) const
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
  throw IndexException();
}

void gbtGameDocument::SetStyle(const gbtStyle &p_style)
{
  m_style = p_style;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

//
// A word about the undo and redo features:
// We store a list of the textual representation of games.  We don't
// store other aspects of the state (e.g., profiles) as yet.
// The "undo" list includes the representation of the current state
// of the game (hence, CanUndo() only returns true when the list has
// more than one element.
//
void gbtGameDocument::Undo()
{
  // The current game is at the end of the undo list; move it to the redo list
  m_redoList.push_back(m_undoList.back());
  m_undoList.pop_back();

  m_game = nullptr;

  while (!m_profiles.empty()) {
    delete m_profiles.back();
    m_profiles.pop_back();
  }
  m_currentProfileList = 0;

  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  std::ofstream f((const char *)tempfile.mb_str());
  f << m_undoList.back() << std::endl;
  f.close();

  LoadDocument(tempfile, false);
  wxRemoveFile(tempfile);

  for (int i = 1; i <= m_views.size(); m_views[i++]->OnUpdate())
    ;
}

void gbtGameDocument::Redo()
{
  m_undoList.push_back(m_redoList.back());
  m_redoList.pop_back();

  m_game = nullptr;

  while (!m_profiles.empty()) {
    delete m_profiles.back();
    m_profiles.pop_back();
  }
  m_currentProfileList = 0;

  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  std::ofstream f((const char *)tempfile.mb_str());
  f << m_undoList.back() << std::endl;
  f.close();

  LoadDocument(tempfile, false);
  wxRemoveFile(tempfile);

  for (int i = 1; i <= m_views.size(); m_views[i++]->OnUpdate())
    ;
}

void gbtGameDocument::SetCurrentProfile(int p_profile)
{
  m_profiles[m_currentProfileList]->SetCurrent(p_profile);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfileList(gbtAnalysisOutput *p_profs)
{
  m_profiles.push_back(p_profs);
  m_currentProfileList = m_profiles.size();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::SetProfileList(int p_index)
{
  m_currentProfileList = p_index;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

/*
void gbtGameDocument::AddProfiles(const Gambit::List<Gambit::MixedBehavProfile<double> >
&p_profiles)
{
  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_profiles[m_currentProfileList].Append(p_profiles[i]);
  }

  m_profiles[m_currentProfileList].SetCurrent(m_profiles[m_currentProfileList].NumProfiles());
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfile(const Gambit::MixedBehavProfile<double> &p_profile)
{
  m_profiles[m_currentProfileList].Append(p_profile);
  m_profiles[m_currentProfileList].SetCurrent(m_profiles[m_currentProfileList].NumProfiles());
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfiles(const Gambit::List<Gambit::MixedStrategyProfile<double> >
&p_profiles)
{
  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_profiles[m_currentProfileList].Append(p_profiles[i]);
  }

  m_profiles[m_currentProfileList].SetCurrent(m_profiles[m_currentProfileList].NumProfiles());
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfile(const Gambit::MixedStrategyProfile<double> &p_profile)
{
  m_profiles[m_currentProfileList].Append(p_profile);
  m_profiles[m_currentProfileList].SetCurrent(m_profiles[m_currentProfileList].NumProfiles());
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}
*/

void gbtGameDocument::SetBehavElimStrength(bool p_strict)
{
  m_behavSupports.SetStrict(p_strict);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::NextBehavElimLevel()
{
  bool ret = m_behavSupports.NextLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  return ret;
}

void gbtGameDocument::PreviousBehavElimLevel()
{
  m_behavSupports.PreviousLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::TopBehavElimLevel()
{
  m_behavSupports.TopLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::CanBehavElim() const { return m_behavSupports.CanEliminate(); }

int gbtGameDocument::GetBehavElimLevel() const { return m_behavSupports.GetLevel(); }

void gbtGameDocument::SetStrategyElimStrength(bool p_strict)
{
  m_stratSupports.SetStrict(p_strict);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::GetStrategyElimStrength() const { return m_stratSupports.GetStrict(); }

bool gbtGameDocument::NextStrategyElimLevel()
{
  bool ret = m_stratSupports.NextLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  return ret;
}

void gbtGameDocument::PreviousStrategyElimLevel()
{
  m_stratSupports.PreviousLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::TopStrategyElimLevel()
{
  m_stratSupports.TopLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::CanStrategyElim() const { return m_stratSupports.CanEliminate(); }

int gbtGameDocument::GetStrategyElimLevel() const { return m_stratSupports.GetLevel(); }

void gbtGameDocument::SetSelectNode(Gambit::GameNode p_node)
{
  m_selectNode = p_node;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

//======================================================================
// Commands for model part of MVC architecture start here.
//======================================================================

void gbtGameDocument::DoSave(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  SaveDocument(file);
  m_filename = p_filename;
  SetModified(false);
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void gbtGameDocument::DoExportEfg(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  m_game->Write(file, "efg");
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void gbtGameDocument::DoExportNfg(const wxString &p_filename)
{
  std::ofstream file(static_cast<const char *>(p_filename.mb_str()));
  BuildNfg();
  m_game->Write(file, "nfg");
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void gbtGameDocument::DoSetTitle(const wxString &p_title, const wxString &p_comment)
{
  m_game->SetTitle(static_cast<const char *>(p_title.mb_str()));
  m_game->SetComment(static_cast<const char *>(p_comment.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoNewPlayer()
{
  GamePlayer player = m_game->NewPlayer();
  player->SetLabel("Player " + lexical_cast<std::string>(player->GetNumber()));
  if (!m_game->IsTree()) {
    player->GetStrategy(1)->SetLabel("1");
  }
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoSetPlayerLabel(GamePlayer p_player, const wxString &p_label)
{
  p_player->SetLabel(static_cast<const char *>(p_label.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoNewStrategy(GamePlayer p_player)
{
  GameStrategy strategy = p_player->NewStrategy();
  strategy->SetLabel(lexical_cast<std::string>(strategy->GetNumber()));
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoDeleteStrategy(GameStrategy p_strategy)
{
  p_strategy->DeleteStrategy();
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoSetStrategyLabel(GameStrategy p_strategy, const wxString &p_label)
{
  p_strategy->SetLabel(static_cast<const char *>(p_label.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoSetInfosetLabel(GameInfoset p_infoset, const wxString &p_label)
{
  p_infoset->SetLabel(static_cast<const char *>(p_label.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoSetActionLabel(GameAction p_action, const wxString &p_label)
{
  p_action->SetLabel(static_cast<const char *>(p_label.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoSetActionProbs(GameInfoset p_infoset, const Array<Number> &p_probs)
{
  m_game->SetChanceProbs(p_infoset, p_probs);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoSetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  p_node->SetInfoset(p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoLeaveInfoset(GameNode p_node)
{
  p_node->LeaveInfoset();
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoRevealAction(GameInfoset p_infoset, GamePlayer p_player)
{
  p_infoset->Reveal(p_player);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoInsertAction(GameNode p_node)
{
  if (!p_node || !p_node->GetInfoset()) {
    return;
  }
  GameAction action = p_node->GetInfoset()->InsertAction();
  action->SetLabel(lexical_cast<std::string>(action->GetNumber()));
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoSetNodeLabel(GameNode p_node, const wxString &p_label)
{
  p_node->SetLabel(static_cast<const char *>(p_label.mb_str()));
  UpdateViews(GBT_DOC_MODIFIED_LABELS);
}

void gbtGameDocument::DoAppendMove(GameNode p_node, GameInfoset p_infoset)
{
  p_node->AppendMove(p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoInsertMove(GameNode p_node, GamePlayer p_player, unsigned int p_actions)
{
  GameInfoset infoset = p_node->InsertMove(p_player, p_actions);
  for (int act = 1; act <= infoset->NumActions(); act++) {
    infoset->GetAction(act)->SetLabel(lexical_cast<std::string>(act));
  }
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoInsertMove(GameNode p_node, GameInfoset p_infoset)
{
  p_node->InsertMove(p_infoset);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoCopyTree(GameNode p_destNode, GameNode p_srcNode)
{
  p_destNode->CopyTree(p_srcNode);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoMoveTree(GameNode p_destNode, GameNode p_srcNode)
{
  p_destNode->MoveTree(p_srcNode);
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoDeleteParent(GameNode p_node)
{
  if (!p_node || !p_node->GetParent()) {
    return;
  }
  p_node->DeleteParent();
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoDeleteTree(GameNode p_node)
{
  p_node->DeleteTree();
  UpdateViews(GBT_DOC_MODIFIED_GAME);
}

void gbtGameDocument::DoSetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_infoset->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    p_infoset->SetPlayer(p_player);
    UpdateViews(GBT_DOC_MODIFIED_GAME);
  }
}

void gbtGameDocument::DoSetPlayer(GameNode p_node, GamePlayer p_player)
{
  if (!p_player->IsChance() && !p_node->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    p_node->GetInfoset()->SetPlayer(p_player);
    UpdateViews(GBT_DOC_MODIFIED_GAME);
  }
}

void gbtGameDocument::DoNewOutcome(GameNode p_node)
{
  p_node->SetOutcome(m_game->NewOutcome());
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoNewOutcome(const PureStrategyProfile &p_profile)
{
  p_profile->SetOutcome(m_game->NewOutcome());
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoSetOutcome(GameNode p_node, GameOutcome p_outcome)
{
  p_node->SetOutcome(p_outcome);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoRemoveOutcome(GameNode p_node)
{
  if (!p_node || !p_node->GetOutcome()) {
    return;
  }
  p_node->SetOutcome(nullptr);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoCopyOutcome(GameNode p_node, GameOutcome p_outcome)
{
  GameOutcome outcome = m_game->NewOutcome();
  outcome->SetLabel("Outcome" + lexical_cast<std::string>(outcome->GetNumber()));
  for (const auto &player : m_game->GetPlayers()) {
    outcome->SetPayoff(player, p_outcome->GetPayoff<Number>(player));
  }
  p_node->SetOutcome(outcome);
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value)
{
  p_outcome->SetPayoff(m_game->GetPlayer(p_player), Number(p_value.ToStdString()));
  UpdateViews(GBT_DOC_MODIFIED_PAYOFFS);
}

void gbtGameDocument::DoAddOutput(gbtAnalysisOutput &p_list, const wxString &p_output)
{
  p_list.AddOutput(p_output);
  UpdateViews(GBT_DOC_MODIFIED_NONE);
}
