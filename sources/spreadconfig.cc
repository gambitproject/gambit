//
// FILE: spreadconfig.cc -- spreadsheet class configuration
//
// $Id$
//

#include "wx.h"
#include "wx_mf.h"

#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif // wx_msw

#ifdef __BORLANDC__
#pragma hdr_stop
#endif // __BORLANDC__

#include "wxmisc.h"

#include "gambit.h"

#include "spread.h"
#include "spreadcanvas.h"
#include "spread3d.h"

//****************************************************************************
//*                              SPREAD SHEET DRAW SETTINGS                  *
//****************************************************************************
SpreadSheetDrawSettings::SpreadSheetDrawSettings(SpreadSheet3D *_parent, int cols)
    : col_width(cols)
{
    ref_cnt = 1;                                        
    parent = _parent;
    
    if (!LoadOptions())
    {
        row_height = DEFAULT_ROW_HEIGHT;
        default_col_width = DEFAULT_COL_WIDTH;
        col_dim_char = TRUE;
        labels = 0;
        vert_fit = TRUE;
        data_font = wxTheFontList->FindOrCreateFont(11, wxMODERN, wxNORMAL, wxNORMAL);
        label_font = wxTheFontList->FindOrCreateFont(12, wxMODERN, wxNORMAL, wxNORMAL);
        gtext = TRUE;
        num_prec = 3;
        SaveOptions();
    }
    
    for (int i = 1; i <= col_width.Length(); i++) col_width[i] = DEFAULT_COL_WIDTH;
    x_scroll = y_scroll = 0;
    scrolling = FALSE;
    x_start = -1;
    y_start = -1;
    ToTextPrecision(num_prec);
}

class dialogSpreadOptions : public guiAutoDialog {
private:
  wxButton *m_labelFontButton, *m_dataFontButton;
  wxSlider *m_horizSize, *m_vertSize, *m_decimals;
  wxChoice *m_columnList;
  wxCheckBox *m_horizFit, *m_vertFit;
  wxCheckBox *m_rowLabels, *m_colLabels, *m_colorLabels;
  SpreadSheetDrawSettings &m_drawSettings;

  static void CallbackLabelFont(wxButton &p_object, wxEvent &)
    { ((dialogSpreadOptions *) p_object.GetClientData())->OnLabelFont(); }
  static void CallbackDataFont(wxButton &p_object, wxEvent &)
    { ((dialogSpreadOptions *) p_object.GetClientData())->OnDataFont(); }

  void OnLabelFont(void);
  void OnDataFont(void);
  
public:
  dialogSpreadOptions(SpreadSheetDrawSettings &p_options,
		      wxWindow *p_parent);
  virtual ~dialogSpreadOptions() { }

  int SelectedColumn(void) const { return m_columnList->GetSelection(); }
  int ColumnWidth(void) const { return m_horizSize->GetValue(); }
  int RowHeight(void) const { return m_vertSize->GetValue(); }
  bool ColumnFit(void) const { return m_horizFit->GetValue(); }
  bool RowFit(void) const { return m_vertFit->GetValue(); }

  bool RowLabels(void) const { return m_rowLabels->GetValue(); }
  bool ColumnLabels(void) const { return m_colLabels->GetValue(); }
  bool ColorCodedLabels(void) const { return m_colorLabels->GetValue(); }

  int NumDecimals(void) const { return m_decimals->GetValue(); }
};

