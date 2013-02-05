///////////////////////////////////////////////////////////////////////////////
// Name:        sheetsel.cpp
// Purpose:     wxSheetSelection
// Author:      John Labenski
// Modified by: 
// Created:     20/02/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Stefan Neis (Stefan.Neis@t-online.de)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/utils.h"         // for wxMin and wxMax
    #include "wx/gdicmn.h"        // for wxRect
#endif // WX_PRECOMP

#include "wx/sheet/sheet.h"
#include "wx/sheet/sheetsel.h"

// if set then 
//#define CHECK_BLOCK_OVERLAP 1
//#define CHECK_BLOCK_SORTING 1

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArraySheetCoords)
WX_DEFINE_OBJARRAY(wxArraySheetBlock)

const wxSheetCoords wxNullSheetCoords( INT_MIN, INT_MIN );
const wxSheetCoords wxGridCellSheetCoords( 0, 0 );
const wxSheetCoords wxRowLabelSheetCoords( 0, -1 );
const wxSheetCoords wxColLabelSheetCoords( -1, 0 );
const wxSheetCoords wxCornerLabelSheetCoords( -1, -1 );
const wxSheetBlock  wxNullSheetBlock( 0, 0, 0, 0 );

// ----------------------------------------------------------------------------
// Compare functions for sorting
// ----------------------------------------------------------------------------

static int wxCMPFUNC_CONV wxsheetcellblock_sort_topleft_bottomright( wxSheetBlock **a, wxSheetBlock **b)
{ 
    return (*a)->CmpTopLeft(**b);
/*
    int row = ((*a)->m_row - (*b)->m_row);
    if (row < 0) return -1;
    if (row == 0) return ((*a)->m_col - (*b)->m_col);
    return 1;
*/
}
static int wxCMPFUNC_CONV wxsheetcellblock_sort_bottomright_topleft( wxSheetBlock **a, wxSheetBlock **b)
{ 
    return (*b)->CmpRightBottom(**a);
/*
    int col = ((*b)->GetRight() - (*a)->GetRight());    
    if (col < 0) return -1;
    if (col == 0) return ((*b)->GetBottom() - (*a)->GetBottom());
    return 1;    
*/
}

// ----------------------------------------------------------------------------
// wxSheetCoords: location of a cell in the grid
// ----------------------------------------------------------------------------

wxSheetCell_Type wxSheetCoords::GetCellCoordsType() const
{
    if ((m_row >= 0) && (m_col >= 0))
    {
        return wxSHEET_CELL_GRID;
    }
    else if (m_row == -1)
    {
        if (m_col == -1)
            return wxSHEET_CELL_CORNERLABEL;
        if (m_col >= 0)
            return wxSHEET_CELL_COLLABEL;
    }
    else if (m_col == -1)
    {
        if (m_row >= 0)
            return wxSHEET_CELL_ROWLABEL;
    }

    return wxSHEET_CELL_UNKNOWN;
}

wxSheetCoords wxSheetCoords::GetCellCoords(wxSheetCell_Type type) const
{
    switch (type)
    {
        case wxSHEET_CELL_GRID        : break;
        case wxSHEET_CELL_ROWLABEL    : return wxSheetCoords(   m_row, -1-m_col);
        case wxSHEET_CELL_COLLABEL    : return wxSheetCoords(-1-m_row,    m_col);
        case wxSHEET_CELL_CORNERLABEL : return wxSheetCoords(-1-m_row, -1-m_col);
        default : break;
    }
    return wxSheetCoords(*this);
}
wxSheetCoords& wxSheetCoords::SetCellCoords(wxSheetCell_Type type)
{
    switch (type)
    {
        case wxSHEET_CELL_GRID        : break;
        case wxSHEET_CELL_ROWLABEL    : m_col = -1-m_col; break;
        case wxSHEET_CELL_COLLABEL    : m_row = -1-m_row; break;
        case wxSHEET_CELL_CORNERLABEL : m_row = -1-m_row; m_col = -1-m_col; break;
        default : break;
    }
    return *this;
}

bool wxSheetCoords::UpdateRows( size_t row, int numRows )
{
    if ((numRows == 0) || (m_row < int(row))) return false;
    if ((numRows  > 0) || (m_row > int(row) - numRows))
        m_row += numRows;
    else
        m_row = int(row) - 1; // put it at beginning of delete
    
    return true;
}
bool wxSheetCoords::UpdateCols( size_t col, int numCols )
{
    if ((numCols == 0) || (m_col < int(col))) return false;
    if ((numCols  > 0) || (m_col > int(col) - numCols))
        m_col += numCols;
    else
        m_col = int(col) - 1; // put it at beginning of delete
    
    return true;
}

// ----------------------------------------------------------------------------
// wxSheetBlock: a rectangular block of cells
// ----------------------------------------------------------------------------

wxSheetBlock::wxSheetBlock( const wxSheetCoords& coords1, 
                            const wxSheetCoords& coords2, bool make_upright )
{
    m_row = coords1.m_row;
    m_col = coords1.m_col;
    m_height = coords2.m_row - coords1.m_row;
    m_width  = coords2.m_col - coords1.m_col;

    // block is upright with coords at corners
    if (make_upright)
    {
        if (m_width < 0)
        {
            m_width = -m_width;
            m_col = coords2.m_col;
        }
        m_width++;
    
        if (m_height < 0)
        {
            m_height = -m_height;
            m_row = coords2.m_row;
        }
        m_height++;
    }
}

