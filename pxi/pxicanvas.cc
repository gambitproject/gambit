//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plotting canvas window
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

#include "pxicanvas.h"

#include "gmisc.h"
#include "wxmisc.h"
#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"
#include "axis.h"

#define NUM_COLORS	11
char *equ_colors[NUM_COLORS+1]={"BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
				"PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

//***************************** PLOT LABELS ********************************
void PxiCanvas::PlotLabels(wxDC &dc, int ch,int cw)
{
  dc.SetTextForeground(*wxBLACK);
  //dc.SetBackgroundMode(wxTRANSPARENT);
  dc.SetFont(m_drawSettings.GetLabelFont());
  for (int i=1;i<=labels.Length();i++) {
    wxCoord tw,th;
    dc.GetTextExtent(labels[i].label,&tw,&th);
    dc.DrawText(labels[i].label,(int) (labels[i].x*cw-tw/2),
		(int) (labels[i].y*ch-th/2));
  }
}
/******************************** CALC Y X************************************/
double PxiCanvas::CalcY_X(double y,int y0,int ch, const PlotInfo &thisplot)
{
  y=gmax(y,thisplot.GetMinY());
  y=gmin(y,thisplot.GetMaxY());
  y-=thisplot.GetMinY();	// set DataMin to correspond to 0
  y/=(thisplot.GetMaxY()-thisplot.GetMinY());	// normalize to [0,1]
  y=y0-y*ch;	// scale to screen size
  return y;
}

/******************************** CALC X X************************************/

double PxiCanvas::CalcX_X(double x,int x0, int cw, const PlotInfo &thisplot)
{
  if (m_drawSettings.GetDataMode()==DATA_TYPE_ARITH) {
    x-=thisplot.GetMinX(); // set MinX to correspond to 0
    x/=(thisplot.GetMaxX()-thisplot.GetMinX()); // normalize to [0,1]
    x=x0+x*cw; // scale to screen size
  }
  else {
    double max_t=(log(thisplot.GetMaxX()/thisplot.GetMinX())/log(m_drawSettings.GetDelL()));
    double t=log(x/thisplot.GetMinX())/log(m_drawSettings.GetDelL());
    x=x0+(t/max_t)*cw; // scale to screen size
  }
  return x;
}

//********************************** DRAW TOKEN ******************************
// Draw a token.  If we are using COLOR_PROB mode, a different token is drawn for
// each strategy #.  Otherwise, we just draw an ellipse.  Note that each token is
// a 8x8 image.
#define		NUM_TOKENS		7
void PxiCanvas::DrawToken(wxDC &dc, int x, int y, int st)
{
  int ts=m_drawSettings.GetTokenSize();	// token dimensions are 2ts x 2ts
  if (m_drawSettings.GetColorMode()==COLOR_PROB) {
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    switch (st%NUM_TOKENS) {
    case 0: // x (cross diag)
      dc.DrawLine((int) (x-ts), (int) (y-ts), (int) (x+ts), (int) (y+ts));
      dc.DrawLine((int) (x-ts), (int) (y+ts), (int) (x+ts), (int) (y-ts));
      break;
    case 1: // + (cross rect)
      dc.DrawLine((int) (x-ts), y, (int) (x+ts), y);
      dc.DrawLine(x, (int) (y-ts), x, (int) (y+ts));
      break;
    case 2:	// circle
      dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
      break;
    case 3: // triangle upsidedown
      dc.DrawLine(x-ts,y-ts,x,y+ts);dc.DrawLine(x,y+ts,x+ts,y-ts);dc.DrawLine(x-ts,y-ts,x+ts,y-ts);break;
    case 4: // triangle rightsideup
      dc.DrawLine(x-ts,y+ts,x,y-ts);dc.DrawLine(x,y-ts,x+ts,y+ts);dc.DrawLine(x-ts,y+ts,x+ts,y+ts);break;
    case 5: // square
      dc.DrawLine(x-ts,y+ts,x-ts,y-ts);dc.DrawLine(x-ts,y-ts,x+ts,y-ts);dc.DrawLine(x+ts,y-ts,x+ts,y+ts);dc.DrawLine(x+ts,y+ts,x-ts,y+ts);break;
    case 6: // * (star)
      dc.DrawLine(x-ts,y-ts,x+ts,y+ts);dc.DrawLine(x-ts,y+ts,x+ts,y-ts);dc.DrawLine(x,y-ts,x,y+ts);break;
    default:
      assert(0);	// if we got here, something is very wrong. make sure # of cases=NUM_TOKENS
    }
  }
  else
    dc.DrawEllipse(x-ts,y-ts,2*ts,2*ts);
}

//****************************** DRAW EXP POINT ******************************
// Draws a little # or a token corresponding to the point # in the experimental
// data overlay
void PxiCanvas::DrawExpPoint_X(wxDC &dc,const PlotInfo &thisplot,double cur_e,int iset,int st,int x0, int y0, int cw,int ch)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
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

/**************************** PLOT DATA X **********************************/
void PxiCanvas::PlotData_X(wxDC& dc, const PlotInfo &thisplot, int x0, int y0, 
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
  
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(m_drawSettings.ConnectDots()) ? new DataLine : 0;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  f>>probs;

  if (m_drawSettings.GetColorMode()!=COLOR_EQU && prev_point) 
    (*prev_point)=probs;
  
  if (true) {
    wxString title;
    title.Printf("Plot %d",thisplot.GetPlotNumber());
    wxCoord tw,th;
    dc.GetTextExtent(title,&tw,&th);
    dc.DrawText(title, x0+cw/2-tw/2, y0-ch-th);
    int lev=0;

    for (int st=1; st <= f_header.NumStrategies(m_page); st++) {
      if (thisplot.GetStrategyShow(m_page, st)) {
	wxString key;
	key.Printf("Strategy %d",st);
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
  
  while (!probs.Done() && !f.eof()) {
    //---------------------------if cur_e is in active range ------------
    if (thisplot.RangeX(probs.E())) {
      x=CalcX_X(probs.E(),x0,cw,thisplot);
      // plot the infosets that are selected for the top graph
      // if we have experimental data, get it
      if (m_drawSettings.GetColorMode()==COLOR_EQU)
	point_color=equs.Check_Equ(probs,&new_equ,prev_point);
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
	  if (m_drawSettings.ConnectDots() && !new_equ) {
	    dc.DrawLine((int) CalcX_X(prev_point->E(),x0, cw,thisplot),
			(int) CalcY_X((*prev_point)[m_page][st],y0,ch,thisplot),
			(int) x, (int) y);
	  }
	  else {
	    dc.DrawPoint((int) x, (int) y);
	  }
	  // if there is an experimental data point for this cur_e, plot it
	  if (exp_data) {
	    DrawExpPoint_X(dc,thisplot,probs.E(),m_page,st,x0,y0,ch,cw);
	  }
	}
      }
    }
    // read in a line of strategies (for all infosets)
    if (m_drawSettings.GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
    f>>probs;
  }
  if (prev_point) delete prev_point;
  if (!m_drawSettings.RestartOverlayColors()) color_start+=max_equ;
}

/**************************** PLOT DATA 2 *********************************/
void PxiCanvas::DrawExpPoint_2(wxDC &dc, const PlotInfo &thisplot, double cur_e, 
			       int pl1,int st1,int pl2,int st2,
			       int x0, int y0, int cw, int ch)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    x=x0+(*s).probs[pl1][st1]*cw;
    y=y0-(*s).probs[pl2][st2]*ch;
    
    if (m_drawSettings.GetOverlaySym()==OVERLAY_TOKEN) {
      int ts=m_drawSettings.GetTokenSize();	// token dimentions are 2ts x 2ts
      dc.SetBrush(m_drawSettings.GetDataBrush());
      dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
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
    delete s;
  }
}

void PxiCanvas::PlotData_2(wxDC& dc,const PlotInfo &thisplot,int x0, int y0, int cw,int ch,
			     const FileHeader &f_header,int level)
{
  double x,y;
  int iset;
  int point_color=1; // color of the pixel, corresponds to equilibrium #
  static int color_start;		// makes each overlay file be plotted a different color set
  int max_equ=0;
  int new_equ=0;
  if (level==1) color_start=0;
  
  // where the actual data gets read in
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(m_drawSettings.ConnectDots()) ? new DataLine : NULL;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  
  EquTracker equs;		// init the EquTracker class
  f>>probs;
  // Figure out what strategies I am plotting.
  int pl1=0,st1=0,pl2=0,st2=0;
  for (int j=1;j<=f_header.NumInfosets();j++)
    for (int i=1;i<=f_header.NumStrategies(j);i++)
      if (thisplot.GetStrategyShow(j,i))
	if (pl1==0) {pl1=j;st1=i;} else {pl2=j;st2=i;}
  
  while (!probs.Done() && !f.eof()) {
    if (probs.E()<thisplot.GetMaxY() && probs.E()>thisplot.GetMinY()) {
      point_color=equs.Check_Equ(probs,&new_equ,prev_point);
      dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[point_color%NUM_COLORS+1],3,wxSOLID)));

      x=x0+probs[pl1][st1]*cw;
      y=y0-probs[pl2][st2]*ch;
      if (m_drawSettings.ConnectDots() && !new_equ) {
	double prev_x=x0+(*prev_point)[pl1][st1]*cw;
	double prev_y=y0-(*prev_point)[pl2][st2]*ch;
	dc.DrawLine((int) prev_x, (int) prev_y, (int) x, (int) y);
      }
      else {
	dc.DrawPoint((int) x, (int) y);
      }
      dc.DrawPoint((int) x, (int) y);
      // if there is an experimental data point for this cur_e, plot it
      if (exp_data) {
	DrawExpPoint_2(dc,thisplot,probs.E(),pl1,st1,pl2,st2, x0,y0,cw,ch);
      }
    }
    f>>probs;
  }
  if (prev_point) delete prev_point;
  PlotLabels(dc,ch,cw);
}

