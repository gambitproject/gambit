//
// FILE: spread3d.cc -- SpreadSheet3D implementation
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

#include "spread3d.h"
#include "spreadcanvas.h"
#include "spread.h"

extern wxPen *grid_line_pen, *grid_border_pen;
extern wxPen *s_selected_pen, *s_hilight_pen, *s_white_pen;
extern wxBrush *s_white_brush, *s_hilight_brush;

SpreadSheet3D::SpreadSheet3D(int rows, int cols, int _levels, int status,
			     char *title,
                             wxFrame *parent, unsigned int _features, 
                             SpreadSheetDrawSettings *drs,
                             SpreadSheetDataSettings *dts)
    : wxFrame(parent, title)
{
    assert(rows > 0 && cols > 0 && _levels > 0 && "SpreadSheet3D::Bad Dimensions");

    // Initialize some global GDI objects
    grid_line_pen   = wxThePenList->FindOrCreatePen("BLACK", 1, wxDOT);
    grid_border_pen = wxThePenList->FindOrCreatePen("BLUE", 3, wxSOLID);
    s_selected_pen  = wxThePenList->FindOrCreatePen("GREEN", 2, wxSOLID);
    s_white_pen     = wxThePenList->FindOrCreatePen("WHITE", 2, wxSOLID);
    s_hilight_pen   = wxThePenList->FindOrCreatePen("LIGHT GREY", 2, wxSOLID);
    s_white_brush   = wxTheBrushList->FindOrCreateBrush("WHITE", wxSOLID);
    s_hilight_brush = wxTheBrushList->FindOrCreateBrush("LIGHT GREY", wxSOLID);

    // Initialize the draw settings
    draw_settings = (drs) ? drs : new SpreadSheetDrawSettings(this, cols);
    draw_settings->SetParent(this);
    data_settings = (dts) ? dts : new SpreadSheetDataSettings;

    // Initialize local variables
    toolbar   = 0;
    completed = wxRUNNING;
    editable  = TRUE;
    levels    = _levels;
    label     = title;
    features  = _features;

    // Create the levels,  must do in two steps since gList(int) is not defined
    int i;

    for (i = 1; i <= levels; i++) 
      data.Append(new SpreadSheet());

    for (i = 1; i <= levels; i++) 
      data[i]->Init(rows, cols, i, 0, this);

    // Turn on level #1
    cur_level = 0;
    SetLevel(1);
    
    if (levels > 1) 
        features |= ANY_BUTTON; // we need a panel for the slider

    MakeFeatures();
    CreateStatusLine(status);

    // Size this frame according to the sheet dimensions
    Resize();
}


SpreadSheet3D::~SpreadSheet3D(void)
{
    Show(FALSE);
    
    if (toolbar)
    {
        delete toolbar;
        toolbar = 0;
    }
    
    if (panel)
    {
        delete panel;
        panel = 0;
    }
    
    if (--draw_settings->ref_cnt == 0) 
        delete draw_settings;
    
    if (--data_settings->ref_cnt == 0) 
        delete data_settings;
}


void SpreadSheet3D::MakeFeatures(void)
{
    panel = 0;
    MakeButtons(features&ALL_BUTTONS);
    SetMenuBar(MakeMenuBar(features&ALL_MENUS));
}


void SpreadSheet3D::MakeButtons(long buttons)
{
    //------------------make the panel---------------------------
    if (buttons) // Create the panel
    {
        if (!panel)
        {
            int h, w;
            panel_x = panel_y = 0;
            panel_new_line = FALSE;
            GetClientSize(&w, &h);
            panel = new wxPanel(this, 0, h-MIN_BUTTON_SPACE, w, 
                                MIN_BUTTON_SPACE, wxBORDER);
        }
        
        if (levels > 1 && !level_item) // create a slider to choose the active level
        {
            level_item = new wxSlider(panel, 
                                      (wxFunction)SpreadSheet3D::spread_slider_func, 
                                      NULL, 1, 1, levels, 140);
            level_item->SetClientData((char *)this);
            panel->NewLine();
            SavePanelPos();
        }
        
        if (buttons & OK_BUTTON) 
            AddButton("OK", (wxFunction)SpreadSheet3D::spread_ok_func);
        
        if (buttons & CANCEL_BUTTON) 
            AddButton("Cancel", (wxFunction)SpreadSheet3D::spread_cancel_func);
        
        if (buttons & PRINT_BUTTON) 
            AddButton("P", (wxFunction)SpreadSheet3D::spread_print_func);
        
        if (buttons & OPTIONS_BUTTON) 
            AddButton("Config", (wxFunction)SpreadSheet3D::spread_options_func);
        
        if (buttons & HELP_BUTTON) 
            AddButton("?", (wxFunction)SpreadSheet3D::spread_help_func);
        
    }
}


