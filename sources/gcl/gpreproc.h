//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to preprocess GCL input
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

#ifndef GPREPROC_H
#define GPREPROC_H

#include "base/base.h"
#include "gcmdline.h"

class GSM;

class gPreprocessor  {
private:
  GSM &m_environment;
  GCL::CommandLine *m_CmdLine;

  gStack<gInput *> m_InputStack;
  gStack<int> m_LineNumberStack;
  gStack<gText> m_FileNameStack;

  gText m_RawLine, m_PrevFileName;
  int m_PrevLineNumber;
  gText m_StartupString;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  gInput *LoadInput(gText &p_name);
  bool ExpectText(const char *p_text);
  
  bool EOL(char p_char) const { return (p_char == '\n' || p_char == '\r'); }

  void SetPrompt(bool p_prompt);
  void GetChar(char &p_char);
  bool IsQuoteEscapeSequence(const gText &p_line) const;

public:
  gPreprocessor(GSM &, GCL::CommandLine *p_cmdline, const char *p_cmd = NULL); 
  ~gPreprocessor();

  //----------------------------------------------
  // eof
  //   If true, then there is no more input from
  //   the preprocessor.
  //-----------------------------------------------
  bool eof(void) const { return m_InputStack.Depth() == 0; }


  //---------------------------------------------
  // GetLine
  //   Returns the preprocessed lines.  See 
  //   gpreproc.cc for details.
  //----------------------------------------------
  gText GetLine(void);


  //---------------------------------------------
  // GetRawLine
  //   Returns the un-preprocessed line corresponding to
  //   the previous GetLine() call.
  //----------------------------------------------
  const gText& GetRawLine(void) const  { return m_RawLine; }


  //---------------------------------------------
  // GetFileName
  //   Returns the file name corresponding to
  //   the previous GetLine() call.
  //---------------------------------------------
  const gText& GetFileName(void) const { return m_PrevFileName; }
  

  //---------------------------------------------
  // GetLineNumber
  //   Returns the line number corresponding to
  //   the previous GetLine() call.
  //---------------------------------------------
  int GetLineNumber(void) const  { return m_PrevLineNumber; }
};

#endif // GPREPROC_H

