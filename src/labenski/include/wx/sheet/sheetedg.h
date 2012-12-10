///////////////////////////////////////////////////////////////////////////////
// Name:        sheetedg.h
// Purpose:     wxSheetArrayEdge
// Author:      John Labenski
// Modified by: 
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETEDG_H__
#define __WX_SHEETEDG_H__

#include "wx/sheet/sheetdef.h"

// ----------------------------------------------------------------------------
// wxPairArrayIntInt - pair array of (int, int)
//   used to store the min col widths for rows and cols
// ----------------------------------------------------------------------------

DECLARE_PAIRED_INT_DATA_ARRAYS( int, wxArrayInt, 
                                wxPairArrayIntInt, class WXDLLIMPEXP_SHEET )

// ----------------------------------------------------------------------------
// wxSheetArrayEdge - a wxArrayInt container that sums its values, starts at 0
//   Used in the wxSheet to lookup the pixel position of the cells
//   Automatically creates the arrays as necessary and deletes them if not.
//   Tries to clear arrays whenever possible to save memory
//   Stores only the maxes (rights/bottoms) of the edges, but can be used
//     to get their mins (lefts/tops) and sizes (widths/heights).
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetArrayEdge
{
public:    
    // Create an array of initial 'count' edges with default_size
    wxSheetArrayEdge( size_t count = 0, size_t default_size = 1, 
                      size_t min_allowed_size = 0 ); 

    // Get the number of edges
    int GetCount() const { return m_count; }
    // Find the index of the element that corresponds to this coord
    //   if clipToMinMax return the min or max edge element, else -1 for out of bounds
    int FindIndex(int val, bool clipToMinMax) const;
    // Find the index of the element whose max edge is within +/- edge_size
    int FindMaxEdgeIndex(int val, int edge_size = WXSHEET_LABEL_EDGE_ZONE) const;
    
    // Get the minimum of the element (left or top edge)
    int GetMin(size_t index) const;
    // Get the maximum of the element (right or bottom edge)
    int GetMax(size_t index) const;
    // Get the size of the element (width or height)
    //   note : GetSize(n) = GetMax(n) - GetMin(n) + 1;
    //   eg. 0123|4567|89 for index 1 : min = 4, max = 7, size = 4 = max-min+1
    int GetSize(size_t index) const;
    
    // Set the size of the element (width or height)
    void SetSize(size_t item, int size);
    
    // Get the default size to use between the edges
    int GetDefaultSize() const { return m_default_size; }
    // Set the default size to use between edges, used when inserting
    //   if resizeExisting then make all equal widths
    void SetDefaultSize(int default_size, bool resizeExisting = false);

    // Get the minimum size for this index, = GetMinAllowedSize unless set higher
    int GetMinSize(size_t index) const;
    // Set the minimum size for this index, must be >= min allowed size
    void SetMinSize(size_t index, int size);

    // Get the min allowed size for all elements
    int GetMinAllowedSize() const { return m_min_allowed_size; }
    // set the min allowed size, resize the existing sizes to match the min size
    //   if resizeExisting and return if anything was done
    bool SetMinAllowedSize(int min_allowed_size, bool resizeExisting = false); 

    // Clear the number of edges to zero
    void Clear() { m_data.Clear(); m_count = 0; }
    // Update the number of edges by inserting at pos if num > 0, else deleting
    //   inserts use the defaut size.
    void UpdatePos(size_t pos, int num);

protected:
    bool CheckMinimize();           // maybe we don't need the array anymore
    void InitArray();               // init array to the default size
    int  m_count;                   // number of elements
    int  m_default_size;            // default size to init with
    int  m_min_allowed_size;        // overall min allowed size
    wxArrayInt        m_data;       // maxes of each index, filled as needed
    wxPairArrayIntInt m_minSizes;   // min sizes for specific indexes
};

#endif  // __WX_SHEETEDG_H__
