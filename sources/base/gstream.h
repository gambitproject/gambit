//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Input and output streaming facilities
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GSTREAM_H
#define GSTREAM_H

#include <stdio.h>
#include "gmisc.h"
#include "gtext.h"

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif   // __GNUG__

class gbtInput  {
  private:
    gbtInput(const gbtInput &);
    gbtInput &operator=(const gbtInput &);

  public:
    gbtInput(void); 
    virtual ~gbtInput();

    virtual gbtInput& operator>>(int &x) = 0;
    virtual gbtInput& operator>>(unsigned int &x) = 0;
    virtual gbtInput& operator>>(long &x) = 0;
    virtual gbtInput& operator>>(char &x) = 0;
    virtual gbtInput& operator>>(double &x) = 0;
    virtual gbtInput& operator>>(float &x) = 0;
    virtual gbtInput& operator>>(char *x) = 0;

    virtual int get(char &c) = 0;
    virtual void unget(char c) = 0;
    virtual bool eof(void) const = 0;
    virtual void seekp(long x) const = 0;
    virtual long getpos(void) const = 0;
    virtual void setpos(long x) const = 0;
};

class gbtFileInput : public gbtInput  {
  private:
    FILE *f;

    gbtFileInput(const gbtFileInput &);
    gbtFileInput &operator=(const gbtFileInput &);

  public:
    class OpenFailed : public gbtException   {
    public:
      virtual ~OpenFailed()   { }
      gbtText Description(void) const;
    };

    class ReadFailed : public gbtException   {
    public:
      virtual ~ReadFailed()   { }
      gbtText Description(void) const;
    };

    gbtFileInput(FILE *);
    gbtFileInput(const char *);
    virtual ~gbtFileInput();

    gbtInput &operator>>(int &x);
    gbtInput &operator>>(unsigned int &x);
    gbtInput &operator>>(long &x);
    gbtInput &operator>>(char &x);
    gbtInput &operator>>(double &x);
    gbtInput &operator>>(float &x);
    gbtInput &operator>>(char *x);

    int get(char &c);
    void unget(char c);
    bool eof(void) const;
    void seekp(long x) const;
    long getpos(void) const;
    void setpos(long x) const;
};

class gbtStandardInput : public gbtInput  {
private:
  gbtStandardInput(const gbtStandardInput &);
  gbtStandardInput &operator=(const gbtStandardInput &);

public:
  class ReadFailed : public gbtException   {
  public:
    virtual ~ReadFailed()   { }
    gbtText Description(void) const;
  };

  gbtStandardInput(void);
  virtual ~gbtStandardInput();

  gbtInput &operator>>(int &x);
  gbtInput &operator>>(unsigned int &x);
  gbtInput &operator>>(long &x);
  gbtInput &operator>>(char &x);
  gbtInput &operator>>(double &x);
  gbtInput &operator>>(float &x);
  gbtInput &operator>>(char *x);

  int get(char &c);
  void unget(char c);
  bool eof(void) const;
  void seekp(long x) const;
  long getpos(void) const;
  void setpos(long x) const;
};

class gbtNullInput : public gbtInput  {
  private:
    gbtNullInput(const gbtNullInput &);
    gbtNullInput &operator=(const gbtNullInput &);

  public:
    gbtNullInput(void);
    virtual ~gbtNullInput();

    gbtInput &operator>>(int &x);
    gbtInput &operator>>(unsigned int &x);
    gbtInput &operator>>(long &x);
    gbtInput &operator>>(char &x);
    gbtInput &operator>>(double &x);
    gbtInput &operator>>(float &x);
    gbtInput &operator>>(char *x);

    int get(char &c);
    void unget(char c);
    bool eof(void) const;
    void seekp(long x) const;
    long getpos(void) const;
    void setpos(long x) const;
};

class gbtOutput  {
  private:
    gbtOutput(const gbtOutput &);
    gbtOutput &operator=(const gbtOutput &);

  public:
    gbtOutput(void);
    virtual ~gbtOutput();

    virtual gbtOutput &operator<<(int x) = 0;
    virtual gbtOutput &operator<<(unsigned int x) = 0;
    virtual gbtOutput &operator<<(bool x) = 0;
    virtual gbtOutput &operator<<(long x) = 0;
    virtual gbtOutput &operator<<(char x) = 0;
    virtual gbtOutput &operator<<(double x) = 0;
    virtual gbtOutput &operator<<(float x) = 0;
    virtual gbtOutput &operator<<(const char *x) = 0;
    virtual gbtOutput &operator<<(const void *x) = 0;

