//
// File: pxi.h -- declarations of main classes for PXI.  [Plot X V2.0]
//

// This program is designed to plot data from simulations/experiments on
// normal form games [support for other types of generic infoset/player games
// is now being implemented].  The basic structure of the file contains the
// number of players/infosets in the data file, number of strategies per
// player/infoset and the data.  The program was originally designed to handle
// 2 player normal form games with a known payoff matrix.  Some features in
// the program are only available for this type of data file.  The program
// also includes a calculating engine to solve the type of problem described
// above.  Note: a special pseudo-3D mode is implemented on a triangular grid
// for two player games in which each player has 3 strategies to choose from.
// The variable parameter (the error) is known as lambda throughout the
// program.
// See pxi.cc for more details (file format, etc).

#ifndef PXI_H
#define PXI_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "wx/dc.h"
#include "wx/dcps.h"
#include "wx/config.h"    // for wxConfig
#include "wx/docview.h"   // for wxFileHistory
#include "expdata.h"
#include "wxmisc.h"

#define PXI_QUIT              100
#define PXI_OUTPUT            101
#define PXI_HELP_ABOUT        108
#define PXI_HELP_CONTENTS     109
#define PXI_LOAD_FILE         111
#define	PXI_NEW_WINDOW	      117
#define	PXI_CHILD_CLOSE	      118
#define	PXI_CHILD_QUIT	      134
#define	PXI_JUST_REPAINT      115
#define	PXI_NEW_FILE_REPAINT  116
#define	PXI_UPDATE_SCREEN     119
#define	PXI_UPDATE_FILE       120
#define	PXI_UPDATE_PRINTER    121
#define	PXI_UPDATE_METAFILE   122
#define	PXI_FILE_DETAIL	      123
#define	PXI_PREFS_COLORS      124
#define	PXI_PREFS_FONTS    125
#define	PXI_PREFS_FONT_AXIS    131
#define	PXI_PREFS_FONT_OVERLAY  132
#define	PXI_PREFS_FONT_LABEL    133
#define	PXI_PAGE_NEXT           135
#define	PXI_PAGE_PREV           136
#define	PXI_PREFS_SCALE         137
#define	PXI_PREFS_SCALE_1         138
#define	PXI_PREFS_SCALE_2         139
#define	PXI_PREFS_SCALE_3         140
#define	PXI_PREFS_SCALE_4         141
#define	PXI_PREFS_SCALE_5         142
#define	PXI_PREFS_SCALE_6         143
#define	PXI_PREFS_SCALE_7         144
#define	PXI_PREFS_SCALE_8         145
#define	PXI_PREFS_SCALE_9         146
#define	PXI_PREFS_SCALE_10         147
#define	PXI_PREFS_ZOOM_IN         150
#define	PXI_PREFS_ZOOM_OUT         151

#define	PXI_NO_SET_STOP      -1.0
#define	PXI_SET_STOP          127
#define	PXI_KEY_STOP          WXK_SPACE
#define	PXI_DISPLAY_OPTIONS   128

#define PXI_DATA_OVERLAY_DATA 215
#define PXI_DATA_OVERLAY_FILE 220

#define PXI_PLOT_2_STEP        .1
#define MAXN    10

#define	OVERLAY_TOKEN   1
#define	OVERLAY_NUMBER  2

#define MIN_TOKEN_SIZE  2
#define MAX_TOKEN_SIZE  10
#define DEF_TOKEN_SIZE  4

#define TOP_PLOT        0
#define	BOTTOM_PLOT     1
#define TEXT_MARGIN	.05
#define DELTA           1e-9


#define	COLOR_EQU       1
#define COLOR_PROB      2
#define COLOR_NONE      3

#define PXI_GUI_HELP  "Extensive Form GUI"

/*********************** PXI.H ******************************************/

extern void pxiExceptionDialog(const wxString &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);

class PxiApp: public wxApp {
private:
  wxString m_currentDir; /* Current position in directory tree. */
  
  bool  OnInit(void);
public:
  virtual ~PxiApp() { }

  const wxString &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const wxString &p_dir)  { m_currentDir = p_dir; }
};

