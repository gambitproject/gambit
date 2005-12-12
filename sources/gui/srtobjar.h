///////////////////////////////////////////////////////////////////////////////
// Name:        srtobjar.h
// Purpose:     "wxSortedObjectArray" macros 
// Author:      John Labenski
// Modified by: 
// Created:     1/08/2004
// RCS-ID:      
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SRTOBJAR_H__
#define __WX_SRTOBJAR_H__

// Note: there is no cpp file as all the code is contained in these macros.

#include "wx/dynarray.h"

// NOTE! This 2x faster than the pairarr for a class with int, int members, but 
//       over 2X slower for a class that has a different one of these as an array

// ============================================================================
// Provides macros for sorting your own class using wxSortedArray's binary
// search to insert/retrieve values. While this doesn't have the performance
// of a good hash table O(n), it is smaller and with a lookup speed of O(log(n))
// is suitable for some applications. You can use virtually any class that can
// be put into wxArrays so long as they have a default and copy constructor, 
// and = operator. You must define your own compare function.
// ----------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// DECLARE_SORTED_OBJARRAY is a sorted WX_DECLARE_OBJARRAY but for 
//   WX_DEFINE_SORTED_ARRAY that allows you to pass them as "const T&" rather 
//   than "T*" as WX_ARRAY does. It automatically creates new ones and deletes 
//   them as necessary. It also ensures that only one item per "index" exists.
//
// The object class to be sorted:
//   You must create a class of objects that need to be sorted (called T). 
//   The class must define a default and copy constructor, and = operator.
//   Additionally you must create a WX_DEFINE_SORTED_ARRAY of these objects
//
// You can use DECLARE_SORTED_OBJARRAY_BASE_CMP to easily extend the class 
//   without having to subclass it.
// ---------------------------------------------------------------------------
// DECLARE_SORTED_OBJARRAY(T, Tsortedarray, name, classexp)
//
// name(wxCMPFUNC_CONV sortfn) - constructor, takes the sort function
// void Copy(const name& other) - make a full copy of the other array
// bool Add(const T& item)  - add an item or change existing, returns true if new item
// bool AddAt(const T& item, size_t index)  - add an item at index (don't unsort it!)
// void Alloc(size_t count) - allocate momory for count items
// void Clear() - clear all the items
// void Empty() - clear all the items
// size_t GetCount() - get the number of items
// int Index(const T& item) - get the index of the item or wxNOT_FOUND
// int IndexForInsert(const T& item) - get the index that this item would be inserted at
// bool IsEmpty() - returns true is GetCount == 0
// T& Item(size_t index) - returns a reference to the item at index (be sure it exists)
// T& Last() - returns a reference to the last item (be sure GetCount() > 0)
// bool Remove(const T& item) - remove this item, returns true if it existed
// void RemoveAt(size_t index) - remove the item at this index
// void Shrink() - free any extra memory allocated to the array
// Tsortedarray& GetArray() - get the writable array
// const Tsortedarray& GetArray() const - get the const array
// name& operator=(const name& other) - copy the array
// T& operator[](size_t index) const  - get a reference to the item at index (be sure it exists)
//
// ---------------------------------------------------------------------------
// DECLARE_SORTED_OBJARRAY_INTUPDATEPOS and DEFINE_SORTED_OBJARRAY_INTUPDATEPOS
// This adds the function UpdatePos for shifting and deleting items with int keys
// You must make the int sort key accessible as either a member variable or 
//   int& GetKey() so it can be modified as a reference.
//
// bool UpdatePos(int pos, int numPos) - 
//   if numPos > 0 - shifts keys greater than pos by numPos 
//   if numPos < 0 - deletes keys between pos and pos-numPos, 
//     and shifts keys greater than by pos-numPos by -numPos
//
// ---------------------------------------------------------------------------
// Example: A sorted int key, int value class
// 
// // Create your object array of items that we'll sort on the "key"
// //   Needs default and const T& copy constructor and = operator only.
// class IntInt
// {
// public:
//     IntInt(const IntInt& i) : m_key(i.m_key), m_value(i.m_value) {}
//     IntInt(int key = 0, int value = 0) : m_key(key), m_value(value) {}
//     int m_key;   // the members of this class can be anything you want
//     int m_value;
//     IntInt& operator=(const IntInt& other) { m_key = other.m_key; m_value = other.m_value; return *this; }
// };
//
// // Create your function to sort the IntInt class anyway you like
// //   Make sure that the sort function uniquely sorts them.
// int wxCMPFUNC_CONV IntIntSort(IntInt* pItem1, IntInt* pItem2)
//     { return pItem1->m_key - pItem2->m_key; }
//
// // Create a wxSortedArray of the IntInt class which uses pointers
// //   to the IntInt items in the array. Remember you have manage the memory
// //   by ensuring that you delete them when they're removed with this array.
// WX_DEFINE_SORTED_ARRAY(IntInt*, SortedArrayIntInt);
//
// // Create the DECLARE_SORTED_OBJARRAY to have it manage the memory 
// //   and ensure that only one item per "index" exists.
// DECLARE_SORTED_OBJARRAY(IntInt, SortedArrayIntInt, SortedObjArrayIntInt, class)
//
// ------------------------------------
// // In some cpp file define the rest of the code.
// DEFINE_SORTED_OBJARRAY(IntInt, SortedArrayIntInt, SortedObjArrayIntInt)
//
// ------------------------------------
// // Now use your new class
// SortedObjArrayIntInt myIntIntSortedArray(IntIntSort);
//
// for (n=0; n < count; n++)             // add some items
//     myIntIntSortedArray.Add(IntInt(rnd.Get(), 1));
//
// // did we add this IntInt? Check for index != wxNOT_FOUND
// int index = myIntIntSortedArray.Index(IntInt(5 /*need only key*/));
// 
// // Let's see if the value exists, copy it out of the array and remove it
// IntInt ii;
// if (index != wxNOT_FOUND) { 
//     ii = myIntIntSortedArray[index];     // get the item
//     myIntIntSortedArray.RemoveAt(index); // remove the item
// }
//
// ------------------------------------
//
// // The UpdatePos functions are specialized for when you have int keys
// // We'll also use the XXX_CMP declaration so we have a default cmp function
// DECLARE_SORTED_OBJARRAY_INTUPDATEPOS_CMP(IntInt, SortedArrayIntInt, SortedObjArrayIntIntUpdate, IntIntSort, class)
// DEFINE_SORTED_OBJARRAY_INTUPDATEPOS_CMP(IntInt, SortedArrayIntInt, m_key, SortedObjArrayIntIntUpdate)
// SortedObjArrayIntIntUpdate myIntIntSortedArrayUpdate(/* default is IntIntSort */);
// // add some items...
//
// // Every item with m_key >= 5 will have 6 added to their m_keys, shifted
// myIntIntSortedArrayUpdate.UpdatePos(5, 6);
//
// // Every item with m_key between 7 and 7-12 will be deleted (if any)
// //   items with key > 7-12 will be shifted downward by 12
// myIntIntSortedArrayUpdate.UpdatePos(7, -12);
// ---------------------------------------------------------------------------

