///////////////////////////////////////////////////////////////////////////
// Name:        sheetsel.cpp
// Purpose:     wxSheetSelection
// Author:      John Labenski
// Modified by: 
// Created:     20/02/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Stefan Neis (Stefan.Neis@t-online.de)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheetsel.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/defs.h"
#if wxUSE_GRID

#include "wx/utils.h"         // for wxMin and wxMax
#include "wx/gdicmn.h"        // for wxRect
#include "sheet.h"
#include "sheetsel.h"

// if set then 
#define CHECK_BLOCK_OVERLAP 1

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArraySheetCoords)
WX_DEFINE_OBJARRAY(wxArraySheetBlock)

const wxSheetCoords wxNullSheetCoords( -2, -2 );
const wxSheetCoords wxGridCellSheetCoords( 0, 0 );
const wxSheetCoords wxRowLabelSheetCoords( 0, -1 );
const wxSheetCoords wxColLabelSheetCoords( -1, 0 );
const wxSheetCoords wxCornerLabelSheetCoords( -1, -1 );
const wxSheetBlock  wxNullSheetBlock ( 0, 0, 0, 0 );
const wxRect        wxSheetNoCellRect( 0, 0, 0, 0 );

static int wxCMPFUNC_CONV wxsheetcellblock_sort_topleft_bottomright( wxSheetBlock **a, wxSheetBlock **b)
{ 
    int row = ((*a)->m_row - (*b)->m_row);
    
    if (row < 0) return -1;
    if (row == 0) return  ((*a)->m_col - (*b)->m_col);
    return 1;
}
static int wxCMPFUNC_CONV wxblockint_sort_bottomright_topleft( wxSheetBlock **a, wxSheetBlock **b)
{ 
/*    
    int row = ((*b)->GetBottom() - (*a)->GetBottom());
    
    if (row < 0) return -1;
    if (row == 0) return  ((*b)->GetRight() - (*a)->GetRight());
    return 1;
*/    
    int col = ((*b)->GetRight() - (*a)->GetRight());
    
    if (col < 0) return -1;
    if (col == 0) return  ((*b)->GetBottom() - (*a)->GetBottom());
    return 1;    
}

// ----------------------------------------------------------------------------
// wxSheetBlock: a rectangular block of cells
// ----------------------------------------------------------------------------

