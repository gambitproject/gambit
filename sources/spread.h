// File: Spread.h, header file for spread.cc
// This file implements a 3-D spreadsheet type dialog box.  It can have
// unlimited dimensions for both rows/columns and the number of layers.
// At the moment each cell contains only a gString, but functions for
// converting gStrings to numbers, etc are available.  The spreadsheet
// implements movement by cursor keys and mouse input.  Each cell can have
// a type attribute which will determine if the cell is to accept numeric(default)
// input only or any alphanumeric input.  The spreadsheet can be made
// display only/disable editing by setting Editable attribute.  The labeling
// of rows and columns is possible through the use of LabelRow/Col functions
// or through the use of the AutoLabel feature.  The spreadsheet can be set
// to allow addition of rows, columns, levels through the SetChange funnction.  A row
// can be added at the end of the matrix by pressing F2 with the
// hilighed cell on the bottom row.  A column is added using F3. A level by F4.

// The control of the various features of the spreadsheet can be
// accomplished using either the menu bar, buttons or a combination of
// both.  The constructor takes an argument that determines which menus
// and buttons are to be created.  See SpreadSheetDrawSettings.

// Buttons:
// The following buttons are supported directly: OK, CANCEL, CHANGE,
// PRINT, HELP
// Optionally, a button can be added to execute the add row/col/level by
// using the '| CHANGE_BUTTON' in the buttons field of the constructor
// Output is supported through the Print command/Print button enabled by
// setting the '| PRINT_BUTTON' in the buttons field of the constructor.
// The user can create additional buttons on the spreadsheet by calling the
// AddButton function.

// Menus:
// The following menus are supported directly: File: CLOSE, OUTPUT,
// Display: OPTIONS, CHANGE, Help: HELP
// Note: Menus are tricky since it is currently impossible to dynamically
// add a menu to a menubar (it is possible to add a menuiterm to a menu).
// So, if you wish to add items to a menubar, you have to:
// call wxMenuBar *tmp_bar=MakeMenuBar();
// add your menus, call SetMenuBar(tmp_bar);

// The physical dimentions of each data cell can be controlled by the
// user.  Each column can have a different width and all rows have the
// same height.  The size can be measured in either screen pixels or be
// tied to the size of the font(*).  In that case, each unit corresponds
// to the width of a letter, and is scaled automatically when font is
// changed.  The row height can also be made to scale w/ the font by
// checking the vert_fit(*) option.

// The spreadsheet now supports the gDrawText extended text format, which
// allows for multiple colors to be used in the same cell.  See wxmisc

// In order to make this class as widely useable as possible, most functions
// are declared virtual for easy overiding.  Note that you can overide
// OnMenuCommand, OnCharNew (this receives events from the active canvas),
// OnSize, OnOk, OnCancel, OnDoubleClick (Ctrl-Click in X), and some others.
 
// $Id$

#ifndef	SPREAD_H
#define	SPREAD_H

#ifndef	wxRUNNING
#define	wxRUNNING	12345
#endif

#include "glist.h"
#include "gblock.h"
#include "grblock.h"
#include "gstring.h"

#define	XSTEPS							20    // Scroll steps horizontally
#define	YSTEPS							20		// Scroll steps vertically

#define	TEXT_OFF						8			// Offset of the text from the grid line
#define	LINE_OFF						3			// Offset of the border from the grid line
#define	MAX_SHEET_HEIGHT		400
#define MAX_SHEET_WIDTH			600
#define	MIN_SHEET_HEIGHT		20
#define MIN_SHEET_WIDTH			160
#define	DEFAULT_ROW_HEIGHT	30
#define	DEFAULT_COL_WIDTH		6
#define MIN_BUTTON_SPACE		60
#define	ROW_LABEL_WIDTH			30
#define	COL_LABEL_HEIGHT		30
#define	COL_WIDTH_UNIT			10

