//
// FILE: gpreproc.cc -- Implementation of gPreprocessor class,
//                      preprocessor for the command language input
//
// $Id$
//

#include <ctype.h>

#include "gpreproc.h"
#include "system.h"

#include "gsm.h"

//-------------------------------------------------------------------------
//            gPreprocessor: private auxiliary member functions
//-------------------------------------------------------------------------

bool gPreprocessor::ExpectText(const char *p_text)
{
  int length = strlen(p_text);
  char c;

  if (m_InputStack.Peek()->eof())
    return false;

  for (int i = 0; i < length; i++) {
    GetChar(c);
    if (m_InputStack.Peek()->eof() || c != p_text[i])
      return false;
  }
  return true;
}

//
// Note: filename might be changed after this call
//
gInput *gPreprocessor::LoadInput(gText &p_name)
{
  gInput *infile = NULL;

  extern char *_SourceDir;
  const char *SOURCE = _SourceDir; 
  const char SLASH = System::Slash();
  
  bool search = (strchr((char *) p_name, SLASH) == NULL);
  gText IniFileName = p_name;

  gList<gText> paths;
  paths.Append(p_name);
  if (search) {
    if (System::GetEnv("HOME") != NULL) 
      paths.Append(System::Slashify(((gText) System::GetEnv("HOME") +
		    (gText) SLASH + (gText) p_name)));
    if (System::GetEnv("GCLLIB") != NULL) 
      paths.Append(System::Slashify(((gText) System::GetEnv("GCLLIB") +
		    (gText) SLASH + (gText) p_name)));
    if (SOURCE != NULL) 
      paths.Append(System::Slashify((gText) SOURCE + (gText) SLASH + (gText) p_name));
  }    

  for (int i = 1; i <= paths.Length(); i++) {
    try {
      infile = new gFileInput(paths[i]);
      p_name = paths[i];
      return infile;
    }

    // This #ifdef is here because under AIX with g++ 2.8.1, this
    // catch handler is causing a coredump.
#ifdef _AIX
    catch (...) {
#else
    catch (gFileInput::OpenFailed &) {
#endif   // _AIX
    }
  }

  return 0;
}

void gPreprocessor::SetPrompt(bool p_prompt)
{
  if (m_InputStack.Peek() == m_CmdLine)
    m_CmdLine->SetPrompt(p_prompt);	  
}

void gPreprocessor::GetChar(char &p_char)
{
  // Input is sought first from any open Include[] files
  // (in particular, gclini.gcl and other command-line arguments);
  // then from any unexecuted parts of the initString;
  // finally, from the GCL::CommandLine object ("standard input")

  if (m_InputStack.Depth() > 1 && !m_InputStack.Peek()->eof()) {
    if (m_InputStack.Peek()->getpos() == 0L) {
      m_environment.OutputStream() << "GCL: Reading file \"";
      m_environment.OutputStream() << m_FileNameStack.Peek();
      m_environment.OutputStream() << "\".\n";
    }
    m_InputStack.Peek()->get(p_char);
    if (EOL(p_char)) {
      ++m_LineNumberStack.Peek();
    }
  }
  else if (m_StartupString.Length() > 0)  {
    p_char = m_StartupString[0u];
    m_StartupString = m_StartupString.Right(m_StartupString.Length() - 1);
  }
  else {
    m_InputStack.Peek()->get(p_char);
    if (EOL(p_char))
      ++m_LineNumberStack.Peek();
  }

  // This is a bit of a hack to eliminate spurious error messages at the
  // end of include files.
  if (!isprint(p_char) && !isspace(p_char)) {
    p_char = '\n';
  }

  m_RawLine += p_char;
}

bool gPreprocessor::IsQuoteEscapeSequence(const gText &p_line) const
{
  bool backslash = false;
  char c = p_line[p_line.Length() - 1];
  if (c == '\"') {
    // if there's an odd number of consecutive backslashes,
    //   then it's an escape sequence 
    unsigned int i = 2;
    while (p_line.Length() >= i && p_line[p_line.Length() - i] == '\\') {
      backslash = !backslash;
      i++;
    }
  }
  return backslash;
}



gPreprocessor::gPreprocessor(GSM &p_environment, GCL::CommandLine *p_cmdline,
			     const char *p_cmd /* = NULL */) 
  : m_environment(p_environment), m_CmdLine(p_cmdline),
    m_PrevFileName("console"), m_PrevLineNumber(1), m_StartupString(p_cmd)
{
  m_InputStack.Push(m_CmdLine);
  m_FileNameStack.Push(m_PrevFileName);
  m_LineNumberStack.Push(m_PrevLineNumber);

  if (p_cmd && !EOL(m_StartupString[m_StartupString.Length() - 1]))
    m_StartupString += '\n';
}

gPreprocessor::~gPreprocessor()
{
  while (m_InputStack.Depth() > 1)
    delete m_InputStack.Pop();
}




//-------------------------------------------------------------------
// GetLine
//   Does the main pre-processing job.
//
//  This function handles the following:
//
//  o Include[] statements.  If the file exists, it is included, and the
//    Include[] function call is replaced with "True".  If the file is
//    not found or there is a syntax error, it is replaced with "False".
//   
//  o Strips all comments.  The comment types processed are C and C++ 
//    style comments.
//  
//  o Filter out explicit continuations, i.e., a backslash at the end 
//    of the line.  Only works if the backslash is the last character
//    on the line; trailing spaces are not accepted.
//
//  o Handle implicit continuations (bracket matching)
//    If brackets are open at a carriage return, then the CR is
//    replaced with a space.
//---------------------------------------------------------------------    

gText gPreprocessor::GetLine(void)
{
  m_RawLine = "";

  // If no more input available, return nothing.
  if (m_StartupString.Length() == 0 && eof()) {
    return "";
  }

  // Record the current file name and line number.
  m_PrevFileName = m_FileNameStack.Peek();
  m_PrevLineNumber = m_LineNumberStack.Peek();

  // This is initialized to work with the explicit continuation
  //   processing code.  The backslask will be stripped.
  gText line = '\\';
  
  gText errorMsg;

  char c = 0;
  bool quote = false;
  bool error = false;
  int bracket = 0;
  bool continuation = false;

  while (line.Right(1) == '\\' && !error) {
    // Strip the trailing backslash
    line = line.Left(line.Length() - 1);

    while (m_StartupString.Length () > 0 || !m_InputStack.Peek()->eof()) {
      GetChar(c);
      
      if (m_StartupString.Length() > 0 || !m_InputStack.Peek()->eof()) {
	// Ignore CR or replace with space as appropriate.
	if (!EOL(c))
	  line += c;
	else {
	  if (line.Right(1) == '\\') {
	    break;
	  }
	  else if (bracket > 0) {
	    line += '\n';
	  }
	  else {
	    line += '\n';
	    break;
	  }
	}

	if (c == '[') {
	  // Turn off prompts until the bracket is matched.
	  if (bracket == 0)
	    SetPrompt(false);	  
	  ++bracket;
	}
	else if (c == ']') {
	  // If brackets are matched, turn prompt back on.
	  --bracket;
	  if (bracket == 0)
	    SetPrompt(true);	  

	  if (bracket < 0)
	    bracket = 0;
	}
	else if (c == '\"') {
	  // In the middle of a string; accept everything
	  //   until the closing quote mark is found.
	  SetPrompt(false);	  
	  quote = !quote;
	  while ((!m_InputStack.Peek()->eof() || m_StartupString.Length() > 0)
		 && quote) {
	    GetChar(c);
	    line += c;

	    if (c == '\"' && !IsQuoteEscapeSequence(line))
	      quote = !quote;
	  }
	  SetPrompt(true);	  
	}

	if (line.Right(2) == "//") {
	  // In a line comment.  Ignore everything until
	  //   end of line.
	  line = line.Left(line.Length() - 2);

	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length() > 0) && !EOL(c))
	    GetChar(c);

	  line += '\n';
	}
	else if (line.Right(2) == "/*") {
	  // In a C style comment.  Ignore everything until
	  //   comment closing is found.
	  line = line.Left(line.Length() - 2);

	  SetPrompt(false);
	  gText comment = "  ";
	  while ((!m_InputStack.Peek()->eof() || m_StartupString.Length() > 0)
		 && (comment.Right(2) != "*/" || quote)) {
	    GetChar(c);
	    comment += c;

	    if (c == '\"') {
	      bool escapeSeq = false;
	      if (quote)
		escapeSeq = IsQuoteEscapeSequence(comment);
	      if (!escapeSeq)
		quote = !quote;
	    }
	  }
	  SetPrompt(true);
	}

	//
	// Handle Include[] directives
	//
	else if (line.Right(7) == "Include") {
	  line = line.Left(line.Length() - 7);
	  c = ' ';
	  
	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length() > 0) && c == ' ')
	    GetChar(c);

	  if (c != '[') {
	    line += "False;";
	    errorMsg = "Include[] syntax error; opening '[' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length() > 0) && c == ' ')
	    GetChar(c);
	  if (c != '\"') {
	    line += "False;";
	    errorMsg = "Include[] syntax error; opening '\"' not found";
	    error = true;
	    break;
	  }

	  gText filename;
	  c = ' ';

	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length() > 0) && c != '\"' && !EOL(c)) {
	    GetChar(c);
	    if (c != '\"')
	      filename += c;
	  }

	  if (EOL(c)) {
	    line += "False;";
	    errorMsg = "Include[] syntax error; closing '\"' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length() > 0) && c == ' ')
	    GetChar(c);
	  if (c != ']') {
	    line += "False;";
	    errorMsg = "Include[] syntax error; closing ']' not found";
	    error = true;
	    break;
	  }

	  // bring in the rest of this line
	  gText restOfLine;
	  c = ' ';
	  while ((!m_InputStack.Peek()->eof() ||
		  m_StartupString.Length () > 0) && !EOL(c)) {
	    GetChar(c);
	    restOfLine += c;
	  }

	  // note: filename might be changed after this call
	  gInput *input = LoadInput(filename);
	  
	  if (input) {
	    line += "True;";
	    m_InputStack.Push(input);
	    m_FileNameStack.Push(filename);
	    m_LineNumberStack.Push(1);
	  }
	  else {
	    line += "False;";
	    m_environment.ErrorStream() << "GCL Warning: Include file \"" << filename;
	    m_environment.ErrorStream() << "\" not found.\n";
	  }

	  line += restOfLine;
	  break;
	  
	} // "Include"

	//
	// Handle GetPath directive
	//
	else if (line.Right(9) == "GetPath[]") {
	  line = line.Left(line.Length() - 9);
	  line += gText('"') + GetFileName() + gText('"');
	}
      }
    }

    // This outer loop deals with explicit line continuation characters
    if (line.Right( 1 ) == '\\') {
      if (!continuation)
	SetPrompt(false);
      continuation = true;
    }
  }

  if (continuation)
    SetPrompt(true);

  if (error) {
    m_environment.ErrorStream() << "GCL Error: " << errorMsg << '\n';
  }

  if (m_InputStack.Peek()->eof()) {
    if (m_InputStack.Depth() > 1) {
      delete m_InputStack.Pop();
    }
    else
      m_InputStack.Pop();
    m_FileNameStack.Pop();
    m_LineNumberStack.Pop();
  }

  return line;
}




