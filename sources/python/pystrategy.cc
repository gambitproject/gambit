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
staticforward int strategy_print(strategyobject *, FILE *, int);

PyTypeObject Strategytype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "strategy",                           /* tp_name */
  sizeof(strategyobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  strategy_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   strategy_print,         /* tp_print    "print x"     */
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
  (reprfunc)     0,                /* tp_str     "str(x)"  */
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
  self->m_strategy = new gbtNfgStrategy();
  return self;
}

static void                   
strategy_dealloc(strategyobject *self) 
{                            
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
  // Implementation: If outcomes are the game underlying object, return
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

static int
strategy_print(strategyobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{strategy} \"%s\">", (char *) self->m_strategy->GetLabel());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initstrategy(void)
{
  Strategytype.ob_type = &PyType_Type;
}