//********************** SPREAD SHEET DATA SETTINGS ***********************
// Both the DataSettings and DrawSettings employ a pseudo-reference
// counting mechanism.  Since it is desirable to be able to use the
// same settings for more than one spreadsheet, or to be able to
// recreate a spredsheet, a copy of these classes can be obtained by
// calling GetThis().  This will increment the internal reference
// count.  The spreadsheet should not delete the classes if ref_cnt
// is non-zero.
class SpreadSheetDataSettings
{
#define	S_CAN_GROW_ROW			1	// Or these together to allow size changes
#define	S_CAN_GROW_COL			2
#define S_CAN_GROW_LEVEL		4
#define	S_CAN_SHRINK_ROW		8
#define	S_CAN_SHRINK_COL		16
#define	S_CAN_SHRINK_LEVEL	32
#define	S_AUTO_LABEL_ROW		1
#define	S_AUTO_LABEL_COL		2
#define	S_AUTO_LABEL_LEVEL	4
private:
	int			change;
	int			auto_label;
	gString	auto_label_row;
	gString	auto_label_col;
	gString auto_label_level;
public:
	int ref_cnt;	// reference counter
	SpreadSheetDataSettings *GetThis(void) {ref_cnt++; return this;}
	// Constructor
	SpreadSheetDataSettings(void) 	{change=0;auto_label=0;}
	// Grow/Shrink the spreadsheet--add/remove rows, columns, and levels
	Bool	Change(int what)		{return change&what;}
	// Set the grow
	void	SetChange(int c)		{change=c;}
	// Automatic labeling for rows/columns.  Use SetAutoLabel to choose what
	// to label.  Then use SetAutoLabelStr to set the fmt parameter to the
	// sprintf(temp,fmt,row/col) function.
	int		AutoLabel(int l)	{return auto_label&l;}
	void	SetAutoLabel(int l)	{auto_label=l;}
	void	SetAutoLabelStr(const gString s,int what);
	gString	AutoLabelStr(int what) const;
};

