/////////////////////////////////////////////////////////////////////////////
// Name:        sheetsel.h
// Purpose:     wxSheetSelection
// Author:      John Labenski
// Modified by:
// Created:     20/02/2000
// RCS-ID:      $$
// Copyright:   (c) John Labenski, Stefan Neis
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETSEL_H__
#define __WX_SHEETSEL_H__

#include "wx/defs.h"
#if wxUSE_GRID

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheetsel.h"
#endif

#include "wx24defs.h"  // wx24 backwards compatibility

class WXDLLIMPEXP_ADV wxSheetCoords;
class WXDLLIMPEXP_ADV wxSheetBlock;

// ----------------------------------------------------------------------------
// wxSheetCoords: location of a cell in the grid
//
// Though this is based on a wxPoint you are *STRONGLY* discouraged from using
// the members int x,y use Get/SetRow and Get/SetCol
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_ADV wxSheetCoords
{
public:
    wxSheetCoords() : m_row(0), m_col(0) {}
    wxSheetCoords( int row, int col ) : m_row(row), m_col(col) {}

    // default copy ctor is ok

    int  GetRow() const          { return m_row; }
    int  GetCol() const          { return m_col; }

    void SetRow( int row )       { m_row = row; }
    void SetCol( int col )       { m_col = col; }
    void Set( int row, int col ) { m_row = row; m_col = col; }
    
    // returns coords shifted by the # of rows and cols
    wxSheetCoords GetShifted( int rows, int cols ) { return wxSheetCoords(m_row+rows, m_col+cols); }
    
    // operators
    wxSheetCoords& operator = ( const wxSheetCoords& other )
       { m_row = other.m_row; m_col = other.m_col; return *this; }

    // arithmetic operations (component wise)
    wxSheetCoords operator+(const wxSheetCoords& c) const { return wxSheetCoords(m_row + c.m_row, m_col + c.m_col); }
    wxSheetCoords operator-(const wxSheetCoords& c) const { return wxSheetCoords(m_row - c.m_row, m_col - c.m_col); }

    wxSheetCoords& operator+=(const wxSheetCoords& c) { m_row += c.m_row; m_col += c.m_col; return *this; }
    wxSheetCoords& operator-=(const wxSheetCoords& c) { m_row -= c.m_row; m_col -= c.m_col; return *this; }

    bool operator == ( const wxSheetCoords& other ) const { return (m_row == other.m_row) && (m_col == other.m_col); }
    bool operator != ( const wxSheetCoords& other ) const { return (m_row != other.m_row) || (m_col != other.m_col); }
    // > and < operators use row for first comparison then col
    bool operator < ( const wxSheetCoords& other ) const  
        { return (m_row < other.m_row) || ((m_row == other.m_row) && (m_col < other.m_col)); }
    bool operator <= ( const wxSheetCoords& other ) const { return ((*this < other) || (*this == other)) ? TRUE : FALSE; }
    bool operator >  ( const wxSheetCoords& other ) const { return !(*this < other); }
    bool operator >= ( const wxSheetCoords& other ) const { return ((*this > other) || (*this == other)) ? TRUE : FALSE; }

    int m_row;
    int m_col;    
};

WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetCoords, wxArraySheetCoords, class WXDLLIMPEXP_ADV);

