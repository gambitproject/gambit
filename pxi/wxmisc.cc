#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "wxmisc.h"

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

}

