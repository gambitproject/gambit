/////////////////////////////////////////////////////////////////////////////
// Name:        grid.cpp
// Author:      John Labenski
// Created:     07/01/02
// Copyright:   John Labenski, 2002
// License:     wxWidgets v2
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "grid.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/menu.h"
#endif // WX_PRECOMP

#include "wx/clipbrd.h"

#include "wx/wxthings/grid.h"
#include "wx/wxthings/spinctld.h"
#include "wx/wxthings/block.h"

// ==============================================================================
// ==============================================================================

#define MYGRID_COMPARE_BLOCKS 1  // an internal check to see if blocks match grid sel

// ==============================================================================
// ==============================================================================

#ifdef MYGRID_COMPARE_BLOCKS
void CompareBlockAndGrid(MyGrid *grid)
{
    wxGridSelectionIterator selIter( grid );
    const wxBlockIntSelection *m_currentSelection = &grid->GetCurrentSelection();
    
    wxGridCellCoords cell = selIter.GetNext();
    
    while (cell != wxGridNoCellCoords)
    {
        if (!m_currentSelection->Contains(wxBlockInt(cell.GetCol(), cell.GetRow(), cell.GetCol(), cell.GetRow())))
        {
            printf("-------------------------------------------------------------\n");
            printf("BLOCK IS MISSING GRID CELLS %d %d\n", cell.GetCol()+1, cell.GetRow()+1);
            wxBell();
        }
        
        cell = selIter.GetNext();
    }
    int i;
    for (i=0; i<m_currentSelection->GetCount(); i++)
    {
        for (int r=(*m_currentSelection)[i].m_y1; r<=(*m_currentSelection)[i].GetBottom(); r++)
        {
            for (int c=(*m_currentSelection)[i].m_x1; c<=(*m_currentSelection)[i].GetRight(); c++)
            {
                if (!selIter.IsInSelection(r, c))
                {
                    printf("==========================================================\n");
                    printf("TOO MANY BLOCK CELLS %d %d\n", c+1, r+1);
                    wxBell();
                }
            }
        }
    }
/*
    wxBlockIntSelectionIterator blockIter(*m_currentSelection);
    wxPoint2DInt pt;
    i = 0;
    while (blockIter.GetNext(pt))
        {  printf("Iter Blocks %d - %d %d\n", i, pt.m_x, pt.m_y); i++; }
*/    
    for (i=0; i<m_currentSelection->GetCount(); i++)
       printf("Blocks %d - %d %d %d %d\n", i, (*m_currentSelection)[i].m_x1+1, (*m_currentSelection)[i].m_y1+1, (*m_currentSelection)[i].m_x2+1, (*m_currentSelection)[i].m_y2+1);
    
    
    printf("Done comparing the grid selections %d\n", m_currentSelection->GetCount());
    fflush(stdout);
}

#endif // MYGRID_COMPARE_BLOCKS

// ==============================================================================
// ==============================================================================

IMPLEMENT_DYNAMIC_CLASS( MyGrid, wxGrid )

BEGIN_EVENT_TABLE(MyGrid, wxGrid)
    EVT_GRID_CELL_RIGHT_CLICK( MyGrid::OnGridRClick )
    EVT_GRID_LABEL_RIGHT_CLICK( MyGrid::OnLabelRClick )
    
    EVT_GRID_RANGE_SELECT( MyGrid::OnCellRangeSelect )
    EVT_SIZE( MyGrid::OnSizeEvent )    
END_EVENT_TABLE()

bool MyGrid::Create( wxWindow *parent, wxWindowID id,
                     const wxPoint& pos, const wxSize& size,
                     long style, const wxString& name )
{
    if (!wxGrid::Create(parent, id, pos, size, style, name))
        return false;
    
    m_copiedSelection = new wxBlockIntSelection();
    m_currentSelection = new wxBlockIntSelection(); 
    return true;    
}

void MyGrid::Init() 
{
    m_equal_col_widths = false;
    m_pasting = false;
    
    m_gridMenu = NULL;
    m_colsMenu = NULL;
    m_rowsMenu = NULL;
    m_grid_menu_static = false;
    m_cols_menu_static = false;
    m_rows_menu_static = false;
    
    m_last_nrows = m_last_ncols = 0;

    m_copiedHash_ncols = 0;
    
    m_copiedSelection = NULL;
    m_currentSelection = NULL;
}

MyGrid::~MyGrid()
{
    SetRightClickGridMenu(NULL);
    SetRightClickColLabelMenu(NULL);
    SetRightClickRowLabelMenu(NULL);
    
    delete m_copiedSelection;
    delete m_currentSelection;
}

