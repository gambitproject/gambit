///////////////////////////////////////////////////////////////////////////////
// Name:        sheetren.cpp
// Purpose:     wxSheet controls
// Author:      Paul Gammans, Roger Gammans
// Modified by: John Labenski
// Created:     11/04/2001
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/utils.h"    
    #include "wx/dc.h"
    #include "wx/dcclient.h"    
    #include "wx/log.h"
    #include "wx/valtext.h"
    #include "wx/settings.h"    
    #include "wx/checkbox.h"
    #include "wx/intl.h"
#endif // WX_PRECOMP

#include "wx/sheet/sheet.h"
#include "wx/sheet/sheetren.h"
#include "wx/tokenzr.h"
#include "wx/renderer.h"
// Required for wxIs... functions
#include <cctype>

#define PRINT_RECT(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());


#if defined(__WXMOTIF__)
    #define WXUNUSED_MOTIF(identifier)  WXUNUSED(identifier)
#else
    #define WXUNUSED_MOTIF(identifier)  identifier
#endif // defined(__WXMOTIF__)

#if defined(__WXGTK__)
    #define WXUNUSED_GTK(identifier)    WXUNUSED(identifier)
#else
    #define WXUNUSED_GTK(identifier)    identifier
#endif // defined(__WXGTK__)

const wxSheetCellRenderer wxNullSheetCellRenderer;

/* XPM */
static const char *s_overflow_arrow_xpm_data[] = {
/* columns rows colors chars-per-pixel */
"4 6 2 1",
"  c None",
"a c Black",
/* pixels */
"aa  ",
"aaa ",
"aaaa",
"aaaa",
"aaa ",
"aa  "};

static wxBitmap s_overflowBitmap; // a bitmap to indicate cell overflows

IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRenderer, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRendererRefData, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellStringRendererRefData, wxSheetCellRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellNumberRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellFloatRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBitmapRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBoolRendererRefData, wxSheetCellRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellDateTimeRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEnumRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAutoWrapStringRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRolColLabelRendererRefData, wxSheetCellStringRendererRefData)

// ----------------------------------------------------------------------------
// wxSheetCellRenderer
// ----------------------------------------------------------------------------
#define M_CELLRENREFDATA ((wxSheetCellRendererRefData*)m_refData)

wxSheetCellRenderer::wxSheetCellRenderer(wxSheetCellRendererRefData *renderer)
{ 
    m_refData = renderer; 
}

void wxSheetCellRenderer::Draw(wxSheet& sheet, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxRect& rect,
                               const wxSheetCoords& coords, bool isSelected)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellRenderer is not created"));    
    M_CELLRENREFDATA->Draw(sheet, attr, dc, rect, coords, isSelected);
}

wxSize wxSheetCellRenderer::GetBestSize(wxSheet& sheet, const wxSheetCellAttr& attr,
                                        wxDC& dc, const wxSheetCoords& coords) 
{ 
    wxCHECK_MSG(Ok(), sheet.GetCellSize(coords), wxT("wxSheetCellRenderer is not created"));
    return M_CELLRENREFDATA->GetBestSize(sheet, attr, dc, coords);
}

void wxSheetCellRenderer::SetParameters(const wxString& params)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellRenderer is not created"));
    M_CELLRENREFDATA->SetParameters(params);
}

bool wxSheetCellRenderer::Copy(const wxSheetCellRenderer& other)
{
    wxCHECK_MSG(other.Ok(), false, wxT("wxSheetCellRenderer is not created"));
    
    UnRef();
    m_refData = ((wxSheetCellRendererRefData*)other.m_refData)->Clone();
    return true;
}

// ----------------------------------------------------------------------------
// wxSheetCellRendererRefData
// ----------------------------------------------------------------------------

// This is testing code to show the different selection blocks with different colours
//#define TEST_SELECTION_BLOCKS

#ifdef TEST_SELECTION_BLOCKS
    #include "wx/sheet/sheetsel.h"
    #include "math.h"
    #define UNUSE(a) a
    wxColour GetRainbow(double v)
    {       
        double vmin = 0, vmax = 255, dv = vmax - vmin;
        if (v < (vmin + 0.25 * dv)) 
            return wxColour(0, int(255.0*(4.0 * (v - vmin) / dv) + 0.5), 255);
        else if (v < (vmin + 0.5 * dv)) 
            return wxColour(0, 255, int(255.0*(1.0 + 4.0 * (vmin + 0.25 * dv - v) / dv) + 0.5));
        else if (v < (vmin + 0.75 * dv)) 
            return wxColour(int(255.0*(4.0 * (v - vmin - 0.5 * dv) / dv) + 0.5), 255, 0);
        else 
            return wxColour(255, int(255.0*(1.0 + 4.0 * (vmin + 0.75 * dv - v) / dv) + 0.5), 0);
    }
