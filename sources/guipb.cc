//
// FILE: guipb.cc -- the GUI playback class.
//
// $Id$
//

#include "glist.h"
#include "guirecdb.h"
#include "guipb.h"

#include "stdlib.h"

#define MAX_LINELENGTH 128  // FIXME: This should really be somewhere else.


// The global GuiPlayback object:
GuiPlayback gui_playback;

// Initialize static class members.
bool GuiPlayback::instantiated = false;
bool GuiPlayback::playing_back = false;


// ----------------------------------------------------------------------
//                  string utilities
// ----------------------------------------------------------------------

gText strip_whitespace(const gText& s)
{
    // Remove leading and trailing whitespace from a string s
    // and return the result.

    if (s.Length() == 0)
        return s;

    int first = 0;            // first non-whitespace character
    int last  = s.Length();   // one past the last non-whitespace character

    // Find first non-whitespace character.

    for (int i = 0; i < s.Length(); i++)
    {
        if (s[i] == ' ' || s[i] == '\t')
            first++;
        else // non-whitespace
            break;
    }

    // Find last non-whitespace character.  
    // "Whitespace" here includes newlines.

    for (int i = s.Length() - 1; i >= 0; i--)
    {
        if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n')
            last--;
        else
            break;
    }

    int len = last - first;

    if (last <= first) // no non-whitespace characters in string
    {
        return gText("");
    }

    // We have to add 1 to first because the Mid function subtracts 1, 
    // for some stupid reason.
    gText out = s.Mid(len, first + 1);

    return out;
}



// ----------------------------------------------------------------------
//                  GuiPlayback exception classes
// ----------------------------------------------------------------------

gText GuiPlayback::FileNotFound::Description(void) const
{
    return "The log file was not found.";
}

gText GuiPlayback::FileNotOpen::Description(void) const
{
    return "The log file is not open.";
}


gText GuiPlayback::InUse::Description(void) const
{
    return "A GuiPlayback object is already in use.";
}


gText GuiPlayback::InvalidInputLine::Description(void) const
{
    return "An invalid input line was read from the log file.";
}


gText GuiPlayback::InvalidInputField::Description(void) const
{
    return "An invalid input field was read from the log file.";
}


gText GuiPlayback::ObjectNotFound::Description(void) const
{
    return "A GUI object from the log file was not found in the database.";
}


gText GuiPlayback::UnknownObjectType::Description(void) const
{
    return "Unknown GUI object type in log file.";
}


// ----------------------------------------------------------------------
//                        GuiPlayback methods
// ----------------------------------------------------------------------

// Constructor, destructor.

GuiPlayback::GuiPlayback()
{
    // Is there an instance already?  If so, abort.
    if (instantiated)
    {
        throw InUse();
    }

    fp = NULL;
    instantiated = true;
}


GuiPlayback::~GuiPlayback()
{ 
    if (fp != NULL)
    {
        fclose(fp);
    }

    instantiated = false;
}


// Playback methods.

void GuiPlayback::Playback(const gText& filename)
{
#ifdef GUIPB_DEBUG
    printf("playing back file: %s\n", (char *)filename);
#endif

    char linebuf[MAX_LINELENGTH];
    gText line;

    fp = fopen((char *)filename, "r");

    if (fp == NULL)
    {
        throw FileNotFound();
    }

    // Read in the file line-by-line and execute each line.

    GuiPlayback::playing_back = true;

    while (1)
    {
        char *result = fgets(linebuf, MAX_LINELENGTH, fp);

        if (result == NULL) // End of file or error.
            break;

        line = linebuf;
        PlaybackLine(line);
    }

    GuiPlayback::playing_back = false;
}


//
// Play back a single line from the log file.
//

void GuiPlayback::PlaybackLine(const gText& line)
{
#ifdef GUIPB_DEBUG
    // Note that line ends with a newline so we don't
    // put another newline in here.
    printf("playing back line: %s", (char *)line);
#endif

    // -------------------------------------------------------
    // If the line is empty (just a newline), do nothing.
    // Otherwise, parse the line.  It should have the form:
    // OBJECT#INSTANCE_NUMBER, COMMAND [, arg1, arg2, ...]
    // -------------------------------------------------------

    // Low-level variables.
    char  linebuf[MAX_LINELENGTH];
    char *word;

    // High-level variables.
    gText object_name;
    gText class_name;
    gText command;
    gText argument;
    gList<gText> arglist;

    strcpy(linebuf, (char *)line);

    if (strcmp(linebuf, "\n") == 0)
        return;  // Empty line.

    // -------------------------------------------------------
    // Process the line.
    // The line consists of comma-separated fields.
    // -------------------------------------------------------

    assert(linebuf != NULL);

    // First, get the name of the object.

    word = strtok(linebuf, ",");

    if (word == NULL)  // No tokens found.
        throw InvalidInputLine();

    object_name = strip_whitespace(gText(word));

    if (object_name.Length() <= 0)
        throw InvalidInputField();

#ifdef GUIPB_DEBUG
    printf("object: %s\n", (char *)object_name);
#endif

    // Then get the command.

    word = strtok(NULL, ",");

    if (word == NULL)  // No more tokens.
        throw InvalidInputLine();

    command = strip_whitespace(gText(word));

    if (command.Length() <= 0)
        throw InvalidInputField();

#ifdef GUIPB_DEBUG
    printf("command: %s\n", (char *)command);
#endif

    // Get the list of arguments, if any.

    while(1)
    {
        word = strtok(NULL, ",");

        if (word == NULL)
            break;  // No more arguments.

        argument = strip_whitespace(gText(word));
        arglist.Append(argument);

#ifdef GUIPB_DEBUG
        printf("argument: %s\n", (char *)argument);
#endif
    }

    // Now execute the command.

    ExecuteCommand(object_name, command, arglist);
}


