//
// FILE: spreadcanvas.h -- SpreadSheetC implementation
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

#include "spread.h"
#include "spreadcanvas.h"
#include "spread3d.h"

// Global GDI objects
wxPen       *grid_line_pen;
wxPen       *grid_border_pen;
wxPen       *s_selected_pen;
wxPen       *s_hilight_pen;
wxPen       *s_white_pen;
wxBrush *s_white_brush;
wxBrush *s_hilight_brush;


SpreadSheetCanvas::SpreadSheetCanvas(SpreadSheet *_sheet, wxFrame *parent, 
                           int x, int y, int w, int h)
    : wxCanvas(parent, x, y, w, h, 0)
{
    top_frame = (SpreadSheet3D *)parent;
    sheet = _sheet;
    draw_settings = top_frame->DrawSettings();
    data_settings = top_frame->DataSettings();

    // Make sure that draw_settings knows about the current font size
    float tw, th;
    GetDataExtent(&tw, &th, "A quick greW");
    draw_settings->UpdateFontSize(tw/12, th);

    // Allow double clicking on canvas
    AllowDoubleClick(TRUE);

    // Give myself some scrollbars if necessary
    //CheckScrollbars();
    Show(FALSE);    // Do not update myself until told by the parent
}


int SpreadSheetCanvas::MaxX(int col)
{
    int temp = draw_settings->XStart();
    
    if (col < 0) 
        col = sheet->GetCols();

    for (int i = 1; i <= col; i++)  
        temp += draw_settings->GetColWidth(i);

    return temp;
}


int SpreadSheetCanvas::MaxY(int row)
{
    int temp = draw_settings->YStart();
    
    if (row < 0) 
        row = sheet->GetRows();

    temp += draw_settings->GetRowHeight() * row;

    return temp;
}


Bool SpreadSheetCanvas::XYtoRowCol(int x, int y, int *row, int *col)
{
  if (x < MaxX(0) || y < MaxY(0) || x > MaxX() || y > MaxY()) {
    return FALSE;
  }

  *row = (y-draw_settings->YStart()) / draw_settings->GetRowHeight()+1;
  *col = 0;
  int i = 1;

  while (*col == 0 && i <= sheet->GetCols())
    {
        if (x < MaxX(i)) 
            *col = i;

        i++;
    }
    
    if (*row < 1) 
        *row = 1;
    
    if (*row > sheet->GetRows()) 
        *row = sheet->GetRows();
    
    if (*col < 1) 
        *col = 1;
    
    if (*col > sheet->GetCols()) 
        *col = sheet->GetCols();

    return TRUE;
}


void SpreadSheetCanvas::OnSize(int _w, int _h)
{
    int h, w;
    GetVirtualSize(&w, &h);
    
    if (w == 0) 
        draw_settings->SetWidth(_w);
    else 
        draw_settings->SetWidth(w);
    
    if (h == 0) 
        draw_settings->SetHeight(_h);
    else 
        draw_settings->SetHeight(h);

    draw_settings->SetRealHeight(_h);
    draw_settings->SetRealWidth(_w);
}


// Desired Size
void    SpreadSheetCanvas::DesiredSize(int *w, int *h)
{
    *w = gmin(MaxX(), MAX_SHEET_WIDTH);
    *h = gmin(draw_settings->YStart() +
              (sheet->GetRows()+1) * draw_settings->GetRowHeight()+3, 
              MAX_SHEET_HEIGHT);
    *w = gmax(*w, MIN_SHEET_WIDTH);
    *h = gmax(*h, MIN_SHEET_HEIGHT);
}


// Check Scrollbars
void    SpreadSheetCanvas::CheckScrollbars(void)
{
    int x_step = -1, y_step = -1;
    
    if (MaxX() > MAX_SHEET_WIDTH)
        x_step = MaxX()/XSTEPS+5;
    
    if (draw_settings->YStart() + sheet->GetRows() * draw_settings->GetRowHeight() > 
        MAX_SHEET_HEIGHT)
        y_step = (draw_settings->YStart() + (sheet->GetRows() + 1) *
                  draw_settings->GetRowHeight()) / YSTEPS + 5;
    
    if (x_step > 0 || y_step > 0)
    {
        // Note that due to a bug in SetClientSize, we must either have no or both scrollbars
        if (x_step <= 0) 
            x_step = MaxX()/XSTEPS+5;
        
        if (y_step <= 0) 
            y_step = (draw_settings->YStart()+(sheet->GetRows()+1) *
                      draw_settings->GetRowHeight())/YSTEPS+5;
        
        if (x_step != draw_settings->XScroll() || y_step != draw_settings->YScroll())
        {
            ((wxCanvas *)this)->SetScrollbars(x_step, y_step, XSTEPS, YSTEPS, 4, 4);
            draw_settings->SetXScroll(x_step);
            draw_settings->SetYScroll(y_step);
            draw_settings->SetScrolling(TRUE);
        }
    }
    
    if (x_step < 0 && y_step < 0 && draw_settings->Scrolling())
    {
        x_step = 1;
        y_step = 1;
        SetScrollbars(x_step, y_step, XSTEPS, YSTEPS, 4, 4);
        draw_settings->SetXScroll(x_step);
        draw_settings->SetYScroll(y_step);
        draw_settings->SetScrolling(FALSE);
    }
    
}


