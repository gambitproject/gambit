///////////////////////////////////////////////////////////////////////////////
// Name:        sheettbl.cpp
// Purpose:     wxSheetTable and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/sheet/sheettbl.h"
#include "wx/sheet/sheet.h"
#include <cmath>                 // used in GetDefaultColLabelValue

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());
#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#define wxStrF wxString::Format

// ----------------------------------------------------------------------------

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayPairArrayIntSheetString)
DEFINE_PAIRED_INT_DATA_ARRAYS(wxString, wxPairArrayIntSheetString)
DEFINE_PAIRED_INT_DATA_ARRAYS(wxPairArrayIntSheetString, wxPairArrayIntPairArraySheetString)

#if !wxUSE_GRID
    WX_DEFINE_OBJARRAY(wxSheetStringArray) // else use wxGridStringArray 
#endif // wxUSE_GRID

// ----------------------------------------------------------------------------
// wxArrayStringUpdatePos - UpdatePos for wxArrayString
// ----------------------------------------------------------------------------

bool wxArrayStringUpdatePos(wxArrayString& arr, size_t pos, int num, bool no_error)
{
    if (num == 0)
        return false;
    else if (num > 0)
    {
        // if no_error check if <= GetCount, else let array error out
        if (!no_error || (pos <= arr.GetCount()))
            arr.Insert( wxEmptyString, pos, num );
    }
    else // if (num < 0)
    {
        const int count = arr.GetCount();
        if (no_error && (int(pos) - num > count))
        {
            num = -(count - int(pos));
            if ((num >= 0) || (count == 0)) return false;
        }
        else
        {
            wxCHECK_MSG( int(pos) - num <= count, false,
                         wxStrF(wxT("Called wxArrayStringUpdatePos(pos=%d, N=%d)\nPos value is invalid for present array with %d elements"),
                                int(pos), num, count) );
        }
    
        if ((pos == 0u) && (num == count))
            arr.Clear();
        else
            arr.RemoveAt( pos, -num );
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxSheetDataBase 
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderBase, wxObject )

void wxSheetValueProviderBase::Clear()
{
    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    if (numRows > 0) UpdateRows(0, -numRows);
    if (numCols > 0) UpdateCols(0, -numCols);
}

void wxSheetValueProviderBase::Copy(const wxSheetValueProviderBase& other)
{
    Clear();
    const int numRows = other.GetNumberRows();
    const int numCols = other.GetNumberCols();
    m_options = other.GetOptions();
    UpdateRows(0, numRows);
    UpdateCols(0, numCols);
    wxSheetCoords c;
        
    for (c.m_row = 0; c.m_row < numRows; c.m_row++)
    {
        for (c.m_col = 0; c.m_col < numCols; c.m_col++)
        {
            wxString val(other.GetValue(c));
            if (!val.IsEmpty())
                SetValue(c, val);
        }
    }
}

// ----------------------------------------------------------------------------
// wxSheetStringData - base class for a string provider class for the table
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderString, wxSheetValueProviderBase )

wxSheetValueProviderString::wxSheetValueProviderString(size_t numRows, size_t numCols, int options)
                           :wxSheetValueProviderBase(0, 0, options) 
{
    m_numRows += numRows;
    m_numCols += numCols;
    DoUpdateRows(0, HasOption(wxSHEET_ValueProviderColPref) ? numRows : numCols);
    DoUpdateCols(0, HasOption(wxSHEET_ValueProviderColPref) ? numCols : numRows);
}