//
// Read in a single argument from a separate line in the log file,
// and return the argument.  Also check that the argument was read
// in the correct location.
//
// FIXME: make it so we can attach string descriptors to thrown
//        exceptions, specifically InvalidInputLine (or add a new
//        one called InvalidArg and/or InvalidArgLine).
//

gText GuiPlayback::ReadArg(const gText& funcname, 
						  int location_in_func)
{
#ifdef GUIPB_DEBUG
    printf("in GuiPlayback::ReadArg...\n");
    printf("funcname: %s\n", (char *)funcname);
    printf("location in func: %d\n", location_in_func);
#endif

    // Low-level variables.
    char  linebuf[MAX_LINELENGTH];
    char *word;

    // High-level variables.
    gText firstarg, secondarg, lastarg;
    int   funcloc;
    
    // Read in a new line.

    if (fp == NULL)
        throw FileNotOpen();

    char *result = fgets(linebuf, MAX_LINELENGTH, fp);

    if (result == NULL) // End of file or error.
        throw InvalidInputLine();  // FIXME: change to MissingInputLine()

#ifdef GUIPB_DEBUG
    printf("reading line: %s\n", linebuf);
#endif

    // Process the line.
    // The line consists of comma-separated fields.

    if (strcmp(linebuf, "\n") == 0)
        throw InvalidInputLine();  


    // First check that the line is being played back in the 
    // correct function, at the correct location. 

    assert(linebuf != NULL);

    // The first argument should be "...", which means that this is a
    // continuation of a previous command.

    word = strtok(linebuf, ",");

    if (word == NULL)  // No tokens found.
        throw InvalidInputLine();

    firstarg = strip_whitespace(gText(word));

    if (firstarg != "...")
        throw InvalidInputLine();


    // The next argument should be the name of the function where playback
    // should be happening.

    word = strtok(NULL, ",");

    if (word == NULL)  // No more tokens.
        throw InvalidInputLine();

    secondarg = strip_whitespace(gText(word));

    if (secondarg != funcname)
        throw InvalidInputLine();

#ifdef GUIPB_DEBUG
    printf("GuiPlayback::ReadArg: function is OK.\n");
#endif

    // The next argument should be the location within the function where
    // playback should be happening.

    word = strtok(NULL, ",");

    if (word == NULL)  // No more tokens.
        throw InvalidInputLine();

    funcloc = atoi(word);

    if (funcloc != location_in_func)
        throw InvalidInputLine();

#ifdef GUIPB_DEBUG
    printf("GuiPlayback::ReadArg: location in function is OK.\n");
#endif

    // If everything is OK, read the actual argument and return it
    // as a string.  The receiving function can convert it to whatever
    // it likes.

    word = strtok(NULL, ",");

    if (word == NULL)  // No more tokens.
        throw InvalidInputLine();

    lastarg = strip_whitespace(gText(word));
    
#ifdef GUIPB_DEBUG
    printf("leaving GuiPlayback::ReadArg...\n");
#endif

    return lastarg;
}



void GuiPlayback::ExecuteCommand(const gText& object_name, 
                                 const gText& command,
                                 const gList<gText>& arglist)
{
#ifdef GUIPB_DEBUG
    printf("in GuiPlayback::ExecuteCommand...\n");

    printf("object_name: %s\n", (char *)object_name);
    printf("command: %s\n", (char *)command);

    for (int i = 1; i <= arglist.Length(); i++)
        printf("arglist[%d] = %s\n", i, (char *)arglist[i]);
#endif

    // Check that the object actually exists in the (global) database.
    // WARNING! Hash class both doesn't do proper error handling and
    //          returns an "illegal value" which is never set anywhere,
    //          can't be checked as a result, and can't be overridden 
    //          by subclasses!  FIXME!

    if (gui_recorder_db.IsDefined(object_name) == 0) // This shouldn't be necessary.
        throw ObjectNotFound();

    GuiObject *object = gui_recorder_db(object_name);

    // Separate the object name from the object number.  The object name is
    // whatever comes before the last '#' and the number is whatever comes
    // after.  The object name should never contain a '#' character.

    // FIXME! Must make a local copy of object_name because LastOccur isn't
    // a const function!

    gText obj(object_name);  
    int length   = obj.Length();
    int position = obj.LastOccur('#');

    if (position == -1) // '#' not found
    {
        throw InvalidInputField();
    }

    position--;  // Move back behind the '#'.

    assert((length >= 0) && (position >= 0) && (position <= length));

    gText object_type = obj.Left(position);                // CHECKME: memory leak?
    gText num         = obj.Right(length - position - 1);  // CHECKME: memory leak?

#ifdef GUIPB_DEBUG
    int number = atoi((char *)num);
    printf("object type: %s\n", (char *)object_type);
    printf("number: %d\n", number);
#endif

    // Call the object's logging command.

    object->ExecuteLoggedCommand(command, arglist);
}


// Debugging functions.

bool GuiPlayback::is_GuiPlayback() const
{
    return true;
}


void GuiPlayback::GuiPlayback_hello() const
{
    printf("instance of class GuiPlayback accessed at %x\n", 
           (unsigned int)this);
}