#else
    #define UNUSE(a) 
#endif

void wxSheetCellRendererRefData::Draw( wxSheet& sheet, 
                                       const wxSheetCellAttr& attr,
                                       wxDC& dc, 
                                       const wxRect& rect,
                                       const wxSheetCoords& UNUSE(coords),
                                       bool isSelected )
{
    dc.SetBackgroundMode( wxSOLID );

    // grey out fields if the sheet is disabled
    if ( sheet.IsEnabled() )
    {
        if ( isSelected )
            dc.SetBrush( wxBrush(sheet.GetSelectionBackground(), wxSOLID) );
        else
            dc.SetBrush( wxBrush(attr.GetBackgroundColour(), wxSOLID) );
    }
    else
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSOLID));

    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(rect);
    
#ifdef TEST_SELECTION_BLOCKS // colouring for identifying different blocks
    if (isSelected)
    {        
        int i = sheet.GetSelection()->Index(coords);
        wxColour c(GetRainbow(i*10));
        dc.SetBrush( wxBrush(c, wxSOLID) );
        dc.DrawRectangle(rect);
    
        wxFont font = dc.GetFont();
        dc.SetFont(*wxSMALL_FONT);
        dc.DrawText(wxString::Format(wxT("%d"), i), rect.x, rect.y);
    }
#endif // TEST_SELECTION_BLOCKS
    
    //FIXME - border drawing code, maybe it goes here?
    //dc.SetPen( wxPen(sheet.GetGridLineColour(), 1, wxSOLID) );
    //dc.DrawRectangle(rect.x-1, rect.y-1, rect.width+2, rect.height+2);
}

wxSize wxSheetCellRendererRefData::GetBestSize(wxSheet& sheet, 
                                               const wxSheetCellAttr& WXUNUSED(attr),
                                               wxDC& WXUNUSED(dc), 
                                               const wxSheetCoords& coords) 
{ 
    return sheet.GetCellSize(coords); // return current size
} 

wxString wxSheetCellRendererRefData::GetString(wxSheet& sheet, const wxSheetCoords& coords)
{
    return sheet.GetCellValue(coords);
}

// ----------------------------------------------------------------------------
// wxSheetCellStringRendererRefData
// ----------------------------------------------------------------------------
wxSheetCellStringRendererRefData::wxSheetCellStringRendererRefData() 
{
    if (!s_overflowBitmap.Ok())
        s_overflowBitmap = wxBitmap(s_overflow_arrow_xpm_data);
}