wxArraySheetCoords wxSheetBlock::GetArrayCoords() const
{
    wxArraySheetCoords arrCoords;
    if (IsEmpty())
        return arrCoords;
    
    arrCoords.Alloc(GetWidth()*GetHeight());
    const int bottom = GetBottom(), right = GetRight();
    wxSheetCoords coords;
    
    for (coords.m_row = m_row; coords.m_row <= bottom; coords.m_row++)
    {
        for (coords.m_col = m_col; coords.m_col <= right; coords.m_col++)
            arrCoords.Add(coords);
    }
    
    return arrCoords;
}

wxSheetBlock wxSheetBlock::GetAligned() const 
{ 
    return wxSheetBlock( m_height < 0 ? m_row + m_height - 1 : m_row,
                         m_width  < 0 ? m_col + m_width  - 1 : m_col, 
                         m_height < 0 ? 2 - m_height         : m_height,
                         m_width  < 0 ? 2 - m_width          : m_width ); 
}

wxSheetBlock wxSheetBlock::Intersect( const wxSheetBlock &other ) const
{
    // no need to check if IsEmpty since wxMin(Right/Bottom) does it
    // ugly code, but fastest in gcc
    int l = other.GetRight();
    int r = GetRight();
    r = wxMin(r, l);
    l = wxMax(m_col, other.m_col);
    int w = r-l+1;
    if (w < 0) return wxSheetBlock();
    int t = other.GetBottom();
    int b = GetBottom();
    b = wxMin(b, t);
    t = wxMax(m_row, other.m_row);
    int h = b-t+1;
    if (h < 0) return wxSheetBlock();
    return wxSheetBlock(t, l, h, w);
    
/*      
    // simplier code, but slower        
    //int l = wxMax(m_col,       other.m_col);
    //int r = wxMin(GetRight(),  other.GetRight());
    //int t = wxMax(m_row,       other.m_row);
    //int b = wxMin(GetBottom(), other.GetBottom());
    //return ((b-t+1 < 0) || (r-l+1 < 0)) ? wxSheetBlock() : 
    //                                      wxSheetBlock(t, l, b-t+1, r-l+1);
*/                                              
}

wxSheetBlock wxSheetBlock::Union( const wxSheetBlock &other ) const
{
    // no need to check if IsEmpty since wxMin(Right/Bottom) does it
    // ugly code, but fastest in gcc
    int l = other.GetRight();
    int r = GetRight();
    r = wxMax(r, l);
    l = wxMin(m_col, other.m_col);
    int w = r-l+1;
    if (w < 0) return wxSheetBlock();
    int t = other.GetBottom();
    int b = GetBottom();
    b = wxMax(b, t);
    t = wxMin(m_row, other.m_row);
    int h = b-t+1;
    if (h < 0) return wxSheetBlock();
    return wxSheetBlock(t, l, h, w);
    
/*  
    // simplier code, but slower        
    //int l = wxMin(m_col,       other.m_col);
    //int r = wxMax(GetRight(),  other.GetRight());
    //int t = wxMin(m_row,       other.m_row);
    //int b = wxMax(GetBottom(), other.GetBottom());
    //return wxSheetBlock(t, l, wxMax(b-t+1, 0), wxMax(r-l+1, 0));
*/        
}

wxSheetBlock wxSheetBlock::ExpandUnion( const wxSheetBlock &other ) const
{
    if (IsEmpty()) return other;        // preserve other block
    if (other.IsEmpty()) return *this;  // preserve this

    // ugly code, but fastest in gcc
    int l = other.GetRight();
    int r = GetRight();
    r = wxMax(r, l);
    l = wxMin(m_col, other.m_col);
    int t = other.GetBottom();
    int b = GetBottom();
    b = wxMax(b, t);
    t = wxMin(m_row, other.m_row);
    return wxSheetBlock(t, l, b-t+1, r-l+1);
    
/*        
    // simplier code, but slower        
    //int l = wxMin(m_col,       other.m_col);
    //int r = wxMax(GetRight(),  other.GetRight());
    //int t = wxMin(m_row,       other.m_row);
    //int b = wxMax(GetBottom(), other.GetBottom());
    //return wxSheetBlock(t, l, b-t+1, r-l+1);
*/        
}

