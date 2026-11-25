//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dialogstyle.h
// Shared layout and presentation conventions for dialogs
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

#ifndef GAMBIT_GUI_DIALOGSTYLE_H
#define GAMBIT_GUI_DIALOGSTYLE_H

#include <string>

#include <wx/artprov.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace Gambit::GUI {

/// @name Spacing, in device-independent pixels
///@{
/// Between a dialog's edge and its content, and between groups of controls
constexpr int c_spaceMargin = 12;
/// Between related controls, such as a label and the control it names
constexpr int c_spaceRelated = 8;
/// Between the parts of a compound control, such as a row of small buttons
constexpr int c_spaceTight = 4;
/// Between an alert's edge and its content
constexpr int c_spaceAlertMargin = 20;
///@}

/// Width, in device-independent pixels, of the small symbol buttons (up, down, delete, add)
/// beside each row of an editable list
constexpr int c_rowButtonWidth = 28;

/// @brief Lays out a dialog: its content inside the standard margin, the standard button
///        row beneath, sized to fit and centred on its parent
/// @param p_dialog The dialog to lay out
/// @param p_content The dialog's content, without any outer border
/// @param p_buttons Standard button flags (wxOK, wxCANCEL, ...), or 0 for no button row
/// @param p_extra Further buttons, placed at the start of the button row; may be null
void SetDialogLayout(wxDialog *p_dialog, wxSizer *p_content, long p_buttons,
                     wxSizer *p_extra = nullptr);

/// @brief Recomputes a laid-out dialog's minimum size after its content has changed,
///        enlarging the dialog if it is now smaller than that minimum
void RefreshDialogMinSize(wxDialog *p_dialog);

/// @brief Creates a two-column grid for label-and-control rows, with the standard gaps and
///        the control column growable
wxFlexGridSizer *NewFormSizer(const wxWindow *p_parent);

/// @brief Creates the text that reports an input problem inline, beneath the controls
wxStaticText *NewErrorText(wxWindow *p_parent);

/// @brief The colour of text reporting a problem
wxColour ErrorTextColour();

/// @brief The background that marks a control whose current value is invalid
wxColour InvalidFieldBackground();
/// @brief The colour of a newly added item's label in a list editor, adjusted for dark mode
wxColour NewLabelColour();
/// @brief The colour of a renamed item's label in a list editor, adjusted for dark mode
wxColour RenamedLabelColour();

/// @brief Reports that an operation could not be carried out, for a reason the user can act on
/// @param p_parent The parent window
/// @param p_title The window title, naming the operation
/// @param p_headline A one-line statement of what went wrong
/// @param p_detail Supporting explanation
void ShowOperationError(wxWindow *p_parent, const wxString &p_title, const wxString &p_headline,
                        const wxString &p_detail);

/// @brief Warns about an operation and asks whether to go ahead with it
/// @param p_parent The parent window
/// @param p_title The window title, naming the operation
/// @param p_headline A one-line statement of the concern
/// @param p_detail Supporting explanation
/// @param p_proceedLabel The label of the button that goes ahead
/// @return True if the user chose to go ahead
bool ConfirmOperation(wxWindow *p_parent, const wxString &p_title, const wxString &p_headline,
                      const wxString &p_detail, const wxString &p_proceedLabel);

/// @brief Reports an unexpected internal error, after which the operation was abandoned
void ShowInternalError(wxWindow *p_parent, const std::string &p_message);

/// @brief A dialog showing an icon beside a bold headline, with detail text beneath and
///        a row of standard buttons
class AlertDialog : public wxDialog {
public:
  /// @param p_parent The parent window
  /// @param p_title The window title
  /// @param p_icon The art identifier of the icon (wxART_ERROR, wxART_WARNING, ...)
  /// @param p_headline A one-line summary
  /// @param p_detail Supporting explanation; paragraphs are separated by blank lines
  /// @param p_buttons Standard button flags (wxOK, wxCANCEL, ...)
  AlertDialog(wxWindow *p_parent, const wxString &p_title, const wxArtID &p_icon,
              const wxString &p_headline, const wxString &p_detail, long p_buttons);
  ~AlertDialog() override = default;

  /// @brief Replaces the label of one of the standard buttons
  void SetButtonLabel(wxWindowID p_id, const wxString &p_label);
  /// @brief Makes one of the standard buttons the default, and gives it the focus
  void SetDefaultButton(wxWindowID p_id);
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DIALOGSTYLE_H
