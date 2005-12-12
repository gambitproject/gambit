///////////////////////////////////////////////////////////////////////////////
// Name:        sheettbl.cpp
// Purpose:     wxSheetTable and related classes
// Author:      John Labenski, Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheettbl.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sheettbl.h"
#include "sheet.h"
#include <math.h>                 // used in GetDefaultColLabelValue

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());
#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#define wxStrF wxString::Format

// ----------------------------------------------------------------------------
// wxSheetTable
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetTable, wxObject )

wxSheetTable::wxSheetTable(wxSheet *view)
             :wxObject(), m_view(view), 
              m_attrProvider(NULL),   m_own_attr_provider(true),
              m_gridCellValues(NULL), m_own_grid_cell_values(true),
              m_rowLabelValues(NULL), m_own_row_label_values(true),
              m_colLabelValues(NULL), m_own_col_label_values(true),
              m_spannedCells(NULL),   m_own_spanned_cells(true)
{
    m_spannedCells = new wxSheetSelection();
}

wxSheetTable::~wxSheetTable()
{
    SetAttrProvider(NULL, true);
    SetGridCellValueProvider(NULL, true);
    SetRowLabelValueProvider(NULL, true);
    SetColLabelValueProvider(NULL, true);
    SetSpannedBlocks(NULL, true);
}

int wxSheetTable::GetNumberRows()
{
    wxCHECK_MSG(GetGridCellValueProvider(), 0, wxT("Invalid cell data for the table"));
    return GetGridCellValueProvider()->GetNumberRows();
}
int wxSheetTable::GetNumberCols()
{
    wxCHECK_MSG(GetGridCellValueProvider(), 0, wxT("Invalid cell data for the table"));
    return GetGridCellValueProvider()->GetNumberCols();
}

wxString wxSheetTable::GetValue( const wxSheetCoords& coords )
{
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID : 
        {
            // need cell data for values, or override this
            wxCHECK_MSG(GetGridCellValueProvider(), wxEmptyString, wxT("Invalid grid cell data for the table"));
            return GetGridCellValueProvider()->GetValue(coords);
        }
        case wxSHEET_CELL_ROWLABEL : 
        {
            // just return default if no row label data
            wxSheetCoords rowCoords(coords.SheetToRowLabel());
            if (GetRowLabelValueProvider() && GetRowLabelValueProvider()->HasValue(rowCoords))
                return GetRowLabelValueProvider()->GetValue(rowCoords);

            return GetDefaultRowLabelValue(coords.m_row);
        }
        case wxSHEET_CELL_COLLABEL : 
        {
            wxSheetCoords colCoords(coords.SheetToColLabel());
            if (GetColLabelValueProvider() && GetColLabelValueProvider()->HasValue(colCoords))
                return GetColLabelValueProvider()->GetValue(colCoords);

            return GetDefaultColLabelValue(coords.m_col);
        }
        case wxSHEET_CELL_CORNERLABEL :
        {
            return m_cornerLabelValue;
        }
        default :
            return wxEmptyString;
    }   
}

