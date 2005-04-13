/////////////////////////////////////////////////////////////////////////////
// Name:        sheet.h
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEET_H__
#define __WX_SHEET_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheet.h"
#endif

#include "wx24defs.h"   // wx2.4 backwards compatibility
#include "wx/defs.h"
#include "wx/object.h"
#include "wx/event.h"

class wxTimer;
class wxTimerEvent;

#include "wx/hashmap.h"
#include "wx/panel.h"
#include "wx/scrolwin.h"
#include "wx/string.h"
#include "wx/scrolbar.h"
#include "wx/dynarray.h"
#include "wx/clntdata.h"
#if wxMINOR_VERSION > 4
    #include "wx/arrstr.h"
#endif

#include "pairarr.h"
#include "sheetsel.h"
#include "wx/grid.h"   // get wxLongToLongHashMap, wxGridStringArray from grid 

// classes implemented by wxSheet
class wxSheet;
class wxSheetCellAttr;
class wxSheetCellRenderer;
class wxSheetCellEditor;
class wxSheetGridWindow;
class wxSheetRowLabelWindow;
class wxSheetColLabelWindow;
class wxSheetCornerLabelWindow;
class wxSheetTableBase;
class wxSheetTypeRegistry;
class wxSheetCoords;
class wxSheetBlock;
class wxSheetSelection;
class wxSheetEvent;
class wxSheetSplitterEvent;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Default parameters for wxSheet
#define WXSHEET_DEFAULT_NUMBER_ROWS            10
#define WXSHEET_DEFAULT_NUMBER_COLS            10
#ifdef __WXMSW__
    #define WXSHEET_DEFAULT_ROW_HEIGHT         25
#else
    #define WXSHEET_DEFAULT_ROW_HEIGHT         30
#endif  // __WXMSW__
#define WXSHEET_DEFAULT_COL_WIDTH              80
#define WXSHEET_DEFAULT_COL_LABEL_HEIGHT       32
#define WXSHEET_DEFAULT_ROW_LABEL_WIDTH        82
#define WXSHEET_LABEL_EDGE_ZONE                 2
#define WXSHEET_MIN_ROW_HEIGHT                 15
#define WXSHEET_MIN_COL_WIDTH                  15

// type names for grid table values
#define wxSHEET_VALUE_STRING     _T("string")
#define wxSHEET_VALUE_BOOL       _T("bool")
#define wxSHEET_VALUE_NUMBER     _T("long")
#define wxSHEET_VALUE_FLOAT      _T("double")
#define wxSHEET_VALUE_CHOICE     _T("choice")
#define wxSHEET_VALUE_CHOICEINT  _T("choiceint")
#define wxSHEET_VALUE_DATETIME   _T("datetime")
#define wxSHEET_VALUE_LABEL      _T("label")

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

// use the one in wx/grid.h for wx >= 2.5
//WX_DECLARE_HASH_MAP_WITH_DECL( long, long, wxIntegerHash, wxIntegerEqual,
//                               wxLongToLongHashMap, class );
#if wxMINOR_VERSION < 5
WX_DECLARE_HASH_MAP( long, long, wxIntegerHash, wxIntegerEqual,
                     wxLongToLongHashMap );
#endif

// A marcro to define operators for wxObject dervived classes for the wxSheet
#define DECLARE_SHEETOBJ_COPY_CLASS(name)                                   \
    public:                                                                 \
    name(const name &obj) { Ref(obj); }                                     \
    bool operator == (const name& obj) const                                \
        { return m_refData == obj.m_refData; }                              \
    bool operator != (const name& obj) const                                \
        { return m_refData != obj.m_refData; }                              \
    name& operator = (const name& obj)                                      \
        {   if ( (* this) != obj )                                          \
                Ref(obj);                                                   \
            return *this;   }                                               \
    name Clone() const      { name obj; obj.Copy(*this); return obj; }      \
    name * NewClone() const { return new name(Clone()); }                   \
    DECLARE_DYNAMIC_CLASS(name)                                             \

// ----------------------------------------------------------------------------
// wxSheet cell drawing scheme, also applies to label cells
// ----------------------------------------------------------------------------
//               
//     ---------------- <- Top grid line is bottom of cell above, none if row 0
//     |              | <- This is GetRowTop(row)
//     |  (row, col)  |    
//     |              |
//     ---------------- <- this pixel is GetRowBottom(row), part of this cell
//     ^^             ^             
//     ||             |- Right side is GetColRight(col), part of this cell
//     ||    
//     ||- This is GetColLeft(col)   
//     |-- Left side is cell to left, or none if col = 0 
    
// ----------------------------------------------------------------------------
// wxSheetCell_Type : enum of the different window cells
// ----------------------------------------------------------------------------

// The sheet consists of 4 individual windows each is accessed with coords
// using the semantics below.
    
//  ##########################################################################
//  # corner label # col label | col label |           |                     #
//  # (-1, -1)     # (-1, 0)   | (-1, 1)   |    ...    | GetNumberCols() - 1 #
//  ##########################################################################
//  # row label    # grid cell | grid cell |           |                     #
//  # (0, -1)      # (0, 0)    | (0, 1)    |    ...    |                     #
//  #--------------#---------------------------------------------------------#
//  # row label    # grid cell | grid cell |           |                     #
//  # (1, -1)      # (1, 0)    | (1, 1)    |    ...    |                     #
//  #--------------#---------------------------------------------------------#
//  #              #           |           |           |                     #
//  #     ...      #   ...     |   ...     |    ...    |                     #
//  #--------------#---------------------------------------------------------#
//  #              #           |           |           | GetNumberRows() - 1 #
//  #                  GetNumberRows() - 1             | GetNumberCols() - 1 #
//  ##########################################################################

enum wxSheetCell_Type
{
    wxSHEET_CELL_UNKNOWN = 0,
    wxSHEET_CELL_GRID,
    wxSHEET_CELL_ROWLABEL,
    wxSHEET_CELL_COLLABEL,
    wxSHEET_CELL_CORNERLABEL
};    
    
// ----------------------------------------------------------------------------
// wxSheetSelectionMode_Type how the selections are made
// ----------------------------------------------------------------------------

enum wxSheetSelectionMode_Type 
{
    wxSHEET_SelectCells,   // single cells, blocks, rows, and cols
    wxSHEET_SelectRows,    // only rows are selected
    wxSHEET_SelectCols     // only cols are selected
};

// ----------------------------------------------------------------------------
// wxSheetCellAttr : contains all the attributes for a wxSheet cell
//
// Note: When created all the HasXXX return FALSE, use Copy, Merge, or SetXXX
//
// The default attr for the different wxSheet areas must be complete so that 
// when a new attr is assigned you need only set the values you want to be 
// different than the default's. Unset values are retrieved from the default
// attr which gets them from it's def attr and so on, they're chained together.
// ----------------------------------------------------------------------------
enum wxSheetAttr_Type    
{
    // A default attr, used when no particular one is set, all areas have def attr
    wxSHEET_AttrDefault = 0x00010, 
    // The attr for a single cell coords, applies to all areas
    wxSHEET_AttrCell    = 0x00020, 
    // The attr for a row of cells, only applies to the grid cells
    //  not label attrs, they only have default or single cell
    wxSHEET_AttrRow     = 0x00040, 
    // The attr for a col of cells, only applies to the grid cells
    //  not label attrs, they only have default or single cell
    wxSHEET_AttrCol     = 0x00080, 
    // Only for getting attrs, get an attr that may be merged from cell/row/col
    //  attrs, if none of those then resort to default attr for area
    //  this is the best choice for getting an attr since it's guaranteed to work
    wxSHEET_AttrAny     = wxSHEET_AttrDefault|wxSHEET_AttrCell|wxSHEET_AttrRow|wxSHEET_AttrCol
};    

// ----------------------------------------------------------------------------
// Alignment of the text (or whatever) for a sheet cell attribute
enum wxSheetAttrAlign_Type
{
    // unfortunately wxALIGN_TOP=wxALIGN_LEFT=wxALIGN_NOT=0, use these for unset
    wxSHEET_AttrAlignHorizUnset  = 0x01000,
    wxSHEET_AttrAlignVertUnset   = 0x02000,
    wxSHEET_AttrAlignUnset       = (wxSHEET_AttrAlignHorizUnset|wxSHEET_AttrAlignVertUnset),
    
    wxSHEET_AttrAlignCenterHoriz = wxALIGN_CENTER_HORIZONTAL, // = 0x0100,
    wxSHEET_AttrAlignLeft        = wxALIGN_LEFT,              // = wxALIGN_NOT,
    wxSHEET_AttrAlignTop         = wxALIGN_TOP,               // = wxALIGN_NOT,
    wxSHEET_AttrAlignRight       = wxALIGN_RIGHT,             // = 0x0200,
    wxSHEET_AttrAlignBottom      = wxALIGN_BOTTOM,            // = 0x0400,
    wxSHEET_AttrAlignCenterVert  = wxALIGN_CENTER_VERTICAL,   // = 0x0800,
    wxSHEET_AttrAlignCenter      = wxALIGN_CENTER,            // = (wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL),
    wxSHEET_AttrAlignType_Mask   = (wxALIGN_MASK|wxSHEET_AttrAlignUnset), // = 0x0f00

