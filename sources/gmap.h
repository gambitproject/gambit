//#
//# FILE: gmap.h -- Declaration of Map container types
//#
//# $Id$
//#

#ifndef GMAP_H
#define GMAP_H

#include "basic.h"
#include "gambitio.h"
#include "gmessage.h"

typedef enum { DEFINE, REMOVE, FLUSH } gMapMessageType;
//
// <category lib=glib sect=Containers>
//
// This class contains the update information passed between a BaseMap and its
// iterators.
//
template <class K, class T> class gBaseMapMessage : public gMessage  {
  friend class gBaseMap<K, T>;
  friend class gBaseMapIter<K, T>;

  private:
    gMapMessageType mod_type;
    int mod_position;

       //
       // Construct a message with the appropriate information.
       // <note> This is declared private since only a gBaseMap should
       //        create a message.
       //
    gBaseMapMessage(int pos, gMapMessageType t);

  public:
       //
       // Return the type of the update
       //
    int Type(void) const;

       //
       // Determine if two messages are equal (identical)
       //
    int operator==(const gMessage &m) const;
};

template <class K, class T> class gBaseMapIter;

//
// <category lib=glib sect=Containers>
//
// The gBaseMap and its derived classes implement associative arrays --
// that is, arrays which are indexed not necessarily by consecutive
// integers but by some arbitrary keys.
//
// The specification and implementation is inspired in part by the AIPS
// library, but the coding is all our own.
//
// This is the abstract class from which all Map classes are derived
//
template <class K, class T> class gBaseMap : public gSender {
  friend class gBaseMapIter<K, T>;
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

template <class K, class T> class gOrdMapIter;
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
  friend class gOrdMapIter<K, T>;
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

//
// <category lib=glib sect=Containers>
//
// The gSparseSet implements a gSet in which the elements need not be
// sequentially numbered.  It is implemented as a gOrdMap in which the
// key is an int.
//
// <note> This class implements functionality similar to the (now obsolescent)
//        gMap class
//
template <class T> class gSparseSet : public gOrdMap<int, T>  {
  public:
//
// Construct a sparse set with no mappings defined.
//
    gSparseSet(const T &d);
//
// Construct a sparse set with the same mappings as another sparse set
//
    gSparseSet(const gSparseSet<T> &s);

//
// Return the least integer greater than zero for which no mapping is defined
//
    int FirstVacancy(void) const;
};


//
// <category lib=glib sect=Containers>
//
// The gUnordMap implements a map in which no ordering is defined on the
// key class.  No uniqueness constraints are imposed on the keys; however,
// if multiple identical keys exist in the map, the operations will only
// locate and manipulate the first instance in the map
//
template <class K, class T> class gUnordMap : public gBaseMap<K, T>  {
  public:
//
// Construct an unordered map with no mappings
//
    gUnordMap(const T &d);
//
// Construct an unordered map with the same mappings as another map
//
    gUnordMap(const gUnordMap<K, T> &m);

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
// Return nonzero when there is a mapping defined for the key
//
    int IsDefined(const K &key) const;

//
// Define a key-value mapping.  If a mapping is already defined for the key,
// a new mapping will be created; however, it will be hidden from access
// to the member functions until all earlier instances of mappings for
// that key are removed.
//
    void Define(const K &key, const T &value);
//
// Remove the mapping for the key, and return the value to which the key
// was formerly mapped.  If no mapping exists for the key, the function
// returns the default value and does not modify the map.  If multiple
// mappings exist for the key, only the first mapping encountered is affected,
// and all other mappings for that key remain unaltered.
//
    T Remove(const K &key);
};

#ifdef __BORLANDC__
#include "gmap.imp"
#endif   // __BORLANDC__


#endif   // GMAP_H


