/////////////////////////////////////////////////////////////////////////////
// Name:        grid.h
// Author:      John Labenski
// Created:     07/01/02
// Copyright:   John Labenski, 2002
// License:     wxWidgets v2
/////////////////////////////////////////////////////////////////////////////

#ifndef __MY_GRID_H__
#define __MY_GRID_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "grid.h"
#endif

class wxGrid;
class wxGridCellTextEditor;
class wxSpinCtrlDbl;
class wxBlockIntSelection;

#include "wx/grid.h"

enum CellPos_Type
{
    cellOnTop = 0x0000,
    cellLeft  = 0x0001,
    cellRight = 0x0010,
    cellAbove = 0x0100,
    cellBelow = 0x1000,
    
    cellBelowRight = 0x1010,
    cellBelowLeft  = 0x1001,
    cellAboveRight = 0x0110,
    cellAboveLeft  = 0x0101,
    
    cellSurround   = 0x1111   // use as a mask for int conversion
};

WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, MyGridCopiedHash);

//-----------------------------------------------------------------------------
// MyGrid
//-----------------------------------------------------------------------------

class MyGrid: public wxGrid
{
public:
    MyGrid() : wxGrid() { Init(); }
    MyGrid( wxWindow *parent,
            wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxWANTS_CHARS,
            const wxString& name = wxPanelNameStr )
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    bool Create( wxWindow *parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxWANTS_CHARS,
                 const wxString& name = wxPanelNameStr );
        
    virtual ~MyGrid();
    
    // must call this after creation (override if subclassing)
    virtual bool CreateGrid( int numRows, int numCols, 
                             wxGrid::wxGridSelectionModes selmode = wxGrid::wxGridSelectCells );
    
    // forces a reread of the table and refreshes the grid, 
    //   only for TheGridTableBase derived tables, does nothing otherwise
    virtual void UpdateTable();

    // make all cols have same width, w/o scrollbars, called in OnSize to keep it this way
    //   doesn't do anything if widths would be less than 10.
    void SetEqualColWidths( bool make_equal ); 

    // Specify exactly the number of rows and cols, returns if anything was done
    bool SetNumberCols( int cols );
    bool SetNumberRows( int rows );
    bool SetNumberCells( int rows, int cols );

    // Pop up this menu on right click, it is deleted if menu_static = false
    //   the menu events are not handled in any way
    //   if you want to reuse the same menu in other places, set with static=true so it won't get deleted twice
    void SetRightClickGridMenu( wxMenu *menu, bool menu_static = false );
    wxMenu *GetRightClickGridMenu() { return m_gridMenu; }
    void SetRightClickColLabelMenu( wxMenu *menu, bool menu_static = false );
    wxMenu *GetRightClickColLabelMenu() { return m_colsMenu; }
    void SetRightClickRowLabelMenu( wxMenu *menu, bool menu_static = false );
    wxMenu *GetRightClickRowLabelMenu() { return m_rowsMenu; }
    
    // Same as wxGrid's select block, but for a whole wxBlockIntSelection
    void SelectBlockInt( const wxBlockIntSelection &sel, bool add_to_selection=false );
    
    // get a wxBlockSelection mirror of the grid's current selection, always updated
    const wxBlockIntSelection &GetCurrentSelection() const
        { return *m_currentSelection; }
    
    // Copy the current selelection internally 
    virtual void CopyCurrentSelection(bool copy_cursor = true)
        { CopyCurrentSelection(*m_copiedSelection, copy_cursor); }
    // Get the internal last copied selection
    const wxBlockIntSelection &GetCopiedSelection() const
        { return *m_copiedSelection; }
    // copy the current grid's selection, or the cursor cell if no selection
    //  fills input blockSel
    void CopyCurrentSelection(wxBlockIntSelection &blockSel, bool copy_cursor);