    // A bit mask for the wxALIGN_XXX enums for extracting horiz and vert alignments
    wxSHEET_AttrAlignHoriz_Mask  = (wxALIGN_LEFT|wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL|wxSHEET_AttrAlignHorizUnset),     
    wxSHEET_AttrAlignVert_Mask   = (wxALIGN_TOP|wxALIGN_BOTTOM|wxALIGN_CENTER_VERTICAL|wxSHEET_AttrAlignVertUnset)     
};

// ----------------------------------------------------------------------------
// Orientation of the text (or whatever) for a sheet cell attribute
enum wxSheetAttrOrientation_Type
{
    wxSHEET_AttrOrientUnset     = 0,
    wxSHEET_AttrOrientHoriz     = wxHORIZONTAL, // 0x0004 Horizontally orientated
    wxSHEET_AttrOrientVert      = wxVERTICAL,   // 0x0008 Vertically orientated
    wxSHEET_AttrOrientType_Mask = (wxSHEET_AttrOrientHoriz|wxSHEET_AttrOrientVert)
};

// ----------------------------------------------------------------------------
// Level of the attribute if there are multiple ones per cell
//    ie. row and/or col and/or cell + default (note: default should be bottom)
enum wxSheetAttrLevel_Type
{
    // no unset, every attr has a level
    wxSHEET_AttrLevelBottom    = 0x0010000,
    wxSHEET_AttrLevelMiddle    = 0x0020000,
    wxSHEET_AttrLevelTop       = 0x0040000,
    wxSHEET_AttrLevelType_Mask = (wxSHEET_AttrLevelBottom|wxSHEET_AttrLevelMiddle|wxSHEET_AttrLevelTop)
};

// ----------------------------------------------------------------------------
// Read/Write mode of an attribute
enum wxSheetAttrRead_Type
{
    wxSHEET_AttrReadUnset     = 0,
    wxSHEET_AttrReadWrite     = 0x0100000,
    wxSHEET_AttrReadOnly      = 0x0200000, 
    wxSHEET_AttrReadType_Mask = (wxSHEET_AttrReadWrite|wxSHEET_AttrReadOnly)
};

// ----------------------------------------------------------------------------
// Can the cell contents overflow in the cells to the right?
enum wxSheetAttrOverflow_Type
{
    wxSHEET_AttrOverflowUnset     = 0,
    wxSHEET_AttrOverflow          = 0x0400000,
    wxSHEET_AttrOverflowNot       = 0x0800000,
    wxSHEET_AttrOverflowType_Mask = (wxSHEET_AttrOverflow|wxSHEET_AttrOverflowNot)
};

// ----------------------------------------------------------------------------
// If the cell contents overflows then should a |> tick mark be drawn to show it
enum wxSheetAttrOverflowMarker_Type
{
    wxSHEET_AttrOverflowMarkerUnset     = 0,
    wxSHEET_AttrOverflowMarker          = 0x1000000,
    wxSHEET_AttrOverflowMarkerNot       = 0x2000000,
    wxSHEET_AttrOverflowMarkerType_Mask = (wxSHEET_AttrOverflowMarker|wxSHEET_AttrOverflowMarkerNot)
};

class wxSheetCellAttr : public wxObject
{
public:
    // if create then create with ref data
    wxSheetCellAttr(bool create = FALSE);

    // Recreate the ref data, unrefing the old
    bool Create();
    void Destroy() { UnRef(); }
    inline bool Ok() const { return m_refData != NULL; }

    // Makes a full new unrefed copy of the other
    bool Copy(const wxSheetCellAttr& other);
    // Copies the values from the other, but only if the other has them
    bool UpdateWith(const wxSheetCellAttr& other);
    // Merges this with the other, copy values of other only this doesn't have them
    bool MergeWith(const wxSheetCellAttr &mergefrom);
    
    // setters
    void SetForegroundColour(const wxColour& foreColour);
    void SetBackgroundColour(const wxColour& backColour);
    void SetFont(const wxFont& font);
    // wxSheetAttrAlign_Type, (same as wxALIGN)
    void SetAlignment(int align);
    void SetAlignment(int horzAlign, int vertAlign);
    // wxSheetAttrOrientation_Type (same as wxORIENTATION)
    void SetOrientation(int orientation);
    void SetLevel(wxSheetAttrLevel_Type level);
    void SetOverflow(bool allow);
    void SetOverflowMarker(bool draw_marker);
    void SetReadOnly(bool isReadOnly);
    void SetRenderer(const wxSheetCellRenderer& renderer);
    void SetEditor(const wxSheetCellEditor& editor);
    void SetKind(wxSheetAttr_Type kind);

    // validation
    bool HasForegoundColour() const;
    bool HasBackgroundColour() const;
    bool HasFont() const;
    bool HasAlignment() const;
    bool HasOrientation() const;
    bool HasLevel() const;
    bool HasOverflowMode() const;
    bool HasOverflowMarkerMode() const;
    bool HasReadWriteMode() const;
    bool HasRenderer() const;
    bool HasEditor() const;
    bool HasDefaultAttr() const;
    // HasKind - always has kind, default is wxSHEET_AttrCell

    // does this attr define all the HasXXX properties, except DefaultAttr
    //   if this is true, it's a suitable default attr for an area
    bool IsComplete() const;  

    // accessors
    const wxColour& GetForegroundColour() const;
    const wxColour& GetBackgroundColour() const;
    const wxFont& GetFont() const;
    int GetAlignment() const;
    wxOrientation GetOrientation() const;
    wxSheetAttrLevel_Type GetLevel() const;
    bool GetOverflow() const;
    bool GetOverflowMarker() const;
    bool GetReadOnly() const;
    wxSheetCellRenderer GetRenderer(wxSheet* grid, const wxSheetCoords& coords) const;
    wxSheetCellEditor GetEditor(wxSheet* grid, const wxSheetCoords& coords) const;
    wxSheetAttr_Type GetKind() const;

    // any unset values of this attr are retrieved from the default attr
    // if you try to set the def attr to this, it's ignored
    // don't bother to link multiple attributes together in a loop, obviously.
    const wxSheetCellAttr& GetDefaultAttr() const;
    void SetDefaultAttr(const wxSheetCellAttr& defaultAttr);

    DECLARE_SHEETOBJ_COPY_CLASS(wxSheetCellAttr)    
};

// ----------------------------------------------------------------------------
// wxSheetCellAttrRefData : data for the wxSheetCellAttr
// 
// only use this as a LAST resort to overriding the behavior
// ----------------------------------------------------------------------------
class wxSheetCellAttrRefData : public wxObjectRefData
{
public:    
    wxSheetCellAttrRefData();
    virtual ~wxSheetCellAttrRefData();

    wxColour m_foreColour,
             m_backColour;
    wxFont   m_font;
    // stores wxSheetAttr_Type, align, orient, overflow, read, level
    wxUint32 m_attrTypes; 
    
    // these are pointers since we can't define the edit/ren/attr all at once
    wxSheetCellRenderer *m_renderer;
    wxSheetCellEditor   *m_editor;
    wxSheetCellAttr     *m_defaultAttr;
};

// A uncreated wxSheetCellAttr for use when there's no attribute
extern const wxSheetCellAttr wxNullSheetCellAttr;

#include "sheetctl.h"  // have to include this here to get renderer/editor 

WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetCellAttr, wxArraySheetCellAttr,
                              class );

DECLARE_PAIRED_DATA_ARRAYS(int, wxArrayInt, wxSheetCellAttr, wxArraySheetCellAttr, wxPairArrayIntSheetCellAttr, class )
DECLARE_PAIRED_DATA_ARRAYS(wxSheetCoords, wxArraySheetCoords, wxSheetCellAttr, wxArraySheetCellAttr, wxPairArraySheetCoordsCellAttrBase, class )

class wxPairArraySheetCoordsCellAttr : public wxPairArraySheetCoordsCellAttrBase
{
public:
    void UpdateRows( size_t pos, int numRows );
    void UpdateCols( size_t pos, int numRows );
};

// ----------------------------------------------------------------------------
// wxSheetCellAttrProvider : for wxSheetTableBase to retrieve/store cell attr
// ----------------------------------------------------------------------------

// implementation note: we separate it from wxSheetTableBase because we wish to
// avoid deriving a new table class if possible, and sometimes it will be
// enough to just derive another wxSheetCellAttrProvider instead
//
// the default implementation is reasonably efficient for the generic case,
// but you might still wish to implement your own for some specific situations
// if you have performance problems with the stock one
class wxSheetCellAttrProvider : public wxClientDataContainer
{
public:
    wxSheetCellAttrProvider() {}
    virtual ~wxSheetCellAttrProvider() {}

    // Get the attribute for the coords of type wxSheetAttr_Type
    // you must implement these cell coords for grid, row/col/corner labels
    // see wxSheet::IsGridCell/IsRowLabelCell/IsColLabelCell/IsCornerLabelCell
    // and all types except wxSHEET_AttrDefault, which sheet should have handled
    // If a type wxSHEET_AttrCell/Row/Col return wxNullSheetCellAttr if one is
    // not set. Type wxSHEET_AttrAny is used to merge row/col/cell attr
    // based on their level.
    // return wxNullSheetCellAttr if none set for coords and type
    virtual wxSheetCellAttr GetAttr( const wxSheetCoords& coords,
                                     wxSheetAttr_Type type );
    
    // Set the attribute for the coords, see GetAttr for coords and type
    //  if the !attr.Ok() the attr is removed w/o error even if it didn't exist
    // use wxNullSheetCellAttr to remove attr for coords and type
    virtual void SetAttr( const wxSheetCoords& coords, 
                          const wxSheetCellAttr &attr,
                          wxSheetAttr_Type type );

