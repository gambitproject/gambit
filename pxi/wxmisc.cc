#include "wx/wx.h"
#include "wx/help.h"  // for help
#include "wx/config.h"  // for config

#pragma hdr_stop
#include "wxmisc.h"

//***************************************************************************
//                       RANDOM USEFUL FUNCTIONS
// These will hopefully be a part of wxwin some day.  These all belong in
// different files/classes.
//***************************************************************************

// List Find String function--finds the index of a string in a wxList

int wxListFindString(wxList *l,char *s)
{
  for (int i=0;i<l->Number();i++)
    if (strcmp((char *)l->Nth(i)->Data(),s)==0) return i;
  return -1;
}

// Font To String--writes all the font data to a string.  Use with String To Font
// Note, returns a static buffer, copy it if needed

char *wxFontToString(wxFont *f)
{
  static char fts_buffer[200];
  // write the size,family,style,weight,underlining
  sprintf(fts_buffer,"%d %d %d %d %d",f->GetPointSize(),f->GetFamily(),
	  f->GetStyle(),f->GetWeight(),f->GetUnderlined());
  return fts_buffer;
}

// String To Font--converts the data in a string created by Font To String
// into a font.  Note that this allocated the required memory.  Delete if
// necessary.

wxFont *wxStringToFont(char *s)
{
  int si,f,st,w,u;
  sscanf(s,"%d %d %d %d %d",&si,&f,&st,&w,&u);
  return (new wxFont(si,f,st,w,u));
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

//------------------------------------------------------------------------
//                 class wxNumberItem: Member functions
//------------------------------------------------------------------------

wxNumberItem::wxNumberItem(wxPanel *p_parent, char *p_label,
			   const wxString &p_default, const wxPoint &pos, 
			   const wxSize &size)
  : wxTextCtrl(p_parent, -1, "", pos, size, 0, 
#ifdef __WXMSW__ // apperent bug in wxwin2 for MSW
	       wxTextValidator(wxFILTER_ASCII, &m_data)), 
#else
	       wxTextValidator(wxFILTER_NUMERIC, &m_data)), 
#endif
    m_data(p_default)
{
  SetValue((const char *) p_default);
}

wxNumberItem::~wxNumberItem()
{ }

void wxNumberItem::SetNumber(const double &p_value)
{
   m_data.Printf("%f",p_value);
}

double wxNumberItem::GetNumber(void)
{
  m_data.ToDouble(&m_value);
  return m_value;
}

#include "gmisc.h"

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
//                    wxOutputDialogBox: Member functions
//========================================================================


wxOutputDialogBox::wxOutputDialogBox(wxStringList *p_extraMedia,
				     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Output Media")
{
  /*
    // some sort of bug here with additional media -- causes crash in 
    // wxRadioBox on motif  -- for now don't worry about extraMedia

  wxStringList mediaList("Printer", "PostScript", "Clipboard",
			 "Metafile", "Print Preview", 0);
  
  if (p_extraMedia) {
    for (int i = 0; i < p_extraMedia->Number(); i++)
      mediaList.Add((const char *)(p_extraMedia->Nth(i)->Data()));
  }
  */


  wxString mediaList[] = { "Printer", "PostScript", "Clipboard", "Metafile", "Print Preview" };

  m_mediaBox = new wxRadioBox(this, 0, "Media", wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			      wxSize(250,125),
#else
			      
			      wxDefaultSize,
#endif
			      // mediaList.Number(), (const wxString *)mediaList.ListToArray(),
			      //(int)(mediaList.Number()/2));
			      5, mediaList, 2);
  
  m_fitBox = new wxCheckBox(this, 0, "Fit to page");
  
#ifndef __WXMSW__ // Printer, Clipboard, and MetaFiles are not yet supp'ed
  m_mediaBox->Enable(0, false);
  m_mediaBox->Enable(2, false);
  m_mediaBox->Enable(3, false);
  m_fitBox->Enable(false);
#endif  // __WXMSW__

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);

  allSizer->Add(m_mediaBox, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_fitBox, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
}

wxOutputMedia wxOutputDialogBox::GetMedia(void) const
{
  switch (m_mediaBox->GetSelection()) {
  case 0: return wxMEDIA_PRINTER;
  case 1: return wxMEDIA_PS;
  case 2: return wxMEDIA_CLIPBOARD;
  case 3: return wxMEDIA_METAFILE;
  case 4: return wxMEDIA_PREVIEW;
  default: return wxMEDIA_NUM;
  }
}

int wxOutputDialogBox::GetExtraMedia(void) const
{
  return ((m_mediaBox->GetSelection() >= wxMEDIA_NUM) ? 
	  m_mediaBox->GetSelection() : -1);
}

wxOutputOption wxOutputDialogBox::GetOption(void) const
{
  return (m_fitBox->GetValue()) ? wxFITTOPAGE : wxWYSIWYG;
}

bool wxOutputDialogBox::ExtraMedia(void) const
{
  return (m_mediaBox->GetSelection() >= wxMEDIA_NUM);
}
