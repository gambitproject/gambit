///////////////////////////////////////////////////////////////////////////////
// Name:        sheetsel.h
// Purpose:     wxSheetSelection
// Author:      John Labenski
// Modified by:
// Created:     20/02/2000
// RCS-ID:      $$
// Copyright:   (c) John Labenski, Stefan Neis
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETSEL_H__
#define __WX_SHEETSEL_H__

#include "wx/sheet/sheetdef.h"

// ----------------------------------------------------------------------------
// wxSheetCoords: location of a cell in the grid
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetCoords
{
public:
    wxSheetCoords() : m_row(0), m_col(0) {}
    wxSheetCoords( int row, int col ) : m_row(row), m_col(col) {}

    // default copy ctor is ok

    int  GetRow() const              { return m_row; }
    int  GetCol() const              { return m_col; }

    void SetRow( int row )           { m_row = row; }
    void SetCol( int col )           { m_col = col; }
    void Set( int row, int col )     { m_row = row; m_col = col; }

    void ShiftRow( int row )         { m_row += row; }
    void ShiftCol( int col )         { m_col += col; }
    void Shift( int rows, int cols ) { m_row += rows; m_col += cols; }
    void SwapRowCol()                { int tmp = m_row; m_row = m_col; m_col = tmp; }

    // returns coords shifted by the # of rows and cols
    wxSheetCoords GetShifted( int rows, int cols ) const { return wxSheetCoords(m_row+rows, m_col+cols); }
    // return coords with row and col swapped
    wxSheetCoords GetSwapped() const { return wxSheetCoords(m_col, m_row); }

    // Get the type of coords as enum for use in a switch statement
    wxSheetCell_Type GetCellCoordsType() const;

    // helper functions to determine what type of cell it is, not check validity
    bool IsGridCell() const  { return (m_row >=  0) && (m_col >=  0); }
    bool IsLabelCell() const { return (m_row >= -1) && (m_col >= -1) &&
                                     ((m_row == -1) || (m_col == -1)); }
    bool IsRowLabelCell() const { return (m_row >=  0) && (m_col == -1); }
    bool IsColLabelCell() const { return (m_row == -1) && (m_col >=  0); }
    bool IsCornerLabelCell() const { return (m_row == -1) && (m_col == -1); }
    
    // Convert this sheet coords to a row/col/corner label coords
    //  eg. for row labels : sheetCell(-1, 5) -> rowLabelCell(0, 5)
    wxSheetCoords SheetToRowLabel()    const { return wxSheetCoords(   m_row, -1-m_col); }
    wxSheetCoords SheetToColLabel()    const { return wxSheetCoords(-1-m_row,    m_col); }
    wxSheetCoords SheetToCornerLabel() const { return wxSheetCoords(-1-m_row, -1-m_col); }
    
    wxSheetCoords GetCellCoords(wxSheetCell_Type type) const;
    wxSheetCoords& SetCellCoords(wxSheetCell_Type type);
    
    // Shift the cell if greater than row/col by numRows/numCols.
    //   if this row/col is < the update row/col do nothing, return false
    //   if this row/col is >= the update row/col + labs(num) then shift by num
    //   if this row/col is >= the update row/col && < row/col + labs(num)
    //      if num > 0 then shift it
    //      if num < 0 it should be deleted, but shift it to row/col - 1
    bool UpdateRows( size_t row, int numRows );
    bool UpdateCols( size_t col, int numCols );
    
    // operators
    wxSheetCoords& operator=(const wxSheetCoords& other) { m_row = other.m_row; m_col = other.m_col; return *this; }

    // arithmetic operations (component wise)
    wxSheetCoords operator+(const wxSheetCoords& c) const { return wxSheetCoords(m_row + c.m_row, m_col + c.m_col); }
    wxSheetCoords operator-(const wxSheetCoords& c) const { return wxSheetCoords(m_row - c.m_row, m_col - c.m_col); }

    wxSheetCoords& operator+=(const wxSheetCoords& c) { m_row += c.m_row; m_col += c.m_col; return *this; }
    wxSheetCoords& operator-=(const wxSheetCoords& c) { m_row -= c.m_row; m_col -= c.m_col; return *this; }

    bool operator == (const wxSheetCoords& other) const { return (m_row == other.m_row) && (m_col == other.m_col); }
    bool operator != (const wxSheetCoords& other) const { return !(*this == other); }
    // > and < operators use row for first comparison then col
    bool operator <  (const wxSheetCoords& other) const 
        { return (m_row < other.m_row) || ((m_row == other.m_row) && (m_col < other.m_col)); }
    bool operator <= (const wxSheetCoords& other) const { return  (*this <  other) || (*this == other); }
    bool operator >  (const wxSheetCoords& other) const { return !(*this <= other); }
    bool operator >= (const wxSheetCoords& other) const { return !(*this <  other); }

    int m_row;
    int m_col;    
};