DECLARE_APP(PxiApp)

typedef enum {
   PXI_PLOT_X = 0, PXI_PLOT_2 = 1 , PXI_PLOT_3 = 2
} PxiPlotMode;

/*
typedef enum {
   COLOR_EQU = 0, COLOR_PROB = 1 , COLOR_NONE = 2
} PxiColorMode;
*/

class PxiCanvas;

class PxiPrintout: public wxPrintout
{
private:
  PxiCanvas &canvas;
  
public:
  PxiPrintout(PxiCanvas &canvas, char *title = "PXI printout");
  ~PxiPrintout();
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};

class PlotInfo
{
private:
  PxiPlotMode plotMode;
  bool showAxis, showTicks, showNums, showLabels, showSquare;
  double x_min,x_max;                  // plot limits on X (lambda) variable
  double y_min,y_max;                  // plot limits on Y variable
  int number;                         // plot number

  gBlock<int> isets;                  // infosets to plot for each plot

  // Bool instead of bool needeed here for template ambiguity in gBlock for BC
  typedef gBlock<Bool> show_actions;
  friend gOutput &operator<<(gOutput &op,const show_actions &p);
  friend gOutput &operator<<(gOutput &op,const PlotInfo &p);
  gBlock< show_actions > strategies;  // strategies to plot for each infoset

public:
  PlotInfo(void);
  ~PlotInfo(void);
  PlotInfo &operator=(const PlotInfo &p);       
  bool operator==(const PlotInfo &p);       
  bool operator!=(const PlotInfo &p) {return !(*this == p);}  

  void Init(const FileHeader &, int num);

  const gBlock<int> &GetIsets(void) const {return isets;}
  bool Contains(int iset) const {return isets.Contains(iset);}
  void AddInfoset(int iset) {if(iset>=1 && iset <=GetNumIsets()) isets.Append(iset);}
  void RemoveInfoset(int iset) {isets.Remove(isets.Find(iset));}

  bool GetStrategyShow(int j, int k) const {return strategies[j][k];}
  int GetNumStrats(int j) const {return strategies[j].Length();}
  int GetNumIsets(void) const {return strategies.Length();}
  int GetPlotNumber(void) const {return number;}
  void SetPlotNumber(int i) {number=i;}

  bool ShowAxis(void) const {return showAxis;}
  bool ShowTicks(void) const {return showTicks;}
  bool ShowNums(void) const {return showNums;}
  bool ShowLabels(void) const {return showLabels;}
  bool ShowSquare(void) const {return showSquare;}

  double GetMinX(void) const {return x_min;}
  double GetMaxX(void) const {return x_max;}
  double GetMinY(void) const {return y_min;}
  double GetMaxY(void) const {return y_max;}
  PxiPlotMode GetPlotMode(void) const {return plotMode;}

  void SetStrategyShow(int j, int k, bool flag) {strategies[j][k]=flag;}
  void SetShowAxis(bool flag) {showAxis = flag;}
  void SetShowTicks(bool flag) {showTicks = flag;}
  void SetShowNums(bool flag) {showNums = flag;}
  void SetShowLabels(bool flag) {showLabels = flag;}
  void SetShowSquare(bool flag) {showSquare = flag;}
  void SetMinX(double x) {x_min = x;}
  void SetMaxX(double x) {x_max = x;}
  void SetMinY(double x) {y_min = x;}
  void SetMaxY(double x) {y_max = x;}
  void SetPlotMode(PxiPlotMode m) {plotMode = m;}

  // RangeX, determines if x falls in the drawable range
  bool RangeX(double x) const {return (x>x_min && x<x_max);}
  // RangeY, determines if y falls in the drawable range
  bool RangeY(double y) const {return (y>y_min && y<y_max);}
};

class PxiDrawSettings
{
private:
  wxFont	overlay_font, label_font, axis_font; 
  wxBrush       clear_brush, exp_data_brush;
  wxColour      axis_text_color;
  
  int           plots_per_page; // Plots per page
  int           maxpage;       // maximum page number
  int           whichpage;    // current page number
  int           whichplot;    // current plot
  
