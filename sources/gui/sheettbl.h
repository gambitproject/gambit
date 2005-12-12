///////////////////////////////////////////////////////////////////////////////
// Name:        sheettbl.h
// Purpose:     wxSheetTable and related classes
// Author:      John Labenski, Michael Bedward (based on code by Julian Smart, Robin Dunn)
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

#include "sheetdef.h"
#include "sheetval.h"

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
    //  the default uses the number of rows/cols in GetGridCellValueProvider()
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
    virtual bool UpdateSheetRowsCols(int update = wxSHEET_UpdateAll );

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

    DECLARE_ABSTRACT_CLASS(wxSheetTable);
    DECLARE_NO_COPY_CLASS(wxSheetTable)
};

#endif  // __WX_SHEETTBL_H__
