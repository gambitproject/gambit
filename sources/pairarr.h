/////////////////////////////////////////////////////////////////////////////
// Name:        pairarr.h
// Purpose:     Sorted Key/Value pairs of wxArrays using a binary search lookup
// Author:      John Labenski
// Modified by: 
// Created:     1/08/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PAIRARR_H__
#define __WX_PAIRARR_H__

#include "wx/dynarray.h"

// ---------------------------------------------------------------------------
// DECLARE_PAIRED_DATA_ARRAYS(Tkey, TkeyArray, Tval, TvalArray, name, classexp)
//   Tkey must have the operators =, ==, <, >, <=, >=, +=, -=, +, -
//     They'll be sorted by using the <> operators
//   Tval must have a default constructor and be able to be passed as const Tval& val
//   You must have created wxObjectArrays of Tkey with name TkeyArray
//     and of Tval named TvalArray, for example wxArrayInt and wxArrayString
//
// Creates a class named "name" that manages the TkeyArray, TvalArray data
//   it keeps the pairs sorted in order of Tkey and uses a binary search
//   to retrieve and set the values.
// 
// name() - default constructor
// name(const name& other) - full copy constructor
// name(Tkey key, const Tval& val) - create with the first pair 
// int GetCount() - get the number of pairs
// int FindIndex(Tkey) - find this key returning position in pair array or wxNOT_FOUND
// size_t FindInsertIndex(Tkey) - find array position to insert key at, returns 
//     GetCount for append (check first in case count=0), the pos to insert
//     before, or the pos with existing key  (see SetValue for code)
// bool HasKey(Tkey) - does this key exist
// Tval& GetValue(Tkey) - get the value for this key or Tval()
// Tval& GetOrCreateValue(Tkey key) - get or create a Tval() value for this key
// void SetValue(Tkey, Tval) - set the Tval for this Tkey, replacing if exists
// bool RemoveValue(Tkey) - remove pair with this Tkey, returns if it existed
// void Clear() - clears the pair arrays
// Tval& GetItemValue(index) - get the Tval at this array index
// Tkey& GetItemKey(index) - get the Tkey at this array index
// void RemoveAt(index) - remove the key and value at this array index
// TvalArray& GetValues() - get the TvalArray
// TkeyArray& GetKeys() - get the TkeyArray
// bool InsertPos(Tkey pos, Tkey numPos) - for int keys, shifts pair keys 
//   greater pos by numPos up, who knows what happens for other key types
// bool DeletePos(Tkey pos, Tkey numPos) - for int keys, shifts pair keys 
//   greater than pos+numPos down and deletes pairs with keys between 
//   pos->pos+numPos, who knows what happens for other key types
// void Copy(const name& other) - make full copy of other
// void Sort() - sort the pairs by the keys (only necessary if you want to  
//   quickly add unorderered pairs using GetKeys().Add(x); GetValues().Add(x);)
//   You MUST keep them sorted for the lookup mechanism to work.
// name& operator=(const name& other) - make full copy of other
//
// Examples:
// DECLARE_PAIRED_DATA_ARRAYS(int, wxArrayInt, wxString, wxArrayString, 
//                            wxPairArrayIntSheetString, class WXDLLIMPEXP_ADV)
//
// You can even make nested pair arrays, 2D arrays (wxSheetStringSparseTable)
// WX_DECLARE_OBJARRAY_WITH_DECL(wxPairArrayIntSheetString, 
//                               wxArrayPairArrayIntSheetString,
//                               class WXDLLIMPEXP_ADV);
// DECLARE_PAIRED_DATA_ARRAYS(int, wxArrayInt, wxPairArrayIntSheetString, 
//       wxArrayPairArrayIntSheetString, wxPairArrayIntPairArraySheetStringBase,
//       class WXDLLIMPEXP_ADV)

