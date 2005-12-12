///////////////////////////////////////////////////////////////////////////////
// Name:        sheetval.h
// Purpose:     wxSheetValueProviderBase and related classes
// Author:      John Labenski, Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETVAL_H__
#define __WX_SHEETVAL_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheetval.h"
#endif

#include "sheetdef.h"

// ----------------------------------------------------------------------------
// wxArrayStringUpdatePos - an UpdatePos function for wxArrayString
//
// Update the position of the values in the array, by inserting wxEmptyStrings
//  or deleting them. If no_error then don't error out for invalid values.
//  no_error may be useful for arrays that are filled only to the last set value
//     and not to the full extent, therefore an insert beyond the end of this 
//     array should simply be ignored.
// ----------------------------------------------------------------------------
bool wxArrayStringUpdatePos(wxArrayString& arr, size_t pos, int num, bool no_error = false);

// ----------------------------------------------------------------------------
// wxSheetStringArray (wxGridStringArray) a 2d array of wxStrings
// ----------------------------------------------------------------------------

#if wxUSE_GRID
    #include "wx/grid.h"           // get wxGridStringArray from grid 
    typedef wxGridStringArray wxSheetStringArray;
#else // !wxUSE_GRID
    WX_DECLARE_OBJARRAY_WITH_DECL(wxArrayString, wxSheetStringArray,
                                  class WXDLLIMPEXP_SHEET);
#endif // wxUSE_GRID

// ----------------------------------------------------------------------------
// wxSheetCellValueProviderBase - base class for a data container for wxSheet.
//   This class must be subclassed - see wxSheetValueProvider(Sparse)String
// ----------------------------------------------------------------------------

// wxSheetCellValueProvider_Type - wxSheetCellValueProvider options
enum wxSheetValueProvider_Type
{
    wxSHEET_ValueProviderColPref = 0x0001 // optimize for more cols than rows
};

class WXDLLIMPEXP_SHEET wxSheetValueProviderBase : public wxObject
{
public:
    wxSheetValueProviderBase(size_t numRows = 0u, size_t numCols = 0u, int options = 0) 
        :m_numRows(numRows), m_numCols(numCols), m_options(options) {}
    virtual ~wxSheetValueProviderBase() {}

    // Get the size of the data structure (override this as necessary) 
    //   note: these are called often so you may wish to set member vars as necessary
    //   all base class calls that require the number of rows/cols use these functions
    //   if you change the number you must alert the sheet - see wxSheetTable::UpdateSheetXXX
    virtual int GetNumberRows() const { return m_numRows; }
    virtual int GetNumberCols() const { return m_numCols; }
    bool ContainsCell(const wxSheetCoords& coords) const 
        { return (coords.m_row >= 0) && (coords.m_col >= 0) &&
                 (coords.m_row < GetNumberRows()) && 
                 (coords.m_col < GetNumberCols()); }

    // Get/SetValue for a single cell
    virtual wxString GetValue( const wxSheetCoords& coords ) const = 0;
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value ) = 0;
    // Is this cell empty - default queries if the string is empty 
    virtual bool HasValue( const wxSheetCoords& coords ) const
        { return !GetValue(coords).IsEmpty(); }
    // Get the first cell to the left that is not empty, or just return col - 1
    //   this is used for optimizing redrawing
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const
        { return coords.m_col - 1; }
    // Clear all values to be `empty`
    virtual void ClearValues() {}

    // Clear all values and set the number of rows/cols to 0
    //   default implementation uses UpdateRows/Cols(0, -numRows/Cols) 
    virtual void Clear();
    // Update number of rows/cols
    virtual bool UpdateRows( size_t row, int numRows ) = 0;
    virtual bool UpdateCols( size_t col, int numCols ) = 0;

    // Get/Set options - usage depends on the subclassed version
    int GetOptions() const { return m_options; }
    bool HasOption(int mask) const { return (GetOptions() & mask) != 0; }
    virtual void SetOptions(int options) { m_options = options; }

    // Make a full copy of the the other value provider.
    //   The default copies using Get/SetValue and sets only if HasValue
    virtual void Copy(const wxSheetValueProviderBase& other);

    virtual wxSheetValueProviderBase& operator = (const wxSheetValueProviderBase& data)
        { Copy(data); return *this; }
    
protected:
    int m_numRows; // Subclassed versions should set numRows/Cols since 
    int m_numCols; //   GetNumberRows/Cols will be called often
    
    int m_options;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderBase);
};

