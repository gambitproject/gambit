///////////////////////////////////////////////////////////////////////////////
// Name:        sheet.h
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEET_H__
#define __WX_SHEET_H__

#include "wx/sheet/sheetdef.h" // Always include sheetdef.h first
#include "wx/sheet/sheetatr.h" // might as well include everything
#include "wx/sheet/sheetsel.h"
#include "wx/sheet/sheetedt.h"
#include "wx/sheet/sheetren.h"
#include "wx/sheet/sheettbl.h"
#include "wx/sheet/sheetedg.h"
#include "wx/window.h"
#include "wx/scrolbar.h"
#include "wx/clntdata.h"

class WXDLLEXPORT wxTimer;
class WXDLLEXPORT wxTimerEvent;

// ----------------------------------------------------------------------------
// wxSheetChildWindow - a child window of the wxSheet, forwards events to sheet
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetChildWindow : public wxWindow
{
public:
    wxSheetChildWindow( wxSheet *parent, wxWindowID id = wxID_ANY,
                        const wxPoint &pos = wxDefaultPosition, 
                        const wxSize &size = wxDefaultSize,
                        long style = wxWANTS_CHARS|wxBORDER_NONE|wxCLIP_CHILDREN, //|wxFULL_REPAINT_ON_RESIZE ) // FIXME
                        const wxString& name = wxT("wxSheetChildWindow") );

    // implementation
    void OnPaint( wxPaintEvent& event );
    void OnMouse( wxMouseEvent& event );
    void OnKeyAndChar( wxKeyEvent& event );
    void OnFocus( wxFocusEvent& event );
    void OnEraseBackground( wxEraseEvent& ); 

    wxSheet* GetOwner() const { return m_owner; }
    
    wxSheet *m_owner;
    int      m_mouseCursor; // remember the last cursor set for this window
    
private:
    DECLARE_ABSTRACT_CLASS(wxSheetChildWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetChildWindow)
};

//-----------------------------------------------------------------------------
// wxSheetRefData - ref counted data to share with refed wxSheets
//
// Please use accessors in the wxSheet to adjust the values, for very 
// complicated scenarios adjust these as a last resort. 
// 
// This is separated from the wxSheetTable since these are required for the
// sheet and the table may not have been assigned yet.
// 
// This MUST always exist as the wxObject::m_refData of the wxSheet. It is 
// automatically created in wxSheet::Init, if you wish to use your own 
// subclassed refdata then in your wxSheet constructor (or Create) function call
// UnRef to delete the original and m_refData=new MySheetRefData to set the new.
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetRefData : public wxObjectRefData
{
public:
    wxSheetRefData();
    virtual ~wxSheetRefData();
    // Find/Add/Remove sheets that share this data - used for wxSheetSplitter
    int  FindSheet(wxSheet* sheet) const;
    bool HasSheet(wxSheet* sheet) const { return FindSheet(sheet) != wxNOT_FOUND; }
    void AddSheet(wxSheet* sheet);
    void RemoveSheet(wxSheet* sheet);
    wxSheet *GetSheet(size_t n) const { return (wxSheet*)m_sheets.Item(n); }
    size_t GetSheetCount() const { return m_sheets.GetCount(); }

    // implementation ---------------------------------------------------------
    
    wxArrayPtrVoid m_sheets;             // list of sheets sharing this data

    wxSheetTable *m_table;               // the table that stores the values
    bool          m_ownTable;

    wxSheetTypeRegistry* m_typeRegistry; // editor/renderer registry

    wxSheetArrayEdge m_rowEdges;         // calculates edge positions and
    wxSheetArrayEdge m_colEdges;         // stores num rows/cols

    // width of row labels and height of col labels
    int      m_rowLabelWidth;
    int      m_colLabelHeight;

    // the size of the margin to the right and bottom of the cell area
    wxSize   m_marginSize;

    int      m_equal_col_widths;

    wxColour m_gridLineColour;
    int      m_gridLinesEnabled;
    wxColour m_cursorCellHighlightColour;
    int      m_cursorCellHighlightPenWidth;
    int      m_cursorCellHighlightROPenWidth;
    wxColour m_labelOutlineColour;

    wxCursor m_rowResizeCursor;
    wxCursor m_colResizeCursor;

    bool     m_editable;    // applies to whole grid - brute force readonly

    int      m_canDrag;     // enum wxSheetDragCellSize_Type

    wxSheetCoords m_cursorCoords;

    wxSheetBlock      m_selectingBlock;   // during selection !Empty
    wxSheetCoords     m_selectingAnchor;  // corner of active selection, other is usually cursor
    wxSheetSelection* m_selection;
    int               m_selectionMode;    // wxSheetSelectionMode_Type
    wxColour          m_selectionBackground;
    wxColour          m_selectionForeground;

    // the default cell attr objects for cells that don't have their own
    wxSheetCellAttr m_defaultGridCellAttr;
    wxSheetCellAttr m_defaultRowLabelAttr;
    wxSheetCellAttr m_defaultColLabelAttr;
    wxSheetCellAttr m_defaultCornerLabelAttr;
    
    wxSheetCellEditor m_cellEditor;       // valid only when editing
    wxSheetCoords     m_cellEditorCoords;

    wxSheetValueProviderSparseString m_copiedData;
    bool                             m_pasting;
};

