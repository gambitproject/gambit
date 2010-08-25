//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/map.h
// Declaration of map container types
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef LIBGAMBIT_MAP_H
#define LIBGAMBIT_MAP_H

namespace Gambit {

template <class K, class T> class MapBase {
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
  MapBase(const T &d);
  //
  // Construct a map to have the same set of relations as another map.
  // 
  MapBase(const MapBase<K, T> &);

  //
  // This is the map destructor.  It deletes all allocated memory, and calls
  // the destructors for the keys and values which remain in the map at the
  // time of its deallocation.
  //
  virtual ~MapBase(); 

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

  virtual T &Lookup(const K &key) = 0;

  //
  // These are the equality and assignment operators for this and all derived
  // classes.
  //+grp
  int operator==(const MapBase &M) const;
  int operator!=(const MapBase &M) const;
  MapBase<K, T> &operator=(const MapBase &M);
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

};

//
// <category lib=glib sect=Containers>
//
// The Map is an ordered map.  That is, the index class has all the
// usual ordering operators defined upon it (==, !=, <, <=, >, >=).  These
// are used to sort the map by keys, thus making search-type operations
// logarithmic instead of linear.  This is a particularly large improvement
// when using keys which are costly to compare
//
template <class K, class T> class Map : public MapBase<K, T>  {
private:
  int Locate(const K &key) const;

public:
  //
  // Construct an ordered map with no mappings and the given default value.
  //
  Map(const T &d);
  //
  // Construct an ordered map with the same key-value mappings as another
  // ordered map.
  //
  Map(const Map<K, T> &m);

  virtual ~Map();

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

  T &Lookup(const K &key);

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

//-------------------------------------------------------------------------
//                    MapBase<K, T> member functions
//-------------------------------------------------------------------------

template <class K, class T> MapBase<K, T>::MapBase(const T &d)
  : length(0), _default(d), keys(0), values(0)
{ }

template <class K, class T>
MapBase<K, T>::MapBase(const MapBase<K, T> &m)
  : length(m.length), _default(m._default)
{
  keys = new K[length];
  values = new T[length];

  for (int i = 0; i < length; i++)   {
    keys[i] = m.keys[i];
    values[i] = m.values[i];
  }
}

template <class K, class T> MapBase<K, T>::~MapBase()
{
  delete [] keys;
  delete [] values;
}

template <class K, class T>
int MapBase<K, T>::operator==(const MapBase<K,T> &M) const
{
  if (length != M.length) return 0;

  for (int i = 0; i < length; i++)
    if (keys[i] != M.keys[i] || values[i] != M.values[i])  return 0;

  return (_default == M._default);
}

template <class K, class T>
int MapBase<K, T>::operator!=(const MapBase<K, T> &M) const
{
  return !(*this == M);
}

template <class K, class T>
MapBase<K, T> &MapBase<K, T>::operator=(const MapBase<K,T> &M)
{
  if (this != &M)   {
    length = M.length;

    if (keys) delete [] keys;
    if (values) delete [] values;

    if (M.length)   {
      keys = new K[M.length];
      values = new T[M.length];
      for (int i = 0; i < length; i++)  {
	keys[i] = M.keys[i];
        values[i] = M.values[i];
      }
    }
    else  {
      keys = 0;
      values = 0;
    }
     
    _default = M._default;
  }
  return *this;
}

template <class K, class T>
T &MapBase<K, T>::Insert(const K &key, int entry, const T &value)
{
  K *new_keys = new K[length + 1];
  T *new_values = new T[length + 1];
  
  if (length > 0)   {
    int i;
    for (i = 0; i < entry; i++)   {
      new_keys[i] = keys[i];
      new_values[i] = values[i];
    }
    for (i++; i <= length; i++)   {
      new_keys[i] = keys[i - 1];
      new_values[i] = values[i - 1];
    }
  }

  new_keys[entry] = key;
  new_values[entry] = value;

  if (length > 0)   {
    delete [] keys;
    delete [] values;
  }

  keys = new_keys;
  values = new_values;
  length++;
  return values[entry];
}

template <class K, class T> T MapBase<K, T>::Delete(int where)
{
  if (length == 1)  {
    T ret = values[0];
    delete [] keys;
    delete [] values;
    keys = 0;
    values = 0;
    length = 0;
    return ret;
  }

  T ret = values[where];
    
  K *new_keys = new K[length - 1];
  T *new_values = new T[length - 1];

  int i;
  for (i = 0; i < where; i++)   {
    new_keys[i] = keys[i];
    new_values[i] = values[i];
  }

  for (i++; i < length; i++)  {
    new_keys[i - 1] = keys[i];
    new_values[i - 1] = values[i];
  }

  delete [] keys;
  delete [] values;
    
  keys = new_keys;
  values = new_values;

  length--;
  return ret;
}

template <class K, class T> T &MapBase<K, T>::Default(void)
{
  return _default;
}

template <class K, class T> const T &MapBase<K, T>::Default(void) const
{
  return _default;
}

template <class K, class T> int MapBase<K, T>::Length(void) const
{
  return length;
}


//-------------------------------------------------------------------------
//                      Map<K, T> member functions
//-------------------------------------------------------------------------

template <class K, class T> Map<K, T>::Map(const T &d)
  : MapBase<K, T>(d)
{ }

template <class K, class T> Map<K, T>::Map(const Map<K, T> &m)
  : MapBase<K, T>(m)
{ }

template <class K, class T> Map<K, T>::~Map()   { }

template <class K, class T>
int Map<K, T>::Locate(const K &key) const
{
  int low = 0, high = this->length - 1, mid = 0;
  
  while (low <= high)   {
    mid = (low + high) / 2;
    if (key < this->keys[mid])     high = mid - 1;
    else if (key > this->keys[mid])    low = mid + 1;
    else    return mid;
  }

  return mid;
}


template <class K, class T> T &Map<K, T>::operator()(const K &key)
{
  int where = Locate(key);

  if (this->length > 0 && this->keys[where] == key) return this->values[where];
  else return Insert(key, ((key < this->keys[where]) ? where : where + 1),
		     this->_default);
}

template <class K, class T> T &Map<K, T>::Lookup(const K &key)
{
  int where = Locate(key);

  if (this->length > 0 && this->keys[where] == key) return this->values[where];
  else return Insert(key, ((key < this->keys[where]) ? where : where + 1),
		     this->_default);
}

template <class K, class T> T Map<K, T>::operator()(const K &key) const
{
  int where = Locate(key);

  if (this->length > 0 && this->keys[where] == key) return this->values[where];
  else return this->_default;
}

template <class K, class T> int Map<K, T>::IsDefined(const K &key) const
{
  if (this->length == 0)   return 0;
  return (this->keys[Locate(key)] == key);
}

template <class K, class T>
void Map<K, T>::Define(const K &key, const T &value)
{
  if (this->length == 0)  {
    Insert(key, 0, value);
    return;
  }

  int where = Locate(key);

  if (this->keys[where] == key)   this->values[where] = value;
  else Insert(key, ((key < this->keys[where]) ? where : where + 1), value);
}

template <class K, class T> T Map<K, T>::Remove(const K &key)
{
  int where = Locate(key);

  if (where >= 0)  return this->Delete(where);
  return this->_default;
}

} // end namespace Gambit

#endif // LIBGAMBIT_MAP_H


