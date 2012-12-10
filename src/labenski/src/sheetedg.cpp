///////////////////////////////////////////////////////////////////////////////
// Name:        sheetedg.cpp
// Purpose:     wxSheetArrayEdge
// Author:      John Labenski
// Modified by: 
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
#endif // WX_PRECOMP

#include "wx/sheet/sheet.h"
#include "wx/sheet/sheetedg.h"

#include "wx/arrimpl.cpp"
DEFINE_PAIRED_INT_DATA_ARRAYS(int, wxPairArrayIntInt)

// ----------------------------------------------------------------------------
// wxSheetArrayEdge - a wxArrayInt container that sums its values
// ----------------------------------------------------------------------------
wxSheetArrayEdge::wxSheetArrayEdge(size_t count, size_t default_size, 
                                   size_t min_allowed_size)
                 : m_count(count), m_default_size(default_size), 
                   m_min_allowed_size(min_allowed_size) 
{}

int wxSheetArrayEdge::FindIndex(int coord, bool clipToMinMax) const
{
    //wxCHECK_MSG(m_count != 0, -1, wxT("No edges to find index of"));
    if (m_count < 0) return -1;
        
    // < 0 is never valid, check for clip
    if (coord < 0)
        return clipToMinMax ? 0 : -1;

    // need default size to at least be 1
    const int default_size = (m_default_size > 0) ? m_default_size : 1;    
    const int count = m_data.GetCount();
    int i_max = coord / default_size;
    int i_min = 0;

    // if no edges in array then they're all of the default size
    if (count == 0)
        return (i_max < m_count) ? i_max : (clipToMinMax ? m_count-1 : -1);

    // quick checks for past end and in first element
    if ( coord > m_data[count-1] )
        return clipToMinMax ? (count-1) : -1;
    if ( coord <= m_data[0] )
        return 0;
    
    // reset the values of max and min if necessary and trim range down
    if ( i_max >= count )
        i_max = count - 1;
    if ( coord > m_data[i_max] )
    {
        i_min = i_max;
        i_max = count - 1;
    }

    int median;
    while ( i_min < i_max )  
    { 
        // no check for == since it's not very likely, at most 1 extra iteration
        median = (i_min + i_max)/2;             
        if (m_data[median] > coord) 
            i_max = median;      
        else              
            i_min = median + 1;  
    } 
   
    return i_min;
}

int wxSheetArrayEdge::FindMaxEdgeIndex(int val, int edge_size) const
{
    const int index = FindIndex(val, true);
    if (index < 0) return -1;
    // we know we're inside the 'index' element (or above or below the array)
    //   find which side is closer and if < edge_size return index
    const int diff = abs(GetMax(index) - val);
    const int diff_min = (index > 0) ? abs(GetMax(index-1) - val) : diff+edge_size+1; 

    const int min_diff = wxMin(diff, diff_min);
    if (min_diff > edge_size)
        return -1;
    else if (min_diff == diff)
        return index;
    else if (min_diff == diff_min)
        return index - 1;
    
    return -1;
    
/*   
    // FIXME I wonder if this really makes complete sense? check it...
    // eg. what would happen if size of cell was only 1 pixel, you couldn't resize it?
    if ( GetSize(index) > edge_size )
    {
        // We know that we are in index, test whether we are
        // close enough to lower or upper border, respectively.
        if ( abs(GetMax(index) - val) < edge_size )
            return index;
        else if ( (index > 0) && (val - GetMin(index) < edge_size) )
            return index - 1;
    }

    return -1;
*/    
}

int wxSheetArrayEdge::GetMin(size_t item) const
{
    wxCHECK_MSG(int(item) < m_count, item*m_default_size, _T("invalid index") );
    return m_data.IsEmpty() ? item*m_default_size : (item > 0 ? m_data[item-1] : 0);
}
int wxSheetArrayEdge::GetMax(size_t item) const
{
    wxCHECK_MSG(int(item) < m_count, (item+1)*m_default_size - 1, _T("invalid index") );
    return (m_data.IsEmpty() ? (item+1)*m_default_size : m_data[item]) - 1;
}
int wxSheetArrayEdge::GetSize(size_t item) const
{
    wxCHECK_MSG(int(item) < m_count, m_default_size, _T("invalid index") );
    return m_data.IsEmpty() ? m_default_size : (m_data[item] - (item > 0 ? m_data[item-1] : 0));
}