void MyGrid::Refresh(bool eraseb, const wxRect* rect)
{
    wxGrid::Refresh(eraseb, rect);
}

bool MyGrid::CreateGrid( int numRows, int numCols, wxGrid::wxGridSelectionModes selmode )
{
    return wxGrid::CreateGrid( numRows, numCols, selmode );
}

void MyGrid::UpdateTable()
{
    TheGridTable *gridTable = wxDynamicCast(GetTable(), TheGridTable);
    if (gridTable && !gridTable->UpdateRowsCols()) // if true it'll be updated anyway
    {
        if (IsShown())
            ForceRefresh();
    }
    AdjustScrollbars();  // FIXME - this is for the broken grid,    
}

void MyGrid::OnSizeEvent( wxSizeEvent &event )
{
    SetEqualColWidths( m_equal_col_widths );
    event.Skip();
}

void MyGrid::SetEqualColWidths( bool equal_widths )
{
    m_equal_col_widths = equal_widths;

    // don't fail here, since EVT_SIZEs are generated before the grid is 
    if (!m_created) return;
        
    if (m_equal_col_widths && (GetNumberCols() > 0))
    {
        int colwidth = (GetClientSize().GetWidth() - GetRowLabelSize())/GetNumberCols() - 1;
        if (colwidth > 10) 
            SetDefaultColSize( colwidth, true );
    }
}

void MyGrid::SetRightClickGridMenu( wxMenu *menu, bool menu_static )
{ 
    if (!m_grid_menu_static && m_gridMenu)
        delete m_gridMenu;

    m_gridMenu = menu; 
    m_grid_menu_static = menu_static; 
}

void MyGrid::SetRightClickColLabelMenu( wxMenu *menu, bool menu_static )
{ 
    if (!m_cols_menu_static && m_colsMenu)
        delete m_colsMenu;

    m_colsMenu = menu; 
    m_cols_menu_static = menu_static; 
}

void MyGrid::SetRightClickRowLabelMenu( wxMenu *menu, bool menu_static )
{ 
    if (!m_rows_menu_static && m_rowsMenu)
        delete m_rowsMenu;

    m_rowsMenu = menu; 
    m_rows_menu_static = menu_static; 
}

void MyGrid::OnGridRClick( wxGridEvent &event )
{
    if (GetRightClickGridMenu()) 
        PopupMenu(GetRightClickGridMenu(), event.GetPosition());
    else
        event.Skip();
}

void MyGrid::OnLabelRClick( wxGridEvent &event )
{
    wxPoint point = event.GetPosition();
    
    // note for corner label subtract both sizes
    if (GetRightClickColLabelMenu() && (event.GetCol() >= 0))
    {
        point.y -= GetColLabelSize();
        PopupMenu(GetRightClickColLabelMenu(), point);
    }
    else if (GetRightClickRowLabelMenu() && (event.GetRow() >= 0))
    {
        point.x -= GetRowLabelSize();
        PopupMenu(GetRightClickRowLabelMenu(), point);
    }
    else
        event.Skip();
}

bool MyGrid::SetNumberCols( int cols )
{
    wxCHECK_MSG(m_created, false, wxT("Called MyGrid::SetNumberCols() before calling CreateGrid()") );

    if (GetNumberCols() > cols) 
        return DeleteCols(cols, GetNumberCols() - cols);
    else if (GetNumberCols() < cols) 
        return AppendCols(cols - GetNumberCols());
    
    return false;
}

bool MyGrid::SetNumberRows( int rows )
{
    wxCHECK_MSG(m_created, false, wxT("Called MyGrid::SetNumberRows() before calling CreateGrid()") );

    if (GetNumberRows() > rows) 
        return DeleteRows(rows, GetNumberRows() - rows);
    else if (GetNumberRows() < rows) 
        return AppendRows(rows - GetNumberRows());
    
    return false;
}

bool MyGrid::SetNumberCells( int rows, int cols )
{
    return (SetNumberRows(rows) || SetNumberCols(cols));
}

