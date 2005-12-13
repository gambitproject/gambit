README.txt for wxSheet - a wxGrid type spreadsheet widget

Copyright : 2004, John Labenski and others (see headers)
License   : wxWidgets license.

This library/program is free software; you can redistribute it and/or modify it 
under the terms of the wxWidgets Licence; either version 3 of the Licence, 
or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE. See the wxWidgets Licence for more details.

If you use this program/library and find any bugs or have added any features
that you feel may be generally useful, please feel free to contact me by 
e-mail at jrl1[at]lehigh[dot]edu.

===============================================================================
Summary:

    wxSheet is a spreadsheet type grid widget for the wxWidgets GUI library. 
The wxSheet code is based on the wxWidget's wxGrid class and is, for the most
part, a complete rewrite, almost every function has been tweaked. 
It is not backwards compatible, but operates and is logically similiar. 
Many of the functions have the same names and you can expect them to perform a 
similiar task, but the parameters may be different. The reason for creating 
a completely separate grid implementation is to allow far more control over the
different aspects of the operation either though subclassing or intercepting 
events. 
    You will find documentation included in the header files with the member
functions ordered by function. You can of course look under the hood, at the 
cpp files, to fully understand what each function does. 

    I welcome any changes or interesting ideas you may have for 
improvement. The current version suits my needs in terms of flexibility, but
I only use a grid widget in a limited capacity.

===============================================================================
Provides:

wxSheet - the grid window itself and the row/col/corner label windows and the
    independent scrollbars.
wxSheetSplitter - a four way splitter window for the wxSheet, it can split
    horizontally and/or vertically. Splitting works by setting 
    wxSheet::EnableSplitVertically/Horizontally which will draw a 
    little button next to the scrollbars like Excel/OOCalc for example. The
    sheet checks for the mouse dragging in the buttons and sends an event. The
    sheet DOES NOT split itself, but if the parent is a wxSheetSplitter it
    gets the event, grabs the mouse, draws the splitting line, and (un)splits
    by appropriately creating/deleting a sheet and showing/hiding the label 
    windows and scrollbars.
    
wxSheetTable - the base class for a data container to provide values, 
    attributes, and spanned cells for wxSheet cells
wxSheetStringArray = wxGridStringArray - a 2D wxString wxArray (uses grid's if
    WXUSE_GRID)
wxPairArrayIntSheetString - pair array of (int, string)
wxArrayPairArrayIntSheetString - array of pairs of (int, string)
wxPairArrayIntPairArraySheetString - pair array [int, array pair (int, string)]
wxSheetValueProviderBase - a base class for a value provider for the table
wxSheetValueProviderString - a wxSheetStringArray (eg. 2D array of wxStrings)
    data provider for the table. Good for mostly full sheets, though it is 
    optimised to only fill the arrays as necessary.
wxSheetValueProviderSparseString - a wxPairArrayIntPairArraySheetString value 
    provider class for probably less than 2/3 - 3/4 full sheets
    
wxSheetCellAttr - an attribute for the grid/row/col/corner label cells. They
    provide colours, text alignment, renders, editors... They are chained
    together so that whatever values are unset for a particular attribute are 
    retrieved from the default attribute.
wxNullSheetCellAttr - a const uncreated attribute     
wxSheetCellAttrProvider - a container for storing non default attributes for the
    grid and label cells.

wxSheetCoords - row, col coords for accessing cells
wxSheetBlock - a rectangular block of cells
wxSheetSelection - a dumb selection container that stores and array of 
    sorted and nonoverlapping wxSheetBlocks.
wxSheetSelectionIterator - an iterator for traversing the selection from top to
    bottom or in the reverse direction.

wxSheetCellEditor - a container for a variety of editors to use with the 
    wxSheet. The ref counted data actually contains the editor code so that
    they can easily passed around. Please see sheetctl.h for a list.
wxSheetCellRenderer - a container for a variety of renderers to use with the 
    wxSheet. The ref counted data actually contains the renderer code so that
    they can easily passed around. Please see sheetctl.h for a list.

===============================================================================
Additional Features:

Faster rendering, about %10-%20 faster, but your mileage may vary.
    GTK2 for example is very slow at rendering antialiased text so that the 
    speedup from the more optimized redrawing is marginalized by the time 
    actually spent drawing.
wxSheetCellAttr/Renderer/Editors are wxObject refed so it's simple to use
    No more Inc/DecRef, treat them just like wxBitmaps, wxPens, etc...
