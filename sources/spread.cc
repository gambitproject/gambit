#include <stdio.h>
#include "wx.h"
#include "wx_mf.h"
#pragma		hdr_stop
#include	"general.h"
#include 	"wxmisc.h"
#include	"spread.h"
#include 	"gconvert.h"

//Global GDI objects
wxPen		*grid_line_pen;
wxPen		*grid_border_pen;
wxPen		*s_selected_pen;
wxPen		*s_hilight_pen;
wxPen		*s_white_pen;
wxBrush	*s_white_brush;
wxBrush *s_hilight_brush;

gOutput &operator<<(gOutput &op,const SpreadSheet3D &s) {return op;}
gOutput &operator<<(gOutput &op,const SpreadSheet &s) {return op;}
gOutput &operator<<(gOutput &op,const SpreadDataCell &c) {return op;}

//****************************************************************************
//*                              SPREAD SHEET DRAW SETTINGS                  *
//****************************************************************************
SpreadSheetDrawSettings::SpreadSheetDrawSettings(SpreadSheet3D *_parent,int cols):
		col_width(cols)
{
ref_cnt=1;
parent=_parent;
if (!LoadOptions())
{
	row_height=DEFAULT_ROW_HEIGHT;
	default_col_width=DEFAULT_COL_WIDTH;
	col_dim_char=TRUE;
	labels=0;
	vert_fit=TRUE;
	data_font=wxTheFontList->FindOrCreateFont(11,wxMODERN,wxNORMAL,wxNORMAL);
	label_font=wxTheFontList->FindOrCreateFont(12,wxMODERN,wxNORMAL,wxNORMAL);
	SaveOptions();
}

for (int i=1;i<=col_width.Length();i++) col_width[i]=DEFAULT_COL_WIDTH;
x_scroll=y_scroll=0;scrolling=FALSE;
x_start=-1;y_start=-1;
}

void	SpreadSheetDrawSettings::SetOptions(void)
{
Bool labels_col=ColLabels(),labels_row=RowLabels();
int horiz=col_width[1],vert=row_height;

MyDialogBox *options_dialog=new MyDialogBox((wxWindow *)parent,"Options");
options_dialog->Form()->Add(wxMakeFormMessage("Fonts"));
wxFormItem *lfont_but=wxMakeFormButton("Label",(wxFunction)spread_options_lfont_func);
options_dialog->Form()->Add(lfont_but);
wxFormItem *dfont_but=wxMakeFormButton("Data",(wxFunction)spread_options_dfont_func);
options_dialog->Form()->Add(dfont_but);
options_dialog->Form()->Add(wxMakeFormNewLine());
options_dialog->Form()->Add(wxMakeFormMessage("Cell size"));
options_dialog->Form()->Add(wxMakeFormNewLine());
options_dialog->Form()->Add(wxMakeFormShort("Horiz",&horiz,wxFORM_SLIDER,new wxList(wxMakeConstraintRange(0, 50), 0)));
options_dialog->Form()->Add(wxMakeFormBool("char",&horiz_fit));
wxStringList *column_list=new wxStringList;
char *col_str=new char[10];
column_list->Add("All");
column_list=wxStringListInts(col_width.Length(),column_list);
options_dialog->Form()->Add(wxMakeFormString("Col",&col_str,wxFORM_CHOICE,
	new wxList(wxMakeConstraintStrings(column_list),0)));
options_dialog->Form()->Add(wxMakeFormNewLine());
options_dialog->Form()->Add(wxMakeFormShort("Vert",&vert,wxFORM_SLIDER,new wxList(wxMakeConstraintRange(0, 50), 0)));
options_dialog->Form()->Add(wxMakeFormBool("Fit to font",&vert_fit));
options_dialog->Form()->Add(wxMakeFormNewLine());
options_dialog->Form()->Add(wxMakeFormMessage("Show Labels"));
options_dialog->Form()->Add(wxMakeFormBool("row",&labels_row));
options_dialog->Form()->Add(wxMakeFormBool("col",&labels_col));
options_dialog->Form()->Add(wxMakeFormNewLine());
Bool save=FALSE;
options_dialog->Form()->Add(wxMakeFormBool("Save now",&save));
options_dialog->Form()->AssociatePanel(options_dialog);
((wxButton *)dfont_but->GetPanelItem())->SetClientData((char *)this);
((wxButton *)lfont_but->GetPanelItem())->SetClientData((char *)this);
options_dialog->Go1();
if (options_dialog->Completed()==wxOK)
{
	int which_col=wxListFindString(column_list,col_str);
	SetColWidth(horiz,which_col);
	SetRowHeight(row_height);
	labels=0;
	if (labels_row) labels|=S_LABEL_ROW;
	if (labels_col) labels|=S_LABEL_COL;
	if (save) SaveOptions();
}
delete options_dialog;delete col_str;
parent->Redraw();
}