void wxSheetCellStringRendererRefData::SetTextColoursAndFont(wxSheet& sheet,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     bool isSelected)
{
    dc.SetBackgroundMode( wxTRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    // different coloured text when the sheet is disabled
    if ( sheet.IsEnabled() )
    {
        if ( isSelected )
        {
            dc.SetTextBackground( sheet.GetSelectionBackground() );
            dc.SetTextForeground( sheet.GetSelectionForeground() );
        }
        else
        {
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetForegroundColour() );
        }
    }
    else
    {
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
    
    dc.SetFont( attr.GetFont() );
}

wxSize wxSheetCellStringRendererRefData::DoGetBestSize(wxSheet& sheet,
                                                       const wxSheetCellAttr& attr,
                                                       wxDC& dc,
                                                       const wxString& text)
{
    wxArrayString lines;
    long w=0, h=0;
    if (sheet.StringToLines(text, lines) > 0)
    {
        dc.SetFont(attr.GetFont());
        sheet.GetTextBoxSize(dc, lines, &w, &h);
    }
    
    return (attr.GetOrientation() == wxHORIZONTAL) ? wxSize(w, h) : wxSize(h, w);
}

wxSize wxSheetCellStringRendererRefData::GetBestSize(wxSheet& sheet,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     const wxSheetCoords& coords)
{
    return DoGetBestSize(sheet, attr, dc, GetString(sheet, coords));
}

void wxSheetCellStringRendererRefData::Draw(wxSheet& sheet,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{    
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);
    DoDraw(sheet, attr, dc, rectCell, coords, isSelected);
    
    //wxRendererNative &ren = wxRendererNative::Get();
    //ren.DrawComboBoxDropButton(&sheet, dc, rectCell);
}

void wxSheetCellStringRendererRefData::DoDraw(wxSheet& sheet,
                                              const wxSheetCellAttr& attr,
                                              wxDC& dc,
                                              const wxRect& rectCell,
                                              const wxSheetCoords& coords,
                                              bool isSelected)
{        
    wxRect rect = rectCell;
    rect.Inflate(-1);
    
    int align = attr.GetAlignment();

    wxString value = sheet.GetCellValue(coords);
    int best_width = DoGetBestSize(sheet, attr, dc, value).GetWidth();
    wxSheetCoords cellSpan(sheet.GetCellSpan(coords)); // shouldn't get here if <=0
    int cell_rows = cellSpan.m_row;
    int cell_cols = cellSpan.m_col;
    
    bool is_grid_cell = wxSheet::IsGridCell(coords);
    // no overflow for row/col/corner labels
    bool overflow = is_grid_cell ? attr.GetOverflow() : false;
    int overflowCols = 0;
    int num_cols = sheet.GetNumberCols();
    // this is the right col which includes overflow
    int rightCol = coords.m_col + cell_cols - 1; 
    
    // Check if this cell should overflow to right and for how many cells
    if (overflow)
    {
        bool is_editing = sheet.IsCellEditControlShown();
        wxSheetCoords editorCell = is_editing ? sheet.GetEditControlCoords() : wxNullSheetCoords;
        int row = coords.GetRow(), col = coords.GetCol();
        wxSheetCoords ownerCell;
        if ((best_width > rectCell.width) && (col < num_cols-1) && sheet.GetTable())
        {
            wxSheetCoords cell;
            for (cell.m_col = col+cell_cols; cell.m_col < num_cols; cell.m_col++)
            {
                bool is_empty = true;
                for (cell.m_row = row; cell.m_row < row+cell_rows; cell.m_row++)
                {
                    // check w/ anchor cell for spanned cell block
                    ownerCell = sheet.GetCellOwner(cell);
                    if ( sheet.GetTable()->HasValue(ownerCell) || 
                         (ownerCell == editorCell) )
                    {
                        is_empty = false;
                        break;
                    }
                }
                
                if (is_empty)
                    rect.width += sheet.GetColWidth(cell.m_col);
                else
                {
                    cell.m_col--;
                    break;
                }
                
                if (rect.width >= best_width) 
                    break;
            }
            // this may extend out of sheet
            overflowCols = cell.m_col - col - cell_cols + 1;
            rightCol = wxMin(coords.m_col+cell_cols-1+overflowCols, num_cols - 1);
        }

        // redraw overflow cells individually for proper selection hilight
        if (overflowCols > 0) 
        {
            // if overflowed then it's left aligned (yes I know ALIGN_LEFT=0)
            align &= ~wxSHEET_AttrAlignHoriz_Mask;
            align |= wxSHEET_AttrAlignLeft;    

            wxRect clip(rect);
            clip.x += rectCell.width;

            int col_width;
            wxSheetCoords cell(coords);
            // draw each cell individually since it may be selected or not
            for (cell.m_col = col+cell_cols; cell.m_col <= rightCol; cell.m_col++)
            {
                col_width = sheet.GetColWidth(cell.m_col);
                clip.width = col_width - 1;
                dc.DestroyClippingRegion();
                dc.SetClippingRegion(clip);
                SetTextColoursAndFont(sheet, attr, dc, sheet.IsCellSelected(cell));
                sheet.DrawTextRectangle(dc, value, rect, align);
                clip.x += col_width - 1;
            }

            rect = rectCell;
            rect.Inflate(-1);
            rect.width++;
            dc.DestroyClippingRegion();
        }
    }

    // Draw the text 
    SetTextColoursAndFont(sheet, attr, dc, isSelected);
    sheet.DrawTextRectangle(dc, value, rect, align);

    if (attr.GetOverflowMarker())
    {        
        // Draw a marker to show that the contents has been clipped off
        int cellRight = sheet.GetColRight(rightCol);
        if (cellRight - rect.x < best_width)
        {
            int bmpWidth   = s_overflowBitmap.GetWidth();
            int bmpHeight  = s_overflowBitmap.GetHeight();
            int cellWidth  = sheet.GetColWidth(rightCol);
            int cellHeight = sheet.GetRowHeight(coords.m_row);
        
            if ((bmpWidth < cellWidth-3) && (bmpHeight < cellHeight-3))
            {
                int cellTop = sheet.GetRowTop(coords.m_row);
            
                int x = cellRight - bmpWidth - 2;
                int y = cellTop + (cellHeight - bmpHeight)/2;
                wxRect r(x-2, cellTop, bmpWidth+4-1, cellHeight-1);
                wxSheetCellAttr rightAttr(attr);
                if (overflowCols > 0)
                {
                    wxSheetCoords clipCell(coords.m_row, rightCol);
                    isSelected = sheet.IsCellSelected(clipCell);
                    rightAttr = sheet.GetAttr(clipCell);
                }
            
                // clear background for bitmap
                wxSheetCellRendererRefData::Draw(sheet, rightAttr, dc, r, coords, isSelected);
                dc.DrawBitmap( s_overflowBitmap, x, y, true );
            }
        }
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringRendererRefData
// ----------------------------------------------------------------------------

void wxSheetCellAutoWrapStringRendererRefData::Draw(wxSheet& sheet,
                                                    const wxSheetCellAttr& attr,
                                                    wxDC& dc,
                                                    const wxRect& rectCell,
                                                    const wxSheetCoords& coords,
                                                    bool isSelected) 
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);
    SetTextColoursAndFont(sheet, attr, dc, isSelected);

    int align = attr.GetAlignment();

    wxRect rect = rectCell;
    rect.Inflate(-1);

    sheet.DrawTextRectangle(dc, GetTextLines(sheet, dc, attr, rect, coords),
                           rect, align);
}

wxArrayString
wxSheetCellAutoWrapStringRendererRefData::GetTextLines(wxSheet& sheet,
                                                       wxDC& dc,
                                                       const wxSheetCellAttr& attr,
                                                       const wxRect& rect,
                                                       const wxSheetCoords& coords)
{
    wxString data = sheet.GetCellValue(coords);

    wxArrayString lines;
    dc.SetFont(attr.GetFont());

    //Taken from wxSheet again!
    wxCoord x = 0, y = 0, curr_x = 0;
    wxCoord max_x = rect.GetWidth();

    wxStringTokenizer tk(data, _T(" \n\t\r"));
    wxString thisline;

    while ( tk.HasMoreTokens() )
    {
        wxString tok = tk.GetNextToken();
        //FIXME: this causes us to print an extra unnecesary
        //       space at the end of the line. But it
        //       is invisible , simplifies the size calculation
        //       and ensures tokens are separated in the display
        tok += _T(" ");

        dc.GetTextExtent(tok, &x, &y);
        if ( curr_x + x > max_x) 
        {
            lines.Add( thisline );
            thisline = tok;
            curr_x = x;
        } 
        else 
        {
            thisline += tok;
            curr_x += x;
        }
    }
    
    lines.Add( thisline ); //Add last line

    return lines;
}

wxSize wxSheetCellAutoWrapStringRendererRefData::GetBestSize(wxSheet& sheet,
                                                      const wxSheetCellAttr& attr,
                                                      wxDC& dc,
                                                      const wxSheetCoords& coords)
{
    wxCoord x, y, height, width = sheet.GetColWidth(coords.GetCol()) -10;
    int count = 250; //Limit iterations..

    wxRect rect(0, 0, width, 10);

    // M is a nice large character 'y' gives descender!.
    dc.SetFont(attr.GetFont());
    dc.GetTextExtent(wxT("My"), &x, &y);

    do     // Search for a shape no taller than the golden ratio.
    {
        width += 10;
        rect.SetWidth(width);
        height = y * GetTextLines(sheet,dc,attr,rect,coords).GetCount();
        count--;
    } while (count && (width < (height*1.68)) );

    return wxSize(width, height);
}

// ----------------------------------------------------------------------------
// wxSheetCellNumberRendererRefData
// ----------------------------------------------------------------------------

wxString wxSheetCellNumberRendererRefData::GetString(wxSheet& sheet, const wxSheetCoords& coords)
{
    wxSheetTable *table = sheet.GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
        return wxString::Format(_T("%ld"), table->GetValueAsLong(coords));

    return sheet.GetCellValue(coords);
}

void wxSheetCellNumberRendererRefData::Draw(wxSheet& sheet,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(sheet, attr, dc, isSelected);

    // draw the text right aligned by default 
    int align = attr.GetAlignment(); // | wxALIGN_RIGHT;  //FIXME Why forced right?

    wxRect rect = rectCell;
    rect.Inflate(-1);

    sheet.DrawTextRectangle(dc, GetString(sheet, coords), rect, align);
}

// ----------------------------------------------------------------------------
// wxSheetCellFloatRendererRefData
// ----------------------------------------------------------------------------
bool wxSheetCellFloatRendererRefData::Copy(const wxSheetCellFloatRendererRefData &other)
{
    SetWidth(other.GetWidth());
    SetPrecision(other.GetPrecision());    
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellFloatRendererRefData::GetString(wxSheet& sheet, const wxSheetCoords& coords)
{
    wxSheetTable *table = sheet.GetTable();

    bool hasDouble = false;
    double val = 0;
    wxString text;
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_FLOAT) )
    {
        val = table->GetValueAsDouble(coords);
        hasDouble = true;
    }
    else
    {
        text = sheet.GetCellValue(coords);
        hasDouble = text.ToDouble(&val);
    }

    if ( hasDouble )
    {
        if ( !m_format )
        {
            if ( m_width < 0 )
            {
                if ( m_precision < 0 )
                    m_format = _T("%f"); // default width/precision
                else
                    m_format.Printf(_T("%%.%df"), m_precision);
            }
            else if ( m_precision < 0 )
                m_format.Printf(_T("%%%d.f"), m_width); // default precision
            else
                m_format.Printf(_T("%%%d.%df"), m_width, m_precision);
        }

        text.Printf(m_format, val);
    }

    return text;    
}

