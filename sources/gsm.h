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




class RefHashTable;


class GSM
{
 private:
  gStack<Portion *> *stack;
  gStack<gString> *RefStack;
  RefHashTable *RefTable;

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
  Portion *resolve_ref( Reference_Portion *p );

 public:
  GSM( int size );
  ~GSM();

  int Depth( void ) const;
  int MaxDepth( void ) const;

  void Push( const double data );
  void Push( const gInteger data );
  void Push( const gRational data );
  void Push( const gString data );

  void Add( void );
  void Subtract( void );
  void Multiply( void );
  void Divide( void );
  void Negate( void );

  void Dump( void );

  void PushRef( const gString data );

  void PushVal( const double data );
  void PushVal( const gInteger data );
  void PushVal( const gRational data );

  void Assign( const gString ref, const double data );
  void Assign( const gString ref, const gInteger data );
  void Assign( const gString ref, const gRational data );
  void UnAssign( const gString ref );
};


#endif  // GSM_H