// ----------------------------------------------------------------------------
// wxSheetValueProviderString - a size optimized container of a 2D array of wxStrings
//   the arrays are filled only to the extent that they're used (Jagged 2d array)
//   it takes the option wxSHEET_ValueProviderColPref 
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_SHEET wxSheetValueProviderString : public wxSheetValueProviderBase
{
public:
    wxSheetValueProviderString(size_t numRows = 0u, size_t numCols = 0u, int options = 0);

    //virtual int GetNumberRows() const { return m_numRows; }
    //virtual int GetNumberCols() const { return m_numCols; }

    virtual wxString GetValue( const wxSheetCoords& coords ) const;
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );
    //virtual bool HasValue( const wxSheetCoords& coords ) const;
    virtual int  GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
    virtual void ClearValues() { m_data.Clear(); }
   
    virtual void Clear() { m_data.Clear(); m_numRows = m_numCols = 0; }
    virtual bool UpdateRows( size_t row, int numRows );
    virtual bool UpdateCols( size_t col, int numCols );

    //int GetOptions() const { return m_options; }
    //bool HasOptions(int mask) const { return (m_options & mask) != 0; }
    virtual void SetOptions(int options);

    // implementation
    const wxSheetStringArray& GetData() const { return m_data; }
    wxSheetStringArray& GetData() { return m_data; }
    
protected:
    bool DoUpdateRows( size_t row, int numRows );
    bool DoUpdateCols( size_t col, int numCols );
    wxSheetStringArray m_data;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderString);
};

// ----------------------------------------------------------------------------
// wxPairArrayIntSheetString - an (int, wxString) pair array for row/col labels
//                             and for grid cols
// wxArrayPairArrayIntSheetString - an array of (int, wxString) pair arrays
// wxPairArrayIntPairArraySheetString - a pair array of (int, wxPairArrayIntSheetString)
//   stores the strings for the sparse table
// ----------------------------------------------------------------------------

DECLARE_PAIRARRAY_INTKEY( wxString, wxArrayString, 
                          wxPairArrayIntSheetString, class WXDLLIMPEXP_SHEET)

// ----------------------------------------------------------------------------

WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString,
                              class WXDLLIMPEXP_SHEET);

// ----------------------------------------------------------------------------
DECLARE_PAIRARRAY_INTKEY( wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString, 
                          wxPairArrayIntPairArraySheetString, class WXDLLIMPEXP_SHEET)

// ----------------------------------------------------------------------------
// wxSheetValueProviderSparseString - a sparse wxString data container class
//   Best used when you expect it to be less than about 3/4 full.
//   Stores an int for each row and an (int, wxString) for each col.
//   It takes the option wxSHEET_ValueProviderColPref 
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_SHEET wxSheetValueProviderSparseString : public wxSheetValueProviderBase
{
public:
    wxSheetValueProviderSparseString(size_t numRows = 0u, size_t numCols = 0u, int options = 0);
    
    //virtual int GetNumberRows() const { return m_numRows; }
    //virtual int GetNumberCols() const { return m_numCols; }
   
    virtual wxString GetValue( const wxSheetCoords& coords ) const;
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );
    virtual bool HasValue( const wxSheetCoords& coords ) const;
    virtual int  GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
    virtual void ClearValues() { m_data.Clear(); }

    virtual void Clear() { m_data.Clear(); m_numRows = m_numCols = 0; }
    virtual bool UpdateRows( size_t row, int numRows );
    virtual bool UpdateCols( size_t col, int numCols );

    virtual void SetOptions(int options);

    // implementation
    void RemoveEmpty(); // if there are no cols then might as well remove row   
    const wxPairArrayIntPairArraySheetString& GetData() const { return m_data; }
    wxPairArrayIntPairArraySheetString& GetData() { return m_data; }
    
protected:
    bool DoUpdateRows( size_t row, int numRows );
    bool DoUpdateCols( size_t col, int numCols );
    wxPairArrayIntPairArraySheetString m_data;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderSparseString);
};

// ----------------------------------------------------------------------------
// THIS IS A TEST FOR SPEED - it's SLOW... DON'T USE
// wxSheetValueProviderSparseStringTest - an (int, wxString) 
// wxArrayPairArrayIntSheetString - an array of (int, wxString) pair arrays
// wxPairArrayIntPairArraySheetString - a pair array of (int, wxPairArrayIntSheetString)
//   stores the strings for the sparse table
// ----------------------------------------------------------------------------

#include "srtobjar.h"

