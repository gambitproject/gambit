#ifndef PXI_H
#define PXI_H

// Program: PXI.  [Plot X V2.0]
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

#define PXI_QUIT              100
#define PXI_PRINT             101
#define PXI_ABOUT             108
#define PXI_LOAD_FILE         111
#define PXI_PRINT_EPS         112  // Windows-only option
#define PXI_COPY_MF           113  // Windows-only option
#define PXI_SAVE_MF           114  // Windows-only option
#define	PXI_NEW_WINDOW	      117
#define	PXI_CHILD_QUIT	      118
#define	PXI_JUST_REPAINT      115
#define	PXI_NEW_FILE_REPAINT  116
#define	PXI_UPDATE_SCREEN     119
#define	PXI_UPDATE_FILE       120
#define	PXI_UPDATE_PRINTER    121
#define	PXI_UPDATE_METAFILE   122
#define	PXI_FILE_DETAIL	      123
#define	PXI_PLOT_X              0
#define	PXI_PLOT_3              1
#define PXI_PLOT_2              2
#define	PXI_SHOW_GAME         126
#define	PXI_NO_SET_STOP      -1.0
#define	PXI_SET_STOP          127
#define	PXI_KEY_STOP          WXK_SPACE
#define	PXI_DISPLAY_OPTIONS   128

#define PXI_DATA_GRID         205
#define PXI_DATA_ONEDOT       210
#define PXI_DATA_OVERLAY_DATA 215
#define PXI_DATA_OVERLAY_FILE 220


#define PXI_PLOT_2_STEP        .1
/*********************** PXI.H ******************************************/
#define MAXN    10

// Define a new application
class PxiApp: public wxApp
{
public:
  wxFrame *OnInit(void);
};

#define	OVERLAY_TOKEN   1
#define	OVERLAY_NUMBER  2

#define MIN_TOKEN_SIZE  2
#define MAX_TOKEN_SIZE  10
#define DEF_TOKEN_SIZE  4

#define TOP_PLOT        0
#define	BOTTOM_PLOT     1

#define	COLOR_EQU       1
#define COLOR_PROB      2
#define COLOR_NONE      3

class PxiDrawSettings
{
private:
  int 		plot_mode;
  int		data_mode;
  int		one_or_two;
  Bool		show_game;
  Bool		connect_dots;
  int		color_mode;
  Bool		restart_overlay_colors;
  int		num_infosets;
  typedef	gBlock<Bool> show_player_strategies;
  friend	gOutput &operator<<(gOutput &op,const show_player_strategies &p);
  gBlock<show_player_strategies>	strategy_show;
  gBlock<int>	plot_top,plot_bottom;
  //	int		plot_left,plot_right;
  float		stop_min,stop_max,data_min,data_max;
  double	l_start,l_stop,l_step;
  wxFont	*overlay_font;
  wxFont	*label_font;
  int		overlay_symbol;
  Bool		overlay_lines;
  int		overlay_token_size;
  unsigned int plot3_features,plotx_features,plot2_features;
  Bool CheckPlot3Mode(void);
  Bool CheckPlot2Mode(void);
  void AskPlotFeatures(void);
  static void plot_features_func(wxButton &ob,wxEvent &);
public:
  static void overlay_func(wxButton &ob,wxEvent &);
  static void overlay_font_func(wxButton &ob,wxEvent &);
  static void label_font_func(wxButton &ob,wxEvent &);
  static void disp_func(wxButton &ob,wxEvent &);
  PxiDrawSettings(FileHeader &header);
  // Get* functions
  // PlotMode, returns the type of plot currently selected:
  // one of PLOT_X | PLOT_3 | PLOT_2
  int 	GetPlotMode(void) {return plot_mode;}
  // NumPlots, returns either 1 or 2, corresponding to # of grids/plots per page
  // currently only works for PlotX
  int	GetNumPlots(void) {return one_or_two;}
  // DataMode, returns one of DATA_TYPE_ARITH | DATA_TYPE_LOG indicating data type
  int	GetDataMode(void) {return data_mode;}
  // ColorMode, how to color the data: by equilibrium #, prob #, or just a constant
  int	GetColorMode(void) {return color_mode;}
  // ShowGame, tells whether to display the game matrix
  Bool	GetShowGame(void) {return show_game;}
  // StrategyShow, tells whether player p's strategy #s is to be plotted
  Bool	GetStrategyShow(int p,int s) {return strategy_show[p][s];}
  // StopMin, returns the value to start plotting at
  double GetStopMin(void) {return stop_min;}
  // StopMax, returns the value to stop plotting at
  double GetStopMax(void) {return stop_max;}
  // MaxL, returns the maximum labmda value in the data file
  double GetMaxL(void) {return l_stop;}
  // MinL, returns the minimum labmda value in the data file
  double GetMinL(void) {return l_start;}
  // DelL, return the lambda step
  double GetDelL(void) {return l_step;}
  // DataMin, returns the smallest value of y to plot
  double GetDataMin(void) {return data_min;}
  // DataMax, returns the largest value of y to plot
  double GetDataMax(void) {return data_max;}
  // If called with no arguments or 0, returns the total # of players to be
  // displayed on the top plot for PlotX mode, or the left triangle for
  // Plot3 mode.  If called with an argument, returns the actual player #
  int	GetPlotTop(int i=0)
    {if (i==0) return plot_top.Length(); else return plot_top[i];}
  // PlotBottom, see PlotTop
  int	GetPlotBottom(int i=0)
    {if (i==0) return plot_bottom.Length(); else return plot_bottom[i];}
  // PlotLeft,
  //	int GetPlotLeft(void) {return plot_left;}
  //	int GetPlotRight(void) {return plot_right;}
  // OverlayFont, if the experimental data overlay is done using the number
  // of the point in the file, this font is used to display that number
  wxFont *GetOverlayFont(void) {return overlay_font;}
  // LabelFont, returns the font to be used for labels [created w/ Shift-Click]
  wxFont *GetLabelFont(void) {return label_font;}
  // OverlaySym, returns one of : OVERLAY_CIRCLE | OVERLAY_NUMBER, indicating
  // if the experimental data overlay points will be plotted using little circles
  // or their number in the data file
  int GetOverlaySym(void) {return overlay_symbol;}
  // OverlaySize, determines the size of the tokens use for overlay
  int GetTokenSize(void) {return overlay_token_size;}
  // OverlayLines, determines if the experimental data overlay dots will be connected
  Bool GetOverlayLines(void) {return overlay_lines;}
  // ConnectDots, determines if the regular data file dots will be connected
  Bool ConnectDots(void) {return connect_dots;}
  // RestartOverlayColors, determines if the colors used for the equilibria
  // lines will restart at 0 for each of the file overlays or will increment
  // from the previous file
  Bool RestartOverlayColors(void) {return restart_overlay_colors;}
  // RangeX, determines if x falls in the drawable range
  Bool RangeX(double x) {return (x>stop_min && x<stop_max);}
  // RangeY, determines if y falls in the drawable range
  Bool RangeY(double y) {return (y>data_min && y<data_max);}
  // PlotFeatures determines if axis and labels,etc. are to be drawn
  unsigned int PlotFeatures(void);
  // Set* functions
  void	SetOptions(wxFrame *parent);
  void	SetStopMax(double sm) {stop_max=sm;}
  void	SetStopMin(double sm) {stop_min=sm;}
  void	ResetSetStop(void) {stop_min=l_start;stop_max=l_stop;}
  