void MyGrid::OnCellRangeSelect( wxGridRangeSelectEvent &event )
{
/*  
    printf("cells %d, topleft %d, bottomright %d, rows %d cols %d\n", 
        wxGrid::GetSelectedCells().GetCount(),
        wxGrid::GetSelectionBlockTopLeft().GetCount(),
        wxGrid::GetSelectionBlockBottomRight().GetCount(),
        wxGrid::GetSelectedRows().GetCount(),
        wxGrid::GetSelectedCols().GetCount() ); 
    
    if (wxGrid::GetSelectedCells().GetCount() > 0)
    {
        printf("selected cell %d %d\n", wxGrid::GetSelectedCells().Item(0).GetRow(), 
        wxGrid::GetSelectedCells().Item(0).GetCol());
    }
    fflush(stdout);
*/  
/*  
    static long count = 0;

    printf("%ld TopLeftCoords %d %d BottomRightCoords %d %d \n", count,
        event.GetTopLeftCoords().GetRow(), event.GetTopLeftCoords().GetCol(),
        event.GetBottomRightCoords().GetRow(), event.GetBottomRightCoords().GetCol() );
    printf("TopRow %d BottomRow %d LeftCol %d RightCol %d \n",
            event.GetTopRow(), event.GetBottomRow(), event.GetLeftCol(), event.GetRightCol());
    printf("Selecting %d Control %d Meta %d Shift %d Alt %d \n\n",
        event.Selecting(), event.ControlDown(), event.MetaDown(), event.ShiftDown(), event.AltDown());
    fflush(stdout);
    count++;
*/

    event.Skip();

    int nrows = GetNumberRows();
    int ncols = GetNumberCols();

    if ((nrows != m_last_nrows) || (ncols != m_last_ncols))
    {
        CopyCurrentSelection(*m_currentSelection, false);
        m_last_nrows = nrows;
        m_last_ncols = ncols;
        return;
    }

    int tl_col = event.GetTopLeftCoords().GetCol();
    int tl_row = event.GetTopLeftCoords().GetRow();
    int br_col = event.GetBottomRightCoords().GetCol();
    int br_row = event.GetBottomRightCoords().GetRow();
    
    wxBlockInt block(tl_col, tl_row, br_col, br_row);
    
    // FIXME - the grid selects - rows and cols, fix it - what does this mean? I forget
    
    if (block.IsEmpty()) return;
    
    if (event.Selecting())
        m_currentSelection->SelectBlock(block);
    else
        m_currentSelection->DeselectBlock(block);

#ifdef MYGRID_COMPARE_BLOCKS
    CompareBlockAndGrid(this);
#endif // MYGRID_COMPARE_BLOCKS
}

void MyGrid::SelectBlockInt( const wxBlockIntSelection &sel, bool add_to_selection )
{
    if (!add_to_selection) ClearSelection();
        
    for (int n = 0; n<sel.GetCount(); n++)
    {
        SelectBlock(sel[n].m_y1, sel[n].m_x1, sel[n].m_y2, sel[n].m_x2, true);
    }
}

wxString MyGrid::CopySelectionToClipboardString(const wxBlockIntSelection &blockSel)
{
    wxBlockIntSelectionIterator blockIter(blockSel);
    
    wxString value;
    wxPoint2DInt cell;
    int last_row = GetNumberRows() + 10;
    int last_col = GetNumberCols() + 10;
    int n;
    
    while (blockIter.GetNext(cell))
    {
        for (n = last_row; n<cell.m_y; n++)
            value += wxT("\n");
        
        for (n = last_col; n<cell.m_x; n++)
            value += wxT("\t");
        
        last_row = cell.m_y;
        last_col = cell.m_x;
        
        printf("last row %d %d %d\n", last_row, last_col, blockSel.GetCount()); fflush(stdout);
        
        value += GetCellValue(last_row, last_col);
    }

    return value;    
}

void MyGrid::CopySelectionToClipboard(const wxBlockIntSelection &blockSel)
{
    if (!wxTheClipboard->Open()) 
        return;
    
    wxTheClipboard->UsePrimarySelection(true);

    wxString value = CopySelectionToClipboardString(blockSel);
    
    if (!value.IsEmpty())
        wxTheClipboard->SetData( new wxTextDataObject(value) );
    
    wxTheClipboard->Close();
}