// Paint message handler
void SpreadSheetCanvas::OnPaint(void)
{
    Update(*(GetDC()));
}

// Mouse message handler
void SpreadSheetCanvas::OnEvent(wxMouseEvent &ev)
{
    if (top_frame->OnEventNew(ev)) 
        return;
    
    if (ev.LeftDown() || ev.ButtonDClick())
    {
        float x, y;
        ev.Position(&x, &y);
        
        if (sheet->XYtoRowCol(x, y, &cell.row, &cell.col) == FALSE) 
            return;
        
        if (ev.LeftDown() && !ev.ControlDown()) 
            ProcessCursor(0);
        
        if (ev.ButtonDClick() || (ev.LeftDown() && ev.ControlDown()))
            top_frame->OnDoubleClick(cell.row, cell.col, 
                                     sheet->GetLevel(), 
                                     sheet->GetValue(cell.row, cell.col));
	top_frame->CanvasFocus();
    }
}


// Keyboard message handler
void SpreadSheetCanvas::OnChar(wxKeyEvent &ev)
{
    // Allow the default behavior to be overriden.
    if (top_frame->OnCharNew(ev)) 
        return;

    int ch = ev.KeyCode();

    // Ignore shift key.
    // Note: this will not affect the use of the shift key e.g. in
    // changing the case of a character.
    if (ch == WXK_SHIFT)
        return;

    // Cursor keys to move the highlight.
    if (IsCursor(ev) || IsEnter(ev))
    {
        ProcessCursor(ch);
        return;
    }

    // F2 on the last row adds a row.
    if (ch == WXK_F2)
    {
        if (data_settings->Change(S_CAN_GROW_ROW) && cell.row == sheet->GetRows()) 
            top_frame->AddRow();

        return;
    }

    // F3 on the last column adds a column.
    if (ch == WXK_F3)
    {
        if (data_settings->Change(S_CAN_GROW_COL) && cell.col == sheet->GetCols()) 
            top_frame->AddCol();

        return;
    }

    // Otherwise, if editing is enabled, just process the key.
    {
        if (top_frame->Editable())
        {
            gSpreadValType cell_type = sheet->GetType(cell.row, cell.col);
            
            if ((cell_type == gSpreadNum && IsNumeric(ev)) ||   
                (cell_type == gSpreadStr && IsAlphaNum(ev)))
            {
                if (cell.editing == FALSE)
                {
                    cell.editing = TRUE;

                    // Preserve the previously-existing color of the cell if any.
                    if (cell.str.Left(3) == "\\C{")
                    {
                        gText prefix = "\\C{";
                        // Append the color number.
                        for (int k = 3; k < cell.str.Length(); k++)
                        {
                            if (cell.str[k] != '}')
                            {
                                prefix += cell.str[k];
                            }
                            else // We found the color delimiter "}".
                            {
                                prefix += "}";
                                break;
                            }
                        }

                        cell.str = prefix;
                    }
                    else
                    {
                        cell.str = "";
                    }
                } // this implements 'overwrite'

                cell.str += (char) ch;
            }
            
            if (IsDelete(ev))
            {
                if (cell.editing == FALSE) 
                    cell.editing = TRUE;
                
                if (cell.str.Length()) 
                    cell.str.Remove(cell.str.Length()-1);
            }

            top_frame->SetStatusText(cell.str);

            // Update the character in place.
            sheet->SetValue(cell.row, cell.col, cell.str);
            UpdateCell(*(GetDC()), cell);
        }
    }
}


