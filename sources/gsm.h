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

  typedef enum 
  { 
    opERROR, opADD, opSUBTRACT, opMULTIPLY, opDIVIDE,
    opEQUAL_TO, opNOT_EQUAL_TO, opGREATER_THAN, opLESS_THAN,
    opGREATER_THAN_OR_EQUAL_TO, opLESS_THAN_OR_EQUAL_TO,
    opLOGICAL_AND, opLOGICAL_OR, opLOGICAL_NOT
  } OperationMode;

  int operation( OperationMode mode );

  int bool_operation(bool_Portion *p1, 
		     bool_Portion *p2,
		     OperationMode mode); 
  int double_operation(double_Portion *p1, 
		       double_Portion *p2,
		       OperationMode mode); 
  int gInteger_operation(gInteger_Portion *p1, 
			 gInteger_Portion *p2,
			 OperationMode mode); 
  int gRational_operation(gRational_Portion *p1, 
			  gRational_Portion *p2,
			  OperationMode mode); 
  int gString_operation(gString_Portion *p1, 
			gString_Portion *p2,
			OperationMode mode);

  Portion *resolve_ref( Reference_Portion *p );



 public:
  GSM( int size );
  ~GSM();

  int Depth( void ) const;
  int MaxDepth( void ) const;

  void Push( const bool      data );
  void Push( const double    data );
  void Push( const gInteger  data );
  void Push( const gRational data );
  void Push( const gString   data );

  void PushRef( const gString   data );

  void PushVal( const bool      data );
  void PushVal( const double    data );
  void PushVal( const gInteger  data );
  void PushVal( const gRational data );
  void PushVal( const gString   data );

  void Assign( const gString ref, const bool     data );
  void Assign( const gString ref, const double    data );
  void Assign( const gString ref, const gInteger  data );
  void Assign( const gString ref, const gRational data );
  void Assign( const gString ref, const gString   data );
  void UnAssign( const gString ref );


  void Add      ( void );
  void Subtract ( void );
  void Multiply ( void );
  void Divide   ( void );
  void Negate   ( void );

  int  EqualTo              ( void );
  int  NotEqualTo           ( void );
  int  GreaterThan          ( void );
  int  LessThan             ( void );
  int  GreaterThanOrEqualTo ( void );
  int  LessThanOrEqualTo    ( void );
  
  void AND ( void );
  void OR  ( void );
  void NOT ( void );


/*
  void operator +  ( void );
  void operator -  ( void );
  void operator *  ( void );
  void operator /  ( void );

  int  operator == ( void );
  int  operator != ( void );
  int  operator >  ( void );
  int  operator <  ( void );
  int  operator >= ( void );
  int  operator <= ( void );
  
  void operator && ( void );
  void operator || ( void );
  void operator !  ( void );
*/

  void Output( void );
  void Dump( void );

};


#endif  // GSM_H

