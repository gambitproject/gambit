// File: paramsd.h -- header file for paramsd.cc : Class for setting the
// gPoly parameter values for normal and extensive forms
// $Id$

#ifndef PARAMSD_H
#define PARAMSD_H

class ParameterSet: public gBlock<gNumber>
{
private:
	gText	name;
public:
   ParameterSet(int l=1,gText n="var") : gBlock<gNumber>(l), name(n) { }
   ParameterSet(const ParameterSet &s): gBlock<gNumber>(s),name(s.name) { }
   void SetName(const gText &m) {name=m;}
   gText Name(void) const {return name;}
};

class ParameterSetList: public gList<ParameterSet>
{
private:
	gText	name;
   int cur_set;
   bool polyval;
public:
	ParameterSetList(gSpace *space,gText n="ParamSet"):
                                         name(n),cur_set(1),polyval(false)
   {Append(ParameterSet(space->Dmnsn(),"DefaultSet"));}
   gText Name(void) const {return name;}
   int &CurSetNum(void) {return cur_set;}
   gArray<gNumber> &CurSet(void) {return (*this)[cur_set];}
   bool &PolyVal(void) {return polyval;}
};

class ParameterDialogC;
class ParametrizedGame
{
public:
   virtual void ChangeParameters(int what) = 0;
   virtual ParameterSetList &Parameters(void) = 0;
};

class ParameterDialog
{
private:
	ParameterDialogC *d;
public:
	ParameterDialog(gSpace *space,ParametrizedGame *game,wxFrame *parent);
   ~ParameterDialog();
};

#endif