// wxArraySheetCoords - a wxObjectArray of wxSheetCoords
WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetCoords, wxArraySheetCoords, class WXDLLIMPEXP_SHEET);

// ----------------------------------------------------------------------------
// wxSheetBlock: a rectangular block of cells
// ----------------------------------------------------------------------------
enum wxSheetBlockExtra_Type
{
    wxSHEET_BLOCK_NONE   = 0,
    wxSHEET_BLOCK_TOP    = 0x0001,
    wxSHEET_BLOCK_BOTTOM = 0x0002,
    wxSHEET_BLOCK_LEFT   = 0x0004,
    wxSHEET_BLOCK_RIGHT  = 0x0008,
    wxSHEET_BLOCK_ALL    = 0x0010    
};

class WXDLLIMPEXP_SHEET wxSheetBlock
{
public:    
    wxSheetBlock() : m_row(0), m_col(0), m_height(0), m_width(0) {}
    wxSheetBlock(int row, int col, int height, int width) 
        : m_row(row), m_col(col), m_height(height), m_width(width) {}
    // make a block from two corner coords, block will be upright
    wxSheetBlock( const wxSheetCoords& coords1, 
                  const wxSheetCoords& coords2, bool make_upright = true );
    wxSheetBlock( const wxSheetCoords& tl, int height, int width ) 
        : m_row(tl.m_row), m_col(tl.m_col), m_height(height), m_width(width) {}

    // Get the coord values of the block
    int GetLeft()   const { return m_col; }
    int GetRight()  const { return m_col + m_width - 1; }
    int GetTop()    const { return m_row; }
    int GetBottom() const { return m_row + m_height - 1; }
    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

    wxSheetCoords GetLeftTop()     const { return wxSheetCoords(m_row, m_col); }    
    wxSheetCoords GetLeftBottom()  const { return wxSheetCoords(GetBottom(), m_col); }
    wxSheetCoords GetRightTop()    const { return wxSheetCoords(m_row, GetRight()); }
    wxSheetCoords GetRightBottom() const { return wxSheetCoords(GetBottom(), GetRight()); } 
    
    wxSheetCoords GetSize() const { return wxSheetCoords(m_height, m_width); }
    
    // get an array of coords going left to right, top to bottom
    wxArraySheetCoords GetArrayCoords() const;
    
    // Set the coord values of the block 
    
    // set the edges, the rest of the block stays in the same place
    void SetLeft( int left )     { m_width  += m_col  - left; m_col = left; }
    void SetTop( int top )       { m_height += m_row  - top;  m_row = top; }
    void SetRight( int right )   { m_width   = right  - m_col + 1; }
    void SetBottom( int bottom ) { m_height  = bottom - m_row + 1; }
    void SetWidth( int width )   { m_width   = width; }
    void SetHeight( int height ) { m_height  = height; }
    
    void SetLeftTop(const wxSheetCoords& lt)     { SetTop(lt.m_row);    SetLeft(lt.m_col); }
    void SetLeftBottom(const wxSheetCoords& lb)  { SetBottom(lb.m_row); SetLeft(lb.m_col); }
    void SetRightTop(const wxSheetCoords& rt)    { SetTop(rt.m_row);    SetRight(rt.m_col); }
    void SetRightBottom(const wxSheetCoords& rb) { SetBottom(rb.m_row); SetRight(rb.m_col); }

    // Set the coords keeping the rest of the block the same
    void SetLeftCoord( int left )     { m_col = left; }
    void SetTopCoord( int top )       { m_row = top; }
    void SetRightCoord( int right )   { m_col = right - m_width + 1; }
    void SetBottomCoord( int bottom ) { m_row = bottom - m_height + 1; }

    void SetLeftTopCoords(const wxSheetCoords& lt)     { SetTopCoord(lt.m_row);    SetLeftCoord(lt.m_col); }
    void SetLeftBottomCoords(const wxSheetCoords& lb)  { SetBottomCoord(lb.m_row); SetLeftCoord(lb.m_col); }
    void SetRightTopCoords(const wxSheetCoords& rt)    { SetTopCoord(rt.m_row);    SetRightCoord(rt.m_col); }
    void SetRightBottomCoords(const wxSheetCoords& rb) { SetBottomCoord(rb.m_row); SetRightCoord(rb.m_col); }
    
