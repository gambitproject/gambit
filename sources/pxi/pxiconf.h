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
  wxBrush       clear_brush, exp_data_brush;
  
  int		color_mode;   // COLOR_EQU, COLOR_PROB, or COLOR_NONE
  bool		connect_dots;   // connect dots on plot.  
  bool          restart_overlay_colors; // new set of colors for each overlay 
  
  bool CheckPlot3Mode(void);
  bool CheckPlot2Mode(void);

public:
  PxiDrawSettings(const FileHeader &p_header, int p_index);
  
  // Get* functions
  
  // ColorMode, how to color the data: by equilibrium #, prob #, or just a constant
  int GetColorMode(void) const {return color_mode;}
  void SetColorMode(int mode ) {color_mode=mode;}

  // ConnectDots, determines if the regular data file dots will be connected
  bool ConnectDots(void) const {return connect_dots;}
  void SetConnectDots(bool flag) {connect_dots = flag;}
  // RestartOverlayColors, determines if the colors used for the equilibria
  // lines will restart at 0 for each of the file overlays or will increment
  // from the previous file
  bool RestartOverlayColors(void) const {return restart_overlay_colors;}
  void SetRestartOverlayColors(bool flag) {restart_overlay_colors = flag;}

  // Set* functions

  void SetOptions(wxWindow *parent);

  const wxBrush &GetDataBrush(void) const {return exp_data_brush;}
  const wxBrush &GetClearBrush(void) const {return clear_brush;}
};


#endif  // PXICONF_H
