//
// FILE: spreadcanvas.h -- Declaration of SpreadSheet canvas class
//
// $Id$
//

#ifndef SPREADCANVAS_H
#define SPREADCANVAS_H

class SpreadSheetCanvas : public wxCanvas {
  friend class SpreadSheetPrintout;

  class SpreadCell {
  public:
    int                 row;
    int                 col;
    Bool                editing;
    gText       str;
    SpreadCell(void)    { row = 1; col = 1; editing = FALSE; }
    ~SpreadCell(void)   { }

    void Reset(const char *s = NULL)
      { editing = FALSE; str = s; }

    void CheckValid(int rows, int cols) {
      if (row < 1) row = 1;
      if (col < 1) col = 1;
      if (row > rows) row = rows;
      if (col > cols) col = cols;
    }

    SpreadCell &operator = (const SpreadCell &cell) {
      row = cell.row;
      col = cell.col;
      str = cell.str;
      editing = FALSE;
      return (*this);
    }
  };

private:
  SpreadSheetDrawSettings     *draw_settings;
  SpreadSheetDataSettings     *data_settings;
  SpreadSheet                 *sheet;
  SpreadSheet3D               *top_frame;
  SpreadCell                  cell, old_cell;
  
  // functions
  void ProcessCursor(int ch);
  void UpdateCell(wxDC &dc, SpreadCell &cell);
  void DrawCell(wxDC &dc, int row, int col);

public:
  SpreadSheetCanvas(SpreadSheet *_sheet, wxFrame *parent, int x, int y, int w, int h);

    // Windows Events Handlers
    void    OnPaint(void);
    void    OnChar(wxKeyEvent &ch);
    void    OnEvent(wxMouseEvent &ev);
    void    Update(wxDC &dc);
    void    OnSize(int _w, int _h);

    // Desired size--minimum size that would fit the sheet without scrolling
    // It is never less than MIN_SHEET_WIDTH and never greater than MAX_SHEET_WIDTH
    void    DesiredSize(int *w, int *h);
    int     MaxX(int col = -1);
    int     MaxY(int row = -1);

    // CheckScrollbars
    void    CheckScrollbars(void);

    // LabelExtent.  Since only the actual canvas can tell the extent...
    void GetLabelExtent(char *str, float *x, float *y)
    {
        SetFont(draw_settings->GetLabelFont());
        GetTextExtent(str, x, y);
    }

    // DataExtent
    void GetDataExtent(float *x, float *y, const char *str = "W")
    {
        SetFont(draw_settings->GetDataFont());
        GetTextExtent(str, x, y);
    }

    // Data Access
    int     Row(void)       { return cell.row; }
    int     Col(void)       { return cell.col; }
    void    SetRow(int r)   { cell.row = r; ProcessCursor(1); }
    void    SetCol(int c)   { cell.col = c; ProcessCursor(1); }

    // Low level access
    Bool XYtoRowCol(int x, int y, int *row, int *col);

    // Printing
    void Print(wxOutputMedia device, wxOutputOption fit);
};

#endif  // SPREADCANVAS_H
