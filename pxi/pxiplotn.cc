//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot for plotting N values
//

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "pxiplotn.h"

#include "gmisc.h"
#include "wxmisc.h"
#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

#define NUM_COLORS	11
static char *equ_colors[NUM_COLORS+1] = {
  "BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
  "PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

void PxiPlotN::PlotAxis_X(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, int cw,int ch,
			  const PxiAxisProperties &p_horizProps, 
			  const PxiAxisProperties &p_vertProps,
			  float log_step)
{
  const int GRID_H = 5;
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


/******************************** CALC Y X************************************/
double PxiPlotN::CalcY_X(double y,int y0,int ch, const PlotInfo &thisplot)
{
  y = gmax(y, m_probAxisProp.m_scale.GetMinimum());
  y = gmin(y, m_probAxisProp.m_scale.GetMaximum());
  y -= m_probAxisProp.m_scale.GetMinimum(); // set DataMin to correspond to 0
  y /= (m_probAxisProp.m_scale.GetMaximum() - 
	m_probAxisProp.m_scale.GetMinimum()); // normalize to [0,1]
  y=y0-y*ch;	// scale to screen size
  return y;
}

/******************************** CALC X X************************************/

double PxiPlotN::CalcX_X(double x,int x0, int cw, const PlotInfo &thisplot)
{
  double min = m_lambdaAxisProp.m_scale.GetMinimum();
  double max = m_lambdaAxisProp.m_scale.GetMaximum();

  if (!m_lambdaAxisProp.m_scale.m_useLog) {
    x -= min;         // set MinX to correspond to 0
    x /= (max - min); // normalize to [0,1]
    x = x0 + x*cw;    // scale to screen size
  }
  else {
    double max_t = (log(max/min)/log(m_drawSettings.GetDelL()));
    double t=log(x/min)/log(m_drawSettings.GetDelL());
    x=x0+(t/max_t)*cw; // scale to screen size
  }
  return x;
}


// Draws a little # or a token corresponding to the point # in the experimental
// data overlay
void PxiPlotN::DrawExpPoint_X(wxDC &dc,const PlotInfo &thisplot,double cur_e,int iset,int st,int x0, int y0, int cw,int ch)
{
  try {
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;  
  point_nums = m_expData.HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s= m_expData[point_nums[i]];
    
    y=CalcY_X((*s).probs[iset][st],y0,ch,thisplot);
    x=CalcX_X(s->e,x0,cw,thisplot);
    dc.SetBrush(m_drawSettings.GetDataBrush());
    if (m_drawSettings.GetOverlaySym()==OVERLAY_TOKEN) {
      DrawToken(dc, (int) x, (int) y, st);
    }
    else {
      char tmp[10];
      sprintf(tmp,"%d",point_nums[i]);
      dc.SetFont(m_drawSettings.GetOverlayFont());
      dc.SetTextForeground(*wxBLACK);
      wxCoord tw,th;
      dc.GetTextExtent(tmp,&tw,&th);
      dc.DrawText(tmp, (int) (x-tw/2), (int) (y-th/2));
      //      dc.DrawText(tmp,x-3,y-6);
    }
    if (m_drawSettings.GetOverlayLines() && st!=1) {
      dc.SetBrush(*wxBLACK_BRUSH);
      int y1=(int)CalcY_X((*s).probs[iset][st-1],y0,ch,thisplot);
      dc.DrawLine((int) x, (int) y, (int) x, (int) y1);
    }
    delete s;
  }
  }
  catch (...) { }
}

void PxiPlotN::PlotData_X(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, 
			   int cw,int ch,const FileHeader &f_header,int level)
  /* This function plots n-dimensional data on a rectangular grid.  The x-axis
   * are error value
   */
{
  double x,y;
  int point_color=1;
  static int color_start;		// makes each overlay file be plotted a different color set
  if (level==1) color_start=0;
  int max_equ=0;
  int new_equ=0;

  EquTracker equs;		// init the EquTracker class
  
  if (true) {
    wxCoord tw,th;
    dc.SetFont(m_titleProp.m_font);
    dc.SetTextForeground(m_titleProp.m_color);
    dc.GetTextExtent(m_titleProp.m_title, &tw, &th);
    dc.DrawText(m_titleProp.m_title, x0+cw/2-tw/2, y0-ch-th);

    if (m_legendProp.m_showLegend) {
      int lev = 0;

      for (int st = 1; st <= f_header.NumStrategies(m_page); st++) {
	if (thisplot.GetStrategyShow(m_page, st)) {
	  wxString key = wxString::Format("Strategy %d", st);
	  dc.SetFont(m_legendProp.m_font);
	  dc.SetTextForeground(m_legendProp.m_color);
	  wxCoord tw,th;
	  dc.GetTextExtent(key,&tw,&th);
	  if (m_drawSettings.GetColorMode()==COLOR_PROB) 
	    dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(st+color_start)%NUM_COLORS+1],1,wxSOLID)));
	  dc.DrawLine(x0+cw-tw-cw/20,   y0-ch+3*th*lev/2+th/2,
		      x0+cw-tw,         y0-ch+3*th*lev/2+th/2);
	  dc.DrawText(key, x0+cw-tw, y0-ch+3*th*lev/2);
	  lev++;
	}
      }
    }
  }

  for (int i = 1; i <= f_header.GetData().Length(); i++) {
    const DataLine &probs = *f_header.GetData()[i];

    //---------------------------if cur_e is in active range ------------
    if (probs.Lambda() >= m_lambdaAxisProp.m_scale.GetMinimum() &&
	probs.Lambda() <= m_lambdaAxisProp.m_scale.GetMaximum()) {
      x=CalcX_X(probs.Lambda(),x0,cw,thisplot);
      // plot the infosets that are selected for the top graph
      // if we have experimental data, get it
#ifdef NOT_PORTED_YET
      if (m_drawSettings.GetColorMode()==COLOR_EQU)
	point_color=equs.Check_Equ(probs,&new_equ,prev_point);
#endif
      if (m_drawSettings.GetColorMode()==COLOR_NONE)
	point_color=2;
      if (point_color>max_equ) max_equ=point_color;
      // next line sets the correct color
      dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(point_color+color_start)%NUM_COLORS+1],1,wxSOLID)));
      
      for (int st = 1; st <= f_header.NumStrategies(m_page); st++) {
	// plot the data point
	y = CalcY_X(probs[m_page][st],y0, ch,thisplot);
	if (thisplot.GetStrategyShow(m_page,st)) {
	  if (m_drawSettings.GetColorMode()==COLOR_PROB) 
	    dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(st+color_start)%NUM_COLORS+1],1,wxSOLID)));
