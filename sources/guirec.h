//
// FILE: guirec.h  -- interface of class for recording gui actions
//                    as commands into a log file.
//
// $Id$
//

#ifndef GUIREC_H
#define GUIREC_H

#include "gstream.h"  
#include "gtext.h"
#include "gmisc.h"

#include <assert.h>


// This is so other files can refer to the global GuiRecorder object.

class GuiRecorder;
extern GuiRecorder gui_recorder;

// Macros; these make accessing the logging state easier.

#define GUI_RECORDING gui_recorder.IsRecording()

// Write a line of the form: "object, command" to the log file with
// a newline at the end.
#define GUI_RECORD(X) \
if (gui_recorder.IsRecording()) { gui_recorder.writeToFile_newline(get_log_name(), X); }

// Make an "argument line" for a command that is being logged.
#define GUI_RECORD_ARG(X, Y, Z) \
if (gui_recorder.IsRecording()) { gui_recorder.writeArgToFile(get_log_name(), X, Y, Z); }

// Close the log file.
#define GUI_RECORDER_CLOSE if (gui_recorder.IsRecording()) { gui_recorder.closeFile(); }


// 
// This class is only meant to have one instance.  
//

class GuiRecorder
{
private:
    static bool recording;  // Is a file being logged now?
    gText filename;
    gFileOutput *file;
    
public:
    // ================== Exception classes. ====================

    class FileInUse : public gException   
    {
    public:
        virtual ~FileInUse()   { }
        gText Description(void) const;
    };
    
    class NoFileToClose : public gException   
    {
    public:
        virtual ~NoFileToClose() { }
        gText Description(void) const;
    };
    
    class NoFileToWriteTo : public gException   
    {
    public:
        virtual ~NoFileToWriteTo() { }
        gText Description(void) const;
    };
    
    class MultipleGuiRecorderObjects : public gException   
    {
    public:
        virtual ~MultipleGuiRecorderObjects() { }
        gText Description(void) const;
    };

    class InvalidLogFileName : public gException
    {
    public:
        virtual ~InvalidLogFileName() { }
        gText Description(void) const;
    };
    
    // ============== Constructors, destructors. ================

    GuiRecorder();
    ~GuiRecorder();

    // =================== Methods. =============================

    bool IsRecording() const { return GuiRecorder::recording; }

    void openFile(const char *name);
    void closeFile();

    // Writing log commands to the log file.
    void writeToFile(const gText& object, const gText& command);
    void writeArgsToFile(const gText& args);
    void writeToFile_newline(const gText& object, const gText& command);
    void writeArgsToFile_newline(const gText& args);

	// Write a single argument to the log file.
    void writeArgToFile(const gText& object, 
						const gText& funcname,
						int location_in_func,
						const gText& arg);
};

#endif  // GUIREC_H




