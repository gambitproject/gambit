#include <stdio.h>
#include "wx.h"
#pragma		hdr_stop
#include	"general.h"
#include	"spread.h"

//Global GDI objects
wxPen		*grid_line_pen;
wxPen		*grid_border_pen;
wxPen		*hilight_pen;
wxPen		*lolight_pen;
wxBrush	*s_white_brush;
wxFont	*label_font;

gOutput &operator<<(gOutput &op,const SpreadSheet3D &s) {};
gOutput &operator<<(gOutput &op,const SpreadSheet &s) {};
gOutput &operator<<(gOutput &op,const SpreadDataCell &c) {};
//**************************** SPREAD SHEET SETTINGS ***********************
SpreadSheetDrawSettings::SpreadSheetDrawSettings(void)
{
row_height=DEFAULT_ROW_HEIGHT;
col_width=DEFAULT_COL_WIDTH;
labels=0;
data_font=wxTheFontList->FindOrCreateFont(10,wxMODERN,wxNORMAL,wxNORMAL);
x_scroll=y_scroll=0;scrolling=FALSE;
x_start=-1;y_start=-1;
}

//**************************** SPREAD SHEET CANVAS ***********************
// Constructor
SpreadSheetC::SpreadSheetC(SpreadSheet *_sheet,wxFrame *parent,int x,int y,int w,int h): wxCanvas(parent,x,y,w,h)
{
top_frame=(SpreadSheet3D *)parent;
sheet=_sheet;
draw_settings=top_frame->DrawSettings();
data_settings=top_frame->DataSettings();
// Give myself some scrollbars if necessary
int x_step=-1,y_step=-1;
if (sheet->GetCols()*draw_settings->GetColWidth()>MAX_SHEET_WIDTH)
	x_step=sheet->GetCols()*draw_settings->GetColWidth()/XSTEPS+1;
if (sheet->GetRows()*draw_settings->GetRowHeight()>MAX_SHEET_HEIGHT)
	y_step=sheet->GetRows()*draw_settings->GetRowHeight()/YSTEPS+1;

if (x_step>0 || y_step>0)
{
	// Note that due to a bug in SetClientSize, we must either have no or both scrollbars
	if(x_step<=0) x_step=sheet->GetCols()*draw_settings->GetColWidth()/XSTEPS+1;
	if(y_step<=0)y_step=sheet->GetRows()*draw_settings->GetRowHeight()/YSTEPS+1;
	SetScrollbars(x_step,y_step,XSTEPS,YSTEPS,4,4);
	draw_settings->SetXScroll(x_step);draw_settings->SetYScroll(y_step);
	draw_settings->SetScrolling(TRUE);
}
Show(FALSE);	// Do not update myself until told by the parent
}

void SpreadSheetC::OnSize(int _w,int _h)
{
int h,w;
GetVirtualSize(&w,&h);
if (w==0) draw_settings->SetWidth(_w); else draw_settings->SetWidth(w);
if (h==0) draw_settings->SetHeight(_h); else draw_settings->SetHeight(h);
draw_settings->SetRealHeight(_h);
draw_settings->SetRealWidth(_w);
}

// Desired Size
void	SpreadSheetC::DesiredSize(int *w,int *h)
{
*w=min(draw_settings->XStart()+sheet->GetCols()*draw_settings->GetColWidth()+3,MAX_SHEET_WIDTH);
*h=min(draw_settings->YStart()+(sheet->GetRows()+1)*draw_settings->GetRowHeight()+1,MAX_SHEET_HEIGHT);
*w=max(*w,MIN_SHEET_WIDTH);
*h=max(*h,MIN_SHEET_HEIGHT);
}
// Paint message handler
void SpreadSheetC::OnPaint(void)
{Update(*(GetDC()));}
// Mouse message handler
void SpreadSheetC::OnEvent(wxMouseEvent &ev)
{
if (ev.LeftDown())
{
	float x,y;
	ev.Position(&x,&y);
	cell.row=(y-draw_settings->YStart())/draw_settings->GetRowHeight()+1;
	cell.col=(x-draw_settings->XStart())/draw_settings->GetColWidth()+1;
  if (cell.row<1) cell.row=1;
	if (cell.col<1) cell.col=1;
	ProcessCursor(0);
}
}