Added OnKeyDown and OnChar for editors so they can be more interactive
Text editor expands as you type by full cells like other spreadsheets
TextRenderer draws a little arrow to show that the text is clipped
Smooth scrolling for selection out of window 
Attributes for all windows, grid row/col/corner labels 
Renderers for all windows, grid row/col/corner labels 
Editors for all windows, grid row/col/corner labels or not.
    The default attributes for the labels are set readonly by default.
    Note: if you can edit the row/col/corner labels then a single click doesn't
    select the row/col/everything. You must drag it a pixel or two. This is a 
    reasonable tradeoff since otherwise for a double click to edit you select
    then deselect which looks bad. If you can't edit it, then single click sels.
Adds wxSheetStringSparseTable that is not just a huge double array.
    Uses binary search for lookup, ideal for less than half full grids.
DECLARE_PAIRED_DATA_ARRAYS used for key/value pairs that need to be sorted
    These are used for faster Attr lookup and sparse string table
Added selecting for page up/down
Grid lines drawn horizontal and/or vertical or none
Additional events RANGE_SELECTING/SELECTED so you can block it
AltDown + Arrows resizes cell like OOffice                  
Independent scrollbars, you can have none, both, as needed, or either.
Events use IDs like other wxWidget events
Left Double click begins editing as well as slow click.
wxSheetBlock(row, col, height, width) a rectangular block of cell
   immensely simplifies logic.
Moved the cell spanning (was wxGrid::SetCellSize) code out of the attributes for
   faster lookup.
Rudimentary copy and paste for strings only, not default behavior and you must
    call the functions yourself.
wxSheetSplitter window, a window that given a single child wxSheet can split
    the view into four panes.

You can have complete control over the selection mechanism, override only
    IsSelection, IsCell/Row/ColSelected, (De)SelectBlock, and ClearSelection.
You can have complete control over the attributes, override only
    CanHaveAttributes, GetOrCreateAttr, GetAttr, and SetAttr. (also works as
    the wxGrid does by passing to the table where you can override it there
    and to the attr provider)
You can have complete control over the cell values in the grid, override only
    Get/SetCellValue. (also works as the wxGrid by passing to the table where
    you can override it there as well)

-------------------------------------------------------------------------------    
Things that were "fixed" (you may disagree however)

MoveCellBlock so that it matches OpenOffice by staying on cells w/ values
Right drag in grid does nothing
Selecting w/ keyboard moves cursor cell
Numpad arrows work to move mouse using EVT_CHAR
Events use IDs 
For selection type rows/cols don't allow sel on opposite col/row labels.

-------------------------------------------------------------------------------    
The sheet consists of 4 individual windows each is accessed with coords
    using the semantics below.

Functions named Get/SetGridXXX(wxSheetCoords, ...) apply to the grid cells, 
    otherwise the function uses the -1 notation to access the label cells.
There are a slew of 
    static bool IsGrid/RowLabel/ColLabel/CornerLabelCell(wxSheetCoords) 
    functions to genericly determine what type of cell it is as well as 
    bool ContainsGrid/RowLabel/ColLabelCell(wxSheetCoords) 
    to determine if the coords are valid for the current grid size.
    While this may seem slightly more complicated, it makes the code 
    quite a bit more readable.
    
##########################################################################
# corner label # col label | col label |           |                     #
# (-1, -1)     # (-1, 0)   | (-1, 1)   |    ...    | GetNumberCols() - 1 #
##########################################################################
# row label    # grid cell | grid cell |           |                     #
# (0, -1)      # (0, 0)    | (0, 1)    |    ...    |                     #
#--------------#---------------------------------------------------------#
# row label    # grid cell | grid cell |           |                     #
# (1, -1)      # (1, 0)    | (1, 1)    |    ...    |                     #
#--------------#---------------------------------------------------------#
#              #           |           |           |                     #
#     ...      #   ...     |   ...     |    ...    |                     #
#--------------#---------------------------------------------------------#
#              #           |           |           | GetNumberRows() - 1 #
#                  GetNumberRows() - 1             | GetNumberCols() - 1 #
##########################################################################

===============================================================================
Differences to wxGrid

wxSheet::Foo( row, col ) -> Foo( const wxSheetCoords& coords )
wxSheet::GetRow/ColSize -> GetRowHeight/Width - size is ambiguous
wxSheet::Get/SetDefaultCol/RowSize -> Get/SetDefaultCol/RowHeight/Width - size is ambiguous
wxSheet::Get/SetHighlightXXX -> Get/SetCursorCellHighlightXXX it's for the cursor
wxSheet::HighlightBlock -> HighlightSelectingBlock that's what it really does
wxSheet::IsVisible -> IsCellVisible
wxSheet::Set/GetCaptureWindow added a locker for captured window, never access m_winCapture
wxSheet::GetAttr/SetAttr/GetOrCreateAttr all the helper Get/SetXXXAttr call these
    uses -1 notation for label cell
