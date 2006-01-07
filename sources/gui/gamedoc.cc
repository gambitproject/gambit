//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of game document class
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/filename.h>       // used to create temp files for undo/redo

#include "libgambit/libgambit.h"
#include "tinyxml.h"    // for XML parser for LoadDocument()

#include "gambit.h"     // for wxGetApp()
#include "gamedoc.h"

//=========================================================================
//                       class gbtBehavDominanceStack
//=========================================================================

gbtBehavDominanceStack::gbtBehavDominanceStack(gbtGameDocument *p_doc,
					       bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
{
  Reset();
}

gbtBehavDominanceStack::~gbtBehavDominanceStack()
{
  for (int i = 1; i <= m_supports.Length(); delete m_supports[i++]);
}

void gbtBehavDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict)  Reset();
  m_strict = p_strict;
}

void gbtBehavDominanceStack::Reset(void)
{
  for (int i = 1; i <= m_supports.Length(); delete m_supports[i++]);
  m_supports = Gambit::Array<Gambit::BehavSupport *>();
  if (m_doc->IsTree()) {
    m_supports.Append(new Gambit::BehavSupport(m_doc->GetGame()));
    m_current = 1;
  }
}

bool gbtBehavDominanceStack::NextLevel(void)
{
  if (m_current < m_supports.Length()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  Gambit::Array<int> players;
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    players.Append(pl);
  }
  
  std::ostringstream gnull;
  Gambit::BehavSupport newSupport = 
    m_supports[m_current]->Undominated(m_strict, true, players, gnull);

  if (newSupport != *m_supports[m_current]) {
    m_supports.Append(new Gambit::BehavSupport(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool gbtBehavDominanceStack::PreviousLevel(void)
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

gbtStrategyDominanceStack::gbtStrategyDominanceStack(gbtGameDocument *p_doc,
						     bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
{
  Reset();
}

gbtStrategyDominanceStack::~gbtStrategyDominanceStack()
{
  for (int i = 1; i <= m_supports.Length(); delete m_supports[i++]);
}

void gbtStrategyDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict)  Reset();
  m_strict = p_strict;
}

void gbtStrategyDominanceStack::Reset(void)
{
  for (int i = 1; i <= m_supports.Length(); delete m_supports[i++]);
  m_supports = Gambit::Array<Gambit::StrategySupport *>();
  m_supports.Append(new Gambit::StrategySupport(m_doc->GetGame()));
  m_current = 1;
  m_noFurther = false;
}

bool gbtStrategyDominanceStack::NextLevel(void)
{
  if (m_current < m_supports.Length()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  Gambit::Array<int> players;
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    players.Append(pl);
  }

  std::ostringstream gnull;
  Gambit::StrategySupport newSupport = 
    m_supports[m_current]->Undominated(m_strict, players, gnull);

  if (newSupport != *m_supports[m_current]) {
    m_supports.Append(new Gambit::StrategySupport(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool gbtStrategyDominanceStack::PreviousLevel(void)
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
  : m_game(p_game),
    m_selectNode(0), m_modified(false),
    m_behavSupports(this, true), m_stratSupports(this, true),
    m_currentProfileList(0)
{
  m_game->Canonicalize();
  m_game->BuildComputedValues();
  wxGetApp().AddDocument(this);

  std::ostringstream s;
  SaveDocument(s);
  m_undoList.Append(s.str());
}

gbtGameDocument::~gbtGameDocument()
{
  wxGetApp().RemoveDocument(this);
}

bool gbtGameDocument::LoadDocument(const wxString &p_filename,
				   bool p_saveUndo)
{
  TiXmlDocument doc((const char *) p_filename.mb_str());
  if (!doc.LoadFile()) {
    // Some error occurred.  Do something smart later.
    return false;
  }

  TiXmlNode *game = doc.FirstChild("game");
  if (!game) {
    // There ought to be at least one game child.  If not... umm...
    return false;
  }

  TiXmlNode *efgfile = game->FirstChild("efgfile");
  if (efgfile) {
    std::istringstream s(efgfile->FirstChild()->Value());
    m_game = Gambit::ReadGame(s);

  }
  
  TiXmlNode *nfgfile = game->FirstChild("nfgfile");
  if (nfgfile) {
    std::istringstream s(nfgfile->FirstChild()->Value());
    m_game = Gambit::ReadGame(s);
  }

  if (!efgfile && !nfgfile) {
    // No game representation... punt!
    return false;
  }
  
  m_behavSupports.Reset();
  m_stratSupports.Reset();

  m_profiles = Gambit::List<gbtAnalysisProfileList>();

  for (TiXmlNode *analysis = game->FirstChild("analysis");
       analysis; analysis = analysis->NextSibling()) {
    const char *type = analysis->ToElement()->Attribute("type");
    if (type && !strcmp(type, "list")) {
      // Read in a list of profiles
      gbtAnalysisProfileList plist(this);
      plist.Load(analysis);
      m_profiles.Append(plist);
    }
  }

  m_currentProfileList = m_profiles.Length();

  TiXmlNode *colors = doc.FirstChild("colors");
  if (colors)  m_style.SetColorXML(colors);
  TiXmlNode *font = doc.FirstChild("font");
  if (font)    m_style.SetFontXML(font);
  TiXmlNode *layout = doc.FirstChild("autolayout");
  if (layout)  m_style.SetLayoutXML(layout);
  TiXmlNode *labels = doc.FirstChild("labels");
  if (labels)  m_style.SetLabelXML(labels);
  TiXmlNode *numbers = doc.FirstChild("numbers");
  if (numbers) {
    int numDecimals = 4;
    numbers->ToElement()->QueryIntAttribute("decimals", &numDecimals);
    m_style.SetNumDecimals(numDecimals);
  }

  if (p_saveUndo) {
    std::ostringstream s;
    SaveDocument(s);
    m_undoList.Append(s.str());
  }

  return true;
}

void gbtGameDocument::SaveDocument(std::ostream &p_file) const
{
  p_file << "<?xml version=\"1.0\" standalone=no>\n";

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
    m_game->WriteEfgFile(p_file);
    p_file << "</efgfile>\n";

    for (int i = 1; i <= m_profiles.Length(); i++) {
      p_file << "<analysis type=\"list\">\n";

      p_file << "<description>\n";
      p_file << (const char *) m_profiles[i].GetDescription().mb_str() << "\n";
      p_file << "</description>\n";

      for (int j = 1; j <= m_profiles[i].NumProfiles(); j++) {
	const Gambit::MixedBehavProfile<double> &behav = m_profiles[i].GetBehav(j);
	p_file << "<profile type=\"behav\">\n";
	for (int k = 1; k <= behav.Length(); k++) {
	  p_file << behav[k];
	  if (k < behav.Length()) {
	    p_file << ",";
	  }
	  else {
	    p_file << "\n";
	  }
	}
	p_file << "</profile>\n";
      }
      p_file << "</analysis>\n";
    }
  }
  else {
    p_file << "<nfgfile>\n";
    m_game->WriteNfgFile(p_file);
    p_file << "</nfgfile>\n";

    for (int i = 1; i <= m_profiles.Length(); i++) {
      p_file << "<analysis type=\"list\">\n";

      p_file << "<description>\n";
      p_file << (const char *) m_profiles[i].GetDescription().mb_str() << "\n";
      p_file << "</description>\n";

      for (int j = 1; j <= m_profiles[i].NumProfiles(); j++) {
	const Gambit::MixedStrategyProfile<double> &mixed = m_profiles[i].GetMixed(j);
	p_file << "<profile type=\"mixed\">\n";
	for (int k = 1; k <= mixed.Length(); k++) {
	  p_file << mixed[k];
	  if (k < mixed.Length()) {
	    p_file << ",";
	  }
	  else {
	    p_file << "\n";
	  }
	}
	p_file << "</profile>\n";
      }
      p_file << "</analysis>\n";
    }
  }

  p_file << "</game>\n";

}

void gbtGameDocument::UpdateViews(gbtGameModificationType p_modifications)
{
  if (p_modifications != GBT_DOC_MODIFIED_NONE) {
    m_modified = true;
    m_game->Canonicalize();
    m_game->BuildComputedValues();
    m_redoList = Gambit::List<std::string>();

    std::ostringstream s;
    SaveDocument(s);
    m_undoList.Append(s.str());
  }

  if (p_modifications == GBT_DOC_MODIFIED_GAME ||
      p_modifications == GBT_DOC_MODIFIED_PAYOFFS) {
    m_behavSupports.Reset();
    m_stratSupports.Reset();

    // Even though modifications only to payoffs doesn't make the
    // computed profiles invalid for the edited game, it does mean
    // that, in general, they won't be Nash.  For now, to avoid confusion,
    // we will wipe them out.
    m_profiles = Gambit::List<gbtAnalysisProfileList>();
    m_currentProfileList = 0;
  }

  for (int i = 1; i <= m_views.Length(); m_views[i++]->OnUpdate());
}

void gbtGameDocument::PostPendingChanges(void)
{
  for (int i = 1; i <= m_views.Length(); m_views[i++]->PostPendingChanges());
}

void gbtGameDocument::BuildNfg(void)
{ 
  if (m_game->IsTree()) {
    m_game->BuildComputedValues();
    m_stratSupports.Reset();
    for (int i = 1; i <= m_profiles.Length(); m_profiles[i++].BuildNfg());
  }
}

//
// A word about the undo and redo features:
// We store a list of the textual representation of games.  We don't
// store other aspects of the state (e.g., profiles) as yet.
// The "undo" list includes the representation of the current state
// of the game (hence, CanUndo() only returns true when the list has
// more than one element.
//
void gbtGameDocument::Undo(void)
{
  // The current game is at the end of the undo list; move it to the redo list
  m_redoList.Append(m_undoList[m_undoList.Length()]);
  m_undoList.Remove(m_undoList.Length());

  m_game = 0;

  m_profiles = Gambit::List<gbtAnalysisProfileList>();
  m_currentProfileList = 0;

  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  std::ofstream f((const char *) tempfile.mb_str());
  f << m_undoList[m_undoList.Length()] << std::endl;
  f.close();

  LoadDocument(tempfile, false);
  wxRemoveFile(tempfile);

  for (int i = 1; i <= m_views.Length(); m_views[i++]->OnUpdate());
}

void gbtGameDocument::Redo(void)
{
  m_undoList.Append(m_redoList[m_redoList.Length()]);
  m_redoList.Remove(m_redoList.Length());

  m_game = 0;

  m_profiles = Gambit::List<gbtAnalysisProfileList>();
  m_currentProfileList = 0;

  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  std::ofstream f((const char *) tempfile.mb_str());
  f << m_undoList[m_undoList.Length()] << std::endl;
  f.close();

  LoadDocument(tempfile, false);
  wxRemoveFile(tempfile);

  for (int i = 1; i <= m_views.Length(); m_views[i++]->OnUpdate());
}


void gbtGameDocument::SetCurrentProfile(int p_profile)
{
  m_profiles[m_currentProfileList].SetCurrent(p_profile);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfileList(void)
{
  m_profiles.Append(gbtAnalysisProfileList(this));
  m_currentProfileList = m_profiles.Length();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfileList(const gbtAnalysisProfileList &p_profs)
{
  m_profiles.Append(p_profs);
  m_currentProfileList = m_profiles.Length();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::SetProfileList(int p_index)
{
  m_currentProfileList = p_index;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::AddProfiles(const Gambit::List<Gambit::MixedBehavProfile<double> > &p_profiles)
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

void gbtGameDocument::AddProfiles(const Gambit::List<Gambit::MixedStrategyProfile<double> > &p_profiles)
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

void gbtGameDocument::SetBehavElimStrength(bool p_strict)
{
  m_behavSupports.SetStrict(p_strict);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::NextBehavElimLevel(void)
{
  bool ret = m_behavSupports.NextLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  return ret;
}

void gbtGameDocument::PreviousBehavElimLevel(void)
{
  m_behavSupports.PreviousLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::TopBehavElimLevel(void)
{
  m_behavSupports.TopLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::CanBehavElim(void) const
{
  return m_behavSupports.CanEliminate();
}

int gbtGameDocument::GetBehavElimLevel(void) const
{
  return m_behavSupports.GetLevel();
}


void gbtGameDocument::SetStrategyElimStrength(bool p_strict)
{
  m_stratSupports.SetStrict(p_strict);
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::GetStrategyElimStrength(void) const
{
  return m_stratSupports.GetStrict();
}

bool gbtGameDocument::NextStrategyElimLevel(void)
{
  bool ret = m_stratSupports.NextLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  return ret;
}

void gbtGameDocument::PreviousStrategyElimLevel(void)
{
  m_stratSupports.PreviousLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

void gbtGameDocument::TopStrategyElimLevel(void)
{
  m_stratSupports.TopLevel();
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

bool gbtGameDocument::CanStrategyElim(void) const
{
  return m_stratSupports.CanEliminate();
}

int gbtGameDocument::GetStrategyElimLevel(void) const
{
  return m_stratSupports.GetLevel();
}


void gbtGameDocument::SetSelectNode(Gambit::GameNode p_node)
{
  m_selectNode = p_node;
  UpdateViews(GBT_DOC_MODIFIED_VIEWS);
}

std::string gbtGameDocument::GetRealizProb(const Gambit::GameNode &p_node) const
{
  if (GetCurrentProfile() == 0 || !p_node) {
    return "";
  }
  return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetRealizProb(p_node),
		m_style.NumDecimals());
}

std::string gbtGameDocument::GetBeliefProb(const Gambit::GameNode &p_node) const
{
  if (GetCurrentProfile() == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }

  if (m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetIsetProb(p_node->GetInfoset()) > Gambit::Rational(0)) {
    return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetBeliefProb(p_node),
		  m_style.NumDecimals());
  }
  else {
    // We don't compute assessments yet!
    return "*";
  }
}

std::string gbtGameDocument::GetNodeValue(const Gambit::GameNode &p_node,
					  int p_player) const
{
  if (GetCurrentProfile() == 0 || !p_node) {
    return "";
  }

  return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetNodeValue(p_node)[p_player], 
		m_style.NumDecimals());
}

std::string gbtGameDocument::GetInfosetProb(const Gambit::GameNode &p_node) const
{
  if (GetCurrentProfile() == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }
  return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetIsetProb(p_node->GetInfoset()),
		m_style.NumDecimals());
}

std::string gbtGameDocument::GetInfosetValue(const Gambit::GameNode &p_node) const
{
  if (GetCurrentProfile() == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }
  if (m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetIsetProb(p_node->GetInfoset()) > Gambit::Rational(0)) {
    return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetIsetValue(p_node->GetInfoset()),
		  m_style.NumDecimals());
  }
  else {
    // In the absence of beliefs, this is not well-defined in general
    return "*";
  }
}

std::string gbtGameDocument::GetActionProb(const Gambit::GameNode &p_node, int p_act) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    Gambit::GameInfoset infoset = p_node->GetInfoset();
    return infoset->GetActionProb<std::string>(p_act);
  }

  if (GetCurrentProfile() == 0 || !p_node->GetPlayer()) {
    return "";
  }
  
  const Gambit::MixedBehavProfile<double> &profile = m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile());

  if (!profile.IsDefinedAt(p_node->GetInfoset())) {
    return "*";
  }
  
  return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetActionProb(p_node->GetInfoset()->GetAction(p_act)),
		m_style.NumDecimals());
}

std::string gbtGameDocument::GetActionValue(const Gambit::GameNode &p_node, int p_act) const
{
  if (GetCurrentProfile() == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }

  if (m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetIsetProb(p_node->GetInfoset()) > Gambit::Rational(0)) {
    return Gambit::ToText(m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile()).GetActionValue(p_node->GetInfoset()->GetAction(p_act)),
		  m_style.NumDecimals());
  }
  else  {
    // In the absence of beliefs, this is not well-defined
    return "*";
  }
}

double gbtGameDocument::ActionProb(const Gambit::GameNode &p_node, int p_action) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return p_node->GetInfoset()->GetActionProb<double>(p_action);
  }

  if (GetCurrentProfile() && p_node->GetInfoset()) {
    return m_profiles[m_currentProfileList].GetBehav(GetCurrentProfile())(p_node->GetInfoset()->GetPlayer()->GetNumber(), p_node->GetInfoset()->GetNumber(), p_action);
  }
  return -1;
}

