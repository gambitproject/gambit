#include "wx/wx.h"
#include "wx/dc.h"

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma hdrstop
#include "gmisc.h"
#include "wxmisc.h"
#include "arrays.h"
#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"
#include "axis.h"

extern wxBrush		*exp_data_brush;
#define NUM_COLORS	11
char *equ_colors[NUM_COLORS+1]={"BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
				"PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

//***************************** PLOT LABELS ********************************
void PxiCanvas::PlotLabels(wxDC &dc, int ch,int cw)
{
  dc.SetTextForeground(*wxBLACK);
  //dc.SetBackgroundMode(wxTRANSPARENT);
  dc.SetFont(*(draw_settings->GetLabelFont()));
  for (int i=1;i<=labels.Length();i++)
    dc.DrawText(labels[i].label,labels[i].x*cw,labels[i].y*ch);
}
/******************************** CALC Y X************************************/
double PxiCanvas::CalcY_X(double y,int ch,int plot)
{
  double y0;
  y0=(plot==TOP_PLOT) ? (ch-XOFF)-(ch/2)*(draw_settings->GetNumPlots()-1) : (ch-XOFF);
  y=gmax(y,draw_settings->GetDataMin());
  y=gmin(y,draw_settings->GetDataMax());
  y-=draw_settings->GetDataMin();	// set DataMin to correspond to 0
  y/=(draw_settings->GetDataMax()-draw_settings->GetDataMin());	// normalize to [0,1]
  y=y0-y*(ch-ch/2*(draw_settings->GetNumPlots()-1)-2*XOFF);	// scale to screen size
  return y;
}
/******************************** CALC X X************************************/

double PxiCanvas::CalcX_X(double x,int cw)
{
  if (draw_settings->GetDataMode()==DATA_TYPE_ARITH) {
    x-=draw_settings->GetStopMin(); // set StopMin to correspond to 0
    x/=(draw_settings->GetStopMax()-draw_settings->GetStopMin()); // normalize to [0,1]
    x=XOFF+x*(cw-2*XOFF); // scale to screen size
  }
  else {
    double max_t=(log(draw_settings->GetStopMax()/draw_settings->GetStopMin())/log(draw_settings->GetDelL()));
    double t=log(x/draw_settings->GetStopMin())/log(draw_settings->GetDelL());
    x=XOFF+t/max_t*(cw-2*XOFF); // scale to screen size
  }
  return x;
}

//********************************** DRAW TOKEN ******************************
// Draw a token.  If we are using COLOR_PROB mode, a different token is drawn for
// each strategy #.  Otherwise, we just draw an ellipse.  Note that each token is
// a 8x8 image.
#define		NUM_TOKENS		7
void PxiCanvas::DrawToken(wxDC &dc,double x,double y,int st)
{
  int ts=draw_settings->GetTokenSize();	// token dimentions are 2ts x 2ts
  if (draw_settings->GetColorMode()==COLOR_PROB) {
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    switch (st%NUM_TOKENS) {
    case 0: // x (cross diag)
      dc.DrawLine(x-ts,y-ts,x+ts,y+ts);dc.DrawLine(x-ts,y+ts,x+ts,y-ts);break;
    case 1: // + (cross rect)
      dc.DrawLine(x-ts,y,x+ts,y);dc.DrawLine(x,y-ts,x,y+ts);break;
    case 2:	// circle
      dc.DrawEllipse(x-ts,y-ts,2*ts,2*ts);break;
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
void PxiCanvas::DrawExpPoint_X(wxDC &dc,double cur_e,int iset,int st,int ch,int cw,int plot)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    y=CalcY_X((*s).probs[iset][st],ch,plot);
    x=CalcX_X(s->e,cw);
    dc.SetBrush(*exp_data_brush);
    if (draw_settings->GetOverlaySym()==OVERLAY_TOKEN)
      DrawToken(dc,x,y,st);
    else {
      char tmp[10];
      sprintf(tmp,"%d",point_nums[i]);
      dc.SetFont(*(draw_settings->GetOverlayFont()));
      dc.SetTextForeground(*wxBLACK);
      dc.DrawText(tmp,x-3,y-6);
    }
    if (draw_settings->GetOverlayLines() && st!=1) {
      dc.SetBrush(*wxBLACK_BRUSH);
      int y1=(int)CalcY_X((*s).probs[iset][st-1],ch,plot);
      dc.DrawLine(x,y,x,y1);
    }
    delete s;
  }
}

/**************************** PLOT DATA X **********************************/
void PxiCanvas::PlotData_X(wxDC& dc,int ch,int cw,const FileHeader &f_header,int level=1)
  /* This function plots n-dimensional data on a rectangular grid.  The x-axis
   * are error value
   */
{
  int	st;
  double	x,y;
  int	iset;
  int	point_color;
  static int color_start;		// makes each overlay file be plotted a different color set
  int max_equ=0;
  int new_equ=0;
  if (level==1) color_start=0;
  
  EquTracker equs;		// init the EquTracker class
  
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(draw_settings->ConnectDots()) ? new DataLine : 0;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  f>>probs;
  if (draw_settings->GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
  
  while (!probs.Done() && f.IsValid()) {
    //---------------------------if cur_e is in active range ------------
    if (draw_settings->RangeX(probs.E())) {
      x=CalcX_X(probs.E(),cw);
      // plot the infosets that are selected for the top graph
      // if we have experimental data, get it
      if (draw_settings->GetColorMode()==COLOR_EQU)
	point_color=equs.Check_Equ(probs,&new_equ,prev_point);
      if (draw_settings->GetColorMode()==COLOR_NONE)
	point_color=2;
      if (point_color>max_equ) max_equ=point_color;
      // next line sets the correct color
      dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(point_color+color_start)%NUM_COLORS+1],1,wxSOLID)));
      for (int top=1;top<=draw_settings->GetPlotTop();top++) {
	iset=draw_settings->GetPlotTop(top);
	for (st=1;st<=f_header.NumStrategies(iset);st++) {
				// plot the data point
	  y=CalcY_X(probs[iset][st],ch,TOP_PLOT);
	  if (draw_settings->GetStrategyShow(iset,st)/* && draw_settings->RangeY(probs[iset][st])*/) {
	    if (draw_settings->GetColorMode()==COLOR_PROB)
	      dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(st+color_start)%NUM_COLORS+1],1,wxSOLID)));
	    if (draw_settings->ConnectDots() && !new_equ)
	      dc.DrawLine(CalcX_X(prev_point->E(),cw),CalcY_X((*prev_point)[iset][st],ch,TOP_PLOT),x,y);
	    else
	      dc.DrawPoint(x,y);
	    // if there is an experimental data point for this cur_e, plot it
	    if (exp_data)	DrawExpPoint_X(dc,probs.E(),iset,st,ch,cw,TOP_PLOT);
	  }
	}
      }
      
      if (draw_settings->GetNumPlots()==2) {
	for (int bottom=1;bottom<=draw_settings->GetPlotBottom();bottom++) {
	  iset=draw_settings->GetPlotBottom(bottom);
	  for (st=1;st<=f_header.NumStrategies(iset);st++) {
	    y=CalcY_X(probs[iset][st],ch,BOTTOM_PLOT);
	    if (draw_settings->GetStrategyShow(iset,st) /* && draw_settings->RangeY(probs[iset][st])*/) {
	    if (draw_settings->GetColorMode()==COLOR_PROB)
	    dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[(st+color_start)%NUM_COLORS+1],1,wxSOLID)));
	    if (draw_settings->ConnectDots() && !new_equ)
	    dc.DrawLine(CalcX_X(prev_point->E(),cw),CalcY_X((*prev_point)[iset][st],ch,BOTTOM_PLOT),x,y);
	    else
	    dc.DrawPoint(x,y);
	    // if there is an experimental data point for this cur_e, plot it
	    if (exp_data)	DrawExpPoint_X(dc,probs.E(),iset,st,ch,cw,BOTTOM_PLOT);
	    }
	  }
	}
      }
    }
    // read in a line of strategies (for all infosets)
    if (draw_settings->GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
    f>>probs;
  }
  if (prev_point) delete prev_point;
  if (!draw_settings->RestartOverlayColors()) color_start+=max_equ;
  PlotLabels(dc,ch,cw);
}