bool wxSheetBlock::Combine(const wxSheetBlock &block)
{
    
    //if (IsEmpty() || block.IsEmpty()) return false;
    if (!Touches(block)) return false;    
    if (Contains(block)) return true;
    if (block.Contains(*this))
    {
        *this = block;
        return true;
    }
    
    // FIXME I forgot why wxSheetBlock::Combine needs this code? Isn't Contains good enough?
    const wxSheetBlock uBlock(Union(block));
    if (uBlock.IsEmpty()) return false;

    // ugh this is really ugly, I can't figure a better way though
/*      
    // at least one of the two blocks has to be at each corner of the union
    if (((uBlock.GetLeftTop() == GetLeftTop()) || (uBlock.GetLeftTop() == block.GetLeftTop())) &&
        ((uBlock.GetRightTop() == GetRightTop()) || (uBlock.GetRightTop() == block.GetRightTop())) &&
        ((uBlock.GetLeftBottom() == GetLeftBottom()) || (uBlock.GetLeftBottom() == block.GetLeftBottom())) &&
        ((uBlock.GetRightBottom() == GetRightBottom()) || (uBlock.GetRightBottom() == block.GetRightBottom())) )
    {
        *this = uBlock;
        return true;
    }
*/

    const int t = GetTop();
    const int b = GetBottom();
    const int l = GetLeft();
    const int r = GetRight();

    const int b_t = block.GetTop();
    const int b_b = block.GetBottom();
    const int b_l = block.GetLeft();
    const int b_r = block.GetRight();

    const int ub_t = uBlock.GetTop();
    const int ub_b = uBlock.GetBottom();
    const int ub_l = uBlock.GetLeft();
    const int ub_r = uBlock.GetRight();

    if ( ( ((ub_t==t)&&(ub_l==l)) || ((ub_t==b_t)&&(ub_l==b_l)) ) &&
         ( ((ub_t==t)&&(ub_r==r)) || ((ub_t==b_t)&&(ub_r==b_r)) ) &&
         ( ((ub_b==b)&&(ub_l==l)) || ((ub_b==b_b)&&(ub_l==b_l)) ) &&
         ( ((ub_b==b)&&(ub_r==r)) || ((ub_b==b_b)&&(ub_r==b_r)) ) )
    {
        *this = uBlock;
        return true;
    }

    return false;


/*
    const int t = GetTop();
    const int b = GetBottom();
    const int l = GetLeft();
    const int r = GetRight();
    if ((t < b) || (l < r)) return false; // this is empty

    const int b_t = block.GetTop();
    const int b_b = block.GetBottom();
    const int b_l = block.GetLeft();
    const int b_r = block.GetRight();
    if ((b_t < b_b) || (b_l < b_r)) return false; // block is empty

    // if this contains other block
    if ((t <= b_t) && (l <= b_l) && (b >= b_b) && (r >= b_r))
        return true;
    // if block contains this
    if ((t >= b_t) && (l >= b_l) && (b <= b_b) && (r <= b_r))
    {
        *this = block;
        return true;
    }
    

    const int ub_t = uBlock.GetTop();
    const int ub_b = uBlock.GetBottom();
    const int ub_l = uBlock.GetLeft();
    const int ub_r = uBlock.GetRight();

    if ( ( ((ub_t==t)&&(ub_l==l)) || ((ub_t==b_t)&&(ub_l==b_l)) ) &&
         ( ((ub_t==t)&&(ub_r==r)) || ((ub_t==b_t)&&(ub_r==b_r)) ) &&
         ( ((ub_b==b)&&(ub_l==l)) || ((ub_b==b_b)&&(ub_l==b_l)) ) &&
         ( ((ub_b==b)&&(ub_r==r)) || ((ub_b==b_b)&&(ub_r==b_r)) ) )
    {
        *this = uBlock;
        return true;
    }

    return false;
*/    
}

int wxSheetBlock::Combine( const wxSheetBlock &block, 
                           wxSheetBlock &top, wxSheetBlock &bottom, 
                           wxSheetBlock &left, wxSheetBlock &right ) const
{       
    wxSheetBlock iBlock(Intersect(block));
    if (iBlock.IsEmpty()) return wxSHEET_BLOCK_NONE; // nothing to combine
    if (Contains(block)) return wxSHEET_BLOCK_ALL; // can combine all of block, no leftover
    
    int combined = wxSHEET_BLOCK_NONE;

    if ( block.GetTop() < GetTop() )
    {
        top.SetCoords( block.GetTop(), block.GetLeft(), GetTop()-1, block.GetRight() );
        combined |= wxSHEET_BLOCK_TOP;
    }
    if ( block.GetBottom() > GetBottom() )
    {
        bottom.SetCoords( GetBottom()+1, block.GetLeft(), block.GetBottom(), block.GetRight() );
        combined |= wxSHEET_BLOCK_BOTTOM;
    }
    if ( block.GetLeft() < GetLeft() )
    {
        left.SetCoords(iBlock.GetTop(), block.GetLeft(), iBlock.GetBottom(), GetLeft()-1 );
        combined |= wxSHEET_BLOCK_LEFT;
    }
    if ( block.GetRight() > GetRight() )
    {
        right.SetCoords( iBlock.GetTop(), GetRight()+1, iBlock.GetBottom(), block.GetRight() );
        combined |= wxSHEET_BLOCK_RIGHT;
    }

    return combined;
}