#pragma argsused		// turn off the ev not used message
void	SpreadSheetDrawSettings::spread_options_lfont_func(wxButton	&ob,wxEvent &ev)
{
SpreadSheetDrawSettings  *draw_settings=(SpreadSheetDrawSettings *)ob.GetClientData();
FontDialogBox *f=new FontDialogBox(NULL,draw_settings->GetLabelFont());
if (f->Completed()==wxOK)
	draw_settings->SetLabelFont(f->MakeFont());
delete f;
}

#pragma argsused		// turn off the ev not used message
void	SpreadSheetDrawSettings::spread_options_dfont_func(wxButton	&ob,wxEvent &ev)
{
SpreadSheetDrawSettings *draw_settings=(SpreadSheetDrawSettings *)ob.GetClientData();
FontDialogBox *f=new FontDialogBox(NULL,draw_settings->GetDataFont());
if (f->Completed()==wxOK)
	draw_settings->SetDataFont(f->MakeFont());
delete f;
}

void	SpreadSheetDrawSettings::SaveOptions(const char *s)
{
char *file_name;
const char *sn="SpreadSheet3D";	// section name
file_name=copystring((s) ? s : "gambit.ini");

wxWriteResource(sn,"Row-Height",row_height,file_name);
wxWriteResource(sn,"Default-Column-Width",default_col_width,file_name);
wxWriteResource(sn,"Col-Dim-Char",col_dim_char,file_name);
wxWriteResource(sn,"Fit-Text-Vert",vert_fit,file_name);
wxWriteResource(sn,"Fit-Text-Horiz",horiz_fit,file_name);
wxWriteResource(sn,"Show-Labels",show_labels,file_name);
wxWriteResource(sn,"Data-Font",wxFontToString(data_font),file_name);
wxWriteResource(sn,"Label-Font",wxFontToString(label_font),file_name);

delete [] file_name;
}

int	SpreadSheetDrawSettings::LoadOptions(const char *s)
{
char *file_name;
const char *sn="SpreadSheet3D";	// section name
file_name=copystring((s) ? s : "gambit.ini");
// now try finding this file in our path
wxPathList *path_list=new wxPathList;
path_list->AddEnvList("PATH");
file_name=path_list->FindValidPath(file_name);
if (file_name) file_name=copystring(file_name); else return 0;

char *font_str=new char[100];
wxGetResource(sn,"Row-Height",&row_height,file_name);
wxGetResource(sn,"Default-Column-Width",&default_col_width,file_name);
wxGetResource(sn,"Col-Dim-Char",&col_dim_char,file_name);
wxGetResource(sn,"Fit-Text-Vert",&vert_fit,file_name);
wxGetResource(sn,"Fit-Text-Horiz",&horiz_fit,file_name);
wxGetResource(sn,"Show-Labels",&show_labels,file_name);
wxGetResource(sn,"Data-Font",&font_str,file_name);
data_font=wxStringToFont(font_str);
wxGetResource(sn,"Label-Font",&font_str,file_name);
label_font=wxStringToFont(font_str);

delete [] file_name;
return 1;
}


// Column width
int	SpreadSheetDrawSettings::GetColWidth(int col)
{
if (!col) col=1;
if (horiz_fit)
	return col_width[col]*tw+2*TEXT_OFF;
else
	return col_width[col]*COL_WIDTH_UNIT;
}
void SpreadSheetDrawSettings::SetColWidth(int	_c,int col)
{
if
	(col) col_width[col]=_c;
else
	for (int i=1;i<=col_width.Length();i++) col_width[i]=_c;
}
// Font Size.  If the cell size is tied to the font size, i.e. if
// vert_fit is used, or if the cell width is measured in chars, we
// need to update these values every time the font changes
void SpreadSheetDrawSettings::UpdateFontSize(float x,float y)
{
if (x<0 && y<0) parent->GetDataExtent(&x,&y);
tw=(int)x;th=(int)y;
}


//****************************************************************************
//*                             SPREAD SHEET DATA SETTINGS                   *
//****************************************************************************
void SpreadSheetDataSettings::SetAutoLabelStr(const gString s,int what)
{
switch (what)
{
	case S_AUTO_LABEL_ROW 	:	auto_label_row=s; 	break;
	case S_AUTO_LABEL_COL 	:	auto_label_col=s; 	break;
	case S_AUTO_LABEL_LEVEL :	auto_label_level=s; break;
}
}
gString	SpreadSheetDataSettings::AutoLabelStr(int what) const
{
gString label;
switch (what)
{
	case S_AUTO_LABEL_ROW 	:	label=auto_label_row; 		break;
	case S_AUTO_LABEL_COL 	:	label=auto_label_col; 		break;
	case S_AUTO_LABEL_LEVEL :	label=auto_label_level; 	break;
	default									: label="";									break;
}
return label;
}