void wxSheetCellFloatRendererRefData::Draw( wxSheet& sheet,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected )
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(sheet, attr, dc, isSelected);

    // draw the text right aligned by default  -- FIXME ? WHY
    int align = attr.GetAlignment(); // | wxALIGN_RIGHT;     

    wxRect rect(rectCell);
    rect.Inflate(-1);

    sheet.DrawTextRectangle(dc, GetString(sheet, coords), rect, align);
}

void wxSheetCellFloatRendererRefData::SetParameters(const wxString& params)
{
    if ( params.IsEmpty() )
    {
        // reset to defaults
        SetWidth(-1);
        SetPrecision(-1);
    }
    else
    {
        wxString tmp = params.BeforeFirst(_T(','));
        if ( !tmp.IsEmpty() )
        {
            long width;
            if ( tmp.ToLong(&width) )
                SetWidth((int)width);
            else
                wxLogDebug(_T("Invalid wxSheetCellFloatRenderer width parameter string '%s ignored"), params.c_str());
        }
        tmp = params.AfterFirst(_T(','));
        if ( !tmp.IsEmpty() )
        {
            long precision;
            if ( tmp.ToLong(&precision) )
                SetPrecision((int)precision);
            else
                wxLogDebug(_T("Invalid wxSheetCellFloatRenderer precision parameter string '%s ignored"), params.c_str());
        }
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellBitmapRendererRefData
// ----------------------------------------------------------------------------

wxSize wxSheetCellBitmapRendererRefData::GetBestSize(wxSheet& sheet,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     const wxSheetCoords& coords)
{
    wxSize bmpSize;
    if (m_bitmap.Ok())
        bmpSize = wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() );
    
    wxSize strSize = wxSheetCellStringRendererRefData::GetBestSize(sheet, attr, dc, coords);
   
    return wxSize(bmpSize.x + strSize.x + 5, wxMax(bmpSize.y, strSize.y));
}

#define MARGIN 2

void wxSheetCellBitmapRendererRefData::Draw(wxSheet& sheet,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc, const wxRect& rect_,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rect_, coords, isSelected);

    int text_align = attr.GetAlignment();    

    wxRect rect(rect_);
    
    wxSize bmpSize;
    if (m_bitmap.Ok())
    {
        bmpSize.x = m_bitmap.GetWidth();
        bmpSize.y = m_bitmap.GetHeight();
    }
    
    wxSize txtSize(wxSheetCellStringRendererRefData::GetBestSize(sheet, attr, dc, coords));

    wxRect bmpRect(rect);
    wxRect txtRect(rect);
    
    if ((txtSize.x == 0) && (bmpSize.x == 0))
        return;
    
    int margin = 2;
    
    if ((txtSize.x == 0) || (bmpSize.x == 0))
        margin = 0;
    
    if ((m_align & wxSHEET_BMPREN_BMPRIGHT) != 0)
    {
        //wxPrintf(wxT("1Printing row %d col %d \n"), coords.m_row, coords.m_col);
        bmpRect.x += txtSize.x + margin;
        bmpRect.width -= txtSize.x + margin;
        if (txtSize.x < rect.width)
            txtRect.width = txtSize.x;
    }
    else if ((m_align & wxSHEET_BMPREN_BMPABOVE) != 0)
    {
        //wxPrintf(wxT("2Printing row %d col %d \n"), coords.m_row, coords.m_col);
        txtRect.y += bmpSize.y + margin;
        txtRect.height -= bmpSize.y + margin;
        if (bmpSize.y < rect.height)
            bmpRect.height = bmpSize.y;
    }
    else if ((m_align & wxSHEET_BMPREN_BMPBELOW) != 0)
    {
        //wxPrintf(wxT("3Printing row %d col %d \n"), coords.m_row, coords.m_col);
        bmpRect.y += txtSize.y + margin;
        bmpRect.height -= txtSize.y + margin;
        if (txtSize.y < rect.height)
            txtRect.height = txtSize.y;
    }
    else //if ((m_align & wxSHEET_BMPREN_BMPLEFT) != 0)
    {
        //wxPrintf(wxT("4Printing row %d col %d \n"), coords.m_row, coords.m_col);
        txtRect.x += bmpSize.x + margin;
        txtRect.width -= bmpSize.x + margin;
        if (bmpSize.x < rect.width)
            bmpRect.width = bmpSize.x;
    }

    bmpRect.SetPosition(sheet.AlignInRect(m_align, bmpRect, bmpSize));
    txtRect.SetPosition(sheet.AlignInRect(text_align, txtRect, txtSize));
        
    bmpRect.Intersect(rect);
    txtRect.Intersect(rect);

    // text renderer will deflate it by 1
    txtRect.Inflate(1);
    
    //wxPrintf(wxT("Printing row %d col %d \n"), coords.m_row, coords.m_col);
    //PRINT_RECT("orig   ", rect);
    //PRINT_RECT("bmpRect", bmpRect);
    //PRINT_RECT("txtRect", txtRect);
    
    if ((txtRect.width > 0) && (txtRect.height > 0))
        wxSheetCellStringRendererRefData::DoDraw(sheet, attr, dc, txtRect, coords, isSelected);
    
    if (m_bitmap.Ok() && (bmpRect.width > 0) && (bmpRect.height > 0))
    {
        dc.SetClippingRegion(rect);
        dc.DrawBitmap(m_bitmap, bmpRect.x, bmpRect.y, true);
        dc.DestroyClippingRegion();
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellBoolRendererRefData
// ----------------------------------------------------------------------------

wxSize wxSheetCellBoolRendererRefData::ms_sizeCheckMark;

// FIXME these checkbox size calculations are really ugly...

// between checkmark and box
#define wxSHEET_CHECKMARK_MARGIN 2

wxSize wxSheetCellBoolRendererRefData::GetBestSize(wxSheet& sheet,
                                                   const wxSheetCellAttr& WXUNUSED(attr),
                                                   wxDC& WXUNUSED(dc),
                                                   const wxSheetCoords& )
{
    // compute it only once (no locks for MT safeness in GUI thread...)
    if ( !ms_sizeCheckMark.x )
    {
        // get checkbox size
        wxCheckBox *checkbox = new wxCheckBox(&sheet, wxID_ANY, wxEmptyString);
        wxSize size = checkbox->GetBestSize();
        wxCoord checkSize = size.y + 2*wxSHEET_CHECKMARK_MARGIN;

        // FIXME wxGTK::wxCheckBox::GetBestSize() gives "wrong" result
#if defined(__WXGTK__) || defined(__WXMOTIF__)
        checkSize -= size.y / 2;
#endif // defined(__WXGTK__) || defined(__WXMOTIF__)

        delete checkbox;
        ms_sizeCheckMark.x = ms_sizeCheckMark.y = checkSize;
    }

    return ms_sizeCheckMark;
}

void wxSheetCellBoolRendererRefData::Draw(wxSheet& sheet,
                                          const wxSheetCellAttr& attr,
                                          wxDC& dc, const wxRect& rect,
                                          const wxSheetCoords& coords,
                                          bool isSelected)
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rect, coords, isSelected);

    // draw a check mark in the centre (ignoring alignment - TODO)
    wxSize size = GetBestSize(sheet, attr, dc, coords);

    // don't draw outside the cell
    wxCoord minSize = wxMin(rect.width, rect.height);
    if ((size.x >= minSize) || (size.y >= minSize))
        size.x = size.y = minSize - 2; // leave (at least) 1 pixel margin

    // draw a border around checkmark
    int align = attr.GetAlignment();

    wxRect rectBorder(rect.GetPosition(), size);
    
    if ((align & wxALIGN_RIGHT) != 0)
        rectBorder.x += rect.width - size.x - 2;
    else if ((align & wxALIGN_CENTRE_HORIZONTAL) != 0)
        rectBorder.x += rect.width/2 - size.x/2;
    else // wxALIGN_LEFT
        rectBorder.x += 2;

    if ((align & wxALIGN_BOTTOM) != 0)
        rectBorder.y += rect.height - size.y - 2;
    else if ((align & wxALIGN_CENTRE_VERTICAL) != 0)
        rectBorder.y += rect.height/2 - size.y/2;
    else // wxALIGN_TOP
        rectBorder.y += 2;

    bool value;
    if ( sheet.GetTable()->CanGetValueAs(coords, wxSHEET_VALUE_BOOL) )
        value = sheet.GetTable()->GetValueAsBool(coords);
    else
    {
        wxString strValue( sheet.GetTable()->GetValue(coords) );
        value = !( strValue.IsEmpty() || (strValue == wxT("0")) );
    }

    if ( value )
    {
        wxRect rectMark = rectBorder;
#ifdef __WXMSW__
        // MSW DrawCheckMark() is weird (and should probably be changed...)
        rectMark.Inflate(-wxSHEET_CHECKMARK_MARGIN/2);
        rectMark.x++;
        rectMark.y++;
#else // !MSW
        rectMark.Inflate(-wxSHEET_CHECKMARK_MARGIN);
#endif // MSW/!MSW

        dc.SetTextForeground(attr.GetForegroundColour());
        dc.DrawCheckMark(rectMark);
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(attr.GetForegroundColour(), 1, wxSOLID));
    dc.DrawRectangle(rectBorder);
}