int wxSheetBlock::Delete( const wxSheetBlock &block, 
                          wxSheetBlock &top, wxSheetBlock &bottom, 
                          wxSheetBlock &left, wxSheetBlock &right ) const
{
    wxSheetBlock iBlock(Intersect(block));    
    if (iBlock.IsEmpty()) return wxSHEET_BLOCK_NONE; // nothing to delete
    if (block.Contains(*this)) return wxSHEET_BLOCK_ALL; // can delete all of this, no leftover

    int deleted = wxSHEET_BLOCK_NONE;
    
    if ( GetTop() < iBlock.GetTop() )
    {
        top.SetCoords( GetTop(), GetLeft(), iBlock.GetTop()-1, GetRight() );
        deleted |= wxSHEET_BLOCK_TOP;
    }
    if ( GetBottom() > iBlock.GetBottom() )
    {
        bottom.SetCoords( iBlock.GetBottom()+1, GetLeft(), GetBottom(), GetRight() );
        deleted |= wxSHEET_BLOCK_BOTTOM;
    }
    if ( GetLeft() < iBlock.GetLeft() )
    {
        left.SetCoords( iBlock.GetTop(), GetLeft(), iBlock.GetBottom(), iBlock.GetLeft()-1 );
        deleted |= wxSHEET_BLOCK_LEFT;
    }
    if ( GetRight() > iBlock.GetRight() )
    {
        right.SetCoords( iBlock.GetTop(), iBlock.GetRight()+1, iBlock.GetBottom(), GetRight() );
        deleted |= wxSHEET_BLOCK_RIGHT;
    }
    
    return deleted;
}

bool wxSheetBlock::UpdateRows( size_t row_, int numRows )
{
    int row = row_;
    if ((numRows == 0) || (GetBottom() < row)) return false;
    bool remove = numRows < 0;

    // this starts above the deleted rows
    if (m_row < row)
    {
        // this ends within deleted rows, trim to row
        if (remove && (GetBottom() < row - numRows))
            SetBottom(row-1);
        // this straddles the inserted/deleted rows - resize
        else 
            m_height += numRows;
    }
    // This is fully below it or an insert - shift coord
    else if (!remove || (m_row > row + labs(numRows)))
    {
        m_row += numRows;
    }
    // a remove and this's row is in deleted rows
    else 
    {
        m_height += m_row - (row - numRows);
        m_row = row;
    }

    return true;
}        
        
bool wxSheetBlock::UpdateCols( size_t col_, int numCols )
{
    int col = col_;
    if ((numCols == 0) || (GetRight() < col)) return false;
    bool remove = numCols < 0;

    // this starts above the deleted rows
    if (m_col < col)
    {
        // this ends within deleted rows, trim to row
        if (remove && (GetRight() < col - numCols))
            SetRight(col-1);
        // this straddles the inserted/deleted rows - resize
        else 
            m_width += numCols;
    }
    // This is fully below it or an insert - shift coord
    else if (!remove || (m_col > col + labs(numCols)))
    {
        m_col += numCols;
    }
    // a remove and this's row is in deleted rows
    else 
    {
        m_width += m_col - (col - numCols);
        m_col = col;
    }

    return true;
}

int wxSheetBlock::CmpTopLeft(const wxSheetBlock& b) const 
{
    int cmp = m_row - b.m_row;       if (cmp != 0) return cmp;
        cmp = m_col - b.m_col;       if (cmp != 0) return cmp;
        cmp = m_height - b.m_height; if (cmp != 0) return cmp;
        cmp = m_width  - b.m_width;                return cmp;
}
int wxSheetBlock::CmpRightBottom(const wxSheetBlock& b) const 
{
    int cmp = GetRight()  - b.GetRight();  if (cmp != 0) return cmp;
        cmp = GetBottom() - b.GetBottom(); if (cmp != 0) return cmp;
        cmp = m_col - b.m_col;             if (cmp != 0) return cmp;
        cmp = m_row - b.m_row;                           return cmp;
}

// ----------------------------------------------------------------------------
// wxSheetSelection
// ----------------------------------------------------------------------------

wxSheetSelection::wxSheetSelection(int options) 
                 : m_minimized(true), m_options(options)
{
}

wxSheetSelection::wxSheetSelection( const wxSheetBlock& block, int options ) 
                 : m_minimized(true), m_options(options)
{
    if (!block.IsEmpty())
    {
        m_blocks.Add(block); 
        m_bounds = block;
    }
}
    
void wxSheetSelection::Copy(const wxSheetSelection &source)
{
    m_blocks.Clear();
    WX_APPEND_ARRAY(m_blocks, source.GetBlockArray());
    m_bounds = source.GetBoundingBlock();
    m_minimized = source.IsMinimzed();
    m_options = source.GetOptions();
}

const wxSheetBlock& wxSheetSelection::GetBlock( size_t index ) const
{
    wxCHECK_MSG(index < m_blocks.GetCount(), wxNullSheetBlock, wxT("Invalid index"));
    return m_blocks[index];
}

void wxSheetSelection::SetBoundingBlock(const wxSheetBlock& block)
{
    m_bounds = wxNullSheetBlock;
    if (block.IsEmpty())
    {
        Clear();
        return;
    }
    
    int n, count = m_blocks.GetCount();
    
    for (n=0; n<count; n++)
    {
        m_blocks[n] = block.Intersect(m_blocks[n]);
        if (m_blocks[n].IsEmpty())
        {
            m_blocks.RemoveAt(n);
            count--;
            n--;
        }
        else
            m_bounds.ExpandUnion(m_blocks[n]);
    }        
}

