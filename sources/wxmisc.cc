//
// FILE: wxmisc.cc -- a few general purpose functions that rely on and enhance
//                    wxwin.
//
// $Id$
//

#include "wx/wx.h"
#include "wx/help.h"  // for help
#include "wx/config.h"  // for config

#ifdef __BORLANDC__
#pragma hdr_stop
#endif  // __BORLANDC__

#include "wxmisc.h"

#include "system.h"
#include "gmisc.h"

//***************************************************************************
//                       RANDOM USEFUL FUNCTIONS
// These will hopefully be a part of wxwin some day.  These all belong in
// different files/classes.
//***************************************************************************

// List Find String function--finds the index of a string in a wxList
int wxListFindString(wxList *l, char *s)
{
    for (int i = 0; i < l->Number(); i++)
        if (strcmp((char *)l->Nth(i)->Data(), s) == 0) return i;

    return -1;
}


// Find In Array function--finds an integer in an array of integers, returns index
int FindIntArray(int *array, int num, int what)
{
    for (int i = 0; i < num; i++) 
    {
        if (array[i] == what) 
            return i;
    }

    return -1;
}


// Returns a wxStringList with string representations
// for integers 1..n, or m..n if the start value is supplied.
// It creates a new list if l == NULL; otherwise it appends to
// the list supplied.
wxStringList* wxStringListInts(int num, wxStringList *l, int start)
{
    wxStringList *tmp = (l) ? l : new wxStringList;
    char tmp_str[10];

    for (int i = start; i <= num; i++)
    {
        sprintf(tmp_str, "%d", i);
        tmp->Add(tmp_str);
    }

    return tmp;
}


// Font To String--writes all the font data to a string.  Use with String To Font
// Note, returns a static buffer, copy it if needed
char *wxFontToString(wxFont *f)
{
    static char fts_buffer[200];
    // write the size, family, style, weight, underlining
    sprintf(fts_buffer, "%d %d %d %d %d", f->GetPointSize(), f->GetFamily(),
            f->GetStyle(), f->GetWeight(), f->GetUnderlined());

    return fts_buffer;
}


// String To Font--converts the data in a string created by Font To String
// into a font.  Note that this allocated the required memory.  Delete if
// necessary.
wxFont *wxStringToFont(char *s)
{
    int si, f, st, w, u;
    sscanf(s, "%d %d %d %d %d", &si, &f, &st, &w, &u);

    return (new wxFont(si, f, st, w, u));
}


// FindFile
char *wxFindFile(const char *name)
{
  wxPathList path_list;
  path_list.AddEnvList("PATH");
  const char *file_name = path_list.FindValidPath((char *)name);
  
  return  (file_name) ? copystring(file_name) : 0;
}

//------------------------
// Help system functions
//------------------------

wxHelpController *s_helpController = 0;

void wxInitHelp(const char *name, const char *help_about_str)
{
  if (!s_helpController) {
    s_helpController = new wxHelpController;
    s_helpController->Initialize((char *) name);
  }

  if (help_about_str) {
    wxHelpAbout(help_about_str);
  }
}

void wxHelpContents(const char *section)
{
#ifdef NOT_PORTED_YET
#ifdef __GNUG__
  // get topic
  gText topic;
  if(!section || section == "") 
    topic = "GAMBIT GUI"; 
  else 
    topic = section;

  // get html Directory
  gText htmlDir = System::GetEnv("GAMBITHOME");
  htmlDir+="/doc/html";
  wxGetResourceStr("Install", "HTML-Dir", htmlDir,"gambitrc");

  // search for html file corresponding to section.  
  System::Shell("grep -l '<title>"+topic+"<' "+htmlDir+"/*.html > junk.hlp");
  gFileInput file("junk.hlp");
  char a;
  gText html_file;
  while (!file.eof()) {
    file >> a;
    if(a != '\n') html_file += a;
  }
  int last = html_file.Length()-1;
  if(html_file[last]!='l') html_file.Remove(last); // get rid of line feed
  System::Shell("rm junk.hlp");

  // display on netscape.
  // Use -install flag when launching netscape to install private color map
  // Otherwise gambit and netscape cannot run simultaneously.  
  if(System::Shell("netscape -remote 'OpenFile("+html_file+")'")) {
    System::Shell("echo Launching Netscape");
    System::Shell("netscape -install "+html_file+" &");
  }
#else
  s_helpController->LoadFile();
#endif  // __GNUG__
#endif  // NOT_PORTED_YET
}

