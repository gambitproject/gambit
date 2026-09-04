#ifndef GAMBIT_WELCOME_H
#define GAMBIT_WELCOME_H

#include <vector>

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/statbmp.h>
#include <wx/commandlinkbutton.h>

class wxButton;
class wxStaticText;
class wxStaticLine;
class wxBoxSizer;

enum class WelcomeNewProblemKind { NormalForm, ExtensiveForm };

wxDECLARE_EVENT(wxEVT_WELCOME_OPEN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_WELCOME_NEW, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_WELCOME_OPEN_RECENT, wxCommandEvent);

class WelcomePanel : public wxPanel {
public:
  explicit WelcomePanel(wxWindow *parent);

  bool RefreshRecentFiles();

private:
  void CreateControls();
  void LayoutControls();

  void OnOpen(wxCommandEvent &p_event);
  void OnNewNormalForm(wxCommandEvent &p_event);
  void OnNewExtensiveForm(wxCommandEvent &p_event);
  void OnOpenRecent(wxCommandEvent &p_event);

  void SendOpenEvent();
  void SendNewEvent(WelcomeNewProblemKind p_kind);
  void SendOpenRecentEvent(const wxString &p_filename);

  wxStaticBitmap *m_logoBitmap{nullptr};
  wxStaticText *m_titleText{nullptr};

  wxCommandLinkButton *m_openButton{nullptr};
  wxCommandLinkButton *m_newNormalFormButton{nullptr};
  wxCommandLinkButton *m_newExtensiveFormButton{nullptr};

  wxStaticLine *m_recentLine{nullptr};
  wxStaticText *m_recentHeading{nullptr};
  wxPanel *m_recentPanel{nullptr};
  std::vector<wxString> m_recentFiles;
};

class WelcomeFrame : public wxFrame {
public:
  explicit WelcomeFrame(wxWindow *parent = nullptr);

private:
  void CreateControls();
  void LayoutControls();
  void FitToContents();

  void OnWelcomeOpen(wxCommandEvent &p_event);
  void OnWelcomeNew(wxCommandEvent &p_event);
  void OnWelcomeOpenRecent(wxCommandEvent &p_event);
  void OnClose(wxCloseEvent &p_event);

  bool DoOpen();
  bool DoOpenRecent(const wxString &p_filename);
  bool DoCreateNew(WelcomeNewProblemKind p_kind);

private:
  WelcomePanel *m_panel{nullptr};
};

#endif // GAMBIT_WELCOME_H