    void Set( int row, int col, int height, int width ) 
        { m_row = row; m_col = col; m_height = height; m_width = width; }
    void SetCoords( int top, int left, int bottom, int right )
        { m_row=top; m_col=left; m_height=bottom-top+1; m_width=right-left+1; }
    void SetSize(const wxSheetCoords& size) { m_height = size.m_row; m_width = size.m_col; }
    
    // Get a block of this that is upright
    wxSheetBlock GetAligned() const;
    
    bool IsEmpty() const { return (m_width < 1) || (m_height < 1); }
    bool IsOneCell() const { return (m_width == 1) && (m_height == 1); }
    
    bool Contains( int row, int col ) const
        { return (row >= m_row) && (col >= m_col) && 
                 (row <= GetBottom()) && (col <= GetRight()); }
    bool Contains( const wxSheetCoords &coord ) const { return Contains(coord.m_row, coord.m_col); }
    bool Contains( const wxSheetBlock &b ) const
        { return !IsEmpty() && !b.IsEmpty() &&
                 (m_row <= b.m_row) && (m_col <= b.m_col) && 
                 (b.GetBottom() <= GetBottom()) && (b.GetRight() <= GetRight()); }

    // do these two blocks intersect (overlap) each other
    bool Intersects( const wxSheetBlock &b ) const
        { return !Intersect(b).IsEmpty(); }
                 //!IsEmpty() && !b.IsEmpty() && 
                 //(wxMax(m_col, b.m_col) <= wxMin(GetRight(),  b.GetRight())) && 
                 //(wxMax(m_row, b.m_row) <= wxMin(GetBottom(), b.GetBottom())); }
    // returns a block that is an intersection of these two blocks
    wxSheetBlock Intersect( const wxSheetBlock &other ) const;

    // union these two blocks
    wxSheetBlock Union( const wxSheetBlock &other ) const;
    
    // expands the union of the two, if one block is empty, return other
    wxSheetBlock ExpandUnion( const wxSheetBlock &other ) const;
    
    // Unlike Intersects this also includes just touching the other block
    bool Touches(const wxSheetBlock &block) const
        { return !IsEmpty() && !block.IsEmpty() && 
                 block.Intersects(wxSheetBlock(m_row-1, m_col-1, m_height+2, m_width+2)); }
    
    // returns a mix of enum wxSheetBlockExtra_Type of what sides of the two
    // blocks are matched
    int SideMatches(const wxSheetBlock& block) const 
    {
        return (m_row       == block.m_row       ? wxSHEET_BLOCK_TOP    : 0) |
               (m_col       == block.m_col       ? wxSHEET_BLOCK_LEFT   : 0) |
               (GetBottom() == block.GetBottom() ? wxSHEET_BLOCK_BOTTOM : 0) |
               (GetRight()  == block.GetRight()  ? wxSHEET_BLOCK_RIGHT  : 0);
    }

    // Try to combine these blocks, they must touch and fit to make a single larger block
    //    this block is expanded if possible, returns success
    bool Combine(const wxSheetBlock &block);

    // test combining the input block with this one, returning the 
    // remainder of block in top, bottom, left, right - each may be IsEmpty()
    // returns enum wxSheetBlockExtra_Type Or'ed together specifying which 
    // blocks have been filled or all of the block may be combined
    // returns false if blocks don't touch or this block already contains block
    // |---------------------------|
    // |           top             |
    // |---------------------------|
    // |   left  |block|  right    |
    // |---------------------------|
    // |          bottom           |
    // |---------------------------|
                 
    int Combine( const wxSheetBlock &block, 
                  wxSheetBlock &top, wxSheetBlock &bottom, 
                  wxSheetBlock &left, wxSheetBlock &right ) const;

    // test removal of a portion or all of this contained in block returning the
    // remainder of this in top, bottom, left, right - each may be IsEmpty()
    // returns false if nothing to delete, this cell is not changed
    int Delete( const wxSheetBlock &block, 
                 wxSheetBlock &top,  wxSheetBlock &bottom, 
                 wxSheetBlock &left, wxSheetBlock &right ) const;

    // shift the block if greater than row/col by numRows/numCols or empty it
    //  if it's inside the deletion
    bool UpdateRows( size_t row, int numRows );
    bool UpdateCols( size_t col, int numCols );