void wxHelpAbout(const char *helpstr)
{
  static char *help_str = "Product based on wxWin";
    
  if (helpstr) {    // init with a new string
    help_str = strdup(helpstr);
  }
  else {
    wxMessageBox(help_str, "Help About");
  }
}

void wxKillHelp(void)
{
  if (s_helpController) {
    s_helpController->Quit();
    delete s_helpController;
    s_helpController = 0;
  }
}

//========================================================================
//                     guiAutoDialog: Member functions
//========================================================================

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

void guiAutoDialog::Go(void)
{
  Layout();
  Fit();

  const wxWindowList &children = GetChildren();

  int minX = 1000, minY = 1000, totalWidth = 0, totalHeight = 0;

  for (wxNode *child = children.First(); child != 0; child = child->Next()) {
    wxWindow *data = (wxWindow *) child->Data();
    int x, y, width, height;
    data->GetPosition(&x, &y);
    data->GetSize(&width, &height);

    minX = gmin(minX, x);
    minY = gmin(minY, y);
    totalWidth = gmax(totalWidth, x + width);
    totalHeight = gmax(totalHeight, y + height);
  }

  SetClientSize(totalWidth + 10, totalHeight + 10);
  Layout();
}

void guiAutoDialog::OnHelp(void)
{
wxHelpContents(HelpString());
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


//***************************** BASIC KEYBOARD STUFF ****************

bool    IsCursor(wxKeyEvent &ev)
{
    long ch = ev.KeyCode();
    return   (ch == WXK_UP || ch == WXK_DOWN || ch == WXK_LEFT || 
              ch == WXK_RIGHT || ch == WXK_TAB || ch == WXK_RETURN);
}


bool    IsEnter(wxKeyEvent &ev)
{
    return (ev.KeyCode() == 'm' && ev.ControlDown());
}


bool    IsNumeric(wxKeyEvent &ev)
{
    long ch = ev.KeyCode();
    return ((ch >= '0' && ch <= '9') || ch == '-' ||    ch == '.');
}


bool    IsAlphaNum(wxKeyEvent &ev)
{
    return !(IsCursor(ev) || IsDelete(ev) || IsEnter(ev));
}


bool    IsDelete(wxKeyEvent &ev)
{
    return ((ev.KeyCode() == WXK_DELETE) ||
            (ev.KeyCode() == WXK_BACK));
}


// gDrawTextGetNum will scan the string, starting at position i,
// for a number.  It will stop at the first non-digit character.
#include <ctype.h> 
#include <stdlib.h>

int gDrawTextGetNum(const gText &s, int *i)
{
    gText tmp;
    (*i) += 2;  // skip the opening {

    while (isdigit(s[*i]) && *i < s.Length() && s[*i] != '}')
    {
        tmp += s[*i];
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

    for (int n = 0; n < s.Length(); n++)
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
  int i = 0, c;
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

    dc.DrawText((char *)tmp, x, y);
    dc.GetTextExtent((char *)tmp, &dx, &dy);
    x += dx;
        
    if (s[i] == '\\')  {   // has to be a command
      i++;

      switch (s[i]) {
      case '\\':
	dc.DrawText("\\", x, y);
	dc.GetTextExtent("\\", &dx, &dy);
	x += dx;
	i++;
	break;

      case 'C':
	c = (gDrawTextGetNum(s, &i)%WX_COLOR_LIST_LENGTH);
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
  int i = 0, c;
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
	c = (gDrawTextGetNum(s, &i) % WX_COLOR_LIST_LENGTH);
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


// Takes a string formated for gDrawText and returns just the text value of it.
gText gPlainText(const gText &s)
{
    int i = 0;
    gText plain;

    while (i < s.Length())
    {
        while (i < s.Length() && s[i] != '\\')
        {
            plain += s[i];
            i++;
        }
        
        if (s[i] == '\\')   // has to be a command
        {
            i++;
            switch (s[i])
            {
            case '\\':
                plain += "\\";
                break;

            case 'C' :
                gDrawTextGetNum(s, &i); // just absorb that info
                break;

            case '^' :
            case '_' :
                plain += s[i];
                break;

            case '~' :
                break;

            default:
                wxError("Unknown code in gDrawText");
                break;
            }
        }
    }

    return plain;
}

gText gFileNameFromPath(const char *path)
{
  gText filename(path);
  if(strstr(path,"/") || strstr(path,"\\"))
    filename = FileNameFromPath((char *)path);
  return filename;
}

gText gPathOnly(const char *name)
{
  gText path;
  if(strstr(name,"/") || strstr(name,"\\"))
    path = wxPathOnly((char *)name);
  return path;
}


