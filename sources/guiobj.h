//
// FILE: guiobj.h -- a mix-in class for GUI objects.
//
// $Id$
//

#ifndef GUIOBJ_H
#define GUIOBJ_H

#include "gtext.h"
#include "ghash.h"


class GuiObject;

// ======================================================================
//
// GuiNames class:
//
// This is a special class to store GUI object names and numbers.
// The name is the "official" name for the GUI object class and 
// should be the same as the actual class name.  The number is the
// total number of objects of the class.  When initializing a new
// object, increment the number and use the new number (this corresponds
// to 1-based indexing).  The GuiNames class is a <gText, int> hash table.
// There must only be one instance if this class.
//
// ======================================================================

class GuiNames : public HashTable<gText, int>
{
private:
    static bool instantiated;  // Only one instance is permitted.
    int  Hash(const gText& ref) const;
    void DeleteAction(int value) { };

public:
    // ================== Exception classes. ====================
 
    class InUse : public gException   
    {
    public:
        virtual ~InUse()   { }
        gText Description() const;
    };
    
    // ==========================================================
 
    // Constructor, destructor.
    GuiNames();
    ~GuiNames();
 
    // Methods.
    int NumBuckets() const;
 
    // Debugging.
    void dump()  const;
};
 



// ======================================================================
//
// GuiObject class:
//
// ======================================================================

class GuiObject
{
private:
    // Class variable: a hash table of (name, number) pairs.
    static GuiNames gui_names;

    // Instance variables:
    gText GuiLogName;

public:
    // Constructor, destructor.

    GuiObject(const gText& my_name);
    virtual ~GuiObject();

    // Member functions.

    const gText& get_log_name() { return GuiLogName; }
    // This function actually does the playback:
    virtual void ExecuteLoggedCommand(const gText& command,
                                      const gList<gText>& arglist) = 0;

    // Exception classes. 

    class InvalidCommand : public gException   
    {
    public:
        virtual ~InvalidCommand()   { }
        gText Description() const;
    };

    class InUse : public gException   
    {
    public:
        virtual ~InUse()   { }
        gText Description() const;
    };
};


#endif // GUIOBJ_H



