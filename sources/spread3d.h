//
// FILE: spread3d.h, header file for spread.cc
//
// $Id$
//

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
// call wxMenuBar *tmp_bar = MakeMenuBar();
// add your menus, call SetMenuBar(tmp_bar);

// Toolbar:
// Basic toolbar functionality is provided for.  A wxToolBar *toolbar member
// can be assigned to to add a toolbar to the canvas.  The OnSize function will
// take this into consideration and allocate some space for it.

// The physical dimentions of each data cell can be controlled by the
// user.  Each column can have a different width and all rows have the
// same height.  The size can be measured in either screen pixels or be
// tied to the size of the font(*).  In that case, each unit corresponds
// to the width of a letter, and is scaled automatically when font is
// changed.  The row height can also be made to scale with the font by
// checking the vert_fit(*) option.

// The spreadsheet now supports the gDrawText extended text format, which
// allows for multiple colors to be used in the same cell.  See wxmisc

// In order to make this class as widely useable as possible, most functions
// are declared virtual for easy overiding.  Note that you can overide
// OnMenuCommand, OnCharNew (this receives events from the active canvas),
// OnSize, OnOk, OnCancel, OnDoubleClick (Ctrl-Click in X), and some others.

// Note: on some platforms (msw,motif?), after any item on the panel has been
// accessed, the keyboard focus switches to the panel.  This can cause
// undesirable behavior (i.e. can not enter data, arrow keys do not work). It
// is advisable to call CanvasFocus member after processing any panel item
// events.

#ifndef SPREAD3D_H
#define SPREAD3D_H

#ifndef wxRUNNING
#define wxRUNNING   12345
#endif

#include "glist.h"
#include "gblock.h"
#include "grblock.h"
#include "gtext.h"

#include "spreadconfig.h"


//**************************** SPREADSHEET3D ******************************
// Feature control constants: the low byte is used by the panel/buttons,
// and the high byte is used by the menubar/menus

class wxToolBar;
class SpreadSheet;

class SpreadSheet3D: public wxFrame {
    friend gOutput &operator<<(gOutput &op, const SpreadSheet3D &s);

private:

    gList<SpreadSheet *>  data;
    SpreadSheetDrawSettings     *draw_settings;
    SpreadSheetDataSettings     *data_settings;
    int                          cur_level;
    wxPanel                     *panel;
    wxMenuBar                   *menubar;
    wxSlider                    *level_item;
    int                          panel_x, panel_y, panel_new_line;
    int                          completed;
    int                          levels;
    Bool                         editable;
    unsigned int                 features;
    gText                        label;

    void    SavePanelPos(void)  { panel->GetCursor(&panel_x, &panel_y); }
    void    MakeFeatures();

    static void spread_slider_func(wxSlider &ob, wxCommandEvent &ev);
    static void spread_ok_func(wxButton &ob, wxEvent &ev);
    static void spread_cancel_func(wxButton  &ob, wxEvent &ev);
    static void spread_print_func(wxButton   &ob, wxEvent &ev);
    static void spread_change_func(wxButton  &ob, wxEvent &ev);
    static void spread_options_func(wxButton &ob, wxEvent &ev);
    static void spread_help_func(wxButton &ob, wxEvent &ev);

protected:
    wxToolBar   *toolbar;
    wxPanel     *Panel(void)   { return panel;   }
    wxMenuBar   *MenuBar(void) { return menubar; }
    virtual wxMenuBar *MakeMenuBar(long menus);
    void SetMenuBar(wxMenuBar *bar);
    virtual void MakeButtons(long buttons);
    void OnOk1(void);     // These are necessary to allow for overloading 
    void OnCancel1(void); // of the next level functions (OnOk, etc) since the 
    void OnPrint1(void);  // static funcs must call the SpreadSheet3D versions.
    void OnHelp1(void);

public:
    // Constructor
    SpreadSheet3D(int rows, int cols, int levels, int status, char *title,
                  wxFrame *parent = NULL, 
                  unsigned int _features = OK_BUTTON | CANCEL_BUTTON,
                  SpreadSheetDrawSettings *drs = NULL, 
                  SpreadSheetDataSettings *_dts = NULL);

    // Destructor
    ~SpreadSheet3D(void);

