//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of a panel to select fonts
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/fontdlg.h"

#include "fontpanel.h"

const int idFONT_BUTTON = 2000;

//=========================================================================
//                    class FontPanel implementation
//=========================================================================

BEGIN_EVENT_TABLE(FontPanel, wxPanel)
  EVT_BUTTON(idFONT_BUTTON, FontPanel::OnFontButton)
END_EVENT_TABLE()

FontPanel::FontPanel(wxWindow *p_parent, const wxFont &p_font)
  : wxPanel(p_parent, -1)
{
  m_fontButton = new wxButton(this, idFONT_BUTTON, "Change...");
  m_fontButton->SetFont(p_font);
}

wxFont FontPanel::GetFont(void) const
{
  return m_fontButton->GetFont();
}

void FontPanel::OnFontButton(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_fontButton->GetFont());

  wxFontDialog dialog(this, &data);
  if (dialog.ShowModal() == wxID_OK) {
    m_fontButton->SetFont(dialog.GetFontData().GetChosenFont());
  }
}
