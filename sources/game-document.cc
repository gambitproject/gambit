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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "game-document.h"

gbtGameDocument::gbtGameDocument(const gbtGame &p_game)
  : m_game(p_game), m_modified(false)
{
  m_playerColor[0] = wxColour(255, 0, 0);
  m_playerColor[1] = wxColour(0, 0, 255);
  m_playerColor[2] = wxColour(0, 128, 0);
  m_playerColor[3] = wxColour(255, 128, 0);
  m_playerColor[4] = wxColour(0, 0, 64);
  m_playerColor[5] = wxColour(128, 0, 255);
  m_playerColor[6] = wxColour(64, 0, 0);
  m_playerColor[7] = wxColour(255, 128, 255);
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

//-----------------------------------------------------------------------
//         gbtGameDocument: Operations modifying the document
//-----------------------------------------------------------------------

gbtGameOutcome gbtGameDocument::NewOutcome(void) 
{ 
  gbtGameOutcome r = m_game->NewOutcome(); 
  m_modified = true;
  UpdateViews(); 
  return r;
}

void gbtGameDocument::SetPayoff(gbtGameOutcome p_outcome,
				const gbtGamePlayer &p_player, 
				const gbtRational &p_value)
{ 
  p_outcome->SetPayoff(p_player, p_value); 
  m_modified = true;
  UpdateViews(); 
}


wxColour gbtGameDocument::GetPlayerColor(int p_player) const
{
  return m_playerColor[(p_player - 1) % 8];
}

void gbtGameDocument::SetPlayerColor(int p_player, const wxColour &p_color)
{
  m_playerColor[(p_player - 1) % 8] = p_color;
  UpdateViews();
}


gbtGameView::gbtGameView(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{ m_doc->AddView(this); }

gbtGameView::~gbtGameView()
{ m_doc->RemoveView(this); }