wxSheetBlock::wxSheetBlock( const wxSheetCoords& coords1, const wxSheetCoords& coords2 )
{
    // block is always upright with coords at corners
    m_row = coords1.m_row;
    m_col = coords1.m_col;
    m_height = coords2.m_row - coords1.m_row;
    m_width  = coords2.m_col - coords1.m_col;

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

wxArraySheetCoords wxSheetBlock::GetArrayCoords() const
{
    wxArraySheetCoords arrCoords;
    if (IsEmpty())
        return arrCoords;
    
    arrCoords.Alloc(GetWidth()*GetHeight());
    int bottom = GetBottom(), right = GetRight();
    wxSheetCoords coords;
    
    for (coords.m_row = m_row; coords.m_row <= bottom; coords.m_row++)
    {
        for (coords.m_col = m_col; coords.m_col <= right; coords.m_col++)
            arrCoords.Add(coords);
    }
    
    return arrCoords;
}

bool wxSheetBlock::Combine(const wxSheetBlock &block)
{ 
    if (IsEmpty() || block.IsEmpty()) return FALSE;
    if (!Touches(block)) return FALSE;
    if (Contains(block)) return TRUE;
    if (block.Contains(*this))
    {
        *this = block;
        return TRUE;
    }
    
    wxSheetBlock uBlock(Union(block));
    if (uBlock.IsEmpty()) return FALSE;

    // ugh this is really ugly, I can't figure a better way though
    
/*    
    // at least one of the two blocks has to be at each corner of the union
    if (((uBlock.GetLeftTop() == GetLeftTop()) || (uBlock.GetLeftTop() == block.GetLeftTop())) &&
        ((uBlock.GetRightTop() == GetRightTop()) || (uBlock.GetRightTop() == block.GetRightTop())) &&
        ((uBlock.GetLeftBottom() == GetLeftBottom()) || (uBlock.GetLeftBottom() == block.GetLeftBottom())) &&
        ((uBlock.GetRightBottom() == GetRightBottom()) || (uBlock.GetRightBottom() == block.GetRightBottom())) )
*/
    
    int ub_t = uBlock.GetTop();
    int ub_b = uBlock.GetBottom();
    int ub_l = uBlock.GetLeft();
    int ub_r = uBlock.GetRight();

    int b_t = block.GetTop();
    int b_b = block.GetBottom();
    int b_l = block.GetLeft();
    int b_r = block.GetRight();

    int t = GetTop();
    int b = GetBottom();
    int l = GetLeft();
    int r = GetRight();
    
    if ( ( ((ub_t==t)&&(ub_l==l)) || ((ub_t==b_t)&&(ub_l==b_l)) ) &&
         ( ((ub_t==t)&&(ub_r==r)) || ((ub_t==b_t)&&(ub_r==b_r)) ) &&
         ( ((ub_b==b)&&(ub_l==l)) || ((ub_b==b_b)&&(ub_l==b_l)) ) &&
         ( ((ub_b==b)&&(ub_r==r)) || ((ub_b==b_b)&&(ub_r==b_r)) ) )
    {
        *this = uBlock;
        return TRUE;
    }

    return FALSE;
}

bool wxSheetBlock::Combine( const wxSheetBlock &block, 
                            wxSheetBlock &top, wxSheetBlock &bottom, 
                            wxSheetBlock &left, wxSheetBlock &right ) const
{
    top = bottom = left = right = wxNullSheetBlock;
    
    if (IsEmpty() || block.IsEmpty())
        return FALSE;

    if (Contains(block)) return TRUE;   // can combine all of block, no leftover
    
    wxSheetBlock iBlock(Intersect(block));
    if (iBlock.IsEmpty()) return FALSE; // nothing to combine

    bool combined = FALSE;
    
    if ( block.GetTop() < GetTop() )
    {
        top.SetCoords( block.GetTop(), block.GetLeft(), GetTop()-1, block.GetRight() );
        combined = TRUE;
    }
    if ( block.GetBottom() > GetBottom() )
    {
        bottom.SetCoords( GetBottom()+1, block.GetLeft(), block.GetBottom(), block.GetRight() );
        combined = TRUE;
    }
    if ( block.GetLeft() < GetLeft() )
    {
        left.SetCoords(iBlock.GetTop(), block.GetLeft(), iBlock.GetBottom(), GetLeft()-1 );
        combined = TRUE;
    }
    if ( block.GetRight() > GetRight() )
    {
        right.SetCoords( iBlock.GetTop(), GetRight()+1, iBlock.GetBottom(), block.GetRight() );
        combined = TRUE;
    }

    return combined;
}


bool wxSheetBlock::Delete( const wxSheetBlock &block, 
                           wxSheetBlock &top, wxSheetBlock &bottom, 
                           wxSheetBlock &left, wxSheetBlock &right ) const
{
    top = bottom = left = right = wxNullSheetBlock;
    
    if (IsEmpty() || block.IsEmpty())
        return FALSE;

    if (block.Contains(*this)) return TRUE; // can delete all of this, no leftover
    
    wxSheetBlock iBlock(Intersect(block));    
    if (iBlock.IsEmpty()) return FALSE; // nothing to delete

    bool deleted = FALSE;
    
    if ( GetTop() < iBlock.GetTop() )
    {
        top.SetCoords( GetTop(), GetLeft(), iBlock.GetTop()-1, GetRight() );
        deleted = TRUE;
    }
    if ( GetBottom() > iBlock.GetBottom() )
    {
        bottom.SetCoords( iBlock.GetBottom()+1, GetLeft(), GetBottom(), GetRight() );
        deleted = TRUE;
    }
    if ( GetLeft() < iBlock.GetLeft() )
    {
        left.SetCoords( iBlock.GetTop(), GetLeft(), iBlock.GetBottom(), iBlock.GetLeft()-1 );
        deleted = TRUE;
    }
    if ( GetRight() > iBlock.GetRight() )
    {
        right.SetCoords( iBlock.GetTop(), iBlock.GetRight()+1, iBlock.GetBottom(), GetRight() );
        deleted = TRUE;
    }
    
    return deleted;
}

// ----------------------------------------------------------------------------
// wxSheetSelection
// ----------------------------------------------------------------------------

wxSheetSelection::wxSheetSelection() : m_minimized(TRUE), m_sorted(TRUE)
{
}

wxSheetSelection::wxSheetSelection( const wxSheetBlock& block ) 
                 : m_minimized(TRUE), m_sorted(TRUE)
{ 
    m_blocks.Add(block); 
}
    
void wxSheetSelection::Copy(const wxSheetSelection &source)
{
    m_blocks.Clear();
    WX_APPEND_ARRAY(m_blocks, source.GetBlockArray());
    m_minimized = source.IsMinimzed();
    m_sorted = source.IsSorted();
}

const wxSheetBlock& wxSheetSelection::GetBlock( size_t index ) const
{
    wxCHECK_MSG((index>=0) && (index<m_blocks.GetCount()), 
                wxNullSheetBlock, wxT("Invalid index"));
    return m_blocks[index];
}

wxSheetBlock wxSheetSelection::GetBoundingBlock() const
{
    int n, count = m_blocks.GetCount();
    if (count == 0) 
        return wxNullSheetBlock;
    
    wxSheetBlock bound(m_blocks[0]);
    
    for (n=1; n<count; n++)
        bound = bound.Union(m_blocks[n]);
    
    return bound;
}

void wxSheetSelection::SetBoundingBlock(const wxSheetBlock& block)
{
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
    }    
}