  int		overlay_symbol;      // one of : OVERLAY_TOKEN | OVERLAY_NUMBER
  Bool		overlay_lines;       // connect overlay points?
  int		overlay_token_size;  
  
  int           data_mode;    // Can be either 1-Log or 0-Linear
  int		color_mode;   // COLOR_EQU, COLOR_PROB, or COLOR_NONE
  Bool		connect_dots;   // connect dots on plot.  
  Bool          restart_overlay_colors; // new set of colors for each overlay 
  
  int           num_infosets; // total number of infosets
  double	l_start,l_stop,l_step;           // data limits on X (lambda)
  
  gBlock< PlotInfo> thisplot;
  
  Bool CheckPlot3Mode(void);
  Bool CheckPlot2Mode(void);
public:
  PxiDrawSettings(FileHeader &header);
  
  // Get* functions
  
  int GetPlotsPerPage(void) const {return plots_per_page;}
  // NumInfosets returns total number of infosets
  int GetNumInfosets(void) const {return num_infosets;}
  // DataMode, returns one of DATA_TYPE_ARITH | DATA_TYPE_LOG indicating data type
  int GetDataMode(void) const {return data_mode;}
  // ColorMode, how to color the data: by equilibrium #, prob #, or just a constant
  int GetColorMode(void) const {return color_mode;}
  void SetColorMode(int mode ) {color_mode=mode;}

  // number of strats in infoset j
  //  int GetNumStrats(int j) {return thisplot[1].GetNumStrats(j);}

  double GetMaxL(void) const {return l_stop;}
  // MinL, returns the minimum labmda value in the data file
  double GetMinL(void) const {return l_start;}
  // DelL, return the lambda step
  double GetDelL(void) const {return l_step;}

  PlotInfo &GetPlotInfo(int i=1) {return thisplot[i];}
  int GetNumPlots(void) const {return thisplot.Length();}
  
  // OverlayFont, if the experimental data overlay is done using the number
  //   of the point in the file, this font is used to display that number
  const wxFont &GetOverlayFont(void) const {return overlay_font;}
  // LabelFont, returns the font to be used for labels [created w/ Shift-Click]
  const wxFont &GetLabelFont(void) const {return label_font;}

  // OverlaySym, returns one of : OVERLAY_CIRCLE | OVERLAY_NUMBER, indicating
  //   if the experimental data overlay points will be plotted using little circles
  //   or their number in the data file
  const int GetOverlaySym(void) const {return overlay_symbol;}
  // OverlaySize, determines the size of the tokens use for overlay
  const int GetTokenSize(void) const {return overlay_token_size;}
  // OverlayLines, determines if the experimental data overlay dots will be connected
  const bool GetOverlayLines(void) const {return overlay_lines;}

  // ConnectDots, determines if the regular data file dots will be connected
  bool ConnectDots(void) const {return connect_dots;}
  void SetConnectDots(bool flag) {connect_dots = flag;}
  // RestartOverlayColors, determines if the colors used for the equilibria
  // lines will restart at 0 for each of the file overlays or will increment
  // from the previous file
  bool RestartOverlayColors(void) const {return restart_overlay_colors;}
  void SetRestartOverlayColors(bool flag) {restart_overlay_colors = flag;}

  // Set* functions

  void SetPage(int i) {if(i>0 && i<=maxpage) whichpage = i;}
  void SetMaxPages(int i) {if(i>0) maxpage = i;SetPage(1);}
  void SetPlotsPerPage(int n) 
    {
      if(n>=1 || n<=2) 
	plots_per_page = n; 
      int npl=GetNumPlots(); 
      SetMaxPages(npl-((n*npl-npl)/n));  // to get the least integer greater than npl/n
    }
  void SetNextPage(void) {SetPage(whichpage+1);}
  void SetPreviousPage(void) {SetPage(whichpage-1);}
  int GetPage(void) const {return whichpage;}
  int GetMaxPage(void) const {return maxpage;}

  void SetWhichPlot(int i) {if( i>=1 && i<=GetNumPlots()) whichplot=i;}
  int GetWhichPlot(void) const {return whichplot;}