//****************************************************************************
//*                               SPREAD SHEET CANVAS                        *
//****************************************************************************

// Constructor
SpreadSheetC::SpreadSheetC(SpreadSheet *_sheet,wxFrame *parent,int x,int y,int w,int h): wxCanvas(parent,x,y,w,h,0)
{
top_frame=(SpreadSheet3D *)parent;
sheet=_sheet;
draw_settings=top_frame->DrawSettings();
data_settings=top_frame->DataSettings();
// Make sure that draw_settings knows about the current font size
float tw,th;
GetDataExtent(&tw,&th);
draw_settings->UpdateFontSize(tw,th);
// Allow double clicking on canvas
AllowDoubleClick(TRUE);
// Give myself some scrollbars if necessary
CheckScrollbars();
Show(FALSE);	// Do not update myself until told by the parent
}

int SpreadSheetC::MaxX(int col)
{
int temp=draw_settings->XStart();
if (col<0) col=sheet->GetCols();
for (int i=1;i<=col;i++)	temp+=draw_settings->GetColWidth(i);
return temp;
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
*w=min(MaxX(),MAX_SHEET_WIDTH);
*h=min(draw_settings->YStart()+(sheet->GetRows()+1)*draw_settings->GetRowHeight()+3,MAX_SHEET_HEIGHT);
*w=max(*w,MIN_SHEET_WIDTH);
*h=max(*h,MIN_SHEET_HEIGHT);
}

// Check Scrollbars
void	SpreadSheetC::CheckScrollbars(void)
{
int x_step=-1,y_step=-1;

if (MaxX()>MAX_SHEET_WIDTH)
	x_step=MaxX()/XSTEPS+5;
if (sheet->GetRows()*draw_settings->GetRowHeight()>MAX_SHEET_HEIGHT)
	y_step=(draw_settings->YStart()+(sheet->GetRows()+1)*draw_settings->GetRowHeight())/YSTEPS+5;

if (x_step>0 || y_step>0)
{
	// Note that due to a bug in SetClientSize, we must either have no or both scrollbars
	if(x_step<=0) x_step=MaxX()/XSTEPS+5;
	if(y_step<=0) y_step=(draw_settings->YStart()+(sheet->GetRows()+1)*draw_settings->GetRowHeight())/YSTEPS+5;
	if (x_step!=draw_settings->XScroll() || y_step!=draw_settings->YScroll())
	{
		((wxCanvas *)this)->SetScrollbars(x_step,y_step,XSTEPS,YSTEPS,4,4);
		draw_settings->SetXScroll(x_step);draw_settings->SetYScroll(y_step);
		draw_settings->SetScrolling(TRUE);
	}
}
if (x_step<0 && y_step<0 && draw_settings->Scrolling())
{
	((wxCanvas *)this)->SetScrollbars(x_step,y_step,XSTEPS,YSTEPS,4,4);
	draw_settings->SetXScroll(x_step);draw_settings->SetYScroll(y_step);
	draw_settings->SetScrolling(FALSE);
}

}

// Paint message handler
void SpreadSheetC::OnPaint(void)
{Update(*(GetDC()));}
// Mouse message handler
void SpreadSheetC::OnEvent(wxMouseEvent &ev)
{
if (ev.LeftDown() || ev.ButtonDClick())
{
	float x,y;
	ev.Position(&x,&y);
	cell.row=(int)((y-draw_settings->YStart())/draw_settings->GetRowHeight()+1);
	cell.col=0;int i=1;
	while (!cell.col && i<=sheet->GetCols())
		{if (x<MaxX(i)) cell.col=i;i++;}
	if (cell.row<1) cell.row=1;
	if (cell.row>sheet->GetRows()) cell.row=sheet->GetRows();
	if (cell.col<1) cell.col=1;
	if (cell.col>sheet->GetCols()) cell.col=sheet->GetCols();
	top_frame->OnSelectedMoved(cell.row,cell.col);
	if (ev.LeftDown() && !ev.ControlDown()) ProcessCursor(0);
	if (ev.ButtonDClick() || (ev.LeftDown() && ev.ControlDown()))
		top_frame->OnDoubleClick(cell.row,cell.col,sheet->GetLevel(),sheet->GetValue(cell.row,cell.col));
}
}