/**************************** PLOT DATA 2 *********************************/
void PxiCanvas::DrawExpPoint_2(wxDC &dc,double cur_e,int pl1,int st1,int pl2,int st2,int side)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    x=XOFF+(*s).probs[pl1][st1]*(side-2*XOFF);
    y=(side-XOFF)-(*s).probs[pl2][st2]*(side-2*XOFF);
    
    if (draw_settings->GetOverlaySym()==OVERLAY_TOKEN) {
      int ts=draw_settings->GetTokenSize();	// token dimentions are 2ts x 2ts
      dc.SetBrush(*exp_data_brush);
      dc.DrawEllipse(x-ts,y-ts,2*ts,2*ts);
    }
    else {
      char tmp[10];
      sprintf(tmp,"%d",point_nums[i]);
      dc.SetFont(*(draw_settings->GetOverlayFont()));
      dc.SetTextForeground(*wxBLACK);
      dc.DrawText(tmp,x-3,y-6);
    }
    delete s;
  }
}

void PxiCanvas::PlotData_2(wxDC& dc,int ch,int cw,const FileHeader &f_header)
{
  double	x,y;
  int	iset;
  int		point_color;			// color of the pixel, corresponds to equilibrium #
  int new_equ=0;
  int side=gmin(ch,cw);	// maintain square aspect ration that fits in this window
  
  // where the actual data gets read in
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(draw_settings->ConnectDots()) ? new DataLine : NULL;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  
  EquTracker equs;		// init the EquTracker class
  f>>probs;
  // Figure out what strategies I am plotting.
  int pl1=0,st1=0,pl2=0,st2=0;
  for (int j=1;j<=f_header.NumInfosets();j++)
    for (int i=1;i<=f_header.NumStrategies(j);i++)
      if (draw_settings->GetStrategyShow(j,i))
	if (pl1==0) {pl1=j;st1=i;} else {pl2=j;st2=i;}
  
  while (!probs.Done() && f.IsValid())
    {
      //------------------- if the cur_e is in range, display it -----------
      if (probs.E()<draw_settings->GetStopMax() && probs.E()>draw_settings->GetStopMin()) {
	iset=draw_settings->GetPlotTop(1);
	point_color=equs.Check_Equ(probs,&new_equ,prev_point);
	dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[point_color%NUM_COLORS+1],3,wxSOLID)));
	/*------------------ ISET #1: plot the point in color ----------------------*/
	x=XOFF+probs[pl1][st1]*(side-2*XOFF);
	y=(side-XOFF)-probs[pl2][st2]*(side-2*XOFF);
	if (draw_settings->ConnectDots() && !new_equ) {
	  double prev_x=XOFF+(*prev_point)[pl1][st1]*(side-2*XOFF);
	  double prev_y=(side-XOFF)-(*prev_point)[pl2][st2]*(side-2*XOFF);
	  dc.DrawLine(prev_x,prev_y,x,y);
	}
	else
	  dc.DrawPoint(x,y);
	dc.DrawPoint(x,y);
	// if there is an experimental data point for this cur_e, plot it
	if (exp_data) DrawExpPoint_2(dc,probs.E(),pl1,st1,pl2,st2,side);
      }
      f>>probs;
    }
  if (prev_point) delete prev_point;
  PlotLabels(dc,ch,cw);
}