bool wxSheetSelection::Contains( const wxSheetBlock &b ) const
{
    // FIXME - not tested, but should work    
    int n, count = m_blocks.GetCount();
    if ((count == 0) || b.IsEmpty())
        return FALSE;
    if (m_sorted && (b.GetBottom() < m_blocks[0].m_row))
        return FALSE;
    
    wxArraySheetBlock extra;
    extra.Add(b);

    wxSheetBlock top, bottom, left, right;
    // iterate though blocks cutting input block out and adding remainder to end
    for (n=0; n<count; n++)
    {
        for (int k=0; k<int(extra.GetCount()); k++)
        {
            if (extra[k].Delete(m_blocks[n], top, bottom, left, right))
            {
                extra.RemoveAt(k);
                k--;

                if (!top.IsEmpty())    extra.Add(top);
                if (!bottom.IsEmpty()) extra.Add(bottom);
                if (!left.IsEmpty())   extra.Add(left);
                if (!right.IsEmpty())  extra.Add(right);
                if (extra.GetCount() == 0u)
                    return TRUE;
            }
        }
    }
    
    return extra.GetCount() == 0u;    
}

int wxSheetSelection::Index( int row, int col ) const
{
    int n, count = m_blocks.GetCount();
    if (count == 0)
        return wxNOT_FOUND;
    if (m_sorted && (row < m_blocks[0].m_row))
        return wxNOT_FOUND;
    
    for (n=0; n<count; n++) 
    {
        if ( m_blocks[n].Contains(row, col) )
            return n;
    }
    return wxNOT_FOUND;
}

int wxSheetSelection::Index( const wxSheetBlock &b ) const
{
    int n, count = m_blocks.GetCount();
    if ((count == 0) || b.IsEmpty())
        return wxNOT_FOUND;
    if (m_sorted && (b.GetBottom() < m_blocks[0].m_row))
        return wxNOT_FOUND;
    
    for (n=0; n<count; n++) 
    {
        if (m_blocks[n].Contains(b)) 
            return n;
    }
    return wxNOT_FOUND;
}

