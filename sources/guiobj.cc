//
// FILE: guiobj.cc -- a mix-in class for GUI objects.
//
// $Id$
//

#include "stdio.h"  // for debugging
#include "glist.imp"
#include "ghash.imp"
#include "guiobj.h"
#include "guirecdb.h"


// ======================================================================
//
// GuiNames class:
//
// ======================================================================


// Instantiate the HashTable template class used in the
// GuiNames class.  The gList template is necessary 
// because the hash implementation uses gLists.

template class HashTable<gText, int>;


// Initialize static class members.
// The GuiNames object is a static member of the GuiObject class.

bool GuiNames::instantiated = false;
GuiNames GuiObject::gui_names;


// ----------------------------------------------------------------------
// GuiNames exception classes
// ----------------------------------------------------------------------

gText GuiNames::InUse::Description(void) const
{
    return "The unique GuiNames object is already in use.";
}


// ----------------------------------------------------------------------
// GuiNames methods
// ----------------------------------------------------------------------

int GuiNames::NumBuckets(void) const
{ 
    return 50; 
}


int GuiNames::Hash(const gText& key) const
{ 
    int sum = 0;

    // Sum the character values of the string; use
    // this value mod 50 as the hash value.

    for (int i = 0; i < key.Length(); i++)
        sum += (int)(key[i]);

    assert(sum >= 0);
    return (sum % 50); 
}


GuiNames::GuiNames(void)
  : HashTable<gText, int>(50)
{
    // Is there an instance already?  If so, abort.
    if (instantiated)
    {
        throw InUse();
    }

    instantiated = true;
}


GuiNames::~GuiNames()
{ 
    Flush(); 
    instantiated = false;
}


// Debugging functions.

void GuiNames::dump() const
{
    // Go through all keys in the database and 
    // print the associated names.

    int num;

    printf("GuiNames dump:\n");
    int nb = NumBuckets();

    assert(nb > 0);

    const gList<gText> *keys = Key();
    int entries = 0;

    for (int i = 0; i < nb; i++)
    {
        entries += keys[i].Length();
    }

    assert(entries >= 0);

    if (entries == 0)
    {
        printf("\tdatabase is empty.\n");
        return;
    }

    for (int i = 0; i < nb; i++)
    {
        const gList<gText>& keylist = keys[i];

        //printf("\tbucket %d, length %d\n", i, keylist.Length());

        for (int j = 0; j < keylist.Length(); j++)
        {
            int index = j + 1;  // $%*#@$^ 1-based indexing!
            const gText& current_key = keylist[index];
            num = operator()(current_key);
            printf("\tkey[%d][%d] = %s\tvalue = %d\n", i, index, 
                   (char *)current_key, num);
        }
    }
}



// ======================================================================
//
// GuiObject class:
//
// ======================================================================

// Constructor

GuiObject::GuiObject(const gText& my_name) 
{
#ifdef GUIOBJ_DEBUG
    printf("in GuiObject constructor for %s\n", (char *)my_name);
#endif

    int num_instances;

    // See if my_name is in the names database.
    // If so, get the number of instances.
    // If not, store it with an instance count of 1.

    if (gui_names.IsDefined(my_name) == 0) 
    {
        num_instances = 1;   // First instance.
    }
    else
    {
        num_instances = gui_names(my_name);
        assert(num_instances > 0);
        num_instances++;
    }

    gui_names.Define(my_name, num_instances);

#ifdef GUIOBJ_DEBUG
    gui_names.dump();
#endif

    // Store the GuiLogName.  This will be used upon playback to
    // get a pointer to the object.

    GuiLogName = my_name + gText("#") + ToText(num_instances);

    // Put the object's pointer into the database.
    // Make sure it isn't already there.

    if (gui_recorder_db.IsDefined(GuiLogName) == 0)
    {
        gui_recorder_db.Define(GuiLogName, this); 
    }
    else  // The object is already there!
    {
        throw InUse();
    }
}



// Destructor

GuiObject::~GuiObject()
{ 
#ifdef GUIOBJ_DEBUG
    gui_recorder_db.dump();
    printf("Removing GUI object %s from recorder database.\n", (char *)GuiLogName);
#endif
    // Remove this object from the global gui object database.
    gui_recorder_db.Remove(GuiLogName);
#ifdef GUIOBJ_DEBUG
    gui_recorder_db.dump();
#endif
}



// Exception classes

gText GuiObject::InvalidCommand::Description(void) const
{
    return "Invalid command for object type in log file.";
}


gText GuiObject::InUse::Description(void) const
{
    return "Object has already been defined in the database.";
}