#ifdef NOT_PORTED_YET
	  if (m_drawSettings.ConnectDots() && !new_equ) {
	    dc.DrawLine((int) CalcX_X(prev_point->E(),x0, cw,thisplot),
			(int) CalcY_X((*prev_point)[m_page][st],y0,ch,thisplot),
			(int) x, (int) y);
	  }
	  else {
#endif
	    dc.DrawPoint((int) x, (int) y);
#ifdef NOT_PORTED_YET
	  }
#endif  
	  // if there is an experimental data point for this cur_e, plot it
	  //	  if (exp_data) {
	  DrawExpPoint_X(dc,thisplot,probs.Lambda(),m_page,st,x0,y0,ch,cw);
	  //	  }
	}
      }
    }
  }

  if (!m_drawSettings.RestartOverlayColors()) color_start+=max_equ;
}

void PxiPlotN::DoPlot(wxDC& dc, const PlotInfo &thisplot,
		      int x0, int y0, int cw,int ch, int level)
  // This function plots n-dimensional data on a rectangular grid.  The x-axis
  //  are error value
{
  PlotAxis_X(dc,thisplot,x0,y0,cw,ch, m_lambdaAxisProp, m_probAxisProp,
	     m_header.EStep());
  PlotData_X(dc,thisplot,x0,y0,cw,ch, m_header, 1);
}

void PxiPlotN::OnEvent(wxMouseEvent &ev)
{
  if (ev.ShiftDown() && ev.ButtonDown()) {  // use shift mouse click to add a label
    int w,h;
    GetClientSize(&w,&h);
    label_struct tmp_label;
    //    tmp_label.x=ev.x/w;tmp_label.y=ev.y/h;
    tmp_label.x=ev.GetX()/w;tmp_label.y=ev.GetY()/h;
    // Check if clicked on an already existing text
    int clicked_on=0;
    for (int i=1;i<=labels.Length();i++)
      if (labels[i].x+TEXT_MARGIN>tmp_label.x && labels[i].x-TEXT_MARGIN<tmp_label.x &&
	  labels[i].y+TEXT_MARGIN>tmp_label.y && labels[i].y-TEXT_MARGIN<tmp_label.y)
	clicked_on=i;
    const char * junk = (clicked_on) ? (const char *)labels[clicked_on].label : "";
    tmp_label.label=wxGetTextFromUser("Enter Label","Enter Label", (char *)junk);
    //					  (clicked_on) ? (char *)labels[clicked_on].label : "");
    if (!clicked_on)
      labels.Append(tmp_label);
    else {
      tmp_label.x=labels[clicked_on].x;
      tmp_label.y=labels[clicked_on].y;
      labels[clicked_on]=tmp_label;
    }
    Render();
  }
}

BEGIN_EVENT_TABLE(PxiPlotN, PxiPlot)
  EVT_MOUSE_EVENTS(PxiPlotN::OnEvent)
END_EVENT_TABLE()

PxiPlotN::PxiPlotN(wxWindow *p_parent, const wxPoint &p_position,
		   const wxSize &p_size,
		   const FileHeader &p_header, int p_page,
		   const ExpData &p_expData)
  : PxiPlot(p_parent, p_position, p_size, p_header, p_page, p_expData)
{ }


PxiPlotN::~PxiPlotN()
{ }

