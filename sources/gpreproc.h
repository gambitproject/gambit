//
// FILE: gpreproc.h -- Implementation of gPreprocessor class,
//                     preprocessor for the command language input
//
// $Id$
//

#ifndef GPREPROC_H
#define GPREPROC_H

#include "gcmdline.h"
#include "gtext.h"
#include "gstack.h"

class GSM;

class gPreprocessor  {
private:
  GSM &m_environment;
  gclCommandLine *m_CmdLine;

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
  gPreprocessor(GSM &, gclCommandLine *p_cmdline, const char *p_cmd = NULL); 
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