//********************** SPREAD SHEET DRAW SETTINGS ***********************
class SpreadSheet3D;
class SpreadSheetDrawSettings
{
#define	S_LABEL_ROW	1
#define	S_LABEL_COL	2
private:
	SpreadSheet3D *parent;
	int 		row_height,default_col_width;
	int			tw,th;
	gBlock<int> col_width;
	int			total_height,total_width,real_height,real_width;
	int			x_scroll,y_scroll;
	int			x_start,y_start;
	int			panel_size;
	Bool		scrolling;
	Bool		vert_fit,horiz_fit,show_labels,col_dim_char;
	int			labels;
	wxFont	*data_font;
	wxFont  *label_font;
	// Functions to read/write the configuration file
	// Note: this always writes to file 'gambit.ini', section [SpreadSheet3D]
	void		SaveOptions(const char *s=NULL);
	int			LoadOptions(const char *s=NULL);
public:
	int ref_cnt;	// reference counter
	SpreadSheetDrawSettings *GetThis(void) {ref_cnt++; return this;}
	// Constructor
	SpreadSheetDrawSettings(SpreadSheet3D *_parent,int cols);
	// A way to set many options through a dialog
	void SetOptions(void);
		// static functions for the above
	static void spread_options_lfont_func(wxButton &ob,wxEvent &ev);
	static void spread_options_dfont_func(wxButton &ob,wxEvent &ev);
	// Allows to use a previously created settings w/ this parent
	void SetParent(SpreadSheet3D *_parent) {parent=_parent;}
	// Inform the class that a new column was added.
	void AddCol(void) {col_width.Append(DEFAULT_COL_WIDTH);}
	// Data Access, Get* functions
		// These functions control the dimentions of each cell i.e. Width X Height
	int	GetRowHeight(void)	{return ((vert_fit) ? (th+2*TEXT_OFF) : row_height);}
	int	GetColWidth(int col=0);
		// GetWidth/Height return the size of the virtual canvas if scrollbars are on (GetVirtualSize)
	int GetWidth(void)			{return total_width;}
	int GetHeight(void)			{return total_height;}
		// GetRealWidth/Height returns the physical size of the window (GetClientSize)
	int GetRealWidth(void)	{return real_width;}
	int GetRealHeight(void)	{return real_height;}
		// Controls what font to use for the cell contents [###: fix rowheight/colwidth to use the font]
	wxFont	*GetDataFont(void)	{return	data_font;}
	wxFont  *GetLabelFont(void) {return label_font;}
		// Start position of actual grid
	int			XStart(void)		{return (x_start==-1) ? RowLabels()*ROW_LABEL_WIDTH : x_start;}
	int			YStart(void)		{return (y_start==-1) ? ColLabels()*COL_LABEL_HEIGHT : y_start;}
		// These are nonzero if scrollbars are on.  Used to calculate scroll position
	int			XScroll(void)		{return x_scroll;}
	int			YScroll(void)		{return y_scroll;}
	Bool		Scrolling(void)	{return scrolling;}
		// These are used for row/column labeling
	int			RowLabels(void)	{return labels&S_LABEL_ROW;}
	int			ColLabels(void)	{return labels&S_LABEL_COL;}
		// Panel Sizing--allows the user to add items to the panel
	int			PanelSize(void)	{return panel_size;}
	void		SetPanelSize(int _s)	{panel_size=_s;}
		// Updates font information after change/on init
	void		UpdateFontSize(float w=-1,float h=-1);
		// Set* functions
	void	SetRowHeight(int _r)	{row_height=_r;}
	void	SetColWidth(int	_c,int col=0);
	void 	SetWidth(int _w)			{total_width=_w;}
	void	SetHeight(int _h)			{total_height=_h;}
	void	SetRealWidth(int _w)	{real_width=_w;}
	void	SetRealHeight(int _h)	{real_height=_h;}
	void	SetDataFont(wxFont *_f)	{data_font=_f;UpdateFontSize();}
	void	SetLabelFont(wxFont *_f){label_font=_f;UpdateFontSize();}
	void	SetXScroll(int _s)		{x_scroll=_s;}
	void	SetYScroll(int _s)		{y_scroll=_s;}
	void	SetScrolling(Bool _s)	{scrolling=_s;}
	void	SetLabels(int _l)			{labels=_l;}
	void	SetXStart(int _x)			{x_start=_x;}
	void	SetYStart(int _y)			{y_start=_y;}
};

//********************* SPREAD SHEET CANVAS *******************************
class SpreadSheet;
class SpreadSheet3D;
class SpreadSheetC: public wxCanvas
{
class SpreadCell {
								public:
								int 				row;
								int 				col;
								Bool				editing;
								gString 		str;
								SpreadCell(void)	{row=1;col=1;editing=FALSE;}
								~SpreadCell(void)	{}
								void Reset(const char *s=NULL)	{editing=FALSE;str=s;}
								void CheckValid(int rows,int cols)
								{if (row<1) row=1;if (col<1) col=1;if (row>rows) row=rows;if (col>cols) col=cols;}
								SpreadCell &operator=(const SpreadCell &cell)
								{row=cell.row;col=cell.col;str=cell.str;editing=FALSE;return (*this);}
								};
private:
	SpreadSheetDrawSettings 	*draw_settings;
	SpreadSheetDataSettings		*data_settings;
	SpreadSheet 				*sheet;
	SpreadSheet3D				*top_frame;
	SpreadCell					cell,old_cell;
	// functions
	void ProcessCursor(int ch);
	void UpdateCell(wxDC &dc,SpreadCell &cell);
	void DrawCell(wxDC &dc,int row,int col);
public:
	// Constructor
	SpreadSheetC(SpreadSheet *_sheet,wxFrame *parent,int x,int y,int w,int h);
	// Windows Events Handlers
	void	OnPaint(void);
	void	OnChar(wxKeyEvent &ch);
	void	OnEvent(wxMouseEvent &ev);
	void	Update(wxDC &dc);
	void 	OnSize(int _w,int _h);
	// Desired size--minimum size that would fit the sheet without scrolling
	// It is never less than MIN_SHEET_WIDTH and never greater than MAX_SHEET_WIDTH
	void	DesiredSize(int *w,int *h);
	int	 MaxX(int col=-1);
	// CheckScrollbars
	void	CheckScrollbars(void);
	// LabelExtent.  Since only the actual canvas can tell the extent...
	void GetLabelExtent(char *str,float *x,float *y)
		{SetFont(draw_settings->GetLabelFont());GetTextExtent(str,x,y);}
	// DataExtent
	void GetDataExtent(float *x,float *y,const char *str="W")
		{SetFont(draw_settings->GetDataFont());GetTextExtent(str,x,y);}
	// Data Access
	int		Row(void)	{return cell.row;}
	int		Col(void)	{return cell.col;}
	void	SetRow(int r)	{cell.row=r;ProcessCursor(0);}
	void 	SetCol(int c)	{cell.col=c;ProcessCursor(0);}
	// Printing
	void Print(int device);
};