#define DECLARE_PAIRED_DATA_ARRAYS(Tkey, TkeyArray, Tval, TvalArray, name, classexp) \
\
classexp name                                                                \
{                                                                            \
public:                                                                      \
    name() {}                                                                \
    name(const name& other) { Copy(other); }                                 \
    name(Tkey key, const Tval& val) { m_keys.Add(key); m_values.Add(val); }  \
    int GetCount() const { return m_keys.GetCount(); }                       \
    int FindIndex(Tkey key) const;                                           \
    size_t FindInsertIndex(Tkey pos) const;                                  \
    bool HasKey(Tkey key) const { return FindIndex(key) != wxNOT_FOUND; }    \
    Tval& GetValue(Tkey key);                                                \
    Tval& GetOrCreateValue(Tkey key);                                        \
    void SetValue(Tkey key, const Tval& value);                              \
    bool RemoveValue(Tkey key);                                              \
    void Clear() { m_keys.Clear(); m_values.Clear(); }                       \
    Tval& GetItemValue(size_t index) { return m_values[index]; }             \
    Tkey& GetItemKey(size_t index)   { return m_keys[index]; }               \
    void RemoveAt(size_t index) { m_keys.RemoveAt(index); m_values.RemoveAt(index); } \
    const TvalArray& GetValues() const { return m_values; }                  \
    const TkeyArray& GetKeys()   const { return m_keys; }                    \
    TvalArray & GetValues() { return m_values; }                             \
    TkeyArray & GetKeys()   { return m_keys; }                               \
    bool InsertPos( Tkey pos, Tkey numPos );                                 \
    bool DeletePos( Tkey pos, Tkey numPos );                                 \
    void Copy(const name& other) { m_keys = other.GetKeys(); m_values = other.GetValues(); } \
    void Sort() { if (GetCount() > 1) q_sort(0, GetCount()-1); }             \
    name& operator=(const name& other) { Copy(other); return *this; }        \
protected :                                                                  \
    void q_sort(int left, int right);                                        \
    TkeyArray m_keys;                                                        \
    TvalArray m_values;                                                      \
    Tval m_defaultValue;                                                     \
}; 

#define DEFINE_PAIRED_DATA_ARRAYS(Tkey, Tval, name) \
\
Tval& name::GetValue(Tkey key) \
{ \
    int n = FindIndex(key); \
    if (n != wxNOT_FOUND) return m_values[n]; \
    return m_defaultValue; \
} \
Tval& name::GetOrCreateValue(Tkey key) \
{ \
    size_t n = FindInsertIndex(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(Tval()); } \
    else if (key != m_keys[n])  \
        { m_keys.Insert(key, n); m_values.Insert(Tval(), n); } \
    return m_values[n]; \
} \
void name::SetValue(Tkey key, const Tval& value) \
{ \
    size_t n = FindInsertIndex(key); \
    if (n == m_keys.GetCount())  \
        { m_keys.Add(key); m_values.Add(value); } \
    else if (key == m_keys[n])  \
        m_values[n] = value; \
    else \
        { m_keys.Insert(key, n); m_values.Insert(value, n); } \
} \
bool name::RemoveValue(Tkey key) \
{ \
    int n = FindIndex(key); \
    if (n != wxNOT_FOUND) { RemoveAt(n); return TRUE; } \
    return FALSE; \
} \
int name::FindIndex(Tkey key) const \
{ \
    int n, count = m_keys.GetCount(); int lo = 0, hi = count; \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;             \
        Tkey &tmp = m_keys[n];       \
        if (tmp == key) return n;    \
        if (tmp  > key) hi = n;      \
        else            lo = n + 1;  \
    } \
    return wxNOT_FOUND; \
} \
size_t name::FindInsertIndex(Tkey key) const \
{ \
    size_t n, count = m_keys.GetCount(); size_t lo = 0, hi = count; \
    if ((count == 0u) || (key < m_keys[0])) return 0u;    \
    if (key > m_keys[count-1])              return count; \
    while ( lo < hi ) \
    { \
        n = (lo + hi)/2;              \
        Tkey & tmp = m_keys[n];       \
        if (tmp == key) return n;     \
        if (tmp  > key) hi = n;       \
        else            lo = n + 1;   \
    } \
    return lo; \
} \
bool name::InsertPos( Tkey key, Tkey numPos ) \
{ \
    size_t n, count = m_keys.GetCount();       \
    for (n=FindInsertIndex(key); n<count; n++) \
        m_keys[n] += numPos;                   \
    return TRUE;                               \
} \
bool name::DeletePos( Tkey key, Tkey numPos ) \
{ \
    Tkey pos_right = key+numPos;               \
    int n, count = m_keys.GetCount();          \
    for (n=FindInsertIndex(key); n<count; n++) \
    { \
        Tkey &k = m_keys[n];                              \
        if (k < pos_right) { RemoveAt(n); n--; count--; } \
        else if (k >= pos_right) { k -= numPos; }         \
    } \
    return TRUE; \
} \
void name::q_sort(int left, int right) \
{ \
    int l_hold = left, r_hold = right; \
    Tkey pivot = m_keys[left]; Tval pivotVal = m_values[left]; \
    while (left < right) \
    { \
        while ((m_keys[right] >= pivot) && (left < right)) right--;       \
        if (left != right) { m_keys[left] = m_keys[right];                \
                             m_values[left] = m_values[right]; left++; }  \
        while ((m_keys[left] <= pivot) && (left < right)) left++;         \
        if (left != right) { m_keys[right] = m_keys[left];                \
                             m_values[right] = m_values[left]; right--; } \
    } \
    m_keys[left] = pivot; m_values[left] = pivotVal; \
    if (l_hold < left) q_sort(l_hold, left-1); \
    if (r_hold > left) q_sort(left+1, r_hold); \
}

#endif  // __WX_PAIRARR_H__
