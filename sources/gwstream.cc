//
// FILE: gwstream.cc -- MFC-based stream classes
//


#include "stdafx.h"
#include "MainFrm.h"
#include "WinEditDoc.h"
#include "WinEditView.h"

#include "gstream.h"

//--------------------------------------------------------------------------
//                         gWinOutput member functions
//--------------------------------------------------------------------------

#include "gtext.h"

gText gWinOutput::OpenFailed::Description(void) const
{
  return "Open failed in gWinOutput";
}

gText gWinOutput::WriteFailed::Description(void) const
{
  return "Write failed in gWinOutput";
}

void gWinOutput::OutputString(const char *s) const
{
  ((CWinEditView*) ((CMainFrame*) AfxGetMainWnd())->GetActiveView())->PutString( s );
}


gWinOutput::gWinOutput(void)
{
  Width=0;
  Prec=6;
  Represent='f';
}


gWinOutput::~gWinOutput()
{ }

int gWinOutput::GetWidth(void)
{
  return Width;
}

gOutput &gWinOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gWinOutput::GetPrec(void)
{
  return Prec;
}

gOutput &gWinOutput::SetPrec(int p) 
{
  Prec = p;
  return *this;
}

gOutput &gWinOutput::SetExpMode(void) 
{
  Represent = 'e';
  return *this;
}

gOutput &gWinOutput::SetFloatMode(void)
{
  Represent = 'f';
  return *this;
}

char gWinOutput::GetRepMode(void)
{
  return Represent;
}


gOutput &gWinOutput::operator<<(int x)
{
  if (sprintf(m_Buffer, "%*d", Width,  x)<0) throw WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(unsigned int x)
{
  if (sprintf(m_Buffer, "%*d", Width,  x)<0) throw WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(bool x)
{
  if(sprintf(m_Buffer, "%c", (x) ? 'T' : 'F')<0) throw WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(long x)
{
  if( sprintf(m_Buffer, "%*ld", Width, x)<0) throw WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(char x)
{
  if(sprintf(m_Buffer, "%c", x)<0) throw WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(double x)
{
  switch (Represent) {
    case 'f':
      //      c = sprintf(m_Buffer, "%*.*f", Width, Prec, x);
      if(sprintf(m_Buffer, "%*.*f", Width, Prec, x)<0) throw WriteFailed();
      break;
    case 'e':
      //      c = sprintf(m_Buffer, "%*.*e", Width, Prec, x);
      if(sprintf(m_Buffer, "%*.*e", Width, Prec, x)<0) throw WriteFailed();
      break;
    }
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(float x)
{
  switch (Represent) {
    case 'f':
      //      c=sprintf(m_Buffer, "%*.*f", Width, Prec, x);
      if(sprintf(m_Buffer, "%*.*f", Width, Prec, x)!=1) WriteFailed();
      break;
    case 'e':
      //      c=sprintf(m_Buffer, "%*.*e", Width, Prec, x);
      if(sprintf(m_Buffer, "%*.*e", Width, Prec, x)!=1) WriteFailed();
      break;
    }
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(const char *x)
{
  if(sprintf(m_Buffer, "%s", x)!=1) WriteFailed();
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(const void *x)
{
  if(sprintf(m_Buffer, "%p", x)!=1) WriteFailed();
  OutputString( m_Buffer );
  return *this;
}
