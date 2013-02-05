///////////////////////////////////////////////////////////////////////////////
// Name:        sheetdef.h
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETDEF_H__
#define __WX_SHEETDEF_H__

#include "wx/defs.h"
#include "wx/datetime.h"
#include "wx/sheet/wx24defs.h"   // wx2.4 backwards compatibility
#include "wx/dynarray.h"
#if wxMINOR_VERSION > 4
    #include "wx/arrstr.h"
#endif
#include "wx/sheet/pairarr.h"

// ----------------------------------------------------------------------------
// DLLIMPEXP macros
// ----------------------------------------------------------------------------

// These are our DLL macros (see the contrib libs like wxPlot)
/*
#ifdef WXMAKINGDLL_SHEET
    #define WXDLLIMPEXP_SHEET WXEXPORT
    #define WXDLLIMPEXP_DATA_SHEET(type) WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_SHEET WXIMPORT
    #define WXDLLIMPEXP_DATA_SHEET(type) WXIMPORT type
#else // not making nor using DLL
*/
    #define WXDLLIMPEXP_SHEET
    #define WXDLLIMPEXP_DATA_SHEET(type) type
/*
#endif
*/

// ----------------------------------------------------------------------------
// Two simple macros that do wxCHECK_RET/MSG for the UpdatePos functions
//   giving reasonable messages to help debugging
// ----------------------------------------------------------------------------

#define wxSHEET_CHECKUPDATE_RET(pos, count, origcount) \
    wxCHECK_RET((count == 0) || ((int(count) > 0) && (int(pos) <= int(origcount))) || \
                                ((int(count) < 0) && (int(pos) - int(count) <= int(origcount))), \
          wxString::Format(wxT("Updating past end of array, pos %d count %d, orig count %d"), pos, count, origcount));

#define wxSHEET_CHECKUPDATE_MSG(pos, count, origcount, ret) \
    wxCHECK_MSG((count == 0) || ((int(count) > 0) && (int(pos) <= int(origcount))) || \
                                ((int(count) < 0) && (int(pos) - int(count) <= int(origcount))), ret, \
          wxString::Format(wxT("Updating past end of array, pos %d count %d, orig count %d"), pos, count, origcount));

// ----------------------------------------------------------------------------
// Complete list of classes implemented by wxSheet 
// ----------------------------------------------------------------------------

// Windows
class WXDLLIMPEXP_SHEET wxSheet;               // the spreadsheet widget
class WXDLLIMPEXP_SHEET wxSheetChildWindow;    // child windows of the sheet
class WXDLLIMPEXP_SHEET wxSheetRefData;        // internal use sheet data
// Events
class WXDLLIMPEXP_SHEET wxSheetEvent;          // events returned from the sheet
class WXDLLIMPEXP_SHEET wxSheetRangeSelectEvent;  // cells (de)selected
class WXDLLIMPEXP_SHEET wxSheetEditorCreatedEvent; // editor created 
class WXDLLIMPEXP_SHEET wxSheetSplitterEvent;  // events sent from the sheet splitter    
// Attributes
class WXDLLIMPEXP_SHEET wxSheetCellAttr;       // attribute for cell/row/col
class WXDLLIMPEXP_SHEET wxSheetCellAttrRefData;// refdata for attribute
class WXDLLIMPEXP_SHEET wxArraySheetCellAttr;  //  array of cell attrs
class WXDLLIMPEXP_SHEET wxPairArrayIntSheetCellAttr; // int, attr pair array
class WXDLLIMPEXP_SHEET wxPairArraySheetCoordsCellAttr; // coords, attr pair array
class WXDLLIMPEXP_SHEET wxSheetCellAttrProvider; // provider of attr for row/col/cell
class WXDLLIMPEXP_SHEET wxSheetTypeRegistry;     // internal use
// String containers
//class WXDLLIMPEXP_SHEET wxSheetStringArray;        // wxArray of wxArrayString 
class WXDLLIMPEXP_SHEET wxPairArrayIntSheetString;   // pair (int, wxString)
class WXDLLIMPEXP_SHEET wxArrayPairArrayIntSheetString; // array of (int, str) pair
class WXDLLIMPEXP_SHEET wxSheetValueProviderBase;         // base class value provider
class WXDLLIMPEXP_SHEET wxSheetValueProviderString;       // a wxArrayString value provider
class WXDLLIMPEXP_SHEET wxSheetValueProviderSparseString; // a string pair array value provider