bool wxSheetSelection::Contains( const wxSheetBlock &block ) const
{
    // FIXME - not tested, but should work    
    int n, k, count = m_blocks.GetCount();
    if ((count == 0) || !m_bounds.Contains(block))
        return false;
    
    wxArraySheetBlock extraBlocks;
    extraBlocks.Add(block);
    int extra_count = 1;

    const int bottom_row = block.GetBottom();
    wxSheetBlock top, bottom, left, right;
    
    // iterate though blocks cutting input block out and adding remainder to end
    for (n=FindTopRow(bottom_row); n<count; n++)
    {
        if (bottom_row < m_blocks[n].GetTop())
            break;
        
        for (k=0; k<extra_count; k++)
        {
            const int deleted = extraBlocks[k].Delete(m_blocks[n], top, bottom, left, right);
            if (deleted != wxSHEET_BLOCK_NONE)
            {
                extraBlocks.RemoveAt(k);
                extra_count--;
                k--;

                if (deleted != wxSHEET_BLOCK_ALL)
                {
                    if ((deleted & wxSHEET_BLOCK_TOP)    != 0) 
                        { extraBlocks.Add(top); extra_count++; }
                    if ((deleted & wxSHEET_BLOCK_BOTTOM) != 0) 
                        { extraBlocks.Add(bottom); extra_count++; }
                    if ((deleted & wxSHEET_BLOCK_LEFT)   != 0)   
                        { extraBlocks.Add(left); extra_count++; }
                    if ((deleted & wxSHEET_BLOCK_RIGHT)  != 0)  
                        { extraBlocks.Add(right); extra_count++; }
                }
                
                if (extra_count == 0)
                    return true;
            }
        }
    }
    
    return extra_count == 0;    
}

int wxSheetSelection::Index( int row, int col ) const
{
    int n, count = m_blocks.GetCount();
    if ((count == 0) || !m_bounds.Contains(row, col))
        return wxNOT_FOUND;
    
    for (n=FindTopRow(row); n<count; n++) 
    {
        if ( m_blocks[n].Contains(row, col) )
            return n;
        if (row < m_blocks[n].GetTop())
            return wxNOT_FOUND;
    }
    return wxNOT_FOUND;
}

int wxSheetSelection::Index( const wxSheetBlock& block ) const
{
    int n, count = m_blocks.GetCount();
    const int bottom_row = block.GetBottom();
    if ((count == 0) || !m_bounds.Contains(block))
        return wxNOT_FOUND;
    
    for (n=FindTopRow(bottom_row); n<count; n++) 
    {
        if (m_blocks[n].Contains(block)) 
            return n;
        if (bottom_row < m_blocks[n].GetTop())
            return wxNOT_FOUND;
    }
    return wxNOT_FOUND;
}

int wxSheetSelection::FindInsertIndex(const wxSheetBlock& block) const
{
    size_t n, lo = 0, hi = m_blocks.GetCount(); 
    if ((hi == 0) || (block < m_blocks[0])) return 0;     
    if (block > m_blocks[hi-1])             return hi; 

    while ( lo < hi ) 
    { 
        n = (lo + hi)/2;              
        const wxSheetBlock& tmp = m_blocks[n];
        if (tmp == block) return n;     
        if (tmp  > block) hi = n;       
        else              lo = n + 1;   
    } 
    return lo; 
} 
int wxSheetSelection::FindTopRow(int row) const
{
    size_t n, lo = 0, hi = m_blocks.GetCount(); 
    if ((hi == 0) || (row >= m_blocks[0].GetTop())) return 0;  
    if (row < m_blocks[hi-1u].GetTop())             return hi;

    while ( lo < hi ) 
    { 
        n = (lo + hi)/2;              
        const int tmp_row = m_blocks[n].GetTop();
        if (tmp_row > row) hi = n;       
        else               lo = n + 1;   
    } 
    return lo; 
}

int wxSheetSelection::IndexIntersects( const wxSheetBlock& block ) const
{
    int n, count = m_blocks.GetCount();
    const int bottom_row = block.GetBottom();
    if ((count == 0) || !m_bounds.Intersects(block))
        return wxNOT_FOUND;
    
    for (n=FindTopRow(bottom_row); n<count; n++) 
    {
        if (m_blocks[n].Intersects(block)) 
            return n;
        if (bottom_row < m_blocks[n].GetTop())
            return wxNOT_FOUND;
    }
    return wxNOT_FOUND;
}