    virtual int GetWidth(void) const = 0;
    virtual gbtOutput &SetWidth(int w) = 0;
    virtual int GetPrec(void) const = 0;
    virtual gbtOutput &SetPrec(int p) = 0;
    virtual gbtOutput &SetExpMode(void) = 0;
    virtual gbtOutput &SetFloatMode(void) = 0;
    virtual char GetRepMode(void) const = 0;
};

class gbtStandardOutput : public gbtOutput  {
private:
  int Width, Prec;
  char Represent;

  gbtStandardOutput(const gbtStandardOutput &);
  gbtStandardOutput &operator=(const gbtStandardOutput &);

public:
  class OpenFailed : public gbtException   {
  public:
    virtual ~OpenFailed()   { }
    gbtText Description(void) const;
  };

  class WriteFailed : public gbtException   {
  public:
    virtual ~WriteFailed()   { }
    gbtText Description(void) const;
  };
  
  gbtStandardOutput(void);
  virtual ~gbtStandardOutput();

  int GetWidth(void) const;
  gbtOutput &SetWidth(int w);
  int GetPrec(void) const;
  gbtOutput &SetPrec(int p);
  gbtOutput &SetExpMode(void);
  gbtOutput &SetFloatMode(void);
  char GetRepMode(void) const;

  gbtOutput &operator<<(int x);
  gbtOutput &operator<<(unsigned int x);
  gbtOutput &operator<<(bool x);
  gbtOutput &operator<<(long x);
  gbtOutput &operator<<(char x);
  gbtOutput &operator<<(double x);
  gbtOutput &operator<<(float x);
  gbtOutput &operator<<(const char *x);
  gbtOutput &operator<<(const void *x);
};

class gbtFileOutput : public gbtOutput  {
  private:
    FILE *f;
    const gbtText filename; 
    bool keepClosed;
    int Width, Prec;
    char Represent;

    gbtFileOutput(const gbtFileOutput &);
    gbtFileOutput &operator=(const gbtFileOutput &);
    void Open(void);
    void Close(void);

  public:
    class OpenFailed : public gbtException   {
    public:
      virtual ~OpenFailed()   { }
      gbtText Description(void) const;
    };

    class WriteFailed : public gbtException   {
    public:
      virtual ~WriteFailed()   { }
      gbtText Description(void) const;
    };

    gbtFileOutput(const char *, bool append = false, bool close = true);
    virtual ~gbtFileOutput();

    int GetWidth(void) const;
    gbtOutput &SetWidth(int w);
    int GetPrec(void) const;
    gbtOutput &SetPrec(int p);
    gbtOutput &SetExpMode(void);
    gbtOutput &SetFloatMode(void);
    char GetRepMode(void) const;

    gbtOutput &operator<<(int x);
    gbtOutput &operator<<(unsigned int x);
    gbtOutput &operator<<(bool x);
    gbtOutput &operator<<(long x);
    gbtOutput &operator<<(char x);
    gbtOutput &operator<<(double x);
    gbtOutput &operator<<(float x);
    gbtOutput &operator<<(const char *x);
    gbtOutput &operator<<(const void *x);
};

class gbtNullOutput : public gbtOutput  {
  private:
    gbtNullOutput(const gbtNullOutput &);
    gbtNullOutput &operator=(const gbtNullOutput &);

  public:
    gbtNullOutput(void);
    virtual ~gbtNullOutput();

    int GetWidth(void) const;
    gbtOutput &SetWidth(int w);
    int GetPrec(void) const;
    gbtOutput &SetPrec(int p);
    gbtOutput &SetExpMode(void);
    gbtOutput &SetFloatMode(void);
    char GetRepMode(void) const;

    gbtOutput &operator<<(int x);
    gbtOutput &operator<<(unsigned int x);
    gbtOutput &operator<<(bool x);
    gbtOutput &operator<<(long x);
    gbtOutput &operator<<(char x);
    gbtOutput &operator<<(double x);
    gbtOutput &operator<<(float x);
    gbtOutput &operator<<(const char *x);
    gbtOutput &operator<<(const void *x);
};

#endif   // GSTREAM_H
