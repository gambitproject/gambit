#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include <assert.h>
#include "axis.h"
#include "gmisc.h"
#include "pxi.h"
#include "pxicanvas.h"

#define XOFF		30
#define YGRID		ch/YGRIDS

/***************************** PLOT AXIS X ********************************/
void PlotAxis_X(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, int cw,int ch,
		const PxiAxisProperties &p_horizProps, 
		const PxiAxisProperties &p_vertProps,
		float log_step)
{
  int i1;
  float tempf;
  double p,k,x_per_grid;
  char axis_label_str[90];
  double xStart = p_horizProps.m_scale.GetMinimum();
  double xEnd = p_horizProps.m_scale.GetMaximum();
  double yStart = p_vertProps.m_scale.GetMinimum();
  double yEnd = p_vertProps.m_scale.GetMaximum();
  
  if (thisplot.ShowAxis()) {
    dc.DrawLine(x0,   y0,
		x0,   y0-ch);
    dc.DrawLine(x0,   y0,
		x0+cw,y0);
  }

  int xgrid = cw / p_horizProps.m_scale.m_divisions; 
  int ygrid = ch / p_vertProps.m_scale.m_divisions;

  if (!p_horizProps.m_scale.m_useLog)  {   // arithmetic scale
    for (i1=0; i1 <= p_horizProps.m_scale.m_divisions; i1++) {
      if (thisplot.ShowTicks())
      dc.DrawLine(x0+i1*xgrid,      y0-GRID_H, 
		  x0+i1*xgrid,      y0+GRID_H);
      if (thisplot.ShowNums()) {
	tempf=xStart+fabs((xStart-xEnd)/p_horizProps.m_scale.m_divisions)*i1;
	sprintf(axis_label_str,"% 3.2f",tempf);
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0+i1*xgrid-tw/2, y0+GRID_H);
      }
    }
  }
  else {																	// geometric scale
    log_step+=1;	// need to translate .1 to 1.1 that we use by convention
    x_per_grid=(log(xEnd/xStart)/log(log_step))/p_horizProps.m_scale.m_divisions;
#ifdef	REAL_LOG_SCALE
    for (i1=0;i1<XGRIDS;i1++) {
      k=x_per_grid*(i1);
      p=x_start*pow(log_step,k);
      double x=(cw-2*XOFF)/(x_end-x_start)*(p-x_start);
      dc.DrawLine(x0+x,     y0-GRID_H,
		  x0+x,     y0+GRID_H);
      sprintf(axis_label_str,"% 3.2f",p);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+x-tw/2,y0+GRID_H);
    }
#else
    for (i1 = 0; i1 <= p_horizProps.m_scale.m_divisions; i1++) {
      k=x_per_grid*(i1);
      p=xStart*pow(log_step,k);
      sprintf(axis_label_str,"% 3.2f",p);
      if (thisplot.ShowTicks())
	dc.DrawLine(x0+i1*xgrid,    y0-GRID_H,
		    x0+i1*xgrid,    y0+GRID_H);
      if (thisplot.ShowNums()) {
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0+i1*xgrid-tw/2,y0+GRID_H);
      }
    }
#endif
  }
  for (i1 = 0; i1 <= p_vertProps.m_scale.m_divisions; i1++) {
    if (thisplot.ShowTicks())
      dc.DrawLine(x0-GRID_H,  y0-ch+i1*ygrid,
		  x0+GRID_H,  y0-ch+i1*ygrid);
      if (thisplot.ShowNums()) {
	tempf=yEnd-fabs((yStart-yEnd)/p_vertProps.m_scale.m_divisions)*i1;
	sprintf(axis_label_str,"%3.2f",tempf);
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0-GRID_H-tw,  y0-ch+i1*ygrid-th/2);
      }
  }
}

/***************************** PLOT AXIS X ********************************/

