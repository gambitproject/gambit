//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for node objects
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
 * NODE: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void node_dealloc(nodeobject *);
staticforward PyObject *node_getattr(nodeobject *, char *);
staticforward int node_compare(nodeobject *, nodeobject *); 
staticforward int node_print(nodeobject *, FILE *, int);

PyTypeObject Nodetype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "node",                           /* tp_name */
  sizeof(nodeobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  node_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   node_print,         /* tp_print    "print x"     */
  (getattrfunc) node_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     node_compare,      /* tp_compare  "x > y"       */
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
node_deletemove(nodeobject *self, PyObject *args)
{
  PyObject *keep;

  if (!PyArg_ParseTuple(args, "O", &keep)) {
    return NULL;
  }

  if (!is_nodeobject(keep)) {
    return NULL;
  }

  self->m_node->DeleteMove(*((nodeobject *) keep)->m_node);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
node_deletetree(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  self->m_node->DeleteTree();
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
node_getchild(nodeobject *self, PyObject *args)
{
  int index;

  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  if (index < 1 || index > self->m_node->NumChildren()) {
    return NULL;
  }

  nodeobject *child = newnodeobject();
  *child->m_node = self->m_node->GetChild(index);
  return (PyObject *) child;
}

static PyObject *
node_getgame(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efgobject *game = newefgobject();
  game->m_efg = new gbtEfgGame(self->m_node->GetGame());
  return (PyObject *) game;
}

static PyObject *
node_getinfoset(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  infosetobject *infoset = newinfosetobject();
  infoset->m_infoset = new gbtEfgInfoset(self->m_node->GetInfoset());
  return (PyObject *) infoset;
}

static PyObject *
node_getlabel(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_node->GetLabel());
}

static PyObject *
node_getnextsibling(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nodeobject *node = newnodeobject();
  node->m_node = new gbtEfgNode(self->m_node->GetNextSibling());
  return (PyObject *) node;
}

static PyObject *
node_getoutcome(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efoutcomeobject *outcome = newefoutcomeobject();
  outcome->m_efoutcome = new gbtEfgOutcome(self->m_node->GetOutcome());
  return (PyObject *) outcome;
}

static PyObject *
node_getparent(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nodeobject *node = newnodeobject();
  node->m_node = new gbtEfgNode(self->m_node->GetParent());
  return (PyObject *) node;
}

static PyObject *
node_getprioraction(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  actionobject *action = newactionobject();
  action->m_action = new gbtEfgAction(self->m_node->GetPriorAction());
  return (PyObject *) action;
}


static PyObject *
node_getpriorsibling(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nodeobject *node = newnodeobject();
  node->m_node = new gbtEfgNode(self->m_node->GetPriorSibling());
  return (PyObject *) node;
}

static PyObject *
node_insertmove(nodeobject *self, PyObject *args)
{
  PyObject *infoset;

  if (!PyArg_ParseTuple(args, "O", &infoset)) {
    return NULL;
  }

  if (!is_infosetobject(infoset)) {
    return NULL;
  }

  nodeobject *newnode = newnodeobject();
  newnode->m_node = new gbtEfgNode(self->m_node->InsertMove(*((infosetobject *) infoset)->m_infoset));
  return (PyObject *) newnode;
}

static PyObject *
node_ispredecessorof(nodeobject *self, PyObject *args)
{
  PyObject *node;

  if (!PyArg_ParseTuple(args, "O", &node)) {
    return NULL;
  }

  if (!is_nodeobject(node)) {
    return NULL;
  }

  return Py_BuildValue("b", 
		       self->m_node->IsPredecessorOf(*((nodeobject *) node)->m_node));
}

static PyObject *
node_numchildren(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_node->NumChildren());
}

static PyObject *
node_setlabel(nodeobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_node->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
node_setoutcome(nodeobject *self, PyObject *args)
{
  PyObject *outcome;

  if (!PyArg_ParseTuple(args, "O", &outcome)) {
    return NULL;
  }

  if (!is_efoutcomeobject(outcome)) {
    return NULL;
  }

  self->m_node->SetOutcome(*((efoutcomeobject *) outcome)->m_efoutcome);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef node_methods[] = {
  { "DeleteMove", (PyCFunction) node_deletemove, 1 },
  { "DeleteTree", (PyCFunction) node_deletetree, 1 },
  { "GetChild", (PyCFunction) node_getchild, 1 },
  { "GetGame", (PyCFunction) node_getgame, 1 }, 
  { "GetInfoset", (PyCFunction) node_getinfoset, 1 }, 
  { "GetLabel", (PyCFunction) node_getlabel, 1 },
  { "GetNextSibling", (PyCFunction) node_getnextsibling, 1 },
  { "GetOutcome", (PyCFunction) node_getoutcome, 1 },
  { "GetParent", (PyCFunction) node_getparent, 1 },
  { "GetPriorAction", (PyCFunction) node_getprioraction, 1 },
  { "GetPriorSibling", (PyCFunction) node_getpriorsibling, 1 },
  { "InsertMove", (PyCFunction) node_insertmove, 1 },
  { "IsPredecessorOf", (PyCFunction) node_ispredecessorof, 1 }, 
  { "NumChildren", (PyCFunction) node_numchildren, 1 },
  { "SetLabel", (PyCFunction) node_setlabel, 1 },
  { "SetOutcome", (PyCFunction) node_setoutcome, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nodeobject *
newnodeobject(void)
{
  nodeobject *self;
  self = PyObject_NEW(nodeobject, &Nodetype);
  if (self == NULL) {
    return NULL;
  }
  self->m_node = new gbtEfgNode();
  return self;
}

static void                   
node_dealloc(nodeobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
node_getattr(nodeobject *self, char *name)
{
  return Py_FindMethod(node_methods, (PyObject *) self, name);
}

static int
node_compare(nodeobject *obj1, nodeobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_node == *obj2->m_node) {
    return 0;
  }
  else if (obj1->m_node < obj2->m_node) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
node_print(nodeobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{node} \"%s\">", (char *) self->m_node->GetLabel());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initnode(void)
{
  Nodetype.ob_type = &PyType_Type;
}
