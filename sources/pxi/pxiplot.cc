//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot base class
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "pxiplot.h"

#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

//======================================================================
//                     class PxiPlot: Lifecycle
//======================================================================

PxiPlot::PxiPlot(wxWindow *p_parent, const wxPoint &p_position,
		 const wxSize &p_size,
		 const gArray<PxiFile *> &p_qreFiles, int p_page,
		 const ExpData &p_expData)
  : wxScrolledWindow(p_parent, -1, p_position, p_size),
    m_qreFiles(p_qreFiles), m_expData(p_expData),
    m_landscape(false), m_width(850/2), m_height(1100/2), m_scale(1.0), 
    m_dc(new wxMemoryDC), m_page(p_page)
{
  m_lambdaAxisProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_lambdaAxisProp.m_color = *wxBLUE;
  m_lambdaAxisProp.m_scale.m_minimum = wxString(".01");
  m_lambdaAxisProp.m_scale.m_maximum = wxString("100"); 
  m_lambdaAxisProp.m_scale.m_divisions = 10;
  m_lambdaAxisProp.m_scale.m_useLog = true;
  m_lambdaAxisProp.m_scale.m_canUseLog = true;
  m_lambdaAxisProp.m_display.m_shown = true;
  m_lambdaAxisProp.m_display.m_ticks = true;
  m_lambdaAxisProp.m_display.m_numbers = true;

  m_probAxisProp.m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);
  m_probAxisProp.m_color = *wxBLUE;
  m_probAxisProp.m_scale.m_minimum = "0.0";
  m_probAxisProp.m_scale.m_maximum = "1.0";
  m_probAxisProp.m_scale.m_divisions = 10;
  m_probAxisProp.m_scale.m_useLog = false;
  m_probAxisProp.m_scale.m_canUseLog = false;
  m_probAxisProp.m_display.m_shown = true;
  m_probAxisProp.m_display.m_ticks = true;
  m_probAxisProp.m_display.m_numbers = true;

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


//======================================================================
//                   class PxiPlot: Event handlers
//======================================================================

BEGIN_EVENT_TABLE(PxiPlot, wxScrolledWindow)
  EVT_PAINT(PxiPlot::OnPaint)
END_EVENT_TABLE()

void PxiPlot::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  dc.Blit(0, 0, GetSize().GetWidth(), GetSize().GetHeight(),
	  m_dc, 0, 0);
}

//======================================================================
//               class PxiPlot: Plot-drawing routines
//======================================================================

void PxiPlot::PlotLabels(wxDC &dc, int ch,int cw)
{
  dc.SetTextForeground(*wxBLACK);
  //dc.SetBackgroundMode(wxTRANSPARENT);
  dc.SetFont(m_legendProp.m_font);
  for (int i=1;i<=labels.Length();i++) {
    wxCoord tw,th;
    dc.GetTextExtent(labels[i].label,&tw,&th);
    dc.DrawText(labels[i].label,(int) (labels[i].x*cw-tw/2),
		(int) (labels[i].y*ch-th/2));
  }
}

//
// Draws a token.
// If we are using COLOR_PROB mode, a different token is drawn for
// each strategy #.  Otherwise, we just draw an ellipse.  
//
void PxiPlot::DrawToken(wxDC &dc, int x, int y, int point)
{
  int ts = GetOverlayProperties().m_tokenSize;

#ifdef UNUSED
  if (m_drawSettings.GetColorMode() == COLOR_PROB) {
#endif  // UNUSED
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);

    switch (GetOverlayProperties().m_token) {
    case Overlay::tokenNUMBER: {
      wxString tmp = wxString::Format("%d", point);
      dc.SetFont(GetOverlayProperties().m_font);
      dc.SetTextForeground(*wxBLACK);
      wxCoord tw, th;
      dc.GetTextExtent(tmp, &tw, &th);
      dc.DrawText(tmp, (int) (x-tw/2), (int) (y-th/2));
      break;
    }
    case Overlay::tokenX:
      dc.DrawLine((int) (x-ts), (int) (y-ts), (int) (x+ts), (int) (y+ts));
      dc.DrawLine((int) (x-ts), (int) (y+ts), (int) (x+ts), (int) (y-ts));
      break;
    case Overlay::tokenPLUS:
      dc.DrawLine((int) (x-ts), y, (int) (x+ts), y);
      dc.DrawLine(x, (int) (y-ts), x, (int) (y+ts));
      break;
    case Overlay::tokenCIRCLE:
      dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
      break;
    case Overlay::tokenDEL:
      dc.DrawLine(x - ts, y - ts, x, y + ts);
      dc.DrawLine(x, y + ts, x + ts, y - ts);
      dc.DrawLine(x - ts, y - ts, x + ts, y - ts);
      break;
    case Overlay::tokenTRIANGLE:
      dc.DrawLine(x - ts, y + ts, x, y - ts);
      dc.DrawLine(x, y - ts, x + ts, y + ts);
      dc.DrawLine(x - ts, y + ts, x + ts, y + ts);
      break;
    case Overlay::tokenSQUARE:
      dc.DrawLine(x - ts, y + ts, x - ts, y - ts);
      dc.DrawLine(x - ts, y - ts, x + ts, y - ts);
      dc.DrawLine(x + ts, y - ts, x + ts, y + ts);
      dc.DrawLine(x + ts, y + ts, x - ts, y + ts);
      break;
    case Overlay::tokenSTAR:
      dc.DrawLine(x - ts, y - ts, x + ts, y + ts);
      dc.DrawLine(x - ts, y + ts, x + ts, y - ts);
      dc.DrawLine(x, y - ts, x, y + ts);
      break;
    }
#ifdef UNUSED
  }
  else {
    dc.DrawEllipse(x-ts,y-ts,2*ts,2*ts);
  }
#endif  // UNUSED
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
  
  dc.SetPen(*wxBLACK_PEN);

  // used for square aspect ratio that fits in window
  const int XOFF = 30;
  //  int side=gmin(cw-2*XOFF, ch-2*XOFF);	

  DoPlot(dc, XOFF, ch-XOFF, cw-2*XOFF, ch-2*XOFF, 1);
  wxEndBusyCursor();
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