void wxSheetTable::SetValue( const wxSheetCoords& coords, const wxString& value )
{
    // You must override this or have set data containers for this to work
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID : 
        {
            wxCHECK_RET(GetGridCellValueProvider(), wxT("Invalid grid cell data for the table"));
            GetGridCellValueProvider()->SetValue(coords, value);
            break;
        }
        case wxSHEET_CELL_ROWLABEL : 
        {
            wxCHECK_RET(GetRowLabelValueProvider(), wxT("Invalid row label data for the table"));
            GetRowLabelValueProvider()->SetValue(coords.SheetToRowLabel(), value);
            break;
        }
        case wxSHEET_CELL_COLLABEL : 
        {
            wxCHECK_RET(GetColLabelValueProvider(), wxT("Invalid col label data for the table"));
            GetColLabelValueProvider()->SetValue(coords.SheetToColLabel(), value);
            break;
        }
        case wxSHEET_CELL_CORNERLABEL :
        {
            m_cornerLabelValue = value;
            break;
        }
        default :
            break;
    }   
}
bool wxSheetTable::HasValue( const wxSheetCoords& coords ) 
{ 
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID : 
        {
            if (GetGridCellValueProvider()) 
                return GetGridCellValueProvider()->HasValue(coords);
            
            break;
        }
        case wxSHEET_CELL_ROWLABEL : 
        {
            if (GetRowLabelValueProvider()) 
                return GetRowLabelValueProvider()->HasValue(coords.SheetToRowLabel());
            
            break;
        }
        case wxSHEET_CELL_COLLABEL : 
        {
            if (GetColLabelValueProvider()) 
                return GetColLabelValueProvider()->HasValue(coords.SheetToColLabel());
            
            break;
        }
        case wxSHEET_CELL_CORNERLABEL :
        default : break;
    }   
    
    return !GetValue(coords).IsEmpty(); 
}
int wxSheetTable::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) 
{ 
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID : 
        {
            if (GetGridCellValueProvider()) 
                return GetGridCellValueProvider()->GetFirstNonEmptyColToLeft(coords);
            
            break;
        }
        case wxSHEET_CELL_ROWLABEL : 
        {
            if (GetRowLabelValueProvider()) 
                return GetRowLabelValueProvider()->GetFirstNonEmptyColToLeft(coords.SheetToRowLabel());
            
            break;
        }
        case wxSHEET_CELL_COLLABEL : 
        {
            if (GetColLabelValueProvider()) 
                return GetColLabelValueProvider()->GetFirstNonEmptyColToLeft(coords.SheetToColLabel());
            
            break;
        }
        case wxSHEET_CELL_CORNERLABEL :
        default : break;
    }   
    
    return coords.m_col-1;
}

void wxSheetTable::ClearValues(int update)
{
    if (((update & wxSHEET_UpdateGridCellValues) != 0) && GetGridCellValueProvider())
        GetGridCellValueProvider()->ClearValues();
    if (((update & wxSHEET_UpdateRowLabelValues) != 0) && GetRowLabelValueProvider())
        GetRowLabelValueProvider()->ClearValues();
    if (((update & wxSHEET_UpdateColLabelValues) != 0) && GetColLabelValueProvider())
        GetColLabelValueProvider()->ClearValues();

    if (GetAttrProvider())
        GetAttrProvider()->Clear(update);
}

wxString wxSheetTable::GetDefaultRowLabelValue( int row ) const
{
    return wxString::Format(wxT("%d"), row+1); // Starting at zero confuses users
}
wxString wxSheetTable::GetDefaultColLabelValue( int col ) const
{
    // default col labels are: cols [0-25]=[A-Z], cols [26-675]=[AA-ZZ]
/*
    wxString s, s2;
    unsigned int i, n;
    for ( n = 1; ; n++ )
    {
        s += wxChar((_T('A')) + (wxChar)( col%26 ));
        col = col/26 - 1;
        if ( col < 0 ) break;
    }
    // reverse the string...
    for ( i = n; i > 0; i-- ) s2 += s[i-1]; 
    //for ( i = 0; i < n;  i++ )  s2 += s[n-i-1];

    return s2;
*/
 
    // new method using log function so you don't have to reverse the string
    wxCHECK_MSG(col >= 0, wxEmptyString, wxT("Invalid col"));
    const size_t chars = size_t(log(col)/3.2580965380); // log_26(x) = log(x)/log(26.0)
    wxString s(wxT('A'), chars+1);
    for ( size_t n = 0; n <= chars; n++ )
    {
        s[size_t(chars - n)] = wxChar(_T('A') + (wxChar)(col%26));
        col = col/26 - 1;
    }
    
    return s;
}