wxString wxSheetValueProviderString::GetValue( const wxSheetCoords& coords_ ) const 
{ 
    wxCHECK_MSG(ContainsCell(coords_), wxEmptyString, wxT("Invalid coords"));    
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    //wxPrintf(wxT("RC %d %d - NumRC %d %d DataRC %d %d '%s'\n"), coords_.m_row, coords_.m_col, m_numRows, m_numCols, 
    //    m_data.GetCount(), int(m_data.GetCount()) > coords.m_col ? m_data[coords.m_col].GetCount() : 0,
    //    wxDateTime::Now().FormatISOTime().c_str());
    
    if ((int(m_data.GetCount()) > coords.m_row) && 
        (int(m_data[coords.m_row].GetCount()) > coords.m_col))
        return m_data[coords.m_row][coords.m_col];
    
    return wxEmptyString;
}
void wxSheetValueProviderString::SetValue( const wxSheetCoords& coords_, const wxString& value )
{ 
    wxCHECK_RET(ContainsCell(coords_), wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    
    // add "rows" as necessary to store value
    int count = m_data.GetCount();
    if (count <= coords.m_row) 
    {
        wxArrayString sa;
        sa.Add( wxEmptyString, 1+coords.m_col );
        m_data.Insert( sa, count, 1+coords.m_row-count );
    }
    else // believe it or not - NOT having this else statement is 10% faster in gcc
    {
        // add "cols" as necessary to store value
        count = m_data[coords.m_row].GetCount();
        if (count <= coords.m_col)
        {
            m_data.Item(coords.m_row).Insert( wxEmptyString, count, 1+coords.m_col-count );
        }
    }    
    m_data[coords.m_row][coords.m_col] = value;
}

int wxSheetValueProviderString::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), coords.m_col - 1, wxT("Invalid coords"));
    if (HasOption(wxSHEET_ValueProviderColPref)) 
    {
        if (int(m_data.GetCount()) <= coords.m_row)
            return -1;
        if (int(m_data[coords.m_row].GetCount()) < coords.m_col)
            return m_data[coords.m_row].GetCount() - 1;
    }
    // the else case cannot be optimized - just use default
    return coords.m_col - 1;
}

bool wxSheetValueProviderString::UpdateRows( size_t row, int numRows )
{ 
    wxSHEET_CHECKUPDATE_MSG(row, numRows, m_numRows, false);
    m_numRows += numRows;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateRows(row, numRows) : 
                                                     DoUpdateCols(row, numRows); 
}
bool wxSheetValueProviderString::UpdateCols( size_t col, int numCols )
{ 
    wxSHEET_CHECKUPDATE_MSG(col, numCols, m_numCols, false);
    m_numCols += numCols;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateCols(col, numCols) : 
                                                     DoUpdateRows(col, numCols); 
}

bool wxSheetValueProviderString::DoUpdateRows( size_t row, int numRows )
{
    // get things right before calling this, remember the rows may not be 
    //  filled completely and this won't error out
    const int curNumRows = m_data.GetCount();
    //const int curNumCols = curNumRows > 0 ? m_data.Item(0).GetCount() : m_numCols;

    if (numRows == 0)
        return false;
    else if (numRows > 0)
    {
        if (curNumRows > int(row)) // only insert, no need to append
        {
            wxArrayString sa;       
            m_data.Insert( sa, row, numRows );
        }
    }
    else // if (numRows < 0)
    {
        if (curNumRows > int(row))
        {
            m_data.RemoveAt( row, wxMin(curNumRows-int(row), -numRows) );
        }
    }

    return true;
}
bool wxSheetValueProviderString::DoUpdateCols( size_t col, int numCols )
{
    // only insert or delete cols, no need to append    
    const int curNumRows = m_data.GetCount();

    if (numCols == 0)
        return false;
    else if (numCols > 0)
    {
        for ( int row = 0; row < curNumRows; row++ )
        {
            if (m_data[row].GetCount() > col) 
                m_data[row].Insert( wxEmptyString, col, numCols );
        }
    }
    else // if (numCols < 0)
    {
        for ( int row = 0; row < curNumRows; row++ )
        {
            const int curNumCols = m_data[row].GetCount();
            if (curNumCols > int(col))
            {
                m_data[row].RemoveAt( col, wxMin(curNumCols-int(col), -numCols) );
            }
        }
    }

    return true;
}

void wxSheetValueProviderString::SetOptions(int options)
{ 
    if (options == m_options) return;
    wxSheetValueProviderString data(0,0,options);
    data.Copy(*this);
    m_options = options;
    Copy(data);
}