    // operators
    bool operator == (const wxSheetBlock& b) const
        { return (m_row == b.m_row) && (m_height == b.m_height) && 
                 (m_col == b.m_col) && (m_width  == b.m_width); }
    bool operator != (const wxSheetBlock& b) const { return !(*this == b); }

    // returns -1 if this block is more to the top left, 0 is equal, 1 if lower and to right
    int CmpTopLeft(const wxSheetBlock& b) const; 
    // returns -1 if this block is more to the right then bottom, 0 is equal, 1 if to the left and higher
    int CmpRightBottom(const wxSheetBlock& b) const;

    // goes top left to bottom right and then by height and width (allows sorting)
    bool operator <  (const wxSheetBlock& b) const 
    { 
        if (m_row    < b.m_row)    return true; else if (m_row    > b.m_row)    return false;
        if (m_col    < b.m_col)    return true; else if (m_col    > b.m_col)    return false;
        if (m_height < b.m_height) return true; else if (m_height > b.m_height) return false;
        if (m_width  < b.m_width)  return true; //else if (m_width > b.m_width) return false;
        return false;
    }
    bool operator <= (const wxSheetBlock& other) const { return  (*this == other) || (*this < other); }
    bool operator >  (const wxSheetBlock& other) const { return !(*this <= other); }
    bool operator >= (const wxSheetBlock& other) const { return !(*this <  other); }
    
protected:    
    int m_row, m_col, m_height, m_width;    
};

// For comparisons...
WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCoords) wxNullSheetCoords;        // (-2, -2)
WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCoords) wxGridCellSheetCoords;    // ( 0,  0)
WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCoords) wxRowLabelSheetCoords;    // ( 0, -1)
WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCoords) wxColLabelSheetCoords;    // (-1,  0)
WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCoords) wxCornerLabelSheetCoords; // (-1, -1)

WXDLLIMPEXP_DATA_SHEET(extern const wxSheetBlock)  wxNullSheetBlock;  // (0, 0, 0, 0)

// wxArraySheetBlock - a wxObjectArray of wxSheetBlocks
WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetBlock, wxArraySheetBlock, class WXDLLIMPEXP_SHEET);

// ----------------------------------------------------------------------------
// wxSheetSelection
// ----------------------------------------------------------------------------

enum wxSheetSelection_Type
{
    wxSHEET_SELECTION_NONE = 0,
    // allow multiple selections to occur, this will cause the 
    //    wxSheetSelection::Index to return the first occurance of possible many
    wxSHEET_SELECTION_MULTIPLE_SEL = 0x0001    
};

class WXDLLIMPEXP_SHEET wxSheetSelection
{
public:
    wxSheetSelection( int options = wxSHEET_SELECTION_NONE );
    wxSheetSelection( const wxSheetSelection& other ) { Copy( other ); }
    wxSheetSelection( const wxSheetBlock& block, 
                      int options = wxSHEET_SELECTION_NONE );
    
    // Make a full copy of the source
    void Copy(const wxSheetSelection &source);

    int GetOptions() const { return m_options; }
    void SetOptions(int options) { m_options = options; }
    
    bool HasSelection() const { return GetCount() != 0; }
    int  GetCount() const     { return m_blocks.GetCount(); }
    bool IsMinimzed() const   { return m_minimized; }
    
    bool Clear() { if (GetCount() != 0) { m_blocks.Clear(); return true; } return false; }
    bool Empty() { if (GetCount() != 0) { m_blocks.Empty(); return true; } return false; }

    const wxArraySheetBlock& GetBlockArray() const { return m_blocks; }

    const wxSheetBlock& GetBlock( size_t index ) const;
    const wxSheetBlock& Item( size_t index ) const { return GetBlock(index); }
    
    // Get a block that bounds the selection
    const wxSheetBlock& GetBoundingBlock() const { return m_bounds; }
    // Set the outer bounds of the selection, trimming it down as necessary
    void SetBoundingBlock(const wxSheetBlock& block);
    
    // do any of the blocks contain the elements
    bool Contains( int row, int col ) const { return Index(row,col) != wxNOT_FOUND; }
    bool Contains( const wxSheetCoords &c ) const { return Contains(c.GetRow(), c.GetCol()); }
    bool Contains( const wxSheetBlock &b ) const;

    // Get the index of a block that fully contains element or wxNOT_FOUND
    int Index( int row, int col ) const;
    int Index( const wxSheetCoords &c ) const { return Index(c.GetRow(), c.GetCol()); }
    int Index( const wxSheetBlock &b ) const;
    // Get the first index of the block that intersects input block or wxNOT_FOUND
    int IndexIntersects( const wxSheetBlock &b ) const;
    
