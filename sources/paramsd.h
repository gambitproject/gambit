// File: paramsd.h -- header file for paramsd.cc : Class for setting the
// gPoly parameter values for normal and extensive forms
// $Id$

#ifndef PARAMSD_H
#define PARAMSD_H

class ParameterSet: public gBlock<gNumber>
{
private:
	gString	name;
public:
   ParameterSet(int l=1,gString n="var") : gBlock<gNumber>(l), name(n) { }
   ParameterSet(const ParameterSet &s): gBlock<gNumber>(s),name(s.name) { }
   void SetName(const gString &m) {name=m;}
   gString Name(void) const {return name;}
};

class ParameterSetList: public gList<ParameterSet>
{
private:
	gString	name;
   int cur_set;
public:
	ParameterSetList(gSpace *space,gString n="ParamSet"):name(n),cur_set(1)
   {Append(ParameterSet(space->Dmnsn(),"DefaultSet"));}
   gString Name(void) const {return name;}
   int &CurSetNum(void) {return cur_set;}
   gArray<gNumber> &CurSet(void) {return (*this)[cur_set];}
};

class ParameterDialogC;
class ParametrizedGame
{
public:
	virtual void UpdateSpace(void) = 0;
};

class ParameterDialog
{
private:
	ParameterDialogC *d;
public:
	ParameterDialog(gSpace *space,ParameterSetList &params,wxFrame *parent,
                   ParametrizedGame *game);
   ~ParameterDialog();
   int Completed(void);
};

#endif