// ----------------------------------------------------------------------------
// wxSheetCellDateTimeRendererRefData
// ----------------------------------------------------------------------------
#if wxUSE_DATETIME

// Enables a sheet cell to display a formated date and or time
bool wxSheetCellDateTimeRendererRefData::Copy(const wxSheetCellDateTimeRendererRefData& other)
{
    m_outFormat = other.m_outFormat;
    m_inFormat  = other.m_inFormat;
    m_dateTime  = other.m_dateTime;
    m_tz        = other.m_tz;
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellDateTimeRendererRefData::GetString(wxSheet& sheet, const wxSheetCoords& coords)
{
    wxSheetTable *table = sheet.GetTable();

    bool hasDatetime = false;
    wxDateTime val;
    wxString text;
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_DATETIME) )
    {
        void* tempval = table->GetValueAsCustom(coords, wxSHEET_VALUE_DATETIME);

        if (tempval)
        {
            val = *((wxDateTime *)tempval);
            hasDatetime = true;
            delete (wxDateTime *)tempval;
        }
    }

    if ( !hasDatetime )
    {
        text = sheet.GetCellValue(coords);
        hasDatetime = val.ParseFormat(text, m_inFormat, m_dateTime) != (wxChar *)NULL ;
    }
    else
        text = val.Format( m_outFormat, m_tz );

    return text; //If we faild to parse string just show what we where given?
}

