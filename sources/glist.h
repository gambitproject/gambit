//#
//# FILE: gList.h -- Implementation of a generic array container class
//#
//# @(#)gList.h	1.11 10/23/94
//#

#ifndef gList_H
#define gList_H

#include <stdlib.h>
#include <assert.h>
#include "gambitio.h"

//
// <category lib=glib sect=Containers>
//
// This class is designed as a convenient alternative to using a pointer
// to a block of objects.  In addition to error checking on the bounds
// of the block, it also provides members to insert, remove, and append
// new elements to/from the block.  This class provides the same interface
// as the gBlock class (with minor exceptions).  The primary advantage of
// gList over gBlock is in situations that require frequent Append/Remove
// operations.  A gBlock is HIGHLY inefficient in this case.
//
//
template <class T> class gNode		{
	private:
		T	data;
		gNode<T>	*prev,*next;
	public:
	// Constructors
		gNode(void):prev(NULL),next(NULL) {}
		gNode(const T &_data,gNode<T> *_prev,gNode<T> *_next):data(_data),prev(_prev),next(_next) {}
		gNode(const gNode<T> &n):data(n.data),prev(n.prev),next(n.next) {}
	// Assignment
		gNode<T> &operator=(const gNode<T> &n) {data=n.data;prev=n.prev;next=n.next; return (*this);}
	// Comparison
		int operator==(const gNode<T> &n) {return (data==n.data && prev==n.prev && next==n.next);}
		int operator!=(const gNode<T> &n) {return !(*this==n);}
	// Data Access
		const T &Data(void)	const {return data;}
		T &Data(void) {return data;}
	// Movement
		const gNode<T> *Next(void) const {return next;}
		const gNode<T> *Prev(void) const {return prev;}
		gNode<T> *Next(void) {return next;}
		gNode<T> *Prev(void) {return prev;}
		void SetNext(gNode<T> *_next) {next=_next;}
		void SetPrev(gNode<T> *_prev) {prev=_prev;}
};

template <class T> class gList    {
	private:
		int length;
		gNode<T> *head;

//
// Insert the given element at the specified location.  Does not do any
// error checking as it assumes that the members which call it have already
// determined the proper location.  Returns the location at which the new
// element was placed.
//
		int InsertAt(const T &t, int where);

	public:
//
// Constructs the a block of the given length.  All elements of the block
// are constructed according to the default constructor for type T.
//
		gList(int len = 0) : length(len)  { head=NULL; if (len) for (int i=1;i<=len;i++) Append(T());}
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
		gList(const gList<T> &);
//
// Deallocates the block of memory, calling the destructors for any
// elements still within the block.
//
		~gList()    { Flush();}

//
// Sets the block to have the same contents as another block.
//
		gList<T> &operator=(const gList<T> &);

//
// Tests for equality of two blocks.  Blocks are equal if they have the
// same length, and the contents of all components are equal.
//+grp
		int operator==(const gList<T> &b) const;
		int operator!=(const gList<T> &b) const
			{ return !(*this == b); }
//-grp

//
// Return an element from the block by index into the block.
// <note> If an out-of-range index is given, the program will terminate in a
//        failed assertion.
//+grp
    const T &operator[](int) const;
    T &operator[](int);
//-grp

//
// Append an element to a gList.  Operator overloaded for ease of use.
//+grp
		gList<T> operator+(const T &e) const
			{ gList<T> result(*this); result.Append(e); return result; }
		gList<T>& operator+=(const T &e)
      { Append(e); return *this; }
//-grp

//
// Concatenate two gLists.  + puts the result in a separate gList,
// while += puts it in the first argument.
//+grp
		gList<T> operator+(const gList<T>& b) const;
		gList<T>& operator+=(const gList<T>& b)
      { *this = *this + b; return *this; }
//-grp

//
// Append a new element to the block, and return the index at which the
// element can be found.  Note that this index is guaranteed to be the
// last (highest) index in the block.
//
    int Append(const T &);
//
// Insert a new element into the block at a given index.  If the index is
// less than 1, the element is inserted at index 1; if the index is greater
// than the highest index, the element is appended to the end of the block.
// Returns the index at which the element actually is placed.
//
    int Insert(const T &, int);
//
// Remove the element at a given index from the block.  Returns the value
// of the element removed.
//
    T Remove(int);

//
// Return the index at which a given element resides in the block, or zero
// if the element is not found.
//
    int Find(const T &) const;
//
// Return true (nonzero) if the element is currently residing in the block.
//
		int Contains(const T &t) const    { return Find(t); }
//
// Return the number of elements currently in the block.
//
    int Length(void) const    { return length; }

//
// Returns the first gNode in the list
//
	gNode<T> *First(void) const {return head;}
	gNode<T> *First(void) {return head;}

//
// Empty the block
//
		void Flush(void)   {
			length = 0;
			gNode<T> *n=First();
			while (n) {gNode<T> *next=n->Next(); delete n; n=next;}
			head = 0; }

//
// Print the contents of the block (for debugging purposes)
//
    void Dump(gOutput &) const;

};