void SpreadSheet3D::SetDimensions(int rows_, int cols_, int levels_)
{
    assert(rows_ > 0 && cols_ > 0 && "SpreadSheet3D::Invalid Dimensions");
    int i;
    
    if (GetRows() != rows_ || GetCols() != cols_)
    {
        for (i = 1; i <= levels; i++) 
            data[i]->SetDimensions(rows_, cols_);

        DrawSettings()->SetDimensions(rows_, cols_);
    }
    
    if (levels_)
    {
        if (levels_ > levels)
        {
            for (i = 1; i <= levels_-levels; i++) 
                AddLevel();
        }
        
        if (levels_ < levels)
        {
            for (i = 1; i <= levels-levels_; i++) 
                DelLevel();
        }
    }
}


void SpreadSheet3D::OnMenuCommand(int id)
{
    switch (id)
    {
    case OUTPUT_MENU: 
        OnPrint();
        break;

    case CLOSE_MENU: 
        OnOk();
        break;

    case OPTIONS_MENU: 
        DrawSettings()->SetOptions();
        break;

    case CHANGE_MENU: 
        break;

    case HELP_MENU_ABOUT: 
        OnHelp(HELP_MENU_ABOUT);
        break;

    case HELP_MENU_CONTENTS: 
        OnHelp();
        break;

    default: 
        wxMessageBox("Unknown");
        break;
    }
}


void SpreadSheet3D::OnSize(int , int )
{
    int w, h;
    int toolbar_height = (toolbar) ? 40 : 0;
    GetClientSize(&w, &h);
    
    if (toolbar) 
        toolbar->SetSize(0, 0, w, toolbar_height);
    
    if (panel) 
        panel->SetSize(0, h-DrawSettings()->PanelSize(), 
                       w, DrawSettings()->PanelSize());

    for (int i = 1; i <= levels; i++) {
      data[i]->SetSize(0, toolbar_height, w, 
		      h-DrawSettings()->PanelSize()-toolbar_height);
      data[i]->CheckSize();
    }
}


// Callback functions
void SpreadSheet3D::spread_ok_func(wxButton  &ob, wxEvent &)
{
    ((SpreadSheet3D *)ob.GetClientData())->OnOk1();
}


void SpreadSheet3D::OnOk1(void)
{
    OnOk();
}


void SpreadSheet3D::OnOk(void)
{
    SetCompleted(wxOK);
    Show(FALSE);
}


void    SpreadSheet3D::OnCancel1(void)
{
    OnCancel();
}


void    SpreadSheet3D::OnCancel(void)
{
    SetCompleted(wxCANCEL);
    Show(FALSE);
}


void SpreadSheet3D::OnPrint1(void)
{
    OnPrint();
}


void SpreadSheet3D::OnPrint(void)
{
        wxStringList extras("ASCII", NULL);
        wxOutputDialogBox od(&extras);

        if (od.Completed() == wxOK)
        {

            if (!od.ExtraMedia())
            {
                Print(od.GetMedia(), od.GetOption());
            }
            else    // only one extra exists--must be ascii.
            {
                char *s = wxFileSelector("Save", NULL, NULL, NULL, "*.asc", wxSAVE);

                if (s) {
		  gFileOutput out(s);
		  data[cur_level]->Output(out);
                }
            }
        }
}



void    SpreadSheet3D::spread_print_func(wxButton   &ob, wxEvent &)
{
    ((SpreadSheet3D *)ob.GetClientData())->OnPrint1();
}


void    SpreadSheet3D::spread_cancel_func(wxButton  &ob, wxEvent &)
{
    ((SpreadSheet3D *)ob.GetClientData())->OnCancel1();
}


void SpreadSheet3D::spread_slider_func(wxSlider &ob, wxCommandEvent &)
{
    ((SpreadSheet3D *)ob.GetClientData())->SetLevel(ob.GetValue());
}


void    SpreadSheet3D::spread_help_func(wxButton    &ob, wxEvent &)
{
    ((SpreadSheet3D *)ob.GetClientData())->OnHelp1();
}