// Coords/Blocks/Selection/Int arrays/Edge arrays
class WXDLLIMPEXP_SHEET wxSheetCoords;         // cell coordinates
class WXDLLIMPEXP_SHEET wxSheetBlock;          // rectangular block of cells
class WXDLLIMPEXP_SHEET wxSheetSelection;      // cell selection container
class WXDLLIMPEXP_SHEET wxSheetSelectionIterator; // selection iterator    
class WXDLLIMPEXP_SHEET wxSheetArrayEdge;      // row/col edge container
class WXDLLIMPEXP_SHEET wxPairArrayIntInt;     // pair of (int, int)
// "Widgets"
class WXDLLIMPEXP_SHEET wxSheetCellRenderer;   // drawing renderer for cell
class WXDLLIMPEXP_SHEET wxSheetCellEditor;     // cell editor
// Tables
class WXDLLIMPEXP_SHEET wxSheetTable;      // cell value/attr provider

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Default parameters for wxSheet
#ifdef __WXMSW__
    #define WXSHEET_DEFAULT_ROW_HEIGHT     25
#else
    #define WXSHEET_DEFAULT_ROW_HEIGHT     30
#endif  // __WXMSW__
#define WXSHEET_DEFAULT_COL_WIDTH          80
#define WXSHEET_DEFAULT_COL_LABEL_HEIGHT   32
#define WXSHEET_DEFAULT_ROW_LABEL_WIDTH    82
#define WXSHEET_LABEL_EDGE_ZONE            2
#define WXSHEET_MIN_ROW_HEIGHT             15
#define WXSHEET_MIN_COL_WIDTH              15

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
// wxSheet cell drawing scheme, also applies to label cells
// ----------------------------------------------------------------------------
//               
// -------------- <- Top line is bottom of cell above, none if row = 0, GetRowBottom(row-1)
// |            | <- This is GetRowTop(row)
// | (row, col) |    
// |            |
// -------------- <- this pixel is GetRowBottom(row), part of this cell
// ^^           ^             
// ||           |- Right side is GetColRight(col), part of this cell
// ||    
// ||- This is GetColLeft(col)   
// |-- Left side is cell to left, or none if col = 0, GetColRight(col-1)
//
// The sheet consists of 4 individual windows each is accessed with coords
// using the semantics below.
//
// ############################################################################
// # corner label # col label | col label |       |                           #
// # (-1, -1)     # (-1, 0)   | (-1, 1)   |  ...  | (-1, GetNumberCols() - 1) #
// ############################################################################
// # row label    # grid cell | grid cell |       |                           #
// # (0, -1)      # (0, 0)    | (0, 1)    |  ...  | ( 0, GetNumberCols() - 1) #
// #--------------#-----------------------------------------------------------#
// # row label    # grid cell | grid cell |       |                           #
// # (1, -1)      # (1, 0)    | (1, 1)    |  ...  | ( 1, GetNumberCols() - 1) #
// #--------------#-----------------------------------------------------------#
// #              #           |           |       |                           #
// #     ...      #   ...     |   ...     |  ...  |           ...             #
// #--------------#-----------------------------------------------------------#
// #              #           |           |       |   (GetNumberRows() - 1,   #
// #              #      GetNumberRows() - 1      |    GetNumberCols() - 1)   #
// ############################################################################

// ----------------------------------------------------------------------------
// wxSheetCell_Type : enum of the different window cell types
// ----------------------------------------------------------------------------

enum wxSheetCell_Type
{
    wxSHEET_CELL_UNKNOWN = 0,   // An invalid cell coords
    wxSHEET_CELL_GRID,          // A grid cell      row >= 0  && col >= 0
    wxSHEET_CELL_ROWLABEL,      // A row label cell row >= 0  && col == -1 
    wxSHEET_CELL_COLLABEL,      // A col label cell row == -1 && col >= 0
    wxSHEET_CELL_CORNERLABEL    // The corner label row == -1 && col == -1
};    