bool wxSheetSelection::DeselectBlock( const wxSheetBlock& block, bool combineNow,
                                      wxArraySheetBlock *deletedBlocks )
{
    int n, count = m_blocks.GetCount();
    if ((count == 0) || !m_bounds.Intersects(block))
        return false;    

    bool done = false;
    bool recalc_bounds = false;
    const int bottom_row = block.GetBottom();
    
    if (deletedBlocks)
        deletedBlocks->Clear();
    
    wxArraySheetBlock extraBlocks;
    wxSheetBlock top, bottom, left, right;
    // iterate though blocks cutting input block out and adding remainder to end
    for (n=FindTopRow(bottom_row); n<count; n++)
    {
        if (bottom_row < m_blocks[n].GetTop())
            break;
        
        const int deleted = m_blocks[n].Delete(block, top, bottom, left, right);
        if (deleted != wxSHEET_BLOCK_NONE)
        {
            done = true;
            int last_n = n;

            if (deletedBlocks) 
                deletedBlocks->Add(m_blocks[n].Intersect(block));
            if (m_bounds.SideMatches(m_blocks[n]) != wxSHEET_BLOCK_NONE)
                recalc_bounds = true;
            
            if (m_blocks[n].Contains(block))
                n = count + 100; // all done, but add cutouts back
            else
                n--;

            m_blocks.RemoveAt(last_n);
            count--;

            if (deleted != wxSHEET_BLOCK_ALL)
            {
                if ((deleted & wxSHEET_BLOCK_TOP)    != 0) 
                    extraBlocks.Add(top);
                if ((deleted & wxSHEET_BLOCK_BOTTOM) != 0) 
                    extraBlocks.Add(bottom);
                if ((deleted & wxSHEET_BLOCK_LEFT)   != 0)   
                    extraBlocks.Add(left);
                if ((deleted & wxSHEET_BLOCK_RIGHT)  != 0)  
                    extraBlocks.Add(right);
            }
        }
    }

    if (done)
    {
        m_minimized = false;
        const int extra_count = extraBlocks.GetCount();
        for (n=0; n<extra_count; n++)
            InsertBlock(extraBlocks.Item(n));        
        if (combineNow) 
            Minimize();
        if (recalc_bounds)
            CalculateBounds();
    }
    
    return done;
}

bool wxSheetSelection::SelectBlock( const wxSheetBlock &block, bool combineNow, 
                                    wxArraySheetBlock *addedBlocks )
{
    if (block.IsEmpty())
        return false;    

    wxArraySheetBlock extraBlocks;
    wxArraySheetBlock *extra = &extraBlocks;
    if (addedBlocks)
    {
        extra = addedBlocks;
        extra->Clear();
    }

    extra->Add(block);
    int extra_count = 1;

    int n, k, count = m_blocks.GetCount();
    if (((m_options & wxSHEET_SELECTION_MULTIPLE_SEL) == 0) && (count > 0) && 
        m_bounds.Intersects(block))
    {
        const int bottom_row = block.GetBottom();
        wxSheetBlock top, bottom, left, right;   

        // interate though blocks breaking up input block if it's already selected
        for (n=FindTopRow(bottom_row); n<count; n++)
        {
            if (bottom_row < m_blocks[n].GetTop())
                break;

            for (k=0; k<extra_count; k++)
            {
                const int combined = m_blocks[n].Combine(extra->Item(k), top, bottom, left, right);
                if (combined != wxSHEET_BLOCK_NONE)
                {
                    extra->RemoveAt(k);
                    extra_count--;
                    k--;

                    if (combined != wxSHEET_BLOCK_ALL)
                    {
                        if ((combined & wxSHEET_BLOCK_TOP)    != 0) 
                            { extra->Add(top);    extra_count++; }
                        if ((combined & wxSHEET_BLOCK_BOTTOM) != 0) 
                            { extra->Add(bottom); extra_count++; }
                        if ((combined & wxSHEET_BLOCK_LEFT)   != 0)   
                            { extra->Add(left);   extra_count++; }
                        if ((combined & wxSHEET_BLOCK_RIGHT)  != 0)  
                            { extra->Add(right);  extra_count++; }
                    }

                    if (extra_count == 0)
                    {
                        n = count;
                        break;
                    }
                }
            }
        }
    }
    
#ifdef CHECK_BLOCK_SORTING
    for (int m=1; m<m_blocks.GetCount(); m++)
        if (m_blocks[m] < m_blocks[m-1]) PRINT_BLOCK("Not sorted", m_blocks[m]);
#endif // CHECK_BLOCK_SORTING    
    
    if (extra_count != 0)
    {
        m_minimized = false;
        for (n=0; n<extra_count; n++)
            InsertBlock(extra->Item(n));
        if (combineNow)
            Minimize();
        
        return true;
    }
    
    return false;
}

int wxSheetSelection::InsertBlock(const wxSheetBlock& block)
{
    const int index = FindInsertIndex(block);
    m_blocks.Insert(block, index);
    m_bounds = m_bounds.ExpandUnion(block);
    return index;
}

void wxSheetSelection::CalculateBounds()
{
    int n, count = m_blocks.GetCount();
    if (count == 0)
    {
        m_bounds = wxNullSheetBlock;
        return;
    }
    
    m_bounds = m_blocks[0];
    
    for (n=1; n<count; n++)
        m_bounds = m_bounds.Union(m_blocks[n]);
}

bool wxSheetSelection::Minimize()
{
    m_minimized = true;
    if (m_blocks.GetCount() < 2u)
        return false;
    
    return DoMinimize(m_blocks);
}