//*********************************** CALC X 3 *****************************
double PxiCanvas::CalcX_3(double p1,double p2,int ch,int cw,int plot)
{
  double side;
  // Determine which dimension imposes the size constraint--the vert or horiz
  // Note: the bit of trig here is self explanatory...
  if ((cw/draw_settings->GetNumPlots()-2*XOFF)/2*TAN60<=(ch-2*XOFF))	// if the constraint is horizontal
    side=cw/draw_settings->GetNumPlots()-2*XOFF;			// two triangles must fit, with XOFF in between
  else
    side=((ch-2*XOFF)/TAN60)*2;
  double x0=(plot==TOP_PLOT) ? XOFF+side : XOFF+side+cw/draw_settings->GetNumPlots();
  p1/=(draw_settings->GetDataMax()-draw_settings->GetDataMin()); // normalize to [0,1]
  p2/=(draw_settings->GetDataMax()-draw_settings->GetDataMin()); // normalize to [0,1]
  
  return (x0-(F2OR3*p2+F1OR3*p1)*PXI_3_HEIGHT);
}

double PxiCanvas::CalcY_3(double p1,int ch,int cw)
{
  double side;
  if ((cw/draw_settings->GetNumPlots()-2*XOFF)/2*TAN60<=(ch-2*XOFF))	// if the constraint is horizontal
    side=cw/draw_settings->GetNumPlots()-2*XOFF;			// two triangles must fit, with XOFF in between
  else
    side=((ch-2*XOFF)/TAN60)*2;
  
  return (ch-XOFF-p1*PXI_3_HEIGHT);
}


