//
//  File:  header file for Portion class, campanion to GSM
//
//


#include "rational.h"
#include "gstring.h"
#include "gambitio.h"


//typedef enum { false = 0, true = 1 } bool;

typedef enum 
{ 
  porERROR, porBOOL, porDOUBLE, porINTEGER, porRATIONAL, 
  porSTRING, porREFERENCE 
} PortionType;

class GSM;


class Portion
{
 protected:
  PortionType type;
  
 public:
  Portion( void );
  PortionType Type( void ) const;
};



class bool_Portion : public Portion
{
 private:
  bool value;

 public:
  bool_Portion( const bool new_value );
  bool Value( void ) const;
  bool& Value( void );
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

class gString_Portion : public Portion
{
 private:
  gString value;

 public:
  gString_Portion( const gString new_value );
  gString Value( void ) const;
  gString& Value( void );
};

class Reference_Portion : public Portion
{
 private:
  gString value;

 public:
  Reference_Portion( const gString new_value );
  gString Value( void ) const;
  gString& Value( void );
};





