//
// FILE: gstream.h -- Input and output streaming facilities
//
// $Id$
//

#ifndef GSTREAM_H
#define GSTREAM_H

#include <stdio.h>
#include "gmisc.h"

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class gInput  {
  private:
    gInput(const gInput &);
    gInput &operator=(const gInput &);

  public:
    gInput(void); 
    virtual ~gInput();

    virtual gInput& operator>>(int &x) = 0;
    virtual gInput& operator>>(unsigned int &x) = 0;
    virtual gInput& operator>>(long &x) = 0;
    virtual gInput& operator>>(char &x) = 0;
    virtual gInput& operator>>(double &x) = 0;
    virtual gInput& operator>>(float &x) = 0;
    virtual gInput& operator>>(char *x) = 0;

    virtual int get(char &c) = 0;
    virtual void unget(char c) = 0;
    virtual bool eof(void) const = 0;
    virtual void seekp(long x) const = 0;
    virtual long getpos(void) const = 0;
    virtual void setpos(long x) const = 0;
};

class gFileInput : public gInput  {
  private:
    FILE *f;

    gFileInput(const gFileInput &);
    gFileInput &operator=(const gFileInput &);

  public:
    class OpenFailed : public gException   {
    public:
      virtual ~OpenFailed()   { }
      gText Description(void) const;
    };

    class ReadFailed : public gException   {
    public:
      virtual ~ReadFailed()   { }
      gText Description(void) const;
    };

    gFileInput(FILE *);
    gFileInput(const char *);
    virtual ~gFileInput();

    gInput &operator>>(int &x);
    gInput &operator>>(unsigned int &x);
    gInput &operator>>(long &x);
    gInput &operator>>(char &x);
    gInput &operator>>(double &x);
    gInput &operator>>(float &x);
    gInput &operator>>(char *x);

    int get(char &c);
    void unget(char c);
    bool eof(void) const;
    void seekp(long x) const;
    long getpos(void) const;
    void setpos(long x) const;
};

class gNullInput : public gInput  {
  private:
    gNullInput(const gNullInput &);
    gNullInput &operator=(const gNullInput &);

  public:
    gNullInput(void);
    virtual ~gNullInput();

    gInput &operator>>(int &x);
    gInput &operator>>(unsigned int &x);
    gInput &operator>>(long &x);
    gInput &operator>>(char &x);
    gInput &operator>>(double &x);
    gInput &operator>>(float &x);
    gInput &operator>>(char *x);

    int get(char &c);
    void unget(char c);
    bool eof(void) const;
    void seekp(long x) const;
    long getpos(void) const;
    void setpos(long x) const;
};

class gOutput  {
  private:
    gOutput(const gOutput &);
    gOutput &operator=(const gOutput &);

  public:
    gOutput(void);
    virtual ~gOutput();

    virtual gOutput &operator<<(int x) = 0;
    virtual gOutput &operator<<(unsigned int x) = 0;
    virtual gOutput &operator<<(bool x) = 0;
    virtual gOutput &operator<<(long x) = 0;
    virtual gOutput &operator<<(char x) = 0;
    virtual gOutput &operator<<(double x) = 0;
    virtual gOutput &operator<<(float x) = 0;
    virtual gOutput &operator<<(const char *x) = 0;
    virtual gOutput &operator<<(const void *x) = 0;

    virtual int GetWidth(void) const = 0;
    virtual gOutput &SetWidth(int w) = 0;
    virtual int GetPrec(void) const = 0;
    virtual gOutput &SetPrec(int p) = 0;
    virtual gOutput &SetExpMode(void) = 0;
    virtual gOutput &SetFloatMode(void) = 0;
    virtual char GetRepMode(void) const = 0;
};

class gFileOutput : public gOutput  {
  private:
    FILE *f;
    int Width, Prec;
    char Represent;

    gFileOutput(const gFileOutput &);
    gFileOutput &operator=(const gFileOutput &);

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

    gFileOutput(const char *, bool append = false);
    gFileOutput(FILE *);
    virtual ~gFileOutput();

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

class gNullOutput : public gOutput  {
  private:
    gNullOutput(const gNullOutput &);
    gNullOutput &operator=(const gNullOutput &);

  public:
    gNullOutput(void);
    virtual ~gNullOutput();

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



#ifdef __BORLANDC__

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

#endif //  __BORLANDC__



extern gInput &gin, &gzero;
extern gOutput &gout, &gerr, &gnull;

#endif   // GSTREAM_H


