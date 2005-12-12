//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of application-level class for Gambit graphical interface
//

#ifndef GAMBIT_H
#define GAMBIT_H

#include <wx/wx.h>
#include <wx/config.h>    // for wxConfig
#include <wx/docview.h>   // for wxFileHistory

extern void gbtExceptionDialog(const std::string &p_message,
			       wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


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
  gbtArray<gbtGameDocument *> m_documents;

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

