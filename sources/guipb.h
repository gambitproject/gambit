//
// guipb.h  -- a class which implements the playback of 
//             previously-recorded GUI actions.
//

#ifndef GUIPB_H
#define GUIPB_H

#include "gtext.h"

// Loggable classes:
#include "gambit.h"
#include "efgshow.h"

#include <stdio.h>
#include <assert.h>

// This is so other files can refer to the global 
// GuiPlayback object.

class GuiPlayback;
extern GuiPlayback gui_playback;

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

// Q: should the callback function for a given class be located in the
//    class or outside it?  Try outside for now i.e. in this class.

class GuiPlayback
{
private:
    static bool instantiated;  // Only one instance is permitted.
    FILE *fp;                  // File pointer.

    // Private methods.
    void PlaybackLine(const gText& line);
    void ExecuteCommand(const gText& object_name, const gText& command,
                        const gList<gText>& arglist);

    // Private callback methods specific to a particular class.
    void GuiPlayback::ExecuteGambitFrameCommand(GambitFrame *object,
                                                const gText& command,
                                                const gList<gText>& arglist);

    void GuiPlayback::ExecuteEfgShowCommand(EfgShow *object,
                                            const gText& command,
                                            const gList<gText>& arglist);

public:
    // ================== Exception classes. ====================

    class FileNotFound : public gException   
    {
    public:
        virtual ~FileNotFound()   { }
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

    class InvalidCommandForObject : public gException   
    {
    public:
        virtual ~InvalidCommandForObject()   { }
        gText Description() const;
    };

    // ==========================================================

    // Constructor, destructor.
    GuiPlayback();
    ~GuiPlayback();

    // Methods.
    void Playback(const gText& filename);

    // Debugging.
    bool is_GuiPlayback() const;
    void GuiPlayback::GuiPlayback_hello() const;
};


#endif // GUIPB_H