wxSheet::Get/SetCellThis/That -> Get/SetAttr/This/That these function
    can actually get specific values for specific attributes, ie. for a 
    cell/row/col attr, of course you must have set one previously. The default
    is like the wxGrid, just any type of attr value you can.
wxSheet & wxSheetCellAttr Get/SetAlignment uses single int, no point in horiz and vert
    since aligments are non intersecting enums
    DrawTextRectangle only takes one alignment 
wxSheet & wxSheetAttr::IsReadOnly -> GetReadOnly everything else is GetXXX
wxSheet Editing redone. Have m_cellEditor, m_cellEditorCoords to get later
    More wxCHECK stuff to ensure that you're not arbitrarily calling editor
    functions since it can be touchy.
wxSheet::GetCellSize -> GetCellSpan
wxSheet::GetCellSize actually gets the size of the cell as wxSize
wxSheet::GetCellSpan, GetCellBlock, GetCellOwner for cell spanning
    this is implemented in the grid using a wxSheetSelection. It is 
    considerably cleaner than the wxGrid attribute method since it only
    stores a single block for a spanned cell.
wxSheet::XToCol/YToRow(val, clipToMinMax) -> removed internalXToCol/YToRow 
Redrawing reworked, more functions for simple refreshes so that the code
    in different functions need only call a single redraw function 
wxSheet::MovePageUp/Down -> MoveCursorUp/DownPage to match the other MoveCursorXXX
wxSheet::DoMoveXXX simplier functions to implement these functions and reduce
    the amount of duplicate code.
wxSheet::IsSelected(cell) -> IsCellSelected (match other cell functions)
wxSheet::IsSelection -> HasSelection (bad grammar)
-------------------------------------------------------------------------------
wxSheetCellEditor and wxSheetCellRenderer are wxObject refed classes
    The actual implementation is in the ref data so they're passed as const &obj
wxSheetCellEditor::Create -> CreateEditor say what it does
Added wxNullSheetCellRenderer, wxNullSheetCellEditor
Added OnKeyDown and OnChar for interactive editors
Added GetValue, GetInitValue so that you can get the current or initial value
wxSheetCellEditor::PaintBackground takes same parameters as 
    wxSheetCellRenderer::Draw
-------------------------------------------------------------------------------
wxSheetCellCoords -> wxSheetCoords other name is too long, members PUBLIC!
wxSheetCoords added useful operators to make it complete
wxSheetNoCellCoords -> wxNullSheetCoords matches other wxWindows empty items
Added wxSheetBlock a rectangular selection of cells 
    blocks are similiar to wxRects and implement similiar functionality
-------------------------------------------------------------------------------
wxSheetTableMessage::GetPosition/RowsCols, not mysterious CommandInt1/2
    creation of messages are now different since com1 = pos, com2 = rows/cols
    in wxGrid the com1 and com2 took different meaning for different msgs
wxSheetTableBase::SendInsert/Append/Delete/Rows/ColsTableMsg to send appropriate
    wxSheetTableMessage since everyone had to duplicate the code used in 
    the wxSheetStringTable anyway.
wxSheetTableBase::GetFirstNonEmptyColToLeft, render helper get next col to left
    that's not empty or -1, (if you can't do this just return col-1, base does this)
wxSheetStringTable is like the wxGridStringTable
wxSheetStringSparseTable uses wxPairArrayIntPairArraySheetString which is an array of 
    strings and ints. Binary search lookup. Size of strings table is now 
    approximately (size of strings) + (1 int per string) + (1 int per row). 
-------------------------------------------------------------------------------
wxSheetEvent is parent for the other events since they're basicly the same
wxSheetEvent doesn't take a million different items in constructor
    add SetKeyDownMousePos( wxEvent* ) to get vals from mouse/key evt.
EVT_SHEET_RANGE_SELECT broke into SELECTING and SELECTED, can Veto it
EVT_SHEET_SELECT_CELL broke into SELECTING and SELECTED
EVT_SHEET_CELL_LEFT/RIGHT_CLICK -> EVT_SHEET_CELL_LEFT/RIGHT_DOWN
Added EVT_SHEET_CELL_LEFT/RIGHT_UP
EVT_SHEET_EDITOR_SHOWN/HIDDEN -> ENABLED/DISABLED since the events didn't
    match what they were named after