void SpreadSheet3D::OnHelp1(void)
{
    OnHelp();
}


void SpreadSheet3D::OnHelp(int )
{ }

void SpreadSheet3D::CanvasFocus(void)
{
  data[cur_level]->SetFocus();
}

void SpreadSheet3D::SetType(int row, int col, gSpreadValType t)
{
  data[cur_level]->SetType(row, col, t);
}

gSpreadValType SpreadSheet3D::GetType(int row, int col) 
{
  return data[cur_level]->GetType(row, col); 
}

void SpreadSheet3D::SetType(int row, int col, int level, gSpreadValType t) 
{ data[level]->SetType(row, col, t); }

gSpreadValType SpreadSheet3D::GetType(int row, int col, int level) 
{ return data[level]->GetType(row, col); }

SpreadSheet &SpreadSheet3D::operator[](int i)
{
  assert(i > 0 && i <= levels);
  return *data[i];
}

void SpreadSheet3D::SetCell(int row, int col, const gText &s)
{ data[cur_level]->SetValue(row, col, s); }

const gText &SpreadSheet3D::GetCell(int row, int col) const
{ return data[cur_level]->GetValue(row, col); }

void SpreadSheet3D::SetCell(int row, int col, int level, const gText &s)
{ 
  assert(level > 0 && level <= levels);
  data[level]->SetValue(row, col, s);
}

const gText &SpreadSheet3D::GetCell(int row, int col, int level) const
{ 
  assert(level > 0 && level <= levels);
  return data[level]->GetValue(row, col);
}

void SpreadSheet3D::Clear(int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->Clear();
}

int SpreadSheet3D::CurRow(int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->CurRow();
}

int SpreadSheet3D::CurCol(int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->CurCol();
}

void SpreadSheet3D::SetCurRow(int r, int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->SetCurRow(r);
}

void SpreadSheet3D::SetCurCol(int c, int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->SetCurCol(c);
}

Bool SpreadSheet3D::EnteredCell(int row, int col, int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->EnteredCell(row, col);
}

void SpreadSheet3D::SetLabelLevel(const gText &s, int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->SetLabel(s);
}

void SpreadSheet3D::SetLabelRow(const gBlock<gText> &vs, int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->SetLabelRow(vs);
}

void SpreadSheet3D::SetLabelCol(const gBlock<gText> &vs, int level)
{
  if (level == 0) 
    level = cur_level;
  data[level]->SetLabelCol(vs);
}

gText SpreadSheet3D::GetLabelRow(int row, int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->GetLabelCol(row);
}

gText SpreadSheet3D::GetLabelCol(int col, int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->GetLabelRow(col);
}

gText SpreadSheet3D::GetLabelLevel(int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->GetLabel();
}

Bool SpreadSheet3D::HiLighted(int row, int col, int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->HiLighted(row, col);
}

void SpreadSheet3D::HiLighted(int row, int col, int level, Bool _e)
{
  if (level == 0) 
    level = cur_level;
  data[level]->HiLighted(row, col, _e);
}

Bool SpreadSheet3D::Bold(int row, int col, int level)
{
  if (level == 0) 
    level = cur_level;
  return data[level]->Bold(row, col);
}

void SpreadSheet3D::Bold(int row, int col, int level, Bool _e)
{
  if (level == 0) 
    level = cur_level;
  data[level]->Bold(row, col, _e);
}

void SpreadSheet3D::GetDataExtent(float *x, float *y, const char *str)
{ data[cur_level]->GetDataExtent(x, y, str); }

Bool SpreadSheet3D::XYtoRowCol(int x, int y, int *row, int *col) 
{ return data[cur_level]->XYtoRowCol(x, y, row, col); }

void SpreadSheet3D::Repaint(void)
{ data[cur_level]->Repaint(); }

void SpreadSheet3D::Print(wxOutputMedia device, wxOutputOption fit) 
{ data[cur_level]->Print(device, fit); }

int SpreadSheet3D::GetRows(void)
{ return data[cur_level]->GetRows(); }

int SpreadSheet3D::GetCols(void)
{ return data[cur_level]->GetCols(); }

void SpreadSheet3D::spread_options_func(wxButton &ob, wxEvent &)
{
    SpreadSheet3D *parent = (SpreadSheet3D *)ob.GetClientData();
    parent->DrawSettings()->SetOptions();
}