void wxSheetCellDateTimeRendererRefData::Draw(wxSheet& sheet,
                                              const wxSheetCellAttr& attr,
                                              wxDC& dc,
                                              const wxRect& rectCell,
                                              const wxSheetCoords& coords,
                                              bool isSelected)
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(sheet, attr, dc, isSelected);

    // draw the text right aligned by default FIXME why?
    int align = attr.GetAlignment(); // | wxRIGHT; 

    wxRect rect = rectCell;
    rect.Inflate(-1);

    sheet.DrawTextRectangle(dc, GetString(sheet, coords), rect, align);
}

void wxSheetCellDateTimeRendererRefData::SetParameters(const wxString& params)
{
    if (!params.IsEmpty())
        m_outFormat = params;
}

#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxSheetCellEnumRendererRefData
// ----------------------------------------------------------------------------
// Renders a number as a textual equivalent.
// eg data in cell is 0,1,2 ... n the cell could be rendered as "John","Fred"..."Bob"

wxSheetCellEnumRendererRefData::wxSheetCellEnumRendererRefData(const wxString& choices)
{
    if (!choices.IsEmpty())
        SetParameters(choices);
}

bool wxSheetCellEnumRendererRefData::Copy(const wxSheetCellEnumRendererRefData& other)
{
    m_choices = other.m_choices;
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellEnumRendererRefData::GetString( wxSheet& sheet, 
                                                    const wxSheetCoords& coords )
{
    wxSheetTable *table = sheet.GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
    {
        int choiceno = table->GetValueAsLong(coords);
        return m_choices[choiceno];
    }

    return sheet.GetCellValue(coords);
}

void wxSheetCellEnumRendererRefData::Draw( wxSheet& sheet,
                                           const wxSheetCellAttr& attr,
                                           wxDC& dc,
                                           const wxRect& rectCell,
                                           const wxSheetCoords& coords,
                                           bool isSelected )
{
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rectCell, coords, isSelected);
    SetTextColoursAndFont(sheet, attr, dc, isSelected);
    
    // draw the text right aligned by default FIXME why?
    int align = attr.GetAlignment(); // | wxRIGHT;     

    wxRect rect = rectCell;
    rect.Inflate(-1);

    sheet.DrawTextRectangle(dc, GetString(sheet, coords), rect, align);
}