    // Update internal data whenever # rows/cols change (must be called)
    //  this shifts rows/cols and deletes them as appropriate
    virtual void UpdateAttrRows( size_t pos, int numRows );
    virtual void UpdateAttrCols( size_t pos, int numCols );

protected:
    wxPairArraySheetCoordsCellAttr m_cellAttrs;

    wxPairArrayIntSheetCellAttr m_rowAttrs,
                                m_colAttrs,
                                m_rowLabelAttrs,
                                m_colLabelAttrs;

    DECLARE_NO_COPY_CLASS(wxSheetCellAttrProvider)
};

// ----------------------------------------------------------------------------
// wxSheetTableMessage - messages from the table back to the sheet
// ----------------------------------------------------------------------------

enum wxSheetTableRequest       // IDs for messages sent from grid table to view
{
    wxSHEETTABLE_REQUEST_VIEW_GET_VALUES = 2000,
    wxSHEETTABLE_REQUEST_VIEW_SEND_VALUES,
    wxSHEETTABLE_NOTIFY_ROWS_INSERTED,
    wxSHEETTABLE_NOTIFY_ROWS_APPENDED,
    wxSHEETTABLE_NOTIFY_ROWS_DELETED,
    wxSHEETTABLE_NOTIFY_COLS_INSERTED,
    wxSHEETTABLE_NOTIFY_COLS_APPENDED,
    wxSHEETTABLE_NOTIFY_COLS_DELETED
};

class wxSheetTableMessage
{
public:
    wxSheetTableMessage() : m_table(NULL), m_id(-1), m_comInt1(-1), m_comInt2(-1) {}
    wxSheetTableMessage( wxSheetTableBase *table, int id, 
                         int comInt1 = -1, int comInt2 = -1 )
        : m_table(table), m_id(id), m_comInt1(comInt1), m_comInt2(comInt2) {}

    void SetTableObject( wxSheetTableBase *table ) { m_table = table; }
    wxSheetTableBase* GetTableObject() const       { return m_table; }
    
    void SetId( int id )               { m_id = id; }
    int  GetId()                       { return m_id; }

    void SetCommandInt( int comInt1 )  { m_comInt1 = comInt1; }
    void SetCommandInt2( int comInt2 ) { m_comInt2 = comInt2; }
    int  GetCommandInt()               { return m_comInt1; }
    int  GetCommandInt2()              { return m_comInt2; }
    
    // For redimensioning these values are used
    int GetPosition()                  { return m_comInt1; }
    int GetRowsCols()                  { return m_comInt2; }

protected:
    wxSheetTableBase *m_table;
    wxWindowID m_id;
    int m_comInt1;
    int m_comInt2;

    DECLARE_NO_COPY_CLASS(wxSheetTableMessage)
};

// ----------------------------------------------------------------------------
// wxSheetTableBase - base table for the wxSheet
//
// Note: functions that take coords use the -1 notation, if you don't want
//  to handle it yourself, but do for the grid cells you should call
//  if (!ContainsGridCell(coords)) return wxSheetTableBase::GetValue(coords);
//  to have the default processing occur
// ----------------------------------------------------------------------------
class wxSheetTableBase : public wxObject, public wxClientDataContainer
{
public:
    wxSheetTableBase( wxSheet *sheet = NULL );
    virtual ~wxSheetTableBase();

    // You must override these functions in a derived table class
    virtual int GetNumberRows() = 0;
    virtual int GetNumberCols() = 0;
    // determine if row/col/coords is in 0 to GetNumberRows/Cols-1
    bool ContainsGridRow( int row ) { return (row >= 0) && (row < GetNumberRows()); }
    bool ContainsGridCol( int col ) { return (col >= 0) && (col < GetNumberCols()); }
    bool ContainsGridCell(const wxSheetCoords& coords)
        { return ContainsGridRow(coords.m_row) && ContainsGridCol(coords.m_col); }
    virtual bool IsEmptyCell( const wxSheetCoords& coords ) { return GetValue(coords).IsEmpty(); }
    // renderer helper, returns first col < coords.m_col that's !empty
    //  or just return the previous col if you can't implement that
    virtual int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords ) { return coords.m_col-1; }
    
    // Get/SetValue as a string, coords use -1 notatation for label cells
    //   base version can handles direction to row/col/corner/label values
    virtual wxString GetValue( const wxSheetCoords& coords );
    virtual void SetValue( const wxSheetCoords& coords, const wxString& value );

    // Data type determination and value access
    virtual wxString GetTypeName( const wxSheetCoords& coords );
    virtual bool CanGetValueAs( const wxSheetCoords& coords, const wxString& typeName );
    virtual bool CanSetValueAs( const wxSheetCoords& coords, const wxString& typeName );

    virtual long   GetValueAsLong( const wxSheetCoords& coords );
    virtual double GetValueAsDouble( const wxSheetCoords& coords );
    virtual bool   GetValueAsBool( const wxSheetCoords& coords );

    virtual void SetValueAsLong( const wxSheetCoords& coords, long value );
    virtual void SetValueAsDouble( const wxSheetCoords& coords, double value );
    virtual void SetValueAsBool( const wxSheetCoords& coords, bool value );

    // For user defined types
    virtual void* GetValueAsCustom( const wxSheetCoords& coords, const wxString& typeName );
    virtual void  SetValueAsCustom( const wxSheetCoords& coords, const wxString& typeName, void* value );

    // Overriding these is optional
    virtual void SetView( wxSheet *grid ) { m_view = grid; }
    virtual wxSheet* GetView() const { return m_view; }

    virtual void Clear() {}
    // if you use these you must override them, the default gives error message
    virtual bool InsertRows( size_t row = 0, size_t numRows = 1 );
    virtual bool AppendRows( size_t numRows = 1 );
    virtual bool DeleteRows( size_t row = 0, size_t numRows = 1 );
    virtual bool InsertCols( size_t col = 0, size_t numCols = 1 );
    virtual bool AppendCols( size_t numCols = 1 );
    virtual bool DeleteCols( size_t col = 0, size_t numCols = 1 );

    // sends an appropriate wxSheetTableMessage to the wxSheet
    //   call these after Insert/Append/Delete/Rows/Cols to update the sheet
    bool SendInsertRowsTableMsg( size_t row = 0, size_t numRows = 1 );
    bool SendAppendRowsTableMsg( size_t numRows = 1 );
    bool SendDeleteRowsTableMsg( size_t row = 0, size_t numRows = 1 );
    bool SendInsertColsTableMsg( size_t col = 0, size_t numCols = 1 );
    bool SendAppendColsTableMsg( size_t numCols = 1 );
    bool SendDeleteColsTableMsg( size_t col = 0, size_t numCols = 1 );

    // If you have a pure virtual table and you merely want to alert the sheet
    //  that the number of rows/cols have changed then call this function.
    //  It compares the wxSheetTable::GetNumberRows/Cols to 
    //  wxSheet::GetNumberRows/Cols and either appends Rows/Cols or deletes 
    //  them from the end by sending a appropriate wxSheetTableMessage.
    //  You probably won't want to use this if you've set attributes for 
    //  particular cells since only appends and deletes from the end.
    virtual bool SendUpdateTableMsg();

    // default GetRow/ColLabelValue returns numbers for rows and A-AZ for cols
    virtual wxString GetRowLabelValue( int row );
    virtual wxString GetColLabelValue( int col );
    virtual void SetRowLabelValue( int WXUNUSED(row), const wxString& ) {}
    virtual void SetColLabelValue( int WXUNUSED(col), const wxString& ) {}

    // Attribute handling

    // Set the attr provider to use - take ownership if is_owner
    virtual void SetAttrProvider(wxSheetCellAttrProvider *attrProvider, 
                                 bool is_owner = TRUE);
    // get the currently used attr provider (may be NULL)
    virtual wxSheetCellAttrProvider* GetAttrProvider() const { return m_attrProvider; }
    // Does this table allow attributes?  Default implementation creates
    // a wxSheetCellAttrProvider if necessary and returns TRUE.
    virtual bool CanHaveAttributes();
    // by default forwarded to wxSheetCellAttrProvider if any. May be
    // overridden to handle attributes directly in the table.
    // See wxSheetCellAttrProvider for coords and type meaning
    virtual wxSheetCellAttr GetAttr( const wxSheetCoords& coords,
                                     wxSheetAttr_Type kind );
    
    // See wxSheetCellAttrProvider for coords and type meaning
    virtual void SetAttr( const wxSheetCoords& coords, 
                          const wxSheetCellAttr &attr, 
                          wxSheetAttr_Type kind );

protected:
    wxSheet *m_view;
    wxSheetCellAttrProvider *m_attrProvider;
    bool m_own_attr_provider;

    DECLARE_ABSTRACT_CLASS(wxSheetTableBase);
    DECLARE_NO_COPY_CLASS(wxSheetTableBase)
};

// ----------------------------------------------------------------------------
// wxSheetStringTable: data table for small tables of strings stored in memory
// 
// This is suited to small tables or tables that are expected to be mostly full
// The data is stored in wxArrayStrings as a 2x2 matrix [rows][cols]
// ----------------------------------------------------------------------------

class wxSheetStringTable : public wxSheetTableBase
{
public:
    wxSheetStringTable() : wxSheetTableBase() {}
    wxSheetStringTable( int numRows, int numCols );
    virtual ~wxSheetStringTable() {}