//-----------------------------------------------------------------------------
// wxPairArrayIntPairArraySheetString - string data
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderSparseString, wxSheetValueProviderBase )

wxSheetValueProviderSparseString::wxSheetValueProviderSparseString(size_t numRows, size_t numCols, int options)
                                 :wxSheetValueProviderBase(0, 0, options) 
{
    m_numRows += numRows;
    m_numCols += numCols;
    DoUpdateRows(0, HasOption(wxSHEET_ValueProviderColPref) ? numRows : numCols);
    DoUpdateCols(0, HasOption(wxSHEET_ValueProviderColPref) ? numCols : numRows);
}

wxString wxSheetValueProviderSparseString::GetValue( const wxSheetCoords& coords_ ) const 
{
    wxCHECK_MSG(ContainsCell(coords_), wxEmptyString, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    const int rowPos = m_data.FindIndex(coords.m_row);
    if (rowPos != wxNOT_FOUND)
        return m_data.GetItemValue(rowPos).GetValue(coords.m_col);
    
    return wxEmptyString;
}

void wxSheetValueProviderSparseString::SetValue( const wxSheetCoords& coords_, 
                                                 const wxString& value )
{
    wxCHECK_RET(ContainsCell(coords_), wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    const int rowPos = m_data.FindIndex(coords.m_row);
    
    if (value.IsEmpty())
    {
        // remove the value if empty
        if (rowPos != wxNOT_FOUND)
        {
            m_data.GetItemValue(rowPos).RemoveValue(coords.m_col);
            // remove this row if empty
            if (m_data.GetItemValue(rowPos).GetCount() == 0)
                m_data.RemoveAt(rowPos);
        }
    }
    else
    {
        if (rowPos == wxNOT_FOUND)
            m_data.GetOrCreateValue(coords.m_row).SetValue(coords.m_col, value);
        else
            m_data.GetItemValue(rowPos).SetValue(coords.m_col, value);
    }
}

bool wxSheetValueProviderSparseString::HasValue( const wxSheetCoords& coords_ ) const
{
    wxCHECK_MSG(ContainsCell(coords_), false, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    const int rowPos = m_data.FindIndex(coords.m_row);
    if (rowPos == wxNOT_FOUND)
        return false;
    
    return m_data.GetItemValue(rowPos).FindIndex(coords.m_col) != wxNOT_FOUND;
}

int wxSheetValueProviderSparseString::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), coords.m_col - 1, wxT("Invalid coords"));
    if (HasOption(wxSHEET_ValueProviderColPref))
    {
        const int rowPos = m_data.FindIndex(coords.m_row);
        if (rowPos != wxNOT_FOUND)
        {
            // pos == 0 meaning nothing to left, == count for col > last filled
            const int colPos = m_data.GetItemValue(rowPos).FindInsertIndex(coords.m_col);
            if (colPos > 0)
                return m_data.GetItemValue(rowPos).GetItemKey(colPos-1);
        }
        return -1;    
    }
    
    return coords.m_col - 1;
}

bool wxSheetValueProviderSparseString::UpdateRows( size_t row, int numRows )
{ 
    wxSHEET_CHECKUPDATE_MSG(row, numRows, m_numRows, false)
    m_numRows += numRows;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateRows(row, numRows) : 
                                                     DoUpdateCols(row, numRows); 
}
bool wxSheetValueProviderSparseString::UpdateCols( size_t col, int numCols )
{ 
    wxSHEET_CHECKUPDATE_MSG(col, numCols, m_numRows, false)
    m_numCols += numCols;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateCols(col, numCols) : 
                                                     DoUpdateRows(col, numCols); 
}
bool wxSheetValueProviderSparseString::DoUpdateRows( size_t row, int numRows )
{
    return m_data.UpdatePos(row, numRows);
}
bool wxSheetValueProviderSparseString::DoUpdateCols( size_t col, int numCols )
{
    bool done = false;
    size_t row, rowCount = m_data.GetCount();
    for (row=0; row<rowCount; row++)
        done |= m_data.GetItemValue(row).UpdatePos(col, numCols);

    return done;
}

