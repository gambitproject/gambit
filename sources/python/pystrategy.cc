//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for strategy objects
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <Python.h>
#include "pygambit.h"
#include "game/nfg.h"

/*************************************************************************
 * STRATEGY: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void strategy_dealloc(strategyobject *);
staticforward PyObject *strategy_getattr(strategyobject *, char *);
staticforward int strategy_compare(strategyobject *, strategyobject *); 
staticforward PyObject *strategy_str(strategyobject *);

PyTypeObject Strategytype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "strategy",                           /* tp_name */
  sizeof(strategyobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  strategy_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   0,         /* tp_print    "print x"     */
  (getattrfunc) strategy_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     strategy_compare,      /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     strategy_str,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
strategy_getlabel(strategyobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_strategy->GetLabel());
}

static PyObject *
strategy_getplayer(strategyobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfplayerobject *player = newnfplayerobject();
  player->m_nfplayer = new gbtNfgPlayer(self->m_strategy->GetPlayer());
  return (PyObject *) player;
}

static PyObject *
strategy_setlabel(strategyobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_strategy->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef strategy_methods[] = {
  { "GetLabel", (PyCFunction) strategy_getlabel, 1 },
  { "GetPlayer", (PyCFunction) strategy_getplayer, 1 }, 
  { "SetLabel", (PyCFunction) strategy_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

strategyobject *
newstrategyobject(void)
{
  strategyobject *self;
  self = PyObject_NEW(strategyobject, &Strategytype);
  if (self == NULL) {
    return NULL;
  }
  self->m_strategy = new gbtNfgAction();
  return self;
}

static void                   
strategy_dealloc(strategyobject *self) 
{                            
  delete self->m_strategy;
  PyMem_DEL(self);           
}

static PyObject *
strategy_getattr(strategyobject *self, char *name)
{
  return Py_FindMethod(strategy_methods, (PyObject *) self, name);
}

static int
strategy_compare(strategyobject *obj1, strategyobject *obj2)
{
  // Implementation: If strategies are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_strategy == *obj2->m_strategy) {
    return 0;
  }
  else if (obj1->m_strategy < obj2->m_strategy) {
    return -1;
  }
  else {
    return 1;
  }
}

static PyObject *
strategy_str(strategyobject *self)
{
  return PyString_FromFormat("<{strategy} \"%s\">",
			     (char *) self->m_strategy->GetLabel());
}

/*************************************************************************
 * NFGCONTINGENCY: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void nfgcontingency_dealloc(nfgcontingencyobject *);
staticforward PyObject *nfgcontingency_getattr(nfgcontingencyobject *, char *);
staticforward int nfgcontingency_compare(nfgcontingencyobject *, nfgcontingencyobject *); 
staticforward PyObject *nfgcontingency_str(nfgcontingencyobject *);

PyTypeObject Nfgcontingencytype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "nfgcontingency",                           /* tp_name */
  sizeof(nfgcontingencyobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  nfgcontingency_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   0,         /* tp_print    "print x"     */
  (getattrfunc) nfgcontingency_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     nfgcontingency_compare,      /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     nfgcontingency_str,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
nfgcontingency_getoutcome(nfgcontingencyobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfoutcomeobject *outcome = newnfoutcomeobject();
  *outcome->m_nfoutcome = self->m_contingency->GetOutcome();
  return (PyObject *) outcome;
}

static PyObject *
nfgcontingency_getstrategy(nfgcontingencyobject *self, PyObject *args)
{
  PyObject *player;

  if (!PyArg_ParseTuple(args, "O", &player)) {
    return NULL;
  }

  if (!is_nfplayerobject(player)) {
    return NULL;
  }

  strategyobject *strategy = newstrategyobject();
  *strategy->m_strategy = self->m_contingency->GetStrategy(((nfplayerobject *) player)->m_nfplayer->GetId());
  return (PyObject *) strategy;
}

static PyObject *
nfgcontingency_setoutcome(nfgcontingencyobject *self, PyObject *args)
{
  PyObject *outcome;

  if (!PyArg_ParseTuple(args, "O", &outcome)) {
    return NULL;
  }

  if (!is_nfoutcomeobject(outcome)) {
    return NULL;
  }

  self->m_contingency->SetOutcome(*((nfoutcomeobject *) outcome)->m_nfoutcome);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
nfgcontingency_setstrategy(nfgcontingencyobject *self, PyObject *args)
{
  PyObject *strategy;

  if (!PyArg_ParseTuple(args, "O", &strategy)) {
    return NULL;
  }

  if (!is_strategyobject(strategy)) {
    return NULL;
  }

  self->m_contingency->SetStrategy(*((strategyobject *) strategy)->m_strategy);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef nfgcontingency_methods[] = {
  { "GetOutcome", (PyCFunction) nfgcontingency_getoutcome, 1 },
  { "GetStrategy", (PyCFunction) nfgcontingency_getstrategy, 1 },
  { "SetOutcome", (PyCFunction) nfgcontingency_setoutcome, 1 },
  { "SetStrategy", (PyCFunction) nfgcontingency_setstrategy, 1 },
  { NULL, NULL }
};


/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nfgcontingencyobject *
newnfgcontingencyobject(void)
{
  nfgcontingencyobject *self;
  self = PyObject_NEW(nfgcontingencyobject, &Nfgcontingencytype);
  if (self == NULL) {
    return NULL;
  }
  self->m_contingency = 0;
  return self;
}

static void                   
nfgcontingency_dealloc(nfgcontingencyobject *self) 
{                            
  delete self->m_contingency;
  PyMem_DEL(self);           
}

static PyObject *
nfgcontingency_getattr(nfgcontingencyobject *self, char *name)
{
  return Py_FindMethod(nfgcontingency_methods, (PyObject *) self, name);
}

static int
nfgcontingency_compare(nfgcontingencyobject *obj1, nfgcontingencyobject *obj2)
{
  // Implementation: If contingencies are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_contingency == *obj2->m_contingency) {
    return 0;
  }
  else if (obj1->m_contingency < obj2->m_contingency) {
    return -1;
  }
  else {
    return 1;
  }
}

static PyObject *
nfgcontingency_str(nfgcontingencyobject *self)
{
  return PyString_FromFormat("<{nfgcontingency}>");
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initstrategy(void)
{
  Strategytype.ob_type = &PyType_Type;
  Nfgcontingencytype.ob_type = &PyType_Type;
}
