//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of document class
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include <fstream>
#include <sstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/filename.h>

#include "game-document.h"

static wxColour s_defaultColors[8] = {
  wxColour(255, 0, 0),
  wxColour(0, 0, 255),
  wxColour(0, 128, 0),
  wxColour(255, 128, 0),
  wxColour(0, 0, 64),
  wxColour(128, 0, 255),
  wxColour(64, 0, 0),
  wxColour(255, 128, 255)
};


gbtGameDocument::gbtGameDocument(const gbtGame &p_game)
  : m_game(p_game), m_modified(false), m_treeZoom(1.0)
{
  if (!m_game.IsNull()) {
    for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
      m_playerColors.Append(s_defaultColors[(pl - 1) % 8]);
    }
  }
}

gbtGameDocument::~gbtGameDocument()
{
  for (int i = 1; i <= m_undoFiles.Length(); i++) {
    wxRemoveFile(m_undoFiles[i]);
  }

  for (int i = 1; i <= m_redoFiles.Length(); i++) {
    wxRemoveFile(m_redoFiles[i]);
  }
}


void gbtGameDocument::AddView(gbtGameView *p_view)
{ 
  m_views.Append(p_view);
}

void gbtGameDocument::RemoveView(gbtGameView *p_view)
{ 
  m_views.Remove(m_views.Find(p_view)); 
  if (m_views.Length() == 0) delete this;
}

void gbtGameDocument::UpdateViews(void)
{
  for (int i = 1; i <= m_views.Length(); m_views[i++]->OnUpdate());
}

void gbtGameDocument::SaveUndo(const wxString &p_description)
{
  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  Save(tempfile);
  m_undoFiles.Append(tempfile);
  m_undoDescriptions.Append(p_description);

  for (int i = 1; i <= m_redoFiles.Last(); i++) {
    wxRemoveFile(m_redoFiles[i]);
  }
  m_redoFiles.Flush();
  m_redoDescriptions.Flush();
}

void gbtGameDocument::Undo(void)
{
  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  Save(tempfile);
  m_redoFiles.Append(tempfile);
  m_redoDescriptions.Append(m_undoDescriptions[m_undoDescriptions.Last()]);

  Load(m_undoFiles[m_undoFiles.Last()]);
  wxRemoveFile(m_undoFiles[m_undoFiles.Last()]);
  m_undoFiles.Remove(m_undoFiles.Last());
  m_undoDescriptions.Remove(m_undoDescriptions.Last());

  if (m_undoFiles.Length() == 0) {
    m_modified = false;
  }

  UpdateViews();
}

void gbtGameDocument::Redo(void)
{
  wxString tempfile = wxFileName::CreateTempFileName(wxT("gambit"));
  Save(tempfile);
  m_undoFiles.Append(tempfile);
  m_undoDescriptions.Append(m_redoDescriptions[m_redoDescriptions.Last()]);

  Load(m_redoFiles[m_redoFiles.Last()]);
  wxRemoveFile(m_redoFiles[m_redoFiles.Last()]);
  m_redoFiles.Remove(m_redoFiles.Last());
  m_redoDescriptions.Remove(m_redoDescriptions.Last());

  m_modified = true;
  UpdateViews();
}


//-----------------------------------------------------------------------
//         gbtGameDocument: Operations modifying the document
//-----------------------------------------------------------------------

gbtGameOutcome gbtGameDocument::NewOutcome(void) 
{ 
  SaveUndo(_("creating new outcome"));
  gbtGameOutcome r = m_game->NewOutcome(); 
  m_modified = true;
  UpdateViews(); 
  return r;
}

void gbtGameDocument::SetPayoff(gbtGameOutcome p_outcome,
				const gbtGamePlayer &p_player, 
				const gbtRational &p_value)
{ 
  SaveUndo(_("setting payoff"));
  p_outcome->SetPayoff(p_player, p_value); 
  m_modified = true;
  UpdateViews(); 
}


