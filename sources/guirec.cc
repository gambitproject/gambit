//
// FILE: guirec.cc -- implementation of class for recording gui actions
//                    as commands into a log file.
//
// $Id$
//

#include "guirec.h"

// The global GuiRecorder object:
GuiRecorder gui_recorder;  

// Initialize static class members.
bool GuiRecorder::recording = false;


// ----------------------------------------------------------------------
//                      Exception classes
// ----------------------------------------------------------------------

gText GuiRecorder::FileInUse::Description(void) const
{
    return "GuiRecorder log file is already in use.";
}

gText GuiRecorder::NoFileToClose::Description(void) const
{
    return "Attempted to close a non-existent GuiRecorder log file.";
}

gText GuiRecorder::NoFileToWriteTo::Description(void) const
{
    return "Attempted to write to a non-existent GuiRecorder log file.";
}

gText GuiRecorder::MultipleGuiRecorderObjects::Description(void) const
{
    return "Multiple GuiRecorder objects are not allowed.";
}

gText GuiRecorder::InvalidLogFileName::Description(void) const
{
    return "Log file names must end in \".log\".";
}


// ----------------------------------------------------------------------
//                      Constructor
// ----------------------------------------------------------------------

GuiRecorder::GuiRecorder()
{
    // This class should only have one instance.
    // You can technically define more than one
    // instance, but you can only have one open
    // log file at a time.

    if (GuiRecorder::recording == true)
    {
        throw MultipleGuiRecorderObjects();
    }  
}


// ----------------------------------------------------------------------
//                      Destructor
// ----------------------------------------------------------------------

GuiRecorder::~GuiRecorder()
{
    if (recording == true)
    {
        gerr << gText("Warning: GuiRecorder going out of scope.  Closing log file: ")
             << filename << gText("\n");
        closeFile();
    }
}


// ----------------------------------------------------------------------
//                      Methods
// ----------------------------------------------------------------------

// Open a new log file.

void GuiRecorder::openFile(const char *name)
{
    //
    // recording is a static variable; if it's true then
    // a GuiRecorder instance already exists.  Since we
    // only allow one instance we abort in this case.
    //
    
    if (GuiRecorder::recording == true)
    {
        throw FileInUse();
    }

    // Check that the log file name ends in .log

    int len = strlen(name);

    if (strcmp(name + len - 4, ".log") != 0)
        throw InvalidLogFileName();

#ifdef GUIREC_DEBUG
    printf("Opening file \"%s\" for logging.\n", name);
#endif
    
    file     = new gFileOutput(name, false);  // Open file for writing.
    filename = name;
    GuiRecorder::recording  = true;
}


// Close the log file.

void GuiRecorder::closeFile()
{
    if (GuiRecorder::recording == false)
    {
        throw NoFileToClose();
    }
    
#ifdef GUIREC_DEBUG
    printf("Closing log file for gui recording.\n");
#endif
    delete file;
    filename = "";
    GuiRecorder::recording = false;
}



// Write a line to the log file.

void GuiRecorder::writeToFile(const gText& object, const gText& command)
{
    if (GuiRecorder::recording == false)
    {
        throw NoFileToWriteTo();
    }
    
#ifdef GUIREC_DEBUG
    printf("Recording to log file: %s, %s\n", (char *)object, (char *)command);
#endif

    (*file) << object << gText(", ") << command << gText(", ");
}

// This is the same as the previous function except that it adds a
// newline at the end.  It's meant to be used for commands without
// argument lists.

void GuiRecorder::writeToFile_newline(const gText& object, const gText& command)
{
    if (GuiRecorder::recording == false)
    {
        throw NoFileToWriteTo();
    }
    
#ifdef GUIREC_DEBUG
    printf("Recording to log file: %s, %s\n", (char *)object, (char *)command);
#endif

    (*file) << object << gText(", ") << command << gText("\n");
}


// This function appends one or more arguments to the last logged command.

void GuiRecorder::writeArgsToFile(const gText& args)
{
    if (GuiRecorder::recording == false)
    {
        throw NoFileToWriteTo();
    }
    
#ifdef GUIREC_DEBUG
    printf("Recording to log file: %s\n", (char *)args);
#endif

    (*file) << args << gText(", ");
}


// This function appends one or more arguments to the last logged command
// and adds a newline.

void GuiRecorder::writeArgsToFile_newline(const gText& args)
{
    if (GuiRecorder::recording == false)
    {
        throw NoFileToWriteTo();
    }
    
#ifdef GUIREC_DEBUG
    printf("Recording to log file: %s\n", (char *)args);
#endif

    (*file) << args << gText("\n");
}