    // Add the block to the selection, returns false if nothing was done 
    //   use combineNow=false to make quick additions, when done call Minimize()
    //   addedBlocks (if !NULL) will be filled with the actual changed selections
    //   by removing the previous selections from the input block
    bool SelectBlock( const wxSheetBlock &block, bool combineNow = true, 
                      wxArraySheetBlock *addedBlocks = NULL );
    
    // Remove the block to the selection, return false if nothing was done
    //   use combineNow=false to make quick additions, when done call Minimize()
    //   deletedBlocks (if !NULL) contains the input block, FIXME
    //    [it should contain only the parts of the input block that were
    //     deleted in the selection, but the speed would greatly suffer, 
    //     unlike SelectBlock which uses an array in the selection process]
    bool DeselectBlock( const wxSheetBlock &block, bool combineNow = true,
                        wxArraySheetBlock *deletedBlocks = NULL );

    // Update the number of rows/cols. In numRows/Cols > 0 insert them else
    //  remove them.
    bool UpdateRows( size_t row, int numRows );
    bool UpdateCols( size_t col, int numCols );

    // Operators
    inline wxSheetBlock operator[](size_t index) const { return GetBlock(index); }
    wxSheetSelection& operator = (const wxSheetSelection& other) { Copy(other); return *this; }

    // implementation   

    // Find where to insert this block
    int FindInsertIndex(const wxSheetBlock& block) const;
    // Find the index in the array that starts at this row
    int FindTopRow(int row) const;

    // Combine the blocks if possible and Sort() them, returns if any were combined
    //   only need to call this if you've called (De)SelectBlock(block, false)
    bool Minimize();
    
    // Calls DoDoMinimize on array until it returns false
    bool DoMinimize( wxArraySheetBlock &blocks ) const;
    // DoMinimize calls this internally so that it doesn't recurse
    //   generic routine using if (b1.Combine(b2)) remove b2 to cleanup array
    bool DoDoMinimize( wxArraySheetBlock &blocks ) const;

protected:
    int  InsertBlock(const wxSheetBlock& block); 
    void CalculateBounds();

    wxArraySheetBlock m_blocks;
    wxSheetBlock      m_bounds;
    bool m_minimized; 
    int  m_options;
};

// ----------------------------------------------------------------------------
// wxSheetSelectionIterator - iterates through a wxSheetSelection cell by cell
// ----------------------------------------------------------------------------

enum wxSheetSelectionIter_Type
{
    wxSSI_FORWARD, // iterate left to right by cols and then down by rows
    wxSSI_REVERSE  // iterate bottom to top by rows and then left by cols
};

enum wxSheetSelectionIterGet_Type
{
    wxSHEET_SELECTIONITER_GET_END         = 0,
    wxSHEET_SELECTIONITER_GET_LEFTTOP     = 0x0001,
    wxSHEET_SELECTIONITER_GET_RIGHTBOTTOM = 0x0002,
    wxSHEET_SELECTIONITER_GET_NEXTROW     = 0x0004,
    wxSHEET_SELECTIONITER_GET_NEXTCOL     = 0x0008
};

class WXDLLIMPEXP_SHEET wxSheetSelectionIterator
{
public :
    wxSheetSelectionIterator( const wxSheetSelection &sel, 
                              wxSheetSelectionIter_Type type = wxSSI_FORWARD );
    wxSheetSelectionIterator( const wxArraySheetBlock &blocks,
                              wxSheetSelectionIter_Type type = wxSSI_FORWARD );
    
    // resets the iterating to start at the beginning
    void Reset(wxSheetSelectionIter_Type type);
    // What direction are we iterating in
    wxSheetSelectionIter_Type GetIterType() const { return (wxSheetSelectionIter_Type)m_type; }

    // Get next cell in the selection, returns wxSheetSelectionIterGet_Type, 0 at end
    wxSheetSelectionIterGet_Type GetNext(wxSheetCoords &coords);

    // checks if this row and col are in this selection
    bool IsInSelection(const wxSheetCoords &c) const { return IsInSelection(c.m_row, c.m_col); }
    bool IsInSelection( int row, int col ) const;

protected :
    wxSheetSelectionIterGet_Type GetNextForward(wxSheetCoords &coords);
    wxSheetSelectionIterGet_Type GetNextReverse(wxSheetCoords &coords);

    int m_block_index;
    int m_type;
    wxSheetCoords m_coords;
    wxArraySheetBlock m_blocks;
};

#endif  // __WXSHEETSEL_H__