void MyGrid::CopyCurrentSelection(wxBlockIntSelection &blockSel, bool copy_cursor)
{
    blockSel.Clear();
    m_copiedHash.clear();
    
    int nrows = GetNumberRows();
    int ncols = GetNumberCols();
    
    m_copiedHash_ncols = ncols;
    
    // just copy the cursor cell if no selection
    if (!IsSelection() && copy_cursor)             
    {      
        int row = m_currentCellCoords.GetRow();
        int col = m_currentCellCoords.GetCol();
        blockSel.SelectBlock(wxBlockInt(col, row, col, row));
        m_copiedHash[row*m_copiedHash_ncols + col] = GetCellValue(row, col);
        return;
    }
   
    int i = 0, count = 0;
    wxGridCellCoords cell;

    // copy the individual selected cells
    const wxGridCellCoordsArray &selCells = GetSelectedCells();    
    count = selCells.GetCount();
    for (i=0; i<count; i++)
    {
        cell = selCells[i];
        int row = cell.GetRow();
        int col = cell.GetCol();
        blockSel.SelectBlock(wxBlockInt(col, row, col, row), false);
        m_copiedHash[row*m_copiedHash_ncols + col] = GetCellValue(row, col);
    }
        
    // copy the blocks of selected cells
    const wxGridCellCoordsArray &selBlockTL = GetSelectionBlockTopLeft();
    const wxGridCellCoordsArray &selBlockBR = GetSelectionBlockBottomRight();
    count = selBlockTL.GetCount();
    for (i=0; i<count; i++)
    {
        cell = selBlockTL[i];
        int rowTL = cell.GetRow();
        int colTL = cell.GetCol();
        cell = selBlockBR[i];
        int rowBR = cell.GetRow();
        int colBR = cell.GetCol();
        blockSel.SelectBlock(wxBlockInt(colTL, rowTL, colBR, rowBR), false);
        
        for (int r = rowTL; r <= rowBR; r++)
        {
            for (int c = colTL; c <= colBR; c++)
            {
                m_copiedHash[r*m_copiedHash_ncols + c] = GetCellValue(r, c);
            }
        }
    }

    // copy the selected rows
    const wxArrayInt &selRows = GetSelectedRows();
    count = selRows.GetCount();
    for (i=0; i<count; i++)
    {
        int row = selRows[i];
        blockSel.SelectBlock(wxBlockInt(0, row, ncols, row), false);
        
        for (int c = 0; c < ncols; c++)
            m_copiedHash[row*m_copiedHash_ncols + c] = GetCellValue(row, c);
    }
        
    // copy the selected cols
    const wxArrayInt &selCols = GetSelectedCols();
    count = selCols.GetCount();
    for (i=0; i<count; i++)
    {
        int col = selCols[i];
        blockSel.SelectBlock(wxBlockInt(col, 0, col, nrows), false);
        
        for (int r = 0; r < nrows; r++)
            m_copiedHash[r*m_copiedHash_ncols + col] = GetCellValue(r, col);
    }

    blockSel.Minimize();
}

void MyGrid::CopyStringToSelection(const wxString &string)
{
    m_copiedSelection->Clear();
    m_copiedHash.clear();
    
    int ncols = 0;
    int max_cols = 0;
    
    const wxChar *c = string.GetData();
    int n, len = string.Length();
    for (n=0; n<len; n++)
    {
        if ((*c) == wxT('\n'))
        {
            if (ncols > max_cols)
                max_cols = ncols;
            
            ncols = 0;
        }
        else if ((*c) == wxT('\t'))
            ncols++;
        
        c++;
    }
    
    if (ncols > max_cols)
        max_cols = ncols;
    
    if (max_cols == 0)
        return;
    
    m_copiedHash_ncols = max_cols;
    
    c = string.GetData();
    int col = 0, row = 0;
    wxString buf;
    
    for (n=0; n<len; n++)
    {
        if ((*c) == wxT('\n'))
        {
            m_copiedSelection->SelectBlock(wxBlockInt(col, row, col, row), false);
            m_copiedHash[row*m_copiedHash_ncols + col] = buf;

            buf.Empty();
            row++;
            col = 0;
        }
        else if ((*c) == wxT('\t'))
        {
            m_copiedSelection->SelectBlock(wxBlockInt(col, row, col, row), false);
            m_copiedHash[row*m_copiedHash_ncols + col] = buf;
            
            buf.Empty();
            col++;
        }
        else
            buf += *c;
        
        c++;
    }    
    
    if (!buf.IsEmpty())
    {
        m_copiedSelection->SelectBlock(wxBlockInt(col, row, col, row), false);
        m_copiedHash[row*m_copiedHash_ncols + col] = buf;
    }
    
    m_copiedSelection->Minimize();
}

CellPos_Type MyGrid::RelativeCellPosition(const wxGridCellCoords &cellA, const wxGridCellCoords &cellB)
{
    int pos = cellOnTop;
    
    if      (cellA.GetRow() < cellB.GetRow()) pos |= cellAbove;
    else if (cellA.GetRow() > cellB.GetRow()) pos |= cellBelow;
        
    if      (cellA.GetCol() < cellB.GetCol()) pos |= cellLeft;
    else if (cellA.GetCol() > cellB.GetCol()) pos |= cellRight;
        
    return CellPos_Type(pos);
}