void wxSheetArrayEdge::SetSize(size_t item, int size)
{
    wxCHECK_RET(int(item) < GetCount(), wxT("Invalid edge index"));
    const int old_size = GetSize(item);
    if (size == old_size)
        return; 
    
    // set to the min size if less than it
    const int min_size = GetMinSize(item);
    if (size < min_size) size = min_size;
    
    // need to really create the array
    if ( m_data.IsEmpty() ) 
        InitArray();
    
    const int diff = size - old_size;
    const int count = m_data.GetCount();
    
    for ( int i = item; i < count; i++ )
        m_data[i] += diff;
    
    if (size == m_default_size)
        CheckMinimize();
}

void wxSheetArrayEdge::SetDefaultSize(int default_size, bool resizeExisting) 
{ 
    wxCHECK_RET(default_size >= 0, wxT("Edge default size must be greater than 0"));
    m_default_size = default_size; 
    if (resizeExisting) 
        m_data.Clear(); 
    else
        CheckMinimize();
}

int wxSheetArrayEdge::GetMinSize(size_t index) const
{
    const int min_index = m_minSizes.FindIndex(index);
    return (min_index != wxNOT_FOUND) ? m_minSizes.GetItemValue(min_index) : 
                                        m_min_allowed_size;
}
void wxSheetArrayEdge::SetMinSize(size_t index, int size)
{
    if (size > m_min_allowed_size) 
        m_minSizes.GetOrCreateValue(index) = size;
    else
        m_minSizes.RemoveValue(index);
}

bool wxSheetArrayEdge::SetMinAllowedSize(int min_allowed_size, bool resizeExisting) 
{
    wxCHECK_MSG(m_default_size >= min_allowed_size, false, wxT("Invalid min allowed size"));
    if (m_min_allowed_size == min_allowed_size)
        return false;

    if (resizeExisting && (min_allowed_size < m_min_allowed_size))
    {
        size_t n, count = m_minSizes.GetCount();
        for (n = 0; n < count; n++)
        {
            if (m_minSizes.GetItemValue(n) < min_allowed_size) 
                m_minSizes.RemoveValue(n);
        }

        count = m_data.GetCount();
        if (count > 0)
        {
            int shift = m_data[0] < min_allowed_size ? min_allowed_size - m_data[0] : 0;
            m_data[0] += shift;
            
            for (n = 1; n < count; n++)
            {
                m_data[n] += shift;
                if (m_data[n] - m_data[n-1] < min_allowed_size)
                {
                    int s = min_allowed_size - (m_data[n] - m_data[n-1]);
                    m_data[n] += s;
                    shift += s;
                }
            }
        }   
    }   

    m_min_allowed_size = min_allowed_size;
    return true;
}

void wxSheetArrayEdge::UpdatePos(size_t pos, int count)
{
    wxSHEET_CHECKUPDATE_RET(pos, count, m_count);

    m_count += count; 

    const int old_count = m_data.GetCount();
    if ((count == 0) || (old_count == 0))
        return;
    
    int start_pos = pos, end_pos = old_count + count;
    int i, edge = 0;
    wxArrayInt sizes;                             // FIXME lazy way
    sizes.Alloc(old_count);
    sizes.Add( m_default_size, old_count);
    
    sizes[0] = m_data[0];
    for ( i = 1; i < old_count; i++ )
        sizes[i] = m_data[i] - m_data[i-1];

    if (count > 0)                                // adding rows/cols
    {
        sizes.Insert( m_default_size, pos, count );
        m_data.Insert( 0, pos, count );
        edge = (pos > 0) ? m_data[pos-1] : 0;
    }
    else // if (count < 0)                        // deleting rows/cols
    {
        sizes.RemoveAt( pos, -count );
        m_data.RemoveAt( pos, -count );
        edge = (pos > 0) ? m_data[pos-1] : 0;
    }
    
    for ( i = start_pos; i < end_pos; i++ )
    {
        edge += sizes[i];
        m_data[i] = edge;
    }   
    
    if (count < 0)                                // maybe deleted non default 
        CheckMinimize();
}

void wxSheetArrayEdge::InitArray()
{
    if (m_count == 0) return;

    m_data.Clear();
    m_data.Alloc( m_count );
    
    int i, edge = 0;
    for ( i = 0; i < m_count; i++ )
    {
        edge += m_default_size;
        m_data.Add( edge );
    }    
}

bool wxSheetArrayEdge::CheckMinimize()
{
    const int count = m_data.GetCount();
    if (count == 0)
        return false;
    
    // start from end since it'll have a better chance of being off
    for (int n = count - 1; n >= 0; n--)     
    {
        if (m_data[n] != m_default_size*n)
            return false;
    }

    m_data.Clear();
    return true;
}