void SpreadSheet3D::DelLevel(void)
{
    if (levels < 2) 
        return;

    delete data.Remove(levels);
    levels--;
    delete level_item;
    level_item = new wxSlider(panel, 
                              (wxFunction)SpreadSheet3D::spread_slider_func, 
                              NULL, 1, 1, levels, 140);
    level_item->SetClientData((char *)this);
    Redraw();
}


void SpreadSheet3D::AddLevel(int level)
{
    if (level == 0) 
        level = levels+1;

    data.Insert(new SpreadSheet(), level);
    levels++;
    data[level]->Init(data[1]->GetRows(), data[1]->GetCols(), levels, NULL, this);
    level_item = new wxSlider(panel, 
                              (wxFunction)SpreadSheet3D::spread_slider_func, 
                              NULL, 1, 1, levels, 140);
    level_item->SetClientData((char *)this);
    Redraw();
}


void SpreadSheet3D::Output(void)
{
  gFileOutput out("spread.out");
  out << levels << "\n";

  for (int i = 1; i <= levels; i++) {
    data[i]->Output(out);
    out << "\n\n";
  }
}


void SpreadSheet3D::SetLevel(int _l)
{
    assert(_l > 0 && _l <= levels);
    
    if (cur_level) 
        data[cur_level]->SetActive(FALSE);

    cur_level = _l;
    data[cur_level]->SetActive(TRUE);
    SetTitle(label + ":" + data[cur_level]->GetLabel());
}


void SpreadSheet3D::SetLabelRow(int row, const gText &s, int level)
{
    if (level == 0)
    {
        for (level = 1; level <= levels; level++) 
            data[level]->SetLabelRow(row, s);
    }
    else
        data[level]->SetLabelRow(row, s);
}


void SpreadSheet3D::SetLabelCol(int col, const gText &s, int level)
{
    if (level == 0)
    {
        for (level = 1; level <= levels; level++)
            data[level]->SetLabelCol(col, s);
    }
    else
        data[level]->SetLabelCol(col, s);
}


void SpreadSheet3D::FitLabels(void)
{
    float w, h;
    int max_w = -1, max_h = -1;
    int i;
    
    if (draw_settings->RowLabels())
    {
        for (i = 1; i <= data[1]->GetRows(); i++)
        {
            data[1]->GetLabelExtent(data[cur_level]->GetLabelRow(i), &w, &h);
            
            if (w > max_w) 
                max_w = (int)w;
        }

        draw_settings->SetXStart(max_w+3);
    }
    
    if (draw_settings->ColLabels())
    {
        for (i = 1; i <= data[1]->GetCols(); i++)
        {
            data[1]->GetLabelExtent(data[cur_level]->GetLabelCol(i), &w, &h);
            
            if (h > max_h) 
                max_h = (int)h;
        }

        draw_settings->SetYStart(max_h+3);
    }
}


void SpreadSheet3D::Resize(void)
{
    int w, h, w1 = 0, h1 = 0;
    int toolbar_height = (toolbar) ? 40 : 0;
    
    data[cur_level]->GetSize(&w, &h);
    
    if (panel) {
      Panel()->Fit();
      Panel()->GetSize(&w1, &h1);
      w = gmax(w, w1);
      h1 = gmax(h1, MIN_BUTTON_SPACE);
      Panel()->SetSize(0, h+toolbar_height, w, h1);
    }

    DrawSettings()->SetPanelSize(h1);
    SetClientSize(w, h+h1+toolbar_height);
}


void SpreadSheet3D::Redraw(void)
{
    char tmp[100];
    int i;
    
    if (data_settings->AutoLabel(S_AUTO_LABEL_ROW))
    {
        for (i = 1; i <= data[1]->GetRows(); i++)
        {
            sprintf(tmp, data_settings->AutoLabelStr(S_AUTO_LABEL_ROW), i);
            SetLabelRow(i, tmp);
        }
    }
    
    if (data_settings->AutoLabel(S_AUTO_LABEL_COL))
    {
        for (i = 1; i <= data[1]->GetCols(); i++)
        {
            sprintf(tmp, data_settings->AutoLabelStr(S_AUTO_LABEL_COL), i);
            SetLabelCol(i, tmp);
        }
    }

    FitLabels();
    Resize();
}


