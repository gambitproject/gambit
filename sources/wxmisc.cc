//
// FILE: wxmisc.cc -- a few general purpose functions that rely on and enhance
//                    wxwin.
// $Id$
//

#include "wx.h"
#include "wx_form.h"
#include "wx_help.h"
#ifdef __BORLANDC__
#pragma hdr_stop
#endif  // __BORLANDC__
#define WXMISC_C
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
    char *file_name = path_list.FindValidPath((char *)name);

    return  (file_name) ? copystring(file_name) : 0;
}

//------------------------
// Help system functions
//------------------------

wxHelpInstance *s_helpInstance = 0;

void wxInitHelp(const char *name, const char *help_about_str)
{
  if (!s_helpInstance) {
    s_helpInstance = new wxHelpInstance(TRUE);
    s_helpInstance->Initialize((char *) name);
  }
    
  if (help_about_str) {
    wxHelpAbout(help_about_str);
  }
}

void wxHelpContents(const char *section)
{
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
    s_helpInstance->LoadFile();
    
  if (!section) {
    s_helpInstance->DisplayContents();
  }
  else {
    s_helpInstance->KeywordSearch((char *) section);
  }
#endif
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
  if (s_helpInstance) {
    s_helpInstance->Quit();
    delete s_helpInstance;
    s_helpInstance = 0;
  }
}

// Need this function since the default wxGeResource takes a char **value,
// and replaces it with a newly created string (while deleting the old one).
// This is NOT what we want.

Bool wxGetResourceStr(char *section, char *entry, gText &value, char *file)
{
  char *tmp_str = 0;
  Bool ok = wxGetResource(section, entry, &tmp_str, file);

  if (ok) {
    value = tmp_str;
    delete [] tmp_str;
  }

  return ok;
}


//========================================================================
//                     guiAutoDialog: Member functions
//========================================================================

guiAutoDialog::guiAutoDialog(wxWindow *p_parent, char *p_title)
  : wxDialogBox(p_parent, p_title, TRUE)
{
  SetAutoLayout(TRUE);

  m_okButton = new wxButton(this, (wxFunction) CallbackOk, "OK");
  m_okButton->SetClientData((char *) this);
  m_okButton->SetDefault();
  m_okButton->SetConstraints(new wxLayoutConstraints);

  m_cancelButton = new wxButton(this, (wxFunction) CallbackCancel, "Cancel");
  m_cancelButton->SetClientData((char *) this);
  m_cancelButton->SetConstraints(new wxLayoutConstraints);

  m_helpButton = new wxButton(this, (wxFunction) CallbackHelp, "Help");
  m_helpButton->SetClientData((char *) this);
  m_helpButton->SetConstraints(new wxLayoutConstraints);
}

