//
// FILE: spread.cc -- Defines a 3 dimensional spreadsheet/table control.
//
// $Id$
//

#include <stdio.h>
#include "wx.h"
#include "wx_mf.h"
#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif
#ifdef __BORLANDC__
#pragma hdr_stop
#endif // __BORLANDC__
#include    "wxmisc.h"

#include    "gmisc.h"

#include "gambit.h"
#include "spread.h"
#include "spreadcanvas.h"

gOutput &operator<<(gOutput &op, const SpreadSheet &s)
{
    s.Output(op);
    return op;
}

gOutput &operator<<(gOutput &op, const SpreadDataCell &c)
{
    op << c.value;
    return op;
}

//****************************************************************************
//*                               SPREAD SHEET                               *
//****************************************************************************

SpreadSheet::SpreadSheet(int _rows, int _cols, int _level, 
                         char *title, wxFrame *parent)
{
    Init(_rows, _cols, _level, title, parent);
}

SpreadSheet::~SpreadSheet()
{
  if (sheet) {
    sheet->Show(FALSE);
    delete sheet;
  }
}

void SpreadSheet::Init(int rows_, int cols_, int level_, 
                       char *title, wxFrame *parent)
{
    SetDimensions(rows_, cols_);
    level = level_;
    int h, w;
    parent->GetClientSize(&w, &h);
    sheet = new SpreadSheetCanvas(this, parent, 0, 0, w, h-MIN_BUTTON_SPACE);
    
    if (title) label = title;
    else label = " : #"+ToText(level);
}


void SpreadSheet::Clear(void)
{
    for (int i = 1; i <= rows; i++)
        for (int j = 1; j <= cols; j++)
            data(i, j).Clear();
}

void SpreadSheet::SetDimensions(int rows_, int cols_)
{
  assert(rows_ > 0 && cols_ > 0 && "SpreadSheet::Invalid Dimensions");
  rows = rows_;
  cols = cols_;
  data = gRectBlock<SpreadDataCell> (rows, cols);
  row_labels = gBlock<gText>(rows);
  col_labels = gBlock<gText>(cols);
  row_selectable = gBlock<bool>(rows);
  for (int i = 1; i <= rows; row_selectable[i++] = true);
  col_selectable = gBlock<bool>(cols);
  for (int i = 1; i <= cols; col_selectable[i++] = true);
}


void SpreadSheet::AddRow(int row)
{
  if (row == 0) 
    row = rows + 1;

  // add a new row to the matrix
  data.InsertRow(row, (const gArray<SpreadDataCell>)gArray<SpreadDataCell>(cols));

  // Copy the cell types from the previous row
  for (int i = 1; i <= cols; i++) 
    data(rows+1, i).SetType(data(rows, i).GetType());

  row_labels.Insert("", row);
  row_selectable.Insert(true, row);
  rows++;
}


void SpreadSheet::AddCol(int col)
{
  if (col == 0) 
    col = cols + 1;

  // add a new column to the matrix
  data.InsertColumn(col, (const gArray<SpreadDataCell>)gArray<SpreadDataCell>(rows));

  col_labels.Insert("", col);
  col_selectable.Insert(true, col);
  cols++;
}


void SpreadSheet::DelRow(int row)
{
  if (rows < 2) 
    return;
    
  if (row == 0) 
    row = rows;

  // remove a row from the matrix
  data.RemoveRow(row);

  row_labels.Remove(row);
  row_selectable.Remove(row);
  rows--;
}


void SpreadSheet::DelCol(int col)
{
  if (cols < 2) 
    return;
    
  if (col == 0) 
    col = cols;

  // remove a column from the matrix
  data.RemoveColumn(col);

  col_labels.Remove(col);
  col_selectable.Remove(col);
  cols--;
}


Bool SpreadSheet::XYtoRowCol(int x, int y, int *row, int *col) 
{ 
  int orig_row = *row, orig_col = *col;
  if (!sheet->XYtoRowCol(x, y, row, col))
    return FALSE;

  if (row_selectable[*row] && col_selectable[*col])
    return TRUE;
  else {
    *row = orig_row;
    *col = orig_col;
    return FALSE;
  }
}

void SpreadSheet::GetSize(int *w, int *h)
{
  sheet->DesiredSize(w, h);
}

void SpreadSheet::SetSize(int xs, int ys, int xe, int ye)
{
  sheet->SetSize(xs, ys, xe, ye);
}

void SpreadSheet::CheckSize(void)
{
  sheet->CheckScrollbars();
}

void SpreadSheet::GetLabelExtent(char *str, float *x, float *y)  
{ sheet->GetLabelExtent(str, x, y); }

void SpreadSheet::GetDataExtent(float *x, float *y, const char *str /*= "W"*/) 
{ sheet->GetDataExtent(x, y, str); }

void SpreadSheet::SetActive(Bool _s)
{
  sheet->Show(_s);
  if (_s) {
    sheet->SetFocus();
    sheet->OnPaint();
  }
  active = _s;
}

void SpreadSheet::SetValue(int row, int col, const gText &s)
{
  data(row, col) = s;
  data(row, col).Entered(TRUE);
}

const gText &SpreadSheet::GetValue(int row, int col) const
{ return data(row, col).GetValue(); }

int SpreadSheet::CurRow(void)
{ return sheet->Row(); }

int SpreadSheet::CurCol(void)
{ return sheet->Col(); }

void SpreadSheet::SetCurRow(int r)
{ if (row_selectable[r])  sheet->SetRow(r);    }

void SpreadSheet::SetCurCol(int c) 
{ if (col_selectable[c])  sheet->SetCol(c);    }

void SpreadSheet::Print(wxOutputMedia device, wxOutputOption fit) 
{ sheet->Print(device, fit); }

void SpreadSheet::Repaint(void) { sheet->OnPaint(); }

void SpreadSheet::SetFocus(void) { sheet->SetFocus(); }

void SpreadSheet::Output(gOutput &o) const
{
    for (int i = 1; i <= rows; i++)
    {
        for (int j = 1; j <= cols; j++) 
            o << gPlainText(data(i, j).GetValue()) << ' ';

        o << '\n';
    }
}