wxButton *SpreadSheet3D::AddButton(const char *label, wxFunction fun)
{
#ifdef wx_msw
    assert(panel);
    panel->RealAdvanceCursor();
    SavePanelPos();
    
    if (panel_new_line)
    {
        panel_y += 40;
        panel_x = PANEL_LEFT_MARGIN;
        panel_new_line = FALSE;
    }

    wxButton *button = new wxButton(panel, fun, (char *)label, panel_x, panel_y);
#else
    
    if (panel_new_line)
    {
        panel->NewLine();
        panel_new_line = FALSE;
    }

    wxButton *button = new wxButton(panel, fun, (char *)label);
#endif
    button->SetClientData((char *)this);
    return button;
}


wxPanel *SpreadSheet3D::AddPanel(void)
{
#ifdef wx_msw
    panel->RealAdvanceCursor();
    SavePanelPos();
    
    if (panel_new_line)
    {
        panel_y += 40;
        panel_x = PANEL_LEFT_MARGIN;
        panel_new_line = FALSE;
    }

    wxPanel *sub_panel = new wxPanel(panel, panel_x, panel_y);
    return sub_panel;
#else
    //if (panel_new_line) panel->NewLine();
    //wxPanel *sub_panel = new wxPanel(panel);
    panel->NewLine();
    return panel;
#endif
}


/*
  void SpreadSheet3D::AddMenu(wxMenu *submenu, const char *label)
  {
  assert(menubar);      // make sure a menubar exists
  menubar->Append(submenu, (char *)label);
  SetMenuBar(menubar);
  }
*/


wxMenuBar *SpreadSheet3D::MakeMenuBar(long menus)
{
    wxMenuBar *tmp_menubar = 0;

    //-------------------------------make menus----------------------------
    if (menus)
    {
        tmp_menubar = new wxMenuBar;
        wxMenu *file_menu = 0;
        
        if (menus & (OUTPUT_MENU | CLOSE_MENU)) 
            file_menu = new wxMenu;
        
        if (menus & OUTPUT_MENU)
            file_menu->Append(OUTPUT_MENU, "Out&put", "Output to any device");
        
        if (menus & CLOSE_MENU)
            file_menu->Append(CLOSE_MENU, "&Close", "Exit");
        
        if (file_menu) 
            tmp_menubar->Append(file_menu, "&File");

        wxMenu *display_menu = 0;
        
        if (menus & (OPTIONS_MENU | CHANGE_MENU)) 
            display_menu = new wxMenu;

        if (menus & OPTIONS_MENU)
            display_menu->Append(OPTIONS_MENU, "&Options", "Configure display options");

        if (menus & CHANGE_MENU)
            display_menu->Append(CHANGE_MENU, "&Change", "Change sheet dimensions");

        if (display_menu) 
            tmp_menubar->Append(display_menu, "&Display");

        if (menus & HELP_MENU)
        {
            wxMenu *help_menu = new wxMenu;
            help_menu->Append(HELP_MENU_ABOUT, "&About");
            help_menu->Append(HELP_MENU_CONTENTS, "&Contents");
            tmp_menubar->Append(help_menu, "&Help");
        }
    }

    return tmp_menubar;
}


void SpreadSheet3D::SetMenuBar(wxMenuBar *bar)
{
    menubar = bar;

    if (menubar) 
        wxFrame::SetMenuBar(menubar);
}


void SpreadSheet3D::OnSelectedMoved(int , int , SpreadMoveDir )
{ }


void SpreadSheet3D::AddRow(int p_row /*= 0*/)
{
  for (int i = 1; i <= levels; i++)
    data[i]->AddRow(p_row);
}

void SpreadSheet3D::AddCol(int p_col /*= 0*/)
{
  for (int i = 1; i <= levels; i++)
    data[i]->AddCol(p_col);
  DrawSettings()->AddCol(p_col);
}

void SpreadSheet3D::DelRow(int p_row /*= 0*/)
{
  for (int i = 1; i <= levels; i++)
    data[i]->DelRow(p_row);
}

void SpreadSheet3D::DelCol(int p_col /*= 0*/)
{
  for (int i = 1; i <= levels; i++)
    data[i]->DelCol(p_col);
  DrawSettings()->DelCol(p_col);
}

void SpreadSheet3D::SetSelectableRow(int p_row, Bool p_selectable)
{
  for (int i = 1; i <= levels; i++)
    data[i]->SetSelectableRow(p_row, p_selectable);
}

void SpreadSheet3D::SetSelectableCol(int p_col, Bool p_selectable)
{
  for (int i = 1; i <= levels; i++)
    data[i]->SetSelectableCol(p_col, p_selectable);
}

gOutput &operator<<(gOutput &op, const SpreadSheet3D &)
{
    return op;
}

