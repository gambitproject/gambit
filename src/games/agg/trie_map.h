//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/trie_map.h
// Trie with STL-like interfaces
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

#ifndef GAMBIT_AGG_TRIEMAP_H
#define GAMBIT_AGG_TRIEMAP_H

//Mapping from vector of ints to type V.
//WARNING: traversal using the iterators is in the reverse order of insertion.

#include <cmath>
#include <list>
#include <vector>
#include <iterator>
#include <sstream>
#include "proj_func.h"

namespace Gambit {

namespace agg {

//forward declarations

template <class V> class trie_map;
template <class V> std::ostream& operator<< (std::ostream& s, const trie_map<V>& t);



template <class V>
struct TrieNode
{
  //members:
  std::vector<TrieNode*> children;
  typename std::list<std::pair<std::vector<int>,V> >::iterator val;

  //methods:

  //constructor  
  TrieNode(size_t branches, typename std::list<std::pair<std::vector<int>, V> >::iterator v)
    : children(branches, (TrieNode *) NULL), val(v)
  {}

  TrieNode *&operator[](size_t i)
  {
    if (i < children.size()) { return children[i]; }
    else {
      children.resize(i + 1, (TrieNode *) NULL);
      return children[i];
    }
  }
};

template <class V> class trie_map {
public:
  using key_type = std::vector<int>;
  using value_type = std::pair<std::vector<int>, V>;

  using pointer = V*;
  using reference = V&;

  using size_type = unsigned int;

  using iterator = typename std::list<value_type>::iterator;
  using const_iterator = typename std::list<value_type>::const_iterator;


  //friends
  friend std::ostream& operator<< <V>(std::ostream& s, const trie_map<V>& t);
  
  friend class agg;

  //constructors
  trie_map() : initBranches(1), root(new TrieNode<V>(1, data.end()))
  {}

  explicit trie_map(int branches)
    : initBranches(branches), root(new TrieNode<V>(branches, data.end()))
  {}

  //copy constructor
  trie_map(const trie_map<V> &other);

  //destructor
  ~trie_map()
  { deleteNodes(root); }

  //assignment
  trie_map<V>& operator=( const trie_map<V>& other);

  void swap (trie_map<V>& other);

  size_type size() const {return data.size();}
  bool empty() const {return data.empty();}
  iterator begin() {return data.begin();}
  iterator end() {return data.end();}
  const_iterator begin() const {return data.begin();}
  const_iterator end() const {return data.end();}

  //insert: same interface as in STL map
  std::pair<iterator,bool> insert (const value_type& x);

  template<class InputIterator>
  void insert(InputIterator f, InputIterator l)
  {
    for (InputIterator p = f; p != l; ++p) {
      insert(*p);
    }
  }

  //insert or add
  trie_map<V> &operator+=(const value_type &x)
  {
    std::pair<typename trie_map<V>::iterator, bool> r = insert(x);
    if (!r.second) { (*r.first).second += x.second; }
    return *this;
  }


  //prefix matching
  //return a reference to iterator
  iterator& find (const key_type& k) const;

  //exact matching
  iterator findExact (const key_type& k);

  //clear the tree structure as well as data
  void clear()
  {
    deleteNodes(root);
    root = new TrieNode<V>(initBranches, data.end());
    data.clear();
    leaves.clear();
  }

  //clear data, but keep the tree structure
  void reset()
  {
    while (!leaves.empty()) {
      leaves.back()->val = data.end();
      leaves.pop_back();
    }
    data.clear();
  }

  //number of elements with key exactly k.
  // returns 1 or 0 
  size_type count(const key_type &k)
  {
    size_t i = 0;
    TrieNode<V> *ptr = root;
    for (; i < k.size() && k[i] < (int) ptr->children.size() &&
           ptr->children[k[i]]; ptr = ptr->children[k[i++]]) {}

    return (i >= k.size() && ptr->val != data.end());
  }

  //in order traversal
  template<class UnaryFunction>
  void in_order(UnaryFunction f)
  {
    in_order_subtree(f, root);
  }

  //polynomial multiplication of t1 and t2, store the result in self
  void multiply(const trie_map<V> &t1, const trie_map<V> &t2, size_t keylen,
                std::vector<projtype> &f);

  //multiply in-place. other should not be the same object as self.
  void multiply(const trie_map<V> &other, size_t keylen, std::vector<projtype> &f);

  //squaring
  void square(trie_map<V> &dest, size_t keylen, std::vector<projtype> &f) const;

  //squaring in-place
  void square(size_t keylen, std::vector<projtype> &f);

  void
  power(size_t p, trie_map<V> &dest, trie_map<V> &scratch, size_t keylen, std::vector<projtype> &f);

  //inner product
  V inner_prod(trie_map<V> &other, V init = (V) (0)) const;

  //first apply the action x, then inner prod
  V inner_prod(const std::vector<int> &x, size_t keylen, std::vector<projtype> &f,
               trie_map<V> &other, V init = (V) (0)) const;

  //polynomial division
  trie_map<V>& operator/=(const std::vector<V>& denom);

  
private:
  //member variables:
  std::list<value_type> data;
  size_type initBranches; //default branching factor
  TrieNode<V> *root;

  std::vector<TrieNode<V>*> leaves;

  static const double THRESH;

  //private methods:
  //div: helper for operator/=()
  void div(const std::vector<V>& denom, TrieNode<V>* n, int current, int pivot);

  void deleteNodes(TrieNode<V> *n);

  //in order traversal
  template<class UnaryFunction>
  void in_order_subtree(UnaryFunction &f, TrieNode<V> *n)
  {
    if (n == NULL) { return; }
    size_type i, s = n->children.size();
    bool is_leaf = true;
    for (i = 0; i < s; ++i) {
      if (n->children[i]) {
        is_leaf = false;
        in_order_subtree(f, n->children[i]);
      }
    }
    if (is_leaf) {
      f(n->val);
    }
  }

  //private helper functor classes:

  struct div_helper {
    div_helper(const std::vector<V> &den, int piv, iterator en)
      : pivot(piv), denom(den), endp(en)
    {}

    void operator()(iterator p)
    {
      if (p == endp) { return; }
      p->first[pivot]--;
      p->second /= denom[pivot];
    }

    int pivot;
    const std::vector<V> &denom;
    iterator endp;
  };

  struct div_helper_mul {
    div_helper_mul(const std::vector<V> &den, int piv, TrieNode<V> *des, iterator en)
      : pivot(piv), denom(den), dest(des), endp(en)
    {}

    void add(const std::vector<int> &conf, V y);
    void operator()(iterator p);

    int pivot;
    const std::vector<V> &denom;
    TrieNode<V> *dest;
    iterator endp;
  };  //end struct div_helper_mul

};

template <class V> const double trie_map<V>::THRESH = 1e-12;

template <class V> std::ostream &operator<<(std::ostream &s, const trie_map<V> &t);

}  // end namespace Gambit::agg

}  // end namespace Gambit

#endif   // GAMBIT_AGG_TRIEMAP_H
