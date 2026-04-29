#ifndef GAMBIT_WELCOME_H
#define GAMBIT_WELCOME_H

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/statbmp.h>
#include <wx/commandlinkbutton.h>

class wxButton;
class wxStaticText;
class wxBoxSizer;

enum class WelcomeNewProblemKind { NormalForm, ExtensiveForm };

wxDECLARE_EVENT(wxEVT_WELCOME_OPEN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_WELCOME_NEW, wxCommandEvent);

class WelcomePanel : public wxPanel {
public:
  explicit WelcomePanel(wxWindow *parent);

private:
  void CreateControls();
  void LayoutControls();

  void OnOpen(wxCommandEvent &p_event);
  void OnNewNormalForm(wxCommandEvent &p_event);
  void OnNewExtensiveForm(wxCommandEvent &p_event);

  void SendOpenEvent();
  void SendNewEvent(WelcomeNewProblemKind p_kind);

  wxStaticBitmap *m_logoBitmap{nullptr};
  wxStaticText *m_titleText{nullptr};

  wxCommandLinkButton *m_openButton{nullptr};
  wxCommandLinkButton *m_newNormalFormButton{nullptr};
  wxCommandLinkButton *m_newExtensiveFormButton{nullptr};
};

class WelcomeFrame : public wxFrame {
public:
  explicit WelcomeFrame(wxWindow *parent = nullptr);

private:
  void CreateControls();
  void LayoutControls();

  void OnWelcomeOpen(wxCommandEvent &p_event);
  void OnWelcomeNew(wxCommandEvent &p_event);
  void OnClose(wxCloseEvent &p_event);

  bool DoOpen();
  bool DoCreateNew(WelcomeNewProblemKind p_kind);

private:
  WelcomePanel *m_panel{nullptr};
};

#endif // GAMBIT_WELCOME_H