  void SetOptions(wxWindow *parent);

  /*
  void SetStopMax(double sm) {stop_max=sm;}
  void SetStopMin(double sm) {stop_min=sm;}

  void ResetSetStop(void) {stop_min=l_start;stop_max=l_stop;}
  */  

  void SetOverlaySym(int s) {overlay_symbol=s;}
  void SetOverlayLines(Bool l)	{overlay_lines=l;}
  void SetTokenSize(int s) {overlay_token_size=s;}
  void SetAxisFont(const wxFont &f) {axis_font=f;}
  void SetLabelFont(const wxFont &f) {label_font=f;}
  void SetOverlayFont(const wxFont &f) {overlay_font=f;}
  const wxFont &GetAxisFont(void) const {return axis_font;}
  const wxBrush &GetDataBrush(void) const {return exp_data_brush;}
  const wxBrush &GetClearBrush(void) const {return clear_brush;}
  const wxColour &GetAxisTextColor(void) const {return axis_text_color;}
};

// Define a new canvas

class PxiCanvas: public wxScrolledWindow
{
friend class PxiChild;
public:
  typedef struct LABELSTRUCT {
    friend gOutput &operator<<(gOutput &op,const LABELSTRUCT &l);
    wxString label;
    double x,y;
    LABELSTRUCT(void):label(""),x(0.0),y(0.0) {}
    LABELSTRUCT &operator=(const LABELSTRUCT &l)
      {label=l.label;x=l.x;y=l.y; return *this;}
    int operator==(const LABELSTRUCT &l)
      {return (label==l.label && x==l.x && y==l.y);}
    int operator!=(const LABELSTRUCT &l)
      {return !(*this==l);}
  } label_struct;
private:
  PxiDrawSettings *draw_settings;       // draw settings, see above

  ExpData *exp_data;                    // experimental data overlay
  gBlock<label_struct> labels;          // labels for generic text
  DataLine probs;                       // a line of data
  gBlock<FileHeader> headers;           // all the basic info about the file
  Bool	stopped;
  double cur_e;
  bool painting;
  bool m_landscape;                     // landscap mode if true
  int m_width, m_height;                // width, height of page
  double m_scale;                        // scaling factor
  double m_ppu;                        // pixels per scroll unit

  int Width(void) const 
    {if(m_landscape) return m_height; return m_width;}
  int Height(void) const 
    {if(m_landscape) return m_width; return m_height;}
  void SetLandscape(bool flag) 
    {m_landscape = flag;}
  bool GetLandscape(void) 
    {return m_landscape;}
  double GetScale(void) const {return m_scale;} 
  void SetScale(double x) 
    {
      m_scale = x; 
      SetScrollbars(m_ppu,m_ppu,GetScale()*Width()/m_ppu, GetScale()*Height()/m_ppu);
    }

  void DoPlot_X(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);
  void DoPlot_2(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);
  void DoPlot_3(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);

  void PlotData_X(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level);
  void PlotData_2(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level=1);
  void PlotData_3(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level=1);

  void DrawExpPoint_X(wxDC &dc, const PlotInfo &thisplot, 
		      double cur_e,int iset,int st,
		      int x0, int y0, int ch,int cw);
  void DrawExpPoint_2(wxDC &dc, const PlotInfo &thisplot, 
		      double cur_e,int pl1,int st1,
		      int pl2,int st2,int x0, int y0, int cw, int ch);
  
  void DrawExpPoint_3(wxDC &dc,const PlotInfo &thisplot, 
		      double cur_e,int iset,int st1,int st2,
		      int x0, int y0, int cw,int ch);

  void PlotLabels(wxDC &dc,int ch,int cw);


  double CalcY_X(double y,int x0, int ch, const PlotInfo &thisplot);
  double CalcX_X(double x,int y0, int cw, const PlotInfo &thisplot);
  double CalcY_3(double p1,int x0, int y0, int ch,int cw);
  double CalcX_3(double p1,double p2,int x0, int y0, int ch,int cw, const PlotInfo &thisplo);
  
