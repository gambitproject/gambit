//
//  FILE: GSM.h  definition of GSM (stack machine) type
//
//


#ifndef GSM_H
#define GSM_H


#include "gstack.h"


typedef int T;


class GSM
{
 private:
  gStack<T> *stack;

 public:
  GSM( int size );
  ~GSM();

  int Depth( void ) const;
  int MaxDepth( void ) const;

  void Push( T element );
  T Pop( void );
  T Peek( void ) const;
  T& Peek( void );

  void Add( void );
  void Subtract( void );
  void Multiply( void );
  void Divide( void );
  void Negate( void );

  void Dump( void );
};


#endif  // GSM_H