// ----------------------------------------------------------------------------
// wxSheetSelectionMode_Type : how selections are made by the mouse and cursor
// ----------------------------------------------------------------------------

enum wxSheetSelectionMode_Type 
{
    wxSHEET_SelectNone  = 0x0001, // don't allow selections by mouse or keyboard
                                  // direct calls to the selections work however
    wxSHEET_SelectCells = 0x0002, // single cells, blocks, rows, and cols
    wxSHEET_SelectRows  = 0x0004, // only rows can be selected
    wxSHEET_SelectCols  = 0x0008 // only cols can be selected
};

// ----------------------------------------------------------------------------
// wxSheetMouseDrag_Type : can the mouse or keyboard be used to resize cells 
// ----------------------------------------------------------------------------

enum wxSheetDragCellSize_Type
{
    wxSHEET_DragLabelRowHeight = 0x0010, // Mouse can resize row heights on label 
    wxSHEET_DragLabelColWidth  = 0x0020, // Mouse can resize col widths on label 
    wxSHEET_DragLabelCellSize  = (wxSHEET_DragLabelRowHeight|wxSHEET_DragLabelColWidth), 

    wxSHEET_DragLabelRowWidth  = 0x0040, // Mouse can resize row label width
    wxSHEET_DragLabelColHeight = 0x0080, // Mouse can resize col label height
    wxSHEET_DragLabelSize      = (wxSHEET_DragLabelRowWidth|wxSHEET_DragLabelColHeight),

    wxSHEET_DragGridRowHeight  = 0x0100, // Mouse can resize row heights on grid
    wxSHEET_DragGridColWidth   = 0x0200, // Mouse can resize col widths on grid
    wxSHEET_DragGridCellSize   = (wxSHEET_DragGridRowHeight|wxSHEET_DragGridColWidth),

    wxSHEET_DragCellSize       = (wxSHEET_DragLabelCellSize|wxSHEET_DragGridCellSize),
    wxSHEET_DragSize           = (wxSHEET_DragCellSize|wxSHEET_DragLabelSize)
};

// ----------------------------------------------------------------------------
// wxSHEET_Update_Type : when redimensioning rows/cols what data containers 
//   should be updated as well. You probably want to do them all, unless you 
//   can ensure that no problems will occur.
// ----------------------------------------------------------------------------

enum wxSheetUpdate_Type
{
    wxSHEET_UpdateNone           = 0x0000, // update nothing
    
    wxSHEET_UpdateGridCellValues = 0x0001, // update the grid cell data container
    wxSHEET_UpdateRowLabelValues = 0x0002, // update the row label data container
    wxSHEET_UpdateColLabelValues = 0x0004, // update the col label data container
    wxSHEET_UpdateLabelValues    = wxSHEET_UpdateRowLabelValues|wxSHEET_UpdateColLabelValues, // update the label containers    
    wxSHEET_UpdateValues         = wxSHEET_UpdateGridCellValues|wxSHEET_UpdateLabelValues,
    
    wxSHEET_UpdateSpanned        = 0x0008, // update the spanned cells
    
    wxSHEET_UpdateGridCellAttrs  = 0x0010, // update grid cell attributes
    wxSHEET_UpdateRowLabelAttrs  = 0x0020, // update row label attributes
    wxSHEET_UpdateColLabelAttrs  = 0x0040, // update col label attributes
    wxSHEET_UpdateLabelAttrs     = wxSHEET_UpdateRowLabelAttrs|wxSHEET_UpdateColLabelAttrs,
    wxSHEET_UpdateAttributes     = wxSHEET_UpdateGridCellAttrs|wxSHEET_UpdateLabelAttrs, // update the attr container
    
    wxSHEET_UpdateSelection      = 0x0100, // update the selection
    
    wxSHEET_UpdateAll            = (wxSHEET_UpdateValues|wxSHEET_UpdateSpanned|wxSHEET_UpdateAttributes|wxSHEET_UpdateSelection),
    wxSHEET_UpdateType_Mask      = wxSHEET_UpdateAll
};

// ----------------------------------------------------------------------------
// wxSheetCellAttr_Type : contains all the attributes for a wxSheet cell
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
    wxSHEET_AttrAny     = wxSHEET_AttrDefault|wxSHEET_AttrCell|wxSHEET_AttrRow|wxSHEET_AttrCol,
    
    wxSHEET_AttrType_Mask = wxSHEET_AttrAny
};    

