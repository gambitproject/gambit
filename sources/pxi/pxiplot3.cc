//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot for plotting 3 values
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

#include "pxiplot3.h"

#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

#define NUM_COLORS	11
static char *equ_colors[NUM_COLORS+1]={"BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
				"PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

const double TAN60 = 1.732;      // tan(60) used as a constant to save time...
const double F2OR3 = 1.1547005;  // 2/sqrt(3)
const double F1OR3 = 0.5773503;  // 1/sqrt(3)


//*********************************** CALC X 3 *****************************
double PxiPlot3::CalcX_3(double p1,double p2,int x0, int y0, int cw,int ch)
{
  const double PXI_3_HEIGHT = cw*TAN60/2;
  double xx= x0+cw;
  double height = (m_probAxisProp.m_scale.GetMaximum() -
		   m_probAxisProp.m_scale.GetMinimum());
  p1 /= height; // normalize to [0,1]
  p2 /= height; // normalize to [0,1]
  
  return (xx-(F2OR3*p2+F1OR3*p1)*PXI_3_HEIGHT);
}

double PxiPlot3::CalcY_3(double p1,int x0, int y0, int cw,int ch)
{
  const double PXI_3_HEIGHT = cw*TAN60/2;
  return (y0-p1*PXI_3_HEIGHT);
}

//****************************** DRAW EXP POINT 3 ******************************
// Draws a little # or a cirlce corresponding to the point # in the experimental
// data overlay

void PxiPlot3::DrawExpPoint_3(wxDC &dc, 
			      double p_lambda, int iset, int st1, int st2,
			      int x0, int y0, int cw, int ch)
{
  try {
    gBlock<int> points(m_expData.FitPoints(p_lambda));
    for (int i = 1; i <= points.Length(); i++) {
      double y = CalcY_3(m_expData.GetDataProb(points[i], iset, st1),
			 x0, y0, cw, ch);
      double x = CalcX_3(m_expData.GetDataProb(points[i], iset, st1),
			 m_expData.GetDataProb(points[i], iset, st2),
			 x0, y0, ch, cw);
      if (m_overlayProp.m_token) {
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
      }
    }
  }
  catch (...) { }
}

/**************************** PLOT DATA 3 *********************************/
// Note Top->Left, Bottom->Right

void PxiPlot3::PlotData_3(wxDC& dc, int x0, int y0, int cw,int ch, 
			  const PxiFile &f_header, int level)
{
  double x,y;
  int iset;
  int point_color=1;                 // color of the pixel, corresponds to equilibrium #
  static int color_start;
  if (level==1) color_start=0;
  int max_equ=-1;
  //  int new_equ=0;

  EquTracker equs;                // init the EquTracker class
  
  if (true) {
    wxString title = wxString::Format("Plot %d", m_page);
    wxCoord tw,th;
    dc.GetTextExtent(title,&tw,&th);
    dc.DrawText(title, x0+cw/2-tw/2, y0-ch-th);
  }

  for (int i = 1; i <= f_header.GetData().Length(); i++) {
    const DataLine &probs = *f_header.GetData()[i];

    //------------------- if the cur_e is in range, display it -----------
    if (probs.Lambda() >= m_lambdaAxisProp.m_scale.GetMinimum() &&
	probs.Lambda() <= m_lambdaAxisProp.m_scale.GetMaximum()) {
      //      point_color=(m_drawSettings.GetColorMode()==COLOR_EQU) ? equs.Check_Equ(probs,&new_equ,prev_point) : 2;
      point_color = 2;
      if (point_color>max_equ) max_equ=point_color;
      wxPen *cpen=wxThePenList->FindOrCreatePen(equ_colors[(point_color+color_start)%NUM_COLORS+1],3,wxSOLID);
      //      if (dc.current_pen!=cpen) dc.SetPen(cpen);
      dc.SetPen(*cpen);

      int st1=0,st2=0;	// which two strategies to plot
      int i=1;
      while (!st1) {
	if (IsStrategyShown(m_page, i)) {
	  st1=i;
	}
	i++;
      }
      while (!st2) {
	if (IsStrategyShown(m_page, i)) {
	  st2=i;
	}
	i++;
      }
      x=CalcX_3(probs[m_page][st1],probs[m_page][st2],x0,y0,cw,ch);
      y=CalcY_3(probs[m_page][st1],x0,y0,cw,ch);

#ifdef NOT_PORTED_YET
      if (m_drawSettings.ConnectDots() && !new_equ) {
	double prev_x=CalcX_3((*prev_point)[iset][st1],(*prev_point)[iset][st2],x0,y0,cw,ch,thisplot);
	double prev_y=CalcY_3((*prev_point)[iset][st1],x0,y0,ch,cw);
	dc.DrawLine((int) prev_x, (int) prev_y, (int) x, (int) y);
      }
      else {
#endif  // NOT_PORTED_YET
	dc.DrawPoint((int) x, (int) y);
#ifdef NOT_PORTED_YET
      }
#endif  
      // if there is an experimental data point for this cur_e, plot it
      //     if (exp_data) {
	DrawExpPoint_3(dc,probs.Lambda(),iset,st1,st2,x0,y0,cw,ch);
	//      }
    }
  }

#ifdef UNUSED
  if (!m_drawSettings.RestartOverlayColors()) color_start+=max_equ;
#endif  // UNUSED
  PlotLabels(dc,ch,cw);
}

/******************************** PLOT AXIS 3 *****************************/
// Draws a triangular axis set for the pseudo-3D mode in PXI.  It can plot one or
// two triangles, corresponding to num_plots = 0,1 respectively.  Note that the
// axis scaling is not currently used.

void PxiPlot3::PlotAxis_3(wxDC& dc, int x0, int y0, int cw,int ch,
			  wxString label[])
{
  const double PXI_3_HEIGHT = cw*TAN60/2;
  dc.DrawLine(x0,        y0,
	      x0+cw,     y0);               // bottom
  dc.DrawLine(x0,        y0,
	      (int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT));  // left side
  dc.DrawLine(x0+cw,     y0,
	      (int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT));  // right side

  if (m_probAxisProp.m_display.m_shown) {
    dc.DrawLine(x0,          y0,
		(int) (x0+cw*3/4), (int) (y0-PXI_3_HEIGHT/2));
    dc.DrawLine(x0+cw,     y0,
		(int) (x0+cw/4), (int) (y0-PXI_3_HEIGHT/2));
    dc.DrawLine((int) (x0+cw/2), (int) (y0-PXI_3_HEIGHT),
		(int) (x0+cw/2),   y0);
  }

#ifdef NOT_PORTED_YET
  if (thisplot.ShowSquare()) {
#endif  // NOT_PORTED_YET
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
#ifdef NOT_PORTED_YET
  }
#endif  // NOT_PORTED_YET
}

void PxiPlot3::DoPlot(wxDC& dc,
		      int x0, int y0, int cw,int ch, int level)
{
  wxString labels[] = {"", "", ""};
  int st1=0,st2=0,st3=0;  // which two strategies to plot
  int j=1;
  while (!st1) {
    if (IsStrategyShown(m_page, j)) {
      st1=j;
    }
    j++;
  }
  while (!st2) {
    if (IsStrategyShown(m_page, j)) {
      st2=j;
    }
    j++;
  }

  st3=j;
  labels[0].Printf("%d",st1);
  labels[1].Printf("%d",st2); 
  labels[2].Printf("%d",st3);

  dc.SetPen(*wxBLACK_PEN);
  PlotAxis_3(dc,x0,y0,cw,ch,labels);
  PlotData_3(dc,x0,y0,cw,ch, m_header, 1);
}

void PxiPlot3::OnEvent(wxMouseEvent &ev)
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

BEGIN_EVENT_TABLE(PxiPlot3, wxScrolledWindow)
  EVT_MOUSE_EVENTS(PxiPlot3::OnEvent)
END_EVENT_TABLE()

PxiPlot3::PxiPlot3(wxWindow *p_parent, const wxPoint &p_position,
		   const wxSize &p_size,
		   const PxiFile &p_header, int p_page,
		   const ExpData &p_expData)
  : PxiPlot(p_parent, p_position, p_size, p_header, p_page, p_expData)
{ }

PxiPlot3::~PxiPlot3()
{ }