// Create a obj array of class "T" using wxSortedArray "Tsortedarray" 
//   called "name" and of class "classexp"
#define DECLARE_SORTED_OBJARRAY(T, Tsortedarray, name, classexp)                     \
DECLARE_SORTED_OBJARRAY_BASE_CMP(T, Tsortedarray, name, wxARRAY_EMPTY_CMP, classexp) \
};

// The same as DECLARE_SORTED_OBJARRAY, except by default use "cmpfunc" for
//   comparing the classes
#define DECLARE_SORTED_OBJARRAY_CMP(T, Tsortedarray, name, cmpfunc, classexp) \
DECLARE_SORTED_OBJARRAY_BASE_CMP(T, Tsortedarray, name, = cmpfunc, classexp)  \
};

// Create a obj array of class "T" using wxSortedArray "Tsortedarray" 
//   called "name" and of class "classexp"
#define DECLARE_SORTED_OBJARRAY_INTUPDATEPOS(T, Tsortedarray, name, classexp) \
DECLARE_SORTED_OBJARRAY_BASE_CMP(T, Tsortedarray, name, wxARRAY_EMPTY_CMP, classexp) \
    bool UpdatePos( int pos, int numPos ); \
};

// The same as DECLARE_SORTED_OBJARRAY_INTUPDATEPOS, except by default use "cmpfunc" for
//   comparing the classes
#define DECLARE_SORTED_OBJARRAY_INTUPDATEPOS_CMP(T, Tsortedarray, name, cmpfunc, classexp) \
DECLARE_SORTED_OBJARRAY_BASE_CMP(T, Tsortedarray, name, = cmpfunc, classexp) \
    bool UpdatePos( int pos, int numPos ); \
};