// ----------------------------------------------------------------------------
// Alignment of the text (or whatever) for a sheet cell attribute
enum wxSheetAttrAlign_Type
{
    // wxALIGN_TOP=wxALIGN_LEFT=wxALIGN_NOT=0, use these for unset
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
// Level of the attribute if there are multiple attrs per cell
//    default ordering if all have same level is cell->row->col
//    default attributes for the wxSheet are initialized to wxSHEET_AttrLevelBottom
enum wxSheetAttrLevel_Type
{
    wxSHEET_AttrLevelUnset     = 0,
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

// ----------------------------------------------------------------------------
// Should the editors always be shown  FIXME not implemented
enum wxSheetAttrShowEditor_Type
{
    wxSHEET_AttrShowEditorUnset     = 0,
    wxSHEET_AttrShowEditor          = 0x4000000,
    wxSHEET_AttrShowEditorNot       = 0x8000000,
    wxSHEET_AttrShowEditorType_Mask = (wxSHEET_AttrShowEditor|wxSHEET_AttrShowEditorNot)
};

// ----------------------------------------------------------------------------
// DECLARE_PAIREDSHEETCOORDS_DATA_ARRAYS 
//   Create a pairing of wxSheetCoords and another class that will be sorted
//   for a quicker lookup. This macro also includes additional functions
//   UpdateRows(size_t pos, int numRows), UpdateCols(size_t pos, int numCols)
//      for when for when rows/cols are inserted/deleted
// ----------------------------------------------------------------------------
#include "wx/sheet/sheetsel.h"

#define DECLARE_PAIREDSHEETCOORDS_DATA_ARRAYS(Tval, TvalArray, name, classexp) \
DECLARE_PAIRED_DATA_ARRAYS_BASE(wxSheetCoords, wxArraySheetCoords, Tval, TvalArray, name, classexp) \
public: \
    bool UpdateRows( size_t pos, int numRows ); \
    bool UpdateCols( size_t pos, int numCols ); \
};

#define DEFINE_PAIREDSHEETCOORDS_DATA_ARRAYS(Tval, name) \
DEFINE_PAIRED_DATA_ARRAYS(wxSheetCoords, Tval, name) \
bool name::UpdateRows( size_t pos_, int numRows ) \
{ \
    if (numRows == 0) return false; \
    int n, count = GetCount(), pos = pos_; \
    bool done = false, remove = numRows < 0; \
    for ( n = 0; n < count; n++ ) \
    { \
        wxSheetCoords& coords = GetItemKey(n); \
        if (coords.m_row >= pos) \
        { \
            if (remove && (coords.m_row < pos - numRows)) \
                { done = true; RemoveAt(n); count--; n--; } \
            else \
                { done = true; coords.m_row += numRows; } \
        } \
    } \
    return done; \
} \
bool name::UpdateCols( size_t pos_, int numCols ) \
{ \
    if (numCols == 0) return false; \
    int n, count = GetCount(), pos = pos_; \
    bool done = false, remove = numCols < 0; \
    for ( n = 0; n < count; n++ ) \
    { \
        wxSheetCoords& coords = GetItemKey(n); \
        if (coords.m_col >= pos) \
        { \
            if (remove && (coords.m_col < pos - numCols)) \
                { done = true; RemoveAt(n); count--; n--; } \
            else \
                { done = true; coords.m_col += numCols; } \
        } \
    } \
    return done; \
} 

// ----------------------------------------------------------------------------
// DECLARE_SHEETOBJREFDATA_COPY_CLASS for Renderers and Editors
// ----------------------------------------------------------------------------
// classes must define a "bool Copy(const classname& other)" function to copy all
// the data in the class so that Clone can create a "new" copied instance.
#define DECLARE_SHEETOBJREFDATA_COPY_CLASS(classname, basename)         \
    DECLARE_DYNAMIC_CLASS(classname)                                    \
    public:                                                             \
    virtual basename * Clone() const { classname * aclass = new classname(); aclass->Copy( * ((classname * )this)); return (basename * )aclass; } 

#endif  // __WX_SHEETDEF_H__
