//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot for plotting 2 values
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

#include "pxiplot2.h"

#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

void PxiPlot2::DrawExpPoint_2(wxDC &dc, 
			      double p_lambda,
			      int pl1,int st1,int pl2,int st2,
			      int x0, int y0, int cw, int ch)
{
  try {
    gBlock<int> points(m_expData.FitPoints(p_lambda));
    for (int i = 1; i <= points.Length(); i++) {
      double x = x0 + m_expData.GetDataProb(points[i], pl1, st1) * cw;
      double y = y0 - m_expData.GetDataProb(points[i], pl2, st2) * ch;
    
      if (m_overlayProp.m_token)  {
	int ts = m_overlayProp.m_tokenSize;
	//	dc.SetBrush(m_drawSettings.GetDataBrush());
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
      }
      else {
	wxString tmp = wxString::Format("%d", points[i]);
	dc.SetFont(m_overlayProp.m_font);
	dc.SetTextForeground(*wxBLACK);
	wxCoord tw,th;
	dc.GetTextExtent(tmp,&tw,&th);
	dc.DrawText(tmp, (int) (x-tw/2), (int) (y-th/2));
	//      dc.DrawText(tmp,x-3,y-6);
      }
    }
  }
  catch (...) { }
}

void PxiPlot2::PlotData_2(wxDC& dc, int x0, int y0, int cw,int ch,
			  const PxiFile &p_file, int level)
{
  int pl1 = 1, st1 = 1, pl2 = 2, st2 = 1;

  for (int i = 1; i <= p_file.GetData().Length(); i++) {
    DataLine probs = *p_file.GetData()[i];

    /*
    if (probs.Lambda() < m_probAxisProp.m_scale.GetMaximum() && 
	probs.Lambda() > m_probAxisProp.m_scale.GetMinimum()) {
    */
      dc.SetPen(wxPen(wxColour("BLUE"), 1, wxSOLID));

      double x = x0 + probs[pl1][st1] * cw;
      double y = y0 - probs[pl2][st2] * ch;
      dc.DrawPoint((int) x, (int) y);
      // if there is an experimental data point for this cur_e, plot it
      DrawExpPoint_2(dc,probs.Lambda(),pl1,st1,pl2,st2, x0,y0,cw,ch);
      /*
    }
      */
  }

  PlotLabels(dc,ch,cw);
}

void PxiPlot2::PlotAxis_2(wxDC& dc,
			  int x0, int y0, int cw,int ch)
{
  int i1;
  float tempf;
  char axis_label_str[90];
  double xStart = m_probAxisProp.m_scale.GetMinimum();
  double xEnd = m_probAxisProp.m_scale.GetMaximum();
  double yStart = m_probAxisProp.m_scale.GetMinimum();
  double yEnd = m_probAxisProp.m_scale.GetMaximum();

  // temporary replacement of old constant; should be made configurable
  const int GRID_H = 5;
  int xgrid = cw / m_probAxisProp.m_scale.m_divisions; 
  int ygrid = ch / m_probAxisProp.m_scale.m_divisions;

  if (m_probAxisProp.m_display.m_shown) {
    dc.DrawLine(x0, y0, x0, y0-ch);
    dc.DrawLine(x0, y0, x0+cw, y0);
  }

  /*
  if (thisplot.ShowSquare()) {
  */
  if (true) {
    dc.DrawLine(x0+cw, y0-ch, x0, y0-ch);
    dc.DrawLine(x0+cw, y0-ch, x0+cw, y0);
  }

  for (i1 = 0; i1 <= m_probAxisProp.m_scale.m_divisions; i1++) {
    if (m_probAxisProp.m_display.m_ticks) {
      dc.DrawLine(x0+i1*xgrid, y0-GRID_H, x0+i1*xgrid, y0+GRID_H);
      /*
      if (thisplot.ShowSquare())
      */
      if (true) {
	dc.DrawLine(x0+i1*xgrid, y0-ch-GRID_H, x0+i1*xgrid,  y0-ch+GRID_H);
      }
    }

    if (m_probAxisProp.m_display.m_numbers) {
      tempf = xStart + fabs((xStart - xEnd) / m_probAxisProp.m_scale.m_divisions)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+i1*xgrid-tw/2, y0+GRID_H);
      /*
      if (thisplot.ShowSquare())
      */
      if (true) {
	dc.DrawText(axis_label_str,
		    x0+i1*xgrid-tw/2,y0-ch-GRID_H-th);
      }
    }
  }
  for (i1 = 0; i1 <= m_probAxisProp.m_scale.m_divisions; i1++) {
    if (m_probAxisProp.m_display.m_ticks) {
      dc.DrawLine(x0-GRID_H,   y0-ch+i1*ygrid,
		  x0+GRID_H,   y0-ch+i1*ygrid);
      /*
      if (thisplot.ShowSquare()) 
      */
      if (true) {
	dc.DrawLine(x0+cw-GRID_H,  y0-ch+i1*ygrid,
		    x0+cw+GRID_H,  y0-ch+i1*ygrid);
      }
    }

    if (m_probAxisProp.m_display.m_numbers) {
      tempf=yEnd-fabs((yStart-yEnd)/m_probAxisProp.m_scale.m_divisions)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0-GRID_H-tw,   y0-ch+i1*ygrid-th/2);
      /*
      if (thisplot.ShowSquare())
      */
      if (true) {
	dc.DrawText(axis_label_str,
		    x0+cw+GRID_H, y0-ch+i1*ygrid-th/2);
      }
    }
  }
}

void PxiPlot2::DoPlot(wxDC& dc, int x0, int y0, int cw,int ch, int level)
{
  PlotAxis_2(dc, x0, y0, cw, ch);
  for (int i = 1; i <= m_qreFiles.Length(); i++) {
    if (m_qreFiles[i]->IsShown()) {
      PlotData_2(dc, x0, y0, cw, ch, *(m_qreFiles[i]), 1);
    }
  }
}

void PxiPlot2::OnEvent(wxMouseEvent &ev)
{
#ifdef UNUSED
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
#endif  // UNUSED
}

BEGIN_EVENT_TABLE(PxiPlot2, PxiPlot)
  EVT_MOUSE_EVENTS(PxiPlot2::OnEvent)
END_EVENT_TABLE()

PxiPlot2::PxiPlot2(wxWindow *p_parent, const wxPoint &p_position,
		   const wxSize &p_size,
		   const gArray<PxiFile *> &p_qreFiles, int p_page,
		   const ExpData &p_expData)
  : PxiPlot(p_parent, p_position, p_size, p_qreFiles, p_page, p_expData)
{ }

PxiPlot2::~PxiPlot2()
{ }