void wxSheetCellEnumRendererRefData::SetParameters(const wxString& params)
{    
    if ( params.IsEmpty() )
        return; // what can we do?

    m_choices.Empty();

    wxStringTokenizer tk(params, _T(','));
    while ( tk.HasMoreTokens() )
        m_choices.Add(tk.GetNextToken());
}

// ----------------------------------------------------------------------------
// wxSheetCellRolColLabelRendererRefData
// ----------------------------------------------------------------------------

void wxSheetCellRolColLabelRendererRefData::Draw(wxSheet& sheet,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{    
    // erase this cells background
    wxRect rect(rectCell);
    wxSheetCellRendererRefData::Draw(sheet, attr, dc, rect, coords, isSelected);
    
    int left   = rectCell.x;
    int top    = rectCell.y;
    int right  = rectCell.GetRight();
    int bottom = rectCell.GetBottom();
    
    //dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW),1, wxSOLID) );
    // right side
    //dc.DrawLine( right, top, right, bottom );
    // left side
    //if (coords.m_col == -1)
    //    dc.DrawLine( left, top, left, bottom );
    // top side
    //if (coords.m_row == -1)
    //    dc.DrawLine( left, top, right, top );
    // bottom
    //dc.DrawLine( left, bottom, right, bottom );

    dc.SetPen( *wxWHITE_PEN );
    // left highlight
    dc.DrawLine( left, top, left, bottom );
    // top highlight
    dc.DrawLine( left, top, right, top );

    SetTextColoursAndFont(sheet, attr, dc, isSelected);

    wxString value = sheet.GetCellValue(coords);

    if (!value.IsEmpty())
    {
        int align  = attr.GetAlignment();
        int orient = attr.GetOrientation();
        rect.Deflate(2); // want margins
        sheet.DrawTextRectangle(dc, value, rect, align, orient);    
    }
    
#if 0    
    // test code for sizing, draws corner tick marks    
    if (1)
    {
        rect = rectCell;
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawLine(left, top, left+25, top);
        dc.DrawLine(right-25, bottom, right, bottom);
        dc.DrawLine(left, top, left, top+10);
        dc.DrawLine(right, bottom-10, right, bottom);
        wxRect r(rectCell);
        dc.SetPen(*wxCYAN_PEN);
        dc.DrawLine(r.x, r.y, r.x+25, r.y);
        dc.DrawLine(r.GetRight()-25, r.GetBottom(), r.GetRight(), r.GetBottom());
        dc.DrawLine(r.x, r.y, r.x, r.y+10);
        dc.DrawLine(r.GetRight(), r.GetBottom()-10, r.GetRight(), r.GetBottom());
    }
#endif // 0    
}