void wxSheetValueProviderSparseString::RemoveEmpty()
{
    int n, count = m_data.GetCount();
    for (n = 0; n < count; n++)
    {
        if (m_data.GetItemValue(n).GetCount() == 0)
        {
            m_data.RemoveAt(n);
            n--;
            count--;
        }
    }
}

void wxSheetValueProviderSparseString::SetOptions(int options)
{ 
    if (options == m_options) return;
    wxSheetValueProviderSparseString data(0,0,options);
    data.Copy(*this);
    m_options = options;
    Copy(data);
}

//-----------------------------------------------------------------------------
// wxSheetValueProviderHashString - string data
//-----------------------------------------------------------------------------
#ifdef wxSHEET_USE_VALUE_PROVIDER_HASH
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderHashString, wxSheetValueProviderBase )

wxSheetValueProviderHashString::wxSheetValueProviderHashString(size_t numRows, size_t numCols, int options)
                               :wxSheetValueProviderBase(0, 0, options) 
{
    m_numRows += numRows;
    m_numCols += numCols;
    // FIXME UpdateRows/Cols for wxSheetValueProviderHashString 
    //DoUpdateRows(0, HasOption(wxSHEET_ValueProviderColPref) ? numRows : numCols);
    //DoUpdateCols(0, HasOption(wxSHEET_ValueProviderColPref) ? numCols : numRows);
}

wxString wxSheetValueProviderHashString::GetValue( const wxSheetCoords& coords_ ) const 
{
    wxCHECK_MSG(ContainsCell(coords_), wxEmptyString, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    wxSheetStringHashStringHash::const_iterator row_iter = m_data.find(coords.m_row);
    if (row_iter != m_data.end())
    {
        wxSheetStringHash::const_iterator col_iter = row_iter->second.find(coords.m_col);
        if (col_iter != row_iter->second.end())
            return col_iter->second;
    }    
    return wxEmptyString;
}

void wxSheetValueProviderHashString::SetValue( const wxSheetCoords& coords_, 
                                                 const wxString& value )
{
    wxCHECK_RET(ContainsCell(coords_), wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    wxSheetStringHashStringHash::iterator row_iter = m_data.find(coords.m_row);
    if (row_iter == m_data.end())
    {
        wxSheetStringHash colHash;
        colHash[coords.m_col] = value;
        m_data[coords.m_row] = colHash;
        return;
    }

    row_iter->second[coords.m_col] = value;
}

bool wxSheetValueProviderHashString::HasValue( const wxSheetCoords& coords_ ) const
{
    wxCHECK_MSG(ContainsCell(coords_), false, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    wxSheetStringHashStringHash::const_iterator row_iter = m_data.find(coords.m_row);
    if (row_iter != m_data.end())
    {
        wxSheetStringHash::const_iterator col_iter = row_iter->second.find(coords.m_col);
        if (col_iter != row_iter->second.end())
            return true;
    }    
    return false;
}

int wxSheetValueProviderHashString::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), coords.m_col - 1, wxT("Invalid coords"));
    if (HasOption(wxSHEET_ValueProviderColPref))
    {
        wxSheetStringHashStringHash::const_iterator row_iter = m_data.find(coords.m_row);
        if (row_iter == m_data.end())
            return -1;
    }
    
    return coords.m_col - 1;
}

bool wxSheetValueProviderHashString::UpdateRows( size_t row, int numRows )
{ 
    wxSHEET_CHECKUPDATE_MSG(row, numRows, m_numRows, false)
    bool append = (int(row) == m_numRows);
    m_numRows += numRows;
    if (!m_numCols || !m_numRows)
    {
        ClearValues();
        return true;
    }
        
    if (!append)
        return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateRows(row, numRows) : 
                                                         DoUpdateCols(row, numRows); 
    return true;
}
bool wxSheetValueProviderHashString::UpdateCols( size_t col, int numCols )
{ 
    wxSHEET_CHECKUPDATE_MSG(col, numCols, m_numRows, false)
    bool append = (int(col) == m_numRows);
    m_numCols += numCols;
    if (!m_numCols || !m_numRows)
    {
        ClearValues();
        return true;
    }
    if (!append)
        return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateCols(col, numCols) : 
                                                         DoUpdateRows(col, numCols);     
    return true;
}