class WXDLLIMPEXP_SHEET wxSheetIntString
{
public:
    wxSheetIntString(int key = 0) : m_key(key) {}
    wxSheetIntString(int key, const wxString& value) : m_key(key), m_value(value) {}
    wxSheetIntString(const wxSheetIntString& i) : m_key(i.m_key), m_value(i.m_value) {}
    int m_key;
    wxString m_value;
    wxSheetIntString& operator=(const wxSheetIntString& other) { m_key = other.m_key; m_value = other.m_value; return *this; }
};

extern const wxSheetIntString wxNullSheetIntString;

extern int wxCMPFUNC_CONV wxSheetIntStringCmp(wxSheetIntString* pItem1, wxSheetIntString* pItem2);
WX_DEFINE_SORTED_ARRAY(wxSheetIntString*, wxSheetIntStringSortedArray);
DECLARE_SORTED_OBJARRAY_INTUPDATEPOS_CMP(wxSheetIntString, wxSheetIntStringSortedArray, 
                                         wxSheetIntStringSortedObjArray, wxSheetIntStringCmp, class WXDLLIMPEXP_SHEET)

extern const wxSheetIntStringSortedObjArray wxNullSheetIntStringSortedObjArray;

// ----------------------------------------------------------------------------

class WXDLLIMPEXP_SHEET wxSheetIntArrayIntString
{
public:
    wxSheetIntArrayIntString(int key = 0) : m_key(key), m_value(new wxSheetIntStringSortedObjArray) {}
    wxSheetIntArrayIntString(int key, wxSheetIntStringSortedObjArray* value) : m_key(key), m_value(value) {}
    wxSheetIntArrayIntString(int key, const wxSheetIntStringSortedObjArray& value) : m_key(key), m_value(new wxSheetIntStringSortedObjArray(value)) {}
    wxSheetIntArrayIntString(const wxSheetIntArrayIntString& i) : m_key(i.m_key), m_value(new wxSheetIntStringSortedObjArray(*i.m_value)) {}
    ~wxSheetIntArrayIntString() { delete m_value; }
    int m_key;
    wxSheetIntStringSortedObjArray *m_value;
    wxSheetIntArrayIntString& operator=(const wxSheetIntArrayIntString& other) { m_key = other.m_key; *m_value = *other.m_value; return *this; }
};

extern int wxCMPFUNC_CONV wxSheetIntArrayIntStringCmp(wxSheetIntArrayIntString* pItem1, wxSheetIntArrayIntString* pItem2);
WX_DEFINE_SORTED_ARRAY(wxSheetIntArrayIntString*, wxSheetIntArrayIntStringSortedArray);
DECLARE_SORTED_OBJARRAY_INTUPDATEPOS_CMP(wxSheetIntArrayIntString, wxSheetIntArrayIntStringSortedArray, 
                                         wxSheetIntArrayIntStringSortedObjArray, wxSheetIntArrayIntStringCmp, class WXDLLIMPEXP_SHEET)

// ----------------------------------------------------------------------------
// wxSheetValueProviderSparseStringTest - a sparse wxString data container class
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_SHEET wxSheetValueProviderSparseStringTest : public wxSheetValueProviderBase
{
public:
    wxSheetValueProviderSparseStringTest(size_t numRows = 0u, size_t numCols = 0u, int options = 0);
    
    //virtual int GetNumberRows() const { return m_numRows; }
    //virtual int GetNumberCols() const { return m_numCols; }
   
    virtual wxString GetValue( const wxSheetCoords& coords ) const;
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );
    virtual bool HasValue( const wxSheetCoords& coords ) const;
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
    virtual void ClearValues() { m_data.Clear(); }

    virtual void Clear() { m_data.Clear(); m_numRows = m_numCols = 0; }
    virtual bool UpdateRows( size_t row, int numRows );
    virtual bool UpdateCols( size_t col, int numCols );

    virtual void SetOptions(int options);

    // implementation
    void RemoveEmpty(); // if there are no cols then might as well remove row   
    const wxSheetIntArrayIntStringSortedObjArray& GetData() const { return m_data; }
    wxSheetIntArrayIntStringSortedObjArray& GetData() { return m_data; }
    
protected:
    wxSheetIntString m_intString;
    wxSheetIntArrayIntString m_intArrayIntString;
    bool DoUpdateRows( size_t row, int numRows );
    bool DoUpdateCols( size_t col, int numCols );
    wxSheetIntArrayIntStringSortedObjArray m_data;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderSparseStringTest);
};

#endif  // __WX_SHEETVAL_H__
