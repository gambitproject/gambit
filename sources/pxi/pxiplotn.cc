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

#include "math/math.h"   // for gmax, gmin
#include "pxiplotn.h"

#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

#define NUM_COLORS	11
static char *equ_colors[NUM_COLORS+1] = {
  "BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
  "PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

void PxiPlotN::PlotAxis_X(wxDC& dc, int x0, int y0, int cw,int ch,
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
  
  if (p_horizProps.m_display.m_shown) {
    dc.DrawLine(x0, y0, x0 + cw, y0);
  }
  if (p_vertProps.m_display.m_shown) {
    dc.DrawLine(x0, y0, x0, y0-ch);
  }

  int xgrid = cw / p_horizProps.m_scale.m_divisions; 
  int ygrid = ch / p_vertProps.m_scale.m_divisions;

  if (!p_horizProps.m_scale.m_useLog)  {   // arithmetic scale
    for (i1=0; i1 <= p_horizProps.m_scale.m_divisions; i1++) {
      if (p_horizProps.m_display.m_ticks) {
	dc.DrawLine(x0+i1*xgrid,      y0-GRID_H, 
		    x0+i1*xgrid,      y0+GRID_H);
	if (p_horizProps.m_display.m_numbers) {
	  tempf=xStart+fabs((xStart-xEnd)/p_horizProps.m_scale.m_divisions)*i1;
	  sprintf(axis_label_str,"% 3.2f",tempf);
	  wxCoord tw,th;
	  dc.GetTextExtent(axis_label_str,&tw,&th);
	  dc.DrawText(axis_label_str,
		      x0+i1*xgrid-tw/2, y0+GRID_H);
	}
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
      if (p_horizProps.m_display.m_ticks) {
	dc.DrawLine(x0+i1*xgrid,    y0-GRID_H,
		    x0+i1*xgrid,    y0+GRID_H);
      }
      if (p_horizProps.m_display.m_numbers) {
	wxCoord tw,th;
	dc.GetTextExtent(axis_label_str,&tw,&th);
	dc.DrawText(axis_label_str,
		    x0+i1*xgrid-tw/2,y0+GRID_H);
      }
    }
#endif
  }
  for (i1 = 0; i1 <= p_vertProps.m_scale.m_divisions; i1++) {
    if (p_vertProps.m_display.m_ticks) {
      dc.DrawLine(x0-GRID_H,  y0-ch+i1*ygrid,
		  x0+GRID_H,  y0-ch+i1*ygrid);
    }
    if (p_vertProps.m_display.m_numbers) {
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
double PxiPlotN::CalcY_X(double y,int y0,int ch)
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

double PxiPlotN::CalcX_X(double x,int x0, int cw)
{
  double min = m_lambdaAxisProp.m_scale.GetMinimum();
  double max = m_lambdaAxisProp.m_scale.GetMaximum();
  double del = 1.05;    // FIXME: formerly read from file; make configurable

  if (!m_lambdaAxisProp.m_scale.m_useLog) {
    x -= min;         // set MinX to correspond to 0
    x /= (max - min); // normalize to [0,1]
    x = x0 + x*cw;    // scale to screen size
  }
  else {
    double max_t = log(max/min) / log(del);
    double t = log(x/min) / log(del);
    x=x0+(t/max_t)*cw; // scale to screen size
  }
  return x;
}


// Draws a little # or a token corresponding to the point # in the experimental
// data overlay
void PxiPlotN::DrawExpPoint_X(wxDC &dc, 
			      double p_lambda, int iset, int st,
			      int x0, int y0, int cw, int ch)
{
  try {
    gBlock<int> points(m_expData.FitPoints(p_lambda)); 
    for (int i = 1; i <= points.Length(); i++) {
      double y = CalcY_X(m_expData.GetDataPoint(points[i], iset, st),
			 y0, ch);
      double x = CalcX_X(m_expData.MLELambda(points[i]), x0, cw);
      // dc.SetBrush(m_drawSettings.GetDataBrush());
      dc.SetBrush(*wxBLACK_BRUSH);
      DrawToken(dc, (int) x, (int) y, points[i]);

      if (m_overlayProp.m_lines && st!=1) {
	dc.SetBrush(*wxBLACK_BRUSH);
	int y1 = (int) CalcY_X(m_expData.GetDataPoint(points[i], iset, st-1),
			       y0, ch);
	dc.DrawLine((int) x, (int) y, (int) x, (int) y1);
      }
    }
  }
  catch (...) { }
}

void PxiPlotN::PlotData_X(wxDC& dc, int x0, int y0, 
			  int cw,int ch,const FileHeader &f_header)
  /* This function plots n-dimensional data on a rectangular grid.  The x-axis
   * are error value
   */
{
  if (true) {
    wxCoord tw,th;
    dc.SetFont(m_titleProp.m_font);
    dc.SetTextForeground(m_titleProp.m_color);
    dc.GetTextExtent(m_titleProp.m_title, &tw, &th);
    dc.DrawText(m_titleProp.m_title, x0+cw/2-tw/2, y0-ch-th);

    if (m_legendProp.m_showLegend) {
      int lev = 0;

      for (int st = 1; st <= f_header.NumStrategies(m_page); st++) {
	if (IsStrategyShown(m_page, st)) {
	  wxString key = wxString::Format("Strategy %d", st);
	  dc.SetFont(m_legendProp.m_font);
	  dc.SetTextForeground(m_legendProp.m_color);
	  wxCoord tw,th;
	  dc.GetTextExtent(key,&tw,&th);
	  dc.SetPen(wxPen(equ_colors[(st-1)%NUM_COLORS+1], 1, wxSOLID));
	  dc.DrawLine(x0+cw-tw-cw/20,   y0-ch+3*th*lev/2+th/2,
		      x0+cw-tw,         y0-ch+3*th*lev/2+th/2);
	  dc.DrawText(key, x0+cw-tw, y0-ch+3*th*lev/2);
	  lev++;
	}
      }
    }
  }

  for (int st = 1; st <= f_header.NumStrategies(m_page); st++) {
    if (IsStrategyShown(m_page, st)) {
      int prevX = -1, prevY = -1;

      for (int i = 1; i <= f_header.GetData().Length(); i++) {
	const DataLine &probs = *f_header.GetData()[i];
	int x = (int) CalcX_X(probs.Lambda(), x0, cw);

	if (probs.Lambda() >= m_lambdaAxisProp.m_scale.GetMinimum() &&
	    probs.Lambda() <= m_lambdaAxisProp.m_scale.GetMaximum()) {
	  int y = (int) CalcY_X(probs[m_page][st],y0, ch);
	  
	  // Must set pen each time for now, since DrawExpPoint might
	  // reset it
	  dc.SetPen(wxPen(equ_colors[(st-1)%NUM_COLORS+1], 1, wxSOLID));
	  if (GetSeriesProperties().m_connectDots &&
	      prevX >= 0 && prevY >= 0) {
	    dc.DrawLine(prevX, prevY, x, y); 
	  }
	  dc.DrawPoint(x, y);
	    
	  DrawExpPoint_X(dc,probs.Lambda(), m_page, st, x0, y0, ch, cw);

	  prevX = x;
	  prevY = y;
	}
      }
    }
  }
}

void PxiPlotN::DoPlot(wxDC& dc,
		      int x0, int y0, int cw,int ch, int level)
  // This function plots n-dimensional data on a rectangular grid.  The x-axis
  //  are error value
{
  PlotAxis_X(dc,x0,y0,cw,ch, m_lambdaAxisProp, m_probAxisProp,
	     m_header.EStep());
  PlotData_X(dc,x0,y0,cw,ch, m_header);
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

