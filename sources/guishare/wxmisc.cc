//
// FILE: wxmisc.cc -- a few general purpose functions that rely on and enhance
//                    wxwin.
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/config.h"  // for config

#include "math/math.h"

#ifdef __BORLANDC__
#pragma hdr_stop
#endif  // __BORLANDC__

#include "base/gmisc.h"
#include "system.h"
#include "wxmisc.h"

//========================================================================
//                     guiAutoDialog: Member functions
//========================================================================

BEGIN_EVENT_TABLE(guiAutoDialog, wxDialog)
  EVT_BUTTON(wxID_HELP, guiAutoDialog::OnHelp)
END_EVENT_TABLE()

guiAutoDialog::guiAutoDialog(wxWindow *p_parent, char *p_title)
  : wxDialog(p_parent, -1, p_title, wxDefaultPosition, wxDefaultSize)
{
  m_okButton = new wxButton(this, wxID_OK, "OK");
  m_okButton->SetDefault();
  m_cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  m_helpButton = new wxButton(this, wxID_HELP, "Help");

  m_buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_buttonSizer->Add(m_okButton, 0, wxALL, 5);
  m_buttonSizer->Add(m_cancelButton, 0, wxALL, 5);
  m_buttonSizer->Add(m_helpButton, 0, wxALL, 5);
}

guiAutoDialog::~guiAutoDialog()
{ }

void guiAutoDialog::OnHelp(void)
{
  //  wxGetApp().HelpController().DisplaySection(HelpString());
}

//========================================================================
//                     guiPagedDialog: Member functions
//========================================================================

guiPagedDialog::guiPagedDialog(wxWindow *p_parent, char *p_title,
			       int p_numValues)
  : guiAutoDialog(p_parent, p_title)
{
  m_grid = new wxGrid(this, -1, wxPoint(10, 50), wxSize(200, 300));
  m_grid->CreateGrid(p_numValues, 1);
  m_grid->SetLabelSize(wxHORIZONTAL, 0);
  m_grid->SetEditable(true);
  m_grid->SetEditInPlace(true);
  m_grid->AdjustScrollbars();
}

guiPagedDialog::~guiPagedDialog()
{ }

gText guiPagedDialog::GetValue(int p_index) const
{
  return m_grid->GetCellValue(p_index - 1, 0).c_str();
}

void guiPagedDialog::SetValue(int p_index, const gText &p_value)
{
  m_grid->SetCellValue((char *) p_value, p_index - 1, 0);
}

void guiPagedDialog::SetLabel(int p_index, const gText &p_label)
{
  m_grid->SetLabelValue(wxVERTICAL, (char *) p_label, p_index - 1);
}

//========================================================================
//                    guiSliderDialog: Member functions
//========================================================================