    // these are pure virtual in wxSheetTableBase
    int GetNumberRows();
    int GetNumberCols();
    wxString GetValue( const wxSheetCoords& coords );
    void SetValue( const wxSheetCoords& coords, const wxString& value );
    bool IsEmptyCell( const wxSheetCoords& coords );

    // overridden functions from wxSheetTableBase
    void Clear();
    bool InsertRows( size_t pos = 0, size_t numRows = 1 );
    bool AppendRows( size_t numRows = 1 );
    bool DeleteRows( size_t pos = 0, size_t numRows = 1 );
    bool InsertCols( size_t pos = 0, size_t numCols = 1 );
    bool AppendCols( size_t numCols = 1 );
    bool DeleteCols( size_t pos = 0, size_t numCols = 1 );

    void SetRowLabelValue( int row, const wxString& value );
    void SetColLabelValue( int col, const wxString& value );
    wxString GetRowLabelValue( int row );
    wxString GetColLabelValue( int col );

protected:
    wxGridStringArray m_data;

    // These only get used if you set your own labels, otherwise the
    // GetRow/ColLabelValue functions return wxxGridTableBase defaults
    wxArrayString     m_rowLabels;
    wxArrayString     m_colLabels;

    DECLARE_DYNAMIC_CLASS_NO_COPY( wxSheetStringTable )
};

// ----------------------------------------------------------------------------
// wxSheetStringSparseTable: data table for tables of strings stored in memory
//
// This table is designed for grids that will be less than half-full, tables that 
// you expect to be full should use the wxSheetStringTable as it will have a 
// higher performance and smaller size.
// ----------------------------------------------------------------------------

// Make a int, wxString pair array for row/col labels and for grid cols
DECLARE_PAIRED_DATA_ARRAYS(int, wxArrayInt, wxString, wxArrayString, wxPairArrayIntSheetString, class )
// Make a pair array of int, (int, wxString) pair arrays for rows     
WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString,
                              class );
DECLARE_PAIRED_DATA_ARRAYS(int, wxArrayInt, wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString, wxPairArrayIntPairArraySheetStringBase, class )

class wxPairArrayIntPairArraySheetString : public wxPairArrayIntPairArraySheetStringBase
{
public:
    void RemoveEmptyRows();
};

class wxSheetStringSparseTable : public wxSheetTableBase
{
public:
    wxSheetStringSparseTable() : wxSheetTableBase(), m_numRows(0), m_numCols(0) {}
    wxSheetStringSparseTable( int numRows, int numCols );
    virtual ~wxSheetStringSparseTable() {}

    // these are pure virtual in wxSheetTableBase
    int GetNumberRows() { return m_numRows; }
    int GetNumberCols() { return m_numCols; }
    wxString GetValue( const wxSheetCoords& coords );
    void SetValue( const wxSheetCoords& coords, const wxString& value );
    bool IsEmptyCell( const wxSheetCoords& coords );
    int GetFirstNonEmptyColToLeft( const wxSheetCoords& coords );

    // overridden functions from wxSheetTableBase
    void Clear();
    bool InsertRows( size_t row = 0, size_t numRows = 1 );
    bool AppendRows( size_t numRows = 1 );
    bool DeleteRows( size_t row = 0, size_t numRows = 1 );
    bool InsertCols( size_t col = 0, size_t numCols = 1 );
    bool AppendCols( size_t numCols = 1 );
    bool DeleteCols( size_t col = 0, size_t numCols = 1 );

    void SetRowLabelValue( int row, const wxString& value );
    void SetColLabelValue( int col, const wxString& value );
    wxString GetRowLabelValue( int row );
    wxString GetColLabelValue( int col );

protected:
    wxPairArrayIntPairArraySheetString m_data;
    int m_numRows, m_numCols;

    // These only get used if you set your own labels, otherwise the
    // GetRow/ColLabelValue functions return wxSheetTableBase defaults
    wxPairArrayIntSheetString m_rowLabels;
    wxPairArrayIntSheetString m_colLabels;

    DECLARE_DYNAMIC_CLASS_NO_COPY( wxSheetStringSparseTable )
};

// ----------------------------------------------------------------------------
// wxSheetRowLabelWindow - the row label window (left window)
// ----------------------------------------------------------------------------
class wxSheetRowLabelWindow : public wxWindow
{
public:
    wxSheetRowLabelWindow() : m_owner(NULL) { }
    wxSheetRowLabelWindow( wxSheet *parent, wxWindowID id = -1,
                           const wxPoint &pos = wxDefaultPosition, 
                           const wxSize &size = wxDefaultSize );

    void OnPaint( wxPaintEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );

    wxSheet* GetOwner() { return m_owner; }
    virtual void PrepareDC( wxDC& dc );
    
    wxSheet *m_owner;
    int      m_mouseCursor;
    
private:
    DECLARE_DYNAMIC_CLASS(wxSheetRowLabelWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetRowLabelWindow)
};

// ----------------------------------------------------------------------------
// wxSheetColLabelWindow - col label window (top window)
// ----------------------------------------------------------------------------
class wxSheetColLabelWindow : public wxWindow
{
public:
    wxSheetColLabelWindow() : m_owner(NULL) { }
    wxSheetColLabelWindow( wxSheet *parent, wxWindowID id = -1,
                           const wxPoint &pos = wxDefaultPosition, 
                           const wxSize &size = wxDefaultSize );

    void OnPaint( wxPaintEvent &event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );

    wxSheet* GetOwner() { return m_owner; }
    virtual void PrepareDC( wxDC& dc );

    wxSheet *m_owner;
    int      m_mouseCursor;

private:
    DECLARE_DYNAMIC_CLASS(wxSheetColLabelWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetColLabelWindow)
};

// ----------------------------------------------------------------------------
// wxSheetCornerLabelWindow - corner label window (upper left window)
// ----------------------------------------------------------------------------
class wxSheetCornerLabelWindow : public wxWindow
{
public:
    wxSheetCornerLabelWindow() : m_owner(NULL) { }
    wxSheetCornerLabelWindow( wxSheet *parent, wxWindowID id = -1,
                              const wxPoint &pos = wxDefaultPosition, 
                              const wxSize &size = wxDefaultSize );

    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnPaint( wxPaintEvent& event );
    void OnChar( wxKeyEvent& event );

    wxSheet* GetOwner() { return m_owner; }

    wxSheet *m_owner;
    int      m_mouseCursor;
    
private:
    DECLARE_DYNAMIC_CLASS(wxSheetCornerLabelWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetCornerLabelWindow)
};

// ----------------------------------------------------------------------------
// wxSheetGridWindow - the grid window
// ----------------------------------------------------------------------------
class wxSheetGridWindow : public wxWindow
{
public:
    wxSheetGridWindow() : m_owner(NULL) {}
    wxSheetGridWindow( wxSheet *parent, wxWindowID id = -1, 
                       const wxPoint &pos = wxDefaultPosition, 
                       const wxSize &size = wxDefaultSize );
    
    void OnPaint( wxPaintEvent &event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );
    void OnFocus( wxFocusEvent& event );
    void OnEraseBackground( wxEraseEvent& ) {}

    wxSheet* GetOwner() { return m_owner; }
    virtual void PrepareDC( wxDC& dc );

    wxSheet *m_owner;
    int      m_mouseCursor;

private:
    DECLARE_DYNAMIC_CLASS(wxSheetGridWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetGridWindow)
};

//-----------------------------------------------------------------------------
// wxSheetRefData - ref counted data to share with refed wxSheets
//
// Please use accessors in the wxSheet to adjust the values, for very 
// complicated scenarios adjust these as a last resort.
// 
// This MUST always exist as the wxObject::m_refData of the wxSheet. It is 
// automatically created in wxSheet::Init, if you wish to use your own 
// subclassed refdata then in your wxSheet constructor (or Create) function call
// UnRef to delete the original and m_refData=new MySheetRefData to set the new.
//-----------------------------------------------------------------------------
class wxSheetRefData : public wxObjectRefData
{
public:
    wxSheetRefData();
    virtual ~wxSheetRefData();

    // Find/Add/Remove sheets that share this data - used for wxSheetSplitter
    int  FindSheet(wxSheet* sheet) const;
    bool HasSheet(wxSheet* sheet) const { return FindSheet(sheet) != wxNOT_FOUND; }
    void AddSheet(wxSheet* sheet);
    void RemoveSheet(wxSheet* sheet);

    // Refresh the other refed sheets - used in wxSheetSplitter
    void RefreshGridWindow(wxSheet* sheet, const wxRect* rect);
    void RefreshRowLabelWindow(wxSheet* sheet, const wxRect* rect);
    void RefreshColLabelWindow(wxSheet* sheet, const wxRect* rect);
    void RefreshCornerLabelWindow(wxSheet* sheet, const wxRect* rect);
    
    wxArrayPtrVoid m_sheets;             // list of sheets sharing this data

    wxSheetTableBase *m_table;           // the table that stores the values
    bool              m_ownTable;

    wxSheetTypeRegistry* m_typeRegistry; // editor/renderer registry

    int m_numRows;                       // number of rows/cols of grid
    int m_numCols;                       //  should match table size

    // NB: *never* access m_row/col arrays directly because they are created
    //     on demand, *always* use accessor functions instead!
    int        m_defaultRowHeight;       
    int        m_defaultColWidth;
    int        m_minAcceptableColWidth;
    int        m_minAcceptableRowHeight;
    wxArrayInt m_rowHeights;
    wxArrayInt m_colWidths;
    wxArrayInt m_rowBottoms;
    wxArrayInt m_colRights;

