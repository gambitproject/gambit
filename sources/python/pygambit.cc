//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension module for Gambit
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

extern PyObject *gbt_new_efg(PyObject *, PyObject *);
extern PyObject *gbt_read_efg(PyObject *, PyObject *); 
extern PyObject *gbt_read_nfg(PyObject *, PyObject *);
extern PyObject *gbt_nash_logit_nfg(PyObject *, PyObject *);

static struct PyMethodDef gambit_methods[] = {
  { "LogitSolve", gbt_nash_logit_nfg, 1 },
  { "NewEfg", gbt_new_efg, 1 },
  { "ReadEfg", gbt_read_efg, 1 },
  { "ReadNfg", gbt_read_nfg, 1 },
  { NULL, NULL }
};

extern void initbehav(void);
extern void initefg(void);
extern void initinfoset(void);
extern void initmixed(void);
extern void initnash(void);
extern void initnfg(void);
extern void initnode(void);
extern void initoutcome(void);
extern void initplayer(void);
extern void initstrategy(void);

extern "C" void
initgambit(void)
{
  PyObject *m;

  initbehav();
  initefg();
  initinfoset();
  initmixed();
  initnash();
  initnfg();
  initnode();
  initoutcome();
  initplayer();
  initstrategy();

  m = Py_InitModule("gambit", gambit_methods);

  if (PyErr_Occurred()) {
    Py_FatalError("can't initialize module gambit");
  }
}
