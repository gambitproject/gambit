//
// FILE: gpreproc.cc -- Implementation of gPreprocessor class,
//                      preprocessor for the command language input
//
// $Id$
//

#include "gpreproc.h"
#include "system.h"


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
  
  try   {
    return new gFileInput(IniFileName);
  }
  catch (gFileInput::OpenFailed &) {
    // I'm thinking there's a more elegant way than this.
    // But I don't know what it is.
    if (search)   {
      if (System::GetEnv("HOME") != NULL)   {
	IniFileName = ((gText) System::GetEnv("HOME") + 
		       (gText) SLASH + (gText) p_name);
	try   {
	  infile = new gFileInput(IniFileName);
	  p_name = IniFileName;
	  return infile;
	}
	catch (gFileInput::OpenFailed &) {
	  infile = 0;
	  // didn't find in home directory... fall on through
	}
      }
      if (System::GetEnv("GCLLIB") != NULL)  {
	IniFileName = ((gText) System::GetEnv("GCLLIB") + 
		       (gText) SLASH + (gText) p_name);
	try  {
	  infile = new gFileInput(IniFileName);
	  p_name = IniFileName;
	  return infile;
	}
	catch (gFileInput::OpenFailed &) {
	  infile = 0;
	  // didn't find it in GCLLIB directory either... keep going
	}
      }
      if (SOURCE != NULL)  {
	IniFileName = (gText) SOURCE + (gText) SLASH + (gText) p_name;
	try  {
	  infile = new gFileInput(IniFileName);
	  p_name = IniFileName;
	  return infile;
	}
	catch (gFileInput::OpenFailed &) {
	  // give up!
	  return 0;
	}
      }
      return 0;
    }
    else {  // don't search, just give up
      return 0;
    }
  }
}

void gPreprocessor::SetPrompt(bool p_prompt)
{
  if (m_InputStack.Peek() == m_CmdLine)
    m_CmdLine->SetPrompt(p_prompt);	  
}

void gPreprocessor::GetChar(char &p_char)
{
  if (m_StartupString.Length() > 0)  {
    p_char = m_StartupString[0];
    m_StartupString = m_StartupString.Right(m_StartupString.Length() - 1);
  }
  else {
    m_InputStack.Peek()->get(p_char);
    if (EOL(p_char))
      ++m_LineNumberStack.Peek();
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
    int i = 2;
    while (p_line.Length() >= i && p_line[p_line.Length() - i] == '\\') {
      backslash = !backslash;
      i++;
    }
  }
  return backslash;
}