    // if a column has a non default min width, it's in this hash table
    wxLongToLongHashMap m_colMinWidths,
                        m_rowMinHeights;

    // width of row labels and height of col labels
    int        m_rowLabelWidth;
    int        m_colLabelHeight;

    // the size of the margin to the right and bottom of the cell area
    int        m_extraWidth;
    int        m_extraHeight;

    int        m_equal_col_widths;

    wxColour   m_gridLineColour;
    bool       m_gridLinesEnabled;
    wxColour   m_cursorCellHighlightColour;
    int        m_cursorCellHighlightPenWidth;
    int        m_cursorCellHighlightROPenWidth;
    wxColour   m_labelOutlineColour;

    wxCursor   m_rowResizeCursor;
    wxCursor   m_colResizeCursor;

    bool       m_editable;    // applies to whole grid

    bool    m_canDragRowSize;
    bool    m_canDragColSize;
    bool    m_canDragGridSize;

    wxString m_cornerLabelValue;

    wxSheetCoords m_cursorCoords;

    wxSheetBlock      m_selectingBlock;   // during selection !Empty
    wxSheetCoords     m_selectingAnchor;  // corner of active selection, other is usually cursor
    wxSheetSelection  *m_selection;
    wxSheetSelectionMode_Type m_selectionMode;
    wxColour          m_selectionBackground;
    wxColour          m_selectionForeground;

    // the default cell attr objects for cells that don't have their own
    wxSheetCellAttr m_defaultGridCellAttr;
    wxSheetCellAttr m_defaultRowLabelAttr;
    wxSheetCellAttr m_defaultColLabelAttr;
    wxSheetCellAttr m_defaultCornerLabelAttr;

    wxSheetSelection *m_spannedCells;
    
    wxSheetCellEditor m_cellEditor;       // valid only when editing
    wxSheetCoords     m_cellEditorCoords;

    wxPairArrayIntPairArraySheetString m_copiedData;
    bool                               m_pasting;
};

// ----------------------------------------------------------------------------
// wxSheet
// ----------------------------------------------------------------------------
class wxSheet : public wxWindow
{
public:
    wxSheet() : m_gridWin(NULL) { Init(); }
    
    wxSheet( wxWindow *parent, wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxWANTS_CHARS,
             const wxString& name = wxT("wxSheet") ) : m_gridWin(NULL)
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
    
    // ------------------------------------------------------------------------
    // Create/Set/Get wxSheetTable - the underlying data to be displayed

    wxSheetTableBase* GetTable() const { return GetSheetRefData()->m_table; }
    
    // Create a wxSheetStringTable to use
    bool CreateGrid( int numRows, int numCols );

    // Set your own table derived from wxSheetTableBase
    bool SetTable( wxSheetTableBase *table, bool takeOwnership );  

    // ------------------------------------------------------------------------
    // Dimensions of the number of cells on the sheet and helper cell functions
    
    int GetNumberRows() const { return GetSheetRefData()->m_numRows; }
    int GetNumberCols() const { return GetSheetRefData()->m_numCols; }

    // Is the coords anywhere in labels or grid, -1 to GetNumberRows/Cols()-1
    bool ContainsCell( const wxSheetCoords& coords ) const 
        { return (coords.m_row >= -1) && (coords.m_col >= -1) &&
                 (coords.m_row < GetSheetRefData()->m_numRows) && 
                 (coords.m_col < GetSheetRefData()->m_numCols); }
                 
    // returns TRUE if the coords are within the grid cells of the sheet
    bool ContainsGridRow( int row ) const { return (row >= 0) && (row < GetSheetRefData()->m_numRows); }
    bool ContainsGridCol( int col ) const { return (col >= 0) && (col < GetSheetRefData()->m_numCols); }
    bool ContainsGridCell( const wxSheetCoords& coords ) const
        { return ContainsGridRow(coords.m_row) && ContainsGridCol(coords.m_col); }

    // retruns TRUE if the coords are within the row/col label cells
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
    void ClearGrid(); 
        
    // Add/delete rows and cols to the grid cells
    bool DeleteRows( int pos = 0, int numRows = 1, bool updateLabels = TRUE );
    bool DeleteCols( int pos = 0, int numCols = 1, bool updateLabels = TRUE );
    bool InsertRows( int pos = 0, int numRows = 1, bool updateLabels = TRUE );
    bool InsertCols( int pos = 0, int numCols = 1, bool updateLabels = TRUE );
    bool AppendRows( int numRows = 1, bool updateLabels = TRUE );
    bool AppendCols( int numCols = 1, bool updateLabels = TRUE );

    // Set exactly the number of rows or cols, these functions Append or
    // Delete rows/cols to/from the end. If you are setting attributes for 
    // particular cells/rows/cols you probably won't want to use these
    bool SetNumberRows( int rows );
    bool SetNumberCols( int cols );
    bool SetNumberCells( int rows, int cols );
        
    // ------------------------------------------------------------------------
    // Dimensions of the row and column sizes
    
    // Get/Set the default height/width of newly created cells
    int  GetDefaultRowHeight() const { return GetSheetRefData()->m_defaultRowHeight; }
    int  GetDefaultColWidth()  const { return GetSheetRefData()->m_defaultColWidth; }
    void SetDefaultRowHeight( int height, bool resizeExistingRows = FALSE );
    void SetDefaultColWidth( int width, bool resizeExistingCols = FALSE );
    
    // Get/Set the absolute min row/col width/height, 0 for no min size
    //   Call during grid creation, existing rows/cols are not resized
    int  GetMinimalAcceptableColWidth()  const { return GetSheetRefData()->m_minAcceptableColWidth; }
    int  GetMinimalAcceptableRowHeight() const { return GetSheetRefData()->m_minAcceptableRowHeight; }
    void SetMinimalAcceptableColWidth( int width )   { GetSheetRefData()->m_minAcceptableColWidth  = wxMax(0,width); }
    void SetMinimalAcceptableRowHeight( int height ) { GetSheetRefData()->m_minAcceptableRowHeight = wxMax(0,height); }

    // Don't allow specific rows/cols to be resized smaller than this
    //   Call during grid creation, existing rows/cols are not resized
    //   The setting is cleared to default val if width/height is < min acceptable
    int  GetMinimalColWidth(int col) const;
    int  GetMinimalRowHeight(int col) const;
    void SetMinimalColWidth( int col, int width );
    void SetMinimalRowHeight( int row, int height );

    // Get the height/top/bottom for rows, uses -1 notation
    int  GetRowHeight(int row) const;
    int  GetRowTop(int row) const;
    int  GetRowBottom(int row) const;
    // Get the width/left/right for cols, uses -1 notation
    int  GetColWidth(int col) const;
    int  GetColLeft(int col) const;
    int  GetColRight(int col) const;
    // Get the width, height of a cell as a wxSize, -1 notation
    //  this does not include spanned cells
    wxSize GetCellSize(const wxSheetCoords& coords) const;
    
    // Get the renderer's best size for the cell, uses -1 notation
    wxSize GetCellBestSize(const wxSheetCoords& coords, wxDC *dc=NULL) const;
    
    // does the cell have a non-zero width and height, may not be visible, -1 notation
    bool IsCellShown( const wxSheetCoords& coords ) const;
    
    // Set the height of a row or width of a col, -1 notation
    void SetRowHeight( int row, int height );
    void SetColWidth(  int col, int width );
   
    // grid may occupy more space than needed for its rows/columns, this
    // function allows to set how big this extra space is
    void SetMargins(int extraWidth, int extraHeight)
        { GetSheetRefData()->m_extraWidth=extraWidth; 
          GetSheetRefData()->m_extraHeight=extraHeight; CalcWindowSizes();}
    
    // ------------------------------------------------------------------------
    // Auto sizing of the row/col widths/heights
    
    // automatically size the col/row to fit to its contents, if setAsMin, 
    // this optimal width will also be set as minimal width for this column
    void AutoSizeColumn( int col, bool setAsMin = TRUE ) { AutoSizeColOrRow(col, setAsMin, TRUE); }
    void AutoSizeRow( int row, bool setAsMin = TRUE )    { AutoSizeColOrRow(row, setAsMin, FALSE); }

    // auto size all columns (very ineffective for big grids!)
    void AutoSizeColumns( bool setAsMin = TRUE ) { (void)SetOrCalcColumnSizes(FALSE, setAsMin); }
    void AutoSizeRows( bool setAsMin = TRUE )    { (void)SetOrCalcRowSizes(FALSE, setAsMin); }

    // auto size the grid, that is make the columns/rows of the "right" size
    // and also set the grid size to just fit its contents
    void AutoSize();

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
    //   else you resize in the label windows (if CanDragRow/ColSize is TRUE)
    
    void EnableDragRowSize( bool enable = TRUE ) { GetSheetRefData()->m_canDragRowSize = enable; }
    void EnableDragColSize( bool enable = TRUE ) { GetSheetRefData()->m_canDragColSize = enable; }
    void EnableDragGridSize(bool enable = TRUE ) { GetSheetRefData()->m_canDragGridSize = enable; }
    void DisableDragRowSize()                    { EnableDragRowSize( FALSE ); }
    void DisableDragColSize()                    { EnableDragColSize( FALSE ); }
    void DisableDragGridSize()                   { EnableDragGridSize(FALSE); }
    bool CanDragRowSize()  const                 { return GetSheetRefData()->m_canDragRowSize; }
    bool CanDragColSize()  const                 { return GetSheetRefData()->m_canDragColSize; }
    bool CanDragGridSize() const                 { return GetSheetRefData()->m_canDragGridSize; }
    
