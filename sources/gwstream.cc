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

void gWinOutput::OutputString(const char *s) const
{
  ((CWinEditView*) ((CMainFrame*) AfxGetMainWnd())->GetActiveView())->PutString( s );
}


gWinOutput::gWinOutput(void)
{
  valid=0;
  Width=0;
  Prec=6;
  Represent='f';
}


gWinOutput::~gWinOutput()
{
  valid=0;
}

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
  int c=sprintf(m_Buffer, "%*d", Width,  x);  valid = (c == 1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(unsigned int x)
{
  int c=sprintf(m_Buffer, "%*d", Width,  x);valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(bool x)
{
  int c=sprintf(m_Buffer, "%c", (x) ? 'T' : 'F');valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(long x)
{
  int c=sprintf(m_Buffer, "%*ld", Width, x);valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(char x)
{
  int c=sprintf(m_Buffer, "%c", x);valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(double x)
{
  int c = 0;

  switch (Represent) {
    case 'f':
      c = sprintf(m_Buffer, "%*.*f", Width, Prec, x);
      break;
    case 'e':
      c = sprintf(m_Buffer, "%*.*e", Width, Prec, x);
      break;
    }
	valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(float x)
{
  int c = 0;

  switch (Represent) {
    case 'f':
      c=sprintf(m_Buffer, "%*.*f", Width, Prec, x);
      break;
    case 'e':
      c=sprintf(m_Buffer, "%*.*e", Width, Prec, x);
      break;
    }
	valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(const char *x)
{
	int c=sprintf(m_Buffer, "%s", x);valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

gOutput &gWinOutput::operator<<(const void *x)
{
  int c=sprintf(m_Buffer, "%p", x);valid=(c==1) ? 1 : 0;
  OutputString( m_Buffer );
  return *this;
}

bool gWinOutput::IsValid(void) const
{
  return valid;
}