// ----------------------------------------------------------------------------
// wxSheet
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheet : public wxWindow
{
public:
    wxSheet() : wxWindow(), m_gridWin(NULL) { Init(); }
    
    wxSheet( wxWindow *parent, wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxWANTS_CHARS,
             const wxString& name = wxT("wxSheet") ) : wxWindow(), m_gridWin(NULL)
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    bool Create( wxWindow *parent, wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxWANTS_CHARS,
                 const wxString& name = wxT("wxSheet") );

    virtual ~wxSheet();
    virtual bool Destroy();

    // ref another wxSheet's ref data - see usage in wxSheetSplitter
    void RefSheet(wxSheet* otherSheet);
    // Create a new wxSheet with same parent, used for wxSheetSplitter.
    //   override this so that the top left sheet in the splitter can return
    //   a "new MySheet" for the other sheets as necessary
    //   This is one of the few ways for the splitter to create new sheets.
    virtual wxSheet* Clone(wxWindowID id = wxID_ANY);

    // override wxWindow::Enable to ensure proper refresh
    virtual bool Enable(bool enable = true);

    // ------------------------------------------------------------------------
    // Create/Set/Get wxSheetTable - the underlying data to be displayed

    wxSheetTable* GetTable() const { return GetSheetRefData()->m_table; }
    // Set your own table derived from wxSheetTable, if takeOwnership the
    //   the table will be deleted when this window is destroyed
    bool SetTable( wxSheetTable *table, bool takeOwnership );  
    
    // Create a wxSheetTable using string data containers to use
    //   see this function to see how to setup the table and use SetTable 
    //   for the case where you want to customize things
    //   This function exists to show how to  create and assign tables
    bool CreateGrid( int numRows, int numCols, int options = 0 );

    // ------------------------------------------------------------------------
    // Dimensions of the number of cells on the sheet and helper cell functions
    
    int GetNumberRows() const { return GetSheetRefData()->m_rowEdges.GetCount(); }
    int GetNumberCols() const { return GetSheetRefData()->m_colEdges.GetCount(); }

    // Is the coords anywhere in labels or grid, -1 to GetNumberRows/Cols()-1
    bool ContainsCell( const wxSheetCoords& coords ) const 
        { return (coords.m_row >= -1) && (coords.m_col >= -1) &&
                 (coords.m_row < GetNumberRows()) && 
                 (coords.m_col < GetNumberCols()); }
                 
    // returns true if the coords are within the grid cells of the sheet
    bool ContainsGridRow( int row ) const { return (row >= 0) && (row < GetNumberRows()); }
    bool ContainsGridCol( int col ) const { return (col >= 0) && (col < GetNumberCols()); }
    bool ContainsGridCell( const wxSheetCoords& coords ) const
        { return ContainsGridRow(coords.m_row) && ContainsGridCol(coords.m_col); }

    // returns true if the coords are within the row/col label cells
    bool ContainsRowLabelCell( const wxSheetCoords& coords ) const
        { return (coords.m_col == -1) && ContainsGridRow(coords.m_row); }
    bool ContainsColLabelCell( const wxSheetCoords& coords ) const
        { return (coords.m_row == -1) && ContainsGridCol(coords.m_col); }

    // static helper functions to determine what type of cell it is, not check validity
        
    static bool IsGridCell(const wxSheetCoords& coords)     
        { return (coords.m_row >=  0) && (coords.m_col >=  0); }
    static bool IsLabelCell(const wxSheetCoords& coords)
        { return (coords.m_row >= -1) && (coords.m_col >= -1) &&
                ((coords.m_row == -1) || (coords.m_col == -1)); }
    static bool IsRowLabelCell(const wxSheetCoords& coords) 
        { return (coords.m_row >=  0) && (coords.m_col == -1); }
    static bool IsColLabelCell(const wxSheetCoords& coords) 
        { return (coords.m_row == -1) && (coords.m_col >=  0); }
    static bool IsCornerLabelCell(const wxSheetCoords& coords) 
        { return (coords.m_row == -1) && (coords.m_col == -1); }
    
    // Get an enum value of what window the coords are meant for
    static wxSheetCell_Type GetCellCoordsType(const wxSheetCoords& coords);
        
    // "clear" the contents of the grid (depends on table's Clear() function)
    //   the string implementations clear the cell values, not the # rows/cols
    void ClearValues(int update = wxSHEET_UpdateValues); 
        
    // Insert/Add/DeleteRows/Cols to the grid cells
    //   update contains or'ed values of enum wxSheetUpdate_Type.
    //   you proabably want UpdateAll unless you ensure that no problems will occur
    //   or you will update some other way
    bool InsertRows( size_t row, size_t numRows, int update = wxSHEET_UpdateAll )
        { return UpdateRows(row, numRows, update); }
    bool AppendRows( size_t numRows, int update = wxSHEET_UpdateAll )
        { return UpdateRows(GetNumberRows(), numRows, update); }
    bool DeleteRows( size_t row, size_t numRows, int update = wxSHEET_UpdateAll )
        { return UpdateRows(row, -int(numRows), update); }
        
    bool InsertCols( size_t col, size_t numCols, int update = wxSHEET_UpdateAll )
        { return UpdateCols(col, numCols, update); }
    bool AppendCols( size_t numCols, int update = wxSHEET_UpdateAll )
        { return UpdateCols(GetNumberCols(), numCols, update); }
    bool DeleteCols( size_t col, size_t numCols, int update = wxSHEET_UpdateAll )
        { return UpdateCols(col, -int(numCols), update); }

    // Set exactly the number of rows or cols, these functions Append or
    // Delete rows/cols to/from the end. If you are setting attributes for 
    // particular cells/rows/cols you probably won't want to use these
    bool SetNumberRows( size_t rows, int update = wxSHEET_UpdateAll );
    bool SetNumberCols( size_t cols, int update = wxSHEET_UpdateAll );
    bool SetNumberCells( size_t rows, size_t cols, int update = wxSHEET_UpdateAll ) 
        { return SetNumberRows(rows, update) || SetNumberCols(cols, update); }
    
    // Inserting/Appending/Deleting rows/cols functions are forwarded here
    //   and then sent to the wxSheetTable::UpdateRows/Cols functions
    //   numRows/Cols > 0 : InsertRows/Cols at row/col else if < 0 delete
    //   row/col == GetNumberRows/Cols && numRows/Cols > 0 to append to end
    virtual bool UpdateRows( size_t row, int numRows, int update = wxSHEET_UpdateAll );
    virtual bool UpdateCols( size_t col, int numCols, int update = wxSHEET_UpdateAll );
        
    // ------------------------------------------------------------------------
    // Dimensions of the row and column sizes
    
    // Get/Set the default height/width of newly created rows/cols
    //   if resizeExisting then resize all to match the newly set default
    int  GetDefaultRowHeight() const { return GetSheetRefData()->m_rowEdges.GetDefaultSize(); }
    int  GetDefaultColWidth()  const { return GetSheetRefData()->m_colEdges.GetDefaultSize(); }
    void SetDefaultRowHeight( int height, bool resizeExistingRows = false );
    void SetDefaultColWidth( int width, bool resizeExistingCols = false );
    
    // Get/Set the absolute min row/col width/height, 0 for no min size
    //   Call during grid creation, existing rows/cols are not resized
    //   This value is used when dragging cell size with the mouse if no 
    //   particular min size for a row/col has been set
    int  GetMinimalAcceptableRowHeight() const { return GetSheetRefData()->m_rowEdges.GetMinAllowedSize(); }
    int  GetMinimalAcceptableColWidth()  const { return GetSheetRefData()->m_colEdges.GetMinAllowedSize(); }
    void SetMinimalAcceptableRowHeight(int height) { GetSheetRefData()->m_rowEdges.SetMinAllowedSize(height); }
    void SetMinimalAcceptableColWidth(int width)   { GetSheetRefData()->m_colEdges.SetMinAllowedSize(width); }

    // Don't allow specific rows/cols to be resized smaller than this
    //   Call during grid creation, existing rows/cols are not resized
    //   The setting is cleared to default val if width/height is < min acceptable
    int  GetMinimalRowHeight(int row) const { return GetSheetRefData()->m_rowEdges.GetMinSize(row); }
    int  GetMinimalColWidth(int col) const  { return GetSheetRefData()->m_colEdges.GetMinSize(col); }
    void SetMinimalRowHeight(int row, int height) { GetSheetRefData()->m_rowEdges.SetMinSize(row, height); }
    void SetMinimalColWidth(int col, int width)   { GetSheetRefData()->m_colEdges.SetMinSize(col, width); }

    // Set the height of a row or width of a col, -1 notation for labels
    //  use height/width = -1 to autosize from the row/col labels
    void SetRowHeight( int row, int height );
    void SetColWidth(  int col, int width );
    
    // Get the height/top/bottom for rows, uses -1 notation
    int GetRowHeight(int row) const;
    int GetRowTop(int row) const;
    int GetRowBottom(int row) const;
    // Get the width/left/right for cols, uses -1 notation
    int GetColWidth(int col) const;
    int GetColLeft(int col) const;
    int GetColRight(int col) const;
    // Get the width, height of a cell as a wxSize, -1 notation
    //  this does not include spanned cells
    wxSize GetCellSize(const wxSheetCoords& coords) const;
        
    // does the cell have a non-zero width and height, may not be visible, -1 notation
    bool IsCellShown( const wxSheetCoords& coords ) const;
       
    // grid may occupy more space than needed for its rows/columns, this
    // function allows to set how big this margin space is
    void SetMargins(int width, int height)
        { GetSheetRefData()->m_marginSize.x = wxMax(0, width); 
          GetSheetRefData()->m_marginSize.y = wxMax(0, height); }
    
    // Get the renderer's best size for the cell, uses -1 notation
    wxSize GetCellBestSize(const wxSheetCoords& coords, wxDC *dc = NULL) const;
    // Get the best height of a row or the best width of a col using the 
    //   renderer's best size
    int GetRowBestHeight(int row) const;
    int GetColBestWidth(int col) const;

    // ------------------------------------------------------------------------
    // Row/Col label size

    // Get the fixed initial size of the width of row labels or height of col labels
    int GetDefaultRowLabelWidth()  const { return WXSHEET_DEFAULT_ROW_LABEL_WIDTH; }
    int GetDefaultColLabelHeight() const { return WXSHEET_DEFAULT_COL_LABEL_HEIGHT; }

    // Get/Set the row/col label widths, 
    //   if zero_not_shown and row/col & corner not shown return 0
    int  GetRowLabelWidth(bool zero_not_shown=true)  const;
    int  GetColLabelHeight(bool zero_not_shown=true) const;
    void SetRowLabelWidth( int width );
    void SetColLabelHeight( int height );

    // ------------------------------------------------------------------------
    // Auto sizing of the row/col widths/heights
    
    // automatically size the col/row to fit to its contents, if setAsMin, 
    // this optimal width will also be set as minimal width for this column
    // returns the width or height used.
    int AutoSizeRow( int row, bool setAsMin = true );
    int AutoSizeCol( int col, bool setAsMin = true );

    // auto size all columns (very ineffective for big grids!)
    void AutoSizeRows( bool setAsMin = true );
    void AutoSizeCols( bool setAsMin = true );

    // auto size the grid, that is make the columns/rows of the "right" size
    // and also set the grid size to just fit its contents
    void AutoSize( bool setAsMin = true );

    // autosize row height depending on label text
    void AutoSizeRowLabelHeight( int row );
    // autosize column width depending on label text
    void AutoSizeColLabelWidth( int col );

    // Force the col widths to be of equal size so that they fit within the 
    // the window size. This is maintained even when the window is resized. 
    // The col widths will not be sized less than min_width in pixels. 
    // Use this if you know that the window will be of a reasonable size to 
    // fit the cols, but you don't want to track the EVT_SIZE yourself.
    // use a min_width <= 0 to turn it off
    void SetEqualColWidths(int min_width);
    
    // ------------------------------------------------------------------------
    // Row/Col drag resizing enabled or disabled
    //
    // if CanDragRow/ColSize the rows/cols can be resized by the mouse
    // if CanDragGridSize you can resize the rows/cols in the grid window
    //   else you resize in the label windows (if CanDragRow/ColSize is true)
    
    void EnableDragRowSize( bool enable = true ) { SetDragCellSize(wxSHEET_DragLabelRowHeight, enable); }
    void EnableDragColSize( bool enable = true ) { SetDragCellSize(wxSHEET_DragLabelColWidth, enable); }
    void EnableDragGridSize(bool enable = true ) { SetDragCellSize(wxSHEET_DragGridCellSize, enable); }
    void DisableDragRowSize()                    { EnableDragRowSize( false ); }
    void DisableDragColSize()                    { EnableDragColSize( false ); }
    void DisableDragGridSize()                   { EnableDragGridSize(false); }
    bool CanDragRowSize()  const                 { return (GetDragCellSize() & wxSHEET_DragLabelRowHeight) != 0; }
    bool CanDragColSize()  const                 { return (GetDragCellSize() & wxSHEET_DragLabelColWidth) != 0; }
    bool CanDragGridSize() const                 { return (GetDragCellSize() & wxSHEET_DragGridCellSize) != 0; }
    
    // Directly set the dragging of the cell size use wxSheetDragCellSize_Type enums
    void SetDragCellSize( int type ) { GetSheetRefData()->m_canDrag = type; }
    void SetDragCellSize( int type, bool enable )
        { SetDragCellSize( enable ? (GetDragCellSize() | type) : (GetDragCellSize() & (~type)) ); }
    int GetDragCellSize() const { return GetSheetRefData()->m_canDrag; }
    
    // ------------------------------------------------------------------------
    // Grid line, cell highlight, selection colouring

    // Draw the grid lines, wxHORIZONAL | wxVERTICAL (wxBOTH), 0 for none
    void EnableGridLines( int dir = wxBOTH );
    int  GridLinesEnabled() const { return GetSheetRefData()->m_gridLinesEnabled; }
    
    const wxColour& GetGridLineColour() const { return GetSheetRefData()->m_gridLineColour; }
    void            SetGridLineColour( const wxColour& colour );
    
    const wxColour& GetCursorCellHighlightColour()     const { return GetSheetRefData()->m_cursorCellHighlightColour; }
    int             GetCursorCellHighlightPenWidth()   const { return GetSheetRefData()->m_cursorCellHighlightPenWidth; }
    int             GetCursorCellHighlightROPenWidth() const { return GetSheetRefData()->m_cursorCellHighlightROPenWidth; }
    void            SetCursorCellHighlightColour( const wxColour& colour );
    void            SetCursorCellHighlightPenWidth(int width);
    void            SetCursorCellHighlightROPenWidth(int width);

    // get/set the colour bounding the labels to give 3-D effect
    const wxColour& GetLabelOutlineColour() const { return GetSheetRefData()->m_labelOutlineColour; }
    void            SetLabelOutlineColour( const wxColour& colour );
    
    const wxColour& GetSelectionBackground() const { return GetSheetRefData()->m_selectionBackground; }
    const wxColour& GetSelectionForeground() const { return GetSheetRefData()->m_selectionForeground; }
    void            SetSelectionBackground(const wxColour& c) { GetSheetRefData()->m_selectionBackground = c; }
    void            SetSelectionForeground(const wxColour& c) { GetSheetRefData()->m_selectionForeground = c; }
    
    // ------------------------------------------------------------------------
    // Span, cells can span across multiple cells, hiding cells below
    //
    // Normally cells are of size 1x1, but they can be larger. 
    // The other cells can still have values and attributes, but they
    // will not be used since GetCellOwner is used for most coord operations
    // so that the underlying cell values and attributes are ignored.
    // The span for the owner cell is 1x1 or larger, the span for other
    // cells contained within the spanned block have a cell span of <= 0, such 
    // that coords + GetCellSpan() = the owner cell
    //
    // You can completely override this functionality if you provide 
    //   HasSpannedCells, GetCellBlock, SetCellSpan

    // Are there any spanned cells at all?
    virtual bool HasSpannedCells() const;
    
    // if cell is part of a spanning cell, return owner's coords else input coords
    wxSheetCoords GetCellOwner( const wxSheetCoords& coords ) const;    
    // Get a block of the cell, unless a spanned cell it's of size 1,1
    //   note: the top left of block is the owner cell of coords
    virtual wxSheetBlock GetCellBlock( const wxSheetCoords& coords ) const;
    // Get the span of a cell, the owner (top right) cell always has a span of 
    //  (1, 1) or greater. The other cells in a spanned block will have a span 
    //  (<1, <1) such that, coords + coordsSpan = ownerCoords
    wxSheetCoords GetCellSpan( const wxSheetCoords& coords ) const;
    // Set the span of a cell, must be 1x1 or greater, 
    // To remove a spanned cell set it to a cell of size 1x1
    // For grid cells the whole block must be contained within the grid cells 
    //  and if the block intersects a previously spanned cell block the top left 
    //  corners must match up. 
    // Row and Col labels can span cells as well, spanned row labels must have a
    //  width of 1 and a height of >= 1, col labels a height of 1 and width >= 1
    virtual void SetCellSpan( const wxSheetBlock& block );
    void SetCellSpan( const wxSheetCoords& coords, const wxSheetCoords& numRowsCols )
        { SetCellSpan(wxSheetBlock(coords, numRowsCols.m_row, numRowsCols.m_col)); }

    // Get a pointer to the spanned blocks to iterate through, may return NULL.
    const wxSheetSelection* GetSpannedBlocks() const { return GetTable() ? GetTable()->GetSpannedBlocks() : NULL; }
    // ------------------------------------------------------------------------
    // Get/Set attributes for the cells, row, col, corner labels

    // See wxSheetAttr_Type for a description of the type of attribute

    // The coords are specified as
    //   The Grid area (0 <= row < GetNumberRows), (0 <= col < GetNumberCols)
    //   The Corner window row = col = -1
    //   The Row labels (0 <= row < GetNumberRows), col = -1
    //   The Col labels row = -1, (0 <= col < GetNumberCols)
    
    // For the wxSHEET_AttrDefault type the coords should be contained within the 
    //   size of the sheet, but the particular values are not used.
    //   see const wxGridCellSheetCoords = (0,0) for example
    //   wxRowLabelSheetCoords, wxColLabelSheetCoords, wxCornerLabelSheetCoords
    
    // To completely override this you may provide alternate 
    // GetOrCreateAttr, GetAttr, and SetAttr functions.  

    // make sure that the last default attr of initAttr is defAttr
    //   This is called internally when you call SetAttr
    bool InitAttr( wxSheetCellAttr& initAttr, const wxSheetCellAttr& defAttr ) const;

    // Get an attribute for the coords if it exists or create a new one 
    // and put it into the table which puts it in the attr provider.
    // The type may be only be wxSHEET_AttrDefault/Cell/Row/Col for the grid cells
    // and wxSHEET_AttrDefault/Cell for the labels
    virtual wxSheetCellAttr GetOrCreateAttr( const wxSheetCoords& coords, 
                                             wxSheetAttr_Type type ) const;
 
    // Get the attribute for any area depending on the coords and type
    // returns a valid attr if the coords are valid and type = wxSHEET_AttrAny
    // The type may be only be wxSHEET_AttrDefault/Cell/Row/Col/Any for the grid cells
    // and wxSHEET_AttrDefault/Cell/Any for the labels
    virtual wxSheetCellAttr GetAttr( const wxSheetCoords& coords, 
                                     wxSheetAttr_Type type = wxSHEET_AttrAny) const;
    // Set the attribute for any area depending on the coords
    // The type may be only be wxSHEET_AttrDefault/Cell/Row/Col for the grid cells
    // and wxSHEET_AttrDefault/Cell for the labels
    virtual void SetAttr(const wxSheetCoords& coords, const wxSheetCellAttr& attr,
                         wxSheetAttr_Type type );

    // ------ Simplified functions for accessing the attributes ---------------
    // Get an attribute for the grid coords, returning a cell/row/col attr or
    //   if multiple attr for the coords an attr that's merged, or the def attr
    wxSheetCellAttr GetGridAttr(const wxSheetCoords& coords) const { return GetAttr(coords, wxSHEET_AttrAny); }

    // Get a specific Cell/Row/Col attr for the coords in the grid
    //   if none set returns wxNullSheetCellAttr
    wxSheetCellAttr GetGridCellAttr(const wxSheetCoords& coords) const { return GetAttr(coords, wxSHEET_AttrCell); }
    wxSheetCellAttr GetGridRowAttr(int row) const { return GetAttr(wxSheetCoords(row,  0), wxSHEET_AttrRow); }
    wxSheetCellAttr GetGridColAttr(int col) const { return GetAttr(wxSheetCoords(0,  col), wxSHEET_AttrCol); }
    // Set a specific Cell/Row/Col attr for coords, row/col only apply to the grid
    void SetGridCellAttr(const wxSheetCoords& coords, const wxSheetCellAttr& attr) { SetAttr(coords, attr, wxSHEET_AttrCell); }
    void SetGridRowAttr(int row, const wxSheetCellAttr& attr) { SetAttr(wxSheetCoords(row,  0), attr, wxSHEET_AttrRow); }
    void SetGridColAttr(int col, const wxSheetCellAttr& attr) { SetAttr(wxSheetCoords(0,  col), attr, wxSHEET_AttrCol); }

    // Get the row/col/corner label attributes, if one is not set for the 
    //  particular coords, returns the default one. (note: only one corner attr)
    //  if you want the particular attr use GetRow/ColLabelCellAttr
    wxSheetCellAttr GetRowLabelAttr(int row) const { return GetAttr(wxSheetCoords(row, -1), wxSHEET_AttrAny); }
    wxSheetCellAttr GetColLabelAttr(int col) const { return GetAttr(wxSheetCoords(-1, col), wxSHEET_AttrAny); }
    wxSheetCellAttr GetCornerLabelAttr() const     { return GetAttr(wxSheetCoords(-1,  -1), wxSHEET_AttrAny); }

    // Get a specific attr the row/col/corner label cell
    //   if none set returns wxNullSheetCellAttr
    wxSheetCellAttr GetRowLabelCellAttr(int row) const { return GetAttr(wxSheetCoords(row, -1), wxSHEET_AttrCell); }
    wxSheetCellAttr GetColLabelCellAttr(int col) const { return GetAttr(wxSheetCoords(-1, col), wxSHEET_AttrCell); }
    // Set a specific attribute for particular row/col/corner label cell
    void SetRowLabelCellAttr(int row, const wxSheetCellAttr& attr) { SetAttr(wxSheetCoords(row, -1), attr, wxSHEET_AttrCell); }
    void SetColLabelCellAttr(int col, const wxSheetCellAttr& attr) { SetAttr(wxSheetCoords(-1, col), attr, wxSHEET_AttrCell); }
    void SetCornerLabelAttr(const wxSheetCellAttr& attr)           { SetAttr(wxSheetCoords(-1,  -1), attr, wxSHEET_AttrCell); }
   
    // Get/Set default attributes for the areas (only one corner attr)
    //  For setting, wxSheetCellAttr::UpdateWith is called with the current default 
    //  attr so you so need only set the values that you want to change.
    wxSheetCellAttr GetDefaultAttr(const wxSheetCoords& coords) const { return GetAttr(coords, wxSHEET_AttrDefault); }
    wxSheetCellAttr GetDefaultGridCellAttr() const { return GetAttr(wxGridCellSheetCoords, wxSHEET_AttrDefault); }
    wxSheetCellAttr GetDefaultRowLabelAttr() const { return GetAttr(wxRowLabelSheetCoords, wxSHEET_AttrDefault); }
    wxSheetCellAttr GetDefaultColLabelAttr() const { return GetAttr(wxColLabelSheetCoords, wxSHEET_AttrDefault); }
    void SetDefaultAttr(const wxSheetCoords& coords, const wxSheetCellAttr& attr) { SetAttr(coords, attr, wxSHEET_AttrDefault); }
    void SetDefaultGridCellAttr(const wxSheetCellAttr& attr) { SetAttr(wxGridCellSheetCoords, attr, wxSHEET_AttrDefault); }
    void SetDefaultRowLabelAttr(const wxSheetCellAttr& attr) { SetAttr(wxRowLabelSheetCoords, attr, wxSHEET_AttrDefault); }
    void SetDefaultColLabelAttr(const wxSheetCellAttr& attr) { SetAttr(wxColLabelSheetCoords, attr, wxSHEET_AttrDefault); }
    
    // These are convienience functions, if for example you want to subclass the
    //  table and modify and return default attr "on the fly" for each cell
    const wxSheetCellAttr& DoGetDefaultGridAttr() const        { return GetSheetRefData()->m_defaultGridCellAttr; }
    const wxSheetCellAttr& DoGetDefaultRowLabelAttr() const    { return GetSheetRefData()->m_defaultRowLabelAttr; }
    const wxSheetCellAttr& DoGetDefaultColLabelAttr() const    { return GetSheetRefData()->m_defaultColLabelAttr; }
    const wxSheetCellAttr& DoGetDefaultCornerLabelAttr() const { return GetSheetRefData()->m_defaultCornerLabelAttr; }
    
    // Get/Set particular attributes for any type of cell/row/col anywhere
    //  The default is to get the attr val for type=wxSHEET_AttrAny meaning that
    //  it'll find a set attr first or return the default attr value as a last resort. 
    //  For GetXXX you will receive an an error message if you specify a 
    //  particular wxSHEET_AttrCell/Row/Col, but there isn't an attribute set
    const wxColour& GetAttrBackgroundColour( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    const wxColour& GetAttrForegoundColour( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    const wxFont&   GetAttrFont( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    int  GetAttrAlignment( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    int  GetAttrOrientation( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    int  GetAttrLevel( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    bool GetAttrOverflow( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    bool GetAttrOverflowMarker( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    bool GetAttrShowEditor( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    bool GetAttrReadOnly( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    wxSheetCellRenderer GetAttrRenderer( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    wxSheetCellEditor   GetAttrEditor( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    
    // Set attributes for a particular cell/row/col, relies on GetOrCreateAttr()
    //  so it creates and adds the attr to the attr provider if there wasn't one
    //  after setting the particular value.
    //  The type may be only be wxSHEET_AttrDefault/Cell/Row/Col 
    void SetAttrBackgroundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrForegroundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrFont( const wxSheetCoords& coords, const wxFont& font, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrAlignment( const wxSheetCoords& coords, int align, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOrientation( const wxSheetCoords& coords, int orientation, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrLevel( const wxSheetCoords& coords, int level, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOverflow( const wxSheetCoords& coords, bool allow, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOverflowMarker( const wxSheetCoords& coords, bool draw_marker, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrShowEditor( const wxSheetCoords& coords, bool show_editor, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrReadOnly( const wxSheetCoords& coords, bool isReadOnly, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrRenderer( const wxSheetCoords& coords, const wxSheetCellRenderer &renderer, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrEditor( const wxSheetCoords& coords, const wxSheetCellEditor &editor, wxSheetAttr_Type type = wxSHEET_AttrCell );

    // helper functions that use SetColAttr to set renderer type
    // set the format for the data in the column: default is string
    void SetColFormatBool(int col)   { SetColFormatCustom(col, wxSHEET_VALUE_BOOL); }
    void SetColFormatNumber(int col) { SetColFormatCustom(col, wxSHEET_VALUE_NUMBER); }
    void SetColFormatFloat(int col, int width = -1, int precision = -1);
    void SetColFormatCustom(int col, const wxString& typeName);

    // ------------------------------------------------------------------------
    // Get/Set cell, row, col, and corner label values
    // To completely override this you need only provide Get/SetCellValue
    
    // Get/Set cell value, uses coords = -1 notation for row/col/corner labels
    virtual wxString GetCellValue( const wxSheetCoords& coords );
    virtual void SetCellValue( const wxSheetCoords& coords, const wxString& value );
    
    // Is this cell empty, see wxSheetTable
    virtual bool HasCellValue( const wxSheetCoords& coords );
    
    wxString GetRowLabelValue( int row ) { return GetCellValue(wxSheetCoords(row, -1)); }
    wxString GetColLabelValue( int col ) { return GetCellValue(wxSheetCoords(-1, col)); }   
    void     SetRowLabelValue( int row, const wxString& value ) { SetCellValue(wxSheetCoords(row, -1), value); }
    void     SetColLabelValue( int col, const wxString& value ) { SetCellValue(wxSheetCoords(-1, col), value); }

    wxString GetCornerLabelValue() { return GetCellValue(wxSheetCoords(-1, -1)); }
    void     SetCornerLabelValue(const wxString& value) { SetCellValue(wxSheetCoords(-1, -1), value); }   
    
    // ------------------------------------------------------------------------
    // Register mapping between data types to Renderers/Editors

    void RegisterDataType( const wxString& typeName,
                           const wxSheetCellRenderer& renderer,
                           const wxSheetCellEditor& editor );

    virtual wxSheetCellEditor   GetDefaultEditorForType(const wxString& typeName) const;
    virtual wxSheetCellRenderer GetDefaultRendererForType(const wxString& typeName) const;
    
    // FIXME what is the point of these?
    virtual wxSheetCellEditor   GetDefaultEditorForCell(const wxSheetCoords& coords) const;
    virtual wxSheetCellRenderer GetDefaultRendererForCell(const wxSheetCoords& coords) const;

    // ------------------------------------------------------------------------
    // Cursor movement and visibility functions

    // check to see if a cell is either wholly visible (the default arg) or
    // at least partially visible, uses -1 notation for labels
    bool IsCellVisible( const wxSheetCoords& coords, bool wholeCellVisible = true ) const;
    bool IsRowVisible( int row, bool wholeRowVisible = true ) const;
    bool IsColVisible( int col, bool wholeColVisible = true ) const;
    // Make this cell visible, uses -1 notation, will not unhide label windows
    void MakeCellVisible( const wxSheetCoords& coords );

    // Get/Set cursor cell, this is the "current" cell where a highlight is drawn
    // the cursor only applies to the grid cells
    const wxSheetCoords& GetGridCursorCell() const { return GetSheetRefData()->m_cursorCoords; }
    int  GetGridCursorRow() const { return GetGridCursorCell().GetRow(); }
    int  GetGridCursorCol() const { return GetGridCursorCell().GetCol(); }
    void SetGridCursorCell( const wxSheetCoords& coords );
    
    bool MoveCursorUp( bool expandSelection )    { return DoMoveCursor(wxSheetCoords(-1, 0), expandSelection); }
    bool MoveCursorDown( bool expandSelection )  { return DoMoveCursor(wxSheetCoords( 1, 0), expandSelection); }
    bool MoveCursorLeft( bool expandSelection )  { return DoMoveCursor(wxSheetCoords( 0,-1), expandSelection); }
    bool MoveCursorRight( bool expandSelection ) { return DoMoveCursor(wxSheetCoords( 0, 1), expandSelection); }
    bool MoveCursorUpBlock( bool expandSelection )    { return DoMoveCursorBlock(wxSheetCoords(-1, 0), expandSelection); }
    bool MoveCursorDownBlock( bool expandSelection )  { return DoMoveCursorBlock(wxSheetCoords( 1, 0), expandSelection); }
    bool MoveCursorLeftBlock( bool expandSelection )  { return DoMoveCursorBlock(wxSheetCoords( 0,-1), expandSelection); }
    bool MoveCursorRightBlock( bool expandSelection ) { return DoMoveCursorBlock(wxSheetCoords( 0, 1), expandSelection); }
    bool MoveCursorUpPage( bool expandSelection )   { return DoMoveCursorUpDownPage(true, expandSelection); }
    bool MoveCursorDownPage( bool expandSelection ) { return DoMoveCursorUpDownPage(false, expandSelection); }

    virtual bool DoMoveCursor( const wxSheetCoords& relCoords, bool expandSelection ); 
    virtual bool DoMoveCursorBlock( const wxSheetCoords& relDir, bool expandSelection );
    virtual bool DoMoveCursorUpDownPage( bool page_up, bool expandSelection );
    
    // ------------------------------------------------------------------------
    // Cell/Row/Col selection and deselection, you can only select grid cells

    // Note: A selection to the # of rows/cols means that the whole row/col is
    //   selected. Otherwise the right/bottom is rows/cols - 1, ie. contained
    //   within the actual number of cells.
    //   If sendEvt a wxEVT_SHEET_RANGE_SELECTED is sent, the SELECTING event
    //     should have been sent by the caller and if vetoed not call these.
    //   All functions (De)Select/Row/Col/Cell, SelectAll go to (De)SelectBlock.
    //   ClearSelection deselects everything and sends a single event with
    //   wxSheetBlock(0,0,rows,cols) to imply everything is cleared.
    
    // To override the selection mechanism you only need to override,
    // HasSelection, IsCell/Row/ColSelected, (De)SelectBlock, and ClearSelection.
    
    // Is there any selection, if selecting, includes the active selection block
    //   which is not yet part of underlying selection system
    virtual bool HasSelection(bool selecting = true) const;
    // Are these coords within either the selecting block or selection
    virtual bool IsCellSelected( const wxSheetCoords& coords ) const;
    bool IsCellSelected( int row, int col ) const { return IsCellSelected(wxSheetCoords(row, col)); }
    virtual bool IsRowSelected( int row ) const;
    virtual bool IsColSelected( int col ) const;
    // Are we currently in the middle of a selection
    bool IsSelecting() const { return !GetSelectingBlock().IsEmpty(); }

    void SetSelectionMode(wxSheetSelectionMode_Type selmode);
    int  GetSelectionMode() const { return GetSheetRefData()->m_selectionMode; }
    bool HasSelectionMode(int mode) { return (GetSelectionMode() & mode) != 0; }

    virtual bool SelectRow( int row, bool addToSelected = false, bool sendEvt = false );
    virtual bool SelectRows( int rowTop, int rowBottom, bool addToSelected = false, bool sendEvt = false );
    virtual bool SelectCol( int col, bool addToSelected = false, bool sendEvt = false );
    virtual bool SelectCols( int colLeft, int colRight, bool addToSelected = false, bool sendEvt = false );
    virtual bool SelectCell( const wxSheetCoords& coords, bool addToSelected = false, bool sendEvt = false );
    virtual bool SelectBlock( const wxSheetBlock& block, bool addToSelected = false, 
                              bool sendEvt = false );
    // selects everything to numRows, numCols
    virtual bool SelectAll(bool sendEvt = false);

    virtual bool DeselectRow( int row, bool sendEvt = false );
    virtual bool DeselectRows( int rowTop, int rowBottom, bool sendEvt = false );
    virtual bool DeselectCol( int col, bool sendEvt = false );
    virtual bool DeselectCols( int colLeft, int colRight, bool sendEvt = false );
    virtual bool DeselectCell( const wxSheetCoords& coords, bool sendEvt = false );
    virtual bool DeselectBlock( const wxSheetBlock& block, bool sendEvt = false );
    // clears selection, single deselect event numRows, numCols
    virtual bool ClearSelection(bool send_event = false);

    // toggle the selection of a single cell, row, or col 
    // addToSelected applies to a selection only, ignored if a deselection
    virtual bool ToggleCellSelection( const wxSheetCoords& coords, 
                            bool addToSelected = false, bool sendEvt = false );
    virtual bool ToggleRowSelection( int row, 
                            bool addToSelected = false, bool sendEvt = false );
    virtual bool ToggleColSelection( int col, 
                            bool addToSelected = false, bool sendEvt = false );
    
    // Get a pointer to the selection mechanism. You are free to do what you 
    //  want with it, do a ForceRefresh to update the grid when done.
    wxSheetSelection* GetSelection() const { return GetSheetRefData()->m_selection; }
    
    // During a selection this is the selecting block, else empty
    const wxSheetBlock& GetSelectingBlock() const { return GetSheetRefData()->m_selectingBlock; }
    const wxSheetCoords& GetSelectingAnchor() const { return GetSheetRefData()->m_selectingAnchor; }

    // These are internal use functions to highlight a block during mouse
    //  dragging or keyboard selecting
    void SetSelectingBlock(const wxSheetBlock& selectingBlock) { GetSheetRefData()->m_selectingBlock = selectingBlock; }
    void SetSelectingAnchor(const wxSheetCoords& selectingAnchor) { GetSheetRefData()->m_selectingAnchor = selectingAnchor; }
    
    // while selecting set and draw m_selectingBlock highlight and clear up last
    virtual void HighlightSelectingBlock( const wxSheetBlock& selectingBlock );
    void HighlightSelectingBlock( const wxSheetCoords& cornerCell,
                                  const wxSheetCoords& otherCell )
        { HighlightSelectingBlock(wxSheetBlock(cornerCell, otherCell)); }
    
    // ------------------------------------------------------------------------
    // Copy/Paste functionality for strings
    
    // Copy the current selection using CopyCurrentSelectionInternal then
    //  to the wxClipboard using CopyInternalSelectionToClipboard
    bool CopyCurrentSelectionToClipboard(bool copy_cursor = true,
                                         const wxChar& colSep = wxT('\t'));
    // Copy the current selection to an internal copied selection mechanism 
    //  storing both the positions and values of the selected cells, if no 
    //  selection and copy_cursor then just copy the cursor value
    bool CopyCurrentSelectionInternal(bool copy_cursor = true);
    // Copy the internal selection to the wxClipboard as both a string using 
    //  colSep to separate columns and as an internal representation for 
    //  pasting back into the wxSheet.
    bool CopyInternalSelectionToClipboard(const wxChar& colSep = wxT('\t'));
    // Copy the internal selection to a clipboard string
    //   uses colSep for cols and \n for rows, called by CopySelectionToClipboard
    wxString CopyInternalSelectionToString(const wxChar& colSep = wxT('\t'));

    // Copies the string (perhaps from the clipboard) to the internal copied 
    //   selection uses colSep for cols and \n for rows, used by PasteFromClipboard
    bool CopyStringToSelection(const wxString& value, const wxChar& colSep = wxT('\t'));
    
    // Tries to get the clipboard data as wxSheet's clipboard data 
    // representation else use CopyStringToSelection to convert a string 
    //  using colSep as the column separator and \n as row separator.
    bool PasteFromClipboard(const wxSheetCoords &topLeft = wxNullSheetCoords,
                            const wxChar& colSep = wxT('\t'));
    // Paste the internal copied selection at the topLeft coords or if 
    //  topLeft = wxNullSheetCoords then if IsSelection use the upper right of 
    //  the current selection and only paste into currently selected cells. 
    //  If no selection the the cursor is the topLeft cell. 
    virtual bool PasteInternalCopiedSelection(const wxSheetCoords &topLeft = wxNullSheetCoords);
    // Are the cells being pasted right now, use this in the table's 
    //    SetCellValue and AppendXXX to differentiate between a user typing
    bool CurrentlyPasting() const { return GetSheetRefData()->m_pasting; }
    
    // ------------------------------------------------------------------------
    // Edit control functions (mostly used internally)
    
    // Is/Make the whole sheet editable or readonly 
    bool IsEditable() const { return GetSheetRefData()->m_editable; }
    void EnableEditing( bool edit );

    // enable and show the editor control at the coords, returns sucess, ie. !vetoed
    bool EnableCellEditControl( const wxSheetCoords& coords );
    // hide and disable the editor and save the value if save_value, returns sucess, ie. !vetoed
    bool DisableCellEditControl( bool save_value );
    // is this cell valid and editable
    bool CanEnableCellControl(const wxSheetCoords& coords) const;
    // is the cell editor created (may not be shown though)
    bool IsCellEditControlCreated() const;
    // is the cell editor valid and shown
    bool IsCellEditControlShown() const;

    // Create and show the appropriate editor at the EnableCellEditControl coords
    //  this is called internally by EnableCellEditControl, but if you call 
    //  HideCellEditControl and if IsCellEditControlCreated then you can reshow 
    //  it with this, returns sucess
    bool ShowCellEditControl();
    // Hide the editor, doesn't destroy it (use DisableCellEditControl)
    //  check if IsCellEditControlShown first, returns sucess
    bool HideCellEditControl();
    // Save the value of the editor, check IsCellEditControlEnabled() first
    void SaveEditControlValue();

    // Get the current editor, !Ok() if !IsCellEditControlCreated()
    const wxSheetCellEditor& GetEditControl() const { return GetSheetRefData()->m_cellEditor; }
    // These are the coords of the editor, check IsCellEditControlCreated before using
    const wxSheetCoords& GetEditControlCoords() const { return GetSheetRefData()->m_cellEditorCoords; }
    
    // ------------------------------------------------------------------------
    // Drawing functions
    
    // Code that does a lot of grid modification can be enclosed
    // between BeginBatch() and EndBatch() calls to avoid screen flicker
    // EndBatch's refresh = false will not refresh when batchCount is 0
    void BeginBatch() { m_batchCount++; }
    void EndBatch(bool refresh=true);
    int  GetBatchCount() const { return m_batchCount; }
    
    // Use ForceRefresh, rather than wxWindow::Refresh(), to force an
    // immediate repainting of the grid. No effect if GetBatchCount() > 0
    // This function is necessary because wxSheet has a minimal OnPaint()
    // handler to reduce screen flicker.
    void ForceRefresh() { BeginBatch(); EndBatch(); }
    
    // *** Use these redrawing functions to ensure refed sheets are redrawn ***
    
    // Refresh a single cell, can also draw cells for labels using -1 notation
    // does nothing if cell !visible, or GetBatchCount != 0
    // if single_cell then literally draw only the single cell, else draw the
    // cell to left in case the overflow marker needs to be drawn and the 
    // cells to the right in case this cell overflows.
    void RefreshCell(const wxSheetCoords& coords, bool single_cell = true);
    // Refresh a block of cells in any/all of the windows by chopping up the block, 
    //   uses -1 notation to refresh labels
    void RefreshBlock(const wxSheetBlock& block);
    // Refresh a single row, row = -1 refreshes all col labels, 
    // does nothing if row !visible, or GetBatchCount != 0
    void RefreshRow(int row);
    // Refresh a single col, col = -1 refreshes all row labels, 
    // does nothing if col !visible, or GetBatchCount != 0
    void RefreshCol(int col);
    // Refresh is called using a rect surrounding the block
    // does nothing if block IsEmpty, !visible, or GetBatchCount != 0
    void RefreshGridCellBlock( const wxSheetBlock& block );
    // After SetAttr call this can appropriately refresh the wxSheet areas
    void RefreshAttrChange(const wxSheetCoords& coords, wxSheetAttr_Type type);

    // ************************************************************************
    // Drawing implementation 

    // Refresh an area of the window that calculates the smaller rects for
    //  each individual window (row/col/corner...) and calls Refresh(subRect)
    //  The rect is the logical rect, not the scrolled device rect
    virtual void Refresh(bool eraseb = true, const wxRect* rect = NULL);
    
    // These directly call wxWindow::Refresh for the appropriate windows
    //   The input rect doesn't have to be clipped to the visible window since
    //   this function takes care of that, but it should be in client coords. 
    void RefreshGridWindow(bool eraseb = true, const wxRect* rect = NULL);
    void RefreshRowLabelWindow(bool eraseb = true, const wxRect* rect = NULL);
    void RefreshColLabelWindow(bool eraseb = true, const wxRect* rect = NULL);
    void RefreshCornerLabelWindow(bool eraseb = true, const wxRect* rect = NULL);

    //    Don't use these if you plan to use the splitter since they only act 
    //    on this sheet.

    // These functions are called by the OnPaint handler of these windows
    //   use these to add "extra touches" before or after redrawing.
    //   The dc should be prepared before calling these.
    virtual void PaintGridWindow( wxDC& dc, const wxRegion& reg );
    virtual void PaintRowLabelWindow( wxDC& dc, const wxRegion& reg );
    virtual void PaintColLabelWindow( wxDC& dc, const wxRegion& reg );
    virtual void PaintCornerLabelWindow( wxDC& dc, const wxRegion& reg );
    virtual void PaintSheetWindow( wxDC& dc, const wxRegion& reg );
    
    // draws a bunch of blocks of grid cells onto the given DC
    virtual void DrawGridCells( wxDC& dc, const wxSheetSelection& blockSel );
    // Draw the area below and to right of grid up to scrollbars
    virtual void DrawGridSpace( wxDC& dc );
    // draw the border around a single cell
    virtual void DrawCellBorder( wxDC& dc, const wxSheetCoords& coords );
    // Draw all the grid lines in the region
    virtual void DrawAllGridLines( wxDC& dc, const wxRegion& reg );
    // Draw a single cell
    virtual void DrawCell( wxDC& dc, const wxSheetCoords& coords );
    // Calls DrawCursorCellHighlight if contained within this selection
    virtual void DrawCursorHighlight( wxDC& dc, const wxSheetSelection& blockSel );
    // Draw the cursor cell highlight
    virtual void DrawCursorCellHighlight(wxDC& dc, const wxSheetCellAttr &attr);

    // draw wxSheetRowLabelWindow labels
    virtual void DrawRowLabels( wxDC& dc, const wxArrayInt& rows );
    // draw wxSheetColLabelWindow labels
    virtual void DrawColLabels( wxDC& dc, const wxArrayInt& cols );
    // wxSheetCornerLabelWindow label
    virtual void DrawCornerLabel( wxDC& dc );

    // Draw the row/col resizing marker and if newDragPos != -1, set the 
    //  new position of the marker
    virtual void DrawRowColResizingMarker( int newDragPos = -1 );

    // Draw the splitter button in the rectangle
    virtual void DrawSplitterButton(wxDC &dc, const wxRect& rect);

    // Calculate the Row/ColLabels and Cells exposed for the wxRegion
    //   returns false if none, used for redrawing windows
    bool CalcRowLabelsExposed( const wxRegion& reg, wxArrayInt& rows ) const;
    bool CalcColLabelsExposed( const wxRegion& reg, wxArrayInt& cols ) const;
    bool CalcCellsExposed( const wxRegion& reg, wxSheetSelection& blockSel ) const;
    int  FindOverflowCell( const wxSheetCoords& coords, wxDC& dc );

    // helper drawing functions
    void DrawTextRectangle( wxDC& dc, const wxString& value, const wxRect& rect,
                            int alignment = wxALIGN_LEFT|wxALIGN_TOP,
                            int textOrientation = wxHORIZONTAL );

    void DrawTextRectangle( wxDC& dc, const wxArrayString& lines, const wxRect&,
                            int alignment = wxALIGN_LEFT|wxALIGN_TOP,
                            int textOrientation = wxHORIZONTAL );

    // Split string by '\n' and add to array, returning the number of lines
    //  returns 0 for empty string.
    int StringToLines( const wxString& value, wxArrayString& lines ) const;
    // Get the size of the lines drawn horizontally, returs true if size > 0
    bool GetTextBoxSize( wxDC& dc, const wxArrayString& lines,
                         long *width, long *height ) const;

    // ------------------------------------------------------------------------
    // Geometry utility functions, pixel <-> cell etc
    
    // Note that all of these functions work with the logical coordinates of
    // grid cells and labels so you will need to convert from device
    // coordinates for mouse events etc. 
    // clipToMinMax means that the return value will be within the grid cells 
    // if !clipToMinMax and out of bounds it returns -1.
    // Use ContainsGridXXX to verify validity, -1 doesn't mean label
    wxSheetCoords XYToGridCell( int x, int y, bool clipToMinMax = false ) const;
    int YToGridRow( int y, bool clipToMinMax = false ) const;
    int XToGridCol( int x, bool clipToMinMax = false ) const;

    // return the row/col number that the x/y coord is near the edge of, or
    // -1 if not near an edge, uses WXSHEET_LABEL_EDGE_ZONE
    // Use ContainsGridXXX to verify validity, -1 doesn't mean label
    int YToEdgeOfGridRow( int y ) const;
    int XToEdgeOfGridCol( int x ) const;

    // Get a rect bounding the cell, handles spanning cells and the label 
    //  windows using the -1 notation, getDeviceRect calls CalcScrolledRect
    wxRect CellToRect( const wxSheetCoords& coords, bool getDeviceRect = false ) const;
    // Get a rect bounding the block, handles label windows using the -1 notation, 
    //  getDeviceRect calls CalcScrolledRect
    wxRect BlockToRect( const wxSheetBlock& block, bool getDeviceRect = false ) const;

    // Expand the block by unioning with intersecting spanned cells
    wxSheetBlock ExpandSpannedBlock( const wxSheetBlock& block ) const;
    
    // Convert the block of cells into a wxRect in device coords, expands the
    //  block to contain spanned cells if expand_spanned. 
    //  These functions do handle label cells, but if you span the block from a label
    //  into the grid then the rect will overlap the windows, probably not what you want.
    wxRect BlockToDeviceRect( const wxSheetBlock& block, bool expand_spanned = true ) const; 
    wxRect BlockToLogicalRect( const wxSheetBlock& block, bool expand_spanned = true ) const;

    // Convert the rect in pixels into a block of cells for the grid
    //   if wholeCell then only include cells in the block that are 
    //   wholly contained by the rect
    wxSheetBlock LogicalGridRectToBlock(const wxRect &rect, bool wholeCell = false) const;

    // get a block containing all the currently (partially/fully) visible cells
    wxSheetBlock GetVisibleGridCellsBlock(bool wholeCellVisible = false) const;

    // Align the size of an object inside the rect using wxALIGN enums
    //   if inside then align it to the left if it would have overflown
    //   always pins size to left hand side
    wxPoint AlignInRect( int align, const wxRect& rect, const wxSize& size, bool inside=true ) const;
        
    // ------------------------------------------------------------------------
    // Scrolling for the window, everything is done with pixels
    //   there is no need for scroll units and they only cause sizing problems

    // Get the scrolled origin of the grid in pixels
    const wxPoint& GetGridOrigin() const { return m_gridOrigin; }
    // Set the absolute scrolled origin of the grid window in pixels 
    //  this checks validity and ensures proper positioning. 
    //  Use x or y = -1 to not change the origin in the x or y direction
    //  Unless setting from a scrollbar event use adjustScrollBars=true
    virtual void SetGridOrigin(int x, int y, bool adjustScrollBars = true, bool sendEvt=false);
    void SetGridOrigin(const wxPoint& pt, bool adjustScrollBars = true, bool sendEvt=false)
        { SetGridOrigin(pt.x, pt.y, adjustScrollBars, sendEvt); }
        
    // Get the virtual size of the grid in pixels, includes extra width/height
    wxSize GetGridVirtualSize(bool add_margin=true) const;
    
    // Get the extent of the grid, this may be more than the virtual size if the
    //  grid is smaller than the containing window
    wxSize GetGridExtent() const;

    // Same as wxScrolledWindow Calc(Un)ScrolledPosition
    void CalcScrolledPosition(int x, int y, int *xx, int *yy) const
        { if (xx) *xx = x - m_gridOrigin.x; if (yy) *yy = y - m_gridOrigin.y; }
    void CalcUnscrolledPosition(int x, int y, int *xx, int *yy) const
        { if (xx) *xx = x + m_gridOrigin.x; if (yy) *yy = y + m_gridOrigin.y; }
    wxPoint CalcScrolledPosition(const wxPoint& pt) const   { return pt - m_gridOrigin; }
    wxPoint CalcUnscrolledPosition(const wxPoint& pt) const { return pt + m_gridOrigin; }

    // returns the scrolled position of the rect, logical -> device coords
    wxRect CalcScrolledRect(const wxRect &r) const
        { return wxRect(r.x-m_gridOrigin.x, r.y-m_gridOrigin.y, r.width, r.height); }
    // returns the unscrolled position of the rect, device -> logical coords
    wxRect CalcUnscrolledRect(const wxRect &r) const
        { return wxRect(r.x+m_gridOrigin.x, r.y+m_gridOrigin.y, r.width, r.height); }

    // Adjust the scrollbars to match the size/origin of the grid window
    //   call this after SetScrollBarMode
    virtual void AdjustScrollbars(bool calc_win_sizes = true);

    enum SB_Mode
    {
        SB_AS_NEEDED    = 0x0,  // Show the scrollbars as needed
        SB_HORIZ_NEVER  = 0x1,  // Never show horiz scrollbar, even if needed  
        SB_VERT_NEVER   = 0x2,  // Never show vert scrollbar, even if needed  
        SB_NEVER        = SB_HORIZ_NEVER | SB_VERT_NEVER,  
        SB_HORIZ_ALWAYS = 0x4,  // Always show horiz scrollbar
        SB_VERT_ALWAYS  = 0x8,  // Always show vert scrollbar
        SB_ALWAYS       = SB_HORIZ_ALWAYS | SB_VERT_ALWAYS, 
        
        SB_HORIZ_MASK = SB_HORIZ_NEVER|SB_HORIZ_ALWAYS,
        SB_VERT_MASK  = SB_VERT_NEVER|SB_VERT_ALWAYS
    };
    
    int GetScrollBarMode() const { return m_scrollBarMode; }
    void SetScrollBarMode(int mode) { m_scrollBarMode = mode; }
    void SetHorizontalScrollBarMode(int mode) 
        { m_scrollBarMode &= (~SB_HORIZ_MASK); m_scrollBarMode |= mode; }
    void SetVerticalScrollBarMode(int mode) 
        { m_scrollBarMode &= (~SB_VERT_MASK);  m_scrollBarMode |= mode; }

    bool NeedsVerticalScrollBar()   const { return GetGridVirtualSize().y > m_gridWin->GetSize().y; }
    bool NeedsHorizontalScrollBar() const { return GetGridVirtualSize().x > m_gridWin->GetSize().x; }
    
    // SetDeviceOrigin for the wxDC as appropriate for these windows
    
    virtual void PrepareGridDC( wxDC& dc );
    virtual void PrepareRowLabelDC( wxDC& dc );
    virtual void PrepareColLabelDC( wxDC& dc );

    // ------------------------------------------------------------------------
    // Splitting of the grid window - note that the sheet does not split at all
    //   and that only a wxEVT_SHEET_SPLIT_BEGIN event is sent to notify the 
    //   parent that splitting should take place, see wxSheetSplitter.
    //   The "splitter" is just two small rectangles at the top of the vertical 
    //   scrollbar and right of the horizontal scrollbar. They're only shown
    //   when the scrollbars are shown and if splitting is enabled. 
    //   Call CalcWindowSizes after setting to update the display.

    bool GetEnableSplitVertically()   const { return m_enable_split_vert; }
    bool GetEnableSplitHorizontally() const { return m_enable_split_horiz; }
    
    void EnableSplitVertically(bool can_split)   { m_enable_split_vert = can_split; }
    void EnableSplitHorizontally(bool can_split) { m_enable_split_horiz = can_split; }
    
    // ------------------------------------------------------------------------
    // implementation

    // helper function to set only the horiz or vert component of orig_align
    //   returns modified alignment, doesn't modify any bits not in wxAlignment
    //   use -1 for hAlign/vAlign to not modify that direction
    static int SetAlignment(int orig_align, int hAlign, int vAlign);

    // Do any of the windows of the wxSheet have the focus?
    bool HasFocus() const;
    
    // Accessors for component windows
    wxSheetChildWindow* GetGridWindow()        const { return m_gridWin; }
    wxSheetChildWindow* GetRowLabelWindow()    const { return m_rowLabelWin; }
    wxSheetChildWindow* GetColLabelWindow()    const { return m_colLabelWin; }
    wxSheetChildWindow* GetCornerLabelWindow() const { return m_cornerLabelWin; }
    // Get the window with these coords, uses -1 notation
    wxWindow* GetWindowForCoords( const wxSheetCoords& coords ) const;

    // ------ event handlers
    void OnMouse( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );
    
    void ProcessSheetMouseEvent( wxMouseEvent& event );
    void ProcessRowLabelMouseEvent( wxMouseEvent& event );
    void ProcessColLabelMouseEvent( wxMouseEvent& event );
    void ProcessCornerLabelMouseEvent( wxMouseEvent& event );
    void ProcessGridCellMouseEvent( wxMouseEvent& event );
    
    void OnScroll( wxScrollEvent& event );

    // End the row/col dragging, returns true if width/height have changed
    bool DoEndDragResizeRowCol();

    // ------ control types
    enum { wxSHEET_TEXTCTRL = 2100,
           wxSHEET_CHECKBOX,
           wxSHEET_CHOICE,
           wxSHEET_COMBOBOX };
    
    enum 
    {
        ID_HORIZ_SCROLLBAR = 1,
        ID_VERT_SCROLLBAR,
        ID_MOUSE_DRAG_TIMER,
        ID_GRID_WINDOW,
        ID_ROW_LABEL_WINDOW,
        ID_COL_LABEL_WINDOW,
        ID_CORNER_LABEL_WINDOW
    };

    virtual void CalcWindowSizes(bool adjustScrollBars = true);
    virtual void Fit() { AutoSize(); } // overridden wxWindow methods

    // Get the ref counted data the sheet uses, *please* try to not access this
    //  directly if a functions exists to do it for you.
    wxSheetRefData* GetSheetRefData() const { return (wxSheetRefData*)GetRefData(); }
    
    // Create and send wxSheetXXXEvent depending on type and fill extra data
    //   from a wxKeyEvent or wxMouseEvent (if NULL all keydown are set false)
    // returns EVT_VETOED/SKIPPED/CLAIMED
    enum 
    {
        EVT_VETOED  = -1,  // veto was called on the event
        EVT_SKIPPED = 0,   // no evt handler found or evt was Skip()ed
        EVT_CLAIMED = 1    // event was handled and not Skip()ed
    };
    int SendEvent( const wxEventType type, const wxSheetCoords& coords, 
                   wxEvent* mouseOrKeyEvt = NULL );

    int SendRangeEvent( const wxEventType type, const wxSheetBlock& block,
                        bool selecting, bool add, wxEvent* mouseOrKeyEvt = NULL );
    // Just send the event returning EVT_VETOED/SKIPPED/CLAIMED
    int DoSendEvent( wxSheetEvent* event );

    enum MouseCursorMode
    {
        WXSHEET_CURSOR_SELECT_CELL = 0x0001,
        WXSHEET_CURSOR_SELECT_ROW  = 0x0002,
        WXSHEET_CURSOR_SELECT_COL  = 0x0004,
        WXSHEET_CURSOR_SELECTING   = WXSHEET_CURSOR_SELECT_CELL|WXSHEET_CURSOR_SELECT_ROW|WXSHEET_CURSOR_SELECT_COL,
        WXSHEET_CURSOR_RESIZE_ROW  = 0x0008,
        WXSHEET_CURSOR_RESIZE_COL  = 0x0010,
        WXSHEET_CURSOR_RESIZING    = WXSHEET_CURSOR_RESIZE_ROW|WXSHEET_CURSOR_RESIZE_COL,
        WXSHEET_CURSOR_SPLIT_VERTICAL   = 0x0020,
        WXSHEET_CURSOR_SPLIT_HORIZONTAL = 0x0040,
        WXSHEET_CURSOR_SPLITTING        = WXSHEET_CURSOR_SPLIT_VERTICAL|WXSHEET_CURSOR_SPLIT_HORIZONTAL
    };
    // Set the m_mouseCursor for the wxCursor and m_mouseCursorMode for behavior
    // you should always use it and not set m_mouseCursor[Mode] directly!
    void SetMouseCursorMode( MouseCursorMode mode, wxWindow *win );
    // Get the mouse cursor mode, &ed with mask, default returns original value
    int GetMouseCursorMode(int mask = ~0) const { return (m_mouseCursorMode & mask); }
    // Is the mouse cursor in the mode?
    bool HasMouseCursorMode(int mode) const { return GetMouseCursorMode(mode) != 0; }
    
    // Set the window that has capture, releases the previous one if necessary
    // always use this, set with NULL to release mouse
    void SetCaptureWindow( wxWindow *win );
    wxWindow *GetCaptureWindow() const { return m_winCapture; }
    
protected:

    bool DoUpdateRows(size_t row, int numRows, int update = wxSHEET_UpdateAll );
    bool DoUpdateCols(size_t col, int numCols, int update = wxSHEET_UpdateAll );

    virtual wxSize DoGetBestSize() const;
    void OnSize( wxSizeEvent& event );

    wxWindow *m_winCapture;   // the window that captured the mouse (don't use!)

    wxSheetChildWindow *m_gridWin;
    wxSheetChildWindow *m_rowLabelWin;
    wxSheetChildWindow *m_colLabelWin;
    wxSheetChildWindow *m_cornerLabelWin;
    wxScrollBar        *m_horizScrollBar;
    wxScrollBar        *m_vertScrollBar;

    wxPoint m_gridOrigin;     // origin of the gridWin in pixels
    int     m_scrollBarMode;

    bool    m_keySelecting;

    // cell attribute cache 
    wxSheetCellAttr m_cacheAttr;
    wxSheetCoords   m_cacheAttrCoords;
    int             m_cacheAttrType;

    // invalidates the attribute cache
    void ClearAttrCache();
    // adds an attribute to cache
    void CacheAttr(const wxSheetCoords& coords, const wxSheetCellAttr &attr, 
                   wxSheetAttr_Type type ) const;
    // looks for an attr in cache, returns true if found
    bool LookupAttr(const wxSheetCoords& coords, wxSheetAttr_Type type, 
                    wxSheetCellAttr &attr) const;

    bool m_inOnKeyDown;
    int  m_batchCount;
    bool m_resizing;

    enum KeyModifiers
    {
        NO_MODIFIERS = 0,
        CTRL_DOWN    = 0x0001,
        SHIFT_DOWN   = 0x0002,
        ALT_DOWN     = 0x0004,
        META_DOWN    = 0x0008  // meta is numlock in GTK so it's ignored
    };
    int GetKeyModifiers(wxEvent *mouseOrKeyEvent) const;
    
    MouseCursorMode m_mouseCursorMode;
    MouseCursorMode m_mouseCursor;

    int     m_dragLastPos;
    int     m_dragRowOrCol;
    bool    m_isDragging;
    wxPoint m_startDragPos;
    wxPoint m_mousePos;

    bool    m_waitForSlowClick;

    // mouse timer for smooth scrolling when selecting or resizing off window
    void StartMouseTimer();
    void StopMouseTimer();
    void OnMouseTimer( wxTimerEvent &event );
    wxTimer *m_mouseTimer;

    bool   m_enable_split_vert;
    bool   m_enable_split_horiz;
    wxRect m_vertSplitRect;      // pos/size of the splitter rect, 0 if not shown
    wxRect m_horizSplitRect;     // pos/size of the splitter rect, 0 if not shown

    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );   // let wxWidgets translate numpad keys

    // ------ functions to get/send data (see also public functions)
    bool GetModelValues();
    bool SetModelValues();
    
private:
    void Init();
    friend class wxSheetTable;
    DECLARE_DYNAMIC_CLASS(wxSheet)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheet)
};

// ----------------------------------------------------------------------------
// wxSheetEvent
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetEvent : public wxNotifyEvent
{
public:
    wxSheetEvent(int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                 const wxSheetCoords &coords = wxNullSheetCoords, 
                 const wxPoint &pos = wxPoint(-1, -1), bool sel = true);

    wxSheetEvent(const wxSheetEvent& event) : wxNotifyEvent(event), 
                     m_coords(event.m_coords), m_pos(event.m_pos),
                     m_selecting(event.m_selecting), 
                     m_control(event.m_control), m_shift(event.m_shift),
                     m_alt(event.m_alt), m_meta(event.m_meta) { }

    int GetRow() const { return m_coords.m_row; }
    int GetCol() const { return m_coords.m_col; }
    const wxSheetCoords& GetCoords() const { return m_coords; }
    const wxPoint& GetPosition() const { return m_pos; }
    bool Selecting() const   { return m_selecting; }
    bool ControlDown() const { return m_control; }
    bool ShiftDown()   const { return m_shift; }
    bool AltDown()     const { return m_alt; }
    bool MetaDown()    const { return m_meta; }

    // implementation
    
    // setup the Ctrl/Shift/Alt/Meta keysDown from a wxKeyEvent or wxMouseEvent
    // Also mouse position, but the GetEventObject must be of type wxSheet
    void SetKeysDownMousePos(wxEvent *mouseOrKeyEvent);

    virtual wxEvent *Clone() const { return new wxSheetEvent(*this); }
    
    wxSheetCoords m_coords;
    wxPoint       m_pos;
    bool m_selecting;
    bool m_control;
    bool m_shift;
    bool m_alt;
    bool m_meta;

    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSheetEvent)
};

// ----------------------------------------------------------------------------
// wxSheetRangeSelectEvent - wxEVT_SHEET_RANGE_SELECTING(ED)
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetRangeSelectEvent : public wxSheetEvent
{
public:
    wxSheetRangeSelectEvent( int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                             const wxSheetBlock& block=wxNullSheetBlock, 
                             bool sel=false, bool add_to_sel=false );

    wxSheetRangeSelectEvent(const wxSheetRangeSelectEvent& event) 
        : wxSheetEvent(event), m_block(event.m_block), m_add(event.m_add) { }

    const wxSheetBlock& GetBlock() const { return m_block; }
    bool GetAddToSelection() const       { return m_add; }
    
    void SetBlock( const wxSheetBlock& block ) { m_block = block; }

    // wxPoint GetPosition() is unused
    // int GetCoords/Row/Col() is unused
    
    // implementation
    virtual wxEvent *Clone() const { return new wxSheetRangeSelectEvent(*this); }
    
    wxSheetBlock m_block;
    bool m_add;

    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSheetRangeSelectEvent)
};

// ----------------------------------------------------------------------------
// wxSheetEditorCreatedEvent - wxEVT_SHEET_EDITOR_CREATED
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetEditorCreatedEvent : public wxCommandEvent 
{
public:
    wxSheetEditorCreatedEvent( int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                               const wxSheetCoords& coords=wxNullSheetCoords, 
                               wxWindow* ctrl=NULL );

    wxSheetEditorCreatedEvent(const wxSheetEditorCreatedEvent& evt) 
        : wxCommandEvent(evt), m_coords(evt.m_coords), m_ctrl(evt.m_ctrl) { }


    const wxSheetCoords& GetCoords() const { return m_coords; }
    wxWindow* GetControl() const           { return m_ctrl; }
    
    void SetCoords(const wxSheetCoords& coords) { m_coords = coords; }
    void SetControl(wxWindow* ctrl)             { m_ctrl = ctrl; }

    // implementation
    virtual wxEvent *Clone() const { return new wxSheetEditorCreatedEvent(*this); }

    wxSheetCoords m_coords;
    wxWindow*     m_ctrl;

    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSheetEditorCreatedEvent)
};

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    // The origin of the grid window has changed
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_VIEW_CHANGED, 1592)

    // The grid cursor is about to be in a new cell, veto or !Skip() to block 
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SELECTING_CELL, 1592)
    // The grid cursor is in a new cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SELECTED_CELL, 1592)

    // left down click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_LEFT_DOWN, 1580)
    // right down click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_RIGHT_DOWN, 1581)
    // left up click in a grid cell, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_LEFT_UP, 1580)
    // right up click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_RIGHT_UP, 1581)
    // left double click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_LEFT_DCLICK, 1582)
    // right double click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_RIGHT_DCLICK, 1583)

    // left down click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_LEFT_DOWN, 1584)
    // right down click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_RIGHT_DOWN, 1585)
    // left up click in a label cell, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_LEFT_UP, 1584)
    // right up click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_RIGHT_UP, 1585)
    // left double click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_LEFT_DCLICK, 1586)
    // right double click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_LABEL_RIGHT_DCLICK, 1587)
    
    // A row has been resized, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_ROW_SIZE, 1588)
    // A col has been resized, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_COL_SIZE, 1589)
    
    // A block of cells is about to be (de)selected (veto to stop)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_RANGE_SELECTING, 1590)
    // A block of cells has been (de)selected
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_RANGE_SELECTED, 1590)
    
    // The value of a cell is about to be changed (veto to stop)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_VALUE_CHANGING, 1591)
    // The value of a cell has been changed (veto to put old val back)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_CELL_VALUE_CHANGED, 1591)
    
    // From EnableCellEditControl, the control is about to enabled (can veto)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_EDITOR_ENABLED, 1593)
    // From DisableCellEditControl, the control is about to disabled (can veto)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_EDITOR_DISABLED, 1594)
    // From EnableCellEditControl, the edit control has been created
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_EDITOR_CREATED, 1595)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxSheetEventFunction)(wxSheetEvent&);
typedef void (wxEvtHandler::*wxSheetRangeSelectEventFunction)(wxSheetRangeSelectEvent&);
typedef void (wxEvtHandler::*wxSheetEditorCreatedEventFunction)(wxSheetEditorCreatedEvent&);

#define EVT_SHEET_VIEW_CHANGED(id, fn)        DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_VIEW_CHANGED,        id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_SELECTING_CELL(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SELECTING_CELL,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_SELECTED_CELL(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SELECTED_CELL,       id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_DOWN(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_DOWN,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_DOWN(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_DOWN,     id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_UP(id, fn)        DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_UP,        id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_UP(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_UP,       id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_DCLICK(id, fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_DCLICK,    id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_DCLICK(id, fn)   DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_DCLICK,   id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_DOWN(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_DOWN,     id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_DOWN(id, fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_DOWN,    id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_UP(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_UP,       id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_UP(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_UP,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_DCLICK(id, fn)   DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_DCLICK,   id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_DCLICK(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_DCLICK,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_ROW_SIZE(id, fn)            DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_ROW_SIZE,            id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_COL_SIZE(id, fn)            DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_COL_SIZE,            id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_RANGE_SELECTING(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_RANGE_SELECTING,     id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetRangeSelectEventFunction, &fn), NULL ),
#define EVT_SHEET_RANGE_SELECTED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_RANGE_SELECTED,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetRangeSelectEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_VALUE_CHANGING(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_VALUE_CHANGING, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_VALUE_CHANGED(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_VALUE_CHANGED,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_ENABLED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_ENABLED,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_DISABLED(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_DISABLED,     id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_CREATED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_CREATED,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEditorCreatedEventFunction, &fn), NULL ),

#if 0  // TODO: implement these ?  others ?

wxEVT_SHEET_CREATE_CELL;
wxEVT_SHEET_CHANGE_LABELS;
wxEVT_SHEET_CHANGE_SEL_LABEL;

#define EVT_SHEET_CREATE_CELL(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CREATE_CELL,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),
#define EVT_SHEET_CHANGE_LABELS(id, fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CHANGE_LABELS,    id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),
#define EVT_SHEET_CHANGE_SEL_LABEL(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CHANGE_SEL_LABEL, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),

#endif // 0

#endif  // __WX_SHEET_H__