    // ------------------------------------------------------------------------
    // Grid line, cell highlight, selection colouring

    void EnableGridLines( bool enable = TRUE );
    bool GridLinesEnabled() const { return GetSheetRefData()->m_gridLinesEnabled; }
    
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
    // Row/Col label size

    // Get the fixed initial size of the width of row labels or height of col labels
    int GetDefaultRowLabelWidth()  const { return WXSHEET_DEFAULT_ROW_LABEL_WIDTH; }
    int GetDefaultColLabelHeight() const { return WXSHEET_DEFAULT_COL_LABEL_HEIGHT; }

    // Get/Set the row/col label widths, 
    //   if zero_not_shown and row/col & corner not shown return 0
    int  GetRowLabelWidth(bool zero_not_shown=TRUE)  const;
    int  GetColLabelHeight(bool zero_not_shown=TRUE) const;
    void SetRowLabelWidth( int width );
    void SetColLabelHeight( int height );

    // ------------------------------------------------------------------------
    // Span, grid cells can span across multiple cells, hiding cells below
    //
    // Normally cells are of size 1x1, but they can be larger. 
    // The other cells can still have values or attributes, but they
    // will not be used since GetCellOwner is used for most coord operations
    // so that the underlying cells are ignored.
    // The span for the owner cell is 1x1 or larger, the span for other
    // cells contained within the spanned block have a cell span of <= 0, such 
    // that coords + GetCellSpan() = the owner cell

    // Are there any spanned cells at all?
    bool HasSpannedCells() const;
    
    // if cell is part of a spanning cell, return owner's coords else input coords
    wxSheetCoords GetCellOwner( const wxSheetCoords& coords ) const;    
    // Get a block of the cell, unless a spanned cell it's of size 1,1
    //   the top left of block is the owner cell of coords
    wxSheetBlock GetCellBlock( const wxSheetCoords& coords ) const;
    // Get the span of a cell, see above
    wxSheetCoords GetCellSpan( const wxSheetCoords& coords ) const;
    // Set the span of a cell, must be 1x1 or greater, the whole block must be
    // contained within the grid cells and the block must not intersect more than
    // one other spanned cell block. If it does intersect a preexisting spanned
    // cell block then the LeftTop corners must match up.
    // To remove a spanned cell set it to a cell of size 1x1
    void SetCellSpan( const wxSheetBlock& block );
    void SetCellSpan( const wxSheetCoords& coords, const wxSheetCoords& numRowsCols )
            { SetCellSpan(wxSheetBlock(coords, numRowsCols.m_row, numRowsCols.m_col)); }

    // Get a pointer to the spanned blocks.
    //   DO NOT adjust unless you understand cell spanning.
    const wxSheetSelection* GetSpannedBlocks() const { return GetSheetRefData()->m_spannedCells; }
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
    //   see wxGridCellSheetCoords = (0,0) for example
    //   wxRowLabelSheetCoords, wxColLabelSheetCoords, wxCornerLabelSheetCoords
    
    // To completely override this you may provide alternate 
    // CanHaveAttributes, GetOrCreateAttr, GetAttr, and SetAttr functions.
    