// ----------------------------------------------------------------------------
// wxSheetBlock: a rectangular block of cells
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxSheetBlock
{
public:    
    wxSheetBlock() : m_row(0), m_col(0), m_height(0), m_width(0) {}
    wxSheetBlock(int row, int col, int height, int width ) 
        : m_row(row), m_col(col), m_height(height), m_width(width) {}
    // make a block from two corner coords, block will be upright
    wxSheetBlock( const wxSheetCoords& coords1, const wxSheetCoords& coords2 );
    wxSheetBlock( const wxSheetCoords& tl, int height, int width ) 
        : m_row(tl.GetRow()), m_col(tl.GetCol()), m_height(height), m_width(width) {}

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

    void SetCoords( int top, int left, int bottom, int right )
        { m_row=top; m_col=left; m_height=bottom-top+1; m_width=right-left+1; }
    
    void SetSize(const wxSheetCoords& size) { m_height = size.m_row; m_width = size.m_col; }
    
    // Get a block of this that is upright
    wxSheetBlock GetAligned() const 
        { return wxSheetBlock( m_height < 0 ? m_row + m_height - 1 : m_row,
                               m_width  < 0 ? m_col + m_width  - 1 : m_col, 
                               m_height < 0 ? -m_height + 2 : m_height,
                               m_width  < 0 ? -m_width  + 2 : m_width ); }
    
    bool IsEmpty() const { return (m_width < 1) || (m_height < 1); }
    bool IsOneCell() const { return (m_width == 1) && (m_height == 1); }
    
    bool Contains( int row, int col ) const
        { return (row >= m_row) && (col >= m_col) && 
                 (row <= GetBottom()) && (col <= GetRight()); }
    bool Contains( const wxSheetCoords &coord ) const { return Contains(coord.m_row, coord.m_col); }
    bool Contains( const wxSheetBlock &b ) const
        { return !IsEmpty() && (m_row <= b.m_row) && (b.GetBottom() <= GetBottom()) && 
                               (m_col <= b.m_col) && (b.GetRight()  <= GetRight()); }

    bool Intersects( const wxSheetBlock &b ) const
        { return !IsEmpty() && !b.IsEmpty() && 
                 (wxMax(m_col, b.m_col) <= wxMin(GetRight(),  b.GetRight())) && 
                 (wxMax(m_row, b.m_row) <= wxMin(GetBottom(), b.GetBottom())); }
    wxSheetBlock Intersect( const wxSheetBlock &other ) const
    {
        int l = wxMax(m_col,       other.m_col);
        int r = wxMin(GetRight(),  other.GetRight());
        int t = wxMax(m_row,       other.m_row);
        int b = wxMin(GetBottom(), other.GetBottom());
        return wxSheetBlock(t, l, wxMax(b-t+1, 0), wxMax(r-l+1, 0));
    }

    // union these two blocks
    wxSheetBlock Union( const wxSheetBlock &other ) const
    {
        int l = wxMin(m_col,       other.m_col);
        int r = wxMax(GetRight(),  other.GetRight());
        int t = wxMin(m_row,       other.m_row);
        int b = wxMax(GetBottom(), other.GetBottom());
        return wxSheetBlock(t, l, wxMax(b-t+1, 0), wxMax(r-l+1, 0));
    }
    
    // expands the union of the two, if one block is empty, return other
    wxSheetBlock ExpandUnion( const wxSheetBlock &other ) const
    {
        if (IsEmpty()) return other;        // preserve other block
        if (other.IsEmpty()) return *this;
            
        int l = wxMin(m_col,       other.m_col);
        int r = wxMax(GetRight(),  other.GetRight());
        int t = wxMin(m_row,       other.m_row);
        int b = wxMax(GetBottom(), other.GetBottom());
        return wxSheetBlock(t, l, wxMax(b-t+1, 0), wxMax(r-l+1, 0));
    }
    
    // Unlike Intersects this also includes just touching the other block
    bool Touches(const wxSheetBlock &block) const
        { return !IsEmpty() && !block.IsEmpty() && 
                 block.Intersects(wxSheetBlock(m_row-1, m_col-1, m_height+2, m_width+2)); }
    
    // Try to combine these blocks, they must touch and fit to make a single larger block
    //    this block is expanded if possible
    bool Combine(const wxSheetBlock &block);

    // test combining the input block with this one, returning the 
    // remainder of block in top, bottom, left, right - each may be IsEmpty()
    // returns FALSE if blocks don't touch or this block already contains block
    // |---------------------------|
    // |           top             |
    // |---------------------------|
    // |   left  |block|  right    |
    // |---------------------------|
    // |          bottom           |
    // |---------------------------|
    bool Combine( const wxSheetBlock &block, 
                  wxSheetBlock &top, wxSheetBlock &bottom, 
                  wxSheetBlock &left, wxSheetBlock &right ) const;

    // test removal of a portion or all of this contained in block returning the
    // remainder of this in top, bottom, left, right - each may be IsEmpty()
    // returns FALSE if nothing to delete, this cell is not changed
    bool Delete( const wxSheetBlock &block, 
                 wxSheetBlock &top,  wxSheetBlock &bottom, 
                 wxSheetBlock &left, wxSheetBlock &right ) const;
    
    // operators
    bool operator == (const wxSheetBlock& b) const
        { return (m_row == b.m_row) && (m_height == b.m_height) && 
                 (m_col == b.m_col) && (m_width  == b.m_width); }
    bool operator != (const wxSheetBlock& b) const { return !(*this == b); }

    int m_row, m_col, m_height, m_width;    
};