#ifdef __GNUG__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type
#endif   //# __GNUG__, __BORLANDC__

template <class T> INLINE gList<T>::gList(const gList<T> &b)
 : length(b.length)
{
	if (length)   {
		gNode<T> *n1=b.First();
		head=new gNode<T>(n1->Data(),NULL,NULL);
		n1=n1->Next();
		gNode<T> *n=First();
		while (n1->Next())
		{
			n->SetNext(new gNode<T>(n1->Data(),n,NULL));
			n=n->Next();n1=n1->Next();
		}
	}
	else
		head = 0;
}

template <class T> INLINE gList<T> &gList<T>::operator=(const gList<T> &b)
{
	if (this != &b)   {
		Flush();
		length = b.length;
		if (length)   {
			gNode<T> *n1=b.First();
			head=new gNode<T>(n1->Data(),NULL,NULL);
			n1=n1->Next();
			gNode<T> *n=First();
			while (n1->Next())
			{
				n->SetNext(new gNode<T>(n1->Data(),n,NULL));
				n=n->Next();n1=n1->Next();
			}
		}
		else
			head = 0;
	}
	return *this;
}

template <class T> INLINE int gList<T>::operator==(const gList<T> &b) const
{
	if (length != b.length) return 0;
	gNode<T> *n=First();
	gNode<T> *n1=b.First();
	while (n) {if (n1!=n) return 0;n=n->Next();n1=n1->Next();}
	return 1;
}

template <class T> INLINE const T &gList<T>::operator[](int num) const
{
	assert(num >= 1 && num <= length);
	gNode<T> *n=First();
	for (int i=1;i<num;i++) n=n->Next();
	return n->Data();
}

template <class T> INLINE T &gList<T>::operator[](int num)
{
	assert(num >= 1 && num <= length);
	gNode<T> *n=First();
	for (int i=1;i<num;i++) n=n->Next();
	return n->Data();
}

template <class T> INLINE
	gList<T> gList<T>::operator+(const gList<T>& b) const
{
	gList<T> result(*this);
	for (int i = 1; i <= b.length; i++)
		result.Append(b[i]);
	return result;
}

template <class T> INLINE int gList<T>::InsertAt(const T &t, int num)
{
	gNode<T> *n=First();
	if (n)
	{
		for (int i = 0; i < num-1 ; i++) n=n->Next();
		gNode<T> *temp=new gNode<T>(t,n,n->Next());
		n->SetNext(temp);
//		n->Next()->SetPrev(temp);
		length++;
	}
	else
		head=new gNode<T>(t,NULL,NULL);
	return num;
}

template <class T> INLINE int gList<T>::Append(const T &t)
{
	return InsertAt(t, length + 1);
}

template <class T> INLINE int gList<T>::Insert(const T &t, int n)
{
	return InsertAt(t, (n < 1) ? 1 : ((n > length + 1) ? length + 1 : n));
}

template <class T> INLINE T gList<T>::Remove(int num)
{
	assert(num >= 1 && num <= length);
	gNode<T> *n=First();
	for (int i = 0; i <num-1; i++) n=n->Next();
	n->Prev()->SetNext(n->Next());
	n->Next()->SetPrev(n->Prev());
	T ret=n->Data();
	delete n;
	return ret;
}

template <class T> INLINE int gList<T>::Find(const T &t) const
{
	gNode<T> *n=First();
	int found=0,i=1;
	while (n && !found) {if (n->Data()==t) found=i;i++;n=n->Next();}
	return found;
}

template <class T> INLINE void gList<T>::Dump(gOutput &f) const
{
	f << "gList " << this << " contents\n";
	if (length)
	{
		gNode<T> *n=First();
		int i=1;
		while (n)
			{f << i << ": " << (n->Data()) << '\n';i++;n=n->Next();}
	}
}

//
// Uses the Dump function to output the gList.  Uses the << operator
// overload to use output streams, gout.
//
template <class T> inline gOutput &operator<<(gOutput &f, const gList<T> &b)
{
  b.Dump(f);   return f;
}


#endif    //# gList_H