guiSliderDialog::guiSliderDialog(wxWindow *p_parent, const gText &p_caption,
				 int p_min, int p_max, int p_default)
  : guiAutoDialog(p_parent, p_caption)
{
  wxBoxSizer *sliderSizer = new wxBoxSizer(wxHORIZONTAL);
  sliderSizer->Add(new wxStaticText(this, -1, (char *) p_caption),
		   0, wxCENTER | wxALL, 5);
  m_slider = new wxSlider(this, -1, p_default, p_min, p_max,
			  wxDefaultPosition, wxDefaultSize,
			  wxSL_HORIZONTAL | wxSL_LABELS);
  sliderSizer->Add(m_slider, 1, wxEXPAND | wxALL, 5);

  m_helpButton->Enable(false);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(sliderSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

//========================================================================
//                  dialogTextWindow: Member functions
//========================================================================

dialogTextWindow::dialogTextWindow(wxWindow *p_parent,
				   const wxString &p_title,
				   const wxString &p_contents)
  : wxDialog(p_parent, -1, p_title, wxDefaultPosition, wxDefaultSize)
{
  SetAutoLayout(true);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();

  wxTextCtrl *textCtrl = new wxTextCtrl(this, -1, p_contents,
					wxDefaultPosition, wxSize(300, 200),
					wxTE_MULTILINE | wxTE_READONLY);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(textCtrl, 0, wxALL, 5);
  topSizer->Add(okButton, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

dialogTextWindow::~dialogTextWindow()
{ }


// gDrawTextGetNum will scan the string, starting at position i,
// for a number.  It will stop at the first non-digit character.
#include <ctype.h> 
#include <stdlib.h>

int gDrawTextGetNum(const gText &s, int *i)
{
    gText tmp;
    (*i) += 2;  // skip the opening {

    while (isdigit(s[(unsigned int) *i]) && (unsigned int) *i < s.Length() && 
	   s[(unsigned int) *i] != '}')
    {
        tmp += s[(unsigned int) *i];
        (*i)++;
    }

    (*i)++;     // skip the closing }
    return atoi((char *)tmp);
}


// Hack to be able to parse gPolys (x^2 -> x\^2\~).  No error checking: a bad
// input will cause a crash.
gText gDrawTextPreParse(const gText &s)
{
    if (((gText &)s).LastOccur('^') == 0) 
        return s;

    gText tmp;

    for (unsigned int n = 0; n < s.Length(); n++)
    {
        if (s[n] != '^')
            tmp += s[n];
        else
        {
            tmp += "\\^";
            while (isdigit(s[++n]) && n < s.Length()) tmp += s[n];
            tmp += "\\~";
            
            if (n < s.Length()) tmp += s[n];
        }
    }

    return tmp;
}


#define WX_COLOR_LIST_LENGTH 11

const char *wx_color_list[WX_COLOR_LIST_LENGTH] = 
{
    "GREEN", "RED", "BLUE", "YELLOW", "VIOLET RED", "SALMON", "ORCHID",
    "TURQUOISE", "BROWN", "PALE GREEN", "BLACK"
};

const char *wx_hilight_color_list[WX_COLOR_LIST_LENGTH] = 
{
    "PALE GREEN", "INDIAN RED", "MEDIUM BLUE", "YELLOW GREEN",
    "MEDIUM VIOLET RED", "TAN", "MEDIUM ORCHID", "MEDIUM TURQUOISE",
    "KHAKI", "SEA GREEN", "DARK GREY"
};

// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the wx_color_list.  To print a \, use a \\.
void gDrawText(wxDC &dc, const gText &s0, float x, float y)
{
  unsigned int i = 0;
  int c;
  long dx, dy;
  gText s = gDrawTextPreParse(s0);
  gText tmp;
  //gText old_foreground(wxTheColourDatabase->FindName(dc.GetTextForeground()));
  gText old_foreground("BLACK");
  wxFont *old_font = 0 /*, *small_font = 0*/;
  //float   old_y = 0;
  //  int old_size = 0;
  
  while (i < s.Length()) {
    tmp = gText();

    while (i < s.Length() && s[i] != '\\') {
      tmp += s[i];
      i++;
    }

    dc.DrawText((char *) tmp, (int) x, (int) y);
    dc.GetTextExtent((char *)tmp, &dx, &dy);
    x += dx;
        
    if (s[i] == '\\')  {   // has to be a command
      i++;

      switch (s[i]) {
      case '\\':
	dc.DrawText("\\", (int) x, (int) y);
	dc.GetTextExtent("\\", &dx, &dy);
	x += dx;
	i++;
	break;

      case 'C':
	c = (gDrawTextGetNum(s, (int *) &i)%WX_COLOR_LIST_LENGTH);
	dc.SetTextForeground(*wxTheColourDatabase->FindColour(wx_color_list[c]));
	break;

      case '^':       // Start superscript
#ifdef NOT_PORTED_YET
	if (!old_font) 
	  old_font = dc.GetFont();
	
	if (!old_size) 
	  old_size = old_font->GetPointSize();
	
	if (!small_font) {
	  small_font = 
	    wxTheFontList->FindOrCreateFont(old_size*2/3, 
					    old_font->GetFamily(), 
					    old_font->GetStyle(), 
					    old_font->GetWeight());
	}

	dc.SetFont(small_font);
	old_y = y;
	y -= dy/4;
#endif  // NOT_PORTED_YET
	i++;
	break;

      case '_':       // Start subscript
#ifdef NOT_PORTED_YET
	if (!old_font) 
	  old_font = dc.GetFont();
	
	if (!old_size) 
	  old_size = old_font->GetPointSize();
	
	if (!small_font) {
	  small_font = 
	    wxTheFontList->FindOrCreateFont(old_size*2/3, 
					    old_font->GetFamily(), 
					    old_font->GetStyle(), 
					    old_font->GetWeight());
	}
	
	dc.SetFont(small_font);
	old_y = y;
	y += dy*2/3;
#endif  // NOT_PORTED_YET
	i++;
	break;

      case '~':       // Stop sub/super script
#ifdef NOT_PORTED_YET
	if (old_font) {
	  dc.SetFont(old_font);
	  y = old_y;
	  i++;
	}
#endif  // NOT_PORTED_YET
	break;
		
      default:
	wxError("Unknown code in gDrawText");
	break;
      }
    }
  }

  dc.SetTextForeground(*wxTheColourDatabase->FindColour((char *) old_foreground));
  
  if (old_font) 
    dc.SetFont(*old_font);
}


void gGetTextExtent(wxDC &dc, const gText &s0, float *x, float *y)
{
  unsigned int i = 0;
  int c;
  long dx, dy;
  gText s = gDrawTextPreParse(s0);
  gText tmp;
  wxFont *old_font = 0/*, *small_font = 0*/;
  // float   old_y = 0;
  //  int old_size = 0;
  *x = 0;
  *y = 0;
    
  while (i < s.Length()) {
    tmp = gText();

    while (i < s.Length() && s[i] != '\\') {
      tmp += s[i];
      i++;
    }

    dc.GetTextExtent((char *)tmp, &dx, &dy);
    *x += dx;
        
    if (dy < *y) 
      *y = dy;

    if (s[i] == '\\') {   // has to be a command
      i++;

      switch (s[i]) {
      case '\\':
	dc.GetTextExtent("\\", &dx, &dy);
	*x += dx;
	i++;
	break;

      case 'C':
	c = (gDrawTextGetNum(s, (int *) &i) % WX_COLOR_LIST_LENGTH);
	break;

      case '^':       // Start superscript
#ifdef NOT_PORTED_YET
	if (!old_font) 
	  old_font = dc.GetFont();
	
	if (!old_size) 
	  old_size = old_font->GetPointSize();
	
	if (!small_font) {
	  small_font = 
	    wxTheFontList->FindOrCreateFont(old_size*2/3, 
					    old_font->GetFamily(), 
					    old_font->GetStyle(), 
					    old_font->GetWeight());
	}
	
	dc.SetFont(small_font);
	old_y = *y;
	*y -= dy/4;
#endif  // NOT_PORTED_YET
	i++;
	break;
	
      case '_':       // Start subscript
#ifdef NOT_PORTED_YET
	if (!old_font) 
	  old_font = dc.GetFont();
                
	if (!old_size) 
	  old_size = old_font->GetPointSize();
                
	if (!small_font) {
	  small_font = wxTheFontList->FindOrCreateFont(old_size*2/3, 
						       old_font->GetFamily(), 
						       old_font->GetStyle(), 
						       old_font->GetWeight());
	}
	
	dc.SetFont(small_font);
	old_y = *y;
	*y += dy*2/3;
#endif  // NOT_PORTED_YET
	i++;
	break;

      case '~':       // Stop sub/super script
#ifdef NOT_PORTED_YET
	if (old_font) {
	  dc.SetFont(old_font);
	  *y = old_y;
	  i++;
	}
#endif  // NOT_PORTED_YET
	break;

      default:
	wxError("Unknown code in gDrawText");
	break;
      }
    }
  }
  
  if (old_font) {
    dc.SetFont(*old_font);
  }
}