void SpreadSheetCanvas::ProcessCursor(int ch)
{
    if (cell.editing && ch != 0) 
        sheet->SetValue(cell.row, cell.col, cell.str);

    SpreadMoveDir how = SpreadMoveJump;

    switch (ch)
    {
    case    WXK_UP:
        if (cell.row > 1) {
            if (!sheet->GetSelectableRow(cell.row - 1))  return;
            cell.row--;
            how = SpreadMoveUp;
        }
        break;

    case    WXK_DOWN:
        if (cell.row < sheet->GetRows()) {
            if (!sheet->GetSelectableRow(cell.row + 1))  return;
            cell.row++;
            how = SpreadMoveDown;
        }
        break;

    case    WXK_RIGHT:
        if (cell.col < sheet->GetCols()) {
            if (!sheet->GetSelectableCol(cell.col + 1))  return;
            cell.col++;
            how = SpreadMoveRight;
        }
        break;

    case WXK_LEFT:
        if (cell.col > 1) {
            if (!sheet->GetSelectableCol(cell.col - 1))  return;
            cell.col--;
            how = SpreadMoveLeft;
        }
        break;

    default:
        break;
    }

    cell.Reset(sheet->GetValue(cell.row, cell.col));
    UpdateCell(*(GetDC()), cell);

    // MCV: commented this out; it shouldn't do anything anyway
    // but for some reason if it's uncommented it makes the
    // spreadsheet do weird things when you select the 
    // bottommost cell after scrolling (it jumps the view
    // back to the top and adds an extra row to the SS).

    top_frame->OnSelectedMoved(cell.row, cell.col, how);
    top_frame->SetStatusText(gPlainText(cell.str));

    // Make sure the cursor is visible by adjusting the scrollbar
    // position.  Don't adjust the scrollbars if this was a mouse
    // movement (ch = 0).
    if (draw_settings->Scrolling() && ch != 0)
    {
		Bool rescroll = FALSE;

        int cx, cy;  // Current x and y scroll positions.
        ViewStart(&cx, &cy);
        int x_scroll = cx;
		int y_scroll = cy;

		int cell_x_min = MaxX(cell.col - 1);  // Minimum x value of cell.
		int cell_x_max = MaxX(cell.col);      // Maximum x value of cell.
		int cell_y_min = MaxY(cell.row - 1);  // Minimum y value of cell.
		int cell_y_max = MaxY(cell.row);      // Maximum y value of cell.
        int x_scroll_width = draw_settings->XScroll();
		int y_scroll_width = draw_settings->YScroll();
		int window_x_min = x_scroll_width * cx;
		int window_x_max = window_x_min + draw_settings->GetRealWidth();
		int window_y_min = y_scroll_width * cy;
		int window_y_max = window_y_min + draw_settings->GetRealHeight();

		// Leave the scrollbars where they are if the new cell position
		// will fit within them.  Otherwise, position the scrollbar(s)
		// so that the new position of the cell is as far left (for 
		// movement in the X direction) or as far up (for movement in
		// the Y direction) as possible.

		if ((cell_x_min < window_x_min) || (cell_x_max >= window_x_max))
		{
			x_scroll = cell_x_min / x_scroll_width;
			rescroll = TRUE;
		}

		if ((cell_y_min < window_y_min) || (cell_y_max >= window_y_max))
		{
			y_scroll = cell_y_min / y_scroll_width;
			rescroll = TRUE;
		}

        if (rescroll)
        {
            Scroll(x_scroll, y_scroll);
        }
    }
    top_frame->CanvasFocus();
}


void SpreadSheetCanvas::UpdateCell(wxDC &dc, SpreadCell &cell)
{
    // Check for the validity of the cell/old_cell
    cell.CheckValid(sheet->GetRows(), sheet->GetCols());
    old_cell.CheckValid(sheet->GetRows(), sheet->GetCols());

    // erase the old hilight
    DrawCell(dc, old_cell.row, old_cell.col);

    // draw the new hilight
    dc.SetBrush(wxTRANSPARENT_BRUSH);
    dc.SetPen(s_selected_pen);
    dc.DrawRectangle(MaxX(cell.col-1)+LINE_OFF,
                     draw_settings->YStart()+(cell.row-1)*draw_settings->GetRowHeight()+LINE_OFF,
                     draw_settings->GetColWidth(cell.col)-2*LINE_OFF,
                     draw_settings->GetRowHeight()-2*LINE_OFF);

    // Update the status line text on the topmost frame
    top_frame->SetStatusText(gPlainText(cell.str));

    // Save the new cell
    old_cell = cell;
}


