//
// guipb.h  -- a class which implements the playback of 
//             previously-recorded GUI actions.
//
// $Id$
//

#ifndef GUIPB_H
#define GUIPB_H

#include "gtext.h"
#include "wx.h"
#include "wxmisc.h"

// Loggable classes:
#include "gambit.h"

#include <stdio.h>
#include <assert.h>


// This is so other files can refer to the global 
// GuiPlayback object.

class GuiPlayback;
extern GuiPlayback gui_playback;


// Macros; these make accessing the logging state easier.

#define GUI_PLAYBACK gui_playback.IsPlayingBack()

#define GUI_READ_ARG(F,N) gui_playback.ReadArg(F,N)


/*
 *
 * The GUI playback class.
 *
 * The procedure for playing back GUI actions stored in a log file
 * is as follows:
 *
 * 1) When loggable objects are encountered in the program, pointers
 *    to these objects are stored in the GUI recorder database.  
 *    This happens whether or not playback is requested.
 *
 * 2) The log file is read in line by line.  Each line is parsed to
 *    determine which command relative to which object has been 
 *    logged.  The object is looked up in the database.  If it isn't
 *    found an exception is thrown.  If it is found a callback function
 *    specific to that object is called with the rest of the line as
 *    text arguments to it (the arguments are really a list of strings).
 *    This function executes the appropriate action and exits.
 *
 */

class GuiPlayback
{
private:
    static bool instantiated;  // Only one instance is permitted.
	static bool playing_back;  // Are we playing back now?
    FILE *fp;                  // File pointer.

    // Private methods.
    void PlaybackLine(const gText& line);
    void ExecuteCommand(const gText& object_name, const gText& command,
                        const gList<gText>& arglist);

public:
    // ================== Exception classes. ====================

    class FileNotFound : public gException   
    {
    public:
        virtual ~FileNotFound()   { }
        gText Description() const;
    };

    class FileNotOpen : public gException   
    {
    public:
        virtual ~FileNotOpen()   { }
        gText Description() const;
    };

    class InUse : public gException   
    {
    public:
        virtual ~InUse()   { }
        gText Description() const;
    };

    class InvalidInputLine : public gException   
    {
    public:
        virtual ~InvalidInputLine()   { }
        gText Description() const;
    };

    class InvalidInputField : public gException   
    {
    public:
        virtual ~InvalidInputField()   { }
        gText Description() const;
    };

    class ObjectNotFound : public gException   
    {
    public:
        virtual ~ObjectNotFound()   { }
        gText Description() const;
    };

    class UnknownObjectType : public gException   
    {
    public:
        virtual ~UnknownObjectType()   { }
        gText Description() const;
    };

    // ==========================================================

    // ------------ Constructor, destructor.
    GuiPlayback();
    ~GuiPlayback();

    // ------------ Methods.
    bool  IsPlayingBack() const { return GuiPlayback::playing_back; }
    void  Playback(const gText& filename);
	gText ReadArg(const gText& funcname, int location_in_func);

	// Read an argument from the log file.
	const gText& ReadArg();

    // ------------ Debugging.
    bool is_GuiPlayback() const;
    void GuiPlayback_hello() const;
};


#endif // GUIPB_H


