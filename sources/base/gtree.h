//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A generic tree container class
//

#ifndef GTREE_H
#define GTREE_H

#include "base.h"

template <class T> class gTree;

template <class T> class gTreeNode   {
  friend class gTree<T>;

  private:
    T data;
    gTreeNode<T> *parent, *prev, *next, *eldest, *youngest;

  public:
    // Constructor
    gTreeNode(const T& _data, 
	      gTreeNode<T>* _parent, 
	      gTreeNode<T>* _prev,
	      gTreeNode<T>* _next,
	      gTreeNode<T>* _eldest,
	      gTreeNode<T>* _youngest);

    ~gTreeNode();

  inline void SetData    (const T&      newdata)      {data = newdata;}
  inline void SetParent  (gTreeNode<T>* newparent)    {parent   = newparent;}
  inline void SetPrev    (gTreeNode<T>* newprev)      {prev     = newprev;}
  inline void SetNext    (gTreeNode<T>* newnext)      {next     = newnext;}
  inline void SetEldest  (gTreeNode<T>* neweldest)    {eldest   = neweldest;}
  inline void SetYoungest(gTreeNode<T>* newyoungest)  {youngest = newyoungest;}

  inline T             GetData()        const {return data;}
  inline gTreeNode<T>* GetParent()      const {return parent;}
  inline gTreeNode<T>* GetPrev()        const {return prev;}
  inline gTreeNode<T>* GetNext()        const {return next;}
  inline gTreeNode<T>* GetEldest()      const {return eldest;}
  inline gTreeNode<T>* GetYoungest()    const {return youngest;}
};


template <class T> class gTree  {
  protected:
    gTreeNode<T>* root; 

    gTreeNode<T>* RecursiveFind(const T&, gTreeNode<T>*) const;
    void RecursiveCopy(gTreeNode<T>*, const gTreeNode<T>*);
    void RecursiveFlush(const gTreeNode<T>*);
    void RecursiveDump(gOutput&, const gTreeNode<T>*, const int) const;
    void Flush(void); 

  public:
    gTree(void);
    gTree(const T&);
    gTree(const gTree<T>&);
    virtual ~gTree();

    gTree<T>& operator=(const gTree<T>&);

    bool operator==(const gTree<T>& b) const;
    bool operator!=(const gTree<T>& b) const;

  // Constructive Manipulation
    void InsertAt(const T&, gTreeNode<T>*);

  // Information
    gList<gTreeNode<T>*> Children(const gTreeNode<T>*)              const;
    gTreeNode<T>*        RootNode()                                 const;
    gTreeNode<T>*        Find(const T&)                             const;
    bool                 Contains(const T& t)                       const;
    bool                 SubtreesAreIsomorphic(const gTreeNode<T>*, 
					       const gTreeNode<T>*) const;

  // Output
    void Dump(gOutput&) const;
};

template <class T> gOutput& operator<<(gOutput& f, const gTree<T>& b);

#endif    // GTREE_H