long wxSheetTable::GetValueAsLong( const wxSheetCoords& coords )
{
    const wxString val(GetValue(coords));
    long lval = 0;
    val.ToLong(&lval);
    //wxCHECK_MSG(val.ToLong(&lval), 0, wxT("Unable to get cell ") + CS(coords) + wxT(" value as long."));
    return lval;
}
double wxSheetTable::GetValueAsDouble( const wxSheetCoords& coords )
{
    const wxString val(GetValue(coords));
    double dval = 0.0;
    val.ToDouble(&dval);
    // wxCHECK_MSG(val.ToDouble(&dval), 0.0, wxT("Unable to get cell ") + CS(coords) + wxT(" value as double."));
    return dval;
}
bool wxSheetTable::GetValueAsBool( const wxSheetCoords& coords )
{
    const wxString val(GetValue(coords));
    if (val.IsEmpty() || (val == wxT("0")) || 
       (val.CmpNoCase(wxT("f")) == 0) || (val.CmpNoCase(wxT("false")) == 0))
        return false;
    
    return true;
}
void wxSheetTable::SetValueAsLong( const wxSheetCoords& coords, long value )
{
    SetValue(coords, wxString::Format(wxT("%ld"), value));
}
void wxSheetTable::SetValueAsDouble( const wxSheetCoords& coords, double value )
{
    SetValue(coords, wxString::Format(wxT("%g"), value));
}
void wxSheetTable::SetValueAsBool( const wxSheetCoords& coords, bool value )
{
    SetValue(coords, value ? wxT("1") : wxT("0"));
}
void* wxSheetTable::GetValueAsCustom( const wxSheetCoords& coords,
                                          const wxString& typeName )
{
    wxFAIL_MSG(wxStrF(wxT("Unable to return wxSheetTable::GetValueAsCustom for cell (%d, %d) and type '%s'"),
                      coords.m_row, coords.m_col, typeName.c_str()));
    
    return NULL;
}
void  wxSheetTable::SetValueAsCustom( const wxSheetCoords& coords,
                                          const wxString& typeName,
                                          void* WXUNUSED(value) )
{
    wxFAIL_MSG(wxStrF(wxT("Unable to set wxSheetTable::SetValueAsCustom for cell (%d, %d) and type '%s'"),
                      coords.m_row, coords.m_col, typeName.c_str()));
}

bool wxSheetTable::CanGetValueAs( const wxSheetCoords& coords,
                                  const wxString& typeName )
{
    if ((typeName == wxSHEET_VALUE_NUMBER) || (typeName == wxSHEET_VALUE_CHOICEINT))
    {
        const wxString val(GetValue(coords));
        long lval = 0;
        return val.ToLong(&lval);
    }
    else if (typeName == wxSHEET_VALUE_FLOAT)
    {
        const wxString val(GetValue(coords));
        double dval = 0.0;
        return val.ToDouble(&dval);
    }
    else if (typeName == wxSHEET_VALUE_BOOL)
    {
        const wxString val(GetValue(coords));
        return val.IsEmpty() || (val == wxT("0")) || (val == wxT("1")) || 
               (val.CmpNoCase(wxT("f")) == 0) || (val.CmpNoCase(wxT("t")) == 0) ||
               (val.CmpNoCase(wxT("false")) == 0) || (val.CmpNoCase(wxT("true")) == 0);
    }

    return typeName == wxSHEET_VALUE_STRING;
}
bool wxSheetTable::CanSetValueAs( const wxSheetCoords& coords, const wxString& typeName )
{
    return CanGetValueAs(coords, typeName);
}

wxString wxSheetTable::GetTypeName( const wxSheetCoords& )
{
    return wxEmptyString; //wxSHEET_VALUE_STRING;
}