// Keyboard message handler
void SpreadSheetC::OnChar(wxKeyEvent &ev)
{
// Allow the default behavior to be overiden
if (top_frame->OnCharNew(ev)) return;
int ch=ev.KeyCode();
// Cursor keys to move the hilight
if (IsCursor(ev) || IsEnter(ev))	{ProcessCursor(ch);return;}
// F2 on the last row to add a row
if (ch==WXK_F2)
{
	if (data_settings->Change(S_CAN_GROW_ROW) && cell.row==sheet->GetRows()) top_frame->AddRow();
	return;
}
// F3 on the last column to add a column
if (ch==WXK_F3)
{
	if (data_settings->Change(S_CAN_GROW_COL) && cell.col==sheet->GetCols()) top_frame->AddCol();
	return;
}
// Otherwise, if editing is enabled, just process the key
{
	if (top_frame->Editable())
	{
		gSpreadValType cell_type=sheet->GetType(cell.row,cell.col);
		if ((cell_type==gSpreadNum && IsNumeric(ev)) ||	(cell_type==gSpreadStr && IsAlphaNum(ev)))
		{
			if (cell.editing==FALSE){cell.editing=TRUE;cell.str="";} // this implements 'overwrite'
			cell.str+=ch;
		}
		if (IsDelete(ev))
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
top_frame->OnSelectedMoved(cell.row,cell.col);
top_frame->SetStatusText(cell.str);
// Make sure the cursor is visible.  Note, do not if this was a mouse
// movement (ch=0)
if (draw_settings->Scrolling() && ch!=0)
{
	int x_scroll=0,y_scroll=0;
	if (draw_settings->YStart()+cell.row*draw_settings->GetRowHeight()>draw_settings->GetRealHeight())
		y_scroll=(draw_settings->YStart()+cell.row*draw_settings->GetRowHeight())/draw_settings->YScroll()-1;
	if (y_scroll<0) y_scroll=0;
	if (MaxX(cell.col-1)>draw_settings->GetRealWidth())
		x_scroll=MaxX(cell.col-1)/draw_settings->XScroll()-1;
	if (x_scroll<0) x_scroll=0;
	Scroll(x_scroll,y_scroll);
}
}

void SpreadSheetC::UpdateCell(wxDC &dc,SpreadCell &cell)
{
// Check for the validity of the cell/old_cell
cell.CheckValid(sheet->GetRows(),sheet->GetCols());
old_cell.CheckValid(sheet->GetRows(),sheet->GetCols());
// erase the old hilight
DrawCell(dc,old_cell.row,old_cell.col);
// draw the new hilight
dc.SetBrush(wxTRANSPARENT_BRUSH);
dc.SetPen(s_selected_pen);
dc.DrawRectangle(	MaxX(cell.col-1)+LINE_OFF,
									draw_settings->YStart()+(cell.row-1)*draw_settings->GetRowHeight()+LINE_OFF,
									draw_settings->GetColWidth(cell.col)-2*LINE_OFF,
									draw_settings->GetRowHeight()-2*LINE_OFF);
// Update the status line text on the topmost frame
top_frame->SetStatusText(cell.str);
// Save the new cell
old_cell=cell;
}

// Draw a cell
void SpreadSheetC::DrawCell(wxDC &dc,int row,int col)
{
dc.SetFont(draw_settings->GetDataFont());
dc.SetBackgroundMode(wxTRANSPARENT);
if (sheet->HiLighted(row,col))
	{dc.SetBrush(s_hilight_brush);dc.SetPen(s_hilight_pen);}
else
	{dc.SetBrush(s_white_brush);dc.SetPen(s_white_pen);}
if (sheet->Bold(row,col))
{
	wxFont *cur=draw_settings->GetDataFont();
	dc.SetFont(wxTheFontList->FindOrCreateFont(
		cur->GetPointSize(),cur->GetFamily(),
		cur->GetStyle(),wxBOLD,cur->GetUnderlined()));
}
dc.DrawRectangle(	MaxX(col-1)+LINE_OFF,
									draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+LINE_OFF,
									draw_settings->GetColWidth(col)-2*LINE_OFF,
									draw_settings->GetRowHeight()-2*LINE_OFF);
dc.SetClippingRegion(MaxX(col-1)+TEXT_OFF,
									draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF,
									draw_settings->GetColWidth(col)-2*TEXT_OFF,
									draw_settings->GetRowHeight()-2*TEXT_OFF);
gDrawText(dc,sheet->GetValue(row,col),
							MaxX(col-1)+TEXT_OFF,
							draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
dc.DestroyClippingRegion();
}

// Updating
void SpreadSheetC::Update(wxDC &dc)
{
int row,col;
// Draw the grid
if (dc.__type!=wxTYPE_DC_METAFILE) dc.Clear();
dc.SetBrush(wxTRANSPARENT_BRUSH);
dc.SetPen(grid_line_pen);
for (row=1;row<=sheet->GetRows();row++)
	dc.DrawLine(draw_settings->XStart(),draw_settings->YStart()+row*draw_settings->GetRowHeight(),
						 MaxX()+1,
						 draw_settings->YStart()+row*draw_settings->GetRowHeight());
for (col=1;col<=sheet->GetCols();col++)
	dc.DrawLine(MaxX(col),draw_settings->YStart(),
							MaxX(col),
							draw_settings->YStart()+sheet->GetRows()*draw_settings->GetRowHeight()+1);
dc.SetPen(grid_border_pen);
dc.SetBrush(wxTRANSPARENT_BRUSH);
dc.DrawRectangle(	draw_settings->XStart(),draw_settings->YStart(),
									MaxX()-draw_settings->XStart(),
									sheet->GetRows()*draw_settings->GetRowHeight()+2);
// Draw the labels if any
dc.SetFont(draw_settings->GetLabelFont());
if (draw_settings->RowLabels())
	for (row=1;row<=sheet->GetRows();row++)
		dc.DrawText(sheet->GetLabelRow(row),0,draw_settings->YStart()+(row-1)*draw_settings->GetRowHeight()+TEXT_OFF);
if (draw_settings->ColLabels())
	for (col=1;col<=sheet->GetCols();col++)
		dc.DrawText(sheet->GetLabelCol(col),MaxX(col-1)+TEXT_OFF,0);
// Fill in the cells
for (row=1;row<=sheet->GetRows();row++)
	for (col=1;col<=sheet->GetCols();col++)
		DrawCell(dc,row,col);

// Hilight the currently selected cell
UpdateCell(dc,cell);
}

void GetRes(wxDC &dc,float *horiz,float *vert)
{
int old_mode=dc.GetMapMode();
dc.SetMapMode(MM_METRIC);
int dev_x=dc.LogicalToDeviceX(10);
*horiz=dev_x/10.0;
int dev_y=dc.LogicalToDeviceY(10);
*vert=dev_y/10.0;
dc.SetMapMode(old_mode);
}

void SpreadSheetC::Print(int device)
{
if (device==wxMEDIA_PRINTER)
{
	#ifdef wx_msw
	wxPrinterDC dc_pr(NULL, NULL, NULL,FALSE);
	if (dc_pr.Ok())
	{
		// using scaling to achieve WYSIWYG look.  This uses the DPI of
		// the Printer and the DPI of the screen
		float res_scr_x,res_scr_y,res_prn_x,res_prn_y;
		GetRes(*GetDC(),&res_scr_x,&res_scr_y);
		GetRes(dc_pr,&res_prn_x,&res_prn_y);
		dc_pr.SetUserScale(res_prn_x/res_scr_x,res_prn_y/res_scr_y);
		dc_pr.StartDoc(sheet->GetLabel());
		dc_pr.StartPage();
		Update(dc_pr);
		dc_pr.EndPage();
		dc_pr.EndDoc();
	}
	#else
	wxMessageBox("Printing not supported under X");
	#endif
}
if (device==wxMEDIA_CLIPBOARD || device==wxMEDIA_METAFILE)
{
	#ifdef wx_msw
	char *metafile_name=NULL;
	if (device==wxMEDIA_METAFILE)
		metafile_name=copystring(wxFileSelector("Save Metafile",0,0,".wmf","*.wmf"));
	wxMetaFileDC dc_mf(metafile_name);
	if (dc_mf.Ok())
	{
		Update(dc_mf);
		wxMetaFile *mf = dc_mf.Close();
		if (mf)
		{
			Bool success=mf->SetClipboard((int)(dc_mf.MaxX()+10),(int)(dc_mf.MaxY()+10));
			if (!success) wxMessageBox("Copy Failed","Error",wxOK | wxCENTRE,this);
			delete mf;
		}
		if (device==wxMEDIA_METAFILE)
			wxMakeMetaFilePlaceable(metafile_name,0,0,(int)(dc_mf.MaxX()+10),(int)(dc_mf.MaxY()+10));
	}
	#else
	wxMessageBox("Metafiles not supported under X");
	#endif
}
if (device==wxMEDIA_PS)
{
	wxPostScriptDC dc_ps(NULL,TRUE);
	if (dc_ps.Ok())
	{
		dc_ps.StartDoc("");
		dc_ps.StartPage();
		Update(dc_ps);
		dc_ps.EndPage();
		dc_ps.EndDoc();
	}
}
}




//****************************************************************************
//*                               SPREAD SHEET                               *
//****************************************************************************

SpreadSheet::SpreadSheet(int _rows,int _cols,int _level,char *title,wxFrame *parent)
{
Init(_rows,_cols,_level,title,parent);
}

void SpreadSheet::Init(int _rows,int _cols,int _level,char *title,wxFrame *parent)
{
rows=_rows;cols=_cols;level=_level;
data=gRectBlock<SpreadDataCell>(rows,cols);
row_labels=gBlock<gString>(rows);
col_labels=gBlock<gString>(cols);
int h,w;
parent->GetClientSize(&w,&h);
sheet=new SpreadSheetC(this,parent,0,0,w,h-MIN_BUTTON_SPACE);
if (title) label=title; else label=" : #"+ToString(level);
}

void SpreadSheet::Clear(void)
{
for (int i=1;i<=rows;i++)
	for (int j=1;j<=cols;j++)
		data(i,j).Clear();
}

void SpreadSheet::AddRow(void)
{
int i;
// add a new row to the matrix
data.AddRow((const gArray<SpreadDataCell>)gArray<SpreadDataCell>(cols));
// Copy the cell types from the previous row
for (i=1;i<=cols;i++) data(rows+1,i).SetType(data(rows,i).GetType());
// add a new entry to the row_labels
rows++;
row_labels.Append((const gString)gString());
}

void SpreadSheet::AddCol(void)
{
// add a new column to the matrix
data.AddColumn((const gArray<SpreadDataCell>)gArray<SpreadDataCell>(rows));
// add a new entry to the col_labels
cols++;
col_labels.Append((const gString)gString());
}

void SpreadSheet::DelRow(int row)
{
if (rows<2) return;
if (row==0) row=rows;
// remove a row from the matrix
data.RemoveRow(row);
// remove an entry from the row_labels;
row_labels.Remove(rows);
rows--;
}

void SpreadSheet::DelCol(int col)
{
if (cols<2) return;
if (col==0) col=cols;
// remove a column from the matrix
data.RemoveColumn(col);
// remove an entry from the col_labels
col_labels.Remove(cols);
cols--;
}


void SpreadSheet::SetSize(int xs,int ys,int xe,int ye)
{
sheet->SetSize(xs,ys,xe,ye);
}


//****************************************************************************
//*                           SPREAD SHEET 3D                                *
//****************************************************************************
SpreadSheet3D::SpreadSheet3D(int rows,int cols,int _levels,char *title,
					wxFrame *parent,unsigned int _features,SpreadSheetDrawSettings *drs,
					SpreadSheetDataSettings *dts):	wxFrame(parent,title)
{
assert(rows>0 && cols>0 && _levels>0 && "SpreadSheet3D::Bad Dimensions");
// Initialize some global GDI objects
grid_line_pen=wxThePenList->FindOrCreatePen("BLACK",1,wxDOT);
grid_border_pen=wxThePenList->FindOrCreatePen("BLUE",3,wxSOLID);
s_selected_pen=wxThePenList->FindOrCreatePen("GREEN",2,wxSOLID);
s_white_pen=wxThePenList->FindOrCreatePen("WHITE",2,wxSOLID);
s_hilight_pen=wxThePenList->FindOrCreatePen("LIGHT GREY",2,wxSOLID);
s_white_brush=wxTheBrushList->FindOrCreateBrush("WHITE",wxSOLID);
s_hilight_brush=wxTheBrushList->FindOrCreateBrush("LIGHT GREY",wxSOLID);
// Initialize the draw settings
draw_settings=(drs) ? drs : new SpreadSheetDrawSettings(this,cols);
draw_settings->SetParent(this);
data_settings=(dts) ? dts : new SpreadSheetDataSettings;
// Initialize local variables
completed=wxRUNNING;
editable=TRUE;
levels=_levels;
label=title;
features=_features;
// Create the levels,  must do in two steps since gList(int) is not defined
for (int i=1;i<=levels;i++) data.Append((const SpreadSheet)SpreadSheet());
for (i=1;i<=levels;i++) data[i].Init(rows,cols,i,0,this);
// Turn on level #1
cur_level=0;
SetLevel(1);
if (levels>1) features|=ANY_BUTTON;	// we need a panel for the slider
MakeFeatures();
CreateStatusLine(2);
// Size this frame according to the sheet dimentions
Resize();
}

void SpreadSheet3D::MakeFeatures(void)
{
//------------------make the panel---------------------------
if (features&ALL_BUTTONS) // Create the panel
{
	int h,w;
	panel_x=panel_y=0;
	panel_new_line=FALSE;
	GetClientSize(&w,&h);
	panel=new wxPanel(this,0,h-MIN_BUTTON_SPACE,w,MIN_BUTTON_SPACE,wxBORDER);

	if (levels>1) // create a slider to choose the active level
	{
		level_item=new wxSlider(panel,(wxFunction)SpreadSheet3D::spread_slider_func,NULL,1,1,levels,140);
		level_item->SetClientData((char *)this);
		panel->NewLine();
		SavePanelPos();
	}
	if (features&OK_BUTTON)
	{
		wxButton	*ok=AddButton("OK",(wxFunction)SpreadSheet3D::spread_ok_func);
		ok->SetClientData((char*)this);
	}
	if (features&CANCEL_BUTTON)
	{
		wxButton	*cancel=AddButton("Cancel",(wxFunction)SpreadSheet3D::spread_cancel_func);
		cancel->SetClientData((char*)this);
	}
	if (features&PRINT_BUTTON)
	{
		wxButton *print=AddButton("P",(wxFunction)SpreadSheet3D::spread_print_func);
		print->SetClientData((char*)this);
	}
	if (features&OPTIONS_BUTTON)
	{
		wxButton *options=AddButton("Config",(wxFunction)SpreadSheet3D::spread_options_func);
		options->SetClientData((char*)this);
	}
	if (features&CHANGE_BUTTON)
	{
		AddButtonNewLine();
		wxButton *change=AddButton("Grow/Shrink",(wxFunction)SpreadSheet3D::spread_change_func);
		change->SetClientData((char *)this);
	}
}
else
	panel=0;
	SetMenuBar(MakeMenuBar());
}

void SpreadSheet3D::OnMenuCommand(int id)
{
switch (id)
{
	case OUTPUT_MENU: OnPrint(); break;
	case CLOSE_MENU: OnOk(); break;
	case OPTIONS_MENU: DrawSettings()->SetOptions(); break;
	case CHANGE_MENU: break;
	case HELP_MENU_ABOUT: OnHelp(HELP_MENU_ABOUT); break;
	case HELP_MENU_CONTENTS: OnHelp(); break;
	default: wxMessageBox("Unknown"); break;
}
}
#pragma argsused		// turn off the _w,_h not used message
void SpreadSheet3D::OnSize(int _w,int _h)
{
int w,h;
GetClientSize(&w, &h);
if (panel) panel->SetSize(0,h-DrawSettings()->PanelSize(),w,DrawSettings()->PanelSize());
for (int i=1;i<=levels;i++) data[i].SetSize(0,0,w,h-DrawSettings()->PanelSize());
}

// Callback functions
#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_ok_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->OnOk();
}
void SpreadSheet3D::OnOk(void)
{
	SetCompleted(wxOK);
	Show(FALSE);
}

void SpreadSheet3D::OnPrint(void)
{
wxOutputDialogBox od;
if (od.Completed()==wxOK) Print(od.GetSelection());
}

#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_print_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->OnPrint();
}
#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_cancel_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->OnCancel();
}
void	SpreadSheet3D::OnCancel(void)
{
	SetCompleted(wxCANCEL);
	Show(FALSE);
}
#pragma argsused		// turn off the ev not used message
void SpreadSheet3D::spread_slider_func(wxSlider &ob,wxCommandEvent &ev)
{
((SpreadSheet3D *)ob.GetClientData())->SetLevel(ob.GetValue());
}