// Keyboard message handler
void SpreadSheetC::OnChar(wxKeyEvent &ev)
{
int ch=ev.KeyCode();
// Cursor keys to move the hilight
if (IsCursor(ch))	{ProcessCursor(ch);return;}
// Ctrl-Enter on the last row to add a row
if (ch==106 && ev.ControlDown() && !ev.ShiftDown())
{
	if (data_settings->GrowRow() && cell.row==sheet->GetRows()) top_frame->AddRow();
	return;
}
// Ctrl-Shift-Enter on the last column to add a column
if (ch==106 && ev.ControlDown() && ev.ShiftDown())
{
	if (data_settings->GrowCol() && cell.col==sheet->GetCols()) top_frame->AddCol();
	return;
}
// Otherwise, if editing is enabled, just process the key
{
	if (top_frame->Editable())
  {
		if (((*sheet)[cell.row][cell.col].GetType()==gSpreadNum && IsNumeric(ch)) ||
				((*sheet)[cell.row][cell.col].GetType()==gSpreadStr && IsAlphaNum(ch)))
		{
			if (cell.editing==FALSE) cell.editing=TRUE;
			cell.str+=ch;
		}
		if (IsDelete(ch) && ev.ControlDown())
		{
			if (cell.editing==FALSE) cell.editing=TRUE;
			cell.str.remove(cell.str.length()-1);
		}
		top_frame->SetStatusText(cell.str);
	}
}
}
void SpreadSheetC::ProcessCursor(int ch)
{
if (cell.editing && ch!=0) sheet->SetValue(cell.row,cell.col,cell.str);
switch (ch)
{
case	WXK_UP:
			if (cell.row>1) cell.row--;break;
case 	WXK_DOWN:
			if (cell.row<sheet->GetRows()) cell.row++;break;
case	WXK_RIGHT:
			if (cell.col<sheet->GetCols()) cell.col++;break;
case WXK_LEFT:
			if (cell.col>1) cell.col--;	break;
default:
			break;
}
cell.Reset(sheet->GetValue(cell.row,cell.col));
UpdateCell(*(GetDC()),cell);
top_frame->SetStatusText(cell.str);
// Make sure the cursor is visible
if (draw_settings->Scrolling())
{
	int x_scroll=0,y_scroll=0;
	if (draw_settings->YStart()+cell.row*draw_settings->GetRowHeight()>draw_settings->GetRealHeight())
		y_scroll=(draw_settings->YStart()+cell.row*draw_settings->GetRowHeight())/draw_settings->YScroll()-1;
	if (y_scroll<0) y_scroll=0;
	if (draw_settings->XStart()+cell.col*draw_settings->GetColWidth()>draw_settings->GetRealWidth())
		x_scroll=(draw_settings->YStart()+cell.col*draw_settings->GetColWidth())/draw_settings->XScroll()-1;
	if (x_scroll<0) x_scroll=0;
	Scroll(x_scroll,y_scroll);
}
}

