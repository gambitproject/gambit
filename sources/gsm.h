//
//  FILE: GSM.h  definition of GSM (stack machine) type
//
//


#ifndef GSM_H
#define GSM_H

#include "rational.h"
#include "gstack.h"
#include "gambitio.h"

#include "portion.h"




class GSM
{
 private:
  gStack<Portion *> *stack;
  typedef enum { opERROR, opADD, opSUBTRACT, opMULTIPLY, opDIVIDE 
		 } OperationMode;
  void operation( OperationMode mode );
  void double_operation( double_Portion *p1, 
			double_Portion *p2,
			OperationMode mode); 
  void gInteger_operation( gInteger_Portion *p1, 
			  gInteger_Portion *p2,
			  OperationMode mode); 
  void gRational_operation( gRational_Portion *p1, 
			   gRational_Portion *p2,
			   OperationMode mode); 

 public:
  GSM( int size );
  ~GSM();

  int Depth( void ) const;
  int MaxDepth( void ) const;

  void Push( const double data );
  void Push( const gInteger data );
  void Push( const gRational data );

  void Pop( double &data );
  void Pop( gInteger &data );
  void Pop( gRational &data );

  void Peek( double &data );
  void Peek( gInteger &data );
  void Peek( gRational &data );

  void Add( void );
  void Subtract( void );
  void Multiply( void );
  void Divide( void );
  void Negate( void );

  void Dump( void );
};


#endif  // GSM_H