// For comparisons...
extern const WXDLLIMPEXP_ADV wxSheetCoords wxNullSheetCoords;        // (-2, -2)
extern const WXDLLIMPEXP_ADV wxSheetCoords wxGridCellSheetCoords;    // ( 0,  0)
extern const WXDLLIMPEXP_ADV wxSheetCoords wxRowLabelSheetCoords;    // ( 0, -1)
extern const WXDLLIMPEXP_ADV wxSheetCoords wxColLabelSheetCoords;    // (-1,  0)
extern const WXDLLIMPEXP_ADV wxSheetCoords wxCornerLabelSheetCoords; // (-1, -1)

extern const WXDLLIMPEXP_ADV wxSheetBlock  wxNullSheetBlock;  // (0, 0, 0, 0)
extern const WXDLLIMPEXP_ADV wxRect        wxSheetNoCellRect; // FIXME this is not necessary

WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetBlock, wxArraySheetBlock, class WXDLLIMPEXP_ADV);

// ----------------------------------------------------------------------------
// wxSheetSelection
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_ADV wxSheetSelection
{
public:
    wxSheetSelection();
    wxSheetSelection( const wxSheetSelection& other ) { Copy( other ); }
    wxSheetSelection( const wxSheetBlock& block );
    
    // Make a full copy of the source
    void Copy(const wxSheetSelection &source);

    bool HasSelection() const { return m_blocks.GetCount() != 0u; }
    int  GetCount() const     { return m_blocks.GetCount(); }
    bool IsMinimzed() const   { return m_minimized; }
    bool IsSorted() const     { return m_sorted; }
    
    bool Clear() { if (m_blocks.GetCount() != 0u) { m_blocks.Clear(); return TRUE; } return FALSE; }
    bool Empty() { if (m_blocks.GetCount() != 0u) { m_blocks.Empty(); return TRUE; } return FALSE; }

    const wxArraySheetBlock& GetBlockArray() const { return m_blocks; }

    const wxSheetBlock& GetBlock( size_t index ) const;
    const wxSheetBlock& Item( size_t index ) const { return GetBlock(index); }
    
    // Get a block that bounds the selection
    wxSheetBlock GetBoundingBlock() const;
    // Set the outer bounds of the selection, trimming it down as necessary
    void SetBoundingBlock(const wxSheetBlock& block);
    
    // do any of the blocks contain the elements
    bool Contains( int row, int col ) const { return Index(row,col) != wxNOT_FOUND; }
    bool Contains( const wxSheetCoords &c ) const { return Index(c.GetRow(), c.GetCol()) != wxNOT_FOUND; }
    bool Contains( const wxSheetBlock &b ) const;

    // Get the index of a block that fully contains element or wxNOT_FOUND
    int Index( int row, int col ) const;
    int Index( const wxSheetCoords &c ) const { return Index(c.GetRow(), c.GetCol()); }
    int Index( const wxSheetBlock &b ) const;
    // Get the first index of the block that intersects input block or wxNOT_FOUND
    int IndexIntersects( const wxSheetBlock &b ) const;
    
    // Add the block to the selection, returns FALSE if nothing was done 
    //   use combineNow=FALSE to make quick additions, when done call Minimize()
    //   addedBlocks (if !NULL) will be filled with the actual changed selections
    //   by removing the previous selections from the input block
    bool SelectBlock( const wxSheetBlock &block, bool combineNow = TRUE, 
                      wxArraySheetBlock *addedBlocks = NULL );
    
    // Remove the block to the selection, return FALSE if nothing was done
    //   use combineNow=FALSE to make quick additions, when done call Minimize()
    //   deletedBlocks (if !NULL) contains the input block, FIXME
    //    [it should contain only the parts of the input block that were
    //     deleted in the selection, but the speed would greatly suffer, 
    //     unlike SelectBlock which uses an array in the selection process]
    bool DeselectBlock( const wxSheetBlock &block, bool combineNow = TRUE,
                        wxArraySheetBlock *deletedBlocks = NULL );

    // Sorts the blocks top_left_bottom_right, by rows then cols
    //   only need to call this if you've called (De)SelectBlock(block, FALSE)
    void Sort();
    
    // Combine the blocks if possible and Sort() them, returns if any were combined
    //   only need to call this if you've called (De)SelectBlock(block, FALSE)
    bool Minimize();

    // Operators
    inline wxSheetBlock operator[](size_t index) const { return GetBlock(index); }
    wxSheetSelection& operator = (const wxSheetSelection& other) { Copy(other); return *this; }

    // implementation
    
    // Calls DoDoMinimize on array until it returns false
    bool DoMinimize( wxArraySheetBlock &blocks ) const;
    // DoMinimize calls this internally so that it doesn't recurse
    //   generic routine using if (b1.Combine(b2)) remove b2 to cleanup array
    bool DoDoMinimize( wxArraySheetBlock &blocks ) const;

protected:
    wxArraySheetBlock m_blocks;
    bool m_minimized; 
    bool m_sorted;
};