    // Copy the current selection to the wxClipboard using tabs for cols and \n for rows
    void CopyCurrentSelectionToClipboard(bool copy_cursor = true)
        { CopyCurrentSelection(*m_copiedSelection, copy_cursor); 
          CopySelectionToClipboard(*m_copiedSelection); }
    // copy the selection to the wxClipboard using tabs for cols and \n for rows
    void CopySelectionToClipboard(const wxBlockIntSelection &blockSel);

    // copy the current selection to a string using tabs for cols and \n for rows
    wxString CopyCurrentSelectionToClipboardString(bool copy_cursor = true)
        { CopyCurrentSelection(*m_copiedSelection, copy_cursor);
          return CopySelectionToClipboardString(*m_copiedSelection); }
    // copy the selection to a string using tabs for cols and \n for rows
    wxString CopySelectionToClipboardString(const wxBlockIntSelection &blockSel);
    
    // update internal "copied" selection from string using a tab for cols, \n for rows
    void CopyStringToSelection(const wxString &val);

    // paste the string value in the clipboard, updates internal "copied" selection
    //    pastes to the current cursor position if topLeft == wxGridNoCellCoords
    void PasteClipboardString(const wxGridCellCoords &topLeft = wxGridNoCellCoords);
          
    // paste the copied selection at the topLeft coords or into the current selection,
    //   or at the cursor position if topLeft = wxGridNoCellCoords
    virtual void PasteCopiedSelection(const wxGridCellCoords &topLeft = wxGridNoCellCoords);
    // Are the cells being pasted right now, use this in the table's 
    //    SetCellValue and AppendXXX to differentiate between a user typing
    bool CurrentlyPasting() const { return m_pasting; }
    
    // utility fn
    CellPos_Type RelativeCellPosition(const wxGridCellCoords &cellA, 
                                      const wxGridCellCoords &cellB);

    virtual void Refresh(bool eraseb = true,
                         const wxRect* rect = (const wxRect *)  NULL);

protected:
    void Init();    

    void OnCellRangeSelect( wxGridRangeSelectEvent &event );
    void OnGridRClick( wxGridEvent &event );
    void OnLabelRClick( wxGridEvent &event );

    void OnSizeEvent( wxSizeEvent &event );

    void DoPasteCopiedSelection(int row_shift, int col_shift, bool top, bool left, bool is_selection);

    bool m_equal_col_widths;
    bool m_pasting;

    // Attached right click menu (or NULL if none)
    wxMenu *m_gridMenu;
    wxMenu *m_colsMenu;
    wxMenu *m_rowsMenu;
    bool m_grid_menu_static;
    bool m_cols_menu_static;
    bool m_rows_menu_static;
    
    wxBlockIntSelection *m_copiedSelection;  // filled by CopyCurrentSelection
    wxBlockIntSelection *m_currentSelection; // duplicatate of wxGrid selection
    
    MyGridCopiedHash m_copiedHash; // last copied data as strings 
    int m_copiedHash_ncols;        // access m_copiedHash[row*m_copiedHash_ncols+col]
    
    int m_last_nrows, m_last_ncols; // last number of rows/cols for syncing m_currentSelection
    
private :  
    DECLARE_DYNAMIC_CLASS(MyGrid)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
// wxGridSelectionIterator - makes a full copy of the wxGrid's selection to iterate through
//-----------------------------------------------------------------------------

class wxGridSelectionIterator
{
public :
    wxGridSelectionIterator( wxGrid *grid );
    
    // resets the iterating to start at the beginning
    void Reset();
    // Get next selection, returns wxGridNoCellCoords if at the end
    wxGridCellCoords GetNext();
    // checks if this row and col (cell) is selected
    bool IsInSelection(int row, int col) const;
    bool IsInSelection(const wxGridCellCoords &coords) const 
        { return IsInSelection(coords.GetRow(), coords.GetCol()); }

protected :
    int m_nrows, m_ncols;

    size_t m_cellSelectionIndex,
           m_rowSelectionIndex,
           m_colSelectionIndex,
           m_blockSelectionIndex;

