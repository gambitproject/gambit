//
// FILE: spreadconfig.h -- SpreadSheet classes configuration
//
// $Id$
//

#ifndef SPREADCONFIG_H
#define SPREADCONFIG_H

#include "gblock.h"

#define ANY_BUTTON			1			// Specify this to create an empty panel
#define ALL_BUTTONS			255
#define	PRINT_BUTTON		2
#define	OK_BUTTON				8
#define	CANCEL_BUTTON		16
#define OPTIONS_BUTTON	32
#define HELP_BUTTON			64

#define	ANY_MENU				256		// Specify this to create an empty menubar
#define	ALL_MENUS       65280
#define OUTPUT_MENU			512
#define CLOSE_MENU			1024
#define OPTIONS_MENU		2048
#define CHANGE_MENU			4096
#define HELP_MENU				8192

#define HELP_MENU_ABOUT			8193
#define HELP_MENU_CONTENTS	8194


#define XSTEPS                  20      // Scroll steps horizontally
#define YSTEPS                  20      // Scroll steps vertically

#define TEXT_OFF                8       // Offset of the text from the grid line
#define LINE_OFF                3       // Offset of the border from the grid line
#define MAX_SHEET_HEIGHT        400
#define MAX_SHEET_WIDTH         600
#define MIN_SHEET_HEIGHT        20
#define MIN_SHEET_WIDTH         160
#define DEFAULT_ROW_HEIGHT      30
#define DEFAULT_COL_WIDTH       6
#define MIN_BUTTON_SPACE        60
#define ROW_LABEL_WIDTH         30
#define COL_LABEL_HEIGHT        30
#define COL_WIDTH_UNIT          10

typedef enum
{
    SpreadMoveJump, SpreadMoveLeft, SpreadMoveRight, SpreadMoveUp, SpreadMoveDown
} SpreadMoveDir;

//********************** SPREADSHEET DATA SETTINGS ***********************
// Both the DataSettings and DrawSettings employ a pseudo-reference
// counting mechanism.  Since it is desirable to be able to use the
// same settings for more than one spreadsheet, or to be able to
// recreate a spredsheet, a copy of these classes can be obtained by
// calling GetThis().  This will increment the internal reference
// count.  The spreadsheet should not delete the classes if ref_cnt
// is non-zero.

class SpreadSheetDataSettings
{
#define S_CAN_GROW_ROW          1   // Or these together to allow size changes
#define S_CAN_GROW_COL          2
#define S_CAN_GROW_LEVEL        4
#define S_CAN_SHRINK_ROW        8
#define S_CAN_SHRINK_COL        16
#define S_CAN_SHRINK_LEVEL      32
#define S_AUTO_LABEL_ROW        1
#define S_AUTO_LABEL_COL        2
#define S_AUTO_LABEL_LEVEL      4

private:
    int     change;
    int     auto_label;
    gText   auto_label_row;
    gText   auto_label_col;
    gText   auto_label_level;

public:
    int ref_cnt;    // reference counter
    SpreadSheetDataSettings *GetThis(void) { ref_cnt++; return this; }

    // Constructor
    SpreadSheetDataSettings(void)   { change = 0; auto_label = 0; }

    // Grow/Shrink the spreadsheet--add/remove rows, columns, and levels
    Bool    Change(int what)        { return change&what; }

    // Set the grow
    void    SetChange(int c)        { change = c; }

    // Automatic labeling for rows/columns.  Use SetAutoLabel to choose what
    // to label.  Then use SetAutoLabelStr to set the fmt parameter to the
    // sprintf(temp,fmt,row/col) function.
    int     AutoLabel(int l)        { return auto_label&l; }
    void    SetAutoLabel(int l)     { auto_label = l; }
    void    SetAutoLabelStr(const gText s, int what);
    gText   AutoLabelStr(int what) const;
};


//********************** SPREADSHEET DRAW SETTINGS ***********************
class SpreadSheet3D;
class SpreadSheetDrawSettings
{
#define S_LABEL_ROW     1
#define S_LABEL_COL     2
#define S_PREC_CHANGED  1
private:
    SpreadSheet3D *parent;
    int         row_height, default_col_width;
    int         tw, th;
    int         num_prec;
    gBlock<int> col_width;
    int         total_height, total_width, real_height, real_width;
    int         x_scroll, y_scroll;
    int         x_start, y_start;
    int         panel_size;
    Bool        scrolling;
    Bool        vert_fit, horiz_fit, show_labels, col_dim_char;
    Bool        gtext;
    int         labels;
    wxFont     *data_font;
    wxFont     *label_font;

