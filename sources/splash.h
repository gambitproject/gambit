//
// FILE: splash.h -- Definition of a splash screen dialog
//
// $Id$
//

#ifndef SPLASH_H
#define SPLASH_H

class Splash : public wxDialog {
private:
  wxBitmap *m_bitmap;
  wxTimer *m_timer;

  // Event handlers
  void OnQuit(wxCommandEvent &);
  void OnTimer(wxTimerEvent &); 
  void OnPaint(wxPaintEvent &);

public:
  Splash(long p_seconds);
  ~Splash();

  DECLARE_EVENT_TABLE()
};

#endif  // SPLASH_H

