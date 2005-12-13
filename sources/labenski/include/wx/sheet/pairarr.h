///////////////////////////////////////////////////////////////////////////////
// Name:        pairarr.h
// Purpose:     Sorted Key/Value pairs of wxArrays using a binary search lookup
// Author:      John Labenski
// Modified by: 
// Created:     1/08/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PAIRARR_H__
#define __WX_PAIRARR_H__

// Note: there is no cpp file as all the code is contained in these macros.

#include "wx/dynarray.h"

// ============================================================================
// Provides macros for creating your own (key, value) pair arrays using a binary
// search to insert/retrieve values using a key. While this doesn't have the 
// performance of a good hash table O(n), it is smaller and with a lookup speed
// O(log(n)) is suitable for some applications. You can use virtually any
// class for keys and values that can be put into wxArrays so long as they
// define the standard comparison operators ==, <, >, <=, >=.
//
// Implementation note: I've chosen to use two independent arrays instead of 
// a single array of a data struct with (key, value) members to squeeze out the 
// slightest increase in performance.
// ----------------------------------------------------------------------------
// DECLARE_PAIRED_DATA_ARRAYS(Tkey, TkeyArray, Tval, TvalArray, name, classexp)
// DEFINE_PAIRED_DATA_ARRAYS(Tkey, Tval, name)
//   Tkey must have the operators =, ==, <, >, <=, >=
//     They'll be sorted by using the <, >, <=, >= operators
//   Tval must have a default constructor and be able to be passed as const Tval& val
//   You must have created wx(Object)Arrays of Tkey with name TkeyArray
//     and of Tval named TvalArray, for example wxArrayInt and wxArrayString
//
// Creates a class named "name" that manages the TkeyArray, TvalArray data.
//   It keeps the pairs sorted in order of Tkey and uses a binary search
//   to retrieve and set the values.
//
// ----------------------------------------------------------------------------
// DECLARE_PAIRED_INT_DATA_ARRAYS(Tval, TvalArray, name, classexp)
// DEFINE_PAIRED_INT_DATA_ARRAYS(Tval, name)
//   Tkey is an int (wxArrayInt), Tval may be anything
//   UpdatePos(int pos, int num) is added for inserting if num > 0 and 
//     deleting if num < 0. The keys above pos are shifted.
//
// ----------------------------------------------------------------------------
// name() - default constructor
// name(const Tval& defaultVal) - initialize the class with the default value, 
//     see Get/SetDefaultValue to change it later.
// name(const name& other) - full copy constructor
// name(Tkey key, const Tval& val) - create with the first pair 
// int GetCount() - get the number of pairs
// int FindIndex(Tkey) - find this key returning position in pair array or wxNOT_FOUND
// size_t FindInsertIndex(Tkey) - find array position to insert key at, returns 
//     GetCount for append (check first in case count=0), the pos to insert
//     before, or the pos with existing key  (see SetValue for code)
// bool HasKey(Tkey) - does this key exist
// Tval& GetValue(Tkey) - get the value for this key or it it doesn't exist 
//     the default value, see also Get/SetDefaultValue.
// Tval& GetOrCreateValue(Tkey key) - get or create a GetDefaultValue() value for this key
// void SetValue(Tkey, Tval) - set the Tval for this Tkey, replacing if exists
// bool RemoveValue(Tkey) - remove pair with this Tkey, returns if it existed
// void Clear() - clears the pair arrays
// const Tval& GetItemValue(index) const - get the Tval at this array index
// const Tkey& GetItemKey(index) const - get the Tkey at this array index
// Tval& GetItemValue(index) - get the Tval at this array index
// Tkey& GetItemKey(index) - get the Tkey at this array index
// void RemoveAt(index) - remove the key and value at this array index
// TvalArray& GetValues() - get the TvalArray
// TkeyArray& GetKeys() - get the TkeyArray (don't unsort them)
// const Tval& GetDefaultValue() const - get the default value to return for 
//   GetValue(Tkey) when the key doesn't exist. (inits to Tval())
// void SetDefaultValue(const Tval& val) - set the default value to return for
//   GetValue(Tkey) when the key doesn't exist. If your values don't have a 
//   default constructor (eg. ints) you'll want to set this.
// void Copy(const name& other) - make full copy of other
// void Sort() - sort the pairs by the keys (only necessary if you want to  
//   quickly add unorderered pairs using GetKeys().Add(x); GetValues().Add(x);)
//   You MUST keep them sorted for the lookup mechanism to work.
// name& operator=(const name& other) - make full copy of other
//
// ----------------------------------------------------------------------------
// DECLARE_PAIRED_INT_DATA_ARRAYS - added functions
// bool UpdatePos(int pos, int numPos) - 
//   if numPos > 0 - shifts keys greater than pos by numPos 
//   if numPos < 0 - deletes keys between pos and pos-numPos, 
//     shifts keys greater than by pos-numPos by -numPos
// 
// ============================================================================
// Examples:
//
// 1.) For string arrays you'll write this in the header
// DECLARE_PAIRED_DATA_ARRAYS(wxString, wxArrayString, wxString, wxArrayString, 
//                            wxPairArrayStringString, class WXDLLIMPEXP_ADV)
// And this code in some cpp file.
// DEFINE_PAIRED_DATA_ARRAYS(wxString, wxString, wxPairArrayStringString)
//
// 2.) For int pairs and wxString values, write this in your header
// DECLARE_PAIRED_INT_DATA_ARRAYS(wxString, wxArrayString, 
//                                wxPairArrayIntSheetString, class WXDLLIMPEXP_ADV)
//
// You can even make nested pair arrays, 2D arrays (wxSheetStringSparseTable)
// WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString, 
//                               wxArrayPairArrayIntSheetString,
//                               class WXDLLIMPEXP_ADV);
// DECLARE_PAIRED_INT_DATA_ARRAYS(wxPairArrayIntSheetString, wxArrayPairArrayIntSheetString, 
//                                wxPairArrayIntPairArraySheetStringBase, class WXDLLIMPEXP_ADV)
//
// In your source file write this to get the code for the pair array
// DEFINE_PAIRED_INT_DATA_ARRAYS(wxString, wxPairArrayIntSheetString)
// DEFINE_PAIRED_INT_DATA_ARRAYS(wxPairArrayIntSheetString, 
//                               wxPairArrayIntPairArraySheetStringBase)
//
// ============================================================================