void MyGrid::PasteClipboardString(const wxGridCellCoords &topLeft)
{
    if (!wxTheClipboard->Open()) 
        return;

    wxTextDataObject dataObj;
    wxTheClipboard->GetData( dataObj );
    wxTheClipboard->Close();
    
    // if the string is empty, then do it anyway to clear copied selection
    CopyStringToSelection(dataObj.GetText());
    
    PasteCopiedSelection(topLeft);   
}

void MyGrid::PasteCopiedSelection(const wxGridCellCoords &topLeft_)
{
    int copied_count = m_copiedSelection->GetCount();
    
    if (copied_count < 1) return;

    wxBlockInt current_bound = m_currentSelection->GetBoundingBlock();
    wxBlockInt copied_bound = m_copiedSelection->GetBoundingBlock();
    
    //printf("curr bound %d %d %d %d\n", current_bound.m_x, current_bound.m_y, current_bound.m_width, current_bound.m_height);
    //printf("copy bound %d %d %d %d\n", copied_bound.m_x, copied_bound.m_y, copied_bound.m_width, copied_bound.m_height);
    //fflush(stdout);
    
    wxGridCellCoords topLeft = topLeft_;
    bool is_selection = m_currentSelection->GetCount() > 0;
    if (topLeft == wxGridNoCellCoords)
    {
        if (is_selection)
        {
            topLeft.SetCol(current_bound.m_x1);
            topLeft.SetRow(current_bound.m_y1);
        }
        else
        {
            topLeft = m_currentCellCoords;
            is_selection = false;
        }
    }
    
    if (topLeft == wxGridNoCellCoords) return;
    
    CellPos_Type pos = RelativeCellPosition(topLeft, wxGridCellCoords(copied_bound.m_x1, copied_bound.m_y1));

    wxBlockSort_Type sort = wxBLOCKSORT_TOPLEFT_BOTTOMRIGHT;
    bool top = true, left = true;
    
    switch (pos)
    {
        case cellBelowRight : top = false; left = false; sort = wxBLOCKSORT_BOTTOMRIGHT_TOPLEFT; break;

        case cellBelow      :
        case cellRight      :
        case cellBelowLeft  : top = false; left = true;  sort = wxBLOCKSORT_BOTTOMLEFT_TOPRIGHT; break;

        case cellAboveRight : top = true;  left = false; sort = wxBLOCKSORT_TOPRIGHT_BOTTOMLEFT; break;
        
        case cellAbove      :
        case cellLeft       :
        case cellAboveLeft  : 
        default             : top = true;  left = true;  sort = wxBLOCKSORT_TOPLEFT_BOTTOMRIGHT; break;
    }

    m_copiedSelection->Sort(sort);
    
    // when the selection is larger than the copied block - paste to fill up selection
    int n_col_pastes = (int)ceil(double(current_bound.GetWidth())/copied_bound.GetWidth());
    int n_row_pastes = (int)ceil(double(current_bound.GetHeight())/copied_bound.GetHeight());
    if (n_col_pastes < 1) n_col_pastes = 1;
    if (n_row_pastes < 1) n_row_pastes = 1;

    int row_shift = topLeft.GetRow() - copied_bound.m_y1;
    int col_shift = topLeft.GetCol() - copied_bound.m_x1;
    
    if (!top) row_shift += (n_row_pastes-1)*copied_bound.GetHeight();
    if (!left) col_shift += (n_col_pastes-1)*copied_bound.GetWidth();
    
    int init_col_shift = col_shift;

    BeginBatch();
    m_pasting = true;
    
    // Check to see if the grid needs and can be expanded
    if (!is_selection)
    {
        if (topLeft.GetRow() + copied_bound.GetHeight() > GetNumberRows())
            AppendRows(topLeft.GetRow() + copied_bound.GetHeight() - GetNumberRows());
        if (topLeft.GetCol() + copied_bound.GetWidth() > GetNumberCols())
            AppendCols(topLeft.GetCol() + copied_bound.GetWidth() - GetNumberCols());
    }
    
    for (int r=0; r<n_row_pastes; r++)
    {
        for (int c=0; c<n_col_pastes; c++)
        {
            DoPasteCopiedSelection(row_shift, col_shift, top, left, is_selection);
            col_shift += left ? copied_bound.GetWidth() : -copied_bound.GetWidth();
        }
        col_shift = init_col_shift;
        row_shift += top ? copied_bound.GetHeight() : -copied_bound.GetHeight();
    }
    
    m_pasting = false;   
    EndBatch();
}