void guiAutoDialog::Go(void)
{
  Layout();
  Fit();
  wxList *children = GetChildren();

  int minX = 1000, minY = 1000, totalWidth = 0, totalHeight = 0;

  for (wxNode *child = children->First(); child != 0; child = child->Next()) {
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
  
  Show(TRUE);
}

void guiAutoDialog::OnOk(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void guiAutoDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool guiAutoDialog::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void guiAutoDialog::OnHelp(void)
{
wxHelpContents(HelpString());
}

//========================================================================
//                     guiPagedDialog: Member functions
//========================================================================

int guiPagedDialog::s_itemsPerPage = 8;

guiPagedDialog::guiPagedDialog(wxWindow *p_parent, char *p_title,
			       int p_numValues)
  : guiAutoDialog(p_parent, p_title),
    m_pageNumber(0), m_numFields(gmin(p_numValues, s_itemsPerPage)),
    m_backButton(0), m_nextButton(0),
    m_dataValues(p_numValues)
{
  m_dataFields = new wxText *[m_numFields];
  for (int i = 1; i <= m_numFields; i++) {
    m_dataFields[i-1] = new wxText(this, 0, ToText(i) + "  ", "", 1, 1,
				   -1, -1, wxFIXED_LENGTH);
  }

  if (p_numValues > s_itemsPerPage) {
    m_backButton = new wxButton(this, (wxFunction) CallbackBack,
				"<< Back");
    m_backButton->SetClientData((char *) this);
    m_backButton->Enable(FALSE);

    m_backButton->SetConstraints(new wxLayoutConstraints);
    m_backButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
                                               wxBottom, 10);
    m_backButton->GetConstraints()->right.SameAs(m_cancelButton, wxCentreX, 5);
    m_backButton->GetConstraints()->height.AsIs();
    m_backButton->GetConstraints()->width.AsIs();

    m_nextButton = new wxButton(this, (wxFunction) CallbackNext,
				"Next >>");
    m_nextButton->SetClientData((char *) this);
    m_nextButton->SetConstraints(new wxLayoutConstraints);
    m_nextButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
					       wxBottom, 10);
    m_nextButton->GetConstraints()->left.SameAs(m_cancelButton, wxCentreX, 5);
    m_nextButton->GetConstraints()->height.AsIs();
    m_nextButton->GetConstraints()->width.AsIs();
  }

  if (m_backButton)  {
    m_okButton->GetConstraints()->top.SameAs(m_backButton, wxBottom, 10);
  }
  else  {
    m_okButton->GetConstraints()->top.SameAs(m_dataFields[m_numFields-1],
					     wxBottom, 10);
  }
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_cancelButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  for (int i = 1; i <= m_numFields; i++) {
    m_dataFields[i-1]->SetConstraints(new wxLayoutConstraints);
    if (i == 1) {
      m_dataFields[i-1]->GetConstraints()->top.SameAs(this, wxTop, 10);
    }
    else {
      m_dataFields[i-1]->GetConstraints()->top.SameAs(m_dataFields[i-2],
						      wxBottom, 10);
    }
    m_dataFields[i-1]->GetConstraints()->height.AsIs();
    m_dataFields[i-1]->GetConstraints()->left.SameAs(m_okButton, wxLeft);
    m_dataFields[i-1]->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  }
}

guiPagedDialog::~guiPagedDialog()
{
  delete [] m_dataFields;
}

void guiPagedDialog::OnOk(void)
{
  guiAutoDialog::OnOk();
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < m_numFields; i++, entry++) {
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();
  }
}

void guiPagedDialog::OnBack(void)
{
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < gmin((m_pageNumber + 1) * s_itemsPerPage,
		m_dataValues.Length()); i++, entry++)
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();

  m_pageNumber--;
  entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++) {
    m_dataFields[entry]->Show(FALSE);
    m_dataFields[entry]->SetValue(m_dataValues[i + 1]);
    m_dataFields[entry]->SetLabel(ToText(i + 1));
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_itemsPerPage - 1; entry >= 0; entry--)
    m_dataFields[entry]->Show(TRUE);
  
  m_dataFields[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void guiPagedDialog::OnNext(void)
{
  int entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++)
    m_dataValues[i + 1] = m_dataFields[entry]->GetValue();

  m_pageNumber++;
  entry = 0;
  for (int i = m_pageNumber * s_itemsPerPage;
       i < (m_pageNumber + 1) * s_itemsPerPage; i++, entry++) {
    if (i < m_dataValues.Length()) {
      m_dataFields[entry]->SetValue(m_dataValues[i + 1]);
      m_dataFields[entry]->SetLabel(ToText(i + 1));
    }
    else
      m_dataFields[entry]->Show(FALSE);
  }

  m_dataFields[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_itemsPerPage <=
		       m_dataValues.Length());
}

void guiPagedDialog::SetValue(int p_index, const gText &p_value)
{
  m_dataValues[p_index] = p_value;
  if ((p_index - 1) / s_itemsPerPage == m_pageNumber)
    m_dataFields[(p_index - 1) % s_itemsPerPage]->SetValue(p_value);
}


//========================================================================
//                    guiSliderDialog: Member functions
//========================================================================

