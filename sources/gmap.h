//
// FILE: gmap.h -- Declaration of Map container types
//
// $Id$
//

#ifndef GMAP_H
#define GMAP_H

#include "gmisc.h"
#include "gstream.h"

template <class K, class T> class gBaseMap  {
  protected:
    int length;
    T _default;
    K *keys;
    T *values;

//
// Insert a new key-value pair at a location in the arrays.
//
    T &Insert(const K &key, int where, const T &value);
//
// Remove the key-value pair at a location in the arrays, and return the
// value which was removed.
//
    T Delete(int where);

  public:
//
// This is the basic map constructor.  It initializes the map to be the
// empty map with no relations defined.
//
    gBaseMap(const T &d);
//
// Construct a map to have the same set of relations as another map.
// 
    gBaseMap(const gBaseMap<K, T> &);

//
// This is the map destructor.  It deletes all allocated memory, and calls
// the destructors for the keys and values which remain in the map at the
// time of its deallocation.
//
    virtual ~gBaseMap(); 

//
// These implement the mapping function which maps a key to a value.  If
// the map from a key to a value is not defined, a mapping will be defined
// from the key to the default value.  The IsDefined() member can be used
// to determine whether a mapping is defined.
//
// <note> If the mapping is not defined for the key in the const map case,
//        the mapping returns the default value and no entry is created in
//        the map for that key.
//+grp
    virtual T &operator()(const K &key) = 0;
    virtual T operator()(const K &key) const = 0;
//-grp

    virtual T *const Lookup(const K &key) = 0;

//
// These are the equality and assignment operators for this and all derived
// classes.
//+grp
    int operator==(const gBaseMap &M) const;
    int operator!=(const gBaseMap &M) const;
    gBaseMap<K, T> &operator=(const gBaseMap &M);
//-grp

//
// Returns the default value for the map
//+grp    
    T &Default(void);
    const T &Default(void) const;
//-grp

//
// Returns the number of mappings defined in the map
//
    int Length(void) const;

//
// Returns nonzero if the key has a mapping defined in the map
//
    virtual int IsDefined(const K &key) const = 0;

//
// These member functions implement adding and removing mapping from the map
//+grp
    virtual void Define(const K &key, const T &value) = 0;
    virtual T Remove(const K &key) = 0;
//-grp

//
// Prints out the contents of a map (for debugging purposes)
//
    void Dump(gOutput &) const;
};

//
// <category lib=glib sect=Containers>
//
// The gOrdMap is an ordered map.  That is, the index class has all the
// usual ordering operators defined upon it (==, !=, <, <=, >, >=).  These
// are used to sort the map by keys, thus making search-type operations
// logarithmic instead of linear.  This is a particularly large improvement
// when using keys which are costly to compare
//
template <class K, class T> class gOrdMap : public gBaseMap<K, T>  {
  private:
    int Locate(const K &key) const;

  public:
//
// Construct an ordered map with no mappings and the given default value.
//
    gOrdMap(const T &d);
//
// Construct an ordered map with the same key-value mappings as another
// ordered map.
//
    gOrdMap(const gOrdMap<K, T> &m);

    virtual ~gOrdMap();

//
// These implement the mapping function which maps a key to a value.  If
// the map from a key to a value is not defined, a mapping will be defined
// from the key to the default value.  The IsDefined() member can be used
// to determine whether a mapping is defined.
//
// <note> If the mapping is not defined for the key in the const map case,
//        the mapping returns the default value and no entry is created in
//        the map for that key.
//+grp
    T &operator()(const K &key);
    T operator()(const K &key) const;
//-grp

    T *const Lookup(const K &key);

//
// Return nonzero exactly when the key has a defined mapping in the map
//
    int IsDefined(const K &key) const;

//
// Define a new key-value relation.  If the key already exists in the map,
// the new value overwrites the old value; otherwise, a new relation is
// created.
//
    void Define(const K &key, const T &value);

//
// Remove the mapping for a key from the relation, and return the value
// to which the key was formerly mapped.  If the key does not have a defined
// mapping, has no effect on the contents of the map, and returns the
// 
    T Remove(const K &key);
};

#endif   // GMAP_H