// Draw a cell
void SpreadSheetCanvas::DrawCell(wxDC &dc, int row, int col)
{
    dc.SetFont(draw_settings->GetDataFont());
    
    if (sheet->HiLighted(row, col))
    {
        dc.SetBrush(s_hilight_brush);
        dc.SetPen(s_hilight_pen);
    }
    else
    {
        dc.SetBrush(s_white_brush);
        dc.SetPen(s_white_pen);
    }
    
    if (sheet->Bold(row, col))
    {
        wxFont *cur = draw_settings->GetDataFont();
        dc.SetFont(wxTheFontList->FindOrCreateFont(cur->GetPointSize(), 
                                                   cur->GetFamily(),
                                                   cur->GetStyle(), 
                                                   wxBOLD, 
                                                   cur->GetUnderlined()));
    }

    dc.DrawRectangle(MaxX(col-1)+LINE_OFF,
                     draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+LINE_OFF,
                     draw_settings->GetColWidth(col)-2*LINE_OFF,
                     draw_settings->GetRowHeight()-2*LINE_OFF);

    dc.SetClippingRegion(MaxX(col-1)+TEXT_OFF,
                         draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF,
                         draw_settings->GetColWidth(col)-2*TEXT_OFF,
                         draw_settings->GetRowHeight()-2*TEXT_OFF);
    
    if (draw_settings->UseGText())  // use possibly colored text
    {
        gDrawText(dc, sheet->GetValue(row, col),
                  MaxX(col-1)+TEXT_OFF,
                  draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+
                  draw_settings->GetTextHeight()/4+TEXT_OFF);
    }
    else
    {
        gDrawText(dc, gPlainText(sheet->GetValue(row, col)),
                  MaxX(col-1)+TEXT_OFF,
                  draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
    }
    
    dc.DestroyClippingRegion();
}


// Updating
// Changed the code to only redraw the visible part of the window.  This
// should make updating large spreadsheets considerably faster.
void SpreadSheetCanvas::Update(wxDC &dc)
{
    int row, col;

    // Find the visible dimensions
    int min_row, max_row, min_col, max_col;
    int x_start, y_start;
    int width = draw_settings->GetRealWidth(), height = draw_settings->GetRealHeight();
    
    if (!height || !width) 
        return;

    ViewStart(&x_start, &y_start);
    x_start *= draw_settings->XScroll();
    y_start *= draw_settings->YScroll();
    min_row = (y_start-draw_settings->YStart())/draw_settings->GetRowHeight()+1;

    if (min_row < 1) 
        min_row = 1;

    max_row = min_row+height/draw_settings->GetRowHeight();
    
    if (max_row > sheet->GetRows()) 
        max_row = sheet->GetRows();

    min_col = 0;
    int i = 1;

    while (!min_col && i <= sheet->GetCols())
    {
        if (x_start < MaxX(i)) 
            min_col = i;

        i++;
    }
    
    if (min_col < 1) 
        min_col = 1;

    max_col = 0;
    i = 1;

    while (!max_col && i <= sheet->GetCols())
    {
        if (x_start+width < MaxX(i)) 
            max_col = i;

        i++;
    }
    
    if (max_col < 1) 
        max_col = sheet->GetCols();

    // Draw the grid
    char *dc_type = dc.GetClassInfo()->GetClassName();
    
    if (strcmp(dc_type, "wxMetaFileDC") != 0)
    {
        dc.Clear();
        dc.BeginDrawing();
    }
    
    if (strcmp(dc_type, "wxPostScriptDC") != 0) 
        dc.SetBackgroundMode(wxTRANSPARENT);

    dc.SetBrush(wxTRANSPARENT_BRUSH);
    dc.SetPen(grid_line_pen);

    for (row = min_row; row <= max_row; row++)
    {
        dc.DrawLine(draw_settings->XStart(), 
                    draw_settings->YStart()+row*draw_settings->GetRowHeight(),
                    MaxX(max_col)+1,
                    draw_settings->YStart()+row*draw_settings->GetRowHeight());
    }

    for (col = min_col; col <= max_col; col++)
    {
        dc.DrawLine(MaxX(col), 
                    draw_settings->YStart(),
                    MaxX(col),
                    draw_settings->YStart()+max_row*draw_settings->GetRowHeight()+1);
    }

    dc.SetPen(grid_border_pen);
    dc.SetBrush(wxTRANSPARENT_BRUSH); // draw bounding rectangle out of 4 lines
    
    if (min_col == 1)   // left
    {
        dc.DrawLine(draw_settings->XStart(), 
                    draw_settings->YStart(), 
                    draw_settings->XStart(),
                    max_row*draw_settings->GetRowHeight()+2+draw_settings->YStart());
    }
    
    if (max_col == sheet->GetCols()) // right
    {
        dc.DrawLine(MaxX(), 
                    draw_settings->YStart(), 
                    MaxX(),
                    max_row*draw_settings->GetRowHeight()+2+draw_settings->YStart());
    }
    
    if (min_row == 1) // top
    {
        dc.DrawLine(draw_settings->XStart(), 
                    draw_settings->YStart(), 
                    MaxX(max_col), 
                    draw_settings->YStart());
    }
    
    if (max_row == sheet->GetRows()) // bottom
    {
        dc.DrawLine(draw_settings->XStart(), 
                    max_row*draw_settings->GetRowHeight()+2+draw_settings->YStart(),
                    MaxX(), 
                    max_row*draw_settings->GetRowHeight()+2+draw_settings->YStart());
    }

    // Draw the labels if any (no sense in showing them if even the first row/col
    // are not visible
    dc.SetFont(draw_settings->GetLabelFont());
    
    if (draw_settings->RowLabels() && min_col == 1)
    {
        for (row = min_row; row <= max_row; row++)
            dc.DrawText(sheet->GetLabelRow(row), 0, 
                        draw_settings->YStart() + 
                        (row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
    }
    
    if (draw_settings->ColLabels() && min_row == 1)
    {
        for (col = min_col; col <= max_col; col++)
            dc.DrawText(sheet->GetLabelCol(col), MaxX(col-1)+TEXT_OFF, 0);
    }

    // Fill in the cells
    for (row = min_row; row <= max_row; row++)
        for (col = min_col; col <= max_col; col++)
            DrawCell(dc, row, col);
    
    // Hilight the currently selected cell
    UpdateCell(dc, cell);
    
    if (strcmp(dc_type, "wxMetaFileDC") != 0)
        dc.EndDrawing();
}

//****************************************************************************
//*                               SPREAD SHEET PRINTOUT
//****************************************************************************
#ifdef wx_msw
#include "wx_print.h"

class SpreadSheetPrintout: public wxPrintout
{
private:
    SpreadSheetCanvas *sheet;
    wxOutputOption fit;
    int num_pages;

public:
    SpreadSheetPrintout(SpreadSheetCanvas *s, wxOutputOption f, 
                        const char *title = "SpreadPrintout");
    Bool OnPrintPage(int page);
    Bool HasPage(int page);
    Bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


SpreadSheetPrintout::SpreadSheetPrintout(SpreadSheetCanvas *s, 
                                         wxOutputOption f, const char *title)
    : sheet(s), fit(f), wxPrintout((char *)title)
{ }

Bool SpreadSheetPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return FALSE;
    
    return TRUE;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void SpreadSheetPrintout::GetPageInfo(int *minPage, int *maxPage, 
                                      int *selPageFrom, int *selPageTo)
{
    num_pages = 1;
    *minPage = 0;
    *maxPage = num_pages;
    *selPageFrom = 0;
    *selPageTo = 0;
}


Bool SpreadSheetPrintout::HasPage(int pageNum)
{
    return (pageNum <= num_pages);
}


Bool SpreadSheetPrintout::OnPrintPage(int /*page*/)
{
    wxDC *dc = GetDC();
    
    if (!dc) 
        return FALSE;
    
    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    
    // Now we have to check in case our real page size is reduced
    // (e.g. because we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    float w, h;
    dc->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);
    float pageScaleX = (float)w/pageWidth;
    float pageScaleY = (float)h/pageHeight;
    dc->SetBackgroundMode(wxTRANSPARENT);
    
    if (!fit) // WYSIWYG
    {
        // This scales the DC so that the printout roughly represents the
        // the screen scaling. The text point size _should_ be the right size
        // but in fact is too small for some reason. This is a detail that will
        // need to be addressed at some point but can be fudged for the
        // moment.
        float scaleX = (float)((float)ppiPrinterX/(float)ppiScreenX);
        float scaleY = (float)((float)ppiPrinterY/(float)ppiScreenY);
        
        // If printer pageWidth == current DC width, then this doesn't
        // change. But w might be the preview bitmap width, so scale down.
        float overallScaleX = scaleX * (float)pageScaleX;
        float overallScaleY = scaleY * (float)pageScaleY;
        dc->SetUserScale(overallScaleX, overallScaleY);
        
        // Make the margins.  They are just 1" on all sides now.
        float marginX = 1*ppiPrinterX, marginY = 1*ppiPrinterY;
        dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
        
        sheet->draw_settings->SetRealWidth((pageWidth-2*marginX)/scaleX);
        sheet->draw_settings->SetRealHeight((pageHeight-2*marginY)/scaleY);
    }
    else    // FIT TO PAGE
    {
        float maxX = sheet->MaxX();
        float maxY = sheet->MaxY();
        
        // Make the margins.  They are just 1" on all sides now.
        float marginX = 1*ppiPrinterX;
        float marginY = 1*ppiPrinterY;
        
        // Calculate a suitable scaling factor
        float scaleX = (float)((pageWidth-2*marginX)/maxX)*pageScaleX;
        float scaleY = (float)((pageHeight-2*marginY)/maxY)*pageScaleY;
        
        // Use x or y scaling factor, whichever fits on the DC
        float actualScale = gmin(scaleX, scaleY);
        
        // Set the scale and origin
        dc->SetUserScale(actualScale, actualScale);
        dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
        
        // Let the spreadsheet know the new dimensions
        sheet->draw_settings->SetRealWidth(maxX);
        sheet->draw_settings->SetRealHeight(maxY);
        sheet->Scroll(0, 0);    // bad--should be set in draw_settings.
    }
    
    sheet->Update(*dc);
    
    return TRUE;
}
#endif


void SpreadSheetCanvas::Print(wxOutputMedia device, wxOutputOption fit)
{
    if (device == wxMEDIA_PRINTER)
    {
#ifdef wx_msw
        wxPrinter printer;
        SpreadSheetPrintout printout(this, fit);
        printer.Print(top_frame, &printout, TRUE);
#else
        wxMessageBox("Printing not supported under X");
#endif
    }
    
    if (device == wxMEDIA_PREVIEW)
    {
#ifdef wx_msw
        wxPrintPreview *preview = 
            new wxPrintPreview(new SpreadSheetPrintout(this, fit), 
                               new SpreadSheetPrintout(this, fit));
        wxPreviewFrame *frame =
            new wxPreviewFrame(preview, top_frame, "Print Preview", 
                               100, 100, 600, 650);
        frame->Centre(wxBOTH);
        frame->Initialize();
        frame->Show(TRUE);
#else
        wxMessageBox("Previewing not supported under X");
#endif
    }
    
    if (device == wxMEDIA_CLIPBOARD || device == wxMEDIA_METAFILE)
    {
#ifdef wx_msw
        // Make the metafile just 640x480 and scale the sheet accordingly
        char *metafile_name = 0;
        
        if (device == wxMEDIA_METAFILE)
        {
            metafile_name = 
                copystring(wxFileSelector("Save Metafile", 0, 0, ".wmf", "*.wmf"));
        }

        wxMetaFileDC dc_mf(metafile_name);
        
        if (dc_mf.Ok())
        {
            float real_scale = gmin(640/MaxX(), 480/MaxY());
            dc_mf.SetUserScale(real_scale, real_scale);
            draw_settings->SetRealWidth(MaxX());
            draw_settings->SetRealHeight(MaxY());
            Update(dc_mf);
            wxMetaFile *mf = dc_mf.Close();
            
            if (mf)
            {
                Bool success = mf->SetClipboard(MaxX(), MaxY());
                
                if (!success) 
                    wxMessageBox("Copy Failed", "Error", wxOK | wxCENTRE, this);

                delete mf;
            }
            
            if (device == wxMEDIA_METAFILE) 
                wxMakeMetaFilePlaceable(metafile_name, 0, 0, MaxX(), MaxY());
        }
#else
        wxMessageBox("Metafiles not supported under X");
#endif
    }
    
    if (device == wxMEDIA_PS)
    {
        wxPostScriptDC dc_ps(NULL, TRUE);
        
        if (dc_ps.Ok())
        {
            Bool gtext = draw_settings->UseGText();
            draw_settings->SetGText(FALSE);
            
            if (fit)
            {
                draw_settings->SetRealWidth(MaxX());
                draw_settings->SetRealHeight(MaxY());
            }

            dc_ps.StartDoc("");
            dc_ps.StartPage();
            Update(dc_ps);
            dc_ps.EndPage();
            dc_ps.EndDoc();
            draw_settings->SetGText(gtext);
        }
    }
}


