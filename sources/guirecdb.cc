//
// FILE: guirecdb.cc -- the GUI recorder database.
//
// $Id$
//

#include "guirecdb.h"
#include "stdio.h"  // for debugging
#include "guiobj.h"

#include "glist.imp"
#include "ghash.imp"


// Instantiate the HashTable template class used in the
// GuiRecorderDatabase class.  The gList template is necessary 
// because the hash implementation uses gLists.

template class gList<GuiObject*>;
template class HashTable<gText, GuiObject*>;


// The global GuiRecorderDatabase object:
GuiRecorderDatabase *gui_recorder_db;

// Initialize static class members.
bool GuiRecorderDatabase::instantiated = false;


// ----------------------------------------------------------------------
//                  GuiRecorderDatabase exception classes
// ----------------------------------------------------------------------

gText GuiRecorderDatabase::DatabaseInUse::Description(void) const
{
    return "The unique GuiRecorderDatabase object is already in use.";
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


GuiRecorderDatabase::GuiRecorderDatabase(void)
  : HashTable<gText, GuiObject *>(50)
{
    // Is there an instance already?  If so, abort.
    if (instantiated)
    {
        throw DatabaseInUse();
    }

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
        }
    }
}
