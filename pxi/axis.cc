#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "wx/wx.h"
#pragma	hdrstop
#include <assert.h>
#include "axis.h"
#include "gmisc.h"

#define XOFF			30
#define YSCALE 		(ch-3*XOFF)/2
#define XGRID			(cw-2*XOFF)/XGRIDS
#define YGRID			(ch-ch/2*num_plots-2*XOFF)/YGRIDS
#define MY_XGRID		cw/XGRIDS
#define MY_YGRID		ch/YGRIDS

/***************************** PLOT AXIS X ********************************/
void PlotAxis_X(wxDC& dc, float x_start,float x_end,float y_start,
		float y_end,int x0, int y0, int cw,int ch,int plot_type,
		unsigned int features,float log_step)
{
  int i1;
  float tempf;
  double p,k,x_per_grid;
  char axis_label_str[90];
  
  if (features&DRAW_AXIS) {
    dc.DrawLine(
		x0,   y0,
		x0,   y0-ch);
    dc.DrawLine(
		x0,   y0,
		x0+cw,y0);
  }
  if (plot_type==DATA_TYPE_ARITH) {   	// arithmetic scale
    for (i1=0;i1<=XGRIDS;i1++) {
      if (features&DRAW_TICKS)
	dc.DrawLine(
		    x0+i1*MY_XGRID,      y0-GRID_H, 
		    x0+i1*MY_XGRID,      y0+GRID_H);
      if (features&DRAW_NUMS) {
	tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
	  sprintf(axis_label_str,"% 3.2f",tempf);
	  wxCoord tw,th;
	  dc.GetTextExtent(axis_label_str,&tw,&th);
	  dc.DrawText(axis_label_str,
		      x0+i1*MY_XGRID-tw/2, y0+GRID_H);
      }
    }
  }
  else {																	// geometric scale
    log_step+=1;	// need to translate .1 to 1.1 that we use by convention
    x_per_grid=(log(x_end/x_start)/log(log_step))/XGRIDS;
#ifdef	REAL_LOG_SCALE
    for (i1=0;i1<XGRIDS;i1++) {
      k=x_per_grid*(i1);
      p=x_start*pow(log_step,k);
      double x=(cw-2*XOFF)/(x_end-x_start)*(p-x_start);
      dc.DrawLine(
		  x0+x,     y0-GRID_H,
		  x0+x,     y0+GRID_H);
      sprintf(axis_label_str,"% 3.2f",p);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+x-tw/2,y0+GRID_H);
    }
#else
    for (i1=0;i1<=XGRIDS;i1++) {
      k=x_per_grid*(i1);
      p=x_start*pow(log_step,k);
      sprintf(axis_label_str,"% 3.2f",p);
      if (features&DRAW_TICKS)
	dc.DrawLine(
		    x0+i1*MY_XGRID,    y0-GRID_H,
		    x0+i1*MY_XGRID,    y0+GRID_H);
      if (features&DRAW_NUMS) {
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0+i1*MY_XGRID-tw/2,y0+GRID_H);
      }
    }
#endif
  }
  for (i1=0;i1<=YGRIDS;i1++) {
    if (features&DRAW_TICKS)
      dc.DrawLine(
		  x0-GRID_H,  y0-ch+i1*MY_YGRID,
		  x0+GRID_H,  y0-ch+i1*MY_YGRID);
      if (features&DRAW_NUMS) {
	tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
	sprintf(axis_label_str,"%3.2f",tempf);
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0-GRID_H-tw,  y0-ch+i1*MY_YGRID-th/2);
      }
  }
}

/***************************** PLOT AXIS X ********************************/

void PlotAxis_2(wxDC& dc,float x_start,float x_end,float y_start,float y_end,
		int x0, int y0, int cw,int ch,unsigned int features)
{
  int i1;
  float tempf;
  char axis_label_str[90];
  int num_plots=0;
  
  if (features&DRAW_AXIS) {
    dc.DrawLine(x0, y0, x0, y0-ch);
    dc.DrawLine(x0, y0, x0+cw, y0);}
  if (features&DRAW_SQUARE) {
    dc.DrawLine(x0+cw, y0-ch, x0, y0-ch);
    dc.DrawLine(x0+cw, y0-ch, x0+cw, y0);}
  for (i1=0;i1<=XGRIDS;i1++) {
    if (features&DRAW_TICKS) {
      dc.DrawLine(x0+i1*MY_XGRID, y0-GRID_H, x0+i1*MY_XGRID, y0+GRID_H);
      if (features&DRAW_SQUARE)
	dc.DrawLine( x0+i1*MY_XGRID, y0-ch-GRID_H, x0+i1*MY_XGRID,  y0-ch+GRID_H);
    }
    if (features&DRAW_NUMS) {
      tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+i1*MY_XGRID-tw/2, y0+GRID_H);
      if (features&DRAW_SQUARE)
	dc.DrawText(axis_label_str,
		    x0+i1*MY_XGRID-tw/2,y0-ch-GRID_H-th);
    }
  }
  for (i1=0;i1<=YGRIDS;i1++) {
    if (features&DRAW_TICKS) {
      dc.DrawLine(
		  x0-GRID_H,   y0-ch+i1*MY_YGRID,
		  x0+GRID_H,   y0-ch+i1*MY_YGRID);
      if (features&DRAW_SQUARE)
	dc.DrawLine(
		    x0+cw-GRID_H,  y0-ch+i1*MY_YGRID,
		    x0+cw+GRID_H,  y0-ch+i1*MY_YGRID);
    }
    if (features&DRAW_NUMS) {
      tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0-GRID_H-tw,   y0-ch+i1*MY_YGRID-th/2);
      if (features&DRAW_SQUARE)
	dc.DrawText(axis_label_str,
		    x0+cw+GRID_H, y0-ch+i1*MY_YGRID-th/2);
    }
  }
}

/******************************** PLOT AXIS 3 *****************************/
// Draws a triangular axis set for the pseudo-3D mode in PXI.  It can plot one or
// two triangles, corresponding to num_plots = 0,1 respectively.  Note that the
// axis scaling is not currently used.

void PlotAxis_3(wxDC& dc, int x0, int y0, int cw,int ch,
		unsigned int features, wxString label[])
{
  dc.DrawLine(
	      x0,        y0,
	      x0+cw,     y0);               // bottom
  dc.DrawLine(
	      x0,        y0,
	      x0+cw/2,   y0-PXI_3_HEIGHT);  // left side
  dc.DrawLine(
	      x0+cw,     y0,
	      x0+cw/2,   y0-PXI_3_HEIGHT);  // right side
  if (features&DRAW_AXIS) { 		// now draw the bisectors
    dc.DrawLine(
		x0,          y0,
		x0+cw*3/4, y0-PXI_3_HEIGHT/2);
    dc.DrawLine(
		x0+cw,     y0,
		x0+cw/4,   y0-PXI_3_HEIGHT/2);
    dc.DrawLine(x0+cw/2,   y0-PXI_3_HEIGHT,
		x0+cw/2,   y0);
  }
  if (features&DRAW_LABELS) {
    wxCoord tw,th;
    dc.GetTextExtent(label[2],&tw,&th);
    dc.DrawText(label[2],
		x0-tw,        y0);
    dc.GetTextExtent(label[0],&tw,&th);
    dc.DrawText(label[0],
		x0+cw/2-tw/2, y0-PXI_3_HEIGHT-th);
    dc.GetTextExtent(label[1],&tw,&th);
    dc.DrawText(label[1],
		x0+cw,        y0);
  }
}