gPreprocessor::gPreprocessor(gCmdLineInput *p_cmdline,
			     const char *p_cmd /* = NULL */) 
  : m_CmdLine(p_cmdline), m_PrevFileName("console"), m_PrevLineNumber(1),
    m_StartupString(p_cmd)
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
  if (eof())
    return "";

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

    while (!m_InputStack.Peek()->eof()) {
      GetChar(c);
      
      if (!m_InputStack.Peek()->eof()) {
	// Ignore CR or replace with space as appropriate.
	if (!EOL(c))
	  line += c;
	else {
	  if (bracket > 0) {
	    if (line.Right(1) == '\\')
	      line = line.Left(line.Length() - 1);
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
	  while (!m_InputStack.Peek()->eof() && quote) {
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

	  while (!m_InputStack.Peek()->eof() && !EOL(c))
	    GetChar(c);
	}
	else if (line.Right(2) == "/*") {
	  // In a C style comment.  Ignore everything until
	  //   comment closing is found.
	  line = line.Left(line.Length() - 2);

	  SetPrompt(false);
	  gText comment = "  ";
	  while (!m_InputStack.Peek()->eof() && 
		 (comment.Right(2) != "*/" || quote)) {
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
	  
	  while (!m_InputStack.Peek()->eof() && c == ' ')
	    GetChar(c);

	  if (c != '[') {
	    line += "False";
	    errorMsg = "Include[] syntax error; opening '[' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while (!m_InputStack.Peek()->eof() && c == ' ')
	    GetChar(c);
	  if (c != '\"') {
	    line += "False";
	    errorMsg = "Include[] syntax error; opening '\"' not found";
	    error = true;
	    break;
	  }

	  gText filename;
	  c = ' ';

	  while (!m_InputStack.Peek()->eof() && c != '\"' && !EOL(c)) {
	    GetChar(c);
	    if (c != '\"')
	      filename += c;
	  }

	  if (EOL(c)) {
	    line += "False";
	    errorMsg = "Include[] syntax error; closing '\"' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while (!m_InputStack.Peek()->eof() && c == ' ')
	    GetChar(c);
	  if (c != ']') {
	    line += "False";
	    errorMsg = "Include[] syntax error; closing ']' not found";
	    error = true;
	    break;
	  }

	  // bring in the rest of this line
	  gText restOfLine;
	  c = ' ';
	  while(!m_InputStack.Peek()->eof() && !EOL(c)) {
	    GetChar(c);
	    restOfLine += c;
	  }

	  // note: filename might be changed after this call
	  gInput *input = LoadInput(filename);
	  
	  if (input) {
	    line += "True";
	    m_InputStack.Push(input);
	    m_FileNameStack.Push(filename);
	    m_LineNumberStack.Push(1);
	  }
	  else {
	    line += "False";
	    gerr << "GCL Warning: Include file \"" << filename;
	    gerr << "\" not found.\n";
	  }

	  line += restOfLine;
	  break;
	  
	} // "Include"

	//
	// Handle GetPath directive
	//
	else if (line.Right(7) == "GetPath") {
	  bool text_found = false;
	  bool closed_bracket = false;
	  bool file = true;
	  bool path = true;

	  line = line.Left(line.Length() - 7);

	  c = ' ';
	  while (!m_InputStack.Peek()->eof() && c == ' ')
	    GetChar(c);

	  if (c != '[') {
	    line += "\"\"";
	    errorMsg = "GetPath[] syntax error; opening '[' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while (!m_InputStack.Peek()->eof() && c == ' ')
	    GetChar(c);
	  text_found = false;
	  if (c == 'T') { // True
	    text_found = ExpectText("rue");
	    if (text_found)
	      file = true;
	  }
	  else if (c == 'F') {
	    text_found = ExpectText("alse");
	    if (text_found)
	      file = false;
	  }
	  else if (c == ']') {
	    text_found = true;
	    closed_bracket = true;
	  }

	  if (!text_found) {
	    line += "\"\"";
	    errorMsg = "GetPath[] syntax error (1)";
	    error = true;
	    break;
	  }

	  if (!closed_bracket) {
	    c = ' ';
	    while (!m_InputStack.Peek()->eof() && c == ' ')
	      GetChar(c);
	    text_found = false;
	    if (c == ',') {
	      text_found = true;
	    }
	    else if (c == ']') {
	      text_found = true;
	      closed_bracket = true;
	    }
	    if (!text_found) {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error (2)";
	      error = true;
	      break;
	    }
	  }

	  if (!closed_bracket) {
	    c = ' ';
	    while (!m_InputStack.Peek()->eof() && c == ' ')
	      GetChar(c);
	    text_found = false;
	    if (c == 'T') { // True
	      text_found = ExpectText("rue");
	      if (text_found)
		path = true;
	    }
	    else if (c == 'F') {
	      text_found = ExpectText("alse");
	      if (text_found)
		path = false;
	    }
	    if (!text_found) {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error (3)";
	      error = true;
	      break;
	    }
	  }

	  if (!closed_bracket) {
	    c = ' ';
	    while (!m_InputStack.Peek()->eof() && c == ' ')
	      GetChar(c);
	    if (c != ']') {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error; closing ']' not found";
	      error = true;
	      break;
	    }
	  }

	  // now the file and path variables are defined,
	  //   determine return value
	  gText txt = GetFileName();
	  const char SLASH = System::Slash();

	  if (file && path) {
	  }
	  else if (file) {
	    if (txt.LastOccur(SLASH) > 0) 
	      txt = txt.Right(txt.Length() - txt.LastOccur(SLASH));
	  }
	  else if (path) {
	    if (txt.LastOccur(SLASH) > 0)
	      txt = txt.Left(txt.LastOccur(SLASH));
	    else
	      txt = "";
	  }
	  else
	    txt = "";
	  
	  line += (gText) '\"' + (gText) txt + (gText) '\"';

	} // "GetPath"
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

  if (error)
    gerr << "GCL Error: " << errorMsg << '\n';

  if (m_InputStack.Peek()->eof()) {
    if (m_InputStack.Depth() > 1)
      delete m_InputStack.Pop();
    else
      m_InputStack.Pop();
    m_FileNameStack.Pop();
    m_LineNumberStack.Pop();
  }

  return line;
}




