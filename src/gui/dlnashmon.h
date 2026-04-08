//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlnashmon.h
// Dialog for monitoring Nash equilibrium computation progress
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

#ifndef GAMBIT_GUI_DLNASHMON_H
#define GAMBIT_GUI_DLNASHMON_H

#include "gamedoc.h"

namespace Gambit::GUI {

class NashMonitorDialog final : public wxDialog {
  GameDocument *m_doc;
  std::shared_ptr<class ExternalProcessRunner> m_runner;
  wxWindow *m_profileList;
  wxStaticText *m_statusText, *m_countText;
  wxButton *m_stopButton, *m_okButton;
  std::shared_ptr<AnalysisOutput> m_output;

  void Start(const std::shared_ptr<AnalysisOutput> &p_command);

  void OnStop(wxCommandEvent &);
  void OnRunnerLine(wxThreadEvent &);
  void OnRunnerFinished(wxThreadEvent &);

public:
  NashMonitorDialog(wxWindow *p_parent, GameDocument *p_doc,
                    const std::shared_ptr<AnalysisOutput> &p_command);
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLNASHMON_H
