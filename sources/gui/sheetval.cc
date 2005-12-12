///////////////////////////////////////////////////////////////////////////////
// Name:        sheetval.cpp
// Purpose:     wxSheetValueProviderBase and related classes
// Author:      John Labenski, Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheetval.h"
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

// ----------------------------------------------------------------------------

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayPairArrayIntSheetString)
DEFINE_PAIRARRAY_INTKEY(wxString, wxPairArrayIntSheetString)
DEFINE_PAIRARRAY_INTKEY(wxPairArrayIntSheetString, wxPairArrayIntPairArraySheetString)

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
                wxString::Format(wxT("Called wxArrayStringUpdatePos(pos=%d, N=%d)\nPos value is invalid for present array with %d elements"),
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
// wxSheetValueProviderBase 
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
// wxSheetValueProviderString - base class for a string provider class for the table
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderString, wxSheetValueProviderBase )

wxSheetValueProviderString::wxSheetValueProviderString(size_t numRows, size_t numCols, int options)
                           :wxSheetValueProviderBase(numRows, numCols, options) 
{
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

    const int rowPos = m_data.Index(coords.m_row);
    if (rowPos != wxNOT_FOUND)
        return m_data.ItemValue(rowPos).GetValue(coords.m_col);
    
    return wxEmptyString;
}

void wxSheetValueProviderSparseString::SetValue( const wxSheetCoords& coords_, 
                                                 const wxString& value )
{
    wxCHECK_RET(ContainsCell(coords_), wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    const int rowPos = m_data.Index(coords.m_row);
    
    if (value.IsEmpty())
    {
        // remove the value if empty
        if (rowPos != wxNOT_FOUND)
        {
            m_data.ItemValue(rowPos).Remove(coords.m_col);
            // remove this row if empty
            if (m_data.ItemValue(rowPos).GetCount() == 0)
                m_data.RemoveAt(rowPos);
        }
    }
    else
    {
        if (rowPos == wxNOT_FOUND)
            m_data.GetOrCreateValue(coords.m_row).Add(coords.m_col, value);
        else
            m_data.ItemValue(rowPos).Add(coords.m_col, value);
    }
}

bool wxSheetValueProviderSparseString::HasValue( const wxSheetCoords& coords_ ) const
{
    wxCHECK_MSG(ContainsCell(coords_), false, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    const int rowPos = m_data.Index(coords.m_row);
    if (rowPos == wxNOT_FOUND)
        return false;
    
    return m_data.ItemValue(rowPos).Index(coords.m_col) != wxNOT_FOUND;
}

int wxSheetValueProviderSparseString::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), coords.m_col - 1, wxT("Invalid coords"));
    if (HasOption(wxSHEET_ValueProviderColPref))
    {
        const int rowPos = m_data.Index(coords.m_row);
        if (rowPos != wxNOT_FOUND)
        {
            // pos == 0 meaning nothing to left, == count for col > last filled
            const int colPos = m_data.ItemValue(rowPos).IndexForInsert(coords.m_col);
            if (colPos > 0)
                return m_data.ItemValue(rowPos).ItemKey(colPos-1);
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
        done |= m_data.ItemValue(row).UpdatePos(col, numCols);

    return done;
}

void wxSheetValueProviderSparseString::RemoveEmpty()
{
    int n, count = m_data.GetCount();
    for (n = 0; n < count; n++)
    {
        if (m_data.ItemValue(n).GetCount() == 0)
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
// wxSheetValueProviderSparseStringTest
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetValueProviderSparseStringTest, wxSheetValueProviderBase )

DEFINE_SORTED_OBJARRAY_INTUPDATEPOS(wxSheetIntString, wxSheetIntStringSortedArray, m_key, wxSheetIntStringSortedObjArray)
DEFINE_SORTED_OBJARRAY_INTUPDATEPOS(wxSheetIntArrayIntString, wxSheetIntArrayIntStringSortedArray, m_key, wxSheetIntArrayIntStringSortedObjArray)

const wxSheetIntString wxNullSheetIntString;
const wxSheetIntStringSortedObjArray wxNullSheetIntStringSortedObjArray;

int wxCMPFUNC_CONV wxSheetIntStringCmp(wxSheetIntString* pItem1, wxSheetIntString* pItem2)
{
    return pItem1->m_key - pItem2->m_key;
}
int wxCMPFUNC_CONV wxSheetIntArrayIntStringCmp(wxSheetIntArrayIntString* pItem1, wxSheetIntArrayIntString* pItem2)
{
    return pItem1->m_key - pItem2->m_key;
}

wxSheetValueProviderSparseStringTest::wxSheetValueProviderSparseStringTest(size_t numRows, size_t numCols, int options)
                                 :wxSheetValueProviderBase(0, 0, options) 
{
    m_numRows += numRows;
    m_numCols += numCols;
    DoUpdateRows(0, HasOption(wxSHEET_ValueProviderColPref) ? numRows : numCols);
    DoUpdateCols(0, HasOption(wxSHEET_ValueProviderColPref) ? numCols : numRows);
}

wxString wxSheetValueProviderSparseStringTest::GetValue( const wxSheetCoords& coords_ ) const 
{
    wxCHECK_MSG(ContainsCell(coords_), wxEmptyString, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    ((wxSheetValueProviderSparseStringTest*)this)->m_intArrayIntString.m_key = coords.m_row;
    const int rowPos = m_data.Index((wxSheetIntArrayIntString*)&m_intArrayIntString);
//    const int rowPos = m_data.Index(wxSheetIntArrayIntString(coords.m_row));
    
    if (rowPos != wxNOT_FOUND)
    {
        ((wxSheetValueProviderSparseStringTest*)this)->m_intString.m_key = coords.m_col;
        const int colPos = m_data[rowPos].m_value->Index((wxSheetIntString*)&m_intString);
        //const int colPos = m_data[rowPos].m_value.Index(wxSheetIntString(coords.m_col));
        if (colPos != wxNOT_FOUND)
            return m_data[rowPos].m_value->Item(colPos).m_value;
    }
    
    return wxEmptyString;
}

void wxSheetValueProviderSparseStringTest::SetValue( const wxSheetCoords& coords_, 
                                                     const wxString& value )
{
    wxCHECK_RET(ContainsCell(coords_), wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());

    m_intArrayIntString.m_key = coords.m_row;
    const int rowPos = m_data.Index(&m_intArrayIntString);
    //const int rowPos = m_data.Index(wxSheetIntArrayIntString(coords.m_row));
    
    if (value.IsEmpty())
    {
        // remove the value if empty
        if (rowPos != wxNOT_FOUND)
        {
            m_data[rowPos].m_value->Remove(wxSheetIntString(coords.m_col));
            // remove this row if empty
            if (m_data[rowPos].m_value->GetCount() == 0)
                m_data.RemoveAt(rowPos);
        }
    }
    else
    {
        if (rowPos == wxNOT_FOUND)
        {
            //m_intArrayIntString.m_key = coords.m_row;
            //m_intArrayIntString.m_value = wxSheetIntStringSortedObjArray(wxSheetIntString(coords.m_col, value));
            //m_intArrayIntString.m_value.Add(new wxSheetIntString(coords.m_col, value));
            //m_data.Add(m_intArrayIntString);
            m_data.Add(new wxSheetIntArrayIntString(coords.m_row, new wxSheetIntStringSortedObjArray(new wxSheetIntString(coords.m_col, value))));
        }
        else
            m_data[rowPos].m_value->Add(new wxSheetIntString(coords.m_col, value));
    }
}

bool wxSheetValueProviderSparseStringTest::HasValue( const wxSheetCoords& coords_ ) const
{
    wxCHECK_MSG(ContainsCell(coords_), false, wxT("Invalid coords"));
    wxSheetCoords coords(HasOption(wxSHEET_ValueProviderColPref) ? coords_ : coords_.GetSwapped());
    const int rowPos = m_data.Index(wxSheetIntArrayIntString(coords.m_row));
    if (rowPos == wxNOT_FOUND)
        return false;
    
    return m_data[rowPos].m_value->Index(wxSheetIntString(coords.m_col)) != wxNOT_FOUND;
}

int wxSheetValueProviderSparseStringTest::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), coords.m_col - 1, wxT("Invalid coords"));
    if (HasOption(wxSHEET_ValueProviderColPref))
    {
        const int rowPos = m_data.Index(wxSheetIntArrayIntString(coords.m_row));
        if (rowPos != wxNOT_FOUND)
        {
            // pos == 0 meaning nothing to left, == count for col > last filled
            const int colPos = m_data[rowPos].m_value->IndexForInsert(wxSheetIntString(coords.m_col));
            if (colPos > 0)
                return m_data[rowPos].m_value->Item(colPos-1).m_key;
        }
        return -1;    
    }
    
    return coords.m_col - 1;
}