enum gSpreadValType {gSpreadNum,gSpreadStr};
//----------------------------Spread Data Cell----------------------------
#define		S_HILIGHT	1
#define		S_BOLD		2
#define		S_ITALIC	4
class SpreadDataCell
{
friend gOutput &operator<<(gOutput &op,const SpreadDataCell &s);
private:
	Bool							entered;
	gSpreadValType		val_type;
	gString						value;
	long							attributes;
public:
	// Constructor
	SpreadDataCell(void) {entered=FALSE;val_type=gSpreadNum;attributes=0;}
  ~SpreadDataCell(void) { }
	SpreadDataCell(const SpreadDataCell &C)
	{
		entered=C.entered;val_type=C.val_type;
		value=C.value;attributes=C.attributes;
	}
	// Assignment operators
	SpreadDataCell &operator=(const SpreadDataCell &C)
	{
		entered=C.entered;val_type=C.val_type;
		value=C.value;attributes=C.attributes;
		return (*this);
	}
	SpreadDataCell &operator=(const gString &S)
	{if (val_type!=gSpreadStr) val_type=gSpreadNum;value=S;return (*this);}
	// Equality
	int operator==(const SpreadDataCell &C) {return value==C.value;}
	int operator!=(const SpreadDataCell &C) {return !(value==C.value);}
	// General info
	Bool	Entered(void) {return entered;}
	void	Entered(Bool _e)	{entered=_e;}
	void	SetAttributes(long a) {attributes=a;}
	Bool	HiLighted(void)	{return attributes&S_HILIGHT;}
	void	HiLighted(Bool _e)	{if (_e) attributes|=S_HILIGHT; else attributes&=(~S_HILIGHT);}
	Bool	Bold(void) {return attributes&S_BOLD;}
	void 	Bold(Bool _b) {if (_b) attributes|=S_BOLD; else attributes&=(~S_BOLD);}
	void	SetType(gSpreadValType _type) {val_type=_type;}
	gSpreadValType	GetType(void)	{return val_type;}
	gString					&GetValue(void)	{return value;}
	void						SetValue(const gString &S) {value=S;}
	// Erase all the data in the cell, including clearing all cell attributes
	void 	Clear(void) {entered=FALSE;value=gString();attributes=0;}
};
gOutput &operator<<(gOutput &op,const gBlock<SpreadDataCell> &s);