void MyGrid::DoPasteCopiedSelection(int row_shift, int col_shift, bool top, bool left, bool is_selection)
{
    int rows, cols, to_row, to_col, from_row, from_col;

    int nrows = GetNumberRows();
    int ncols = GetNumberCols();
    int copied_count = m_copiedSelection->GetCount();
    
    for (int n=0; n<copied_count; n++)
    {
        rows = m_copiedSelection->Item(n).GetHeight();
        from_row = m_copiedSelection->Item(n).m_y1 + (top ? 0 : rows - 1);
        to_row = from_row + row_shift;
        
        for (int r=0; r<rows; r++)
        {
            cols = m_copiedSelection->Item(n).GetWidth();
            from_col = m_copiedSelection->Item(n).m_x1 + (left ? 0 : cols - 1);
            to_col = from_col + col_shift;
            
            for (int c=0; c<cols; c++)
            {
                //printf("Pasting from %d %d to %d %d\n",  from_col+1, from_row+1, to_col+1, to_row+1); fflush(stdout);
            
                // FIXME - increasing the grid size when pasting is untested in the sucess case
                if ((!is_selection || m_currentSelection->Contains(wxPoint2DInt(to_col,to_row))) &&
                     (to_row >= 0) && (to_row < nrows) && 
                     (to_col >= 0) && (to_col < ncols))
                {
                    //printf("Actually Pasting from %d %d to %d %d\n",  from_col+1, from_row+1, to_col+1, to_row+1); fflush(stdout);
                    
                    // use this if not using hash but copying directly from current grid
                    //SetCellValue(to_row, to_col, GetCellValue(from_row, from_col));
                    
                    SetCellValue(to_row, to_col, m_copiedHash[from_row*m_copiedHash_ncols + from_col]);
                }

                from_col += left ? 1 : -1;
                to_col += left ? 1 : -1;
            }
            
            from_row += top ? 1 : -1;
            to_row += top ? 1 : -1;
        }
    }
}

// ----------------------------------------------------------------------------
// wxGridSelectionIterator
// ----------------------------------------------------------------------------
wxGridSelectionIterator::wxGridSelectionIterator( wxGrid *grid )
{
    Reset();
    
    m_nrows = 0;
    m_ncols = 0;
    
    wxCHECK_RET(grid, wxT("Invalid wxGrid in wxGridSelectionIterator"));

    m_nrows = grid->GetNumberRows();
    m_ncols = grid->GetNumberCols();
    
    WX_APPEND_ARRAY(m_cellSelection, grid->GetSelectedCells()); 
    WX_APPEND_ARRAY(m_blockSelectionTopLeft, grid->GetSelectionBlockTopLeft()); 
    WX_APPEND_ARRAY(m_blockSelectionBottomRight, grid->GetSelectionBlockBottomRight()); 
    WX_APPEND_ARRAY(m_colSelection, grid->GetSelectedCols());   
    WX_APPEND_ARRAY(m_rowSelection, grid->GetSelectedRows());   
}

void wxGridSelectionIterator::Reset()
{
    m_cellSelectionIndex = 0;
    m_rowSelectionIndex = 0;
    m_rowSelectionCoords = wxGridNoCellCoords;
    m_colSelectionIndex = 0;
    m_colSelectionCoords = wxGridNoCellCoords;
    m_blockSelectionIndex = 0;
    m_blockSelectionCoords = wxGridNoCellCoords;
}

bool wxGridSelectionIterator::IsInSelection(int row, int col) const
{
    size_t n, count;
    wxGridCellCoords *coords1, *coords2;
    
    count = m_cellSelection.GetCount();
    for (n=0; n<count; n++)
    {
        coords1 = &m_cellSelection[n];
        if ( (row == coords1->GetRow()) && (col == coords1->GetCol()) ) return true;
    }
    
    count = m_blockSelectionTopLeft.GetCount();
    for (n=0; n<count; n++)
    {
        coords1 = &m_blockSelectionTopLeft[n];
        coords2 = &m_blockSelectionBottomRight[n];
        if ( (row >= coords1->GetRow()) && (row <= coords2->GetRow()) &&
             (col >= coords1->GetCol()) && (col <= coords2->GetCol()) ) return true;
    }
    
    count = m_rowSelection.GetCount();
    for (n=0; n<count; n++)
    {
        if ( row == m_rowSelection[n] ) return true;
    }
    
    count = m_colSelection.GetCount();
    for (n=0; n<count; n++)
    {
        if ( col == m_colSelection[n] ) return true;
    }

    return false;
}

