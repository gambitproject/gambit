//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of application-level class for Gambit graphical interface
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef GAMBIT_H
#define GAMBIT_H

#include "base/base.h"
#include "wx/wx.h"
#include "wx/config.h"    // for wxConfig
#include "wx/docview.h"   // for wxFileHistory
#include "wx/listctrl.h"

#include "gamedoc.h"

extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


class GambitApp : public wxApp {
private:
  wxString m_currentDir; /* Current position in directory tree. */
  wxFileHistory m_fileHistory;

  bool OnInit(void);

public:
  GambitApp(void);
  virtual ~GambitApp();
  
  const wxString &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const wxString &p_dir)  { m_currentDir = p_dir; }

  wxFileHistory &GetFileHistory(void) { return m_fileHistory; }

  void LoadFile(const wxString &);

  // Handlers for common menu items
  void OnFileNew(wxWindow *);
  void OnFileOpen(wxWindow *);
  void OnFileMRUFile(wxCommandEvent &);
  void OnFileImportComLab(wxWindow *);

  void OnHelpContents(void);
  void OnHelpIndex(void);
  void OnHelpAbout(wxWindow *);
};

DECLARE_APP(GambitApp)

const int wxID_HELP_INDEX = 1310;

#endif // GAMBIT_H

