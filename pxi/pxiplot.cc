//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot base class
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

#include "pxiplot.h"

#include "gmisc.h"
#include "wxmisc.h"
#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

void PxiPlot::PlotLabels(wxDC &dc, int ch,int cw)
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


//********************************** DRAW TOKEN ******************************
// Draw a token.  If we are using COLOR_PROB mode, a different token is drawn for
// each strategy #.  Otherwise, we just draw an ellipse.  Note that each token is
// a 8x8 image.
#define		NUM_TOKENS		7
void PxiPlot::DrawToken(wxDC &dc, int x, int y, int st)
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

void PxiPlot::Update(wxDC& dc,int device)
{
  int cw,ch;
  const wxFont &font = m_lambdaAxisProp.m_font;
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
  dc.SetTextForeground(m_lambdaAxisProp.m_color);
  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(m_drawSettings.GetClearBrush());
  
  const PlotInfo &thisplot(m_drawSettings.GetPlotInfo());
  dc.SetPen(*wxBLACK_PEN);

  // used for square aspect ratio that fits in window
  const int XOFF = 30;
  int side=gmin(cw-2*XOFF, ch-2*XOFF);	

  DoPlot(dc,thisplot,XOFF,ch-XOFF,cw-2*XOFF, ch-2*XOFF, 1);
  wxEndBusyCursor();
}

void PxiPlot::NewExpData(ExpDataParams &P) 
{ 
  if(exp_data) delete exp_data;
  exp_data = NULL;
  exp_data = new ExpData(P); 
}

BEGIN_EVENT_TABLE(PxiPlot, wxScrolledWindow)
  EVT_PAINT(PxiPlot::OnPaint)
END_EVENT_TABLE()

PxiPlot::PxiPlot(wxWindow *p_parent, const wxPoint &p_position,
		     const wxSize &p_size,
		     const FileHeader &p_header, int p_page)
  : wxScrolledWindow(p_parent, -1, p_position, p_size),
    m_header(p_header), m_drawSettings(m_header, p_page),
    exp_data(NULL),
    m_landscape(false), m_width(850/2), m_height(1100/2), m_scale(1.0), 
    m_dc(new wxMemoryDC), m_page(p_page)
{
  m_lambdaAxisProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_lambdaAxisProp.m_color = *wxBLUE;
  m_lambdaAxisProp.m_scale.m_minimum =
    wxString::Format("%f", p_header.EStart());
  m_lambdaAxisProp.m_scale.m_maximum = 
    wxString::Format("%f", p_header.EStop());
  m_lambdaAxisProp.m_scale.m_divisions = 10;
  m_lambdaAxisProp.m_scale.m_useLog = true;
  m_lambdaAxisProp.m_scale.m_canUseLog = true;

  m_probAxisProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_probAxisProp.m_color = *wxBLUE;
  m_probAxisProp.m_scale.m_minimum = "0.0";
  m_probAxisProp.m_scale.m_maximum = "1.0";
  m_probAxisProp.m_scale.m_divisions = 10;
  m_probAxisProp.m_scale.m_useLog = false;
  m_probAxisProp.m_scale.m_canUseLog = false;

  m_titleProp.m_title = wxString::Format("Plot %d", p_page);
  m_titleProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_titleProp.m_color = *wxBLUE;

  m_legendProp.m_showLegend = true;
  m_legendProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_legendProp.m_color = *wxBLUE;

  // fit to 8 1/2 x 11 inch  
  SetScale(1.0);

  m_dc->SelectObject(wxBitmap(2000, 2000));
  Show(true);
}

PxiPlot::~PxiPlot()
{
  delete m_dc;
}

void PxiPlot::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  dc.Blit(0, 0, GetSize().GetWidth(), GetSize().GetHeight(),
	  m_dc, 0, 0);
}

void PxiPlot::Render(void)
{
  m_dc->Clear();
  m_dc->BeginDrawing();
  Update(*m_dc, PXI_UPDATE_SCREEN);
  m_dc->EndDrawing();
  Refresh();
}

void PxiPlot::SetScale(double x) 
{
  const int ppu = 25;
  m_scale = x; 
  SetScrollbars(ppu, ppu,
		(int) (GetScale()*Width()/ppu),
		(int) (GetScale()*Height()/ppu));
}