void PlotAxis_2(wxDC& dc, const PlotInfo &thisplot, 
		int x0, int y0, int cw,int ch)
{
#ifdef NOT_PORTED_YET
  int i1;
  float tempf;
  char axis_label_str[90];
  int num_plots=0;
  float x_start = thisplot.GetMinY();
  float x_end = thisplot.GetMaxY();
  float y_start = thisplot.GetMinY();
  float y_end = thisplot.GetMaxY();

  // temporary replacement of old constant; should be made configurable
  const int XGRIDS = 10;
  int XGRID = XGRIDS / cw;

  if (thisplot.ShowAxis()) {
    dc.DrawLine(x0, y0, x0, y0-ch);
    dc.DrawLine(x0, y0, x0+cw, y0);}
  if (thisplot.ShowSquare()) {
    dc.DrawLine(x0+cw, y0-ch, x0, y0-ch);
    dc.DrawLine(x0+cw, y0-ch, x0+cw, y0);}
  for (i1=0;i1<=XGRIDS;i1++) {
    if (thisplot.ShowTicks()) {
      dc.DrawLine(x0+i1*XGRID, y0-GRID_H, x0+i1*XGRID, y0+GRID_H);
      if (thisplot.ShowSquare())
	dc.DrawLine( x0+i1*XGRID, y0-ch-GRID_H, x0+i1*XGRID,  y0-ch+GRID_H);
    }
    if (thisplot.ShowNums()) {
      tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
      sprintf(axis_label_str,"%2.1f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+i1*XGRID-tw/2, y0+GRID_H);
      if (thisplot.ShowSquare())
	dc.DrawText(axis_label_str,
		    x0+i1*XGRID-tw/2,y0-ch-GRID_H-th);
    }
  }
  for (i1=0;i1<=YGRIDS;i1++) {
    if (thisplot.ShowTicks()) {
      dc.DrawLine(
		  x0-GRID_H,   y0-ch+i1*YGRID,
		  x0+GRID_H,   y0-ch+i1*YGRID);
      if (thisplot.ShowSquare())
	dc.DrawLine(
		    x0+cw-GRID_H,  y0-ch+i1*YGRID,
		    x0+cw+GRID_H,  y0-ch+i1*YGRID);
    }
    if (thisplot.ShowNums()) {
      tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0-GRID_H-tw,   y0-ch+i1*YGRID-th/2);
      if (thisplot.ShowSquare())
	dc.DrawText(axis_label_str,
		    x0+cw+GRID_H, y0-ch+i1*YGRID-th/2);
    }
  }

#endif // NOT_PORTED_YET
}

/******************************** PLOT AXIS 3 *****************************/
// Draws a triangular axis set for the pseudo-3D mode in PXI.  It can plot one or
// two triangles, corresponding to num_plots = 0,1 respectively.  Note that the
// axis scaling is not currently used.

void PlotAxis_3(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, int cw,int ch,
		wxString label[])
{
  dc.DrawLine(x0,        y0,
	      x0+cw,     y0);               // bottom
  dc.DrawLine(x0,        y0,
	      (int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT));  // left side
  dc.DrawLine(x0+cw,     y0,
	      (int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT));  // right side
  if (thisplot.ShowAxis()) { 		// now draw the bisectors
    dc.DrawLine(x0,          y0,
		(int) (x0+cw*3/4), (int) (y0-PXI_3_HEIGHT/2));
    dc.DrawLine(x0+cw,     y0,
		(int) (x0+cw/4), (int) (y0-PXI_3_HEIGHT/2));
    dc.DrawLine((int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT),
		(int) (x0+cw/2),   y0);
  }
  if (thisplot.ShowSquare()) {
    wxCoord tw,th;
    dc.GetTextExtent(label[2],&tw,&th);
    dc.DrawText(label[2],
		x0-tw,        y0);
    dc.GetTextExtent(label[0],&tw,&th);
    dc.DrawText(label[0],
		(int) (x0+cw/2-tw/2), (int) (y0-PXI_3_HEIGHT-th));
    dc.GetTextExtent(label[1],&tw,&th);
    dc.DrawText(label[1],
		x0+cw,        y0);
  }
}