bool wxSheetValueProviderSparseStringTest::UpdateRows( size_t row, int numRows )
{ 
    wxSHEET_CHECKUPDATE_MSG(row, numRows, m_numRows, false)
    m_numRows += numRows;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateRows(row, numRows) : 
                                                     DoUpdateCols(row, numRows); 
}
bool wxSheetValueProviderSparseStringTest::UpdateCols( size_t col, int numCols )
{ 
    wxSHEET_CHECKUPDATE_MSG(col, numCols, m_numRows, false)
    m_numCols += numCols;
    return HasOption(wxSHEET_ValueProviderColPref) ? DoUpdateCols(col, numCols) : 
                                                     DoUpdateRows(col, numCols); 
}
bool wxSheetValueProviderSparseStringTest::DoUpdateRows( size_t row, int numRows )
{
    return m_data.UpdatePos(row, numRows);
}
bool wxSheetValueProviderSparseStringTest::DoUpdateCols( size_t col, int numCols )
{
    bool done = false;
    size_t row, rowCount = m_data.GetCount();
    for (row=0; row<rowCount; row++)
        done |= m_data[row].m_value->UpdatePos(col, numCols);

    return done;
}

void wxSheetValueProviderSparseStringTest::RemoveEmpty()
{
    int n, count = m_data.GetCount();
    for (n = 0; n < count; n++)
    {
        if (m_data[n].m_value->GetCount() == 0)
        {
            m_data.RemoveAt(n);
            n--;
            count--;
        }
    }
}

void wxSheetValueProviderSparseStringTest::SetOptions(int options)
{ 
    if (options == m_options) return;
    wxSheetValueProviderSparseStringTest data(0,0,options);
    data.Copy(*this);
    m_options = options;
    Copy(data);
}
