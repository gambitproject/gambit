//
//  File:  header file for Portion class, campanion to GSM
//
//


#include "rational.h"
#include "gstring.h"


typedef enum { porERROR, porDOUBLE, porINTEGER, porRATIONAL, porREFERENCE } PortionType;

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
};

class gInteger_Portion : public Portion
{
 private:
  gInteger value;

 public:
  gInteger_Portion( const gInteger new_value );
  gInteger Value( void ) const;
  gInteger& Value( void );
};

class gRational_Portion : public Portion
{
 private:
  gRational value;

 public:
  gRational_Portion( const gRational new_value );
  gRational Value( void ) const;
  gRational& Value( void );
};

class Reference_Portion : public Portion
{
 private:
  gString value;
//  PortionType ref_data_type;

 public:
  Reference_Portion( const gString new_value );
  gString Value( void ) const;
  gString& Value( void );
//  PortionType RefDataType( void );
};