#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_change_func(wxButton	&ob,wxEvent &ev)
{
SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
// Create the Grow/Shrink dialog box
MyDialogBox *gs=new MyDialogBox(0,"Grow/Shrink");
char *choices[6]={"Add Row","Add Column","Add Level","Del Row","Del Column","Del Level"};
wxRadioBox *rb=new wxRadioBox(gs,NULL,NULL,-1,-1,-1,-1,6,choices,2);
if (!parent->DataSettings()->Change(S_CAN_GROW_ROW)) rb->Enable(0,FALSE);
if (!parent->DataSettings()->Change(S_CAN_GROW_COL)) rb->Enable(1,FALSE);
if (!parent->DataSettings()->Change(S_CAN_GROW_LEVEL)) rb->Enable(2,FALSE);
if (!parent->DataSettings()->Change(S_CAN_SHRINK_ROW)) rb->Enable(3,FALSE);
if (!parent->DataSettings()->Change(S_CAN_SHRINK_COL)) rb->Enable(4,FALSE);
if (!parent->DataSettings()->Change(S_CAN_SHRINK_LEVEL)) rb->Enable(5,FALSE);

gs->Go();
if (gs->Completed()==wxOK)
{
	switch (rb->GetSelection())
	{
	case	0: parent->AddRow(); 		break;
	case	1: parent->AddCol(); 		break;
	case	2: parent->AddLevel();	break;
	case	3: parent->DelRow();		break;
	case	4: parent->DelCol();		break;
	case	5: parent->DelLevel();	break;
	}
}
delete gs;
}

