//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of hash tables used by GCL
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef GSMHASH_H
#define GSMHASH_H

#include "base/ghash.h"

class Portion;
class gclFunction;
class gText;


class RefHashTable : public HashTable<gText, Portion*> {
private:
  int Hash( const gText& ref ) const;
  void DeleteAction(Portion* value );
  
public:
  RefHashTable();
  virtual ~RefHashTable();
};

class FunctionHashTable : public HashTable<gText, gclFunction*> {
private:
  int Hash( const gText& funcname ) const;
  void DeleteAction( gclFunction* func );

public:
  FunctionHashTable();
  virtual ~FunctionHashTable();

};

template <class T> class RefCountHashTable : public HashTable<T, int> {
 private:
  int Hash( const T& ptr ) const;
  void DeleteAction( int value );
  
public:
  RefCountHashTable();
  virtual ~RefCountHashTable();
};

#endif // GSMHASH_H
