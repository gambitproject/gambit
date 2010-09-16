//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/gambit.h
// Declaration of application-level class for Gambit graphical interface
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

#include <wx/wx.h>
#include <wx/config.h>    // for wxConfig
#include <wx/docview.h>   // for wxFileHistory

class gbtGameDocument;

typedef enum {
  GBT_APP_FILE_OK = 0,
  GBT_APP_OPEN_FAILED = 1,
  GBT_APP_PARSE_FAILED = 2
} gbtAppLoadResult;

class gbtApplication : public wxApp {
private:
  wxString m_currentDir; /* Current position in directory tree. */
  wxFileHistory m_fileHistory;
  Gambit::Array<gbtGameDocument *> m_documents;

  bool OnInit(void);

public:
  gbtApplication(void);
  virtual ~gbtApplication();
  
  const wxString &GetCurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const wxString &p_dir)  { m_currentDir = p_dir; }

  wxString GetHistoryFile(int index) const
    { return m_fileHistory.GetHistoryFile(index); }
  void AddMenu(wxMenu *p_menu) 
    { m_fileHistory.UseMenu(p_menu); m_fileHistory.AddFilesToMenu(p_menu); }
  void RemoveMenu(wxMenu *p_menu)  { m_fileHistory.RemoveMenu(p_menu); }
  

  gbtAppLoadResult LoadFile(const wxString &);

  //!
  //! These manage the list of open documents
  //!
  //@{
  void AddDocument(gbtGameDocument *p_doc) 
    { m_documents.Append(p_doc); }
  void RemoveDocument(gbtGameDocument *p_doc)
    { m_documents.Remove(m_documents.Find(p_doc)); }
  bool AreDocumentsModified(void) const;
  //@}
};

DECLARE_APP(gbtApplication)

#endif // GAMBIT_H