bool wxSheetValueProviderHashString::DoUpdateRows( size_t row, int numRows )
{
    if (numRows == 0)
        return false;
    else if (numRows > 0)
    {
        if (!m_data.empty()) // FIXME only insert, already checked append
        {
            wxFAIL_MSG(wxT("Not implemented"));
        }
    }
    else // if (numRows < 0)
    {
        const int row_end = row + numRows;
        wxSheetStringHashStringHash::iterator row_iter = m_data.begin();
        while (row_iter != m_data.end())
        {
            int data_row = row_iter->first;
            if ((data_row >= int(row)) && (data_row < row_end))
            {
                wxSheetStringHashStringHash::iterator it = row_iter;
                row_iter++;
                m_data.erase(it);
            }
            else
                row_iter++;
        }
    }

    return true;
}

bool wxSheetValueProviderHashString::DoUpdateCols( size_t col, int numCols )
{
    if (numCols == 0)
        return false;
    else if (numCols > 0)
    {
        if (!m_data.empty()) // FIXME only insert, already checked append
        {
            wxFAIL_MSG(wxT("Not implemented"));
        }
    }
    else // if (numCols < 0)
    {
        const int col_end = col + numCols;
        wxSheetStringHashStringHash::iterator row_iter = m_data.begin();
        while (row_iter != m_data.end())
        {
            wxSheetStringHash::iterator col_iter = row_iter->second.begin();
            while (col_iter != row_iter->second.end())
            {
                int data_col = col_iter->first;
                if ((data_col >= int(col)) && (data_col < col_end))
                {
                    wxSheetStringHash::iterator it = col_iter;
                    col_iter++;
                    row_iter->second.erase(it);
                }
                else
                    col_iter++;
            }
            row_iter++;
        }        
    }

    return true;
}

void wxSheetValueProviderHashString::SetOptions(int options)
{ 
    if (options == m_options) return;
    wxSheetValueProviderHashString data(0,0,options);
    data.Copy(*this);
    m_options = options;
    Copy(data);
}
#endif // wxSHEET_USE_VALUE_PROVIDER_HASH

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
/*    
    FIXME should add clearing attrs here
    if (((update & wxSHEET_UpdateGridCellAttrs) != 0) && GetAttrProvider())
        GetAttrProvider()->
    if (((update & wxSHEET_UpdateRowLabelAttrs) != 0) && GetAttrProvider())
        GetAttrProvider()->
    if (((update & wxSHEET_UpdateColLabelAttrs) != 0) && GetAttrProvider())
        GetAttrProvider()->
*/    
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
    const size_t chars = size_t(log((double) col)/3.2580965380); // log_26(x) = log(x)/log(26.0)
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
    // Ok to return nothing
    if (GetAttrProvider())
        return GetAttrProvider()->GetAttr(coords, kind);

    return wxNullSheetCellAttr;
}

void wxSheetTable::SetAttr( const wxSheetCoords& coords, 
                            const wxSheetCellAttr& attr, wxSheetAttr_Type kind)
{
    // This is called from the 
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
    //int index = wxNOT_FOUND;

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
    
    return UpdateSheetCols(col, numCols, update);
}

bool wxSheetTable::UpdateSheetRows( size_t row, int numRows, int update )
{
    return GetView() && GetView()->DoUpdateRows(row, numRows, update);
}
bool wxSheetTable::UpdateSheetCols( size_t col, int numCols, int update )
{
    return GetView() && GetView()->DoUpdateCols(col, numCols, update);
}
bool wxSheetTable::UpdateSheetRowsCols(int update)
{
    bool done = false;
    if (!GetView()) return done;
    
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
