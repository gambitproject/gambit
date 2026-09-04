//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/app.cc
// Implementation of main wxApp class
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
#endif // WX_PRECOMP
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/image.h>
#include <wx/intl.h>

#include "games.h"

#include "app.h"
#include "gameframe.h"
#include "welcome.h"

#include "bitmaps/gambitbig.xpm"

namespace Gambit::GUI {

static wxBitmap MakeScaledSplashBitmap(const wxBitmap &srcBmp, double fracOfShortSide)
{
  const wxPoint mouse = wxGetMousePosition();
  const int dispIdx = wxDisplay::GetFromPoint(mouse);
  wxDisplay disp(dispIdx == wxNOT_FOUND ? 0 : dispIdx);

  wxRect geom = disp.GetGeometry(); // pixels in that display
  const int shortSide = std::min(geom.width, geom.height);
  const int targetMax = std::max(200, int(shortSide * fracOfShortSide));

  const int w = srcBmp.GetWidth();
  const int h = srcBmp.GetHeight();
  const double s = double(targetMax) / double(std::max(w, h));

  const int newW = std::max(1, int(std::lround(w * s)));
  const int newH = std::max(1, int(std::lround(h * s)));

  wxImage img = srcBmp.ConvertToImage();
  img.Rescale(newW, newH, wxIMAGE_QUALITY_HIGH);

  return wxBitmap(img);
}

wxBEGIN_EVENT_TABLE(Application, wxApp) EVT_TIMER(wxID_ANY, Application::OnSplashDismissTimer)
    wxEND_EVENT_TABLE()