//-------------------------------------------------------------------------
//************************ SPREAD SHEET ***********************************
class SpreadSheet
{
friend gOutput &operator<<(gOutput &op,const SpreadSheet &s);
private:
	SpreadSheetC	*sheet;
	gRectBlock<SpreadDataCell> data;
	gBlock<gString>	row_labels;
	gBlock<gString> col_labels;
	int						rows,cols,level;
	gString				label;
	Bool					active;
public:
	// Constructors & destructors
	SpreadSheet(void) : sheet(0) {;}	// Void constructor--must have to init arrays of this
	SpreadSheet(int rows,int cols,int level,char *title=NULL,wxFrame *parent=NULL);
	~SpreadSheet(void)	{if (sheet) {sheet->Show(FALSE); delete sheet;}}
	// Post-Constructor, use if creating arrays of this
	void Init(int rows,int cols,int level,char *title=NULL,wxFrame *parent=NULL);
	// Sizing info for the canvas ...
	void SetSize(int xs,int ys,int xe,int ye);
	void GetSize(int *w,int *h) {sheet->DesiredSize(w,h);}
	void CheckSize(void) {sheet->CheckScrollbars();}
	void GetLabelExtent(char *str,float *x,float *y)	{sheet->GetLabelExtent(str,x,y);}
	void GetDataExtent(float *x,float *y,const char *str="W")	{sheet->GetDataExtent(x,y,str);}
	// Row/Col manipulation
	void AddRow(void);
	void AddCol(void);
	void DelRow(int row);
	void DelCol(int col);
	// Data access stuff for the canvas
	int		GetRows(void)			{return rows;}
	int		GetCols(void)			{return cols;}
	// Data access for top level
	void	SetActive(Bool _s)
	{
	sheet->Show(_s);
	if (_s) {sheet->SetFocus();sheet->OnPaint();}
	active=_s;
	}
	// General data access
	void		SetValue(int row,int col,const gString &s) {data(row,col)=s;data(row,col).Entered(TRUE);}
	gString &GetValue(int row,int col) {return data(row,col).GetValue();}
	gString &GetLabel(void)	{return label;}
	void		SetLabel(const gString &s) {label=s;}
	void		SetType(int row,int col,gSpreadValType t) {data(row,col).SetType(t);}
	int			GetLevel(void)	{return level;}
	// Cell attributes
	gSpreadValType GetType(int row,int col) {return data(row,col).GetType();}
	Bool		Bold(int row,int col) {return data(row,col).Bold();}
	Bool		HiLighted(int row,int col) {return data(row,col).HiLighted();}
	void		Bold(int row,int col,Bool _b) {data(row,col).Bold(_b);}
	void		HiLighted(int row,int col,Bool _h) {data(row,col).HiLighted(_h);}
	// Erase all the data in the spreadsheet, including clearing all cell attributes
	void Clear(void);
	// Checking if the cell has something in it
	Bool		EnteredCell(int row,int col) {return data(row,col).Entered();}
	// Row/Column labeling
	void		SetLabelRow(int row,const gString &s) 	{row_labels[row]=s;}
	void		SetLabelCol(int col,const gString &s)	{col_labels[col]=s;}
	void		SetLabelRow(const gBlock<gString> &vs) {row_labels=vs;}
	void		SetLabelCol(const gBlock<gString> &vs) {col_labels=vs;}
	gString	GetLabelRow(int row)	{return row_labels[row];}
	gString	GetLabelCol(int col)	{return col_labels[col];}
	// Accessing the currently hilighted cell
	int			CurRow(void)	{return sheet->Row();}
	int			CurCol(void)	{return sheet->Col();}
	void		SetCurRow(int r)	{sheet->SetRow(r);}
	void		SetCurCol(int c)	{sheet->SetCol(c);}
	// Equality operators to allow this class to be used in a gList
	int			operator==(const SpreadSheet &s) {return 0;}
	int			operator!=(const SpreadSheet &s) {return 1;}
	// Printing
	void Print(int device) {sheet->Print(device);}
	// Forced updating
	void Repaint(void) {sheet->OnPaint();}
	// Debugging
	void		Dump(gOutput &out) {data.Dump(out);}
};
//**************************** SPREAD SHEET 3D ******************************
// Feature control constants: the low byte is used by the panel/buttons,
// and the high byte is used by the menubar/menus
#include "sprconst.h"