void wxSheetTable::SetGridCellValueProvider(wxSheetValueProviderBase *gridCellValues, bool is_owner)
{
    if (m_gridCellValues && m_own_grid_cell_values)
        delete m_gridCellValues;
    
    m_gridCellValues = gridCellValues;
    m_own_grid_cell_values = is_owner;
}
void wxSheetTable::SetRowLabelValueProvider(wxSheetValueProviderBase *rowLabelValues, bool is_owner)
{
    if (m_rowLabelValues && m_own_row_label_values)
        delete m_rowLabelValues;
    
    m_rowLabelValues = rowLabelValues;
    m_own_row_label_values = is_owner;
}
void wxSheetTable::SetColLabelValueProvider(wxSheetValueProviderBase *colLabelValues, bool is_owner)
{
    if (m_colLabelValues && m_own_col_label_values)
        delete m_colLabelValues;
    
    m_colLabelValues = colLabelValues;
    m_own_col_label_values = is_owner;
}

wxSheetCellAttr wxSheetTable::GetAttr( const wxSheetCoords& coords, 
                                       wxSheetAttr_Type kind )
{  
    // Ok to return nothing, the sheet will use the default attr if AttrAny
    if (GetAttrProvider())
        return GetAttrProvider()->GetAttr(coords, kind);

    return wxNullSheetCellAttr;
}

void wxSheetTable::SetAttr( const wxSheetCoords& coords, 
                            const wxSheetCellAttr& attr, wxSheetAttr_Type kind)
{
    // You must have set a provider
    wxCHECK_RET(GetAttrProvider(), wxT("Invalid attr provider in table")); 
    GetAttrProvider()->SetAttr(coords, attr, kind);    
}

void wxSheetTable::SetAttrProvider(wxSheetCellAttrProvider *attrProvider, bool is_owner)
{
    if (m_attrProvider && m_own_attr_provider)
        delete m_attrProvider;
    
    m_attrProvider = attrProvider;
    m_own_attr_provider = is_owner;
}

bool wxSheetTable::HasSpannedCells()
{
    return GetSpannedBlocks() && (GetSpannedBlocks()->GetCount() != 0u);
}
wxSheetBlock wxSheetTable::GetCellBlock( const wxSheetCoords& coords )
{
    if (GetSpannedBlocks()) // ok not to have this, just return 1x1
    {
        const int n = GetSpannedBlocks()->Index(coords);
        if (n != wxNOT_FOUND) 
            return GetSpannedBlocks()->GetBlock(n);
    }
    
    return wxSheetBlock(coords, 1, 1);
}
void wxSheetTable::SetCellSpan( const wxSheetBlock& block )
{
    wxCHECK_RET(GetSpannedBlocks(), wxT("Invalid spanned block container"));
    wxCHECK_RET(!block.IsEmpty(), wxT("Cannot set cell size smaller than (1,1)"));
    wxCHECK_RET((ContainsGridCell(block.GetLeftTop()) && ContainsGridCell(block.GetRightBottom())) ||
                (ContainsRowLabelCell(block.GetLeftTop()) && ContainsRowLabelCell(block.GetRightBottom())) ||
                (ContainsColLabelCell(block.GetLeftTop()) && ContainsColLabelCell(block.GetRightBottom())), 
                  wxT("Cannot set cell size for cell out of grid"));
    
    wxSheetBlock bounds(block);
    const wxArraySheetBlock &arrBlock = GetSpannedBlocks()->GetBlockArray();
    int n, index = wxNOT_FOUND, intersections = 0, count = arrBlock.GetCount();
       
    // Check for multiple intersections, one is fine, but topleft corners must match
    for (n=0; n<count; n++)
    {
        if (arrBlock[n].Intersects(block))
        {
            intersections++;
            index = n;
        }
    }
    
    wxCHECK_RET( (intersections < 2) && 
                ((intersections == 0) || (arrBlock[index].GetLeftTop() == block.GetLeftTop())), 
                 wxT("Setting cell span for cells already spanned"));
    
    wxSheetSelection *sel = (wxSheetSelection*)GetSpannedBlocks();
    
    // delete old block and expand refresh bounds 
    if (index != wxNOT_FOUND)
    {
        bounds = bounds.Union(arrBlock[index]);
        sel->DeselectBlock(arrBlock[index], false);
    }

    // no need to add a 1x1 block
    if ((block.GetWidth() > 1) || (block.GetHeight() > 1))
    {
        // don't combine them
        sel->SelectBlock(block, false); 
    }
    
    if (GetView()) GetView()->RefreshGridCellBlock(bounds);
}