bool wxSheetSelection::DoMinimize(wxArraySheetBlock &blocks) const
{
    int n, count = GetCount() > 0 ? 1000 : 0;
    for (n=0; n<count; n++) // usually just takes a few
    {
        if (!DoDoMinimize(blocks)) break;
    }

    // sanity check - this shouldn't and hasn't happened
    wxCHECK_MSG(!count || (n <= count), true, wxT("Unable to minimize wxSheetSelection"));
    
#ifdef CHECK_BLOCK_OVERLAP
    for (size_t a=0; a<blocks.GetCount(); a++)
    {
        for (size_t b=a+1; b<blocks.GetCount(); b++)
        {
            if (blocks[a].Intersects(blocks[b]))
            {
                wxPrintf(wxT("Intersecting blocks in wxBlockIntSelection::DoMinimize\n")); fflush(stdout);
                PRINT_BLOCK(wxT("First  block "), blocks[a]);
                PRINT_BLOCK(wxT("Second block "), blocks[b]);
                wxBell();
            }
        }
    }
#endif    
    
    return n != 0;    
}

bool wxSheetSelection::DoDoMinimize(wxArraySheetBlock &blocks) const
{
    bool done = false;
    int count = blocks.GetCount();
    for (int i=0; i<count-1; i++)
    {
        for (int j=i+1; j<count; j++)
        {
            if (blocks[i].Combine(blocks[j]))
            {
                blocks.RemoveAt(j);
                j--;
                count--;
                done = true;
            }
        }
    }
    return done;
}

bool wxSheetSelection::UpdateRows( size_t row, int numRows )
{
    if (numRows == 0) return false; 
    bool done = false;
    int n, count = GetCount();

    for (n = 0; n < count; n++)
    {
        wxSheetBlock &b = m_blocks[n];
        done |= b.UpdateRows(row, numRows);
        if (b.IsEmpty())
        {
            m_blocks.RemoveAt(n); 
            count--; 
            n--; 
        }
    }        

    CalculateBounds();
    return done;
}
bool wxSheetSelection::UpdateCols( size_t col, int numCols )
{
    if (numCols == 0) return false; 
    bool done = false;
    int n, count = GetCount();
    
    for (n = 0; n < count; n++)
    {
        wxSheetBlock &b = m_blocks[n];
        done |= b.UpdateCols(col, numCols);
        if (b.IsEmpty())
        {
            m_blocks.RemoveAt(n); 
            count--; 
            n--; 
        }
    }        

    CalculateBounds();
    return done;    
}

// ----------------------------------------------------------------------------
// wxSheetSelectionIterator - iterates through a wxSheetSelection
// ----------------------------------------------------------------------------

wxSheetSelectionIterator::wxSheetSelectionIterator( const wxSheetSelection &sel,
                                                    wxSheetSelectionIter_Type type)
                                                          
{
    m_type = -1;
    WX_APPEND_ARRAY(m_blocks, sel.GetBlockArray());
    Reset(type);
}

wxSheetSelectionIterator::wxSheetSelectionIterator( const wxArraySheetBlock &blocks,
                                                    wxSheetSelectionIter_Type type)
                                                          
{
    m_type = -1;
    WX_APPEND_ARRAY(m_blocks, blocks);    
    Reset(type);
}

void wxSheetSelectionIterator::Reset(wxSheetSelectionIter_Type type)
{    
    m_block_index = -1;      

    if (m_type != type)
    {
        m_type = type;
        
        if (type == wxSSI_REVERSE)
            m_blocks.Sort(wxsheetcellblock_sort_bottomright_topleft);
        else
            m_blocks.Sort(wxsheetcellblock_sort_topleft_bottomright);
    }
}

wxSheetSelectionIterGet_Type wxSheetSelectionIterator::GetNext(wxSheetCoords &coords)
{
    if (m_type == wxSSI_REVERSE)
        return GetNextReverse(coords);
    
    return GetNextForward(coords);
}

wxSheetSelectionIterGet_Type wxSheetSelectionIterator::GetNextForward(wxSheetCoords &coord)
{
    if ((m_blocks.GetCount() < 1u) || (m_block_index >= int(m_blocks.GetCount())))
        return wxSHEET_SELECTIONITER_GET_END;
    
    // first time here
    if (m_block_index < 0)
    {
        m_block_index = 0;
        coord = m_coords = m_blocks[m_block_index].GetLeftTop();
        return wxSHEET_SELECTIONITER_GET_LEFTTOP;
    }

    // at end of block swap to new one
    if (m_coords == m_blocks[m_block_index].GetRightBottom())
    {
        ++m_block_index;
        if (m_block_index < int(m_blocks.GetCount()))
        {
            coord = m_coords = m_blocks[m_block_index].GetLeftTop();
            return wxSHEET_SELECTIONITER_GET_LEFTTOP;
        }
        else  // past end nothing more to check
            return  wxSHEET_SELECTIONITER_GET_END;
    }
    // at end of col, down to next row
    if (m_coords.GetCol() == m_blocks[m_block_index].GetRight())
    {
        m_coords.SetCol(m_blocks[m_block_index].GetLeft());
        m_coords.m_row++;
        
        coord = m_coords;
        return (coord == m_blocks[m_block_index].GetRightBottom()) ?
            wxSHEET_SELECTIONITER_GET_RIGHTBOTTOM : wxSHEET_SELECTIONITER_GET_NEXTROW;
    }
    
    // increment the col
    m_coords.m_col++;
    coord = m_coords;
    
    return wxSHEET_SELECTIONITER_GET_NEXTCOL;
}