  void DrawToken(wxDC &dc,double x,double y,int st);
public:
  PxiCanvas(wxFrame *frame, const wxPoint &p_position,
	    const wxSize &p_size, int style=0,const char *file_name=NULL);
  void Update(wxDC& dc,int device);
  void OnPaint(wxPaintEvent &ev);
  void OnChar(wxKeyEvent &ev);
  void OnEvent(wxMouseEvent &ev);
  void ShowDetail(void);
  void StopIt(void);
  void Render(void) {wxClientDC dc(this); dc.BeginDrawing();Render(dc);dc.EndDrawing();}
  void Render(wxDC &dc) {Update(dc,PXI_UPDATE_SCREEN);}
  void SetPage(int page) {draw_settings->SetPage(page);}
  void SetNextPage(void) {draw_settings->SetNextPage();}
  void SetPreviousPage(void) {draw_settings->SetPreviousPage();}

  PxiDrawSettings *DrawSettings(void) {return draw_settings;}
  void NewExpData(ExpDataParams &P);
  wxString PxiName(void) const {return headers[1].FileName();}
  const FileHeader &Header(int i) {return headers[i];}
  void  AppendHeader(const FileHeader &h) {headers.Append(h);}

  DECLARE_EVENT_TABLE()
};

// Define a new frame

class PxiFrame: public wxFrame
{
private:
  wxFileHistory m_fileHistory;

  // Menu event handlers
  void OnFileLoad(wxCommandEvent &);
  void OnMRUFile(wxCommandEvent &);
  void OnDataGridSolve(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);

  void MakeToolbar(void);

public:
  PxiFrame(wxFrame *frame,  const wxString &p_filename,
	   const wxPoint &p_position, const wxSize &p_size, 
	   long p_style = wxDEFAULT_FRAME_STYLE);
  virtual ~PxiFrame();

  void LoadFile(const wxString &);

  DECLARE_EVENT_TABLE()
};

class PxiChildToolbar;

class PxiChild: public wxFrame
{
private:
  PxiFrame *parent;
  PxiCanvas *canvas;

  wxPrintData  *m_printData;
  wxPageSetupData* m_pageSetupData;
  gBlock<double>  scaleValues;
  PxiChildToolbar *m_toolbar;

  void OnPrint(wxCommandEvent &);  // output to printer
  void OnPrintPreview(wxCommandEvent &);  // output to printer

  void OnQuit(wxCommandEvent &);
  void OnOverlayData(wxCommandEvent &);
  void OnOverlayFile(wxCommandEvent &);
  void OnFileDetail(wxCommandEvent &);
  void OnFileOutput(wxCommandEvent &);
  void OnDisplayOptions(wxCommandEvent &);
  void OnPrefsFontAxis(wxCommandEvent &);
  void OnPrefsFontLabel(wxCommandEvent &);
  void OnPrefsFontOverlay(wxCommandEvent &);
  void OnPrefsScale(wxCommandEvent &);
  void MarkScaleMenu(void);
  void OnPrefsColors(wxCommandEvent &);
  void OnPrefsZoomIn(wxCommandEvent &);
  void OnPrefsZoomOut(wxCommandEvent &);
  void OnNextPage(wxCommandEvent &);
  void OnPreviousPage(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);
  void OnChar(wxKeyEvent &ev) {canvas->OnChar(ev);}
  void OnEvent(wxMouseEvent &ev) {canvas->OnEvent(ev);}
  void MakeMenus(void);
public:
  PxiChild(PxiFrame *p_parent, const wxString &p_title);
  ~PxiChild(void);

  void  print_eps(wxOutputOption fit);                 // output to postscript file
  void  print(wxOutputOption fit,bool preview=false);  // output to printer (WIN only)
  void  print_mf(wxOutputOption fit,bool save_mf=false);  // copy to clipboard (WIN Only)
  void  save_mf(wxOutputOption fit,bool save_mf=false);  // save clipboard (WIN Only)

  void  LoadFile(const wxString &file) {parent->LoadFile(file);}

  DECLARE_EVENT_TABLE()
};

#endif // PXI_H
