//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class to store plot configuration settings
//

#ifndef PXICONF_H
#define PXICONF_H

#include "pxi.h"

class PxiDrawSettings {
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


#endif  // PXICONF_H