wxSheetSelectionIterGet_Type wxSheetSelectionIterator::GetNextReverse(wxSheetCoords &coord)
{
    if ((m_blocks.GetCount() < 1u) || (m_block_index >= int(m_blocks.GetCount())))
        return wxSHEET_SELECTIONITER_GET_END;
    
    // first time here
    if (m_block_index < 0)
    {
        m_block_index = 0;
        coord = m_coords = m_blocks[m_block_index].GetRightBottom();
        return wxSHEET_SELECTIONITER_GET_RIGHTBOTTOM;
    }

    // at end of block swap to new one
    if (m_coords == m_blocks[m_block_index].GetLeftTop())
    {
        ++m_block_index;
        if (m_block_index < int(m_blocks.GetCount()))
        {
            coord = m_coords = m_blocks[m_block_index].GetRightBottom();
            return wxSHEET_SELECTIONITER_GET_RIGHTBOTTOM;
        }
        else  // past end nothing more to check
            return  wxSHEET_SELECTIONITER_GET_END;
    }
    
    // at left col, up to next row
    if (m_coords.GetCol() == m_blocks[m_block_index].GetLeft())
    {
        m_coords.SetCol(m_blocks[m_block_index].GetRight());
        m_coords.m_row--;
        
        coord = m_coords;
        return (coord == m_blocks[m_block_index].GetLeftTop()) ?
            wxSHEET_SELECTIONITER_GET_LEFTTOP : wxSHEET_SELECTIONITER_GET_NEXTROW;
    }    
    // increment the col
    m_coords.m_col--;
    
    coord = m_coords;    
    return wxSHEET_SELECTIONITER_GET_NEXTCOL;
}

bool wxSheetSelectionIterator::IsInSelection(int row, int col) const
{
    int n, count = m_blocks.GetCount();
    for (n=0; n<count; n++) 
    {
        if (m_blocks[n].Contains(row, col)) 
            return true;
    }
    return false;
}


// ============================================================================
// wxSheetVariant
// ============================================================================
#include "wx/variant.h"

class WXDLLIMPEXP_SHEET wxSheetVariant
{
public:
    wxSheetVariant( const wxSheetVariant& sheetVariant ) : m_data(NULL)
    {
        Copy(sheetVariant);
    }
    wxSheetVariant( const wxVariant& variant ) : m_data(NULL)
    {
        Copy(variant);
    }
    wxSheetVariant( const wxVariantData& data ) : m_data(NULL)
    {
        Copy(data);
    }
    wxSheetVariant( const wxVariantData* data ) : m_data(NULL)
    {
        Copy(data);
    }

#if wxCHECK_VERSION(2,7,0)
    virtual ~wxSheetVariant() { m_data->DecRef(); }
#else
    virtual ~wxSheetVariant() { delete m_data; }
#endif

    void Copy(const wxSheetVariant& other);
    void Copy(const wxVariant& other);
    void Copy(const wxVariantData& other);
    void Copy(const wxVariantData* other);
    
    wxVariant GetwxVariant() const { return wxVariant(m_data); }
    wxVariantData* GetData() const { return m_data; }
    
    wxSheetVariant& operator=(const wxSheetVariant& other) { Copy(other); return *this; }
    wxSheetVariant& operator=(const wxVariant& other) { Copy(other); return *this; }
    wxSheetVariant& operator=(const wxVariantData& other) { Copy(other); return *this; }
    
    wxVariantData* m_data;    
};


void wxSheetVariant::Copy(const wxSheetVariant& other)
{
    Copy(other.GetData());
}
void wxSheetVariant::Copy(const wxVariant& other)
{
    Copy(other.GetData());
}
void wxSheetVariant::Copy(const wxVariantData& other)
{
#if wxCHECK_VERSION(2,7,0)
    m_data->DecRef();
    m_data = const_cast<wxVariantData *>(&other);
    m_data->IncRef();
#else
    delete m_data;
    m_data = (wxVariantData*) other.GetClassInfo()->CreateObject();
    m_data->Copy(*(wxVariantData*)&other);
#endif 

}

void wxSheetVariant::Copy(const wxVariantData* other)
{
#if wxCHECK_VERSION(2,7,0)
    m_data->DecRef();
#else
    delete m_data;
#endif
    
    if (other)
        Copy(*other);
    else
        m_data = NULL;
}


WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetVariant, wxArraySheetVariant, class WXDLLIMPEXP_SHEET);
#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArraySheetVariant)

class WXDLLIMPEXP_SHEET wxSheetRowColVariantContainer
{
public:
    wxSheetRowColVariantContainer(bool has_data = false);

    

    // Update the number of rows/cols. In numRows/Cols > 0 insert them else
    //  remove them.
    bool UpdateRows( size_t row, int numRows );
    bool UpdateCols( size_t col, int numCols );

protected:
    wxArraySheetVariant *m_data;    
};

wxSheetRowColVariantContainer::wxSheetRowColVariantContainer(bool has_data) : m_data(NULL)
{
    if (has_data)
        m_data = new wxArraySheetVariant;
}    

bool wxSheetRowColVariantContainer::UpdateRows( size_t row, int numRows )
{ 
    return false; 
}
bool wxSheetRowColVariantContainer::UpdateCols( size_t col, int numCols ) 
{ 
    return false; 
}