    // Windows Events Handlers
    void Update(wxDC *dc = NULL);
    virtual void OnSize(int w, int h);
    virtual void OnMenuCommand(int id);
    virtual void OnOk(void);
    virtual void OnCancel(void);
    virtual void OnDoubleClick(int , int , int , const gText &) { }
    virtual void OnSelectedMoved(int row, int col, 
                                 SpreadMoveDir how = SpreadMoveJump);
    virtual void OnOptionsChanged(unsigned int /*opts*/ = 0) { }
    virtual void OnPrint(void);
    virtual void OnHelp(int =0);
    virtual Bool OnCharNew(wxKeyEvent &)          { return FALSE; }
    virtual Bool OnEventNew(wxMouseEvent &/*ev*/) { return FALSE; }
    void CanvasFocus(void);

    // General data access
    void SetType(int row, int col, gSpreadValType t);
    gSpreadValType GetType(int row, int col) ;
    void SetType(int row, int col, int level, gSpreadValType t); 
    gSpreadValType GetType(int row, int col, int level); 

    SpreadSheet &operator[](int i);

    void SetCell(int row, int col, const gText &s);
    const gText &GetCell(int row, int col) const;
    void SetCell(int row, int col, int level, const gText &s);
    const gText &GetCell(int row, int col, int level) const;

    // Erase all the data in the spreadsheet, 
    // including clearing all cell attributes
    void Clear(int level = 0);

    // Accesing different levels
    void SetLevel(int _l);
    int  GetLevel(void)      { return cur_level; }

    // Accessing the currently hilighted cell
    int  CurRow(int level = 0);
    int  CurCol(int level = 0);
    void SetCurRow(int r, int level = 0);
    void SetCurCol(int c, int level = 0);

    // Checking if the cell has something in it
    Bool EnteredCell(int row, int col, int level = 0);

    // Accessing dimensions
    int  GetRows(void);
    int  GetCols(void);

    // Row/Col/Level manipulation

    // Does nothing if current dimensions are given
    void SetDimensions(int rows, int cols, int levels = 0);  

    void AddRow(int p_row = 0);
    void AddCol(int p_col = 0);
    void AddLevel(int level = 0);
    void DelRow(int row = 0);
    void DelCol(int col = 0);
    void DelLevel(void);

    // Row/Column labeling
    void  SetLabelRow(int row, const gText &s, int level = 0);
    void  SetLabelCol(int col, const gText &s, int level = 0);

    void SetSelectableRow(int, Bool);
    void SetSelectableCol(int, Bool);

    void  SetLabelLevel(const gText &s, int level = 0);
    void  SetLabelRow(const gBlock<gText> &vs, int level = 0);
    void  SetLabelCol(const gBlock<gText> &vs, int level = 0);
    gText GetLabelRow(int row, int level = 0);
    gText GetLabelCol(int col, int level = 0);
    gText GetLabelLevel(int level = 0);

    // User Interface
    int   Completed(void) { return completed; }
    void  SetCompleted(int c)     { completed = c; }
    Bool  Editable(void)  { return editable; }
    void  SetEditable(Bool _e) { editable = _e; }

    Bool  HiLighted(int row, int col, int level = 0);
    void  HiLighted(int row, int col, int level = 0, Bool _e = FALSE);
    Bool  Bold(int row, int col, int level = 0);
    void  Bold(int row, int col, int level = 0, Bool _e = FALSE);

    // Drawing/Data parameters
    SpreadSheetDrawSettings *DrawSettings(void) { return draw_settings; }
    SpreadSheetDataSettings *DataSettings(void) { return data_settings; }

    // Some low level info about the canvases
    void GetDataExtent(float *x, float *y, const char *str = "W");
    Bool XYtoRowCol(int x, int y, int *row, int *col);

    // Labeling
    void  FitLabels(void);
    void  Resize(void);
    void  Redraw(void);
    void  Repaint(void);

    // Printing
    void Print(wxOutputMedia device, wxOutputOption fit);

    // Adding buttons
    wxButton *AddButton(const char *label, wxFunction fun);
    wxPanel  *AddPanel(void);
    void      AddButtonNewLine(void) { panel_new_line = TRUE; }

    // Debugging
    void Output(void);
};

#endif // SPREAD3D_H