//****************************** DRAW EXP POINT 3 ******************************
// Draws a little # or a cirlce corresponding to the point # in the experimental
// data overlay
void PxiCanvas::DrawExpPoint_3(wxDC &dc,double cur_e,int iset,int st1,int st2,int ch,int cw,int plot)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    y=CalcY_3((*s).probs[iset][st1],ch,cw);
    x=CalcX_3((*s).probs[iset][st1],(*s).probs[iset][st2],ch,cw,plot);
    if (draw_settings->GetOverlaySym()==OVERLAY_TOKEN) {
      int ts=draw_settings->GetTokenSize();	// token dimentions are 2ts x 2ts
      dc.SetBrush(*exp_data_brush);
      dc.DrawEllipse(x-ts,y-ts,2*ts,2*ts);
    }
    else {
      char tmp[10];
      sprintf(tmp,"%d",point_nums[i]);
      dc.SetFont(*(draw_settings->GetOverlayFont()));
      dc.SetTextForeground(*wxBLACK);
      dc.DrawText(tmp,x-3,y-6);
    }
    delete s;
  }
}

/**************************** PLOT DATA 3 *********************************/
// Note Top->Left, Bottom->Right
void PxiCanvas::PlotData_3(wxDC& dc,int ch,int cw,const FileHeader &f_header,int level=1)
{
  static int color_start;
  if (level==1) color_start=0;
  int max_equ=-1;
  double	x,y;
  int	iset;
  int new_equ=0;
  // where the actual data gets read in
  
  int		point_color;			// color of the pixel, corresponds to equilibrium #
  
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(draw_settings->ConnectDots()) ? new DataLine : NULL;
  int ok=FindStringInFile(f,"Data:");assert(ok);
  
  EquTracker equs;		// init the EquTracker class
  f>>probs;
  if (draw_settings->GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
  
  while (!probs.Done() && f.IsValid()) {
    //------------------- if the cur_e is in range, display it -----------
    if (draw_settings->RangeX(probs.E())) {
      point_color=(draw_settings->GetColorMode()==COLOR_EQU) ? equs.Check_Equ(probs,&new_equ,prev_point) : 2;
      if (point_color>max_equ) max_equ=point_color;
      wxPen *cpen=wxThePenList->FindOrCreatePen(equ_colors[(point_color+color_start)%NUM_COLORS+1],3,wxSOLID);
      //      if (dc.current_pen!=cpen) dc.SetPen(cpen);
      dc.SetPen(*cpen);
      for (int plots=0;plots<draw_settings->GetNumPlots();plots++) {	// draw one or two triangle plots
	for (int iset_num=1;iset_num<=((plots) ?  draw_settings->GetPlotBottom() : draw_settings->GetPlotTop());iset_num++) {
	  iset=(plots) ? draw_settings->GetPlotBottom(iset_num) : draw_settings->GetPlotTop(iset_num);
				/*------------------ plot the point in color ----------------------*/
	  int st1=0,st2=0;	// which two strategies to plot
	  int i=1;
	  while(!st1) {if (draw_settings->GetStrategyShow(iset,i)) st1=i;i++;}
	  while(!st2) {if (draw_settings->GetStrategyShow(iset,i)) st2=i;i++;}
	  x=CalcX_3(probs[iset][st1],probs[iset][st2],ch,cw,(plots) ? BOTTOM_PLOT : TOP_PLOT);
	  y=CalcY_3(probs[iset][st1],ch,cw);
	  if (draw_settings->ConnectDots() && !new_equ) {
	    double prev_x=CalcX_3((*prev_point)[iset][st1],(*prev_point)[iset][st2],ch,cw,(plots) ? BOTTOM_PLOT : TOP_PLOT);
	    double prev_y=CalcY_3((*prev_point)[iset][st1],ch,cw);
	    dc.DrawLine(prev_x,prev_y,x,y);
	  }
	  else
	    dc.DrawPoint(x,y);
				// if there is an experimental data point for this cur_e, plot it
	  if (exp_data) DrawExpPoint_3(dc,probs.E(),iset,st1,st2,ch,cw,(plots) ? BOTTOM_PLOT : TOP_PLOT);
	}
      }
    }
    /*------------------ read in a line of data-----------------*/
    if (draw_settings->GetColorMode()!=COLOR_EQU && prev_point) (*prev_point)=probs;
    f>>probs;
  }
  if (prev_point) delete prev_point;
  if (!draw_settings->RestartOverlayColors()) color_start+=max_equ;
  PlotLabels(dc,ch,cw);
}


/******************************* UPDATE ***********************************/
void PxiCanvas::Update(wxDC& dc,int device)
{
  int		cw,ch;
  wxBeginBusyCursor();
  GetClientSize(&cw,&ch);
  if (device==PXI_UPDATE_SCREEN) {
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
  }
  else {
#ifdef wx_msw
    // For printing, scale the picture
    if (device==PXI_UPDATE_PRINTER) {
      // This will print each plot in a square box
      float width,height;
      dc.GetSize(&width,&height);
      cw=width*0.8;ch=height*0.8;
      dc.SetDeviceOrigin(width*0.1,height*0.1);
    }
#endif
    // For metafile stuff, set transparent text backround
    if (device==PXI_UPDATE_METAFILE) {
      dc.SetBackground(*wxWHITE_BRUSH);
      dc.SetBackgroundMode(wxTRANSPARENT);
    }
  }
  if (draw_settings->GetPlotMode()==PXI_PLOT_X) {
    PlotAxis_X(dc,draw_settings->GetStopMin(),draw_settings->GetStopMax(),draw_settings->GetDataMin(),
	       draw_settings->GetDataMax(),ch,cw,draw_settings->GetNumPlots(),
	       draw_settings->GetDataMode(),draw_settings->PlotFeatures(),headers[1].EStep());
    for (int i=1;i<=headers.Length();i++) PlotData_X(dc,ch,cw,headers[i],i);
  }
  if (draw_settings->GetPlotMode()==PXI_PLOT_2) {
    PlotAxis_2(dc,0,1,0,1,ch,cw,draw_settings->PlotFeatures());
    for (int i=1;i<=headers.Length();i++) PlotData_2(dc,ch,cw,headers[i]);
  }
  if (draw_settings->GetPlotMode()==PXI_PLOT_3) {
    PlotAxis_3(dc,ch,cw,draw_settings->GetNumPlots(),draw_settings->PlotFeatures());
    for (int i=1;i<=headers.Length();i++) PlotData_3(dc,ch,cw,headers[i],i);
  }
  //  if (draw_settings->GetShowGame()) ShowGame(dc,cw,ch,headers[1]);
  wxEndBusyCursor();
}