    // Does the table have an attribute provider to store new attributes
    //   the default attributes always work though
    virtual bool CanHaveAttributes() const;

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
    void SetGridRowAttr(int row, const wxSheetCellAttr& attr)      { SetAttr(wxSheetCoords(row,  0), attr, wxSHEET_AttrRow); }
    void SetGridColAttr(int col, const wxSheetCellAttr& attr)      { SetAttr(wxSheetCoords(0,  col), attr, wxSHEET_AttrCol); }

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
    void SetCornerLabelAttr(const wxSheetCellAttr& attr)       { SetAttr(wxSheetCoords(-1,  -1), attr, wxSHEET_AttrCell); }
   
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
    bool GetAttrReadOnly( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    wxSheetCellRenderer GetAttrRenderer( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    wxSheetCellEditor   GetAttrEditor( const wxSheetCoords& coords, wxSheetAttr_Type type = wxSHEET_AttrAny ) const;
    
    // Set attributes for a particular cell/row/col, relies on GetOrCreateAttr()
    //  so it creates and adds the attr to the attr provider if there wasn't one
    //  after setting the particular value.
    //  The type may be only be wxSHEET_AttrDefault/Cell/Row/Col 
    void SetAttrBackgroundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrForegoundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrFont( const wxSheetCoords& coords, const wxFont& font, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrAlignment( const wxSheetCoords& coords, int align, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOrientation( const wxSheetCoords& coords, int orientation, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrLevel( const wxSheetCoords& coords, int level, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOverflow( const wxSheetCoords& coords, bool allow, wxSheetAttr_Type type = wxSHEET_AttrCell );
    void SetAttrOverflowMarker( const wxSheetCoords& coords, bool draw_marker, wxSheetAttr_Type type = wxSHEET_AttrCell );
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
    virtual bool IsCellEmpty( const wxSheetCoords& coords );
    
    wxString GetRowLabelValue( int row ) { return GetCellValue(wxSheetCoords(row, -1)); }
    wxString GetColLabelValue( int col ) { return GetCellValue(wxSheetCoords(-1, col)); }   
    void     SetRowLabelValue( int row, const wxString& value ) { SetCellValue(wxSheetCoords(row, -1), value); }
    void     SetColLabelValue( int col, const wxString& value ) { SetCellValue(wxSheetCoords(-1, col), value); }

    wxString GetCornerLabelValue() { return GetCellValue(wxSheetCoords(-1, -1)); }
    void     SetCornerLabelValue(const wxString& value) { SetCellValue(wxSheetCoords(-1, -1), value); }
    
    // Since there is only be one corner label value it is provided in the sheet
    //  Use these functions to directly manipulate it, the "normal" method
    //  of get/setting the corner label value goes through Get/SetCellValue
    //  so that the subclassed sheet or table may provide a value on the fly.
    wxString DoGetCornerLabelValue() const { return GetSheetRefData()->m_cornerLabelValue; }
    void DoSetCornerLabelValue(const wxString& value) { GetSheetRefData()->m_cornerLabelValue = value; }
    
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
    bool IsCellVisible( const wxSheetCoords& coords, bool wholeCellVisible = TRUE ) const;
    bool IsRowVisible( int row, bool wholeRowVisible = TRUE ) const;
    bool IsColVisible( int col, bool wholeColVisible = TRUE ) const;
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
    bool MoveCursorUpPage( bool expandSelection )   { return DoMoveCursorUpDownPage(TRUE, expandSelection); }
    bool MoveCursorDownPage( bool expandSelection ) { return DoMoveCursorUpDownPage(FALSE, expandSelection); }

    bool DoMoveCursor( const wxSheetCoords& relCoords, bool expandSelection ); 
    bool DoMoveCursorBlock( const wxSheetCoords& relDir, bool expandSelection );
    bool DoMoveCursorUpDownPage( bool page_up, bool expandSelection );
    
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
    virtual bool HasSelection(bool selecting = TRUE) const;
    // Are these coords within either the selecting block or selection
    virtual bool IsCellSelected( const wxSheetCoords& coords ) const;
    bool IsCellSelected( int row, int col ) const { return IsCellSelected(wxSheetCoords(row, col)); }
    virtual bool IsRowSelected( int row ) const;
    virtual bool IsColSelected( int col ) const;
    // Are we currently in the middle of a selection
    bool IsSelecting() const { return !GetSelectingBlock().IsEmpty(); }

    void SetSelectionMode(wxSheetSelectionMode_Type selmode);
    wxSheetSelectionMode_Type GetSelectionMode() const { return GetSheetRefData()->m_selectionMode; }

    virtual bool SelectRow( int row, bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool SelectRows( int rowTop, int rowBottom, bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool SelectCol( int col, bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool SelectCols( int colLeft, int colRight, bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool SelectCell( const wxSheetCoords& coords, bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool SelectBlock( const wxSheetBlock& block, bool addToSelected = FALSE, 
                              bool sendEvt = FALSE );
    // selects everything to numRows, numCols
    virtual bool SelectAll(bool sendEvt = FALSE);

    virtual bool DeselectRow( int row, bool sendEvt = FALSE );
    virtual bool DeselectRows( int rowTop, int rowBottom, bool sendEvt = FALSE );
    virtual bool DeselectCol( int col, bool sendEvt = FALSE );
    virtual bool DeselectCols( int colLeft, int colRight, bool sendEvt = FALSE );
    virtual bool DeselectCell( const wxSheetCoords& coords, bool sendEvt = FALSE );
    virtual bool DeselectBlock( const wxSheetBlock& block, bool sendEvt = FALSE );
    // clears selection, single deselect event numRows, numCols
    virtual bool ClearSelection(bool send_event = FALSE);

    // toggle the selection of a single cell, row, or col 
    // addToSelected applies to a selection only, ignored if a deselection
    virtual bool ToggleCellSelection( const wxSheetCoords& coords, 
                            bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool ToggleRowSelection( int row, 
                            bool addToSelected = FALSE, bool sendEvt = FALSE );
    virtual bool ToggleColSelection( int col, 
                            bool addToSelected = FALSE, bool sendEvt = FALSE );
    
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
    bool CopyCurrentSelectionToClipboard(bool copy_cursor = TRUE,
                                         const wxChar& colSep = wxT('\t'));
    // Copy the current selection to an internal copied selection mechanism 
    //  storing both the positions and values of the selected cells, if no 
    //  selection and copy_cursor then just copy the cursor value
    bool CopyCurrentSelectionInternal(bool copy_cursor = TRUE);
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
    // EndBatch's refresh = FALSE will not refresh when batchCount is 0
    void BeginBatch() { m_batchCount++; }
    void EndBatch(bool refresh=TRUE);
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
    void RefreshCell(const wxSheetCoords& coords, bool single_cell = TRUE);
    // Refresh a single row, uses -1 notation, 
    // does nothing if row !visible, or GetBatchCount != 0
    void RefreshRow(int row);
    // Refresh a single col, uses -1 notation, 
    // does nothing if col !visible, or GetBatchCount != 0
    void RefreshCol(int col);
    // Refresh is called using a rect surrounding the block, clipped to visible
    // does nothing if block IsEmpty, !visible, or GetBatchCount != 0
    void RefreshGridCellBlock( const wxSheetBlock& block );
    // After SetAttr call this can appropriately refresh the wxSheet areas
    void RefreshAttrChange(const wxSheetCoords& coords, wxSheetAttr_Type type);

    // drawing implementation

    // Refresh an area of the window that calculates the smaller rects for
    //  each individual window (row/col/corner...) and calls Refresh(subRect)
    virtual void Refresh(bool eraseb = TRUE, const wxRect* rect = NULL);
    
    // These directly call wxWindow::Refresh for the appropriate windows
    //   The input rect doesn't have to be clipped to the visible window since
    //   this function takes care of that, but it should be in client coords. 
    void RefreshGridWindow(bool eraseb = TRUE, const wxRect* rect = NULL);
    void RefreshRowLabelWindow(bool eraseb = TRUE, const wxRect* rect = NULL);
    void RefreshColLabelWindow(bool eraseb = TRUE, const wxRect* rect = NULL);
    void RefreshCornerLabelWindow(bool eraseb = TRUE, const wxRect* rect = NULL);
    
    // These functions are called by the OnPaint handler of these windows
    //   use these to add "extra touches" before or after redrawing
    virtual void OnGridWindowPaint( wxDC& dc, const wxRegion& reg );
    virtual void OnRowLabelWindowPaint( wxDC& dc, const wxRegion& reg );
    virtual void OnColLabelWindowPaint( wxDC& dc, const wxRegion& reg );
    virtual void OnCornerLabelWindowPaint( wxDC& dc, const wxRegion& reg );
    
    // draws a bunch of blocks of grid cells
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

    // Draw the splitter button rectangles 
    virtual void DrawSplitterButton(wxDC &dc, const wxRect& rect);

    // Calculate the Row/ColLabels and Cells exposed for the wxRegion
    //   returns false if none, used for redrawing windows
    bool CalcRowLabelsExposed( const wxRegion& reg, wxArrayInt& rows ) const;
    bool CalcColLabelsExposed( const wxRegion& reg, wxArrayInt& cols ) const;
    bool CalcCellsExposed( const wxRegion& reg, wxSheetSelection& blockSel ) const;

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
    // Get the size of the lines drawn horizontally, returs TRUE if size > 0
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
    wxSheetCoords XYToGridCell( int x, int y, bool clipToMinMax = FALSE ) const;
    int YToGridRow( int y, bool clipToMinMax = FALSE ) const;
    int XToGridCol( int x, bool clipToMinMax = FALSE ) const;

    // return the row/col number that the y coord is near the edge of, or
    // -1 if not near an edge, uses WXSHEET_LABEL_EDGE_ZONE
    // Use ContainsGridXXX to verify validity, -1 doesn't mean label
    int YToEdgeOfGridRow( int y ) const;
    int XToEdgeOfGridCol( int x ) const;

    // Get a rect bounding the cell, handles spanning cells and the label 
    //  windows using the -1 notation, getDeviceRect calls CalcScrolledRect
    // Gambit modification: make this virtual 
    // (overriding allows a merged labels hack)
    virtual wxRect CellToRect( const wxSheetCoords& coords, bool getDeviceRect = FALSE ) const;

    // Expand the block by unioning with intersecting spanned cells
    wxSheetBlock ExpandSpannedBlock( const wxSheetBlock& block ) const;
    
    // Convert the block of cells into a wxRect in device coords. 
    wxRect BlockToDeviceRect( const wxSheetBlock& block ) const; 
    
    // get a block containing all the currently (partially/fully) visible cells
    wxSheetBlock GetVisibleGridCellsBlock(bool wholeCellVisible = FALSE) const;

    // Align the size of an object inside the rect using wxALIGN enums
    wxPoint AlignInRect( int align, const wxRect& rect, const wxSize& size ) const;
        
    // ------------------------------------------------------------------------
    // Scrolling for the window, everything is done with pixels
    //   there is no need for scroll units and they only cause sizing problems

    // Get the scrolled origin of the grid in pixels
    const wxPoint& GetGridOrigin() const { return m_gridOrigin; }
    // Set the absolute scrolled origin of the grid window in pixels 
    //  this checks validity and ensures proper positioning. 
    //  Use x or y = -1 to not change the origin in the x or y direction
    //  Unless setting from a scrollbar event use adjustScrollBars=TRUE
    virtual void SetGridOrigin(int x, int y, bool adjustScrollBars = TRUE, bool sendEvt=FALSE);
    void SetGridOrigin(const wxPoint& pt, bool adjustScrollBars = TRUE, bool sendEvt=FALSE)
        { SetGridOrigin(pt.x, pt.y, adjustScrollBars, sendEvt); }
        
    // Get the virtual size of the grid in pixels, includes extra width/height
    wxSize GetGridVirtualSize(bool add_extra=TRUE) const;
    
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
    virtual void AdjustScrollbars(bool calc_win_sizes = TRUE);

    enum SB_Mode
    {
        SB_AS_NEEDED    = 0x0,  // Show the scrollbars as needed
        SB_HORIZ_NEVER  = 0x1,  // Never show horiz scrollbar, even if needed  
        SB_VERT_NEVER   = 0x2,  // Never show vert scrollbar, even if needed  
        SB_NEVER        = SB_HORIZ_NEVER | SB_VERT_NEVER,  
        SB_HORIZ_ALWAYS = 0x4,  // Always show horiz scrollbar
        SB_VERT_ALWAYS  = 0x8,  // Always show vert scrollbar
        SB_ALWAYS = SB_HORIZ_ALWAYS | SB_VERT_ALWAYS, // Always show scrollbars
        
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
    wxSheetGridWindow*        GetGridWindow()        const { return m_gridWin; }
    wxSheetRowLabelWindow*    GetRowLabelWindow()    const { return m_rowLabelWin; }
    wxSheetColLabelWindow*    GetColLabelWindow()    const { return m_colLabelWin; }
    wxSheetCornerLabelWindow* GetCornerLabelWindow() const { return m_cornerLabelWin; }
    // Get the window with these coords, uses -1 notation
    wxWindow* GetWindowForCoords( const wxSheetCoords& coords ) const;

    // ------ event handlers
    void ProcessRowLabelMouseEvent( wxMouseEvent& event );
    void ProcessColLabelMouseEvent( wxMouseEvent& event );
    void ProcessCornerLabelMouseEvent( wxMouseEvent& event );
    void ProcessGridCellMouseEvent( wxMouseEvent& event );
    bool ProcessTableMessage( wxSheetTableMessage& msg);

    void OnScroll( wxScrollEvent& event );
    void OnMouse( wxMouseEvent& event );
    
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

    virtual void CalcWindowSizes(bool adjustScrollBars = TRUE);
    virtual void Fit() { AutoSize(); } // overridden wxWindow methods

    // Get the ref counted data the sheet uses, *please* try to not access this
    //  directly if a functions exists to do it for you.
    wxSheetRefData* GetSheetRefData() const { return (wxSheetRefData*)GetRefData(); }
    
    // Create and send wxSheetXXXEvent depending on type and fill extra data
    //   from a wxKeyEvent or wxMouseEvent (if NULL all keydown are set FALSE)
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

    // Set the window that has capture, releases the previous one if necessary
    // always use this, set with NULL to release mouse
    void SetCaptureWindow( wxWindow *win );
    wxWindow *GetCaptureWindow() const { return m_winCapture; }
    wxWindow *m_winCapture;   // the window that captured the mouse (don't use!)
    
protected:
    virtual wxSize DoGetBestSize() const;

    virtual void DoSetSize(int x, int y, int width, int height,
                           int sizeFlags = wxSIZE_AUTO);

    bool m_created;

    wxSheetGridWindow         *m_gridWin;
    wxSheetRowLabelWindow     *m_rowLabelWin;
    wxSheetColLabelWindow     *m_colLabelWin;
    wxSheetCornerLabelWindow  *m_cornerLabelWin;
    wxScrollBar               *m_horizScrollBar;
    wxScrollBar               *m_vertScrollBar;

    wxPoint m_gridOrigin;     // origin of the gridWin in pixels
    int     m_scrollBarMode;

    bool    m_keySelecting;

    
    void InitRowHeights(); // init the m_rowHeights/Bottoms arrays with def values
    void InitColWidths();  // init the m_colWidths/Rights arrays

    // common part of AutoSizeColumn/Row() and GetBestSize()
    int SetOrCalcColumnSizes(bool calcOnly, bool setAsMin = TRUE);
    int SetOrCalcRowSizes(bool calcOnly, bool setAsMin = TRUE);
    // common part of AutoSizeColumn/Row()
    void AutoSizeColOrRow(int n, bool setAsMin, bool column /* or row? */);

    // cell attribute cache 
    wxSheetCellAttr m_cacheAttr;
    wxSheetCoords   m_cacheAttrCoords;
    int             m_cacheAttrType;

    // invalidates the attribute cache
    void ClearAttrCache();
    // adds an attribute to cache
    void CacheAttr(const wxSheetCoords& coords, const wxSheetCellAttr &attr, 
                   wxSheetAttr_Type type ) const;
    // looks for an attr in cache, returns TRUE if found
    bool LookupAttr(const wxSheetCoords& coords, wxSheetAttr_Type type, wxSheetCellAttr &attr) const;

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
    wxRect m_vertSplitRect;
    wxRect m_horizSplitRect;

    bool Redimension( wxSheetTableMessage& msg );

    void OnPaint( wxPaintEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnKeyDown( wxKeyEvent& event );
    void OnKeyUp( wxKeyEvent& event );
    void OnChar( wxKeyEvent& event );   // let wxWindows translate numpad keys
    void OnEraseBackground( wxEraseEvent& ) { /* do nothing */ }

    // ------ functions to get/send data (see also public functions)
    bool GetModelValues();
    bool SetModelValues();
    
private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSheet)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheet)
};

// ----------------------------------------------------------------------------
// wxSheetEvent
// ----------------------------------------------------------------------------
class wxSheetEvent : public wxNotifyEvent
{
public:
    wxSheetEvent(int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                 const wxSheetCoords &coords = wxNullSheetCoords, 
                 const wxPoint &pos = wxPoint(-1, -1), bool sel = TRUE);

    wxSheetEvent(const wxSheetEvent& event) : wxNotifyEvent(event), 
                     m_coords(event.m_coords), m_pos(event.m_pos),
                     m_selecting(event.m_selecting), 
                     m_control(event.m_control), m_shift(event.m_shift),
                     m_alt(event.m_alt), m_meta(event.m_meta) { }
                     
    const wxSheetCoords& GetCoords() const { return m_coords; }
    int GetRow() const                     { return m_coords.GetRow(); }
    int GetCol() const                     { return m_coords.GetCol(); }
    
    const wxPoint& GetPosition() const { return m_pos; }
    
    bool Selecting() const   { return m_selecting; }
    
    bool ControlDown() const { return m_control; }
    bool ShiftDown()   const { return m_shift; }
    bool AltDown()     const { return m_alt; }
    bool MetaDown()    const { return m_meta; }
    
    // setup the Ctrl/Shift/Alt/Meta keysDown from a wxKeyEvent or wxMouseEvent
    // Also mouse position, but the GetEventObject must be of type wxSheet
    void SetKeysDownMousePos(wxEvent *mouseOrKeyEvent);

    // implementation
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
class wxSheetRangeSelectEvent : public wxSheetEvent
{
public:
    wxSheetRangeSelectEvent( int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                             const wxSheetBlock& block=wxNullSheetBlock, 
                             bool sel=FALSE, bool add_to_sel=FALSE );

    wxSheetRangeSelectEvent(const wxSheetRangeSelectEvent& event) 
        : wxSheetEvent(event), m_block(event.m_block), m_add(event.m_add) { }
        

    const wxSheetBlock& GetBlock() const       { return m_block; }
    wxSheetCoords GetTopLeftCoords() const     { return m_block.GetLeftTop(); }
    wxSheetCoords GetBottomRightCoords() const { return m_block.GetRightBottom(); }
    int GetTopRow() const                      { return m_block.GetTop(); }
    int GetBottomRow() const                   { return m_block.GetBottom(); }
    int GetLeftCol() const                     { return m_block.GetLeft(); }
    int GetRightCol() const                    { return m_block.GetRight(); }

    bool GetAddToSelection() const             { return m_add; }
    
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
class wxSheetEditorCreatedEvent : public wxCommandEvent 
{
public:
    wxSheetEditorCreatedEvent( int id=0, wxEventType type=wxEVT_NULL, wxObject* obj=NULL,
                               const wxSheetCoords& coords=wxNullSheetCoords, 
                               wxWindow* ctrl=NULL );

    wxSheetEditorCreatedEvent(const wxSheetEditorCreatedEvent& evt) 
        : wxCommandEvent(evt), m_coords(evt.m_coords), m_ctrl(evt.m_ctrl) { }


    const wxSheetCoords& GetCoords() const { return m_coords; }
    int GetRow() const                     { return m_coords.GetRow(); }
    int GetCol() const                     { return m_coords.GetCol(); }
    wxWindow* GetControl() const           { return m_ctrl; }
    
    void SetCoords(const wxSheetCoords& coords) { m_coords = coords; }
    void SetRow(int row)                        { m_coords.SetRow(row); }
    void SetCol(int col)                        { m_coords.SetCol(col); }
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
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_VIEW_CHANGED, 1592)

    // The grid cursor is about to be in a new cell, veto or !Skip() to block 
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SELECTING_CELL, 1592)
    // The grid cursor is in a new cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SELECTED_CELL, 1592)

    // left down click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_LEFT_DOWN, 1580)
    // right down click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_RIGHT_DOWN, 1581)
    // left up click in a grid cell, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_LEFT_UP, 1580)
    // right up click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_RIGHT_UP, 1581)
    // left double click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_LEFT_DCLICK, 1582)
    // right double click in a grid cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_RIGHT_DCLICK, 1583)

    // left down click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_LEFT_DOWN, 1584)
    // right down click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_RIGHT_DOWN, 1585)
    // left up click in a label cell, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_LEFT_UP, 1584)
    // right up click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_RIGHT_UP, 1585)
    // left double click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_LEFT_DCLICK, 1586)
    // right double click in a label cell
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_LABEL_RIGHT_DCLICK, 1587)
    
    // A row has been resized, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_ROW_SIZE, 1588)
    // A col has been resized, sent after default processing
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_COL_SIZE, 1589)
    
    // A block of cells is about to be (de)selected (veto to stop)
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_RANGE_SELECTING, 1590)
    // A block of cells has been (de)selected
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_RANGE_SELECTED, 1590)
    
    // The value of a cell is about to be changed (veto to stop)
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_VALUE_CHANGING, 1591)
    // The value of a cell has been changed (veto to put old val back)
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_CELL_VALUE_CHANGED, 1591)
    
    // From EnableCellEditControl, the control is about to enabled (can veto)
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_EDITOR_ENABLED, 1593)
    // From DisableCellEditControl, the control is about to disabled (can veto)
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_EDITOR_DISABLED, 1594)
    // From EnableCellEditControl, the edit control has been created
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_EDITOR_CREATED, 1595)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxSheetEventFunction)(wxSheetEvent&);
typedef void (wxEvtHandler::*wxSheetRangeSelectEventFunction)(wxSheetRangeSelectEvent&);
typedef void (wxEvtHandler::*wxSheetEditorCreatedEventFunction)(wxSheetEditorCreatedEvent&);

#define EVT_SHEET_VIEW_CHANGED(id, fn)        DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_VIEW_CHANGED,        id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_SELECTING_CELL(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SELECTING_CELL,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_SELECTED_CELL(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SELECTED_CELL,       id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_DOWN(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_DOWN,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_DOWN(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_DOWN,     id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_UP(id, fn)        DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_UP,        id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_UP(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_UP,       id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_LEFT_DCLICK(id, fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_LEFT_DCLICK,    id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_RIGHT_DCLICK(id, fn)   DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_RIGHT_DCLICK,   id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_DOWN(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_DOWN,     id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_DOWN(id, fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_DOWN,    id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_UP(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_UP,       id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_UP(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_UP,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_LEFT_DCLICK(id, fn)   DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_LEFT_DCLICK,   id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_LABEL_RIGHT_DCLICK(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_LABEL_RIGHT_DCLICK,  id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_ROW_SIZE(id, fn)            DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_ROW_SIZE,            id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_COL_SIZE(id, fn)            DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_COL_SIZE,            id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_RANGE_SELECTING(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_RANGE_SELECTING,     id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetRangeSelectEventFunction, &fn), NULL ),
#define EVT_SHEET_RANGE_SELECTED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_RANGE_SELECTED,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetRangeSelectEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_VALUE_CHANGING(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_VALUE_CHANGING, id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_CELL_VALUE_CHANGED(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CELL_VALUE_CHANGED,  id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_ENABLED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_ENABLED,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_DISABLED(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_DISABLED,     id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, &fn), NULL ),
#define EVT_SHEET_EDITOR_CREATED(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_EDITOR_CREATED,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEditorCreatedEventFunction, &fn), NULL ),

#if 0  // TODO: implement these ?  others ?

extern const int wxEVT_SHEET_CREATE_CELL;
extern const int wxEVT_SHEET_CHANGE_LABELS;
extern const int wxEVT_SHEET_CHANGE_SEL_LABEL;

#define EVT_SHEET_CREATE_CELL(fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CREATE_CELL,      -1, -1, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),
#define EVT_SHEET_CHANGE_LABELS(fn)    DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CHANGE_LABELS,    -1, -1, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),
#define EVT_SHEET_CHANGE_SEL_LABEL(fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_CHANGE_SEL_LABEL, -1, -1, (wxObjectEventFunction) (wxEventFunction) (wxSheetEventFunction) &fn, NULL ),

#endif // 0

#endif  // __WX_SHEET_H__