#define DECLARE_PAIRED_DATA_ARRAYS_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
\
classexp name                                                                \
{                                                                            \
public:                                                                      \
    name() {}                                                                \
    name(const Tval& defaultVal) : m_defaultValue(defaultVal) {}             \
    name(const name& other) { Copy(other); }                                 \
    name(const Tkey& key, const Tval& val) { m_keys.Add(key); m_values.Add(val); } \
    int GetCount() const { return m_keys.GetCount(); }                       \
    int FindIndex(const Tkey& key) const;                                    \
    size_t FindInsertIndex(const Tkey& pos) const;                           \
    bool HasKey(const Tkey& key) const { return FindIndex(key) != wxNOT_FOUND; } \
    const Tval& GetValue(const Tkey& key) const;                             \
    Tval& GetValue(const Tkey& key);                                         \
    Tval& GetOrCreateValue(const Tkey& key);                                 \
    void SetValue(const Tkey& key, const Tval& value);                       \
    bool RemoveValue(const Tkey& key);                                       \
    void Clear() { m_keys.Clear(); m_values.Clear(); }                       \
    const Tval& GetItemValue(size_t index) const { return m_values[index]; } \
    const Tkey& GetItemKey(size_t index)   const { return m_keys[index]; }   \
    Tval& GetItemValue(size_t index) { return m_values[index]; }             \
    Tkey& GetItemKey(size_t index)   { return m_keys[index]; }               \
    void RemoveAt(size_t index) { m_keys.RemoveAt(index); m_values.RemoveAt(index); } \
    const TvalArray& GetValues() const { return m_values; }                  \
    const TkeyArray& GetKeys()   const { return m_keys; }                    \
    TvalArray& GetValues() { return m_values; }                              \
    TkeyArray& GetKeys()   { return m_keys; }                                \
    const Tval& GetDefaultValue() const { return m_defaultValue; }           \
    void SetDefaultValue(const Tval& val) { m_defaultValue = val; }          \
    void Copy(const name& other);                                            \
    void Sort() { if (GetCount() > 1) q_sort(0, GetCount()-1); }             \
    name& operator=(const name& other) { Copy(other); return *this; }        \
protected :                                                                  \
    void q_sort(int left, int right);                                        \
    TkeyArray m_keys;                                                        \
    TvalArray m_values;                                                      \
    Tval m_defaultValue;
 
// ----------------------------------------------------------------------------
// Note: The above macros is incomplete to allow you to extend the class.

#define DECLARE_PAIRED_DATA_ARRAYS(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
DECLARE_PAIRED_DATA_ARRAYS_BASE(Tkey, TkeyArray, Tval, TvalArray, name, classexp)    \
};