        bool Application::OnInit()
{
  wxConfigBase::Set(new wxConfig(_T("Gambit"), _T("Gambit")));
  m_fileHistory.Load(*wxConfigBase::Get());
  // Immediately saving this back forces the entries to be created at
  // the "top level" of the config file when using the wxFileConfig
  // implementation (which seems to still be buggy).
  // m_fileHistory.Save(config);
  wxConfigBase::Get()->Read(_T("/General/CurrentDirectory"), &m_currentDir, _T(""));

  // Apply the persisted language preference (if any), so that every dialog/window
  // constructed from here on is localized.
  wxString languagePref;
  wxConfigBase::Get()->Read(_T("/General/Language"), &languagePref, _T("System"));
  SetLanguage(languagePref);

  // Process command line arguments, if any.
  for (int i = 1; i < argc; i++) {
    LoadFile(argv[i], nullptr);
  }

  if (m_documents.empty()) {
    auto *frame = new WelcomeFrame(nullptr);
    frame->Show(true);
    SetTopWindow(frame);
  }
  else {
    const wxBitmap bitmap(gambitbig_xpm);
    m_splashTimer.Start();
    m_splash = new wxSplashScreen(MakeScaledSplashBitmap(bitmap, 0.45),
                                  wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, nullptr,
                                  wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_splash->Show();
    m_splash->Update();
    this->CallAfter(&Application::DismissSplash);
  }
  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

void Application::DismissSplash()
{
  if (!m_splash) {
    return;
  }

  const long minDisplay = 1000;
  const long elapsed = m_splashTimer.Time();

  if (elapsed < minDisplay) {
    m_splashDismissTimer.SetOwner(this);
    m_splashDismissTimer.StartOnce(minDisplay - elapsed);
    return;
  }

  m_splash->Destroy();
  m_splash = nullptr;
}

bool Application::SetLanguage(const wxString &p_language)
{
  std::string code = p_language.ToStdString();
  if (code == "System" || code.empty()) {
    code = "en";
  }

  wxLanguage lang = wxLANGUAGE_ENGLISH;
  if (code == "en") {
    lang = wxLANGUAGE_ENGLISH;
  }
  else if (code == "es") {
    lang = wxLANGUAGE_SPANISH;
  }
  else if (code == "fr") {
    lang = wxLANGUAGE_FRENCH;
  }
  else {
    lang = wxLANGUAGE_UNKNOWN;
    return false;
  }

  // Recreate the locale on every call: wxLocale::Init() may only be invoked once
  // per object, so a runtime language change (via the Preferences dialog) must build
  // a fresh locale rather than re-initializing the existing one.  The new locale is
  // only committed once it has been initialized successfully.
  auto *newLocale = new wxLocale;

#ifndef LOCALEDIR
#define LOCALEDIR "share/locale"
#endif
  newLocale->AddCatalogLookupPathPrefix(LOCALEDIR);
  if (!newLocale->Init(lang)) {
    delete newLocale;
    return false;
  }
  newLocale->AddCatalog("gambit");

  delete m_locale;
  m_locale = newLocale;
  return true;
}

Application::~Application()
{
  delete m_locale;
  m_locale = nullptr;
}

namespace {

/// A simple single-button error notice, styled like GameFrame's CloseWarningDialog:
/// an icon beside a bold headline, with supporting detail underneath.
class FileErrorDialog final : public wxDialog {
public:
  FileErrorDialog(wxWindow *parent, const wxString &title, const wxString &headline,
                  const wxString &detail)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
  {
    auto *topSizer = new wxBoxSizer(wxVERTICAL);

    auto *contentSizer = new wxBoxSizer(wxHORIZONTAL);
    contentSizer->Add(new wxStaticBitmap(this, wxID_ANY,
                                         wxArtProvider::GetBitmap(wxART_ERROR, wxART_MESSAGE_BOX,
                                                                  wxSize(32, 32))),
                      0, wxALIGN_TOP | wxRIGHT, FromDIP(16));

    auto *textSizer = new wxBoxSizer(wxVERTICAL);

    auto *headlineText = new wxStaticText(this, wxID_ANY, headline);
    auto font = headlineText->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    headlineText->SetFont(font);
    headlineText->Wrap(FromDIP(400));

    auto *detailText = new wxStaticText(this, wxID_ANY, detail);
    detailText->Wrap(FromDIP(400));

    textSizer->Add(headlineText, 0, wxBOTTOM, FromDIP(8));
    textSizer->Add(detailText, 0);

    contentSizer->Add(textSizer, 1, wxEXPAND);

    topSizer->Add(contentSizer, 1, wxEXPAND | wxALL, FromDIP(20));

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *okButton = new wxButton(this, wxID_OK, _("OK"));
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(okButton, 0);

    topSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(20));

    SetSizerAndFit(topSizer);
    SetMinSize(wxSize(FromDIP(420), -1));

    SetEscapeId(wxID_OK);
    SetAffirmativeId(wxID_OK);
    okButton->SetDefault();
    okButton->SetFocus();

    CentreOnParent();
  }
};

} // namespace

AppLoadResult Application::LoadFile(const wxString &p_filename, wxWindow *p_parent)
{
  const auto [result, doc] = GameDocument::Load(p_filename);

  switch (result) {
  case GameDocument::LoadResult::OpenFailed:
    FileErrorDialog(p_parent, _("Unable to open file"),
                    _("Gambit could not open this file for reading."), p_filename)
        .ShowModal();
    return AppLoadResult::OpenFailed;

  case GameDocument::LoadResult::ParseFailed:
    FileErrorDialog(p_parent, _("Unable to read file"),
                    _("File is not in a format Gambit recognizes."), p_filename)
        .ShowModal();
    return AppLoadResult::ParseFailed;

  case GameDocument::LoadResult::UnsupportedRepresentation:
    FileErrorDialog(p_parent, _("Unsupported game representation"),
                    _("Action graph games are not currently supported by the "
                      "graphical interface."),
                    p_filename)
        .ShowModal();
    return AppLoadResult::UnsupportedRepresentation;

  case GameDocument::LoadResult::Success:
    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    (void)new GameFrame(nullptr, doc);
    return AppLoadResult::Success;
  }
}

void Application::SetCurrentDir(const wxString &p_dir)
{
  m_currentDir = p_dir;
  wxConfigBase::Get()->Write(_T("/General/CurrentDirectory"), p_dir);
}

} // namespace Gambit::GUI

IMPLEMENT_APP(Gambit::GUI::Application)