  void	SetOverlayFont(wxFont *f) {overlay_font=f;}
  void	SetOverlaySym(int s) {overlay_symbol=s;}
  void	SetOverlayLines(Bool l)	{overlay_lines=l;}
  void	SetTokenSize(int s) {overlay_token_size=s;}
  void	SetLabelFont(wxFont *f)	{label_font=f;}
  
  void	SetPlotFeatures(unsigned int feat);
};



// Define a new canvas
class PxiCanvas: public wxCanvas
{
public:
  typedef struct LABELSTRUCT {
    friend gOutput &operator<<(gOutput &op,const LABELSTRUCT &l);
    gString label;
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
  void PlotData_X(wxDC& dc,int ch,int cw,const FileHeader &f_header,int level);
  void PlotData_3(wxDC& dc,int ch,int cw,const FileHeader &f_header,int level);
  void PlotData_2(wxDC& dc,int ch,int cw,const FileHeader &f_header);
  void ShowGame(wxDC& dc,int cw,int ch,const FileHeader &header);
  void PlotLabels(wxDC &dc,int ch,int cw);
  
  void DrawExpPoint_X(wxDC &dc,double cur_e,int iset,int st,int ch,int cw,int plot);
  double CalcY_X(double y,int ch,int plot);
  double CalcX_X(double x,int cw);
  
  void DrawExpPoint_3(wxDC &dc,double cur_e,int iset,int st1,int st2,int ch,int cw,int plot);
  double CalcY_3(double p1,int ch,int cw);
  double CalcX_3(double p1,double p2,int ch,int cw,int plot);
  
  void DrawExpPoint_2(wxDC &dc,double cur_e,int pl1,int st1,int pl2,int st2,int side);
  
  void DrawToken(wxDC &dc,double x,double y,int st);
public:
  PxiCanvas(wxFrame *frame, int x, int y, int w, int h,int style=0,const char *file_name=NULL);
  void Update(wxDC& dc,int device);
  void OnPaint(void);
  void OnChar(wxKeyEvent &ev);
  void OnEvent(wxMouseEvent &ev);
  void ShowDetail(void);
  void GetGame(void);
  void StopIt(void);
  void MakeOverlayData(void);
  void MakeOverlayFile(void);
  void AddFileOverlay(void);
  PxiDrawSettings *DrawSettings(void) {return draw_settings;}
};

// Define a new frame
class PxiFrame: public wxFrame
{
public:
  PxiFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
  Bool OnClose(void);
  void MakeOneDot(char *in_filename=NULL,char *out_filename=NULL);
  char *MakeDataFile(void);
  void MakeChild(char *file_name=NULL);
  void OnMenuCommand(int id);
};

class PxiChild: public wxFrame
{
public:
  PxiChild(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
  ~PxiChild(void);
  Bool OnClose(void);
  void OnMenuCommand(int id);
  PxiCanvas *canvas;
};

#endif