int wxSheetSelection::IndexIntersects( const wxSheetBlock &b ) const
{
    int n, count = m_blocks.GetCount();
    if ((count == 0) || b.IsEmpty())
        return wxNOT_FOUND;
    if (m_sorted && (b.GetBottom() < m_blocks[0].m_row))
        return wxNOT_FOUND;
    
    for (n=0; n<count; n++) 
    {
        if (m_blocks[n].Intersects(b)) 
            return n;
    }
    return wxNOT_FOUND;
}

void wxSheetSelection::Sort()
{
    m_sorted = TRUE;
    if (m_blocks.GetCount() > 1u)
        m_blocks.Sort(wxsheetcellblock_sort_topleft_bottomright);
}

bool wxSheetSelection::DeselectBlock( const wxSheetBlock &block, bool combineNow,
                                      wxArraySheetBlock *deletedBlocks )
{
    int count = m_blocks.GetCount();
    if ((count == 0) || block.IsEmpty())
        return FALSE;    
    if (m_sorted && (block.GetBottom() < m_blocks[0].m_row))
        return FALSE;

    bool done = FALSE;
    
    if (deletedBlocks)
        deletedBlocks->Clear();
    
    wxSheetBlock top, bottom, left, right;
    // iterate though blocks cutting input block out and adding remainder to end
    for (int n=0; n<count; n++)
    {
        if (m_blocks[n].Delete(block, top, bottom, left, right))
        {
            done = TRUE;
            int last_n = n;

            if (m_blocks[n].Contains(block))
                n = m_blocks.GetCount() + 100; // all done, but add cutouts back
            else
                n--;

            m_blocks.RemoveAt(last_n);
            count--;

            if (!top.IsEmpty())    m_blocks.Add(top);
            if (!bottom.IsEmpty()) m_blocks.Add(bottom);
            if (!left.IsEmpty())   m_blocks.Add(left);
            if (!right.IsEmpty())  m_blocks.Add(right);

            // if (deletedBlocks) FIXME only add what was deleted
            // but deleting is a LOT more work than adding, probably too much
        }
    }

    if (done)
    {
        m_minimized = FALSE;
        m_sorted = FALSE;
        
        if (deletedBlocks)
            deletedBlocks->Add(block);
    
        if (combineNow) 
            Minimize();
    }
    
    return done;
}

bool wxSheetSelection::SelectBlock( const wxSheetBlock &block, bool combineNow, 
                                    wxArraySheetBlock *addedBlocks )
{
    if (block.IsEmpty())
        return FALSE;    

    wxArraySheetBlock extraBlocks;
    wxArraySheetBlock *extra = &extraBlocks;
    if (addedBlocks)
    {
        extra = addedBlocks;
        extra->Empty();
    }
    
    extra->Add(block);

    int count = m_blocks.GetCount();
    if (!m_sorted || ((count > 0) && (block.GetBottom() >= m_blocks[0].m_row)))
    {
        wxSheetBlock top, bottom, left, right;   
    
        // interate though blocks breaking up input block if it's already selected
        for (int n=0; n<count; n++)
        {
            for (int k=0; k<int(extra->GetCount()); k++)
            {
                if (m_blocks[n].Combine(extra->Item(k), top, bottom, left, right))
                {
                    extra->RemoveAt(k);
                    k--;
                    
                    if (!top.IsEmpty())    extra->Add(top); 
                    if (!bottom.IsEmpty()) extra->Add(bottom);
                    if (!left.IsEmpty())   extra->Add(left);
                    if (!right.IsEmpty())  extra->Add(right);

                    if (extra->GetCount() == 0u)
                    {
                        n = count;
                        break;
                    }
                }
            }
        }
    }
    
    if (extra->GetCount() > 0u)
    {
        WX_APPEND_ARRAY(m_blocks, *extra);
        m_minimized = FALSE;
        m_sorted = FALSE;
        
        if (combineNow)
            Minimize();
        
        return TRUE;
    }
    
    return FALSE;
}

