//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Definition of gStack container type
//

#ifndef GSTACK_H
#define GSTACK_H

// gStack -- implements a stack
// This implementation is designed for use with built-in or other simple
// data types.  It won't be very efficient with complicated classes or
// structs; another implementation would be necessary.
// This implementation uses a fixed-size array to minimize copying,
// allocation, and deallocation; the array currently cannot be expanded
// or contracted at all.

template <class T> class gStack  {
protected:
  int depth;
  int max_depth;
  T*  contents;

public:
  class StackUnderflow : public gException {
  public:
    virtual ~StackUnderflow() { }
    gText Description(void) const { return "gStack underflow error"; }
  };

  // CONSTRUCTORS AND DESTRUCTOR
  gStack(int md = 10);       // initialize stack of maximum depth 'md'
  virtual ~gStack();    // destruct gStack

  // GENERAL DATA ACCESS
  int Depth(void) const;
  int MaxDepth(void) const;
    
  // STACK OPERATIONS
  //       Peek() and Pop() assume that depth > 0
  void Push (T new_el);    // push an element onto top of stack
  T    Pop  (void);        // pop the top element off the stack
  T    Peek (void) const;  // get the top element without popping
  T&   Peek (void);        // also get the top element w/o popping
                            // (the const version is used to just examine
                            //  the data without modifying it; the non-const
                            //  version allows the TOS to be modified
                            //  without the overhead of a push+pop --
                            //  use this with care!)
  void Resize(int new_max_depth);
  void Flush (void);       // empty the stack in one fell swoop
};

#endif    // GSTACK_H