void SpreadSheetC::UpdateCell(wxDC &dc,SpreadCell &cell)
{
// erase the old hiligth
dc.SetBrush(s_white_brush);
dc.SetPen(lolight_pen);
dc.DrawRectangle(	draw_settings->XStart()+(old_cell.col-1)*draw_settings->GetColWidth()+LINE_OFF,
									draw_settings->YStart()+(old_cell.row-1)*draw_settings->GetRowHeight()+LINE_OFF,
									draw_settings->GetColWidth()-LINE_OFF,
									draw_settings->GetRowHeight()-LINE_OFF);
// draw the new hilight
dc.SetBrush(wxTRANSPARENT_BRUSH);
dc.SetPen(hilight_pen);
dc.DrawRectangle(	draw_settings->XStart()+(cell.col-1)*draw_settings->GetColWidth()+LINE_OFF,
									draw_settings->YStart()+(cell.row-1)*draw_settings->GetRowHeight()+LINE_OFF,
									draw_settings->GetColWidth()-LINE_OFF,
									draw_settings->GetRowHeight()-LINE_OFF);
// Update the value in the old_cell
dc.DrawText(sheet->GetValue(old_cell.row,old_cell.col),
						draw_settings->XStart()+(old_cell.col-1)*draw_settings->GetColWidth()+TEXT_OFF,
						draw_settings->YStart()+(old_cell.row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
// Save the new cell
old_cell=cell;
}


// Updating
void SpreadSheetC::Update(wxDC &dc)
{
int row,col;
// Draw the grid
dc.Clear();
dc.SetBrush(wxTRANSPARENT_BRUSH);
dc.SetPen(grid_line_pen);
for (row=1;row<=sheet->GetRows();row++)
	dc.DrawLine(draw_settings->XStart(),draw_settings->YStart()+row*draw_settings->GetRowHeight(),
						 draw_settings->XStart()+sheet->GetCols()*draw_settings->GetColWidth()+1,
						 draw_settings->YStart()+row*draw_settings->GetRowHeight());
for (col=1;col<=sheet->GetCols();col++)
	dc.DrawLine(draw_settings->XStart()+col*draw_settings->GetColWidth(),draw_settings->YStart(),
							draw_settings->XStart()+col*draw_settings->GetColWidth(),
							draw_settings->YStart()+sheet->GetRows()*draw_settings->GetRowHeight()+1);
dc.SetPen(grid_border_pen);
dc.DrawRectangle(	draw_settings->XStart(),draw_settings->YStart(),
									sheet->GetCols()*draw_settings->GetColWidth()+2,
									sheet->GetRows()*draw_settings->GetRowHeight()+2);
// Draw the labels if any
dc.SetFont(label_font);
if (draw_settings->RowLabels())
	for (row=1;row<=sheet->GetRows();row++)
		dc.DrawText(sheet->GetLabelRow(row),0,draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
if (draw_settings->ColLabels())
	for (col=1;col<=sheet->GetCols();col++)
		dc.DrawText(sheet->GetLabelCol(col),draw_settings->XStart()+(col-1)*draw_settings->GetColWidth()+TEXT_OFF,0);
// Fill in the cells
dc.SetFont(draw_settings->GetFont());
for (row=1;row<=sheet->GetRows();row++)
	for (col=1;col<=sheet->GetCols();col++)
	{
		dc.DrawText(sheet->GetValue(row,col),
							draw_settings->XStart()+(col-1)*draw_settings->GetColWidth()+TEXT_OFF,
							draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
	}

// Hilight the currently selected cell
UpdateCell(dc,cell);
}

void SpreadSheetC::Print(void)
{
#ifdef wx_msw
wxPrinterDC dc(NULL, NULL, NULL);
if (dc.Ok())
{
	dc.StartDoc(sheet->GetLabel());
  dc.StartPage();
	Update(dc);
  dc.EndPage();
	dc.EndDoc();
}
#else
wxMessageBox("Printing not supported under X");
#endif
}



//************************ SPREAD SHEET ***********************************
SpreadSheet::SpreadSheet(int _rows,int _cols,int _level,char *title,wxFrame *parent)
{
rows=_rows;cols=_cols;level=_level;
data=gMatrix1<SpreadDataCell>(rows,cols);
row_labels=gTuple<gString>(1,rows);
col_labels=gTuple<gString>(1,cols);
int h,w;
parent->GetClientSize(&w,&h);
sheet=new SpreadSheetC(this,parent,0,0,w,h-DEFAULT_BUTTON_SPACE);
if (title)
	label=title;
else
{
	char tmp[20];
	sprintf(tmp," : #%2d",level);
	label=tmp;
}
}

void SpreadSheet::Init(int _rows,int _cols,int _level,char *title,wxFrame *parent)
{
rows=_rows;cols=_cols;level=_level;
data=gMatrix1<SpreadDataCell>(rows,cols);
row_labels=gTuple<gString>(1,rows);
col_labels=gTuple<gString>(1,cols);
int h,w;
parent->GetClientSize(&w,&h);
sheet=new SpreadSheetC(this,parent,0,0,w,h-DEFAULT_BUTTON_SPACE);
if (title)
	label=title;
else
{
	char tmp[20];
	sprintf(tmp," : #%2d",level);
	label=tmp;
}
}
void SpreadSheet::SetSize(int xs,int ys,int xe,int ye)
{
sheet->SetSize(xs,ys,xe,ye);
}

//**************************** SPREAD SHEET 3D *****************************
SpreadSheet3D::SpreadSheet3D(int rows,int cols,int _levels,char *title,wxFrame *parent,Bool _printable):
		wxFrame(parent,title)
{
assert(rows>0);assert(cols>0);assert(_levels>0);
// Initialize some global GDI objects
grid_line_pen=wxThePenList->FindOrCreatePen("BLACK",1,wxDOT);
grid_border_pen=wxThePenList->FindOrCreatePen("BLUE",3,wxSOLID);
hilight_pen=wxThePenList->FindOrCreatePen("GREEN",2,wxSOLID);
lolight_pen=wxThePenList->FindOrCreatePen("WHITE",2,wxSOLID);
s_white_brush=wxTheBrushList->FindOrCreateBrush("WHITE",wxSOLID);
label_font=wxTheFontList->FindOrCreateFont(12,wxSWISS,wxNORMAL,wxNORMAL);
// Initialize the draw settings
draw_settings=new SpreadSheetDrawSettings;
data_settings=new SpreadSheetDataSettings;
// Initialize local variables
completed=wxRUNNING;
editable=TRUE;
levels=_levels;
label=title;
data=gTuple<SpreadSheet>(1,levels);
// have to do this in two steps since arrays can not take constructors
for (int i=1;i<=levels;i++) data[i].Init(rows,cols,i,NULL,this);
// Turn on level #1
cur_level=0;
SetLevel(1);
// Create the panel
int h,w,v_spacing=0;
GetClientSize(&w,&h);
panel=new wxPanel(this,0,h-DEFAULT_BUTTON_SPACE,w,DEFAULT_BUTTON_SPACE,wxBORDER);
if (levels>1)	// create a slider to choose the active level
{
	level_item=new wxSlider(panel,(wxFunction)SpreadSheet3D::spread_slider_func,NULL,1,1,levels,140);
	level_item->SetClientData((char *)this);
	panel->NewLine();
  v_spacing+=10;
}
wxButton	*ok=new wxButton(panel,(wxFunction)SpreadSheet3D::spread_ok_func,"OK");
wxButton	*cancel=new wxButton(panel,(wxFunction)SpreadSheet3D::spread_cancel_func,"Cancel");
if (_printable)
{
	wxButton	*print=new wxButton(panel,(wxFunction)SpreadSheet3D::spread_print_func,"P");
	print->SetClientData((char*)this);
}
v_spacing+=25;
panel->SetVerticalSpacing(v_spacing);
panel->NewLine();
ok->SetClientData((char*)this);
cancel->SetClientData((char*)this);
CreateStatusLine(2);
// Size this frame according to the sheet dimentions
Resize();
}

#pragma argsused		// turn off the _w,_h not used message
void SpreadSheet3D::OnSize(int _w,int _h)
{
int w,h;
GetClientSize(&w, &h);
panel->SetSize(0,h-DrawSettings()->PanelSize(),w,DrawSettings()->PanelSize());
for (int i=1;i<=levels;i++) data[i].SetSize(0,0,w,h-DrawSettings()->PanelSize());
}                                      

// Callback functions
#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_ok_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
  parent->SetCompleted(wxOK);
	parent->Show(FALSE);
}
#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_print_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->Print();
}
#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_cancel_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->SetCompleted(wxCANCEL);
	parent->Show(FALSE);
}
#pragma argsused		// turn off the ev not used message
void SpreadSheet3D::spread_slider_func(wxSlider &ob,wxCommandEvent &ev)
{
((SpreadSheet3D *)ob.GetClientData())->SetLevel(ob.GetValue());
}

void SpreadSheet3D::Dump(void)
{
gOutput	out("spread.out");
out<<levels<<"\n";
for (int i=1;i<=levels;i++){ data[i].Dump(out);out<<"\n\n";}
}

void SpreadSheet3D::SetLevel(int _l)
{
assert(_l>0&&_l<=levels);
if (cur_level) data[cur_level].SetActive(FALSE);
cur_level=_l;
data[cur_level].SetActive(TRUE);
gString tmp=label+':'+data[cur_level].GetLabel();
SetTitle(tmp);
}

void SpreadSheet3D::FitLabels(void)
{
float w,h;
int	max_w=-1,max_h=-1;
int i;
if (draw_settings->RowLabels())
{
	for (i=1;i<=data[1].GetRows();i++)
	{
		GetTextExtent(data[cur_level].GetLabelRow(i),&w,&h);
		if (w>max_w) max_w=w;
	}
  draw_settings->SetXStart(max_w+3);
}
if (draw_settings->ColLabels())
{
	for (i=1;i<=data[1].GetCols();i++)
	{
		GetTextExtent(data[cur_level].GetLabelCol(i),&w,&h);
		if (h>max_h) max_h=h;
	}
	draw_settings->SetYStart(max_h+3);
}
}

void SpreadSheet3D::Resize(void)
{
int w,h,w1,h1;
data[cur_level].GetSize(&w,&h);
Panel()->Fit();Panel()->GetSize(&w1,&h1);
DrawSettings()->SetPanelSize(h1);
Panel()->SetSize(0,h-DrawSettings()->PanelSize(),w,DrawSettings()->PanelSize());
SetClientSize(w,h+DrawSettings()->PanelSize());
}


void SpreadSheet3D::Redraw(void)
{
char tmp[100];
int i;
if (data_settings->AutoLabel(S_AUTO_LABEL_ROW))
{
	for (i=1;i<=data[1].GetRows();i++)
	{
		sprintf(tmp,data_settings->AutoLabelStr(S_AUTO_LABEL_ROW),i);
		SetLabelRow(i,tmp);
	}
}
if (data_settings->AutoLabel(S_AUTO_LABEL_COL))
{
	for (i=1;i<=data[1].GetCols();i++)
	{
		sprintf(tmp,data_settings->AutoLabelStr(S_AUTO_LABEL_COL),i);
		SetLabelCol(i,tmp);
	}
}
FitLabels();
Resize();
}