dialogSpreadOptions::dialogSpreadOptions(SpreadSheetDrawSettings &p_options,
					 wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Options"),
    m_drawSettings(p_options)
{
  wxGroupBox *fontGroup = new wxGroupBox(this, "Fonts", 1, 1);

  m_labelFontButton = new wxButton(this, (wxFunction) CallbackLabelFont,
				   "Label");
  m_labelFontButton->SetClientData((char *) this);
  m_dataFontButton = new wxButton(this, (wxFunction) CallbackDataFont, "Data");
  m_dataFontButton->SetClientData((char *) this);

  m_labelFontButton->SetConstraints(new wxLayoutConstraints);
  m_labelFontButton->GetConstraints()->left.SameAs(fontGroup, wxLeft, 10);
  m_labelFontButton->GetConstraints()->top.SameAs(fontGroup, wxTop, 20);
  m_labelFontButton->GetConstraints()->width.AsIs();
  m_labelFontButton->GetConstraints()->height.AsIs();

  m_dataFontButton->SetConstraints(new wxLayoutConstraints);
  m_dataFontButton->GetConstraints()->left.SameAs(m_labelFontButton,
						  wxRight, 10);
  m_dataFontButton->GetConstraints()->top.SameAs(m_labelFontButton, wxTop);
  m_dataFontButton->GetConstraints()->width.AsIs();
  m_dataFontButton->GetConstraints()->height.AsIs();

  fontGroup->SetConstraints(new wxLayoutConstraints);
  fontGroup->GetConstraints()->left.SameAs(this, wxLeft, 10);
  fontGroup->GetConstraints()->top.SameAs(this, wxTop, 10);
  fontGroup->GetConstraints()->right.SameAs(m_dataFontButton, wxRight, -10);
  fontGroup->GetConstraints()->bottom.SameAs(m_dataFontButton, wxBottom, -10);

  wxGroupBox *sizeGroup = new wxGroupBox(this, "Cell sizing");

  m_horizSize = new wxSlider(this, 0, "Column width", 
			     m_drawSettings.GetColWidth(), 0, 50, 250);
  m_horizSize->SetConstraints(new wxLayoutConstraints);
  m_horizSize->GetConstraints()->left.SameAs(sizeGroup, wxLeft, 10);
  m_horizSize->GetConstraints()->top.SameAs(sizeGroup, wxTop, 20);
  m_horizSize->GetConstraints()->width.AsIs();
  m_horizSize->GetConstraints()->height.AsIs();

  m_horizFit = new wxCheckBox(this, 0, "Fit");
  m_horizFit->SetValue(m_drawSettings.GetColFit());
  m_horizFit->SetConstraints(new wxLayoutConstraints);
  m_horizFit->GetConstraints()->left.SameAs(m_horizSize, wxRight, 10);
  m_horizFit->GetConstraints()->centreY.SameAs(m_horizSize, wxCentreY);
  m_horizFit->GetConstraints()->width.AsIs();
  m_horizFit->GetConstraints()->height.AsIs();

  m_columnList = new wxChoice(this, 0, "Column");
  m_columnList->Append("All");
  for (int i = 1; i <= p_options.NumColumns(); i++) {
    m_columnList->Append(ToText(i));
  } 
  m_columnList->SetSelection(0);
  m_columnList->SetConstraints(new wxLayoutConstraints);
  m_columnList->GetConstraints()->left.SameAs(m_horizFit, wxRight, 10);
  m_columnList->GetConstraints()->centreY.SameAs(m_horizFit, wxCentreY);
  m_columnList->GetConstraints()->width.AsIs();
  m_columnList->GetConstraints()->height.AsIs();

  m_vertSize = new wxSlider(this, 0, "Row height", 
			    m_drawSettings.GetRowHeight(), 0, 50, 250);
  m_vertSize->SetConstraints(new wxLayoutConstraints);
  m_vertSize->GetConstraints()->left.SameAs(m_horizSize, wxLeft);
  m_vertSize->GetConstraints()->top.SameAs(m_horizSize, wxBottom, 10);
  m_vertSize->GetConstraints()->width.AsIs();
  m_vertSize->GetConstraints()->height.AsIs();

  m_vertFit = new wxCheckBox(this, 0, "Fit");
  m_vertFit->SetValue(m_drawSettings.GetRowFit());
  m_vertFit->SetConstraints(new wxLayoutConstraints);
  m_vertFit->GetConstraints()->left.SameAs(m_vertSize, wxRight, 10);
  m_vertFit->GetConstraints()->centreY.SameAs(m_vertSize, wxCentreY);
  m_vertFit->GetConstraints()->width.AsIs();
  m_vertFit->GetConstraints()->height.AsIs();

  sizeGroup->SetConstraints(new wxLayoutConstraints);
  sizeGroup->GetConstraints()->left.SameAs(fontGroup, wxLeft);
  sizeGroup->GetConstraints()->top.SameAs(fontGroup, wxBottom, 10);
  sizeGroup->GetConstraints()->right.SameAs(m_columnList, wxRight, -10);
  sizeGroup->GetConstraints()->bottom.SameAs(m_vertSize, wxBottom, -10);

  wxGroupBox *labelsGroup = new wxGroupBox(this, "Show labels");

  m_rowLabels = new wxCheckBox(this, 0, "Rows");
  m_rowLabels->SetValue(m_drawSettings.RowLabels());
  m_rowLabels->SetConstraints(new wxLayoutConstraints);
  m_rowLabels->GetConstraints()->left.SameAs(labelsGroup, wxLeft, 10);
  m_rowLabels->GetConstraints()->top.SameAs(labelsGroup, wxTop, 20);
  m_rowLabels->GetConstraints()->width.AsIs();
  m_rowLabels->GetConstraints()->height.AsIs();

  m_colLabels = new wxCheckBox(this, 0, "Columns");
  m_colLabels->SetValue(m_drawSettings.ColLabels());
  m_colLabels->SetConstraints(new wxLayoutConstraints);
  m_colLabels->GetConstraints()->left.SameAs(m_rowLabels, wxRight, 10);
  m_colLabels->GetConstraints()->top.SameAs(m_rowLabels, wxTop);
  m_colLabels->GetConstraints()->width.AsIs();
  m_colLabels->GetConstraints()->height.AsIs();

  m_colorLabels = new wxCheckBox(this, 0, "Color Text");
  m_colorLabels->SetValue(m_drawSettings.UseGText());
  m_colorLabels->SetConstraints(new wxLayoutConstraints);
  m_colorLabels->GetConstraints()->left.SameAs(m_colLabels, wxRight, 10);
  m_colorLabels->GetConstraints()->top.SameAs(m_colLabels, wxTop);
  m_colorLabels->GetConstraints()->width.AsIs();
  m_colorLabels->GetConstraints()->height.AsIs();
  
  labelsGroup->SetConstraints(new wxLayoutConstraints);
  labelsGroup->GetConstraints()->left.SameAs(sizeGroup, wxLeft);
  labelsGroup->GetConstraints()->top.SameAs(sizeGroup, wxBottom, 10);
  labelsGroup->GetConstraints()->right.SameAs(m_colorLabels, wxRight, -10);
  labelsGroup->GetConstraints()->bottom.SameAs(m_colorLabels, wxBottom, -10);

  m_decimals = new wxSlider(this, 0, "Decimal Places", 2, 0, 25, 250);
  m_decimals->SetConstraints(new wxLayoutConstraints);
  m_decimals->GetConstraints()->left.SameAs(labelsGroup, wxLeft, 10);
  m_decimals->GetConstraints()->top.SameAs(labelsGroup, wxBottom, 10);
  m_decimals->GetConstraints()->width.AsIs();
  m_decimals->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_decimals, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

void dialogSpreadOptions::OnLabelFont(void)
{
  FontDialogBox dialog(NULL, m_drawSettings.GetLabelFont());
    
  if (dialog.Completed() == wxOK) {
    m_drawSettings.SetLabelFont(dialog.MakeFont());
  }
}

void dialogSpreadOptions::OnDataFont(void)
{
  FontDialogBox dialog(NULL, m_drawSettings.GetDataFont());
    
  if (dialog.Completed() == wxOK) {
    m_drawSettings.SetDataFont(dialog.MakeFont());
  }
}

void SpreadSheetDrawSettings::SetOptions(void)
{
  dialogSpreadOptions dialog(*this, parent);

  if (dialog.Completed() == wxOK) {
    unsigned int changed = 0; // what exactly has changed ...
    int which_col = dialog.SelectedColumn();
        
    if (which_col) 
      SetColWidth(dialog.ColumnWidth(), which_col-1);

    SetRowHeight(dialog.RowHeight());
    labels = 0;
        
    if (dialog.RowLabels()) 
      labels |= S_LABEL_ROW;
        
    if (dialog.ColumnLabels())
      labels |= S_LABEL_COL;
        
    gtext = dialog.ColorCodedLabels();
    vert_fit = dialog.RowFit();
    horiz_fit = dialog.ColumnFit();

    if (ToTextPrecision() != dialog.NumDecimals()) {
      ToTextPrecision(dialog.NumDecimals());
      changed |= S_PREC_CHANGED;
    }
        
    parent->OnOptionsChanged(changed);
  }

  parent->Redraw();
}

void    SpreadSheetDrawSettings::SaveOptions(const char *s)
{
    char *file_name;
    const char *sn = "SpreadSheet3D";   // section name
    file_name = copystring((s) ? s : (char *) gambitApp.ResourceFile());
    
    wxWriteResource(sn, "SpreadSheet3D-Version", 2, file_name);
    wxWriteResource(sn, "Row-Height", row_height, file_name);
    wxWriteResource(sn, "Default-Column-Width", default_col_width, file_name);
    wxWriteResource(sn, "Col-Dim-Char", col_dim_char, file_name);
    wxWriteResource(sn, "Fit-Text-Vert", vert_fit, file_name);
    wxWriteResource(sn, "Fit-Text-Horiz", horiz_fit, file_name);
    wxWriteResource(sn, "Show-Labels", show_labels, file_name);
    wxWriteResource(sn, "Data-Font", wxFontToString(data_font), file_name);
    wxWriteResource(sn, "Label-Font", wxFontToString(label_font), file_name);
    wxWriteResource(sn, "Use-GText", gtext, file_name);
    wxWriteResource("Gambit", "Output-Precision", num_prec, file_name);
    delete [] file_name;
}


int SpreadSheetDrawSettings::LoadOptions(const char *s)
{
    const char *sn = "SpreadSheet3D";   // section name
    const char *file_name = (s) ? s : (char *) gambitApp.ResourceFile();
    
    char *font_str = new char[100];
    int version = 0;
    wxGetResource(sn, "SpreadSheet3D-Version", &version, file_name);
    
    if (!version) 
        return 0;

    wxGetResource(sn, "Row-Height", &row_height, file_name);
    wxGetResource(sn, "Default-Column-Width", &default_col_width, file_name);
    wxGetResource(sn, "Col-Dim-Char", &col_dim_char, file_name);
    wxGetResource(sn, "Fit-Text-Vert", &vert_fit, file_name);
    wxGetResource(sn, "Fit-Text-Horiz", &horiz_fit, file_name);
    wxGetResource(sn, "Show-Labels", &show_labels, file_name);
    wxGetResource(sn, "Data-Font", &font_str, file_name);
    data_font = wxStringToFont(font_str);
    wxGetResource(sn, "Label-Font", &font_str, file_name);
    label_font = wxStringToFont(font_str);
    wxGetResource(sn, "Use-GText", &gtext, file_name);
    wxGetResource("Gambit", "Output-Precision", &num_prec, file_name);
    
    return 1;
}


// Column width
int SpreadSheetDrawSettings::GetColWidth(int col)
{
    if (!col) 
        col = 1;
    else 
        assert(col >= 1 && col <= col_width.Length() && "ColWidth::Invalid");
    
    if (horiz_fit)
        return col_width[col]*tw+2*TEXT_OFF;
    else
        return col_width[col]*COL_WIDTH_UNIT;
}

int SpreadSheetDrawSettings::GetColWidthRaw(int p_column) const
{
  return col_width[p_column];
}

void SpreadSheetDrawSettings::SetColWidth(int _c, int col)
{
    if (col) 
    {
        col_width[col] = _c;
    }
    else
    {
        for (int i = 1; i <= col_width.Length(); i++) 
            col_width[i] = _c;
    }
}


// Font Size.  If the cell size is tied to the font size, i.e. if
// vert_fit is used, or if the cell width is measured in chars, we
// need to update these values every time the font changes
void SpreadSheetDrawSettings::UpdateFontSize(float x, float y)
{
    if (x < 0 && y < 0)
    {
        parent->GetDataExtent(&x, &y, "A quick greW");
        x /= 12;
    }

    tw = (int)x;
    th = (int)y;
}


// We need to know how many columns there are ...
void SpreadSheetDrawSettings::SetDimensions(int /*rows*/, int cols)
{
    int old_cols = col_width.Length();

    if (cols > old_cols)
    {
        for (int i = 1; i <= cols-old_cols; i++) 
            AddCol();
    }

    if (cols < col_width.Length())
    {
        for (int i = old_cols; i <= cols; i--) 
            col_width.Remove(i);
    }
}



//****************************************************************************
//*                             SPREAD SHEET DATA SETTINGS                   *
//****************************************************************************
void SpreadSheetDataSettings::SetAutoLabelStr(const gText s, int what)
{
    switch (what)
    {
    case S_AUTO_LABEL_ROW:  
        auto_label_row = s;
        break;

    case S_AUTO_LABEL_COL:  
        auto_label_col = s;
        break;

    case S_AUTO_LABEL_LEVEL:    
        auto_label_level = s;
        break;
    }
}


gText   SpreadSheetDataSettings::AutoLabelStr(int what) const
{
    gText label;
    switch (what)
    {
    case S_AUTO_LABEL_ROW:  
        label = auto_label_row;
        break;

    case S_AUTO_LABEL_COL:  
        label = auto_label_col;
        break;

    case S_AUTO_LABEL_LEVEL:    
        label = auto_label_level;
        break;

    default: 
        label = "";
        break;
    }

    return label;
}