#pragma argsused		// turn off the ev not used message
void	SpreadSheet3D::spread_options_func(wxButton	&ob,wxEvent &ev)
{
	SpreadSheet3D *parent=(SpreadSheet3D *)ob.GetClientData();
	parent->DrawSettings()->SetOptions();
}


void SpreadSheet3D::DelLevel(void)
{
if (levels<2) return;
data.Remove(levels);
levels--;
delete level_item;
level_item=new wxSlider(panel,(wxFunction)SpreadSheet3D::spread_slider_func,NULL,1,1,levels,140);
level_item->SetClientData((char *)this);
Redraw();
}

void SpreadSheet3D::AddLevel(void)
{
data.Append(SpreadSheet());
levels++;
data[levels].Init(data[1].GetRows(),data[1].GetCols(),levels,NULL,this);
level_item=new wxSlider(panel,(wxFunction)SpreadSheet3D::spread_slider_func,NULL,1,1,levels,140);
level_item->SetClientData((char *)this);
Redraw();
}

void SpreadSheet3D::Dump(void)
{
gFileOutput	out("spread.out");
out<<levels<<"\n";
for (int i=1;i<=levels;i++){ data[i].Dump(out);out<<"\n\n";}
}

void SpreadSheet3D::SetLevel(int _l)
{
assert(_l>0&&_l<=levels);
if (cur_level) data[cur_level].SetActive(FALSE);
cur_level=_l;
data[cur_level].SetActive(TRUE);
SetTitle(label+":"+data[cur_level].GetLabel());
}

