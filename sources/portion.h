//
//  File: implementation of Portion class, campanion to GSM
//
//


#include "rational.h"



typedef enum { porERROR, porDOUBLE, porINTEGER, porRATIONAL } PortionType;

class GSM;


class Portion
{
 protected:
  PortionType type;
  
 public:
  Portion( void );
  PortionType Type( void ) const;
};

class double_Portion : public Portion
{
 private:
  double value;

 public:
  double_Portion( const double new_value );
  double Value( void ) const;
  double& Value( void );
//  operator double&( void );
};

class gInteger_Portion : public Portion
{
 private:
  gInteger value;

 public:
  gInteger_Portion( const gInteger new_value );
  gInteger Value( void ) const;
  gInteger& Value( void );
// operator gInteger&( void );
};

class gRational_Portion : public Portion
{
 private:
  gRational value;

 public:
  gRational_Portion( const gRational new_value );
  gRational Value( void ) const;
  gRational& Value( void );
//  operator gRational&( void );
};

