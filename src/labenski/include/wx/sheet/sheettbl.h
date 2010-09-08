///////////////////////////////////////////////////////////////////////////////
// Name:        sheettbl.h
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETTBL_H__
#define __WX_SHEETTBL_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheettbl.h"
#endif

#include "wx/sheet/sheetdef.h"

// ===========================================================================
// wxSheetData - data containers and providers for the table
// ===========================================================================

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
// wxSheetCellValueProviderBase - base class for a data container for wxSheet cells
//   this class must be subclassed - see wxSheetCellData(Sparse)String
// ----------------------------------------------------------------------------

// wxSheetCellValueProvider_Type - values for the wxSheetCellValueProvider options
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
    // Is this cell empty - defaults ueries if the string is empty 
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

    // make this an identical copy of other
    //   the default copies using Get/SetValue and sets only if HasValue
    virtual void Copy(const wxSheetValueProviderBase& other);

    virtual wxSheetValueProviderBase& operator = (const wxSheetValueProviderBase& data)
        { Copy(data); return *this; }
    
protected:
    int m_numRows, m_numCols;
    int m_options;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderBase)
};

// ----------------------------------------------------------------------------
// wxSheetCellStringData - a size optimized container of a 2D array of wxStrings
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
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
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
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderString)
};

// ----------------------------------------------------------------------------
// wxPairArrayIntSheetString - an (int, wxString) pair array for row/col labels
//                             and for grid cols
// wxArrayPairArrayIntSheetString - an array of (int, wxString) pair arrays
// wxPairArrayIntPairArraySheetString - a pair array of (int, wxPairArrayIntSheetString)
//   stores the strings for the sparse table
// ----------------------------------------------------------------------------

DECLARE_PAIRED_INT_DATA_ARRAYS( wxString, wxArrayString, 
                                wxPairArrayIntSheetString, class WXDLLIMPEXP_SHEET)                                  

// ----------------------------------------------------------------------------

WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString,
                              class WXDLLIMPEXP_SHEET);

// ----------------------------------------------------------------------------
DECLARE_PAIRED_INT_DATA_ARRAYS( wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString, 
                                wxPairArrayIntPairArraySheetString, class WXDLLIMPEXP_SHEET)

// ----------------------------------------------------------------------------
// wxSheetCellStringSparseData - a sparse wxString data container class
//   best used when you expect it to be less than about 3/4 full or so
//   since it stores an int for each row and an (int, wxString) for each col
//   it takes the option wxSHEET_ValueProviderColPref 
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
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
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
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderSparseString)
};

// The wxSheetValueProviderHashString is probably not for everyone 
//   UpdateRows/Cols functions are hard to implement reasonably

//   This is mostly a proof of concept and will probably be removed
//     since it's performance is not impressive.
#define wxSHEET_USE_VALUE_PROVIDER_HASH
#ifdef wxSHEET_USE_VALUE_PROVIDER_HASH
// ----------------------------------------------------------------------------
// wxSheetValueProviderHashString - a wxHasMap wxString data container class
//   best used when you expect it to be less than about 3/4 full or so
//   since it stores an int for each row and an (int, wxString) for each col
//   it takes the option wxSHEET_ValueProviderColPref 
// ----------------------------------------------------------------------------
#include "wx/hashmap.h"
WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, wxSheetStringHash );
WX_DECLARE_HASH_MAP( int, wxSheetStringHash, wxIntegerHash, wxIntegerEqual, wxSheetStringHashStringHash );

class WXDLLIMPEXP_SHEET wxSheetValueProviderHashString : public wxSheetValueProviderBase
{
public:
    wxSheetValueProviderHashString(size_t numRows = 0u, size_t numCols = 0u, int options = 0);
    
    //virtual int GetNumberRows() const { return m_numRows; }
    //virtual int GetNumberCols() const { return m_numCols; }
   
    virtual wxString GetValue( const wxSheetCoords& coords ) const;
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );
    virtual bool HasValue( const wxSheetCoords& coords ) const;
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) const;
    virtual void ClearValues() { m_data.clear(); }

    virtual void Clear() { m_data.clear(); m_numRows = m_numCols = 0; }
    virtual bool UpdateRows( size_t row, int numRows );
    virtual bool UpdateCols( size_t col, int numCols );

    virtual void SetOptions(int options);

    // implementation
    const wxSheetStringHashStringHash& GetData() const { return m_data; }
    wxSheetStringHashStringHash& GetData() { return m_data; }
    
protected:
    bool DoUpdateRows( size_t row, int numRows );
    bool DoUpdateCols( size_t col, int numCols );
    wxSheetStringHashStringHash m_data;
    DECLARE_ABSTRACT_CLASS(wxSheetValueProviderHashString)
};
#endif // wxSHEET_USE_VALUE_PROVIDER_HASH