void gbtGameDocument::NewPlayer(void)
{
  SaveUndo(_("adding player"));
  m_playerColors.Append(s_defaultColors[m_game->NumPlayers() % 8]);
  gbtGamePlayer player = m_game->NewPlayer();
  player->SetLabel((const char *) wxString::Format(wxT("Player%d"),
						   m_game->NumPlayers()).mb_str());
  if (!m_game->HasTree()) {
    player->GetInfoset(1)->GetAction(1)->SetLabel("Strategy1");
  }
  else {
    m_game->Canonicalize();
  }

  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::InsertStrategy(int p_player, int p_where)
{
  SaveUndo(_("adding strategy"));
  gbtGamePlayer player = m_game->GetPlayer(p_player);
  gbtGameAction action = player->GetInfoset(1)->InsertAction(p_where);
  action->SetLabel((const char *) 
		   wxString::Format(wxT("Strategy%d"),
				    player->NumStrategies()).mb_str());
				    
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::SetStrategyLabel(gbtGameStrategy p_strategy,
				       const std::string &p_label)
{
  SaveUndo(_("setting strategy label"));
  p_strategy->SetLabel(p_label);
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::NewMove(gbtGameNode p_node, gbtGamePlayer p_player)
{
  SaveUndo(_("adding new move"));
  gbtGameInfoset infoset = p_player->NewInfoset(2);
  p_node->InsertMove(infoset);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::SetMove(gbtGameNode p_node, gbtGameInfoset p_infoset)
{
  SaveUndo(_("adding new move"));
  p_node->InsertMove(p_infoset);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::NewAction(gbtGameInfoset p_infoset)
{
  SaveUndo(_("adding new action"));
  p_infoset->InsertAction(p_infoset->NumActions() + 1);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::SetPlayer(gbtGameInfoset p_infoset,
				gbtGamePlayer p_player)
{
  SaveUndo(_("changing player at node"));
  p_infoset->SetPlayer(p_player);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::CopyTree(gbtGameNode p_src, gbtGameNode p_dest)
{
  SaveUndo(_("copying tree"));
  p_dest->CopyTree(p_src);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::MoveTree(gbtGameNode p_src, gbtGameNode p_dest)
{
  SaveUndo(_("moving tree"));
  p_dest->MoveTree(p_src);
  m_game->Canonicalize();
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::SetNodeLabel(gbtGameNode p_node,
				   const std::string &p_label)
{
  SaveUndo(_("setting node label"));
  p_node->SetLabel(p_label);
  m_modified = true;
  UpdateViews();
}

void gbtGameDocument::SetInfoset(gbtGameNode p_node,
				 gbtGameInfoset p_infoset)
{
  SaveUndo(_("changing information set"));
  p_node->LeaveInfoset();
  p_node->JoinInfoset(p_infoset);
  m_modified = true;
  UpdateViews();
}

wxColour gbtGameDocument::GetPlayerColor(int p_player) const
{
  if (p_player == 0) {
    return *wxLIGHT_GREY;
  }
  else {
    return m_playerColors[p_player];
  }
}

void gbtGameDocument::SetPlayerColor(int p_player, const wxColour &p_color)
{
  m_playerColors[p_player] = p_color;
  UpdateViews();
}


void gbtGameDocument::Load(const wxString &p_filename)
{
  std::ifstream file((const char *) p_filename.mb_str());

  while (!file.eof()) {
    std::string key, value;
    char c;
    
    while (!file.eof() && (c = file.get()) != '=') {
      key += c;
    }

    while (!file.eof() && (c = file.get()) != '\n') {
      value += c;
    }

    if (key == "efg") {
      std::istringstream iss(value);
      m_game = ReadEfg(iss);
      m_game->Canonicalize();
    }
    else if (key == "nfg") {
      std::istringstream iss(value);
      m_game = ReadNfg(iss);
    }
    else if (key == "playercolor") {
      int pl, r, g, b;
      sscanf(value.c_str(), "%d %d %d %d", &pl, &r, &g, &b);
      if (m_playerColors.Last() >= pl) {
	m_playerColors[pl] = wxColour(r, g, b);
      }
      else {
	m_playerColors.Append(wxColour(r, g, b));
      }
    }
  }
}

void gbtGameDocument::Save(const wxString &p_filename) const
{
  std::ostringstream oss;
  if (m_game->HasTree()) {
    m_game->WriteEfg(oss);
  }
  else {
    m_game->WriteNfg(oss);
  }
  std::string gamefile = oss.str();
  for (int i = 0; i < gamefile.length(); i++) {
    if (gamefile[i] == '\n')  gamefile[i] = ' ';
  }

  std::ofstream file((const char *) p_filename.mb_str());
  if (m_game->HasTree()) {
    file << "efg= " << gamefile << std::endl;
  }
  else {
    file << "nfg= " << gamefile << std::endl;
  }
  for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
    file << "playercolor= " << pl << " ";
    file << (int) m_playerColors[pl].Red() << " ";
    file << (int) m_playerColors[pl].Green() << " ";
    file << (int) m_playerColors[pl].Blue() << std::endl;
  }
}



gbtGameView::gbtGameView(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{ m_doc->AddView(this); }

gbtGameView::~gbtGameView()
{ m_doc->RemoveView(this); }
