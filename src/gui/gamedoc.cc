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

#include <algorithm>
#include <sstream>
#include <fstream>
#include <utility>

#include <wx/file.h>
#include <wx/log.h>

#include "games.h"
#include "games/gametree.h"
#include "games/gameagg.h"
#include "games/gamebagg.h"
#include "games/workspace.h"

#include "app.h" // for wxGetApp()
#include "gamedoc.h"

namespace Gambit::GUI {

namespace {

/// Replaces the contents of the file p_filename with p_text. If writing fails, any existing file
/// is left as it was.
void ReplaceFile(const wxString &p_filename, const std::string &p_text)
{
  // A failure is reported by the exception, not by wxWidgets' log window.
  const wxLogNull noLog;
  wxTempFile file;
  if (!file.Open(p_filename) || !file.Write(p_text.data(), p_text.size()) || !file.Commit()) {
    throw std::runtime_error("Check that the folder exists and that you can write to it.");
  }
}

} // namespace

//=========================================================================
//                          class GameDocument
//=========================================================================

GameDocument::GameDocument(Game p_game) : m_gameWorkspace(std::move(p_game))
{
  // Observing first, the document's catalog identity is current before any view is notified.
  m_gameWorkspace.AddObserver(this);
  wxGetApp().AddDocument(this);
}

GameDocument::~GameDocument()
{
  m_gameWorkspace.RemoveObserver(this);
  wxGetApp().RemoveDocument(this);
}

void GameDocument::AddView(GameView *p_view)
{
  m_views.push_back(p_view);
  m_gameWorkspace.AddObserver(p_view);
}

void GameDocument::RemoveView(GameView *p_view)
{
  m_gameWorkspace.RemoveObserver(p_view);
  m_views.erase(std::ranges::find(m_views, p_view));
}

GameDocument::LoadOutcome GameDocument::Load(const wxString &p_filename)
{
  std::ifstream input(p_filename.mb_str());
  if (!input) {
    return {.result = LoadResult::OpenFailed, .document = nullptr};
  }

  // First, see whether this is a Gambit workspace (.gbt) file.
  try {
    const WorkspaceFile workspace = ReadWorkspace(input);
    std::istringstream game_text(workspace.game);
    const auto doc = std::make_shared<GameDocument>(ReadGame(game_text));
    doc->m_gameWorkspace.Load(workspace);
    doc->SetFilename(p_filename);
    return {.result = LoadResult::Success, .document = doc};
  }
  // NOLINTNEXTLINE(bugprone-empty-catch)
  catch (const std::exception &) {
    // Not a recognized (or not fully valid) workspace file; fall through to
    // try it as a bare game file instead.
  }

  // Not a (valid) workspace -- try reading it as a bare .efg/.nfg file.
  input.clear();
  input.seekg(0);
  try {
    const Game game = ReadGame(input);
    if (As<GameAGGRep>(game) || As<GameBAGGRep>(game)) {
      return {.result = LoadResult::UnsupportedRepresentation, .document = nullptr};
    }
    const auto doc = std::make_shared<GameDocument>(game);
    doc->SetFilename(p_filename);
    return {.result = LoadResult::Success, .document = doc};
  }
  catch (const InvalidFileException &) {
    return {.result = LoadResult::ParseFailed, .document = nullptr};
  }
}

std::string GameDocument::WriteGame() const
{
  std::ostringstream text;
  GetGame()->Write(text, As<GameTreeRep>(GetGame()) ? "efg" : "nfg");
  return text.str();
}

void GameDocument::SetCatalogOrigin(const wxString &p_slug, const wxString &p_version)
{
  m_filename.clear();
  m_catalogOrigin = CatalogOrigin{.slug = p_slug,
                                  .version = p_version,
                                  .game = WriteGame(),
                                  .title = GetGame()->GetTitle(),
                                  .description = GetGame()->GetDescription()};
  m_isCatalogGame = true;
  m_gameWorkspace.NotifyChanged(GameModificationType::None);
}

void GameDocument::OnWorkspaceChanged(GameModificationType)
{
  if (!m_catalogOrigin) {
    return;
  }
  const bool isCatalogGame = WriteGame() == m_catalogOrigin->game;
  if (m_isCatalogGame && !isCatalogGame) {
    // An edited catalog game is no longer the game its catalog title and description describe.
    // This joins the edit's undoable step: undo restores the snapshot taken before the edit.
    const Game game = GetGame();
    if (game->GetTitle() == m_catalogOrigin->title) {
      game->SetTitle("");
    }
    if (game->GetDescription() == m_catalogOrigin->description) {
      game->SetDescription("");
    }
  }
  m_isCatalogGame = isCatalogGame;
}

void GameDocument::PostPendingChanges()
{
  std::ranges::for_each(m_views, std::mem_fn(&GameView::PostPendingChanges));
}

void GameDocument::DoSave(const wxString &p_filename, GameSaveFormat p_format)
{
  std::ostringstream text;
  switch (p_format) {
  case GameSaveFormat::Workspace:
    m_gameWorkspace.Save(text);
    break;
  case GameSaveFormat::Efg:
    GetGame()->Write(text, "efg");
    break;
  case GameSaveFormat::Nfg:
    BuildNfg();
    GetGame()->Write(text, "nfg");
    break;
  }
  ReplaceFile(p_filename, text.str());

  // The document is now the saved file, whatever its format; a game file holds no results or
  // drawing settings, so those stay unsaved.
  m_filename = p_filename;
  m_catalogOrigin.reset();
  m_isCatalogGame = false;
  m_gameWorkspace.MarkSaved(p_format == GameSaveFormat::Workspace);
  m_gameWorkspace.NotifyChanged(GameModificationType::None);
}

} // namespace Gambit::GUI