wxGridCellCoords wxGridSelectionIterator::GetNext()
{
    if (m_cellSelection.GetCount() > m_cellSelectionIndex)
    {
        return m_cellSelection[m_cellSelectionIndex++];
    }
    if (m_rowSelection.GetCount() > m_rowSelectionIndex)
    {
        int row = m_rowSelection[m_rowSelectionIndex];

        // first time here
        if (m_rowSelectionCoords.GetRow() != row)
        {
            m_rowSelectionCoords = wxGridCellCoords(row, 0);
            return m_rowSelectionCoords;
        }
        // at bottom of col, switch to new row index
        if (m_rowSelectionCoords.GetCol() >= m_ncols-1)
        {
            m_rowSelectionIndex++;
            if (m_rowSelection.GetCount() > m_rowSelectionIndex)
            {
                row = m_rowSelection.Item(m_rowSelectionIndex);
                m_rowSelectionCoords = wxGridCellCoords(row, 0);
                return m_rowSelectionCoords;
            }
        }
        // check if past selected row count, else fall though
        if (m_rowSelection.GetCount() > m_rowSelectionIndex)
        {
            m_rowSelectionCoords.SetCol(m_rowSelectionCoords.GetCol() + 1);
            return m_rowSelectionCoords;
        }
    }
    if (m_colSelection.GetCount() > m_colSelectionIndex)
    {
        int col = m_colSelection[m_colSelectionIndex];

        // first time here
        if (m_colSelectionCoords.GetCol() != col)
        {
            m_colSelectionCoords = wxGridCellCoords(0, col);
            return m_colSelectionCoords;
        }
        // at end of row, switch to new row
        if (m_colSelectionCoords.GetRow() >= m_nrows-1)
        {
            m_colSelectionIndex++;
            if (m_colSelection.GetCount() > m_colSelectionIndex)
            {
                col = m_colSelection[m_colSelectionIndex];
                m_colSelectionCoords = wxGridCellCoords(0, col);
                return m_colSelectionCoords;
            }
        }
        // check if past selected col count, else fall through
        if (m_colSelection.GetCount() > m_colSelectionIndex)
        {
            m_colSelectionCoords.SetRow(m_colSelectionCoords.GetRow() + 1);
            return m_colSelectionCoords;
        }
    }
    if (m_blockSelectionTopLeft.GetCount() > m_blockSelectionIndex)
    {
        // first time here
        if (m_blockSelectionCoords.GetRow() < m_blockSelectionTopLeft[m_blockSelectionIndex].GetRow())
        {
            m_blockSelectionCoords = m_blockSelectionTopLeft[m_blockSelectionIndex];
            return m_blockSelectionCoords;
        }
        // at end of block swap to new one
        if (m_blockSelectionCoords == m_blockSelectionBottomRight[m_blockSelectionIndex])
        {
            m_blockSelectionIndex++;
            if (m_blockSelectionTopLeft.GetCount() > m_blockSelectionIndex)
            {
                m_blockSelectionCoords = m_blockSelectionTopLeft[m_blockSelectionIndex];
                return m_blockSelectionCoords;
            }
            else  // past end nothing more to check
                return  wxGridNoCellCoords;
        }
        // at end of col, down to next row
        if (m_blockSelectionCoords.GetCol() == m_blockSelectionBottomRight[m_blockSelectionIndex].GetCol())
        {
            m_blockSelectionCoords.SetCol(m_blockSelectionTopLeft[m_blockSelectionIndex].GetCol());
            m_blockSelectionCoords.SetRow(m_blockSelectionCoords.GetRow() + 1);
            return m_blockSelectionCoords;
        }
        // increment the col
        m_blockSelectionCoords.SetCol(m_blockSelectionCoords.GetCol() + 1);
        return m_blockSelectionCoords;
    }

    return wxGridNoCellCoords;
}

// ----------------------------------------------------------------------------
// wxGridCellSpinDblEditor
// ----------------------------------------------------------------------------

wxGridCellSpinDblEditor::wxGridCellSpinDblEditor(double value, double min, 
                                                 double max, double increment, 
                                                 wxFont *font)
{
    m_value = value;
    m_min = min;
    m_max = max;
    m_increment = increment;
    m_font = font;
}

void wxGridCellSpinDblEditor::Create( wxWindow* parent, wxWindowID id,
                                      wxEvtHandler* evtHandler)
{
    m_value = 0.0;
    if ( !HasRange() ) { m_min = -1E6; m_max = 1E6; }

    m_control = new wxSpinCtrlDbl(*parent, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize,
                                 0,
                                 m_min, m_max, m_value, m_increment);
    
    if ( m_font ) m_control->SetFont( *m_font );
    
    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellSpinDblEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    // first get the value
    wxGridTableBase *table = grid->GetTable();
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_FLOAT) )
    {
        m_valueOld = table->GetValueAsDouble(row, col);
    }
    else
    {
        wxString sValue = table->GetValue(row, col);
        if (!sValue.ToDouble(&m_valueOld))
        {
            wxFAIL_MSG( wxT("this cell doesn't have numeric value") );
            return;
        }
    }

    Spin()->SetValue(m_valueOld);
    Spin()->SetFocus();
}

