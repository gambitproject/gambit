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

/***************************** PLOT AXIS X ********************************/
void PlotAxis_X(wxDC& dc, float x_start,float x_end,float y_start,
		float y_end,int ch,int cw,int num_plots,int plot_type,
		unsigned int features,float log_step)
{
  int i1;
  float tempf;
  double p,k,x_per_grid;
  char axis_label_str[90];
  
  assert(num_plots==1 || num_plots==2);
  num_plots-=1;	       // convert from 1,2 to 0,1
  for (int i=0;i<=num_plots;i++) {
    if (features&DRAW_AXIS) {
      dc.DrawLine(XOFF,XOFF+(ch/2)*num_plots*i,XOFF,(ch-XOFF)-(ch/2)*num_plots*(1-i));
      dc.DrawLine(XOFF,(ch-XOFF)-(ch/2)*num_plots*(1-i),cw-XOFF,(ch-XOFF)-(ch/2)*num_plots*(1-i));
    }
    if (plot_type==DATA_TYPE_ARITH) {   	// arithmetic scale
      for (i1=0;i1<=XGRIDS;i1++) {
	if (features&DRAW_TICKS)
	  dc.DrawLine(
		      XOFF+i1*XGRID,      (ch-XOFF)-(ch/2)*num_plots*(1-i)-GRID_H, 
		      XOFF+i1*XGRID,      (ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
	if (features&DRAW_NUMS) {
	  tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
	  sprintf(axis_label_str,"% 3.2f",tempf);
	  wxCoord tw,th;
	  dc.GetTextExtent(axis_label_str,&tw,&th);
	  dc.DrawText(axis_label_str,
		      XOFF+i1*XGRID-tw/2,       (ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
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
		    XOFF+x,     (ch-XOFF)-(ch/2)*num_plots*(1-i)-GRID_H,
		    XOFF+x,     (ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
	sprintf(axis_label_str,"% 3.2f",p);
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    XOFF+x-tw/2,(ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
      }
#else
      for (i1=0;i1<=XGRIDS;i1++) {
	k=x_per_grid*(i1);
	p=x_start*pow(log_step,k);
	sprintf(axis_label_str,"% 3.2f",p);
	if (features&DRAW_TICKS)
	  dc.DrawLine(
		      XOFF+i1*XGRID,    (ch-XOFF)-(ch/2)*num_plots*(1-i)-GRID_H,
		      XOFF+i1*XGRID,    (ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
	if (features&DRAW_NUMS) {
	  wxCoord tw,th;
	  dc.GetTextExtent(axis_label_str,&tw,&th);
	  dc.DrawText(axis_label_str,
		      XOFF+i1*XGRID-tw/2,    (ch-XOFF)-(ch/2)*num_plots*(1-i)+GRID_H);
	}
      }
#endif
    }
    for (i1=0;i1<=YGRIDS;i1++) {
      if (features&DRAW_TICKS)
	dc.DrawLine(
		    XOFF-GRID_H,  XOFF+(ch/2)*num_plots*i+i1*YGRID,
		    XOFF+GRID_H,  XOFF+(ch/2)*num_plots*i+i1*YGRID);
      if (features&DRAW_NUMS) {
	tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
	sprintf(axis_label_str,"%3.2f",tempf);
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    XOFF-GRID_H-tw,  XOFF+(ch/2)*num_plots*i+i1*YGRID-th/2);
      }
    }
  }
}

/***************************** PLOT AXIS X ********************************/

void PlotAxis_2(wxDC& dc,float x_start,float x_end,float y_start,
		float y_end,int ch,int cw,unsigned int features)
{
  int i1;
  float tempf;
  char axis_label_str[90];
  int num_plots=0;
  int side=gmin(ch,cw);	// maintain square aspect ration that fits in this window
  ch=side;cw=side;
  
  if (features&DRAW_AXIS)
    if (features&DRAW_SQUARE)
      dc.DrawRectangle(XOFF,XOFF,side-2*XOFF,side-2*XOFF);
    else {
      dc.DrawLine(XOFF,XOFF,XOFF,side-XOFF);dc.DrawLine(XOFF,side-XOFF,side-XOFF,side-XOFF);}
  for (i1=0;i1<=XGRIDS;i1++) {
    if (features&DRAW_TICKS) {
      dc.DrawLine(
		  XOFF+i1*XGRID,  (side-XOFF)-GRID_H,
		  XOFF+i1*XGRID,  (side-XOFF)+GRID_H);
      if (features&DRAW_SQUARE)
	dc.DrawLine(
		    XOFF+i1*XGRID,  XOFF-GRID_H,
		    XOFF+i1*XGRID,  XOFF+GRID_H);
    }
    if (features&DRAW_NUMS) {
      tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  XOFF+i1*XGRID-tw/2,   (side-XOFF)+GRID_H);
      if (features&DRAW_SQUARE)
	dc.DrawText(axis_label_str,
		    XOFF+i1*XGRID-tw/2,XOFF-GRID_H-th);
    }
  }
  for (i1=0;i1<=YGRIDS;i1++) {
    if (features&DRAW_TICKS) {
      dc.DrawLine(
		  XOFF-GRID_H,   XOFF+i1*YGRID,
		  XOFF+GRID_H,   XOFF+i1*YGRID);
      if (features&DRAW_SQUARE)
	dc.DrawLine(
		    (side-XOFF)-GRID_H,  XOFF+i1*YGRID,
		    (side-XOFF)+GRID_H,  XOFF+i1*YGRID);
    }
    if (features&DRAW_NUMS) {
      tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  XOFF-GRID_H-tw,   XOFF+i1*YGRID-th/2);
      if (features&DRAW_SQUARE)
	dc.DrawText(axis_label_str,
		    (side-XOFF)+GRID_H, XOFF+i1*YGRID-th/2);
    }
  }
}

/******************************** PLOT AXIS 3 *****************************/
// Draws a triangular axis set for the pseudo-3D mode in PXI.  It can plot one or
// two triangles, corresponding to num_plots = 0,1 respectively.  Note that the
// axis scaling is not currently used.
void PlotAxis_3(wxDC& dc, int ch,int cw,int num_plots, unsigned int features, wxString label[])
{
  int i;
  float side;
  assert(num_plots==1 || num_plots==2);

  // Determine which dimention imposes the size constraint--the vert or horiz
  if ((cw/num_plots-2*XOFF)/2*TAN60<=(ch-2*XOFF))	// if the constraint is horizontal
    side=cw/num_plots-2*XOFF;			// two triangles must fit, with XOFF in between
  else
    side=((ch-2*XOFF)/TAN60)*2;
  
  for (i=0;i<num_plots;i++) {      	// draw the two triangles
    dc.DrawLine(
		XOFF+cw/2*i,        ch-XOFF,
		XOFF+side+cw/2*i,   ch-XOFF);               // bottom
    dc.DrawLine(
		XOFF+cw/2*i,        ch-XOFF,
		XOFF+side/2+cw/2*i, ch-XOFF-PXI_3_HEIGHT);  // left side
    dc.DrawLine(
		XOFF+side+cw/2*i,   ch-XOFF,
		XOFF+side/2+cw/2*i, ch-XOFF-PXI_3_HEIGHT);  // right side
    if (features&DRAW_AXIS) { 		// now draw the bisectors
      dc.DrawLine(XOFF+cw/2*i,ch-XOFF,XOFF+side*3/4+cw/2*i,ch-XOFF-PXI_3_HEIGHT/2);
      dc.DrawLine(XOFF+side+cw/2*i,ch-XOFF,XOFF+side/4+cw/2*i,ch-XOFF-PXI_3_HEIGHT/2);
      dc.DrawLine(XOFF+side/2+cw/2*i,ch-XOFF-PXI_3_HEIGHT,XOFF+side/2+cw/2*i,ch-XOFF);
    }
    if (features&DRAW_LABELS) {
      wxCoord tw,th;
      dc.GetTextExtent(label[2+3*i],&tw,&th);
      dc.DrawText(label[2+3*i],
		  XOFF+cw/2*i-tw, ch-XOFF);
      dc.GetTextExtent(label[0+3*i],&tw,&th);
      dc.DrawText(label[0+3*i],
		  XOFF+side/2+cw/2*i-tw/2, ch-XOFF-PXI_3_HEIGHT-th);
      dc.GetTextExtent(label[1+3*i],&tw,&th);
      dc.DrawText(label[1+3*i],
		  XOFF+side+cw/2*i,ch-XOFF);
    }
  }
}

