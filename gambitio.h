//#
//# FILE: gambitio.h -- Declaration of input and output classes
//#
//# $Id$
//#

#ifndef GAMBITIO_H
#define GAMBITIO_H

#include <stdio.h>

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class gInput  {
  private:
    gInput(const gInput &);
    gInput &operator=(const gInput &);

  public:
    gInput(void)   { }
    virtual ~gInput()  { }

    virtual gInput& operator>>(int &x) = 0;
    virtual gInput& operator>>(unsigned int &x) = 0;
    virtual gInput& operator>>(long &x) = 0;
    virtual gInput& operator>>(char &x) = 0;
    virtual gInput& operator>>(double &x) = 0;
    virtual gInput& operator>>(float &x) = 0;
    virtual gInput& operator>>(char *x) = 0;

    virtual int get(char &c) = 0;
    virtual void unget(char c) = 0;
    virtual int eof(void) const = 0;
    virtual void seekp(long x) const = 0;

    virtual int IsValid(void) const = 0;
};

class gFileInput : public gInput  {
  private:
    FILE *f;
    int valid;

    gFileInput(const gFileInput &);
    gFileInput &operator=(const gFileInput &);

  public:
    gFileInput(void);
    gFileInput(FILE *);
    gFileInput(const char *);
    virtual ~gFileInput();

    gFileInput &operator=(FILE *);
    gFileInput &operator=(const char *);

    gInput &operator>>(int &x);
    gInput &operator>>(unsigned int &x);
    gInput &operator>>(long &x);
    gInput &operator>>(char &x);
    gInput &operator>>(double &x);
    gInput &operator>>(float &x);
    gInput &operator>>(char *x);

    int get(char &c);
    void unget(char c);
    int eof(void) const;
    void seekp(long x) const;

    int IsValid(void) const;
};

extern gFileInput gin;


class gOutput  {
  private:
    gOutput(const gOutput &);
    gOutput &operator=(const gOutput &);

  public:
    gOutput(void)   { }
    virtual ~gOutput()   { }

    virtual gOutput &operator<<(int x) = 0;
    virtual gOutput &operator<<(unsigned int x) = 0;
    virtual gOutput &operator<<(long x) = 0;
    virtual gOutput &operator<<(char x) = 0;
    virtual gOutput &operator<<(double x) = 0;
    virtual gOutput &operator<<(float x) = 0;
    virtual gOutput &operator<<(const char *x) = 0;
    virtual gOutput &operator<<(const void *x) = 0;

    virtual int IsValid(void) const = 0;
};

class gFileOutput : public gOutput  {
  private:
    FILE *f;
    int valid;

    gFileOutput(const gFileOutput &);
    gFileOutput &operator=(const gFileOutput &);

  public:
    gFileOutput(void);
    gFileOutput(const char *);
    gFileOutput(FILE *);
    virtual ~gFileOutput();

    gFileOutput &operator=(FILE *);
    gFileOutput &operator=(const char *);

    gOutput &operator<<(int x);
    gOutput &operator<<(unsigned int x);
    gOutput &operator<<(long x);
    gOutput &operator<<(char x);
    gOutput &operator<<(double x);
    gOutput &operator<<(float x);
    gOutput &operator<<(const char *x);
    gOutput &operator<<(const void *x);

    int IsValid(void) const;
};

extern gFileOutput gout, gerr;

#endif   // GAMBITIO_H