guiSliderDialog::guiSliderDialog(wxWindow *p_parent, const gText &p_caption,
				 int p_min, int p_max, int p_default)
  : guiAutoDialog(p_parent, p_caption)
{
  m_slider = new wxSlider(this, 0, p_caption, p_default, p_min, p_max, 250);
  m_slider->SetConstraints(new wxLayoutConstraints);
  m_slider->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_slider->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_slider->GetConstraints()->width.AsIs();
  m_slider->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->top.SameAs(m_slider, wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->Show(FALSE);
  m_helpButton->GetConstraints()->top.SameAs(this, wxTop);
  m_helpButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

//========================================================================
//                     guiFloatDialog: Member functions
//========================================================================

guiFloatDialog::guiFloatDialog(wxWindow *p_parent, const gText &p_caption,
			       const gText &p_label,
			       double p_min, double p_max, double p_default)
  : guiAutoDialog(p_parent, p_caption), m_min(p_min), m_max(p_max)
{
  m_value = new wxText(this, 0, p_label);
  m_value->SetValue(ToText(p_default));
  m_value->SetConstraints(new wxLayoutConstraints);
  m_value->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_value->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_value->GetConstraints()->width.AsIs();
  m_value->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->top.SameAs(m_value, wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->Show(FALSE);
  m_helpButton->GetConstraints()->top.SameAs(this, wxTop);
  m_helpButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

void guiFloatDialog::OnOk(void)
{
  double value = GetValue();

  if (value >= m_min && value <= m_max) {
    guiAutoDialog::OnOk();
  }
  else {
    wxMessageBox("Value must be between " + ToText(m_min) + " and " +
		 ToText(m_max), "Out of range");
  }
}

double guiFloatDialog::GetValue(void) const
{ return ToNumber(m_value->GetValue()); }

//========================================================================
//                     FontDialogBox: Member functions
//========================================================================

FontDialogBox::FontDialogBox(wxWindow *p_parent, wxFont *p_default) 
  : guiAutoDialog(p_parent, "Select Font")
{
  char *nameChoices[] = { "Swiss", "Roman", "Decorative", "Modern",
			  "Script" };
  m_nameItem = new wxChoice(this, 0, "Name", 1, 1, -1, -1,
			    5, nameChoices);
  if (p_default) {
    switch (p_default->GetFamily()) {
    case wxSWISS: m_nameItem->SetSelection(0);  break;
    case wxROMAN: m_nameItem->SetSelection(1);  break;
    case wxDECORATIVE: m_nameItem->SetSelection(2);  break;
    case wxMODERN: m_nameItem->SetSelection(3);  break;
    case wxSCRIPT: m_nameItem->SetSelection(4);  break;
    default:   break;
    }
  }
  m_nameItem->SetConstraints(new wxLayoutConstraints);
  m_nameItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_nameItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_nameItem->GetConstraints()->width.AsIs();
  m_nameItem->GetConstraints()->height.AsIs();

  char *sizeChoices[] = { "8", "10", "12", "14", "18", "24", "28" };
  m_sizeItem = new wxChoice(this, 0, "Size", 1, 1, -1, -1,
			    7, sizeChoices);
  if (p_default) {
    switch (p_default->GetPointSize()) {
    case 8: m_sizeItem->SetSelection(0);  break;
    case 10: m_sizeItem->SetSelection(1);  break;
    case 12: m_sizeItem->SetSelection(2);  break;
    case 14: m_sizeItem->SetSelection(3);  break;
    case 18: m_sizeItem->SetSelection(4);  break;
    case 24: m_sizeItem->SetSelection(5);  break;
    case 28: m_sizeItem->SetSelection(6);  break;
    default:  break;
    }
  }
  m_sizeItem->SetConstraints(new wxLayoutConstraints);
  m_sizeItem->GetConstraints()->left.SameAs(m_nameItem, wxRight, 10);
  m_sizeItem->GetConstraints()->top.SameAs(m_nameItem, wxTop);
  m_sizeItem->GetConstraints()->width.AsIs();
  m_sizeItem->GetConstraints()->height.AsIs();

  char *styleChoices[] = { "Normal", "Italic", "Slant" };
  m_styleItem = new wxRadioBox(this, 0, "Style", 1, 1, -1, -1,
			       3, styleChoices);
  if (p_default) {
    switch (p_default->GetStyle()) {
    case wxNORMAL: m_styleItem->SetSelection(0);  break;
    case wxITALIC: m_styleItem->SetSelection(1);  break;
    case wxSLANT: m_styleItem->SetSelection(2);  break;
    default:  break;
    }
  }
  m_styleItem->SetConstraints(new wxLayoutConstraints);
  m_styleItem->GetConstraints()->left.SameAs(m_nameItem, wxLeft);
  m_styleItem->GetConstraints()->top.SameAs(m_nameItem, wxBottom, 10);
  m_styleItem->GetConstraints()->width.AsIs();
  m_styleItem->GetConstraints()->height.AsIs();

  char *weightChoices[] = { "Normal", "Light", "Bold" };
  m_weightItem = new wxRadioBox(this, 0, "Weight", 1, 1, -1, -1,
				3, weightChoices);
  if (p_default) {
    switch (p_default->GetWeight()) {
    case wxNORMAL: m_weightItem->SetSelection(0);  break;
    case wxLIGHT: m_weightItem->SetSelection(1);  break;
    case wxBOLD: m_weightItem->SetSelection(2);  break;
    default:  break;
    }
  }
  m_weightItem->SetConstraints(new wxLayoutConstraints);
  m_weightItem->GetConstraints()->left.SameAs(m_styleItem, wxLeft);
  m_weightItem->GetConstraints()->top.SameAs(m_styleItem, wxBottom, 10);
  m_weightItem->GetConstraints()->width.AsIs();
  m_weightItem->GetConstraints()->height.AsIs();

  m_underlineItem = new wxCheckBox(this, 0, "Underline", 1, 1, -1, -1);
  if (p_default) {
    m_underlineItem->SetValue(p_default->GetUnderlined());
  }
  m_underlineItem->SetConstraints(new wxLayoutConstraints);
  m_underlineItem->GetConstraints()->left.SameAs(m_weightItem, wxLeft);
  m_underlineItem->GetConstraints()->top.SameAs(m_weightItem, wxBottom, 10);
  m_underlineItem->GetConstraints()->width.AsIs();
  m_underlineItem->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_underlineItem, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->top.AsIs();
  m_helpButton->GetConstraints()->left.AsIs();
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->Show(FALSE);

  Go();
}

wxFont *FontDialogBox::MakeFont(void)
{
  static int sizes[] = { 8, 10, 12, 14, 18, 24, 28 };
  static int names[] = { wxSWISS, wxROMAN, wxDECORATIVE, wxMODERN, wxSCRIPT };
  static int styles[] = { wxNORMAL, wxITALIC, wxSLANT };
  static int weights[] = { wxNORMAL, wxLIGHT, wxBOLD };

  return new wxFont(sizes[m_sizeItem->GetSelection()],
		    names[m_nameItem->GetSelection()],
		    styles[m_styleItem->GetSelection()],
		    weights[m_weightItem->GetSelection()],
		    m_underlineItem->GetValue());
}


//========================================================================
//                    wxOutputDialogBox: Member functions
//========================================================================

wxOutputDialogBox::wxOutputDialogBox(wxStringList *p_extraMedia,
				     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Output Media")
{
  wxStringList mediaList("Printer", "PostScript", "Clipboard",
			 "Metafile", "Print Preview", 0);
  if (p_extraMedia) {
    for (int i = 0; i < p_extraMedia->Number(); i++)
      mediaList.Add((const char *)(p_extraMedia->Nth(i)->Data()));
  }

  m_mediaBox = new wxRadioBox(this, 0, "Media", 1, 1, -1, -1, 
			      mediaList.Number(), mediaList.ListToArray(), 
			      (int)(mediaList.Number()/2));
  m_mediaBox->SetConstraints(new wxLayoutConstraints);
  m_mediaBox->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_mediaBox->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_mediaBox->GetConstraints()->width.AsIs();
  m_mediaBox->GetConstraints()->height.AsIs();

  m_fitBox = new wxCheckBox(this, 0, "Fit to page");
  m_fitBox->SetConstraints(new wxLayoutConstraints);
  m_fitBox->GetConstraints()->top.SameAs(m_mediaBox, wxBottom, 10);
  m_fitBox->GetConstraints()->left.SameAs(m_mediaBox, wxLeft);
  m_fitBox->GetConstraints()->width.AsIs();
  m_fitBox->GetConstraints()->height.AsIs();

#ifdef wx_x // Printer, Clipboard, and MetaFiles are not yet supp'ed
  m_mediaBox->Enable(0, FALSE);
  m_mediaBox->Enable(2, FALSE);
  m_mediaBox->Enable(3, FALSE);
  m_fitBox->Enable(FALSE);
#endif  // wx_x

  m_okButton->GetConstraints()->top.SameAs(m_fitBox, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->top.AsIs();
  m_helpButton->GetConstraints()->left.AsIs();
  m_helpButton->GetConstraints()->width.AsIs();
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->Show(FALSE);

  Go();
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

Bool wxOutputDialogBox::ExtraMedia(void) const
{
  return (m_mediaBox->GetSelection() >= wxMEDIA_NUM);
}


//***************************** BASIC KEYBOARD STUFF ****************

Bool    IsCursor(wxKeyEvent &ev)
{
    long ch = ev.KeyCode();
    return   (ch == WXK_UP || ch == WXK_DOWN || ch == WXK_LEFT || 
              ch == WXK_RIGHT || ch == WXK_TAB || ch == WXK_RETURN);
}


Bool    IsEnter(wxKeyEvent &ev)
{
    return (ev.KeyCode() == 'm' && ev.ControlDown());
}


Bool    IsNumeric(wxKeyEvent &ev)
{
    long ch = ev.KeyCode();
    return ((ch >= '0' && ch <= '9') || ch == '-' ||    ch == '.');
}


Bool    IsAlphaNum(wxKeyEvent &ev)
{
    return !(IsCursor(ev) || IsDelete(ev) || IsEnter(ev));
}


Bool    IsDelete(wxKeyEvent &ev)
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


// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the wx_color_list.  To print a \, use a \\.
void gDrawText(wxDC &dc, const gText &s0, float x, float y)
{
    int i = 0, c;
    float dx, dy;
    gText s = gDrawTextPreParse(s0);
    gText tmp;
    //gText old_foreground(wxTheColourDatabase->FindName(dc.GetTextForeground()));
    gText old_foreground("BLACK");
    wxFont *old_font = 0, *small_font = 0;
    float   old_y = 0;
    int old_size = 0;
    
    while (i < s.Length())
    {
        tmp = gText();

        while (i < s.Length() && s[i] != '\\')
        {
            tmp += s[i];
            i++;
        }

        dc.DrawText((char *)tmp, x, y);
        dc.GetTextExtent((char *)tmp, &dx, &dy);
        x += dx;
        
        if (s[i] == '\\')   // has to be a command
        {
            i++;

            switch (s[i])
            {
            case '\\':
                dc.DrawText("\\", x, y);
                dc.GetTextExtent("\\", &dx, &dy);
                x += dx;
                i++;
                break;

            case 'C':
                c = (gDrawTextGetNum(s, &i)%WX_COLOR_LIST_LENGTH);
                dc.SetTextForeground(wxTheColourDatabase->FindColour(wx_color_list[c]));
                break;

            case '^':       // Start superscript
                if (!old_font) 
                    old_font = dc.GetFont();
                
                if (!old_size) 
                    old_size = old_font->GetPointSize();
                
                if (!small_font) 
                {
                    small_font = 
                        wxTheFontList->FindOrCreateFont(old_size*2/3, 
                                                        old_font->GetFamily(), 
                                                        old_font->GetStyle(), 
                                                        old_font->GetWeight());
                }

                dc.SetFont(small_font);
                old_y = y;
                y -= dy/4;
                i++;
                break;

            case '_':       // Start subscript
                if (!old_font) 
                    old_font = dc.GetFont();
                
                if (!old_size) 
                    old_size = old_font->GetPointSize();
                
                if (!small_font) 
                {
                    small_font = 
                        wxTheFontList->FindOrCreateFont(old_size*2/3, 
                                                        old_font->GetFamily(), 
                                                        old_font->GetStyle(), 
                                                        old_font->GetWeight());
                }

                dc.SetFont(small_font);
                old_y = y;
                y += dy*2/3;
                i++;
                break;

            case '~':       // Stop sub/super script
                if (old_font)
                {
                    dc.SetFont(old_font);
                    y = old_y;
                    i++;
                }
                break;

            default:
                wxError("Unknown code in gDrawText");
                break;
            }
        }
    }

    dc.SetTextForeground(wxTheColourDatabase->FindColour(old_foreground));
    
    if (old_font) 
        dc.SetFont(old_font);
}


void gGetTextExtent(wxDC &dc, const gText &s0, float *x, float *y)
{
    int i = 0, c;
    float dx, dy;
    gText s = gDrawTextPreParse(s0);
    gText tmp;
    wxFont *old_font = 0, *small_font = 0;
    float   old_y = 0;
    int old_size = 0;
    *x = 0;
    *y = 0;
    
    while (i < s.Length())
    {
        tmp = gText();

        while (i < s.Length() && s[i] != '\\')
        {
            tmp += s[i];
            i++;
        }

        dc.GetTextExtent((char *)tmp, &dx, &dy);
        *x += dx;
        
        if (dy < *y) 
            *y = dy;

        if (s[i] == '\\')   // has to be a command
        {
            i++;

            switch (s[i])
            {
            case '\\':
                dc.GetTextExtent("\\", &dx, &dy);
                *x += dx;
                i++;
                break;

            case 'C':
                c = (gDrawTextGetNum(s, &i) % WX_COLOR_LIST_LENGTH);
                break;

            case '^':       // Start superscript
                if (!old_font) 
                    old_font = dc.GetFont();
                
                if (!old_size) 
                    old_size = old_font->GetPointSize();
                
                if (!small_font)
                {
                    small_font = 
                        wxTheFontList->FindOrCreateFont(old_size*2/3, 
                                                        old_font->GetFamily(), 
                                                        old_font->GetStyle(), 
                                                        old_font->GetWeight());
                }

                dc.SetFont(small_font);
                old_y = *y;
                *y -= dy/4;
                i++;
                break;

            case '_':       // Start subscript
                if (!old_font) 
                    old_font = dc.GetFont();
                
                if (!old_size) 
                    old_size = old_font->GetPointSize();
                
                if (!small_font)
                {
                    small_font = wxTheFontList->FindOrCreateFont(old_size*2/3, 
                                                                 old_font->GetFamily(), 
                                                                 old_font->GetStyle(), 
                                                                 old_font->GetWeight());
                }

                dc.SetFont(small_font);
                old_y = *y;
                *y += dy*2/3;
                i++;
                break;

            case '~':       // Stop sub/super script
                if (old_font)
                {
                    dc.SetFont(old_font);
                    *y = old_y;
                    i++;
                }
                break;

            default:
                wxError("Unknown code in gDrawText");
                break;
            }
        }
    }
    
    if (old_font) 
        dc.SetFont(old_font);
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


// gGetTextLine is a fancy version of the wxGetTextFrom user.  This single line
// input allows for all of the capability of gDrawText, line editing using
// cursor keys and the mouse.

class gGetTextCanvas;

class gGetTextFrame : public wxFrame
{
private:
    gGetTextCanvas *d;

public:
    gGetTextFrame(const gText s0, wxFrame *frame, 
                  int x, int y, const char *title, 
                  bool titlebar);
    Bool OnClose(void);
    int  Completed(void) const;
    gText GetString(void) const;
};


class gGetTextCanvas: public wxCanvas
{
private:
    int completed;
    gGetTextFrame *parent;
    gText s;
    float w;
    int pos;

public:
    gGetTextCanvas(gGetTextFrame *p, const gText s0);
    void OnChar(wxKeyEvent &ev);
    void OnEvent(wxMouseEvent &ev);
    void OnPaint(void);
    void OnKillFocus(void);
    void SetCompleted(int c);
    gText GetString(void) const;
    int Completed(void) const;
};



#define GETTEXT_FRAME_WIDTH     100
#define GETTEXT_FRAME_HEIGHT    30
#define GETTEXT_FRAME_WIDTH_M   300
#define TEXT_OFF                5

gGetTextFrame::gGetTextFrame(const gText s0, wxFrame *frame, int x, int y,
                             const char *title, bool titlebar)
    : wxFrame(frame, (char *) title, x, y, 
              GETTEXT_FRAME_WIDTH, GETTEXT_FRAME_HEIGHT,
              (titlebar) ? wxDEFAULT_FRAME : wxRESIZE_BORDER)
{
    d = new gGetTextCanvas(this, s0);
}


Bool gGetTextFrame::OnClose()
{
    d->SetCompleted(wxCANCEL);
    Show(FALSE);

    return FALSE;
}


int gGetTextFrame::Completed(void) const
{
    return d->Completed();
}


gText gGetTextFrame::GetString(void) const
{
    return d->GetString();
}


gGetTextCanvas::gGetTextCanvas(gGetTextFrame *parent_, const gText s0):
    wxCanvas(parent_), parent(parent_), s(s0)
{
    SetBackground(wxTheBrushList->FindOrCreateBrush("YELLOW", wxSOLID));
    SetFont(wxTheFontList->FindOrCreateFont(12, wxSWISS, wxNORMAL, wxNORMAL));
    GetDC()->SetBackgroundMode(wxTRANSPARENT);
    pos = s.Length();
    float h;
    gGetTextExtent(*GetDC(), s, &w, &h);
    parent->SetSize(-1, -1, (int)w*3/2+2*TEXT_OFF, -1, wxSIZE_USE_EXISTING);
    completed = wxRUNNING;
}


void gGetTextCanvas::OnChar(wxKeyEvent &ev)
{
    float h;
    int cur_w, cur_h;
    
    switch(ev.KeyCode())
    {
    case WXK_ESCAPE: 
        completed = wxCANCEL;
        break;

    case WXK_RETURN: 
        completed = wxOK;
        break;

    case WXK_LEFT:
        if (pos < 1) 
            break;
        
        pos--;
        OnPaint();
        break;

    case WXK_RIGHT:
        if (pos > s.Length() - 1) 
            break;

        pos++;
        OnPaint();
        break;
    
    case WXK_DELETE:
    case WXK_BACK:
        if (pos < 1)
            break;

        pos--;
        s.Remove(pos);
        gGetTextExtent(*GetDC(), s, &w, &h);
        GetSize(&cur_w, &cur_h);
        
        if (w < cur_w*2/3 && w > GETTEXT_FRAME_WIDTH)
        {
            cur_w = (int)(w+3*TEXT_OFF);
            parent->SetSize(-1, -1, cur_w, -1, wxSIZE_USE_EXISTING);
        }
        else
            OnPaint();
        break;

    default:
        s.Insert((char)ev.KeyCode(), pos);
        pos++;
        gGetTextExtent(*GetDC(), s, &w, &h);
        GetSize(&cur_w, &cur_h);
        
        if (w > cur_w && cur_w < GETTEXT_FRAME_WIDTH_M)
        {
            cur_w = (int)(w*3/2+3*TEXT_OFF);
            parent->SetSize(-1, -1, cur_w, -1, wxSIZE_USE_EXISTING);
        }
        else
            OnPaint();
        break;
    }
}


void gGetTextCanvas::OnPaint(void)
{
    Clear();
    gDrawText(*GetDC(), s, TEXT_OFF, TEXT_OFF);
    float h;
    gGetTextExtent(*GetDC(), s.Left(pos), &w, &h);
    SetPen(wxGREEN_PEN);
    DrawLine(w+TEXT_OFF*3/2, 0, w+TEXT_OFF*3/2, GETTEXT_FRAME_HEIGHT);
}


void gGetTextCanvas::OnEvent(wxMouseEvent &ev)
{
    if (ev.LeftDown())
    {
        float tw, th, tw1 = -1;
        // Find out where we clicked
        int npos = -1;

        for (int tpos = 1; tpos <= s.Length() && npos == -1; tpos++)
        {
            gGetTextExtent(*GetDC(), s.Left(tpos), &tw, &th);
            
            if (ev.x > tw1 && ev.x <= tw) 
                npos = tpos-1;

            tw1 = tw;
        }
        
        if (npos == -1) 
            npos = s.Length();

        pos = npos;
        OnPaint();
    }
}


int gGetTextCanvas::Completed(void) const
{
    return completed;
}


gText gGetTextCanvas::GetString(void) const
{
    return s;
}


void gGetTextCanvas::SetCompleted(int c)
{
    completed = c;
}


void gGetTextCanvas::OnKillFocus(void)
{
    SetFocus();
}


gText gGetTextLine(const gText &s0, wxFrame *parent, int x, int y,
                   const char *title, bool titlebar)
{
    gGetTextFrame *f = new gGetTextFrame(s0, parent, x, y, title, titlebar);
    f->Show(TRUE);
    f->CaptureMouse();

    while (f->Completed() == wxRUNNING) 
        wxYield();

    gText result_str;
    
    if (f->Completed() == wxOK) 
        result_str = f->GetString();

    f->ReleaseMouse();
    delete f;
    return result_str;
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


//------------------------------------------------------------------------
//                class wxIntegerItem: Member functions
//------------------------------------------------------------------------

static bool IsInteger(wxText &p_item)
{
  char *entry = p_item.GetValue();
  for (int i = 0; entry[i] != '\0'; i++) {
    if (!isdigit(entry[i]))  return false;
  }
  return true;
}

wxIntegerItem::wxIntegerItem(wxPanel *p_parent, char *p_label, int p_default,
			     int p_x, int p_y, int p_width, int p_height)
  : wxText(p_parent, (wxFunction) EventCallback, p_label, "",
	   p_x, p_y, p_width, p_height), m_value(p_default)
{
  // Passing a nonempty string to the wxText ctor affects the default
  // sizing.  So, we set after the fact instead.
  SetValue(ToText(p_default));
}

void wxIntegerItem::EventCallback(wxIntegerItem &p_item,
				  wxCommandEvent &p_event)
{
  if (p_event.eventType == wxEVENT_TYPE_TEXT_COMMAND) {
    if (!IsInteger(p_item)) {
      p_item.SetInteger(p_item.GetInteger());
      p_item.SetValue(ToText(p_item.GetInteger()));
    }
    else {
      p_item.SetInteger(atoi(p_item.GetValue()));
    }
  }
}

void wxIntegerItem::SetInteger(int p_value)
{
  m_value = p_value;
}


//------------------------------------------------------------------------
//                 class wxNumberItem: Member functions
//------------------------------------------------------------------------

static bool IsNumber(wxText &p_item)
{
  int i = 0;
  char *entry = p_item.GetValue();

  if (entry[i] == '-')  i = 1;
  for (; isdigit(entry[i]) && entry[i] != '\0'; i++);
  if (entry[i] == '\0')
    return true;
  else if (entry[i] == '.' || entry[i] == '/') {
    for (i++; isdigit(entry[i]) && entry[i] != '\0'; i++);
    return (entry[i] == '\0');
  }
  else
    return false;
}

//
// NB: This ctor does not check for valid number entries
//
wxNumberItem::wxNumberItem(wxPanel *p_parent, char *p_label,
			   const gText &p_default,
			   int p_x, int p_y, int p_width, int p_height)
  : wxText(p_parent, (wxFunction) EventCallback, p_label, "",
	   p_x, p_y, p_width, p_height), m_value(ToNumber(p_default))
{
  SetValue(p_default);
}

void wxNumberItem::EventCallback(wxNumberItem &p_item,
				 wxCommandEvent &p_event)
{
  if (p_event.eventType == wxEVENT_TYPE_TEXT_COMMAND) {
    if (!IsNumber(p_item)) {
      p_item.SetNumber(p_item.GetNumber());
      p_item.SetValue(ToText(p_item.GetNumber()));
    }
    else {
      p_item.SetNumber(ToNumber(p_item.GetValue()));
    }
  }
}

void wxNumberItem::SetNumber(const gNumber &p_value)
{
  m_value = p_value;
}