// ----------------------------------------------------------------------------
// wxSheetSelectionIterator - iterates through a wxSheetSelection cell by cell
// ----------------------------------------------------------------------------

enum wxSheetSelectionIter_Type
{
    wxSSI_FORWARD, // iterate left to right by cols and then down by rows
    wxSSI_REVERSE  // iterate bottom to top by rows and then left by cols
};

class wxSheetSelectionIterator
{
public :
    wxSheetSelectionIterator( const wxSheetSelection &sel, 
                              wxSheetSelectionIter_Type type = wxSSI_FORWARD );
    wxSheetSelectionIterator( const wxArraySheetBlock &blocks,
                              wxSheetSelectionIter_Type type = wxSSI_FORWARD );
    
    // resets the iterating to start at the beginning
    void Reset(wxSheetSelectionIter_Type type);
    wxSheetSelectionIter_Type GetIterType() const { return (wxSheetSelectionIter_Type)m_type; }

    // Get next cell in the selection, returns FALSE if at end
    bool GetNext(wxSheetCoords &coords);

    // checks if this row and col are in this selection
    bool IsInSelection(const wxSheetCoords &c) const { return IsInSelection(c.m_row, c.m_col); }
    bool IsInSelection( int row, int col ) const;

protected :
    bool GetNextForward(wxSheetCoords &coords);
    bool GetNextReverse(wxSheetCoords &coords);

    int m_block_index;
    int m_type;
    wxSheetCoords m_coords;
    wxArraySheetBlock m_blocks;
};


#endif  // #ifdef __WXSHEETSEL_H__
#endif  // #ifndef wxUSE_GRID