//*********************************** CALC X 3 *****************************
double PxiCanvas::CalcX_3(double p1,double p2,int x0, int y0, int cw,int ch, const PlotInfo &thisplot)
{
  double xx= x0+cw;
  p1/=(thisplot.GetMaxY()-thisplot.GetMinY()); // normalize to [0,1]
  p2/=(thisplot.GetMaxY()-thisplot.GetMinY()); // normalize to [0,1]
  
  return (xx-(F2OR3*p2+F1OR3*p1)*PXI_3_HEIGHT);
}

double PxiCanvas::CalcY_3(double p1,int x0, int y0, int cw,int ch)
{
  return (y0-p1*PXI_3_HEIGHT);
}

//****************************** DRAW EXP POINT 3 ******************************
// Draws a little # or a cirlce corresponding to the point # in the experimental
// data overlay

void PxiCanvas::DrawExpPoint_3(wxDC &dc,const PlotInfo &thisplot,double cur_e,int iset,int st1,int st2,
				 int x0, int y0, int cw,int ch)
{
  exp_data_struct *s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    y=CalcY_3((*s).probs[iset][st1],x0,y0,cw,ch);
    x=CalcX_3((*s).probs[iset][st1],(*s).probs[iset][st2],x0,y0,ch,cw,thisplot);
    if (m_drawSettings.GetOverlaySym()==OVERLAY_TOKEN) {
      int ts=m_drawSettings.GetTokenSize();	// token dimentions are 2ts x 2ts
      dc.SetBrush(m_drawSettings.GetDataBrush());
      dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
    }
    else {
      wxString tmp;
      tmp.Printf("%d",point_nums[i]);
      dc.SetFont(m_drawSettings.GetOverlayFont());
      dc.SetTextForeground(*wxBLACK);
      wxCoord tw,th;
      dc.GetTextExtent(tmp,&tw,&th);
      dc.DrawText(tmp, (int) (x-tw/2), (int) (y-th/2));
    }
    delete s;
  }
}