Events take IDs now
Removed wxSheetSizeEvent - there is no point for it since wxSheetEvent 
   contains all necessary information
-------------------------------------------------------------------------------
wxSheetCellAttr - wxObject ref counted, just like wxBitmap etc...
wxSheetNoCellAttr -> wxNullSheetCellAttr matches other wxWindows empty items
wxSheetCellAttr::IsReadOnly = HasReadWriteMode everything else is HasXXX
wxSheetCellAttr::Get/SetAlignment single alignment, no need for horiz & vert
Added Get/Set/HasOrientation for wxHORIZONTAL, wxVERTICAL
wxSheetCellAttr::GetSize removed, this is now in wxSheet and called Span
All enum/int values are stored in single int so it's smaller
-------------------------------------------------------------------------------
wxSheetCellAttrProvider::Get/SetAttr - Get/SetCellAttr and param order matches sheet
wxSheetCellAttrProvider::UpdateAttrRows/Cols -> UpdateRows/Cols
wxSheetCellAttrData & wxSheetRowOrColAttrData - use binary search 
-------------------------------------------------------------------------------
wxSheetSelection is completely different, it just selects blocks of cells
    for full rows/cols it selects to # of rows/cols as opposed to rows/cols-1. 
    You can use wxSheetSelection::SetBoundingBlock to trim it back to grid size
    if you desire.
The selection is a dumb container and is used for other purposes, cell span,
    and redrawing for example.
You can freely modify the selection! Just repaint when done.
You can completely override the selection.
wxSheetSelection type is stored in the wxSheet since the selection does
    nothing with it anyway.
Added wxSheetSelectionIterator that can iterate forwards from top left by rows 
    then cols or backwards from bottom left by cols then rows.
The selection is not preserved when switching selection type or 
    inserting/deleting rows/cols. Ideally you should send slews of confusing
    (de)selection events and so I think it's best to just clear it.
    if you click anywhere it gets cleared so the selection is not
    that precious anyway. There is no technical reason why this couldn't be 
    implemented however.
Added EVT_SHEET_RANGE_SELECTING(ED), you can block the SELECTING

===============================================================================
List of files

include/wx/sheet/
    sheet.h - the main header file for the wxSheet component
    sheetctl.h - the header for the renderers and editors controls
    sheetsel.h - the header for the wxSheetSelection, selection mechanism
                 also contains wxSheetCoords, wxSheetBlock.
    sheetspt.h - header for wxSheetSplitter
    wx24defs.h - provides backwards compatibility for code written for 
                 wxWidgets >= 2.5 to compile in wxWidgets 2.4.x
    pairarr.h - defines a a macro for arbitrary wxArrays of key, value pairs
   
src/
    sheet.cpp - the main source file for the wxSheet
    sheetctl.cpp - the source file for the renderers and editors
    sheetsel.cpp - the source file for the wxSheetSelection mechanism
    sheetspt.cpp - the source file for the wxSheetSplitter
    Makefile - makefile for gcc, relies on wx-config in your path
               generates a library in your `wx-config --prefix`/lib dir
    sheetlib_wx24.dsp - project file for lib, Visual Studio for wxWidgets 2.4.x
    sheetlib_wx25.dsp - project file for lib, Visual Studio for wxWidgets 2.5.x

samples/sheet
    sheetdemo.h/cpp - a sample program based on wxWidget's griddemo sample
    Makefile - makefile for gcc, relies on wx-config in your path
    sheetdemo_wx24.dsp - project file for Visual Studio for wxWidgets 2.4.x
    sheetdemo_wx25.dsp - project file for Visual Studio for wxWidgets 2.5.x

===============================================================================
Compliation and use:

    wxSheet has been tested in wxMSW and wxGTK using wxWidgets 2.4.2 and 2.5. 
It should be able to run on any platform that the wxGrid does. There may however
be some "off by one" sizing errors for the editors since calls to 
wxWindow::SetSize may not work as expected. I tried to preserve 
the different sizing #ifdefs as used in the wxGrid, but sometimes they were 
quite baffling and should be properly cleaned up when the can be tested. 
I welcome any fixes for different platforms or versions of wxWindows than the
ones I use (GTK, MSW, 2.4.2, but mostly CVS head).

There is a Makefile for building a library and the sample sheetdemo (griddemo)
program for Unix systems and a MSVS project file for the sample.

You can build it as a library or just include the source files in your 
program. It is an independent widget and doesn't rely on the wxGrid, though
it does use the hash map and string array defined in grid.h.

