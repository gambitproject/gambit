//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Inteface to PXI plot for plotting N values
//

#ifndef PXIPLOTN_H
#define PXIPLOTN_H

#include "pxi.h"
#include "pxiconf.h"
#include "pxiplot.h"

class PxiPlotN : public PxiPlot {
friend class PxiChild;
private:
  // Implementing virtual functions
  void DoPlot(wxDC& dc,
	      int x0, int y0, int cw,int ch, int level=1);
  void PlotData_X(wxDC& dc,
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level);
  void DrawExpPoint_X(wxDC &dc,
		      double cur_e,int iset,int st,
		      int x0, int y0, int ch,int cw);
  void PlotAxis_X(wxDC& dc, int x0, int y0, int cw,int ch,
		  const PxiAxisProperties &p_horizProps, 
		  const PxiAxisProperties &p_vertProps,
		  float log_step);

  double CalcY_X(double y,int x0, int ch);
  double CalcX_X(double x,int y0, int cw);
  
  // Event handlers
  void OnEvent(wxMouseEvent &);

public:
  PxiPlotN(wxWindow *p_parent, const wxPoint &p_position,
	   const wxSize &p_size, const FileHeader &, int p_page,
	   const ExpData &p_expData);
  virtual ~PxiPlotN();

  DECLARE_EVENT_TABLE()
};

#endif  // PXIPLOTN_H