void SpreadSheet3D::SetLabelRow(int row,const gString &s,int level)
{
if (level==0)
	for (level=1;level<=levels;level++) data[level].SetLabelRow(row,s);
else
	data[level].SetLabelRow(row,s);
}
void SpreadSheet3D::SetLabelCol(int col,const gString &s,int level)
{
if (level==0)
	for (level=1;level<=levels;level++)
		data[level].SetLabelCol(col,s);
else
	data[level].SetLabelCol(col,s);
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
		data[1].GetLabelExtent(data[cur_level].GetLabelRow(i),&w,&h);
		if (w>max_w) max_w=(int)w;
	}
	draw_settings->SetXStart(max_w+3);
}
if (draw_settings->ColLabels())
{
	for (i=1;i<=data[1].GetCols();i++)
	{
		data[1].GetLabelExtent(data[cur_level].GetLabelCol(i),&w,&h);
		if (h>max_h) max_h=(int)h;
	}
	draw_settings->SetYStart(max_h+3);
}
}

void SpreadSheet3D::Resize(void)
{
int w,h,w1,h1;
data[cur_level].GetSize(&w,&h);
for (int i=1;i<=data.Length();i++) data[i].CheckSize();
Panel()->Fit();Panel()->GetSize(&w1,&h1);
w=max(w,w1);
h1=max(h1,MIN_BUTTON_SPACE);
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

wxButton *SpreadSheet3D::AddButton(const char *label,wxFunction fun)
{
#ifdef wx_msw
assert(panel);
panel->RealAdvanceCursor();
SavePanelPos();
if (panel_new_line)
	{panel_y+=40;panel_x=PANEL_LEFT_MARGIN;panel_new_line=FALSE;}
wxButton *button=new wxButton(panel,fun,(char *)label,panel_x,panel_y);
#else
if (panel_new_line) panel->NewLine();
wxButton *button=new wxButton(panel,fun,(char *)label);
#endif

return button;
}
wxPanel *SpreadSheet3D::AddPanel(void)
{
#ifdef wx_msw
panel->RealAdvanceCursor();
SavePanelPos();
if (panel_new_line)
	{panel_y+=40;panel_x=PANEL_LEFT_MARGIN;panel_new_line=FALSE;}
wxPanel *sub_panel=new wxPanel(panel,panel_x,panel_y);
return sub_panel;
#else
//if (panel_new_line) panel->NewLine();
//wxPanel *sub_panel=new wxPanel(panel);
panel->NewLine();
return panel;
#endif
}
/*
void SpreadSheet3D::AddMenu(wxMenu *submenu,const char *label)
{
assert(menubar);		// make sure a menubar exists
menubar->Append(submenu,(char *)label);
SetMenuBar(menubar);
}
*/
wxMenuBar *SpreadSheet3D::MakeMenuBar(void)
{
wxMenuBar *tmp_menubar=0;
//-------------------------------make menus----------------------------
if (features&ALL_MENUS)
{
	tmp_menubar=new wxMenuBar;
	wxMenu *file_menu=0;
	if (features&(OUTPUT_MENU|CLOSE_MENU)) file_menu=new wxMenu;
	if (features&OUTPUT_MENU)
		file_menu->Append(OUTPUT_MENU,"Out&put","Output to any device");
	if (features&CLOSE_MENU)
		file_menu->Append(CLOSE_MENU,"&Close","Exit");
	if (file_menu) tmp_menubar->Append(file_menu,"&File");
	wxMenu *display_menu=0;
	if (features&(OPTIONS_MENU|CHANGE_MENU)) display_menu=new wxMenu;
	if (features&OPTIONS_MENU)
		display_menu->Append(OPTIONS_MENU,"&Options","Configure display options");
	if (features&CHANGE_MENU)
		display_menu->Append(CHANGE_MENU,"&Change","Change sheet dimentions");
	if (display_menu) tmp_menubar->Append(display_menu,"&Display");
	if (features&HELP_MENU)
	{
		wxMenu *help_menu=new wxMenu;
		help_menu->Append(HELP_MENU_ABOUT,"&About");
		help_menu->Append(HELP_MENU_CONTENTS,"&Contents");
		tmp_menubar->Append(help_menu,"&Help");
	}
}
return tmp_menubar;
}

void SpreadSheet3D::SetMenuBar(wxMenuBar *bar)
{
menubar=bar;
if (menubar) wxFrame::SetMenuBar(menubar);
}