// ----------------------------------------------------------------------------
// wxSheetTable - data provider table for the wxSheet you may want to subclass
//    this to optimize it for your purposes
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetTable : public wxObject, public wxClientDataContainer
{
public:
    wxSheetTable( wxSheet *sheet = NULL );
    virtual ~wxSheetTable();

    // Set/Get the wxSheet "owner" of this data
    virtual void SetView( wxSheet *sheet ) { m_view = sheet; }
    virtual wxSheet* GetView() const { return m_view; }

    // ------------------------------------------------------------------------
    // Get the number of rows and cols
    //  the default uses the number of rows/cols in GetCellData()
    virtual int GetNumberRows();
    virtual int GetNumberCols();
    // determine if row/col/coords is in 0 to GetNumberRows/Cols-1
    bool ContainsGridRow( int row ) { return (row >= 0) && (row < GetNumberRows()); }
    bool ContainsGridCol( int col ) { return (col >= 0) && (col < GetNumberCols()); }
    bool ContainsGridCell(const wxSheetCoords& coords)
        { return ContainsGridRow(coords.m_row) && ContainsGridCol(coords.m_col); }
    // returns true if the coords are within the row/col label cells
    bool ContainsRowLabelCell( const wxSheetCoords& coords )
        { return (coords.m_col == -1) && ContainsGridRow(coords.m_row); }
    bool ContainsColLabelCell( const wxSheetCoords& coords )
        { return (coords.m_row == -1) && ContainsGridCol(coords.m_col); }
    
    // ------------------------------------------------------------------------
    // Cell value handling
        
    // GetValue as a string, coords use -1 notatation for label cells
    //   you must have set the value providers for cell values or subclass 
    //   this and return values on the fly.
    //   the label will return a default value if no provider is set
    virtual wxString GetValue( const wxSheetCoords& coords );
    // SetValue as a string, coords use -1 notatation for label cells
    //   you must have previously set containers 
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );
    // Is this cell empty, uses container's HasValue or !GetValue().IsEmpty()
    virtual bool HasValue( const wxSheetCoords& coords );
    // renderer helper, returns first col < coords.m_col that's !empty
    //  or just return the previous col if you can't implement that
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords );

    // Clear the values of the grid, the defaut can be used to clear
    //   ORed together different enum wxSheetUpdate_Type 
    virtual void ClearValues(int update = wxSHEET_UpdateValues);
        
    // default GetRow/ColLabelValue returns numbers for rows and A-AZ for cols
    //   GetValue is routed to these functions if no value providers for the
    //   labels are set.
    wxString GetDefaultRowLabelValue( int row ) const;
    wxString GetDefaultColLabelValue( int col ) const;
        
    // Specific data type determination and value access

    // try to return GetValue as a long, returns 0 on failure
    virtual long   GetValueAsLong( const wxSheetCoords& coords );
    // try to return GetValue as a double, returns 0 on failure
    virtual double GetValueAsDouble( const wxSheetCoords& coords );
    // try to return GetValue as a bool, 
    //    false is "", "0", "f", "false" w/ no case, true otherwise
    virtual bool   GetValueAsBool( const wxSheetCoords& coords );

    // Set the cell's value as a long "%ld", double "%g", or bool "0" or "1"
    //  the default converts them to a string and calls SetValue
    virtual void SetValueAsLong( const wxSheetCoords& coords, long value );
    virtual void SetValueAsDouble( const wxSheetCoords& coords, double value );
    virtual void SetValueAsBool( const wxSheetCoords& coords, bool value );

    // For user defined types you must cast the void* pointer to your datatype
    //   as well as delete it.
    //   The default implementation does nothing and gives an error if used
    virtual void* GetValueAsCustom( const wxSheetCoords& coords, const wxString& typeName );
    virtual void  SetValueAsCustom( const wxSheetCoords& coords, const wxString& typeName, void* value );
    
    // Returns whether the type is supported, default is wxSHEET_VALUE_STRING
    virtual bool CanGetValueAs( const wxSheetCoords& coords, const wxString& typeName );
    // Can you set the value as type? default returns CanGetValueAs
    virtual bool CanSetValueAs( const wxSheetCoords& coords, const wxString& typeName );

    // Get the type name for the cell, default returns wxEmptyString, if you 
    //  return one of wxSHEET_VALUE_XXX the attributes will return an 
    //  appropriate renderer and editor
    virtual wxString GetTypeName( const wxSheetCoords& coords );

    // Get/Set value providers, may be NULL. 
    //   The GridCellValueProvider is required unless you have subclassed
    //   the table and override GetValue yourself. The label providers
    //   are not necesary and GetDefaultRow/ColLabelValue will be called.
    // The providers will be deleted when done if is_owner = true.
    virtual wxSheetValueProviderBase* GetGridCellValueProvider() const { return m_gridCellValues; }
    virtual wxSheetValueProviderBase* GetRowLabelValueProvider() const { return m_rowLabelValues; }
    virtual wxSheetValueProviderBase* GetColLabelValueProvider() const { return m_colLabelValues; }
    void SetGridCellValueProvider(wxSheetValueProviderBase* gridCellValues, bool is_owner);
    void SetRowLabelValueProvider(wxSheetValueProviderBase* rowLabelValues, bool is_owner);
    void SetColLabelValueProvider(wxSheetValueProviderBase* colLabelValues, bool is_owner);

    // ------------------------------------------------------------------------
    // Attribute handling
   
    // by default forwarded to wxSheetCellAttrProvider if any. May be
    // overridden to handle attributes directly in the table.
    // See wxSheetCellAttrProvider for coords and type meaning
    virtual wxSheetCellAttr GetAttr( const wxSheetCoords& coords,
                                     wxSheetAttr_Type kind );
    
    // See wxSheetCellAttrProvider for coords and type meaning
    virtual void SetAttr( const wxSheetCoords& coords, 
                          const wxSheetCellAttr &attr, 
                          wxSheetAttr_Type kind );

    // get the currently used attr provider (may be NULL)
    virtual wxSheetCellAttrProvider* GetAttrProvider() const { return m_attrProvider; }
    // Set the attr provider to use - take ownership if is_owner
    void SetAttrProvider(wxSheetCellAttrProvider *attrProvider, bool is_owner);
    
    // ------------------------------------------------------------------------
    // Cell spanning - cells may overlap neighbor cells to the right and below, 
    //   hiding the cells they cover. 
    //   By default, the internal selection is created for you, however in a 
    //     subclassed table you may return NULL so long as you don't call 
    //     SetCellSpan or handle it yourself.
    
    // Note: this code should be as efficient as possible since it's called 
    //  numerous times during drawing. It would be nice to have two virtual 
    //  functions Get/Set and use them exculsively, but direct access to the 
    //  selection's array is decidedly faster under many circumstances. 
    //  See DrawAllGridLines and ExpandSpannedBlock for examples

    // Are there any spanned cells at all?
    virtual bool HasSpannedCells();
    // Get a block of the cell, unless a spanned cell it's of size 1,1
    //   note: the top left of block is the 'owner' cell of coords
    virtual wxSheetBlock GetCellBlock( const wxSheetCoords& coords );
    // Set the span of a cell, must be 1x1 or greater. 
    // To remove a spanned cell set it to a cell of size 1x1
    // For grid cells the whole block must be contained within the grid cells 
    //  and if the block intersects a previously spanned cell block the top left 
    //  corners must match up. 
    // Row and Col labels can span cells as well, spanned row labels must have a
    //  width of 1 and a height of >= 1, col labels a height of 1 and width >= 1
    virtual void SetCellSpan( const wxSheetBlock& block );

    virtual wxSheetSelection* GetSpannedBlocks() const { return m_spannedCells; }
    // Set the attr provider to use - take ownership if is_owner
    void SetSpannedBlocks(wxSheetSelection *spannedCells, bool is_owner);

    // ------------------------------------------------------------------------   

    // Update the number of rows cols 
    //   if numRows/Cols < 0 delete, else append if row/col == the current 
    //   number of rows/cols, else insert at row/col
    virtual bool UpdateRows( size_t row, int numRows, int update = wxSHEET_UpdateAll );
    virtual bool UpdateCols( size_t col, int numCols, int update = wxSHEET_UpdateAll );

    // If you have a pure virtual table and you merely want to alert the sheet
    //  that the number of rows/cols have changed then call this function.
    //  It compares the wxSheetTable::GetNumberRows/Cols to 
    //  wxSheet::GetNumberRows/Cols and either appends Rows/Cols or deletes 
    //  them from the end.
    //  You probably won't want to use this if you've set attributes for 
    //  particular cells since this only appends and deletes from the end.
    bool UpdateSheetRowsCols(int update = wxSHEET_UpdateAll );

protected:
    // return update functions back to the sheet calling DoUpdateRows/Cols 
    // if you subclass this you must make sure this is called
    bool UpdateSheetRows( size_t row, int numRows, int update = wxSHEET_UpdateAll );
    bool UpdateSheetCols( size_t col, int numCols, int update = wxSHEET_UpdateAll );

    wxSheet *m_view;                          // 'parent' wxSheet

    wxSheetCellAttrProvider *m_attrProvider;  // attribute provider, if any
    bool m_own_attr_provider;

    wxSheetValueProviderBase *m_gridCellValues; // grid cell values
    bool m_own_grid_cell_values;
    wxSheetValueProviderBase *m_rowLabelValues; // row label values
    bool m_own_row_label_values;
    wxSheetValueProviderBase *m_colLabelValues; // col label values
    bool m_own_col_label_values;

    wxString m_cornerLabelValue;                // single corner label value

    wxSheetSelection* m_spannedCells;           // spanned blocks
    bool m_own_spanned_cells;

    DECLARE_ABSTRACT_CLASS(wxSheetTable)
    DECLARE_NO_COPY_CLASS(wxSheetTable)
};

#endif  // __WX_SHEETTBL_H__