bool wxSheetSelection::Minimize()
{
    m_minimized = TRUE;
    if (m_blocks.GetCount() < 2u)
    {
        m_sorted = TRUE;
        return FALSE;
    }
    
    bool ret = DoMinimize(m_blocks);
    Sort();
    return ret;
}

bool wxSheetSelection::DoMinimize(wxArraySheetBlock &blocks) const
{
    int n;
    for (n=0; n<10000; n++) // usually just takes a few
    {
        if (!DoDoMinimize(blocks)) break;
    }

#ifdef CHECK_BLOCK_OVERLAP
    for (size_t a=0; a<blocks.GetCount(); a++)
    {
        for (size_t b=a+1; b<blocks.GetCount(); b++)
        {
            if (blocks[a].Intersects(blocks[b]))
            {
                printf("Intersecting blocks in wxBlockIntSelection::DoMinimize\n"); fflush(stdout);
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
    bool done = FALSE;
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
                done = TRUE;
            }
        }
    }
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
            m_blocks.Sort(wxblockint_sort_bottomright_topleft);
        else
            m_blocks.Sort(wxsheetcellblock_sort_topleft_bottomright);
    }
}

bool wxSheetSelectionIterator::GetNext(wxSheetCoords &coords)
{
    if (m_type == wxSSI_REVERSE)
        return GetNextReverse(coords);
    
    return GetNextForward(coords);
}

bool wxSheetSelectionIterator::GetNextForward(wxSheetCoords &coord)
{
    if ((m_blocks.GetCount() < 1u) || (m_block_index >= int(m_blocks.GetCount())))
        return FALSE;
    
    // first time here
    if (m_block_index < 0)
    {
        m_block_index = 0;
        coord = m_coords = m_blocks[m_block_index].GetLeftTop();
        return TRUE;
    }

    // at end of block swap to new one
    if (m_coords == m_blocks[m_block_index].GetRightBottom())
    {
        ++m_block_index;
        if (m_block_index < int(m_blocks.GetCount()))
        {
            coord = m_coords = m_blocks[m_block_index].GetLeftTop();
            return TRUE;
        }
        else  // past end nothing more to check
            return  FALSE;
    }
    // at end of col, down to next row
    if (m_coords.GetCol() == m_blocks[m_block_index].GetRight())
    {
        m_coords.SetCol(m_blocks[m_block_index].GetLeft());
        m_coords.m_row++;
        
        coord = m_coords;
        return TRUE;
    }
    
    // increment the col
    m_coords.m_col++;
    coord = m_coords;
    
    return TRUE;
}

bool wxSheetSelectionIterator::GetNextReverse(wxSheetCoords &coord)
{
    if ((m_blocks.GetCount() < 1u) || (m_block_index >= int(m_blocks.GetCount())))
        return FALSE;
    
    // first time here
    if (m_block_index < 0)
    {
        m_block_index = 0;
        coord = m_coords = m_blocks[m_block_index].GetRightBottom();
        return TRUE;
    }

    // at end of block swap to new one
    if (m_coords == m_blocks[m_block_index].GetLeftTop())
    {
        ++m_block_index;
        if (m_block_index < int(m_blocks.GetCount()))
        {
            coord = m_coords = m_blocks[m_block_index].GetRightBottom();
            return TRUE;
        }
        else  // past end nothing more to check
            return  FALSE;
    }
    // at top of row, left to next col
    if (m_coords.GetRow() == m_blocks[m_block_index].GetTop())
    {
        m_coords.SetRow(m_blocks[m_block_index].GetBottom());
        m_coords.m_col--;
        
        coord = m_coords;
        return TRUE;
    }
    
    // increment the row
    m_coords.m_row--;
    coord = m_coords;
    
    return TRUE;
}

bool wxSheetSelectionIterator::IsInSelection(int row, int col) const
{
    int n, count = m_blocks.GetCount();
    for (n=0; n<count; n++) 
    {
        if (m_blocks[n].Contains(row, col)) 
            return TRUE;
    }
    return FALSE;
}

#endif // wxUSE_GRID