// Base version of the macro
#define DECLARE_SORTED_OBJARRAY_BASE_CMP(T, Tsortedarray, name, cmpfunc, classexp) \
classexp name                                                               \
{                                                                           \
public:                                                                     \
    typedef int (wxCMPFUNC_CONV *CMPFUNC)(T* pItem1, T* pItem2);            \
    name(CMPFUNC fn cmpfunc) : m_array(fn) {}                               \
    name(const name& other);                                                \
    name(const T& item, CMPFUNC fn cmpfunc) : m_array(fn) { Add(item); }    \
    name(T* item, CMPFUNC fn cmpfunc) : m_array(fn) { Add(item); }          \
    void Copy(const name& other);                                           \
    ~name() { Clear(); }                                                    \
    bool Add(const T& item);                                                \
    bool Add(T* item);                                                      \
    void AddAt(const T& item, size_t index);                                \
    void AddAt(T* item, size_t index);                                      \
    void Alloc(size_t count) { m_array.Alloc(count); }                      \
    void Clear();                                                           \
    void Empty() { Clear(); }                                               \
    size_t GetCount() const { return m_array.GetCount(); }                  \
    int Index(const T& item) const { return m_array.Index((T*)&item); }     \
    int Index(T* item) const { return m_array.Index(item); }                \
    int IndexForInsert(const T& item) const { return m_array.IndexForInsert((T*)&item); } \
    int IndexForInsert(T* item) const { return m_array.IndexForInsert(item); } \
    bool IsEmpty() const { return m_array.IsEmpty(); }                      \
    T& Item(size_t index) const { return *m_array.Item(index); }            \
    T& Last() const { return *m_array.Last(); }                             \
    bool Remove(const T& item);                                             \
    bool Remove(T* item);                                                   \
    void RemoveAt(size_t index);                                            \
    void Shrink() { m_array.Shrink(); }                                     \
    Tsortedarray& GetArray() { return m_array; }                            \
    const Tsortedarray& GetArray() const { return m_array; }                \
    name& operator=(const name& other) { Copy(other); return *this; }       \
    T& operator[](size_t index) const { return Item(index); }               \
    Tsortedarray m_array;                                                   \
    static T m_default;

// for Item() wxCHECK_MSG((index<GetCount()), m_default, wxT("Invalid index"));

// NB: The first name::Add using m_array.Add is faster than the second using
//     IndexForInsert() and AddAt(). I'm not sure why... probably 
//     some internal machine optimization. This is true for VC6 and gcc.
/*

void name::Add(const T &item) 
    int index = Index(item); 
    if (index == wxNOT_FOUND) m_array.Add(new T(item)); 
    else { Item(index) = item; } 

void name::Add(const T &item) 
    int n = m_array.IndexForInsert((T*)&item); 
    if (n == (int)m_array.GetCount()) 
        m_array.AddAt(new T(item), n); 
    else if (m_compare((T*)&item, m_array[n]) == 0) 
        *m_array[n] = item; 
    else 
        m_array.AddAt(new T(item), n); 
*/

// Alternate Add(T*)    T* i = m_array[index]; m_array.RemoveAt(index); delete i; m_array.Add(item); return false; 

#define DEFINE_SORTED_OBJARRAY(T, Tsortedarray, name) \
\
T name::m_default; \
\
name::name(const name& other) : m_array(other.m_array) \
{ \
    m_array.Clear(); Copy(other); \
} \
void name::Copy(const name& other) \
{ \
    Clear(); \
    size_t n, count = other.GetCount(); \
    if (!count) return; m_array.Alloc(count); \
    for (n = 0; n < count; n++) m_array.Add(new T(other[n])); \
} \
bool name::Add(const T& item) \
{ \
    int index = Index(item); \
    if (index == wxNOT_FOUND) { m_array.Add(new T(item)); return true; } \
    *m_array[index] = item; return false; \
} \
bool name::Add(T* item) \
{ \
    int index = m_array.Index(item); \
    if (index == wxNOT_FOUND) { m_array.Add(item); return true; } \
    T* i = m_array[index]; delete i; m_array[index] = item; return false; \
} \
void name::AddAt(const T& item, size_t index) \
{ \
    m_array.AddAt(new T(item), index); \
} \
void name::AddAt(T* item, size_t index) \
{ \
    m_array.AddAt(item, index); \
} \
void name::Clear() \
{ \
    size_t n, count = m_array.GetCount(); if (!count) return; \
    for (n = 0; n < count; n++) { T* item = m_array[0]; m_array.RemoveAt(0); delete item; } \
    m_array.Clear(); \
} \
void name::RemoveAt(size_t index) \
{ \
    T* item = m_array[index]; if (item) { m_array.RemoveAt(index); delete item; } \
} \
bool name::Remove(const T& item) \
{ \
    int index = Index(item); \
    if (index == wxNOT_FOUND) return false; \
    RemoveAt(index); return true; \
} \
bool name::Remove(T* item) \
{ \
    int index = Index(item); \
    if (index == wxNOT_FOUND) return false; \
    RemoveAt(index); return true; \
}

#define DEFINE_SORTED_OBJARRAY_INTUPDATEPOS(T, Tsortedarray, intKeyMember, name) \
DEFINE_SORTED_OBJARRAY(T, Tsortedarray, name) \
bool name::UpdatePos( int pos, int numPos ) \
{ \
    int n, count = m_array.GetCount(), start_pos = IndexForInsert(T(pos)); \
    if ((numPos == 0) || (start_pos >= count)) return false; \
    if ( numPos > 0 ) \
    { \
        for (n=start_pos; n<count; n++) \
            m_array[n]->intKeyMember += numPos; \
    } \
    else if ( numPos < 0 ) \
    { \
        int pos_right = pos-numPos;     \
        for (n=start_pos; n<count; n++) \
        { \
            int &k = m_array[n]->intKeyMember;                \
            if (k < pos_right) { RemoveAt(n); n--; count--; } \
            else if (k >= pos_right) { k += numPos; }         \
        } \
    } \
    return true; \
}

#endif  // __WX_SRTOBJAR_H__
