//
// FILE: gwstream.h -- MFC-based stream output class
//
// $Id$
//

#ifndef GWSTREAM_H
#define GWSTREAM_H

#include "gstream.h"

class gWinOutput : public gOutput  {
  private:
    int Width,Prec;
    char Represent;
    char m_Buffer[4096];

    gWinOutput(const gWinOutput &);
    gWinOutput &operator=(const gWinOutput &);

  protected:
    void OutputString(const char* s) const;

  public:
    class OpenFailed : public gException   {
    public:
      virtual ~OpenFailed()   { }
      gText Description(void) const;
    };

    class WriteFailed : public gException   {
    public:
      virtual ~WriteFailed()   { }
      gText Description(void) const;
    };

    gWinOutput(void);
    virtual ~gWinOutput();

    int GetWidth(void) const;
    gOutput &SetWidth(int w);
    int GetPrec(void) const;
    gOutput &SetPrec(int p);
    gOutput &SetExpMode(void);
    gOutput &SetFloatMode(void);
    char GetRepMode(void) const;

    gOutput &operator<<(int x);
    gOutput &operator<<(unsigned int x);
    gOutput &operator<<(bool x);
    gOutput &operator<<(long x);
    gOutput &operator<<(char x);
    gOutput &operator<<(double x);
    gOutput &operator<<(float x);
    gOutput &operator<<(const char *x);
    gOutput &operator<<(const void *x);
};

#endif  // GWSTREAM_H
