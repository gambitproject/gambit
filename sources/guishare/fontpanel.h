//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to generic panel for selecting fonts
//

#ifndef FONTPANEL_H
#define FONTPANEL_H

class FontPanel : public wxPanel {
private:
  wxButton *m_fontButton;

  // Event handlers
  void OnFontButton(wxCommandEvent &);

public:
  FontPanel(wxWindow *p_parent, const wxFont &p_font);

  wxFont GetFont(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // FONTPANEL_H
