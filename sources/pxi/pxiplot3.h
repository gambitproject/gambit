//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to PXI plot for plotting 3 values
//

#ifndef PXIPLOT3_H
#define PXIPLOT3_H

#include "pxi.h"
#include "pxiplot.h"

class PxiPlot3 : public PxiPlot {
friend class PxiChild;
private:
  // Implementing virtual functions
  void DoPlot(wxDC& dc, int x0, int y0, int cw,int ch, int level=1);
  void PlotData_3(wxDC& dc,
		  int x0, int y0, int cw,int ch,
		  const PxiFile &f_header, int level=1);
  void DrawExpPoint_3(wxDC &dc,
		      double cur_e,int iset,int st1,int st2,
		      int x0, int y0, int cw,int ch);
  void PlotAxis_3(wxDC& dc, int x0, int y0, int cw,int ch, 
		  wxString labels[]);

  double CalcY_3(double p1,int x0, int y0, int ch,int cw);
  double CalcX_3(double p1,double p2,int x0, int y0, int ch,int cw);
  
  // Event handlers
  void OnEvent(wxMouseEvent &);

public:
  PxiPlot3(wxWindow *p_parent, const wxPoint &p_position,
	   const wxSize &p_size, const gArray<PxiFile *> &, int p_page,
	   const ExpData &p_expData);
  virtual ~PxiPlot3();

  DECLARE_EVENT_TABLE()
};

#endif  // PXIPLOT3_H

