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
    void RecursiveDump(std::ostream &,
		       const gbtTreeNode<T>*, const int) const;
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
    void Dump(std::ostream &) const;
};

//--------------------------------------------------------------------------
//                 gbtTreeNode<T>: Member function implementations
//--------------------------------------------------------------------------

template <class T>
gbtTreeNode<T>::gbtTreeNode(const T& _data, 
			gbtTreeNode<T>* _parent, 
			gbtTreeNode<T>* _prev, 
			gbtTreeNode<T>* _next, 
			gbtTreeNode<T>* _eldest, 
			gbtTreeNode<T>* _youngest)
  : data(_data), 
    parent(_parent), 
    prev(_prev), 
    next(_next), 
    eldest(_eldest), 
    youngest(_youngest)
{ }

template <class T> gbtTreeNode<T>::~gbtTreeNode()
{ 
}



//--------------------------------------------------------------------------
//                 gbtTree<T>: Member function implementations
//--------------------------------------------------------------------------

template <class T> gbtTree<T>::gbtTree(void) 
: root(NULL)
{ }

template <class T> gbtTree<T>::gbtTree(const T& rootdatum) 
: root(NULL)
{ 
  root = new gbtTreeNode<T>(rootdatum,NULL,NULL,NULL,NULL,NULL);
}

template <class T> gbtTree<T>::gbtTree(const gbtTree<T>& b)
: root(NULL)
{
  if (b.root != NULL)
    {
      root = new gbtTreeNode<T>(b.root->data, NULL, NULL, NULL, NULL, NULL);
      RecursiveCopy(root, b.root);
    }
}

template <class T> gbtTree<T>::~gbtTree()
{
  Flush();
}

template <class T> void gbtTree<T>::InsertAt(const T& t, gbtTreeNode<T>* n)
{
  if (n->eldest == NULL) {
    gbtTreeNode<T>* newn = new gbtTreeNode<T>(t,n,NULL,NULL,NULL,NULL);
    n->SetEldest(newn);
    n->SetYoungest(newn);
  }
  else {
    gbtTreeNode<T>* newn = new gbtTreeNode<T>(t,n,n->youngest,NULL,NULL,NULL);
    n->youngest->SetNext(newn);
    n->SetYoungest(newn);
  }
}

template <class T> void gbtTree<T>::RecursiveCopy(      gbtTreeNode<T>* copyn,
						const gbtTreeNode<T>* orign)
{
  gbtList<gbtTreeNode<T>*> oldchildren = Children(orign);
  int i;
  for (i = 1; i <= oldchildren.Length(); i++)
    InsertAt(oldchildren[i]->data,copyn);
  gbtList<gbtTreeNode<T>*> newchildren = Children(copyn);
  for (i = 1; i <= newchildren.Length(); i++)
    RecursiveCopy(newchildren[i],oldchildren[i]);
}

//--------------------- operators ------------------------

template <class T> gbtTree<T>& gbtTree<T>::operator=(const gbtTree<T>& b)
{
  if (this != &b)   {
    Flush();
    if (root != NULL)   {
      root = new gbtTreeNode<T>(b.root->data, NULL, NULL, NULL, NULL, NULL);
      RecursiveCopy(root, b.root);
    }
    else
      root = NULL;
  }
  return *this;
}

template <class T> bool gbtTree<T>::operator==(const gbtTree<T>& b) const
{
  if (root == NULL && b.root == NULL) return true;
  if (root == NULL || b.root == NULL) return false;
  return SubtreesAreIsomorphic(root,b.root);
}

template <class T> bool gbtTree<T>::operator!=(const gbtTree<T>& b) const
{
  return !(*this == b);
}

template <class T> 
gbtList<gbtTreeNode<T>*> gbtTree<T>::Children(const gbtTreeNode<T>* n) const
{
  gbtList<gbtTreeNode<T>*> answer;
  for (gbtTreeNode<T>* child = n->eldest; child != NULL; child = child->next)
    answer += child;
  return answer;
}

template <class T> gbtTreeNode<T>* gbtTree<T>::RootNode() const
{
  return root;
}

template <class T> 
gbtTreeNode<T>* gbtTree<T>::RecursiveFind(const T& t, gbtTreeNode<T>* n) const
{
  gbtTreeNode<T>* answer = NULL;
  if (n->data == t) answer = n;
  else {
    gbtTreeNode<T>* probe = n->eldest;
    while (answer == NULL && probe != NULL) {
      answer = RecursiveFind(t,probe);
      probe = probe->next;
    }
  }
  return answer;
}

template <class T> gbtTreeNode<T>* gbtTree<T>::Find(const T& t) const
{
  return RecursiveFind(t,root);
}

template <class T> bool gbtTree<T>::Contains(const T& t) const
{
  return (Find(t) != NULL);
}

template <class T> 
bool gbtTree<T>::SubtreesAreIsomorphic(const gbtTreeNode<T>* lhs, 
				     const gbtTreeNode<T>* rhs) const
{
  if (lhs->data != rhs->data) return false;
  gbtList<gbtTreeNode<T>*> lchildren = Children(lhs);
  gbtList<gbtTreeNode<T>*> rchildren = Children(rhs);
  if (lchildren.Length() != rchildren.Length()) return false;
  for (int i = 1; i <= lchildren.Length(); i++)
    if ( !SubtreesAreIsomorphic(lchildren[i],rchildren[i]) ) return false;
  return true;
}

template <class T> void gbtTree<T>::RecursiveFlush(const gbtTreeNode<T>* n)
{
  gbtList<gbtTreeNode<T>*> children = Children(n);
  for (int i = 1; i <= children.Length(); i++)
    RecursiveFlush(children[i]);
  delete n;
}

template <class T> void gbtTree<T>::Flush(void)
{
  RecursiveFlush(root);
  root = NULL;
}

template <class T> void gbtTree<T>::RecursiveDump(std::ostream& f, 
						const gbtTreeNode<T>* n, 
						const int level) const
{
  int i;
  //  for (i = 1; i <= level; i++) gout << "   ";
  //  gout << n->data << "\n";

  gbtList<gbtTreeNode<T>*> children = Children(n);
  for (i = 1; i <= children.Length(); i++) {
    RecursiveDump(f,children[i],level+1);
  }
}

template <class T> void gbtTree<T>::Dump(std::ostream& f) const
{
  RecursiveDump(f,root,0);
}


#endif    // GTREE_H