bool wxGridCellSpinDblEditor::EndEdit(int row, int col, wxGrid* grid)
{
    bool changed;
    double value;
        
    value = Spin()->GetValue();
    changed = (value != m_valueOld);

    if ( changed )
    {
//        if (grid->GetTable()->CanSetValueAs(row, col, wxGRID_VALUE_FLOAT))
//            grid->GetTable()->SetValueAsDouble(row, col, value);
//        else
//            grid->GetTable()->SetValue(row, col, wxString::Format(wxT("%lf"), value));
        grid->GetTable()->SetValue(row, col, wxString::Format(((wxSpinCtrlDbl*)m_control)->GetFormat().c_str(), value));
    }

    return changed;
}

void wxGridCellSpinDblEditor::Reset()
{
    Spin()->SetValue(m_valueOld);
}

bool wxGridCellSpinDblEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_NUMPAD0:
            case WXK_NUMPAD1:
            case WXK_NUMPAD2:
            case WXK_NUMPAD3:
            case WXK_NUMPAD4:
            case WXK_NUMPAD5:
            case WXK_NUMPAD6:
            case WXK_NUMPAD7:
            case WXK_NUMPAD8:
            case WXK_NUMPAD9:
            case WXK_ADD:
            case WXK_NUMPAD_ADD:
            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT:
            case WXK_UP:
            case WXK_DOWN:
                return true;

            default:
                if ( (keycode < 128) && wxIsdigit(keycode) )
                    return true;
        }
    }

    return false;
}

void wxGridCellSpinDblEditor::StartingKey(wxKeyEvent& event)
{
    event.Skip();
}

void wxGridCellSpinDblEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
        // reset to default
        m_min =
        m_max = -1;
    }
    else
    {
        double tmp;
        if ( params.BeforeFirst(wxT(',')).ToDouble(&tmp) )
        {
            m_min = tmp;

            if ( params.AfterFirst(wxT(',')).ToDouble(&tmp) )
            {
                m_max = tmp;

                // skip the error message below
                return;
            }
        }

        wxLogDebug(wxT("Invalid wxGridCellSpinDblEditor parameter string '%s' ignored"), params.c_str());
    }
}

//----------------------------------------------------------------------------
// TheGridTable
//----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( TheGridTable, wxGridTableBase )

bool TheGridTable::IsEmptyCell( int row, int col )
{
    wxASSERT_MSG( (row < GetNumberRows()) && (col < GetNumberCols()),
                  wxT("invalid row or column index in TheGridTable") );

    return true;
}

bool TheGridTable::InsertRows( size_t, size_t ) { return false; }
bool TheGridTable::AppendRows( size_t )         { return false; }
bool TheGridTable::DeleteRows( size_t, size_t ) { return false; }
bool TheGridTable::InsertCols( size_t, size_t ) { return false; }
bool TheGridTable::AppendCols( size_t )         { return false; }
bool TheGridTable::DeleteCols( size_t, size_t ) { return false; }

bool TheGridTable::UpdateRowsCols()
{
    bool updated = false;    
    int rows = GetNumberRows();
    int cols = GetNumberCols();
    
    if (rows > m_rows)
    {
        if ( GetView() )
        {
            wxGridTableMessage msg( this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                    0, rows - m_rows );

            GetView()->ProcessTableMessage( msg );
            updated = true;
        }
    }
    else if (rows < m_rows)
    {
        if ( GetView() )
        {
            wxGridTableMessage msg( this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                    0, m_rows - rows );

            GetView()->ProcessTableMessage( msg );
            updated = true;
        }
    }

    if (cols > m_cols)
    {
        if ( GetView() )
        {
            wxGridTableMessage msg( this, wxGRIDTABLE_NOTIFY_COLS_INSERTED,
                                    0, cols - m_cols );

            GetView()->ProcessTableMessage( msg );
            updated = true;
        }
    }
    else if (cols < m_cols)
    {
        if ( GetView() )
        {
            wxGridTableMessage msg( this, wxGRIDTABLE_NOTIFY_COLS_DELETED,
                                    0, m_cols - cols );

            GetView()->ProcessTableMessage( msg );
            updated = true;
        }
    }
    
    m_rows = rows;
    m_cols = cols;
    return updated;
}
