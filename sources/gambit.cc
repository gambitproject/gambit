//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main application class
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
#include <wx/config.h>
#include <wx/image.h>     // for wxInitAllImageHandlers

#include "gambit.h"
#include "game-frame.h"

gbtApplication::gbtApplication(void)
{ }

bool gbtApplication::OnInit(void)
{
  wxInitAllImageHandlers();

  m_fileHistory = new wxFileHistory;
  wxConfig config(wxT("GambitDev"));
  m_fileHistory->Load(config);

  gbtArray<int> dim(2);  dim[1] = dim[2] = 2;
  gbtGame nfg = NewNfg(dim);
  nfg->SetLabel("Untitled normal form game");
  nfg->GetPlayer(1)->SetLabel("Player1");
  nfg->GetPlayer(1)->GetStrategy(1)->SetLabel("Strategy1");
  nfg->GetPlayer(1)->GetStrategy(2)->SetLabel("Strategy2");
  nfg->GetPlayer(2)->SetLabel("Player2");
  nfg->GetPlayer(2)->GetStrategy(1)->SetLabel("Strategy1");
  nfg->GetPlayer(2)->GetStrategy(2)->SetLabel("Strategy2");
  (void) new gbtGameFrame(0, new gbtGameDocument(nfg));
  return true;
}

int gbtApplication::OnExit(void)
{
  wxConfig config(wxT("GambitDev"));
  m_fileHistory->Save(config);
  delete m_fileHistory;
  return 0;
}

gbtApplication::~gbtApplication()
{ }

IMPLEMENT_APP(gbtApplication)