    wxGridCellCoords m_rowSelectionCoords,
                     m_colSelectionCoords,
                     m_blockSelectionCoords;

    wxGridCellCoordsArray m_cellSelection;
    wxGridCellCoordsArray m_blockSelectionTopLeft;
    wxGridCellCoordsArray m_blockSelectionBottomRight;
    wxArrayInt            m_rowSelection;
    wxArrayInt            m_colSelection;
};

//-----------------------------------------------------------------------------
// wxGridCellSpinDblEditor
//-----------------------------------------------------------------------------

class wxGridCellSpinDblEditor : public wxGridCellTextEditor
{
public:
    // allows to specify the range - if min == max == -1, no range checking is done
    wxGridCellSpinDblEditor(double value = 0.0, double min = -1.0, double max = -1.0,
                             double increment = 1.0, wxFont *font = (wxFont*)NULL);

    virtual void Create(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler);

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(int row, int col, wxGrid* grid);
    virtual bool EndEdit(int row, int col, wxGrid* grid);

    virtual void Reset();
    virtual void StartingKey(wxKeyEvent& event);

    // parameters string format is "min,max"
    virtual void SetParameters(const wxString& params);

    virtual wxGridCellEditor *Clone() const
        { return new wxGridCellSpinDblEditor(m_value, m_min, m_max); }

    wxSpinCtrlDbl *Spin() const { return (wxSpinCtrlDbl *)m_control; }
        
protected:
    // if HasRange(), we use wxSpinCtrl - otherwise wxTextCtrl
    bool HasRange() const { return m_min != m_max; }

    // string representation of m_valueOld
    wxString GetString() const
        { return wxString::Format(wxT("%lf"), m_valueOld); }

    double m_value, m_min, m_max, m_increment;
    wxFont *m_font;

    double m_valueOld;
};

//----------------------------------------------------------------------------
// TheGridTable
//----------------------------------------------------------------------------

class WXDLLEXPORT TheGridTable : public wxGridTableBase
{
public:
    TheGridTable():wxGridTableBase() { m_rows = 0; m_cols = 0; }

    // added function to simply manage adding/deleting the rows/cols
    virtual bool UpdateRowsCols(); 
    
    virtual ~TheGridTable() {}

    // these are pure virtual in wxGridTableBase
    //virtual int GetNumberRows() = 0;
    //virtual int GetNumberCols() = 0;
    virtual bool IsEmptyCell( int row, int col );
    //virtual wxString GetValue( int row, int col ) = 0;
    virtual void SetValue( int WXUNUSED(row), int WXUNUSED(col), const wxString& WXUNUSED(s) ) {}

    // overridden functions from wxGridTableBase

    virtual void Clear() {}
    virtual bool InsertRows( size_t pos = 0, size_t numRows = 1 ); // { return false; }
    virtual bool AppendRows( size_t numRows = 1 );                 // { return false; }
    virtual bool DeleteRows( size_t pos = 0, size_t numRows = 1 ); // { return false; }
    virtual bool InsertCols( size_t pos = 0, size_t numCols = 1 ); // { return false; }
    virtual bool AppendCols( size_t numCols = 1 );                 // { return false; }
    virtual bool DeleteCols( size_t pos = 0, size_t numCols = 1 ); // { return false; }

    virtual void SetRowLabelValue( int WXUNUSED(row), const wxString& ) {}
    virtual void SetColLabelValue( int WXUNUSED(col), const wxString& ) {}
    
    virtual wxString GetRowLabelValue( int row ) { return wxString::Format(wxT("%d"), row+1); }
    virtual wxString GetColLabelValue( int col ) { return wxString::Format(wxT("%d"), col+1); }

protected:  
    int m_rows, m_cols;
    
private:
    DECLARE_ABSTRACT_CLASS( TheGridTable )
};

#endif // __MY_GRID_H__