    // Functions to read/write the configuration file
    // Note: this always writes to file 'gambit.ini', section [SpreadSheet3D]
    void        SaveOptions(const char *s = NULL);
    int         LoadOptions(const char *s = NULL);

public:
    int ref_cnt;    // reference counter
    SpreadSheetDrawSettings *GetThis(void) { ref_cnt++; return this; }

    // Constructor
    SpreadSheetDrawSettings(SpreadSheet3D *_parent, int cols);

    // A way to set many options through a dialog
    void SetOptions(void);

    // Allows to use a previously created settings with this parent
    void SetParent(SpreadSheet3D *_parent) { parent = _parent; }

    // Inform the class that dimensionality has changed.
    void SetDimensions(int rows, int cols); // only the col info is used now.

    void AddCol(int col = 0) 
    { 
        col_width.Insert(DEFAULT_COL_WIDTH, ((col) ? col :col_width.Length()+1));
    }

    void DelCol(int col) { col_width.Remove(col); }
    int NumColumns(void) const { return col_width.Length(); }

    // Data Access, Get* functions
    // These functions control the dimentions of each cell i.e. Width X Height
    int GetRowHeight(void)  { return ((vert_fit) ? (th*5/4+2*TEXT_OFF) : row_height); }
    int GetRowHeightRaw(void) const { return row_height; }
    int GetColWidth(int col = 0);
    int GetColWidthRaw(int col) const;

    bool GetRowFit(void) { return vert_fit; }
    bool GetColFit(void) { return horiz_fit; }

    // These functions tell about the size of the currently selected font
    int GetTextHeight(void) { return th; }
    int GetTextWidth(void)  { return tw; }

    // GetWidth/Height return the size of the virtual canvas 
    // if scrollbars are on (GetVirtualSize)
    int GetWidth(void)      { return total_width;  }
    int GetHeight(void)     { return total_height; }

    // GetRealWidth/Height returns the physical size of the window (GetClientSize)
    int GetRealWidth(void)  { return real_width;  }
    int GetRealHeight(void) { return real_height; }

    // Controls what font to use for the cell contents 
    // [###: fix rowheight/colwidth to use the font]
    wxFont  *GetDataFont(void)  { return data_font;  }
    wxFont  *GetLabelFont(void) { return label_font; }

    // Start position of actual grid
    int XStart(void)        
    { return (x_start == -1) ? RowLabels()*ROW_LABEL_WIDTH : x_start; }

    int YStart(void)
    { return (y_start == -1) ? ColLabels()*COL_LABEL_HEIGHT : y_start; }

    // These are nonzero if scrollbars are on.  Used to calculate scroll position
    int     XScroll(void)         { return x_scroll;  }
    int     YScroll(void)         { return y_scroll;  }
    Bool    Scrolling(void)       { return scrolling; }

    // These are used for row/column labeling
    int     RowLabels(void)       { return labels&S_LABEL_ROW; }
    int     ColLabels(void)       { return labels&S_LABEL_COL; }

    // Panel Sizing--allows the user to add items to the panel
    int     PanelSize(void)       { return panel_size; }
    void    SetPanelSize(int _s)  { panel_size = _s; }

    // Updates font information after change/on init
    void    UpdateFontSize(float w = -1, float h = -1);

    // Are we to use the color-capable gtext or the plain text
    Bool    UseGText(void) const  { return gtext; }

    // Number of decimal places to output for floating point numbers
    int     NumPrec(void) const   { return num_prec; }

    // Set* functions
    void    SetRowHeight(int _r)        { row_height = _r;   }
    void    SetColWidth(int _c, int col = 0);
    void    SetWidth(int _w)            { total_width = _w;  }
    void    SetHeight(int _h)           { total_height = _h; }
    void    SetRealWidth(int _w)        { real_width = _w;   }
    void    SetRealHeight(int _h)       { real_height = _h;  }
    void    SetDataFont(wxFont *_f)     { data_font = _f;  UpdateFontSize(); }
    void    SetLabelFont(wxFont *_f)    { label_font = _f; UpdateFontSize(); }
    void    SetXScroll(int _s)          { x_scroll = _s;     }
    void    SetYScroll(int _s)          { y_scroll = _s;     }
    void    SetScrolling(Bool _s)       { scrolling = _s;    }
    void    SetLabels(int _l)           { labels = _l;       }
    void    SetXStart(int _x)           { x_start = _x;      }
    void    SetYStart(int _y)           { y_start = _y;      }
    void    SetGText(Bool g)            { gtext = g;         }
};

enum gSpreadValType { gSpreadNum, gSpreadStr };

#endif  // SPREADCONFIG_H