class SpreadSheet3D: public wxFrame
{
friend gOutput &operator<<(gOutput &op,const SpreadSheet3D &s);
private:
	gList<SpreadSheet>	data;
	SpreadSheetDrawSettings 	*draw_settings;
	SpreadSheetDataSettings		*data_settings;
	int										cur_level;
	wxPanel								*panel;
	wxMenuBar							*menubar;
	wxSlider							*level_item;
	int										panel_x,panel_y,panel_new_line;
	int										completed;
	int										levels;
	Bool									editable;
	unsigned int					features;
	gString								label;
	void	SavePanelPos(void)	{panel->GetCursor(&panel_x,&panel_y);}
	void	MakeFeatures();
protected:
	wxPanel *Panel(void) {return panel;}
	wxMenuBar *MenuBar(void) {return menubar;}
	wxMenuBar *MakeMenuBar(void);
	void SetMenuBar(wxMenuBar *bar);
public:
	// Constructor
	SpreadSheet3D(int rows,int cols,int levels,char *title,
					wxFrame *parent=NULL,unsigned int _features=OK_BUTTON | CANCEL_BUTTON,
					SpreadSheetDrawSettings *drs=NULL,SpreadSheetDataSettings *_dts=NULL);
	// Destructor
	~SpreadSheet3D(void)
	{
		if (--draw_settings->ref_cnt==0) delete draw_settings;
		if (--data_settings->ref_cnt==0) delete data_settings;
	}
	// Windows Events Handlers
	void Update(wxDC *dc=NULL);
	virtual void OnSize(int w,int h);
	virtual void OnMenuCommand(int id);
	static void spread_slider_func(wxSlider &ob,wxCommandEvent &ev);
	static void spread_ok_func(wxButton	&ob,wxEvent &ev);
	static void spread_cancel_func(wxButton	 &ob,wxEvent &ev);
	static void	spread_print_func(wxButton	 &ob,wxEvent &ev);
	static void spread_change_func(wxButton  &ob,wxEvent &ev);
	static void spread_options_func(wxButton &ob,wxEvent &ev);
	virtual void OnOk(void);
	virtual void OnCancel(void);
	virtual void OnDoubleClick(int row,int col,int level,const gString &value) { }
	virtual void OnSelectedMoved(int row,int col) { }
	virtual void OnPrint(void);
	virtual void OnHelp(int help_type=0) { }
	virtual Bool OnCharNew(wxKeyEvent &ev) {return FALSE;}
	// General data access
	void		SetType(int row,int col,gSpreadValType t) {data[cur_level].SetType(row,col,t);}
	gSpreadValType GetType(int row,int col) {return data[cur_level].GetType(row,col);}
	void		SetType(int row,int col,int level,gSpreadValType t) {data[level].SetType(row,col,t);}
	gSpreadValType GetType(int row,int col,int level) {return data[level].GetType(row,col);}
	SpreadSheet &operator[](int i){assert(i>0&&i<=levels);return data[i];}
	void		SetCell(int row,int col,const gString &s)
		{data[cur_level].SetValue(row,col,s);}
	gString &GetCell(int row,int col)
		{return data[cur_level].GetValue(row,col);}
	void		SetCell(int row,int col,int level,const gString &s)
		{assert(level>0 && level<=levels);data[level].SetValue(row,col,s);}
	gString &GetCell(int row,int col,int level)
		{assert(level>0 && level<=levels);return data[level].GetValue(row,col);}
	// Erase all the data in the spreadsheet, including clearing all cell attributes
	void Clear(int level=0) {if (level==0) level=cur_level;data[level].Clear();}
	// Accesing different levels
	void		SetLevel(int _l);
	int			GetLevel(void)		{return cur_level;}
	// Accessing the currently hilighted cell
	int			CurRow(int level=0) {if (level==0) level=cur_level;return data[level].CurRow();}
	int			CurCol(int level=0) {if (level==0) level=cur_level;return data[level].CurCol();}
	void		SetCurRow(int r,int level=0)	{if (level==0) level=cur_level;data[level].SetCurRow(r);}
	void		SetCurCol(int c,int level=0)	{if (level==0) level=cur_level;data[level].SetCurCol(c);}
	// Checking if the cell has something in it
	Bool		EnteredCell(int row,int col,int level=0)
		{if (level==0) level=cur_level;return data[level].EnteredCell(row,col);}
	// Accessing dimentions
	int			GetRows(void)		{return data[cur_level].GetRows();}
	int			GetCols(void)		{return data[cur_level].GetCols();}
	// Row/Col/Level manipulation
	void AddRow(void){for(int i=1;i<=levels;i++) data[i].AddRow();Redraw();}
	void AddCol(void){for(int i=1;i<=levels;i++) data[i].AddCol();DrawSettings()->AddCol(); Redraw();}
	void AddLevel(void);
	void DelRow(int row=0){for(int i=1;i<=levels;i++) data[i].DelRow(row);Redraw();}
	void DelCol(int col=0){for(int i=1;i<=levels;i++) data[i].DelCol(col);Redraw();}
	void DelLevel(void);
	// Row/Column labeling
	void		SetLabelRow(int row,const gString &s,int level=0);
	void		SetLabelCol(int col,const gString &s,int level=0);
	void		SetLabelLevel(const gString &s,int level=0)
		{if (level==0) level=cur_level;data[level].SetLabel(s);}
	void		SetLabelRow(const gBlock<gString> &vs,int level=0)
		{if (level==0) level=cur_level;data[level].SetLabelRow(vs);}
	void		SetLabelCol(const gBlock<gString> &vs,int level=0)
		{if (level==0) level=cur_level;data[level].SetLabelCol(vs);}
	gString	GetLabelRow(int row,int level=0)
		{if (level==0) level=cur_level;return data[level].GetLabelCol(row);}
	gString	GetLabelCol(int col,int level=0)
		{if (level==0) level=cur_level;return data[level].GetLabelRow(col);}
	gString GetLabelLevel(int level=0)
		{if (level==0) level=cur_level;return data[level].GetLabel();}
	// User Interface
	int			Completed(void)	{return completed;}
	void		SetCompleted(int c) 	{completed=c;}
	Bool		Editable(void)	{return editable;}
	void		SetEditable(Bool _e) {editable=_e;}
	Bool		HiLighted(int row,int col,int level=0)
		{if (level==0) level=cur_level;return data[level].HiLighted(row,col);}
	void		HiLighted(int row,int col,int level=0,Bool _e=FALSE)
		{if (level==0) level=cur_level;data[level].HiLighted(row,col,_e);}
	Bool		Bold(int row,int col,int level=0)
		{if (level==0) level=cur_level;return data[level].Bold(row,col);}
	void		Bold(int row,int col,int level=0,Bool _e=FALSE)
		{if (level==0) level=cur_level;data[level].Bold(row,col,_e);}
	// Drawing/Data parameters
	SpreadSheetDrawSettings *DrawSettings(void)	{return draw_settings;}
	SpreadSheetDataSettings *DataSettings(void)	{return data_settings;}
	// Some low level info about the canvases
	void GetDataExtent(float *x,float *y,const char *str="W")	{data[cur_level].GetDataExtent(x,y,str);}
	// Labeling
	void	FitLabels(void);
	void	Resize(void);
	void	Redraw(void);
	void 	Repaint(void) {data[cur_level].Repaint();}
	// Printing
	void Print(int device) {data[cur_level].Print(device);}
	// Adding buttons
	wxButton *AddButton(const char *label,wxFunction fun);
	wxPanel *AddPanel(void);
	void		AddButtonNewLine(void) {panel_new_line=TRUE;}
// Could not find a way to do this
//	void 		AddMenu(wxMenu *submenu,const char *label);
	// Debugging
	void		Dump(void);
};

#endif
