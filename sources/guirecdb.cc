//
// FILE: guirecdb.cc -- the GUI recorder database.
//

#include "guirecdb.h"
#include "stdio.h"  // for debugging

#include "glist.imp"
#include "hash.imp"


// Instantiate the Hashtable template class used in the
// GuiRecorderDatabase class.  The gList template is necessary 
// because the hash implementation uses gLists.

template class gList<GuiObject*>;
template class HashTable<gText, GuiObject*>;


// The global GuiRecorderDatabase object:
GuiRecorderDatabase gui_recorder_db;

// Initialize static class members.
bool GuiRecorderDatabase::instantiated = false;


// ----------------------------------------------------------------------
//                          GuiObject class
// ----------------------------------------------------------------------

// Debugging functions.

bool GuiObject::is_GuiObject() const
{
    return true;
}


void GuiObject::GuiObject_hello() const
{
    printf("instance of class GuiObject accessed at %x\n", 
           (unsigned int)this);
}


bool GuiObject::check() const
{
    assert((ID >= FIRST_GUI_ID) && (ID <= LAST_GUI_ID));

    // These initializers must be outside the switch statement for
    // some reason or g++ complains.

    switch (ID)
    {
    case GAMBIT_FRAME:
    {
        GambitFrame *gambit_frame = (GambitFrame *)object;
        assert(gambit_frame->is_GambitFrame());
        break;
    }

    case EFG_SHOW:
    {
        EfgShow* efgshow = (EfgShow *)object;
        assert(efgshow->is_EfgShow());
        break;
    }

    default:
        fprintf(stderr, "Unknown GuiObject type ID: %d\n", ID);
        return false;
    }

    return true;
}


void GuiObject::dump() const
{
    printf("GuiObject dump:\n");
    assert((ID >= FIRST_GUI_ID) && (ID <= LAST_GUI_ID));
    
    // These initializers must be outside the switch statement for
    // some reason or g++ complains.

    switch (ID)
    {
    case GAMBIT_FRAME:
    {
        printf("GAMBIT_FRAME object at %x\n", (unsigned int)object);
        GambitFrame *gambit_frame = (GambitFrame *)object;
        gambit_frame->GambitFrame_hello();
        break;
    }

    case EFG_SHOW:
    {
        printf("EFG_SHOW object at %x\n", (unsigned int)object);
        EfgShow *efg_show = (EfgShow *)object;
        efg_show->EfgShow_hello();
        break;
    }

    default:
        fprintf(stderr, "Unknown GuiObject type ID: %d\n", ID);
    }
}


// ----------------------------------------------------------------------
//                  GuiRecorderDatabase exception classes
// ----------------------------------------------------------------------

gText GuiRecorderDatabase::DatabaseInUse::Description(void) const
{
    return "GuiRecorderDatabase is already in use.";
}


// ----------------------------------------------------------------------
//                  GuiRecorderDatabase methods
// ----------------------------------------------------------------------



int GuiRecorderDatabase::NumBuckets(void) const
{ 
    return 50; 
}


int GuiRecorderDatabase::Hash(const gText& key) const
{ 
    int sum = 0;

    // Sum the character values of the string; use
    // this value mod 50 as the hash value.

    for (int i = 0; i < key.Length(); i++)
        sum += (int)(key[i]);

    assert(sum >= 0);
    return (sum % 50); 
}


void GuiRecorderDatabase::DeleteAction(GuiObject* value)
{ 
    assert(value != NULL);
    delete value; 
}


GuiRecorderDatabase::GuiRecorderDatabase()
{
    // Is there an instance already?  If so, abort.
    if (instantiated)
    {
        throw DatabaseInUse();
    }

    Init();
    instantiated = true;
}


GuiRecorderDatabase::~GuiRecorderDatabase()
{ 
    Flush(); 
    instantiated = false;
}


// Debugging functions.

bool GuiRecorderDatabase::is_GuiRecorderDatabase() const
{
    return true;
}


void GuiRecorderDatabase::GuiRecorderDatabase_hello() const
{
    printf("instance of class GuiRecorderDatabase accessed at %x\n", 
           (unsigned int)this);
}


bool GuiRecorderDatabase::check() const
{
    //printf("checking GuiRecorderDatabase...\n");

    // Go through all keys in the database and 
    // check the associated GuiObjects.

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
        return true;
    }

    for (int i = 0; i < nb; i++)
    {
        const gList<gText>& keylist = keys[i];

        for (int j = 0; j < keylist.Length(); j++)
        {
            int index = j + 1;  // $%*#@$^ 1-based indexing!
            const gText& current_key = keylist[index];
            GuiObject *gui_object = operator()(current_key);
            gui_object->check();
        }
    }

    return true;
}


void GuiRecorderDatabase::dump() const
{
    // Go through all keys in the database and 
    // dump the associated GuiObjects.

    printf("GuiRecorderDatabase dump:\n");
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
            printf("\tkey[%d][%d] = %s\n", i, index, (char *)current_key);
            GuiObject *gui_object = operator()(current_key);
            gui_object->dump();
        }
    }
}