#define DECLARE_PAIRED_INT_DATA_ARRAYS_BASE(Tval, TvalArray, name, classexp)      \
DECLARE_PAIRED_DATA_ARRAYS_BASE(int, wxArrayInt, Tval, TvalArray, name, classexp) \
public: \
    bool UpdatePos( int pos, int numPos );

#define DECLARE_PAIRED_INT_DATA_ARRAYS(Tval, TvalArray, name, classexp) \
DECLARE_PAIRED_INT_DATA_ARRAYS_BASE(Tval, TvalArray, name, classexp)    \
};

// ============================================================================
#define DEFINE_PAIRED_DATA_ARRAYS(Tkey, Tval, name) \
\
const Tval& name::GetValue(const Tkey& key) const \
{ \
    const int n = FindIndex(key); \
    if (n != wxNOT_FOUND) return m_values[n]; \
    return m_defaultValue; \
} \
Tval& name::GetValue(const Tkey& key) \
{ \
    const int n = FindIndex(key); \
    if (n != wxNOT_FOUND) return m_values[n]; \
    return m_defaultValue; \
} \
Tval& name::GetOrCreateValue(const Tkey& key) \
{ \
    const size_t n = FindInsertIndex(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(Tval(m_defaultValue)); } \
    else if (key != m_keys[n])  \
        { m_keys.Insert(key, n); m_values.Insert(Tval(m_defaultValue), n); } \
    return m_values[n]; \
} \
void name::SetValue(const Tkey& key, const Tval& value) \
{ \
    const size_t n = FindInsertIndex(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(value); } \
    else if (key == m_keys[n])  \
        m_values[n] = value; \
    else \
        { m_keys.Insert(key, n); m_values.Insert(value, n); } \
} \
bool name::RemoveValue(const Tkey& key) \
{ \
    const int n = FindIndex(key); \
    if (n != wxNOT_FOUND) { RemoveAt(n); return true; } \
    return false; \
} \
int name::FindIndex(const Tkey& key) const \
{ \
    size_t n, lo = 0, hi = m_keys.GetCount(); \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;             \
        const Tkey &tmp = m_keys[n]; \
        if (tmp == key) return n;    \
        if (tmp  > key) hi = n;      \
        else            lo = n + 1;  \
    } \
    return wxNOT_FOUND; \
} \
size_t name::FindInsertIndex(const Tkey& key) const \
{ \
    size_t n, lo = 0, hi = m_keys.GetCount(); \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;             \
        const Tkey &tmp = m_keys[n]; \
        if (tmp == key) return n;    \
        if (tmp  > key) hi = n;      \
        else            lo = n + 1;  \
    } \
    return lo; \
} \
void name::Copy(const name& other) \
{ \
    m_keys = other.GetKeys();                 \
    m_values = other.GetValues();             \
    m_defaultValue = other.GetDefaultValue(); \
} \
void name::q_sort(int left, int right) \
{ \
    int l_hold = left, r_hold = right; \
    Tkey pivot = m_keys[left]; Tval pivotVal = m_values[left]; \
    while (left < right) \
    { \
        while ((m_keys[right] >= pivot) && (left < right)) right--;       \
        if (left != right) { m_keys[left]   = m_keys[right];              \
                             m_values[left] = m_values[right]; left++; }  \
        while ((m_keys[left] <= pivot) && (left < right)) left++;         \
        if (left != right) { m_keys[right]   = m_keys[left];              \
                             m_values[right] = m_values[left]; right--; } \
    } \
    m_keys[left] = pivot; m_values[left] = pivotVal; \
    if (l_hold < left) q_sort(l_hold, left-1); \
    if (r_hold > left) q_sort(left+1, r_hold); \
}

// ----------------------------------------------------------------------------

#define DEFINE_PAIRED_INT_DATA_ARRAYS(Tval, name) \
DEFINE_PAIRED_DATA_ARRAYS(int, Tval, name)  \
bool name::UpdatePos( int pos, int numPos ) \
{ \
    int n, count = m_keys.GetCount(), start_pos = FindInsertIndex(pos); \
    if ((numPos == 0) || (start_pos >= count)) return false; \
    if ( numPos > 0 ) \
    { \
        for (n=start_pos; n<count; n++) \
            m_keys[n] += numPos; \
    } \
    else if ( numPos < 0 ) \
    { \
        int pos_right = pos-numPos;     \
        for (n=start_pos; n<count; n++) \
        { \
            int &k = m_keys[n];                               \
            if (k < pos_right) { RemoveAt(n); n--; count--; } \
            else if (k >= pos_right) { k += numPos; }         \
        } \
    } \
    return true; \
}

#endif  // __WX_PAIRARR_H__
