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
#include "../gui/wxmisc.h"

//
// wxWindows does not provide this constant
const int wxID_HELP_INDEX = 1310;

const int PXI_VIEW_OPTIONS = 128;
const int PXI_VIEW_DETAIL = 123;
const int PXI_VIEW_ZOOM = 137;
const int PXI_VIEW_ZOOM_IN = 150;
const int PXI_VIEW_ZOOM_OUT = 151;

const int PXI_FORMAT_LAMBDA_AXIS = 130;
const int PXI_FORMAT_PROFILE_AXIS = 131;
const int PXI_FORMAT_TITLE = 133;
const int PXI_FORMAT_LEGEND = 134;
const int PXI_FORMAT_OVERLAY = 132;
const int PXI_FORMAT_COLORS = 124;

#define	PXI_NEW_WINDOW	      117
#define	PXI_JUST_REPAINT      115
#define	PXI_NEW_FILE_REPAINT  116
#define	PXI_UPDATE_SCREEN     119
#define	PXI_UPDATE_FILE       120
#define	PXI_UPDATE_PRINTER    121
#define	PXI_UPDATE_METAFILE   122

#define	PXI_NO_SET_STOP      -1.0
#define	PXI_SET_STOP          127

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

class PxiPlot;

class PxiPrintout: public wxPrintout
{
private:
  PxiPlot &canvas;
  
public:
  PxiPrintout(PxiPlot &canvas, char *title = "PXI printout");
  ~PxiPrintout();
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};

class PlotInfo
{
private:
  bool showAxis, showTicks, showNums, showLabels, showSquare;
  int number;                         // plot number

  // Bool instead of bool needeed here for template ambiguity in gBlock for BC
  typedef gBlock<bool> show_actions;
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

  bool GetStrategyShow(int j, int k) const {return strategies[j][k];}
  int GetNumStrats(int j) const {return strategies[j].Length();}
  int GetNumIsets(void) const { return 1; }
  int GetPlotNumber(void) const {return number;}
  void SetPlotNumber(int i) {number=i;}

  bool ShowAxis(void) const {return showAxis;}
  bool ShowTicks(void) const {return showTicks;}
  bool ShowNums(void) const {return showNums;}
  bool ShowLabels(void) const {return showLabels;}
  bool ShowSquare(void) const {return showSquare;}

  void SetStrategyShow(int j, int k, bool flag) {strategies[j][k]=flag;}
  void SetShowAxis(bool flag) {showAxis = flag;}
  void SetShowTicks(bool flag) {showTicks = flag;}
  void SetShowNums(bool flag) {showNums = flag;}
  void SetShowLabels(bool flag) {showLabels = flag;}
  void SetShowSquare(bool flag) {showSquare = flag;}
};

class PxiFrame : public wxFrame {
  friend class PxiChild;
private:
  wxFileHistory m_fileHistory;

  // Menu event handlers
  void OnFileOpen(wxCommandEvent &);
  void OnMRUFile(wxCommandEvent &);
  void OnDataGridSolve(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

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

#endif // PXI_H
