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

#include "game-document.h"

gbtGameDocument::gbtGameDocument(const gbtGame &p_game)
  : m_game(p_game)
{ }


void gbtGameDocument::AddView(gbtGameView *p_view)
{ m_views.Append(p_view); }

void gbtGameDocument::RemoveView(gbtGameView *p_view)
{ 
  m_views.Remove(m_views.Find(p_view)); 
  if (m_views.Length() == 0) delete this;
}

void gbtGameDocument::UpdateViews(void)
{
  for (int i = 1; i <= m_views.Length(); m_views[i++]->OnUpdate());
}


gbtGameView::gbtGameView(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{ m_doc->AddView(this); }

gbtGameView::~gbtGameView()
{ m_doc->RemoveView(this); }
