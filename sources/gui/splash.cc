//
// FILE: splash.cc -- Implements a splash screen
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "splash.h"
#include "bitmaps/gambit.xpm"

const int idTIMER = 3000;

BEGIN_EVENT_TABLE(Splash, wxFrame)
  EVT_TIMER(idTIMER, Splash::OnTimer)
  EVT_PAINT(Splash::OnPaint)
END_EVENT_TABLE()

Splash::Splash(long p_seconds)
  : wxFrame(0, -1, "", wxDefaultPosition, wxDefaultSize,
	     wxDIALOG_MODAL | wxSIMPLE_BORDER),
    m_bitmap(NULL), m_timer(NULL)
{
  m_bitmap = new wxBITMAP(gambit);
  m_timer = new wxTimer(this, idTIMER);

  SetSize(0, 0, m_bitmap->GetWidth() + 4, m_bitmap->GetHeight() + 4);
  Center();
  if (m_timer) {
    m_timer->Start(1000*p_seconds, true);
  }
}

Splash::~Splash()
{
  delete m_timer;
  delete m_bitmap;
}

void Splash::OnQuit(wxCommandEvent &)
{
  Close(true);
}

void Splash::OnTimer(wxTimerEvent &p_event)
{
  if (m_timer) {
    m_timer->Stop(); 
  }
  Show(false);
  Destroy(); 
}

void Splash::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  
  if (m_bitmap) {
    dc.DrawBitmap(*m_bitmap, 2, 2);
  }
}

