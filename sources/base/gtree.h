//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A generic tree container class
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

#ifndef GTREE_H
#define GTREE_H

#include "base.h"

template <class T> class gbtTree;

template <class T> class gbtTreeNode   {
  friend class gbtTree<T>;

  private:
    T data;
    gbtTreeNode<T> *parent, *prev, *next, *eldest, *youngest;

  public:
    // Constructor
    gbtTreeNode(const T& _data, 
	      gbtTreeNode<T>* _parent, 
	      gbtTreeNode<T>* _prev,
	      gbtTreeNode<T>* _next,
	      gbtTreeNode<T>* _eldest,
	      gbtTreeNode<T>* _youngest);

    ~gbtTreeNode();

  inline void SetData    (const T&      newdata)      {data = newdata;}
  inline void SetParent  (gbtTreeNode<T>* newparent)    {parent   = newparent;}
  inline void SetPrev    (gbtTreeNode<T>* newprev)      {prev     = newprev;}
  inline void SetNext    (gbtTreeNode<T>* newnext)      {next     = newnext;}
  inline void SetEldest  (gbtTreeNode<T>* neweldest)    {eldest   = neweldest;}
  inline void SetYoungest(gbtTreeNode<T>* newyoungest)  {youngest = newyoungest;}

  inline T             GetData()        const {return data;}
  inline gbtTreeNode<T>* GetParent()      const {return parent;}
  inline gbtTreeNode<T>* GetPrev()        const {return prev;}
  inline gbtTreeNode<T>* GetNext()        const {return next;}
  inline gbtTreeNode<T>* GetEldest()      const {return eldest;}
  inline gbtTreeNode<T>* GetYoungest()    const {return youngest;}
};


template <class T> class gbtTree  {
  protected:
    gbtTreeNode<T>* root; 

    gbtTreeNode<T>* RecursiveFind(const T&, gbtTreeNode<T>*) const;
    void RecursiveCopy(gbtTreeNode<T>*, const gbtTreeNode<T>*);
    void RecursiveFlush(const gbtTreeNode<T>*);
    void RecursiveDump(gbtOutput&, const gbtTreeNode<T>*, const int) const;
    void Flush(void); 

  public:
    gbtTree(void);
    gbtTree(const T&);
    gbtTree(const gbtTree<T>&);
    virtual ~gbtTree();

    gbtTree<T>& operator=(const gbtTree<T>&);

    bool operator==(const gbtTree<T>& b) const;
    bool operator!=(const gbtTree<T>& b) const;

  // Constructive Manipulation
    void InsertAt(const T&, gbtTreeNode<T>*);

  // Information
    gbtList<gbtTreeNode<T>*> Children(const gbtTreeNode<T>*)              const;
    gbtTreeNode<T>*        RootNode()                                 const;
    gbtTreeNode<T>*        Find(const T&)                             const;
    bool                 Contains(const T& t)                       const;
    bool                 SubtreesAreIsomorphic(const gbtTreeNode<T>*, 
					       const gbtTreeNode<T>*) const;

  // Output
    void Dump(gbtOutput&) const;
};

template <class T> gbtOutput& operator<<(gbtOutput& f, const gbtTree<T>& b);

#endif    // GTREE_H