void wxSheetTable::SetSpannedBlocks(wxSheetSelection *spannedCells, bool is_owner)
{
    if (m_spannedCells && m_own_spanned_cells)
        delete m_spannedCells;
    
    m_spannedCells = spannedCells;
    m_own_spanned_cells = is_owner;
}

bool wxSheetTable::UpdateRows( size_t row, int numRows, int update)
{
    const int curNumRows = GetNumberRows();
    wxSHEET_CHECKUPDATE_MSG(row, numRows, curNumRows, false);

    if (((update & wxSHEET_UpdateGridCellValues) != 0) && GetGridCellValueProvider())
        GetGridCellValueProvider()->UpdateRows(row, numRows);
    if (((update & wxSHEET_UpdateRowLabelValues) != 0) && GetRowLabelValueProvider())
        GetRowLabelValueProvider()->UpdateRows(row, numRows);
    
    if (((update & wxSHEET_UpdateSpanned) != 0) && GetSpannedBlocks()) 
        ((wxSheetSelection*)GetSpannedBlocks())->UpdateRows( row, numRows );    
    if (((update & wxSHEET_UpdateAttributes) != 0) && GetAttrProvider())
        GetAttrProvider()->UpdateRows( row, numRows, update );    
    
    // return back to sheet to let it update itself
    return UpdateSheetRows(row, numRows, update);
}
bool wxSheetTable::UpdateCols( size_t col, int numCols, int update )
{
    const int curNumCols = GetNumberCols();
    wxSHEET_CHECKUPDATE_MSG(col, numCols, curNumCols, false);

    if (((update & wxSHEET_UpdateGridCellValues) != 0) && GetGridCellValueProvider())
        GetGridCellValueProvider()->UpdateCols(col, numCols);
    if (((update & wxSHEET_UpdateColLabelValues) != 0) && GetColLabelValueProvider())
        GetColLabelValueProvider()->UpdateCols(col, numCols);
    
    if (((update & wxSHEET_UpdateSpanned) != 0) && GetSpannedBlocks())
        ((wxSheetSelection*)GetSpannedBlocks())->UpdateCols( col, numCols );    
    if (((update & wxSHEET_UpdateAttributes) != 0) && GetAttrProvider())
        GetAttrProvider()->UpdateCols( col, numCols, update );
    
    // return back to sheet to let it update itself
    return UpdateSheetCols(col, numCols, update);
}

bool wxSheetTable::UpdateSheetRows( size_t row, int numRows, int update )
{
    // this is not an error to allow people to use this independent of the sheet
    return GetView() && GetView()->DoUpdateRows(row, numRows, update);
}
bool wxSheetTable::UpdateSheetCols( size_t col, int numCols, int update )
{
    return GetView() && GetView()->DoUpdateCols(col, numCols, update);
}
bool wxSheetTable::UpdateSheetRowsCols(int update)
{
    if (!GetView()) return false;

    bool done = false;    
    const int tableRows = GetNumberRows();
    const int tableCols = GetNumberCols();
    const int sheetRows = GetView()->GetNumberRows();
    const int sheetCols = GetView()->GetNumberCols();
    if (tableRows != sheetRows)
        done |= UpdateSheetRows(tableRows < sheetRows ? tableRows : sheetRows, 
                                tableRows - sheetRows, update);
    if (tableCols != sheetCols)
        done |= UpdateSheetCols(tableCols < sheetCols ? tableCols : sheetCols, 
                                tableCols - sheetCols, update);
    
    return done;
}
