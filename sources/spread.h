//
// FILE: spread.h -- Declaration of SpreadSheet class
//
// $Id$
//

#ifndef SPREAD_H
#define SPREAD_H

#include "spreadconfig.h"
#include "grblock.h"

//----------------------------SpreadDataCell----------------------------
#define     S_HILIGHT   1
#define     S_BOLD      2
#define     S_ITALIC    4

class SpreadDataCell
{
    friend gOutput &operator<<(gOutput &op, const SpreadDataCell &s);

private:
    Bool                            entered;
    gSpreadValType      val_type;
    gText                       value;
    long                            attributes;

public:
    // Constructor
    SpreadDataCell(void)
    {
        entered    = FALSE;
        val_type   = gSpreadNum;
        attributes = 0;
    }

    SpreadDataCell(const SpreadDataCell &C)
    {
        entered    = C.entered;
        val_type   = C.val_type;
        value      = C.value;
        attributes = C.attributes;
    }

    ~SpreadDataCell(void) { }

    // Assignment operators
    SpreadDataCell& operator= (const SpreadDataCell &C)
    {
        entered    = C.entered;
        val_type   = C.val_type;
        value      = C.value;
        attributes = C.attributes;
        return (*this);
    }

    SpreadDataCell& operator= (const gText &S)
    {
        if (val_type != gSpreadStr) 
            val_type = gSpreadNum;

        value = S;
        return (*this);
    }

    // Equality
    int operator== (const SpreadDataCell &C) { return value == C.value; }
    int operator!= (const SpreadDataCell &C) { return !(value == C.value); }

    // General info
    Bool    Entered(void) const   { return entered; }
    void    Entered(Bool _e)      { entered = _e; }
    void    SetAttributes(long a) { attributes = a; }
    Bool    HiLighted(void) const { return attributes & S_HILIGHT; }

    void    HiLighted(Bool _e)
    {
        if (_e) 
            attributes |= S_HILIGHT;
        else 
            attributes &= (~S_HILIGHT);
    }

    Bool    Bold(void) const { return attributes & S_BOLD; }

    void    Bold(Bool _b)
    {
        if (_b) 
            attributes |= S_BOLD;
        else 
            attributes &= (~S_BOLD);
    }

    void             SetType(gSpreadValType _type) { val_type = _type; }
    gSpreadValType   GetType(void)   const         { return val_type; }
    const gText     &GetValue(void)  const         { return value; }
    void             SetValue(const gText &S)      { value = S; }

    // Erase all the data in the cell, including clearing all cell attributes
    void    Clear(void)
    {
        entered    = FALSE;
        value      = gText();
        attributes = 0;
    }
};

gOutput &operator<<(gOutput &op, const gBlock<SpreadDataCell> &s);

//-------------------------------------------------------------------------
//************************ SPREADSHEET ***********************************

class SpreadSheetCanvas;

class SpreadSheet
{
    friend gOutput &operator<<(gOutput &op, const SpreadSheet &s);

private:
    SpreadSheetCanvas    *sheet;
    gRectBlock<SpreadDataCell> data;
    gBlock<gText>    row_labels, col_labels;
    gBlock<bool>     row_selectable, col_selectable;
    int              rows, cols, level;
    gText            label;
    Bool             active;

public:
    // Constructors & destructors

    // Void constructor--must have to init arrays of this
  SpreadSheet(void) : sheet(0) {} 
  SpreadSheet(int rows, int cols, int level, 
	      char *title = NULL, wxFrame *parent = NULL);
  ~SpreadSheet();

    // Post-Constructor, use if creating arrays of this
    void Init(int rows, int cols, int level, 
              char *title = NULL, wxFrame *parent = NULL);

    // Change dimensions.  Does nothing if current dimensions are given
    void SetDimensions(int rows, int cols);

    // Sizing info for the canvas ...
    void SetSize(int xs, int ys, int xe, int ye);
    void GetSize(int *w, int *h);
    void CheckSize(void);

    void GetLabelExtent(char *str, float *x, float *y);
    void GetDataExtent(float *x, float *y, const char *str = "W"); 

    // Low level access
    Bool XYtoRowCol(int x, int y, int *row, int *col); 

    // Row/Col manipulation
    void AddRow(int row = 0);
    void AddCol(int col = 0);
    void DelRow(int row);
    void DelCol(int col);

    // Data access stuff for the canvas
    int     GetRows(void)   const       { return rows; }
    int     GetCols(void)   const       { return cols; }

    // Data access for top level
    void    SetActive(Bool _s);

    // General data access
    void        SetValue(int row, int col, const gText &s);
    const gText &GetValue(int row, int col) const;

    gText &     GetLabel(void)            { return label; }
    void        SetLabel(const gText &s)  { label = s;    }

    void        SetType(int row, int col, gSpreadValType t) 
    { data(row, col).SetType(t); }

    int         GetLevel(void)            { return level; }

    // Cell attributes
    gSpreadValType GetType(int row, int col)  { return data(row, col).GetType();   }
    Bool        Bold(int row, int col)        { return data(row, col).Bold();      }
    Bool        HiLighted(int row, int col)   { return data(row, col).HiLighted(); }
    void        Bold(int row, int col, Bool _b)      { data(row, col).Bold(_b);      }
    void        HiLighted(int row, int col, Bool _h) { data(row, col).HiLighted(_h); }

    // Erase all the data in the spreadsheet, including clearing all cell attributes
    void        Clear(void);

    // Checking if the cell has something in it
    Bool        EnteredCell(int row, int col) { return data(row, col).Entered(); }

    // Row/Column labeling
    void        SetLabelRow(int row, const gText &s) { row_labels[row] = s;    }
    void        SetLabelCol(int col, const gText &s) { col_labels[col] = s;    }
    void        SetLabelRow(const gBlock<gText> &vs) { row_labels = vs;        }
    void        SetLabelCol(const gBlock<gText> &vs) { col_labels = vs;        }
    gText       GetLabelRow(int row)                 { return row_labels[row]; }
    gText      GetLabelCol(int col)                  { return col_labels[col]; }

  void SetSelectableRow(int row, Bool sel) { row_selectable[row] = sel; }
  void SetSelectableCol(int col, Bool sel) { col_selectable[col] = sel; }

  Bool GetSelectableRow(int row) const { return row_selectable[row]; }
  Bool GetSelectableCol(int col) const { return col_selectable[col]; }

    // Accessing the currently hilighted cell
    int CurRow(void);
    int CurCol(void);
    void SetCurRow(int r);
    void SetCurCol(int c); 

    // Equality operators to allow this class to be used in a gList
    int         operator==(const SpreadSheet &) { return 0; }
    int         operator!=(const SpreadSheet &) { return 1; }

    // Printing
    void Print(wxOutputMedia device, wxOutputOption fit); 

    // Forced updating
    void Repaint(void);

    // Forced focus.  Need this if focus gets lost to a button/menu.  
    // That would disable the keyboard input to the canvas.  
    // Call this to force focus.
    void SetFocus(void);

    // Debugging
    void Output(gOutput &out) const;
};

#endif // SPREAD_H