/**************************** PLOT DATA 3 *********************************/
// Note Top->Left, Bottom->Right

void PxiCanvas::PlotData_3(wxDC& dc,const PlotInfo &thisplot,int x0, int y0, int cw,int ch, 
			     const FileHeader &f_header, int level)
{
  double x,y;
  int iset;
  int point_color=1;                 // color of the pixel, corresponds to equilibrium #
  static int color_start;
  if (level==1) color_start=0;
  int max_equ=-1;
  int new_equ=0;

  EquTracker equs;                // init the EquTracker class
  
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(m_drawSettings.ConnectDots()) ? new DataLine : NULL;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  f>>probs;

  if (m_drawSettings.GetColorMode()!=COLOR_EQU && prev_point) 
    (*prev_point)=probs;
  
  if (true) {
    wxString title;
    title.Printf("Plot %d",thisplot.GetPlotNumber());
    wxCoord tw,th;
    dc.GetTextExtent(title,&tw,&th);
    dc.DrawText(title, x0+cw/2-tw/2, y0-ch-th);
  }

  while (!probs.Done() && !f.eof()) {
    //------------------- if the cur_e is in range, display it -----------
    if (thisplot.RangeX(probs.E())) {
      point_color=(m_drawSettings.GetColorMode()==COLOR_EQU) ? equs.Check_Equ(probs,&new_equ,prev_point) : 2;
      if (point_color>max_equ) max_equ=point_color;
      wxPen *cpen=wxThePenList->FindOrCreatePen(equ_colors[(point_color+color_start)%NUM_COLORS+1],3,wxSOLID);
      //      if (dc.current_pen!=cpen) dc.SetPen(cpen);
      dc.SetPen(*cpen);

      int st1=0,st2=0;	// which two strategies to plot
      int i=1;
      while (!st1) {
	if (thisplot.GetStrategyShow(m_page, i)) {
	  st1=i;
	}
	i++;
      }
      while (!st2) {
	if (thisplot.GetStrategyShow(m_page, i)) {
	  st2=i;
	}
	i++;
      }
      x=CalcX_3(probs[m_page][st1],probs[m_page][st2],x0,y0,cw,ch,thisplot);
      y=CalcY_3(probs[m_page][st1],x0,y0,cw,ch);

      if (m_drawSettings.ConnectDots() && !new_equ) {
	double prev_x=CalcX_3((*prev_point)[iset][st1],(*prev_point)[iset][st2],x0,y0,cw,ch,thisplot);
	double prev_y=CalcY_3((*prev_point)[iset][st1],x0,y0,ch,cw);
	dc.DrawLine((int) prev_x, (int) prev_y, (int) x, (int) y);
      }
      else {
	dc.DrawPoint((int) x, (int) y);
      }
      // if there is an experimental data point for this cur_e, plot it
      if (exp_data) {
	DrawExpPoint_3(dc,thisplot,probs.E(),iset,st1,st2,x0,y0,cw,ch);
      }
    }

    if (m_drawSettings.GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
    f>>probs;
  }
  if (prev_point) delete prev_point;
  if (!m_drawSettings.RestartOverlayColors()) color_start+=max_equ;
  PlotLabels(dc,ch,cw);
}

/******************************* UPDATE ***********************************/

void PxiCanvas::Update(wxDC& dc,int device)
{
  int cw,ch;
  const wxFont &font = m_drawSettings.GetAxisFont(); 
  wxBeginBusyCursor();
  GetClientSize(&cw,&ch);
  cw=850/2;ch=1100/2;

  if (device==PXI_UPDATE_SCREEN) {
    PrepareDC(dc);
    dc.SetUserScale(GetScale(), GetScale());
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
  }
  else {
#ifdef __WXMSW__
    // For printing, scale the picture
    if (device==PXI_UPDATE_PRINTER) {

      //      int cw,ch;
      //      canvas->GetClientSize(&cw,ch);
      float maxX = cw;
      float maxY = ch;
      
      // Let's have at least 50 device units margin
      float marginX = 50;
      float marginY = 50;
      
      maxX += (2*marginX);
      maxY += (2*marginY);
      
      // Get the size of the DC in pixels
      int w, h;
      dc.GetSize(&w, &h);
      
      // Calculate a suitable scaling factor
      float scaleX=(float)(w/maxX);
      float scaleY=(float)(h/maxY);
      float actualScale = wxMin(scaleX,scaleY);
    
      // Calculate the position on the DC for centring the graphic
      float posX = (float)((w - (cw*actualScale))/2.0);
      float posY = (float)((h - (ch*actualScale))/2.0);
      
      // Set the scale and origin
      dc.SetUserScale(actualScale, actualScale);
      dc.SetDeviceOrigin( (long)posX, (long)posY );
      
      /*
      frame->Draw(*dc);
      wxCoord width,height;
      dc.GetSize(&width,&height);
      cw=width*0.8;ch=height*0.8;
      dc.SetDeviceOrigin(width*0.1,height*0.1);
      */
    }
#endif
    // For metafile stuff, set transparent text backround
    if (device==PXI_UPDATE_METAFILE) {
      dc.SetBackground(*wxWHITE_BRUSH);
      dc.SetBackgroundMode(wxTRANSPARENT);
    }
  }

  dc.SetFont(font);
  dc.SetTextForeground(m_drawSettings.GetAxisTextColor());
  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(m_drawSettings.GetClearBrush());
  
  const PlotInfo &thisplot(m_drawSettings.GetPlotInfo());
  dc.SetPen(*wxBLACK_PEN);

  // used for square aspect ratio that fits in window
  int side=gmin(cw-2*XOFF, ch-2*XOFF);	

  switch(thisplot.GetPlotMode()) { 
  case PXI_PLOT_X:
    DoPlot_X(dc,thisplot,XOFF,ch-XOFF,cw-2*XOFF, ch-2*XOFF, 1);
    break;
  case PXI_PLOT_2:
    DoPlot_2(dc,thisplot,XOFF,ch-XOFF, side, side, 1);
    break;
  case PXI_PLOT_3:
    DoPlot_3(dc,thisplot,XOFF,ch-XOFF, side, side, 1);
  }
  wxEndBusyCursor();
}


void PxiCanvas::DoPlot_X(wxDC& dc, const PlotInfo &thisplot,
			 int x0, int y0, int cw,int ch, int level)
  // This function plots n-dimensional data on a rectangular grid.  The x-axis
  //  are error value
{
  PlotAxis_X(dc,thisplot,x0,y0,cw,ch,m_drawSettings.GetDataMode(),
	     m_header.EStep());
  PlotData_X(dc,thisplot,x0,y0,cw,ch, m_header, 1);
}


void PxiCanvas::DoPlot_2(wxDC& dc, const PlotInfo &thisplot,
			 int x0, int y0, int cw,int ch, int level)
{
  PlotAxis_2(dc,thisplot,x0,y0,cw,ch);
  PlotData_2(dc,thisplot,x0,y0,cw,ch, m_header, 1);
}

void PxiCanvas::DoPlot_3(wxDC& dc, const PlotInfo &thisplot,
			 int x0, int y0, int cw,int ch, int level)
{
  wxString labels[] = {"", "", ""};
  int st1=0,st2=0,st3=0;  // which two strategies to plot
  int j=1;
  while (!st1) {
    if (thisplot.GetStrategyShow(m_page,j)) {
      st1=j;
    }
    j++;
  }
  while (!st2) {
    if (thisplot.GetStrategyShow(m_page,j)) {
      st2=j;
    }
    j++;
  }

  st3=j;
  labels[0].Printf("%d",st1);
  labels[1].Printf("%d",st2); 
  labels[2].Printf("%d",st3);

  dc.SetPen(*wxBLACK_PEN);
  PlotAxis_3(dc,thisplot,x0,y0,cw,ch,labels);
  PlotData_3(dc,thisplot,x0,y0,cw,ch, m_header, 1);
}

void PxiCanvas::ShowDetail(void)
{
  char tempstr[200];
  wxString message;
  int		i1,i;
  sprintf(tempstr, "Detail for: %s\n",
	  (const char *) FileNameFromPath(m_header.FileName()));
  message+=tempstr;
  sprintf(tempstr,"Error (lambda) step:  %4.4f\n",m_header.EStep());
  message+=tempstr;
  sprintf(tempstr,"Error (lambda) start: %4.4f\n",m_header.EStart());
  message+=tempstr;
  sprintf(tempstr,"Error (lambda) stop : %4.4f\n",m_header.EStop());
  message+=tempstr;
  sprintf(tempstr,"Minimum data value  : %4.4f\n",m_header.DataMin());
  message+=tempstr;
  sprintf(tempstr,"Maximum data value  : %4.4f\n",m_header.DataMax());
  message+=tempstr;
  sprintf(tempstr,"Data type:  %s\n",(m_header.DataType()==DATA_TYPE_ARITH) ? "Arithmetic" : "Logarithmic");
  message+=tempstr;
  message+="\n";
  if (m_header.MError()>-.99) {
    sprintf(tempstr,"Probability step :    %4.4f\n",m_header.QStep());
    message+=tempstr;
    sprintf(tempstr,"Margin of error:      %4.4f\n",m_header.MError());
    message+=tempstr;
  }
  wxMessageBox(message,"File Details",wxOK);
}

void PxiCanvas::NewExpData(ExpDataParams &P) 
{ 
  if(exp_data) delete exp_data;
  exp_data = NULL;
  exp_data = new ExpData(P); 
}

void PxiCanvas::OnChar(wxKeyEvent &ev)
{
  switch(ev.KeyCode()) {
  case WXK_PRIOR:
    SetPreviousPage();
    Render();
    break;
  case WXK_NEXT:
    SetNextPage();
    Render();
    break;
  default:
    wxScrolledWindow::OnChar(ev);
    break;
  }
}

void PxiCanvas::OnEvent(wxMouseEvent &ev)
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

BEGIN_EVENT_TABLE(PxiCanvas, wxScrolledWindow)
  EVT_PAINT(PxiCanvas::OnPaint)
END_EVENT_TABLE()

PxiCanvas::PxiCanvas(wxWindow *p_parent, const wxPoint &p_position,
		     const wxSize &p_size,
		     const FileHeader &p_header, int p_page)
  : wxScrolledWindow(p_parent, -1, p_position, p_size),
    m_header(p_header), m_drawSettings(m_header, p_page),
    exp_data(NULL), probs(p_header.FileName()),
    m_landscape(false), m_width(850/2), m_height(1100/2), m_scale(1.0), 
    m_ppu(25),
    m_dc(new wxMemoryDC), m_page(p_page)
{
  // fit to 8 1/2 x 11 inch  
  SetScale(1.0);

  m_dc->SelectObject(wxBitmap(2000, 2000));
  Update(*m_dc, PXI_UPDATE_SCREEN);

  Show(true);
}

PxiCanvas::~PxiCanvas()
{
  delete m_dc;
}

void PxiCanvas::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  dc.Blit(0, 0, GetSize().GetWidth(), GetSize().GetHeight(),
	  m_dc, 0, 0);
}

void PxiCanvas::Render(void)
{
  m_dc->Clear();
  m_dc->BeginDrawing();
  Update(*m_dc, PXI_UPDATE_SCREEN);
  m_dc->EndDrawing();
}

void PxiCanvas::SetScale(double x) 
{
  m_scale = x; 
  SetScrollbars((int) m_ppu, (int) m_ppu,
		(int) (GetScale()*Width()/m_ppu),
		(int) (GetScale()*Height()/m_ppu));
}

