//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to PXI plot for plotting 2 values
//

#ifndef PXIPLOT2_H
#define PXIPLOT2_H

#include "pxi.h"
#include "pxiconf.h"
#include "pxiplot.h"

class PxiPlot2 : public PxiPlot {
friend class PxiChild;
private:
  // Implementing virtual functions 
  void DoPlot(wxDC& dc,const PlotInfo &thisplot,
	      int x0, int y0, int cw,int ch, int level=1);
  void PlotData_2(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level=1);
  void DrawExpPoint_2(wxDC &dc, const PlotInfo &thisplot, 
		      double cur_e,int pl1,int st1,
		      int pl2,int st2,int x0, int y0, int cw, int ch);
  void PlotAxis_2(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, int cw,int ch);

  void PlotLabels(wxDC &dc,int ch,int cw);
  
  // Event handlers
  void OnEvent(wxMouseEvent &);

public:
  PxiPlot2(wxWindow *p_parent, const wxPoint &p_position,
	   const wxSize &p_size, const FileHeader &, int p_page,
	   const ExpData &p_expData);
  virtual ~PxiPlot2();

  DECLARE_EVENT_TABLE()
};

#endif  // PXIPLOT2_H

